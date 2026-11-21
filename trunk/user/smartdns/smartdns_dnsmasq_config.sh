#!/bin/sh
# Manage only SmartDNS's marked DNSMasq block. Legacy state uses exact directives.
smartdns_dnsmasq_normalize_port()
{
	[ "$#" -eq 1 ] || return 2
	smartdns_port_value="$1"
	case "$smartdns_port_value" in ''|*[!0-9]*) return 1 ;; esac
	[ "${#smartdns_port_value}" -le 5 ] || return 1
	while [ "${smartdns_port_value#0}" != "$smartdns_port_value" ]; do
		smartdns_port_value="${smartdns_port_value#0}"
	done
	[ -n "$smartdns_port_value" ] || smartdns_port_value=0
	[ "$smartdns_port_value" -ge 1 ] 2>/dev/null &&
	[ "$smartdns_port_value" -le 65535 ] 2>/dev/null || return 1
	printf '%s\n' "$smartdns_port_value"
}

smartdns_dnsmasq_publish_managed_block()
{
	[ "$#" -ge 2 ] && [ "$#" -le 4 ] || return 2
	smartdns_config_file="$1"
	shift
	[ -f "$smartdns_config_file" ] && [ ! -L "$smartdns_config_file" ] &&
	[ -r "$smartdns_config_file" ] && [ -w "$smartdns_config_file" ] || return 1

	if smartdns_scan_result=$(awk '
		$0=="# smartdns-managed-dnsmasq-begin" ||
		$0=="# smartdns-managed-dnsmasq-end" { found=1 }
		END { print found ? 1 : 0 }
	' "$smartdns_config_file"); then
		:
	else
		smartdns_status=$?
		return "$smartdns_status"
	fi
	case "$smartdns_scan_result" in
		0) ;;
		1) return 2 ;;
		*) return 1 ;;
	esac

	smartdns_tmp_file=$(mktemp "${smartdns_config_file}.tmp.XXXXXX") || return 1
	if cp -p "$smartdns_config_file" "$smartdns_tmp_file"; then :; else
		smartdns_status=$?
		rm -f "$smartdns_tmp_file"
		return "$smartdns_status"
	fi
	if printf '%s\n' '# smartdns-managed-dnsmasq-begin' >> "$smartdns_tmp_file"; then :; else
		smartdns_status=$?
		rm -f "$smartdns_tmp_file"
		return "$smartdns_status"
	fi
	for smartdns_directive do
		case "$smartdns_directive" in
			''|*'
'*) rm -f "$smartdns_tmp_file"; return 2 ;;
		esac
		if printf '%s\n' "$smartdns_directive" >> "$smartdns_tmp_file"; then :; else
			smartdns_status=$?
			rm -f "$smartdns_tmp_file"
			return "$smartdns_status"
		fi
	done
	if printf '%s\n' '# smartdns-managed-dnsmasq-end' >> "$smartdns_tmp_file"; then :; else
		smartdns_status=$?
		rm -f "$smartdns_tmp_file"
		return "$smartdns_status"
	fi
	if mv -f "$smartdns_tmp_file" "$smartdns_config_file"; then
		return 0
	else
		smartdns_status=$?
		rm -f "$smartdns_tmp_file"
		return "$smartdns_status"
	fi
}

smartdns_dnsmasq_remove_exact_directives()
{
	[ "$#" -ge 2 ] && [ "$#" -le 4 ] || return 2
	smartdns_config_file="$1"
	smartdns_expected_one="$2"
	smartdns_expected_two="${3-}"
	smartdns_expected_three="${4-}"
	[ -n "$smartdns_config_file" ] && [ -n "$smartdns_expected_one" ] || return 2
	if [ ! -e "$smartdns_config_file" ] && [ ! -L "$smartdns_config_file" ]; then return 0; fi
	[ -f "$smartdns_config_file" ] && [ ! -L "$smartdns_config_file" ] &&
	[ -r "$smartdns_config_file" ] && [ -w "$smartdns_config_file" ] || return 1
	if smartdns_scan_result=$(awk -v one="$smartdns_expected_one" -v two="$smartdns_expected_two" -v three="$smartdns_expected_three" '
		{ line=$0; sub(/^[[:space:]]*/, "", line); sub(/[[:space:]]*$/, "", line)
		  if (line==one || (two!="" && line==two) || (three!="" && line==three)) found=1 }
		END { print found ? 1 : 0 }
	' "$smartdns_config_file"); then :; else
		smartdns_status=$?
		return "$smartdns_status"
	fi
	case "$smartdns_scan_result" in
		0) return 0 ;;
		1) ;;
		*) return 1 ;;
	esac
	smartdns_tmp_file=$(mktemp "${smartdns_config_file}.tmp.XXXXXX") || return 1
	if cp -p "$smartdns_config_file" "$smartdns_tmp_file"; then :; else
		smartdns_status=$?; rm -f "$smartdns_tmp_file"; return "$smartdns_status"
	fi
	if awk -v one="$smartdns_expected_one" -v two="$smartdns_expected_two" -v three="$smartdns_expected_three" '
		{ line=$0; sub(/^[[:space:]]*/, "", line); sub(/[[:space:]]*$/, "", line)
		  if (line!=one && (two=="" || line!=two) && (three=="" || line!=three)) print $0 }
	' "$smartdns_config_file" > "$smartdns_tmp_file"; then :; else
		smartdns_status=$?; rm -f "$smartdns_tmp_file"; return "$smartdns_status"
	fi
	if cmp -s "$smartdns_config_file" "$smartdns_tmp_file"; then
		rm -f "$smartdns_tmp_file"; return 0
	else
		smartdns_status=$?
		if [ "$smartdns_status" -ne 1 ]; then rm -f "$smartdns_tmp_file"; return "$smartdns_status"; fi
	fi
	if mv -f "$smartdns_tmp_file" "$smartdns_config_file"; then return 0; else
		smartdns_status=$?; rm -f "$smartdns_tmp_file"; return "$smartdns_status"
	fi
}

