#!/bin/sh

result=0
mtd_part_name="Storage"
mtd_part_dev="/dev/mtdblock5"
mtd_part_size=65536
dir_storage="/etc/storage"
slk="/tmp/.httpd-private/.storage_locked"
tmp="/tmp/.httpd-private/storage.tar"
tbz="${tmp}.bz2"
rtbz="/tmp/.httpd-private/storage-restore.tar.bz2"
hsh="/tmp/.httpd-private/storage_md5"
lock_dir="/tmp/.httpd-private"
lock_file="${lock_dir}/.mtd_storage.lock"

# Serialize every MTD storage command (save, backup, restore, erase/reset,
# load and fill). The marker file blocks later saves after destructive
# operations; this advisory lock also closes the check-then-run race when a
# save has already started as restore/erase begins.
if [ -L "$lock_dir" ] || { [ -e "$lock_dir" ] && [ ! -d "$lock_dir" ]; }; then
	logger -t "Storage" "Unsafe storage lock directory: $lock_dir"
	exit 1
fi
if [ ! -d "$lock_dir" ] && ! mkdir -m 700 "$lock_dir"; then
	logger -t "Storage" "Cannot create storage lock directory: $lock_dir"
	exit 1
fi
if ! chmod 700 "$lock_dir"; then
	logger -t "Storage" "Cannot secure storage lock directory: $lock_dir"
	exit 1
fi
if [ -L "$lock_file" ] || { [ -e "$lock_file" ] && [ ! -f "$lock_file" ]; }; then
	logger -t "Storage" "Unsafe storage operation lock file: $lock_file"
	exit 1
fi
umask 077
if ! exec 9>>"$lock_file"; then
	logger -t "Storage" "Cannot open storage operation lock: $lock_file"
	exit 1
fi
if ! flock -n 9; then
	logger -t "Storage" "Another MTD storage operation is already running"
	exit 1
fi

func_get_mtd()
{
	local mtd_info mtd_idx mtd_hex

	# Parse exactly one complete /proc/mtd record. Do not truncate the device
	# number to two characters or let duplicate/malformed labels pick a target.
	mtd_info=$(awk -v name="\"$mtd_part_name\"" '
		$4 == name {
			if (NF != 4 || $1 !~ /^mtd[0-9]+:$/ ||
			    $2 !~ /^[[:xdigit:]]+$/ || $3 !~ /^[[:xdigit:]]+$/) {
				invalid = 1
				next
			}
			idx = substr($1, 4, length($1) - 4)
			if (idx ~ /^0[0-9]/ || idx + 0 < 4) {
				invalid = 1
				next
			}
			matches++
			selected_idx = idx
			selected_size = $2
		}
		END {
			if (invalid || matches != 1)
				exit 1
			printf "%s %s\n", selected_idx, selected_size
		}
	' /proc/mtd) || {
		logger -t "Storage" "Cannot identify one valid MTD partition: $mtd_part_name"
		exit 1
	}
	IFS=' ' read -r mtd_idx mtd_hex <<EOF_MTD_INFO
$mtd_info
EOF_MTD_INFO
	mtd_part_size=$((0x$mtd_hex))
	if [ "$mtd_part_size" -le 0 ]; then
		logger -t "Storage" "Invalid MTD partition size: $mtd_part_name"
		exit 1
	fi
	mtd_part_dev="/dev/mtdblock$mtd_idx"
}

func_mdir()
{
	if [ -L "$dir_storage" ] || [ -f "$dir_storage" ] ||
	   { [ -e "$dir_storage" ] && [ ! -d "$dir_storage" ]; }; then
		logger -t "Storage" "Unsafe storage directory: $dir_storage"
		return 1
	fi
	if [ ! -d "$dir_storage" ] && ! mkdir -p -m 755 "$dir_storage"; then
		logger -t "Storage" "Cannot create storage directory: $dir_storage"
		return 1
	fi
}

# Return 0 when rstats is running, 1 when absent and 2 when procfs
# cannot provide a complete answer. A partial scan must never authorize a
# destructive storage restore/reset.
func_rstats_presence()
{
	local proc_root="/proc"
	local proc_entry proc_pid stat_line stat_comm stat_fields stat_state
	local stat_ppid stat_pgrp stat_session stat_start_time stat_value found

	[ -d "$proc_root" ] && [ -r "$proc_root" ] || return 2
	found=0
	for proc_entry in "$proc_root"/[0-9]*; do
		[ -d "$proc_entry" ] || continue
		proc_pid=${proc_entry##*/}
		case "$proc_pid" in
			''|*[!0-9]*) continue ;;
		esac
		if ! IFS= read -r stat_line < "$proc_entry/stat"; then
			[ ! -d "$proc_entry" ] && continue
			return 2
		fi
		case "$stat_line" in
			"$proc_pid ("*) ;;
			*) return 2 ;;
		esac
		stat_comm=${stat_line#"$proc_pid ("}
		case "$stat_comm" in
			*") "*) ;;
			*) return 2 ;;
		esac
		stat_comm=${stat_comm%)*}
		# The closing ") " is the only reliable boundary because comm may
		# itself contain spaces or right parentheses. A complete Linux stat
		# record has fields through starttime (20 tokens after the PID/comm).
		stat_fields=${stat_line##*) }
		set -- $stat_fields
		[ "$#" -ge 20 ] || return 2
		stat_state=$1
		stat_ppid=$2
		stat_pgrp=$3
		stat_session=$4
		stat_start_time=${20}
		case "$stat_state" in
			R|S|D|T|t|Z|X|I) ;;
			*) return 2 ;;
		esac
		for stat_value in "$stat_ppid" "$stat_pgrp" "$stat_session" "$stat_start_time"; do
			case "$stat_value" in
				''|*[!0-9]*) return 2 ;;
			esac
		done
		# A zombie/dead task still has a proc directory but is not a live
		# rstats daemon and must not satisfy startup or block storage teardown.
		if [ "$stat_comm" = "rstats" ] &&
		   [ "$stat_state" != "Z" ] && [ "$stat_state" != "X" ]; then
			found=1
		fi
	done
	if [ "$found" -eq 1 ]; then
		return 0
	fi
	return 1
}

func_stop_apps()
{
	local attempts presence

	func_rstats_presence
	presence=$?
	case "$presence" in
		1) return 0 ;;
		0) ;;
		*) logger -t "Storage" "Cannot verify rstats process inventory; refusing storage mutation"; return 1 ;;
	esac

	# killall status alone is not a stop postcondition. Recheck procfs until the
	# daemon is confirmed absent; a missing/partial inventory aborts the caller.
	killall -q rstats >/dev/null 2>&1 || true
	for attempts in 1 2 3 4 5; do
		func_rstats_presence
		presence=$?
		case "$presence" in
			1) return 0 ;;
			0) ;;
			*) logger -t "Storage" "rstats process state became unknown; refusing storage mutation"; return 1 ;;
		esac
		[ "$attempts" -lt 5 ] && sleep 1 || :
	done

	logger -t "Storage" "rstats did not stop within the bounded wait; refusing storage mutation"
	return 1
}

