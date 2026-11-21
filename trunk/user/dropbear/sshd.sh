#!/bin/sh

dir_storage="${DROPBEAR_STORAGE_DIR:-/etc/storage/dropbear}"
rsa_key="$dir_storage/rsa_host_key"
dss_key="$dir_storage/dss_host_key"
ecdsa_key="$dir_storage/ecdsa_host_key"
ed25519_key="$dir_storage/ed25519_host_key"
extra_args="$dir_storage/extra_args"
old_pattern="${DROPBEAR_OLD_KEY_PREFIX:-/etc/storage/dropbear_}"
dropbear_bin="${DROPBEAR_BIN:-/usr/sbin/dropbear}"
dropbearkey_bin="${DROPBEAR_KEYGEN_BIN:-/usr/bin/dropbearkey}"

func_log()
{
	logger -t "dropbear" "$1"
}

func_keygen()
{
	key_type="$1"
	key_file="$2"
	if "$dropbearkey_bin" -t "$key_type" -f "$key_file"; then
		:
	else
		keygen_status=$?
		func_log "host-key generation failed for $key_type (status=$keygen_status)"
		return "$keygen_status"
	fi
	if chmod 600 "$key_file"; then
		return 0
	else
		chmod_status=$?
		func_log "host-key permission update failed for $key_type (status=$chmod_status)"
		return "$chmod_status"
	fi
}

func_createkeys()
{
	if [ ! -d "$dir_storage" ]; then
		if mkdir -p -m 755 "$dir_storage"; then :; else
			storage_status=$?
			func_log "cannot create key storage (status=$storage_status)"
			return "$storage_status"
		fi
	fi
	if rm -f "$rsa_key" "$dss_key" "$ecdsa_key" "$ed25519_key"; then :; else
		remove_status=$?
		func_log "cannot remove previous host keys (status=$remove_status)"
		return "$remove_status"
	fi
	func_keygen rsa "$rsa_key"
	key_status=$?
	[ "$key_status" -eq 0 ] || return "$key_status"
	func_keygen dss "$dss_key"
	key_status=$?
	[ "$key_status" -eq 0 ] || return "$key_status"
	func_keygen ecdsa "$ecdsa_key"
	key_status=$?
	[ "$key_status" -eq 0 ] || return "$key_status"
	func_keygen ed25519 "$ed25519_key"
	return $?
}

func_get_extra_args()
{
	if [ ! -f "$extra_args" ]; then
		printf '%s\n' "-p 22" > "$extra_args" || return $?
	fi
	cat "$extra_args"
}

func_start()
{
	if [ ! -d "$dir_storage" ]; then
		if mkdir -p -m 755 "$dir_storage"; then :; else
			storage_status=$?
			func_log "cannot create key storage (status=$storage_status)"
			return "$storage_status"
		fi
	fi
	args=$(func_get_extra_args)
	args_status=$?
	if [ "$args_status" -ne 0 ]; then
		func_log "cannot read daemon arguments (status=$args_status)"
		return "$args_status"
	fi
	for i in rsa_host_key dss_host_key ecdsa_host_key; do
		old_key="${old_pattern}_${i}"
		if [ -f "$old_key" ]; then
			if mv -n "$old_key" "$dir_storage/$i"; then :; else
				move_status=$?
				func_log "cannot migrate host key $old_key (status=$move_status)"
				return "$move_status"
			fi
		fi
	done
	if [ ! -f "$rsa_key" ] || [ ! -f "$dss_key" ]; then
		func_createkeys
		key_status=$?
		[ "$key_status" -eq 0 ] || return "$key_status"
	fi
	if [ ! -f "$ecdsa_key" ]; then
		func_keygen ecdsa "$ecdsa_key"
		key_status=$?
		[ "$key_status" -eq 0 ] || return "$key_status"
	fi
	if [ ! -f "$ed25519_key" ]; then
		func_keygen ed25519 "$ed25519_key"
		key_status=$?
		[ "$key_status" -eq 0 ] || return "$key_status"
	fi

	# Keep the persisted whitespace-separated extra_args convention without xargs
	# exit-status translation or pathname expansion.
	if [ -n "$1" ]; then
		args="$1 $args"
	fi
	set -f
	set -- $args
	if "$dropbear_bin" "$@"; then
		return 0
	else
		launch_status=$?
		func_log "daemon launch failed (status=$launch_status)"
		return "$launch_status"
	fi
}

func_stop()
{
	if killall -q dropbear; then
		return 0
	else
		stop_status=$?
		func_log "daemon stop failed (status=$stop_status)"
		return "$stop_status"
	fi
}

action_status=0
case "$1" in
start)
	func_start "$2"
	action_status=$?
	;;
stop)
	func_stop
	action_status=$?
	;;
newkeys)
	func_createkeys
	action_status=$?
	;;
*)
	echo "Usage: $0 {start|stop|newkeys}"
	action_status=1
	;;
esac
exit "$action_status"
