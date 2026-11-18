#!/bin/sh
# Remove only an explicitly owned DNSMasq directive. Near-matches remain.
ss_dnsmasq_remove_exact_directive() {
	[ "$#" -eq 2 ] || return 2
	local ss_dnsmasq_config_file="$1"
	local ss_dnsmasq_expected="$2"
	local ss_dnsmasq_tmp_file=""
	local ss_dnsmasq_match=""
	local ss_dnsmasq_status=0

	[ -n "$ss_dnsmasq_config_file" ] && [ -n "$ss_dnsmasq_expected" ] || return 2
	[ -f "$ss_dnsmasq_config_file" ] || return 0
	[ ! -L "$ss_dnsmasq_config_file" ] || return 1
	[ -r "$ss_dnsmasq_config_file" ] && [ -w "$ss_dnsmasq_config_file" ] || return 1

	if awk -v expected="$ss_dnsmasq_expected" '
		{ line = $0; sub(/^[[:space:]]*/, "", line); sub(/[[:space:]]*$/, "", line); if (line == expected) found = 1 }
		END { exit(found ? 0 : 1) }
	' "$ss_dnsmasq_config_file"; then
		:
	else
		ss_dnsmasq_status=$?
		[ "$ss_dnsmasq_status" -eq 1 ] && return 0
		return "$ss_dnsmasq_status"
	fi

	ss_dnsmasq_tmp_file=$(mktemp "${ss_dnsmasq_config_file}.tmp.XXXXXX") || return 1
	if cp -p "$ss_dnsmasq_config_file" "$ss_dnsmasq_tmp_file"; then :; else
		ss_dnsmasq_status=$?; rm -f "$ss_dnsmasq_tmp_file"; return "$ss_dnsmasq_status"
	fi
	if awk -v expected="$ss_dnsmasq_expected" '
		{ line = $0; sub(/^[[:space:]]*/, "", line); sub(/[[:space:]]*$/, "", line); if (line != expected) print $0 }
	' "$ss_dnsmasq_config_file" > "$ss_dnsmasq_tmp_file"; then :; else
		ss_dnsmasq_status=$?; rm -f "$ss_dnsmasq_tmp_file"; return "$ss_dnsmasq_status"
	fi

	if cmp -s "$ss_dnsmasq_config_file" "$ss_dnsmasq_tmp_file"; then
		rm -f "$ss_dnsmasq_tmp_file"; return 0
	else
		ss_dnsmasq_status=$?
		if [ "$ss_dnsmasq_status" -ne 1 ]; then rm -f "$ss_dnsmasq_tmp_file"; return "$ss_dnsmasq_status"; fi
	fi
	if mv -f "$ss_dnsmasq_tmp_file" "$ss_dnsmasq_config_file"; then
		return 0
	else
		ss_dnsmasq_status=$?
		rm -f "$ss_dnsmasq_tmp_file"
		return "$ss_dnsmasq_status"
	fi
}