func_start_apps()
{
	local attempts presence

	if ! /sbin/rstats 9>&-; then
		logger -t "Storage" "Unable to launch rstats"
		return 1
	fi
	# A successful launcher exit is not proof that its daemon survived.
	# Require a complete procfs inventory to confirm the process is present;
	# UNKNOWN must fail closed just like the bounded stop-side check.
	for attempts in 1 2 3 4 5; do
		func_rstats_presence
		presence=$?
		case "$presence" in
			0) return 0 ;;
			1) ;;
			*) logger -t "Storage" "Cannot verify rstats startup; storage service state is unknown"; return 1 ;;
		esac
		[ "$attempts" -lt 5 ] && sleep 1 || :
	done

	logger -t "Storage" "rstats launcher exited successfully but daemon did not become visible"
	return 1
}

func_load()
{
	local fsz load_storage backup_storage storage_parent storage_name backup_suffix had_storage hash_tmp

	# A decompressor can emit a prefix and still report corruption.
	if ! bzcat "$mtd_part_dev" > "$tmp" 2>/dev/null; then
		fsz=$(stat -c %s "$tmp" 2>/dev/null)
		if [ "$fsz" = "0" ]; then
			# A successful erase leaves a one-shot save lock until the next boot
			# confirms that the MTD is blank and default storage can be recreated.
			# Keep the lock if the stale hash cannot be removed: func_save() must
			# not mistake the blank partition for an unchanged saved snapshot.
			if ! rm -f "$hsh"; then
				logger -t "Storage load" "Unable to clear storage hash for an empty MTD partition; save lock retained"
			elif ! rm -f "$slk"; then
				logger -t "Storage load" "Unable to clear save lock for an empty MTD partition"
			fi
		fi
		result=1
		rm -f "$tmp"
		logger -t "Storage load" "Unable to decompress storage data from MTD partition: $mtd_part_dev"
		return 1
	fi

	fsz=$(stat -c %s "$tmp" 2>/dev/null)
	case "$fsz" in
		""|*[!0-9]*)
			result=1
			rm -f "$tmp"
			logger -t "Storage load" "Invalid storage data size in MTD partition: $fsz"
			return 1
			;;
	esac
	if [ "$fsz" -le 0 ]; then
		result=1
		rm -f "$tmp"
		if ! rm -f "$hsh"; then
			logger -t "Storage load" "Unable to clear storage hash for an empty MTD partition; save lock retained"
		elif ! rm -f "$slk"; then
			logger -t "Storage load" "Unable to clear save lock for an empty MTD partition"
		fi
		logger -t "Storage load" "Empty storage data in MTD partition: $mtd_part_dev"
		return 1
	fi
	if [ -L "$dir_storage" ] || { [ -e "$dir_storage" ] && [ ! -d "$dir_storage" ]; }; then
		result=1
		rm -f "$tmp"
		logger -t "Storage load" "Storage destination is not a real directory: $dir_storage"
		return 1
	fi

	# Extract into a private sibling on the same tmpfs, not into live settings.
	storage_parent=${dir_storage%/*}
	storage_name=${dir_storage##*/}
	load_storage=$(mktemp -d "${storage_parent}/.${storage_name}.load.XXXXXX" 2>/dev/null)
	if [ -z "$load_storage" ] || [ ! -d "$load_storage" ]; then
		result=1
		rm -f "$tmp"
		logger -t "Storage load" "Cannot allocate private storage load directory"
		return 1
	fi
	if ! tar xf "$tmp" -C "$load_storage" 2>/dev/null; then
		result=1
		rm -rf "$load_storage"
		rm -f "$tmp"
		logger -t "Storage load" "Unable to extract storage archive from MTD partition: $mtd_part_dev"
		return 1
	fi
	if ! chmod 755 "$load_storage"; then
		result=1
		rm -rf "$load_storage"
		rm -f "$tmp"
		logger -t "Storage load" "Unable to set storage directory permissions: $load_storage"
		return 1
	fi

	# Stage the cache only after a complete extraction.
	hash_tmp="${hsh}.tmp.$$"
	if ! md5sum "$tmp" > "$hash_tmp"; then
		result=1
		rm -rf "$load_storage"
		rm -f "$tmp" "$hash_tmp"
		logger -t "Storage load" "Unable to stage storage hash checkpoint"
		return 1
	fi

	backup_suffix=0
	backup_storage="${storage_parent}/.${storage_name}.load-backup.$$"
	while [ -e "$backup_storage" ] || [ -L "$backup_storage" ]; do
		backup_suffix=$((backup_suffix + 1))
		if [ "$backup_suffix" -gt 999 ]; then
			result=1
			rm -rf "$load_storage"
			rm -f "$tmp" "$hash_tmp"
			logger -t "Storage load" "Cannot allocate storage rollback pathname"
			return 1
		fi
		backup_storage="${storage_parent}/.${storage_name}.load-backup.$$-$backup_suffix"
	done

	had_storage=0
	if [ -d "$dir_storage" ]; then
		if ! mv "$dir_storage" "$backup_storage"; then
			result=1
			rm -rf "$load_storage"
			rm -f "$tmp" "$hash_tmp"
			logger -t "Storage load" "Unable to preserve the previous storage directory: $dir_storage"
			return 1
		fi
		had_storage=1
	fi
	if [ -e "$dir_storage" ] || [ -L "$dir_storage" ] ||
	   ! mv "$load_storage" "$dir_storage"; then
		result=1
		if [ "$had_storage" -eq 1 ] && [ ! -e "$dir_storage" ] && [ ! -L "$dir_storage" ]; then
			if ! mv "$backup_storage" "$dir_storage"; then
				logger -t "Storage load" "CRITICAL: unable to restore previous storage directory; backup remains at $backup_storage"
			fi
		elif [ "$had_storage" -eq 1 ]; then
			logger -t "Storage load" "CRITICAL: destination reappeared; previous storage preserved at $backup_storage"
		fi
		rm -rf "$load_storage"
		rm -f "$tmp" "$hash_tmp"
		logger -t "Storage load" "Unable to publish verified storage directory: $dir_storage"
		return 1
	fi

	# A cache failure loses only the skip optimization; keep the complete tree.
	if ! mv -f "$hash_tmp" "$hsh"; then
		result=1
		logger -t "Storage load" "Storage loaded but hash checkpoint publication failed"
	fi
	if [ "$had_storage" -eq 1 ] && ! rm -rf "$backup_storage"; then
		result=1
		logger -t "Storage load" "Storage loaded but previous directory cleanup failed: $backup_storage"
	fi
	if ! rm -f "$tmp"; then
		result=1
		logger -t "Storage load" "Storage loaded but temporary archive cleanup failed: $tmp"
	fi
	if ! rm -f "$slk"; then
		result=1
		logger -t "Storage load" "Storage loaded but storage lock marker cleanup failed: $slk"
	fi
}
func_tarb()
{
	local find_list

	rm -f "$tmp"
	if ! cd "$dir_storage"; then
		logger -t "Storage" "Cannot enter storage directory: $dir_storage"
		exit 1
	fi
	# A shell glob omits dotfiles, and filtering directories drops empty dirs.
	# Snapshot the NUL-delimited inventory first: without pipefail, a find or
	# sort error in the old pipelines could be masked by a successful xargs.
	find_list=$(mktemp "${tmp}.find.XXXXXX" 2>/dev/null)
	if [ -z "$find_list" ] || [ ! -f "$find_list" ]; then
		cd - >/dev/null
		logger -t "Storage" "Cannot allocate storage inventory list"
		exit 1
	fi
	if ! find . ! -name . -print0 > "$find_list"; then
		rm -f "$find_list" "$tmp"
		cd - >/dev/null
		logger -t "Storage" "Cannot enumerate complete storage tree"
		exit 1
	fi
	if ! xargs -0 touch -c -h -t 201001010000.00 < "$find_list"; then
		rm -f "$find_list" "$tmp"
		cd - >/dev/null
		logger -t "Storage" "Cannot normalize storage file timestamps"
		exit 1
	fi
	# Let tar recurse from the root instead of passing the inventory via
	# xargs. BusyBox xargs caps command size at 32 KiB, so an oversized tree
	# invokes "tar -c" repeatedly and each invocation truncates the same archive.
	if ! tar -cf "$tmp" . 2>/dev/null; then
		rm -f "$find_list" "$tmp"
		cd - >/dev/null
		logger -t "Storage" "Cannot create complete storage tarball: $tmp"
		exit 1
	fi
	rm -f "$find_list"
	cd - >/dev/null
	if [ ! -f "$tmp" ] || [ ! -s "$tmp" ]; then
		logger -t "Storage" "Cannot create tarball file: $tmp"
		exit 1
	fi
}

