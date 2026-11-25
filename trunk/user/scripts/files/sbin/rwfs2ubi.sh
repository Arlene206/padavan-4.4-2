#!/bin/sh

sbin_dir=/sbin
proc_mtd=/proc/mtd
dev_dir=/dev

fail()
{
	printf '%s\n' "$*" >&2
	exit 1
}

for tool in ubiattach ubiformat ubimkvol ubidetach; do
	if [ ! -x "$sbin_dir/$tool" ]; then
		fail "Unable to find executable: $sbin_dir/$tool"
	fi
done
for tool in awk mount mtd_write nvram; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		fail "Unable to find executable: $tool"
	fi
done
if [ ! -r "$proc_mtd" ]; then
	fail "Unable to read MTD partition inventory: $proc_mtd"
fi

# Match one exact RWFS partition. A substring match could select RWFS_BACKUP,
# and multiple matches make the target ambiguous; both cases must fail closed.
rwfs_idx=$(awk '
	$4 == "\"RWFS\"" {
		if (NF != 4 || $1 !~ /^mtd[0-9]+:$/ ||
		    $2 !~ /^[[:xdigit:]]+$/ || $3 !~ /^[[:xdigit:]]+$/) {
			invalid = 1
			next
		}
		idx_value = substr($1, 4, length($1) - 4)
		if (idx_value ~ /^0[0-9]/) {
			invalid = 1
			next
		}
		matches++
		selected = idx_value
	}
	END {
		if (invalid || matches != 1)
			exit 1
		print selected
	}
' "$proc_mtd") || fail "RWFS partition is missing, malformed, or ambiguous in $proc_mtd"

case "$rwfs_idx" in
	''|*[!0-9]*) fail "Invalid RWFS MTD index: $rwfs_idx" ;;
esac

mtd_device="$dev_dir/mtd$rwfs_idx"
ubi_device="$dev_dir/ubi$rwfs_idx"
if [ ! -e "$mtd_device" ]; then
	fail "RWFS MTD device does not exist: $mtd_device"
fi

# Reject only exact devices/volumes belonging to this MTD index. Prefix grep
# would confuse ubi5 with ubi50, while also missing ubiblock-backed mounts.
if ! mount_inventory=$(mount 2>/dev/null); then
	fail "Unable to inspect mounted filesystems before RWFS conversion"
fi
printf '%s\\n' "$mount_inventory" | awk -v idx="$rwfs_idx" '
	NF == 0 { next }
	{
		if (NF < 6 || $2 != "on" || $4 != "type") {
			invalid = 1
			next
		}
		source = $1
		ubi = "/dev/ubi" idx
		if (source == ubi ||
		    index(source, ubi "_") == 1 ||
		    index(source, "/dev/ubiblock" idx "_") == 1 ||
		    source == "/dev/mtdblock" idx ||
		    index(source, "ubi" idx ":") == 1)
			found = 1
	}
	END {
		if (NR == 0 || invalid)
			exit 2
		exit !found
	}
'

mount_check_status=$?
case "$mount_check_status" in
	0) fail "RWFS UBI/MTD device is already mounted; refusing to erase $mtd_device" ;;
	1) ;;
	*) fail "Unable to parse mounted filesystem inventory" ;;
esac

# The partition may not currently be attached; that expected case is harmless.
# ubiformat below remains the authoritative guard against an attached/busy MTD.
if ! "$sbin_dir/ubidetach" -p "$mtd_device" >/dev/null 2>&1; then
	printf '%s\n' "No existing UBI attachment could be detached; proceeding to checked format" >&2
fi

if ! mtd_write erase "$mtd_device"; then
	fail "Erase RWFS partition failed: $mtd_device"
fi

if ! "$sbin_dir/ubiformat" "$mtd_device" -y -q; then
	fail "Format RWFS partition ($mtd_device) to UBIFS FAILED!"
fi

if ! "$sbin_dir/ubiattach" -p "$mtd_device" -d "$rwfs_idx"; then
	printf '%s\n' "Attach UBIFS ($mtd_device) FAILED!" >&2
	# An attach command can fail after partially publishing an attachment.
	if ! "$sbin_dir/ubidetach" -p "$mtd_device" >/dev/null 2>&1; then
		printf '%s\n' "Cleanup detach also failed after UBI attach error" >&2
	fi
	exit 1
fi

operation_status=0
if ! "$sbin_dir/ubimkvol" "$ubi_device" -m -N rwfs; then
	printf '%s\n' "Create UBI volume FAILED!" >&2
	operation_status=1
else
	printf '%s\n\n' "Create UBI volume done!"
	if ! nvram set mtd_rwfs_mount=1; then
		printf '%s\n' "Unable to set mtd_rwfs_mount=1 in NVRAM" >&2
		operation_status=1
	elif ! nvram commit; then
		printf '%s\n' "Unable to commit RWFS auto-mount setting to NVRAM" >&2
		operation_status=1
	fi
fi

# Always attempt cleanup after a successful attach, but never let a successful
# detach hide an earlier volume-creation or persistent-configuration failure.
if ! "$sbin_dir/ubidetach" -p "$mtd_device"; then
	printf '%s\n' "Detach UBIFS ($mtd_device) after conversion FAILED!" >&2
	operation_status=1
fi

if [ "$operation_status" -ne 0 ]; then
	exit 1
fi

printf '%s\n' "Please reboot your router to automount RWFS partition at '/media/mtd_rwfs'."
printf '%s\n' "To automount RWFS to /opt, create the 'opt' directory in '/media/mtd_rwfs' before rebooting."
printf '%s\n' "To install Entware in /opt, set 'optw_enable=2' in NVRAM and run 'opt-start.sh'."
exit 0
