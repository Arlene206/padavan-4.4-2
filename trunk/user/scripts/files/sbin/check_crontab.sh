#!/bin/sh
# $1-5: crontab expression fields; "a" is the router's wildcard alias.
# $6: script basename under /usr/bin
# Exit 0: entry already exists; exit 1: entry appended; exit 2+: update failed.
[ "$#" -eq 6 ] || exit 2
[ -n "$6" ] || exit 0

for field in "$1" "$2" "$3" "$4" "$5"; do
	case "$field" in
		''|*[!0-9a*/,-]*) exit 2 ;;
	esac
	[ "${#field}" -le 32 ] || exit 2
done

case "$6" in
	[!A-Za-z0-9]*|*[!A-Za-z0-9_.-]*|*..*) exit 2 ;;
esac

cd /etc/storage || exit 2
exp=`printf '%s %s %s %s %s' "$1" "$2" "$3" "$4" "$5" | tr 'a' '*'` || exit 2
cron_file="cron/crontabs/admin"
script_path="/usr/bin/$6"

# Reject pre-existing symlinked components before creating/appending below.
[ ! -L cron ] || exit 2
[ ! -L cron/crontabs ] || exit 2
if [ -e cron ] && [ ! -d cron ]; then
	exit 2
fi
if [ -e cron/crontabs ] && [ ! -d cron/crontabs ]; then
	exit 2
fi
umask 077
mkdir -p cron/crontabs || exit 2
[ -d cron/crontabs ] || exit 2
[ ! -L "$cron_file" ] || exit 2
if [ -e "$cron_file" ] && [ ! -f "$cron_file" ]; then
	exit 2
fi

if [ -f "$cron_file" ]; then
	if grep -F -- "$script_path" "$cron_file" >/dev/null 2>&1; then
		exit 0
	else
		grep_status=$?
		case "$grep_status" in
			1) ;;
			*) exit 2 ;;
		esac
	fi
fi

printf '%s %s > /dev/null 2>&1\n' "$exp" "$script_path" >> "$cron_file" || exit 2
exit 1