func_save()
{
	local fsz hash_tmp

	logger -t "Storage save" "Save storage files to MTD partition \"$mtd_part_dev\""
	echo "Save storage files to MTD partition \"$mtd_part_dev\""
	hash_tmp="${hsh}.tmp.$$"
	rm -f "$tbz" "$hash_tmp"
	if md5sum -c "$hsh" >/dev/null 2>&1; then
		echo "Storage hash is not changed, skip write to MTD partition. Exit."
		rm -f "$tmp"
		return 0
	fi
	if ! md5sum "$tmp" > "$hash_tmp"; then
		result=1
		echo "Error! Cannot stage storage hash"
		logger -t "Storage save" "Cannot stage storage hash"
		rm -f "$tmp" "$hash_tmp"
		return 1
	fi
	if ! bzip2 -9 "$tmp" 2>/dev/null; then
		result=1
		echo "Error! Cannot compress storage archive"
		logger -t "Storage save" "Cannot compress storage archive"
		rm -f "$tmp" "$tbz" "$hash_tmp"
		return 1
	fi
	fsz=`stat -c %s "$tbz" 2>/dev/null`
	if [ -n "$fsz" ] && [ "$fsz" -ge 16 ] && [ "$fsz" -le "$mtd_part_size" ] ; then
		if mtd_write write "$tbz" "$mtd_part_name"; then
			if mv -f "$hash_tmp" "$hsh"; then
				echo "Done."
				logger -t "Storage save" "Done."
			else
				result=1
				echo "Error! MTD write succeeded but storage hash checkpoint failed"
				logger -t "Storage save" "MTD write succeeded but storage hash checkpoint failed"
			fi
		else
			result=1
			echo "Error! MTD write FAILED"
			logger -t "Storage save" "Error write to MTD partition: $mtd_part_dev"
		fi
	else
		result=1
		echo "Error! Invalid storage final data size: $fsz"
		logger -t "Storage save" "Invalid storage final data size: $fsz"
	fi
	rm -f "$tmp" "$tbz" "$hash_tmp"
}

func_backup()
{
	local fsz

	rm -f "$tbz"
	if ! bzip2 -9 "$tmp" 2>/dev/null; then
		result=1
		rm -f "$tmp" "$tbz"
		logger -t "Storage backup" "Cannot create BZ2 file!"
		return 1
	fi

	fsz=$(stat -c %s "$tbz" 2>/dev/null)
	case "$fsz" in
		""|*[!0-9]*)
			result=1
			rm -f "$tmp" "$tbz"
			logger -t "Storage backup" "Invalid BZ2 file size: $fsz"
			return 1
			;;
	esac
	if [ "$fsz" -lt 16 ] || [ "$fsz" -gt "$mtd_part_size" ]; then
		result=1
		rm -f "$tmp" "$tbz"
		logger -t "Storage backup" "BZ2 archive size is outside the restorable MTD range: $fsz"
		return 1
	fi
	# Validate the compressed stream and tar structure with firmware applets.
	if ! bzcat "$tbz" > "$tmp" 2>/dev/null ||
	   ! tar tf "$tmp" >/dev/null 2>&1; then
		result=1
		rm -f "$tmp" "$tbz"
		logger -t "Storage backup" "Created BZ2 archive failed integrity verification"
		return 1
	fi
	rm -f "$tmp"
	return 0
}