smartdns_dnsmasq_remove_managed_block()
{
	[ "$#" -eq 1 ] || return 2
	smartdns_config_file="$1"
	if [ ! -e "$smartdns_config_file" ] && [ ! -L "$smartdns_config_file" ]; then return 0; fi
	[ -f "$smartdns_config_file" ] && [ ! -L "$smartdns_config_file" ] &&
	[ -r "$smartdns_config_file" ] && [ -w "$smartdns_config_file" ] || return 1
	smartdns_block_status=$(awk '
		$0=="# smartdns-managed-dnsmasq-begin" {
			if (inside || seen_begin) bad=1
			inside=1; seen_begin=1
			next
		}
		$0=="# smartdns-managed-dnsmasq-end" {
			if (!inside || seen_end) bad=1
			inside=0; seen_end=1
			next
		}
		END {
			if (bad || inside || (seen_begin != seen_end)) print "invalid"
			else if (!seen_begin) print "absent"
			else print "present"
		}
	' "$smartdns_config_file") || {
		smartdns_status=$?
		return "$smartdns_status"
	}
	case "$smartdns_block_status" in
		absent) return 0 ;;
		present) ;;
		invalid) return 2 ;;
		*) return 1 ;;
	esac
	smartdns_tmp_file=$(mktemp "${smartdns_config_file}.tmp.XXXXXX") || return 1
	if cp -p "$smartdns_config_file" "$smartdns_tmp_file"; then :; else
		smartdns_status=$?; rm -f "$smartdns_tmp_file"; return "$smartdns_status"
	fi
	if awk '
		$0=="# smartdns-managed-dnsmasq-begin" { inside=1; next }
		$0=="# smartdns-managed-dnsmasq-end" { inside=0; next }
		!inside { print $0 }
	' "$smartdns_config_file" > "$smartdns_tmp_file"; then :; else
		smartdns_status=$?; rm -f "$smartdns_tmp_file"; return "$smartdns_status"
	fi
	if cmp -s "$smartdns_config_file" "$smartdns_tmp_file"; then
		rm -f "$smartdns_tmp_file"; return 0
	else
		smartdns_status=$?
		if [ "$smartdns_status" -ne 1 ]; then rm -f "$smartdns_tmp_file"; return "$smartdns_status"; fi
	fi
	if mv -f "$smartdns_tmp_file" "$smartdns_config_file"; then return 0; else
		smartdns_status=$?; rm -f "$smartdns_tmp_file"; return "$smartdns_status"
	fi
}

smartdns_dnsmasq_remove_saved_directives()
{
	[ "$#" -eq 4 ] || return 2
	smartdns_saved_redirect="$2"
	smartdns_saved_port=$(smartdns_dnsmasq_normalize_port "$3") || return 1
	case "$smartdns_saved_redirect" in ''|0) smartdns_saved_redirect=0 ;; 1|2) ;; *) return 1 ;; esac
	case "$4" in
		managed-v1) smartdns_dnsmasq_remove_managed_block "$1"; return $? ;;
		''|legacy) ;;
		*) return 1 ;;
	esac
	set -- "$1"
	if [ "$smartdns_saved_redirect" = 1 ]; then
		set -- "$@" no-resolv "server=127.0.0.1#$smartdns_saved_port"
	fi
	if [ "$smartdns_saved_port" = 53 ]; then set -- "$@" port=0; fi
	[ "$#" -gt 1 ] || return 0
	smartdns_dnsmasq_remove_exact_directives "$@"
}
