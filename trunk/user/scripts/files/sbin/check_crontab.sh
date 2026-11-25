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

storage_dir=/etc/storage
[ ! -L "$storage_dir" ] && [ -d "$storage_dir" ] || exit 2
cd "$storage_dir" || exit 2
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

# Serialize the check-and-append operation so simultaneous service
# startups cannot race and create duplicate crontab entries.
lock_file=cron/.admin.lock
command -v flock >/dev/null 2>&1 || exit 2
[ ! -L "$lock_file" ] || exit 2
if [ -e "$lock_file" ] && [ ! -f "$lock_file" ]; then
	exit 2
fi
if [ ! -e "$lock_file" ]; then
	if ! ( set -C; : > "$lock_file" ) 2>/dev/null; then
		[ ! -L "$lock_file" ] && [ -f "$lock_file" ] || exit 2
	fi
fi
[ ! -L "$lock_file" ] && [ -f "$lock_file" ] || exit 2
exec 9>>"$lock_file" || exit 2
flock -x 9 || exit 2

# Recheck the target after taking the lock, then compare the actual command
# token in field six. Substring matches in comments or similarly named
# executables must not suppress a required job.
[ ! -L cron ] && [ -d cron ] || exit 2
[ ! -L cron/crontabs ] && [ -d cron/crontabs ] || exit 2
[ ! -L "$cron_file" ] || exit 2
if [ -e "$cron_file" ] && [ ! -f "$cron_file" ]; then
	exit 2
fi
if [ -f "$cron_file" ]; then
	[ -r "$cron_file" ] || exit 2
	while IFS= read -r cron_line || [ -n "$cron_line" ]; do
		# A comment may be indented; only the first non-whitespace character
		# determines whether the record is a crontab comment.
		cron_line_trimmed=${cron_line#"${cron_line%%[![:space:]]*}"}
		case "$cron_line_trimmed" in
			''|'#'*) continue ;;
		esac
		set -f
		set -- $cron_line_trimmed
		set +f
		if [ "$#" -ge 6 ] && [ "$6" = "$script_path" ]; then
			exit 0
		fi
	done < "$cron_file"
fi

printf '%s %s > /dev/null 2>&1\n' "$exp" "$script_path" >> "$cron_file" || exit 2
exit 1