func_restore()
{
	local fsz tmp_storage restore_suffix stage_storage rollback_storage rollback_suffix had_storage

	[ ! -f "$rtbz" ] && exit 1

	# Block concurrent MTD saves before reading or replacing the restore archive.
	if ! touch "$slk"; then
		result=1
		logger -t "Storage restore" "Cannot create storage save lock before restore: $slk"
		return 1
	fi

	fsz=`stat -c %s "$rtbz" 2>/dev/null`
	if [ -z "$fsz" ] || [ "$fsz" -lt 16 ] || [ "$fsz" -gt "$mtd_part_size" ] ; then
		result=1
		rm -f "$rtbz"
		logger -t "Storage restore" "Invalid BZ2 file size: $fsz"
		return 1
	fi

	tmp_storage="/tmp/.httpd-private/storage-restore"
	restore_suffix=0
	while ! mkdir -m 700 "$tmp_storage" 2>/dev/null; do
		restore_suffix=$((restore_suffix + 1))
		if [ "$restore_suffix" -gt 999 ]; then
			result=1
			logger -t "Storage restore" "Cannot allocate private restore scratch directory"
			return 1
		fi
		tmp_storage="/tmp/.httpd-private/storage-restore-$restore_suffix"
	done
	tar xjf "$rtbz" -C "$tmp_storage" 2>/dev/null
	if [ $? -ne 0 ] ; then
		result=1
		rm -rf "$tmp_storage"
		logger -t "Storage restore" "Unable to extract BZ2 file: $tbz"
		return 1
	fi
	if [ -L "$tmp_storage/start_script.sh" ] || [ ! -f "$tmp_storage/start_script.sh" ] ; then
		result=1
		rm -rf "$tmp_storage"
		logger -t "Storage restore" "Invalid content of BZ2 file: $tbz"
		return 1
	fi

	# Reject unexpected destination types rather than moving or replacing them.
	if [ -L "$dir_storage" ] || { [ -e "$dir_storage" ] && [ ! -d "$dir_storage" ]; }; then
		result=1
		rm -rf "$tmp_storage"
		logger -t "Storage restore" "Storage destination is not a real directory: $dir_storage"
		return 1
	fi

	# Copy the complete restored tree beside /etc/storage. This keeps the
	# installation rename on the same filesystem and leaves the live tree
	# untouched if staging or copying fails.
	stage_storage=`mktemp -d "${dir_storage}.restore.XXXXXX" 2>/dev/null`
	if [ -z "$stage_storage" ] || [ ! -d "$stage_storage" ]; then
		result=1
		rm -rf "$tmp_storage"
		logger -t "Storage restore" "Cannot allocate same-filesystem restore staging directory"
		return 1
	fi
	if ! cp -a "$tmp_storage"/. "$stage_storage"/ || ! chmod 755 "$stage_storage"; then
		result=1
		rm -rf "$stage_storage" "$tmp_storage"
		logger -t "Storage restore" "Cannot prepare complete storage restore tree"
		return 1
	fi

	rollback_storage="${dir_storage}.rollback.$$"
	rollback_suffix=0
	while [ -e "$rollback_storage" ] || [ -L "$rollback_storage" ]; do
		rollback_suffix=$((rollback_suffix + 1))
		if [ "$rollback_suffix" -gt 999 ]; then
			result=1
			rm -rf "$stage_storage" "$tmp_storage"
			logger -t "Storage restore" "Cannot allocate storage rollback pathname"
			return 1
		fi
		rollback_storage="${dir_storage}.rollback.$$-$rollback_suffix"
	done

	if ! func_stop_apps; then
		result=1
		rm -rf "$stage_storage" "$tmp_storage"
		logger -t "Storage restore" "rstats did not stop cleanly; previous storage tree retained"
		return 1
	fi
	had_storage=0
	if [ -d "$dir_storage" ]; then
		if ! mv "$dir_storage" "$rollback_storage"; then
			result=1
			rm -rf "$stage_storage" "$tmp_storage"
			if [ -d "$dir_storage" ] && [ ! -L "$dir_storage" ] && [ -f "$dir_storage/start_script.sh" ]; then
				func_start_apps
			else
				logger -t "Storage restore" "rstats remains stopped because no storage tree is available: $dir_storage"
			fi
			logger -t "Storage restore" "Cannot preserve the previous storage directory before restore"
			return 1
		fi
		had_storage=1
	fi

	if ! mv "$stage_storage" "$dir_storage"; then
		if [ "$had_storage" -eq 1 ]; then
			if ! mv "$rollback_storage" "$dir_storage"; then
				logger -t "Storage restore" "CRITICAL: restore install and rollback both failed; previous settings remain at $rollback_storage"
			fi
		fi
		result=1
		rm -rf "$stage_storage" "$tmp_storage"
		if [ -d "$dir_storage" ] && [ ! -L "$dir_storage" ] && [ -f "$dir_storage/start_script.sh" ]; then
			func_start_apps
		else
			logger -t "Storage restore" "rstats remains stopped because no storage tree is available: $dir_storage"
		fi
		logger -t "Storage restore" "Cannot install the staged storage directory"
		return 1
	fi

	if ! func_start_apps; then
		result=1
		rm -rf "$tmp_storage"
		logger -t "Storage restore" "Restored storage tree is active, but rstats failed to start; save lock, archive and rollback snapshot retained"
		return 1
	fi

	if ! rm -f "$slk"; then
		result=1
		rm -rf "$tmp_storage"
		logger -t "Storage restore" "Storage restore installed and rstats started, but the storage lock marker could not be removed"
		return 1
	fi

	rm -f "$rtbz"
	rm -rf "$tmp_storage"
	if [ "$had_storage" -eq 1 ] && ! rm -rf "$rollback_storage"; then
		logger -t "Storage restore" "Restored settings are active; previous storage backup remains at $rollback_storage"
	fi
	return 0
}
func_erase()
{
	# Block concurrent persistence before touching the flash partition.
	if ! touch "$slk"; then
		result=1
		logger -t "Storage" "Cannot create storage save lock before MTD erase: $slk"
		return 1
	fi
	if ! func_stop_apps; then
		result=1
		logger -t "Storage" "MTD erase aborted because rstats did not stop cleanly; storage save lock retained"
		return 1
	fi
	if ! mtd_write erase "$mtd_part_name"; then
		# The flash may have been partially erased; retain the lock until boot
		# can inspect the partition instead of allowing a possibly unsafe save.
		result=1
		logger -t "Storage" "MTD erase failed; storage save lock retained: $mtd_part_name"
		return 1
	fi
	if ! rm -f "$hsh"; then
		result=1
		logger -t "Storage" "MTD erase succeeded but storage hash cleanup failed: $hsh"
		return 1
	fi
	if ! rm -rf "$dir_storage"; then
		result=1
		logger -t "Storage" "MTD erase succeeded but storage directory removal failed: $dir_storage"
		return 1
	fi
	if ! mkdir -p -m 755 "$dir_storage"; then
		result=1
		logger -t "Storage" "MTD erase succeeded but storage directory recreation failed: $dir_storage"
		return 1
	fi
	# Keep the marker: only the next boot's blank-MTD check may release it.
	return 0
}

func_reset()
{
	# Keep persistence disabled until the replacement defaults are complete.
	if ! touch "$slk"; then
		result=1
		logger -t "Storage reset" "Cannot create storage save lock: $slk"
		return 1
	fi
	if ! rm -f "$hsh"; then
		result=1
		logger -t "Storage reset" "Cannot remove stale storage hash: $hsh"
		return 1
	fi
	if ! rm -rf "$dir_storage"; then
		result=1
		logger -t "Storage reset" "Cannot remove old storage directory: $dir_storage"
		return 1
	fi
	if ! mkdir -p -m 755 "$dir_storage"; then
		result=1
		logger -t "Storage reset" "Cannot recreate storage directory: $dir_storage"
		return 1
	fi
	return 0
}

func_fill_check_dir()
{
	local target remaining current component

	target=$1
	case "$target" in
		"$dir_storage") remaining= ;;
		"$dir_storage"/*) remaining=${target#"$dir_storage"/} ;;
		*)
			logger -t "Storage fill" "Refusing path outside storage tree: $target"
			return 1
			;;
	esac

	current="$dir_storage"
	if [ -L "$current" ] || { [ -e "$current" ] && [ ! -d "$current" ]; }; then
		logger -t "Storage fill" "Unsafe storage root: $current"
		return 1
	fi
	while [ -n "$remaining" ]; do
		case "$remaining" in
			*/*) component=${remaining%%/*}; remaining=${remaining#*/} ;;
			*) component=$remaining; remaining= ;;
		esac
		current="$current/$component"
		if [ -L "$current" ] || { [ -e "$current" ] && [ ! -d "$current" ]; }; then
			logger -t "Storage fill" "Unsafe storage directory component: $current"
			return 1
		fi
	done
	return 0
}

func_fill_check_file()
{
	local target parent

	target=$1
	parent=${target%/*}
	if ! func_fill_check_dir "$parent"; then
		return 1
	fi
	if [ -L "$target" ] || { [ -e "$target" ] && [ ! -f "$target" ]; }; then
		logger -t "Storage fill" "Unsafe storage file target: $target"
		return 1
	fi
	return 0
}

func_fill_validate_paths()
{
	local path

	# Check every directory and all of its ancestors before mkdir -p or tar
	# extraction can follow a symlink into a path outside /etc/storage.
	for path in "$dir_crond" "$dir_httpssl" "$dir_chnroute" \
		    "$dir_dnsmasq" "$dir_inadyn" "$dir_wlan" \
		    "$dir_ovpncli" "$dir_ovpnsvr" \
		    "$dir_sswan" "$dir_sswan_crt" \
		    "$dir_sswan_crt/cacerts" "$dir_sswan_crt/certs" \
		    "$dir_sswan_crt/private"; do
		if ! func_fill_check_dir "$path"; then
			return 1
		fi
	done

	# Existing leaf symlinks are unsafe too: [ -f ] follows them and would
	# otherwise make fill silently accept an external configuration file.
	for path in "$script_start" "$script_started" "$script_shutd" \
		    "$script_postf" "$script_postw" "$script_inets" \
		    "$script_vpnsc" "$script_vpncs" "$script_ezbtn" \
		    "$dir_storage/dnsmasq.conf" "$dir_storage/hosts" \
		    "$user_hosts" "$user_dnsmasq_conf" \
		    "$user_dnsmasq_servers" "$user_dhcp_conf" \
		    "$user_inadyn_conf" "$dir_wlan/AP.dat" \
		    "$dir_wlan/AP_5G.dat" "$user_ovpnsvr_conf" \
		    "$user_ovpncli_conf" "$user_sswan_conf" \
		    "$user_sswan_ipsec_conf" "$user_sswan_secrets" \
		    "$dir_storage/authorized_keys" \
		    "$dir_storage/openvpn/ca.crt" \
		    "$dir_storage/openvpn/dh1024.pem" \
		    "$dir_storage/openvpn/server.crt" \
		    "$dir_storage/openvpn/server.key" \
		    "$dir_storage/openvpn/server.conf" \
		    "$dir_storage/openvpn/ta.key" \
		    "$dir_ovpnsvr/ca.crt" "$dir_ovpnsvr/dh1024.pem" \
		    "$dir_ovpnsvr/server.crt" "$dir_ovpnsvr/server.key" \
		    "$dir_ovpnsvr/server.conf" "$dir_ovpnsvr/ta.key"; do
		if ! func_fill_check_file "$path"; then
			return 1
		fi
	done
	return 0
}

func_fill_set_mode()
{
	local target mode

	target=$1
	mode=$2
	if [ -L "$target" ] || [ ! -f "$target" ]; then
		logger -t "Storage fill" "Missing or unsafe default file: $target"
		return 1
	fi
	if ! chmod "$mode" "$target"; then
		logger -t "Storage fill" "Cannot set default file mode: $target"
		return 1
	fi
	return 0
}

func_fill_mkdir()
{
	local target mode

	target=$1
	mode=$2
	if ! func_fill_check_dir "$target"; then
		return 1
	fi
	if [ -d "$target" ]; then
		return 0
	fi
	if ! mkdir -p -m "$mode" "$target"; then
		logger -t "Storage fill" "Cannot create storage directory: $target"
		return 1
	fi
	return 0
}

func_fill_begin_file()
{
	local target

	target=$1
	if ! func_fill_check_file "$target"; then
		return 1
	fi
	fill_stage=$(mktemp "${target}.tmp.XXXXXX") || {
		logger -t "Storage fill" "Cannot stage default storage file: $target"
		fill_stage=
		return 1
	}
	return 0
}

func_fill_abort_file()
{
	local stage

	stage=$fill_stage
	fill_stage=
	if [ -n "$stage" ] && ! rm -f "$stage"; then
		logger -t "Storage fill" "CRITICAL: cannot remove incomplete default stage: $stage"
		return 1
	fi
	return 0
}

func_fill_publish_file()
{
	local target mode

	target=$1
	mode=$2
	if ! func_fill_set_mode "$fill_stage" "$mode"; then
		return 1
	fi
	# -n preserves a user file created after the initial absence check.
	if ! mv -n "$fill_stage" "$target"; then
		logger -t "Storage fill" "Cannot publish default storage file: $target"
		return 1
	fi
	if [ -e "$fill_stage" ] || [ -L "$fill_stage" ]; then
		if [ -f "$target" ] && [ ! -L "$target" ]; then
			func_fill_abort_file
			return 0
		fi
		logger -t "Storage fill" "Default storage target changed during publication: $target"
		return 1
	fi
	if [ -L "$target" ] || [ ! -f "$target" ]; then
		logger -t "Storage fill" "Published default storage file is not regular: $target"
		return 1
	fi
	fill_stage=
	return 0
}

func_fill_write_file()
{
	local target mode

	target=$1
	mode=$2
	if ! func_fill_begin_file "$target"; then
		return 1
	fi
	if ! cat > "$fill_stage"; then
		logger -t "Storage fill" "Cannot write default storage file: $target"
		func_fill_abort_file || true
		return 1
	fi
	if ! func_fill_publish_file "$target" "$mode"; then
		func_fill_abort_file || true
		return 1
	fi
	return 0
}

func_fill()
{
	dir_httpssl="$dir_storage/https"
	dir_dnsmasq="$dir_storage/dnsmasq"
	dir_ovpnsvr="$dir_storage/openvpn/server"
	dir_ovpncli="$dir_storage/openvpn/client"
	dir_sswan="$dir_storage/strongswan"
	dir_sswan_crt="$dir_sswan/ipsec.d"
	dir_inadyn="$dir_storage/inadyn"
	dir_crond="$dir_storage/cron/crontabs"
	dir_wlan="$dir_storage/wlan"
	dir_chnroute="$dir_storage/chinadns"
	#dir_gfwlist="$dir_storage/gfwlist"

	script_start="$dir_storage/start_script.sh"
	script_started="$dir_storage/started_script.sh"
	script_shutd="$dir_storage/shutdown_script.sh"
	script_postf="$dir_storage/post_iptables_script.sh"
	script_postw="$dir_storage/post_wan_script.sh"
	script_inets="$dir_storage/inet_state_script.sh"
	script_vpnsc="$dir_storage/vpns_client_script.sh"
	script_vpncs="$dir_storage/vpnc_server_script.sh"
	script_ezbtn="$dir_storage/ez_buttons_script.sh"

	user_hosts="$dir_dnsmasq/hosts"
	user_dnsmasq_conf="$dir_dnsmasq/dnsmasq.conf"
	user_dnsmasq_servers="$dir_dnsmasq/dnsmasq.servers"
	user_dhcp_conf="$dir_dnsmasq/dhcp.conf"
	user_ovpnsvr_conf="$dir_ovpnsvr/server.conf"
	user_ovpncli_conf="$dir_ovpncli/client.conf"
	user_inadyn_conf="$dir_inadyn/inadyn.conf"
	user_sswan_conf="$dir_sswan/strongswan.conf"
	user_sswan_ipsec_conf="$dir_sswan/ipsec.conf"
	user_sswan_secrets="$dir_sswan/ipsec.secrets"
	
	chnroute_file="/etc_ro/chnroute.bz2"
	#gfwlist_conf_file="/etc_ro/gfwlist.bz2"

	# Refuse symlinked parents and leaves before any mkdir, move or heredoc
	# can follow a user-controlled path outside the storage tree.
	if ! func_fill_validate_paths; then
		return 1
	fi

	# create crond dir
	if ! func_fill_mkdir "$dir_crond" 730; then return 1; fi

	# create https dir
	if ! func_fill_mkdir "$dir_httpssl" 700; then return 1; fi

	# create chnroute.txt
	if [ ! -d "$dir_chnroute" ] ; then
		if [ -f "$chnroute_file" ]; then
			if ! func_fill_mkdir "$dir_chnroute" 755; then
				return 1
			fi
			if ! tar jxf "$chnroute_file" -C "$dir_chnroute"; then
				if ! rm -rf "$dir_chnroute"; then
					logger -t "Storage fill" "CRITICAL: failed chnroute extraction left partial directory: $dir_chnroute"
				fi
				logger -t "Storage fill" "Cannot extract chnroute archive: $chnroute_file"
				return 1
			fi
		fi
	fi

	# create gfwlist
	#if [ ! -d "$dir_gfwlist" ] ; then
		#if [ -f "$gfwlist_conf_file" ]; then	
			#mkdir -p "$dir_gfwlist" && tar jxf "$gfwlist_conf_file" -C "$dir_gfwlist"
		#fi
	#fi

	# create start script
	if [ ! -f "$script_start" ] ; then
		if ! reset_ss.sh -a; then
			logger -t "Storage fill" "Cannot generate default start script: $script_start"
			return 1
		fi
		if [ ! -f "$script_start" ] || [ -L "$script_start" ]; then
			logger -t "Storage fill" "Default start-script generator did not publish a safe file: $script_start"
			return 1
		fi
	fi

	# create started script
	if [ ! -f "$script_started" ] ; then
		if ! func_fill_write_file "$script_started" 755 <<EOF
#!/bin/sh

### Custom user script
### Called after router started and network is ready

### Example - load ipset modules
#modprobe ip_set
#modprobe ip_set_hash_ip
#modprobe ip_set_hash_net
#modprobe ip_set_bitmap_ip
#modprobe ip_set_list_set
#modprobe xt_set
echo 4096 131072  6291456 > /proc/sys/net/ipv4/tcp_rmem
echo 4194304 >/proc/sys/net/core/rmem_max
echo 212992 > /proc/sys/net/core/rmem_default

### drop caches
sync && echo 3 > /proc/sys/vm/drop_caches

### Roaming assistant for mt76xx WiFi
#iwpriv ra0 set KickStaRssiLow=-85
#iwpriv ra0 set AssocReqRssiThres=-80
#iwpriv rai0 set KickStaRssiLow=-85
#iwpriv rai0 set AssocReqRssiThres=-80

### Mount SATA disk
#mdev -s

#wing start trojan://password@host
#wing start socks5://192.168.1.9:1080
#ipset add gfwlist 91.108.56.0/22
#ipset add gfwlist 91.108.4.0/22
#ipset add gfwlist 109.239.140.0/24
#ipset add gfwlist 149.154.160.0/20


EOF
		then
			return 1
		fi
	fi

	# create shutdown script
	if [ ! -f "$script_shutd" ] ; then
		if ! func_fill_write_file "$script_shutd" 755 <<EOF
#!/bin/sh

### Custom user script
### Called before router shutdown
### \$1 - action (0: reboot, 1: halt, 2: power-off)

EOF
		then
			return 1
		fi
	fi

	# create post-iptables script
	if [ ! -f "$script_postf" ] ; then
		if ! func_fill_write_file "$script_postf" 755 <<EOF
#!/bin/sh

### Custom user script
### Called after internal iptables reconfig (firewall update)

#wing resume

EOF
		then
			return 1
		fi
	fi

	# create post-wan script
	if [ ! -f "$script_postw" ] ; then
		if ! func_fill_write_file "$script_postw" 755 <<EOF
#!/bin/sh

### Custom user script
### Called after internal WAN up/down action
### \$1 - WAN action (up/down)
### \$2 - WAN interface name (e.g. eth3 or ppp0)
### \$3 - WAN IPv4 address

### Solution for UPnP working in case no delicated (no WHITE) external IP adress
#echo "ext_ip=1.1.1.1" >> /etc/miniupnpd.conf && killall miniupnpd && miniupnpd -f /etc/miniupnpd.conf

EOF
		then
			return 1
		fi
	fi

	# create inet-state script
	if [ ! -f "$script_inets" ] ; then
		if ! func_fill_write_file "$script_inets" 755 <<EOF
#!/bin/sh

### Custom user script
### Called on Internet status changed
### \$1 - Internet status (0/1)
### \$2 - elapsed time (s) from previous state

logger -t "di" "Internet state: \$1, elapsed time: \$2s."

EOF
		then
			return 1
		fi
	fi

	# create vpn server action script
	if [ ! -f "$script_vpnsc" ] ; then
		if ! func_fill_write_file "$script_vpnsc" 755 <<EOF
#!/bin/sh

### Custom user script
### Called after remote peer connected/disconnected to internal VPN server
### \$1 - peer action (up/down)
### \$2 - peer interface name (e.g. ppp10)
### \$3 - peer local IP address
### \$4 - peer remote IP address
### \$5 - peer name

peer_if="\$2"
peer_ip="\$4"
peer_name="\$5"

### example: add static route to private LAN subnet behind a remote peer

func_ipup()
{
#  if [ "\$peer_name" == "dmitry" ] ; then
#    route add -net 192.168.5.0 netmask 255.255.255.0 dev \$peer_if
#  elif [ "\$peer_name" == "victoria" ] ; then
#    route add -net 192.168.8.0 netmask 255.255.255.0 dev \$peer_if
#  fi
   return 0
}

func_ipdown()
{
#  if [ "\$peer_name" == "dmitry" ] ; then
#    route del -net 192.168.5.0 netmask 255.255.255.0 dev \$peer_if
#  elif [ "\$peer_name" == "victoria" ] ; then
#    route del -net 192.168.8.0 netmask 255.255.255.0 dev \$peer_if
#  fi
   return 0
}

case "\$1" in
up)
  func_ipup
  ;;
down)
  func_ipdown
  ;;
esac

EOF
		then
			return 1
		fi
	fi

	# create vpn client action script
	if [ ! -f "$script_vpncs" ] ; then
		if ! func_fill_write_file "$script_vpncs" 755 <<EOF
#!/bin/sh

### Custom user script
### Called after internal VPN client connected/disconnected to remote VPN server
### \$1        - action (up/down)
### \$IFNAME   - tunnel interface name (e.g. ppp5 or tun0)
### \$IPLOCAL  - tunnel local IP address
### \$IPREMOTE - tunnel remote IP address
### \$DNS1     - peer DNS1
### \$DNS2     - peer DNS2

# private LAN subnet behind a remote server (example)
peer_lan="192.168.9.0"
peer_msk="255.255.255.0"

### example: add static route to private LAN subnet behind a remote server

func_ipup()
{
#  route add -net \$peer_lan netmask \$peer_msk gw \$IPREMOTE dev \$IFNAME
   return 0
}

func_ipdown()
{
#  route del -net \$peer_lan netmask \$peer_msk gw \$IPREMOTE dev \$IFNAME
   return 0
}

logger -t vpnc-script "\$IFNAME \$1"

case "\$1" in
up)
  func_ipup
  ;;
down)
  func_ipdown
  ;;
esac

EOF
		then
			return 1
		fi
	fi

	# create Ez-Buttons script
	if [ ! -f "$script_ezbtn" ] ; then
		if ! func_fill_write_file "$script_ezbtn" 755 <<EOF
#!/bin/sh

### Custom user script
### Called on WPS or FN button pressed
### \$1 - button param

[ -x /opt/bin/on_wps.sh ] && /opt/bin/on_wps.sh \$1 &

EOF
		then
			return 1
		fi
	fi

	# create user dnsmasq.conf
	if ! func_fill_mkdir "$dir_dnsmasq" 755; then return 1; fi
	for i in dnsmasq.conf hosts ; do
		if [ -f "$dir_storage/$i" ]; then
			if ! mv -n "$dir_storage/$i" "$dir_dnsmasq"; then
				logger -t "Storage fill" "Cannot migrate legacy dnsmasq file: $dir_storage/$i"
				return 1
			fi
		fi
	done
	if [ ! -f "$user_dnsmasq_conf" ] ; then
		if ! func_fill_begin_file "$user_dnsmasq_conf"; then return 1; fi
		if ! cat > "$fill_stage" <<EOF
# Custom user conf file for dnsmasq
# Please add needed params only!

### Web Proxy Automatic Discovery (WPAD)
dhcp-option=252,"\n"

### Set the limit on DHCP leases, the default is 150
#dhcp-lease-max=150

### Add local-only domains, queries are answered from hosts or DHCP only
#local=/router/localdomain/

### Examples:

### Enable built-in TFTP server
#enable-tftp

### Set the root directory for files available via TFTP.
#tftp-root=/opt/srv/tftp

### Make the TFTP server more secure
#tftp-secure

### Set the boot filename for netboot/PXE
#dhcp-boot=pxelinux.0

### Log for all queries
#log-queries

### Keep DHCP host name valid at any times
#dhcp-to-host

### Do NOT forward queries with no domain part
domain-needed

EOF
		then
			logger -t "Storage fill" "Cannot write default storage file: $user_dnsmasq_conf"
			func_fill_abort_file || true
			return 1
		fi
	if [ -f /usr/bin/vlmcsd ]; then
		if ! cat >> "$fill_stage" <<EOF
### vlmcsd related
srv-host=_vlmcs._tcp,my.router,1688,0,100

EOF
		then
			logger -t "Storage fill" "Cannot append default storage file: $user_dnsmasq_conf"
			func_fill_abort_file || true
			return 1
		fi
	fi

	if [ -f /usr/bin/wing ]; then
		if ! cat >> "$fill_stage" <<EOF
# Custom domains to gfwlist
#gfwlist=mit.edu

EOF
		then
			logger -t "Storage fill" "Cannot append default storage file: $user_dnsmasq_conf"
			func_fill_abort_file || true
			return 1
		fi
	fi

	if [ -d $dir_gfwlist ]; then
		if ! cat >> "$fill_stage" <<EOF
### gfwlist related (resolve by port 5353)
#min-cache-ttl=3600
#conf-dir=/etc/storage/gfwlist

EOF
		then
			logger -t "Storage fill" "Cannot append default storage file: $user_dnsmasq_conf"
			func_fill_abort_file || true
			return 1
		fi
	fi
		if ! func_fill_publish_file "$user_dnsmasq_conf" 644; then
			func_fill_abort_file || true
			return 1
		fi
	fi

	# create user dns servers
	if [ ! -f "$user_dnsmasq_servers" ] ; then
		if ! func_fill_write_file "$user_dnsmasq_servers" 644 <<EOF
# Custom user servers file for dnsmasq
# Example:
#server=/mit.ru/izmuroma.ru/10.25.11.30

EOF
		then
			return 1
		fi
	fi

	# create user dns dhcp_conf
	if [ ! -f "$user_dhcp_conf" ] ; then
		if ! func_fill_write_file "$user_dhcp_conf" 644 <<EOF
# Custom user hosts for dhcp_hostsfile
# Example:
#6C:96:CF:E0:95:55,192.168.1.10,iMac

EOF
		then
			return 1
		fi
	fi

	# create user inadyn.conf"
	if ! func_fill_mkdir "$dir_inadyn" 755; then return 1; fi
	if [ ! -f "$user_inadyn_conf" ] ; then
		if ! func_fill_write_file "$user_inadyn_conf" 644 <<EOF
# Custom user conf file for inadyn DDNS client
# Please add only new custom system!

### Example for twoDNS.de:

#system custom@http_srv_basic_auth
#  ssl
#  checkip-url checkip.two-dns.de /
#  server-name update.twodns.de
#  server-url /update\?hostname=
#  username account
#  password secret
#  alias example.dd-dns.de

EOF
		then
			return 1
		fi
	fi

	# create user hosts
	if [ ! -f "$user_hosts" ] ; then
		if ! func_fill_write_file "$user_hosts" 644 <<EOF
# Custom user hosts file
# Example:
# 192.168.1.100		Boo

EOF
		then
			return 1
		fi
	fi

	# create user AP confs
	if ! func_fill_mkdir "$dir_wlan" 755; then return 1; fi
	if [ ! -f "$dir_wlan/AP.dat" ] ; then
		if ! func_fill_write_file "$dir_wlan/AP.dat" 644 <<EOF
# Custom user AP conf file

EOF
		then
			return 1
		fi
	fi

	if [ ! -f "$dir_wlan/AP_5G.dat" ] ; then
		if ! func_fill_write_file "$dir_wlan/AP_5G.dat" 644 <<EOF
# Custom user AP conf file

EOF
		then
			return 1
		fi
	fi

	# create openvpn files
	if [ -x /usr/sbin/openvpn ] ; then
		if ! func_fill_mkdir "$dir_ovpncli" 700 ||
		   ! func_fill_mkdir "$dir_ovpnsvr" 700; then
			return 1
		fi
		dir_ovpn="$dir_storage/openvpn"
		for i in ca.crt dh1024.pem server.crt server.key server.conf ta.key ; do
			if [ -f "$dir_ovpn/$i" ]; then
				if ! mv -n "$dir_ovpn/$i" "$dir_ovpnsvr"; then
					logger -t "Storage fill" "Cannot migrate legacy OpenVPN file: $dir_ovpn/$i"
					return 1
				fi
			fi
		done
		if [ ! -f "$user_ovpnsvr_conf" ] ; then
			if ! func_fill_write_file "$user_ovpnsvr_conf" 644 <<EOF
# Custom user conf file for OpenVPN server
# Please add needed params only!

### Max clients limit
max-clients 10

### Internally route client-to-client traffic
client-to-client

### Allow clients with duplicate "Common Name"
;duplicate-cn

### Legacy LZO adaptive compression
;comp-lzo adaptive
;push "comp-lzo adaptive"

### Keepalive and timeout
keepalive 10 60

### Process priority level (0..19)
nice 3

### Syslog verbose level
verb 0
mute 10

EOF
			then
				return 1
			fi
		fi

		if [ ! -f "$user_ovpncli_conf" ] ; then
			if ! func_fill_write_file "$user_ovpncli_conf" 644 <<EOF
# Custom user conf file for OpenVPN client
# Please add needed params only!

### If your server certificates with the nsCertType field set to "server"
remote-cert-tls server

### Process priority level (0..19)
nice 0

### Syslog verbose level
verb 0
mute 10

EOF
			then
				return 1
			fi
		fi
	fi

	# create strongswan files
	if [ -x /usr/sbin/ipsec ] ; then
		if ! func_fill_mkdir "$dir_sswan" 700 ||
		   ! func_fill_mkdir "$dir_sswan_crt" 700 ||
		   ! func_fill_mkdir "$dir_sswan_crt/cacerts" 700 ||
		   ! func_fill_mkdir "$dir_sswan_crt/certs" 700 ||
		   ! func_fill_mkdir "$dir_sswan_crt/private" 700; then
			return 1
		fi

		if [ ! -f "$user_sswan_conf" ] ; then
			if ! func_fill_write_file "$user_sswan_conf" 644 <<EOF
### strongswan.conf - user strongswan configuration file

EOF
			then
				return 1
			fi
		fi
		if [ ! -f "$user_sswan_ipsec_conf" ] ; then
			if ! func_fill_write_file "$user_sswan_ipsec_conf" 644 <<EOF
### ipsec.conf - user strongswan IPsec configuration file

EOF
			then
				return 1
			fi
		fi
		if [ ! -f "$user_sswan_secrets" ] ; then
			if ! func_fill_write_file "$user_sswan_secrets" 644 <<EOF
### ipsec.secrets - user strongswan IPsec secrets file

EOF
			then
				return 1
			fi
		fi
	fi
}

case "$1" in
load)
	func_get_mtd
	func_mdir || exit 1
	func_load
	;;
save)
	[ -f "$slk" ] && exit 1
	func_get_mtd
	func_mdir || exit 1
	func_tarb
	func_save
	;;
backup)
	func_get_mtd
	func_mdir || exit 1
	func_tarb
	func_backup
	;;
restore)
	func_get_mtd
	func_restore
	;;
erase)
	func_get_mtd
	func_erase
	;;
reset)
	# Establish the save barrier before stopping apps or touching the tree.
	if ! touch "$slk"; then
		result=1
		logger -t "Storage reset" "Cannot create storage save lock: $slk"
	elif ! func_stop_apps; then
		result=1
		logger -t "Storage reset" "Reset aborted because rstats did not stop cleanly; save lock retained"
	elif func_reset && func_fill; then
		# Keep the save barrier until the services are confirmed started.
		if ! func_start_apps; then
			result=1
			logger -t "Storage reset" "Defaults prepared but storage applications failed to start; save lock retained"
		elif ! rm -f "$slk"; then
			result=1
			logger -t "Storage reset" "Defaults prepared and applications started, but storage save lock cleanup failed: $slk"
		fi
	else
		result=1
		logger -t "Storage reset" "Reset preparation failed; storage save lock retained"
	fi
	;;
fill)
	func_mdir || exit 1
	if ! func_fill; then
		result=1
	fi
	;;
*)
	echo "Usage: $0 {load|save|backup|restore|erase|reset|fill}"
	exit 1
	;;
esac

exit $result
