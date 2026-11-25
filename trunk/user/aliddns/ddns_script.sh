# Stock IPv4 discovery uses the shared HTTPS/CA-verifying helper.
arIpAddress () {
	pppoemwan=$(nvram get_checked pppoemwan_enable) || return $?
	case "$pppoemwan" in
		""|0) /usr/bin/aliddns-ipv4.sh ;;
		*) /usr/bin/aliddns-ipv4.sh ppp0 ;;
	esac
}

arIpAddress6 () {
	wan6_ifname=$(nvram get_checked wan0_ifname6) || return $?
	case "$wan6_ifname" in
		""|*[!A-Za-z0-9_.:-]*|-*) return 1 ;;
	esac
	[ "${#wan6_ifname}" -le 15 ] || return 1

	# BusyBox ip marks temporary IPv6 addresses as "secondary". Prefer the
	# only usable public stable address, falling back to one public temporary
	# address only when no public stable address is available.
	ip_output=$(ip -6 addr show dev "$wan6_ifname" 2>/dev/null) || return 1
	wan6_candidates=$(printf '%s\n' "$ip_output" | awk '
		/^[[:space:]]*inet6[[:space:]]/ {
			address = $2
			sub(/\/.*/, "", address)
			is_global = 0
			secondary = 0
			unusable = 0
			for (i = 3; i <= NF; i++) {
				if ($i == "scope" && $(i + 1) == "global")
					is_global = 1
				if ($i == "secondary")
					secondary = 1
				if ($i == "tentative" || $i == "deprecated")
					unusable = 1
			}
			if (is_global && !unusable)
				printf "%s %s\n", secondary ? "T" : "S", address
		}
	') || return 1

	# Filter non-GUA global-scope addresses before deciding whether the
	# stable/temporary set is ambiguous. ULA and reserved space must not mask
	# an otherwise usable public address on the same WAN interface.
	wan6_stable_count=0
	wan6_temporary_count=0
	while IFS=' ' read -r wan6_candidate_type wan6_candidate; do
		[ -n "$wan6_candidate" ] || continue
		case "$wan6_candidate_type" in
			S|T) ;;
			*) return 1 ;;
		esac
		aliddns_ipv6_public_valid "$wan6_candidate" || continue
		case "$wan6_candidate_type" in
			S)
				wan6_stable_address=$wan6_candidate
				wan6_stable_count=$((wan6_stable_count + 1))
				;;
			T)
				wan6_temporary_address=$wan6_candidate
				wan6_temporary_count=$((wan6_temporary_count + 1))
				;;
		esac
	done <<EOF_WAN6_CANDIDATES
$wan6_candidates
EOF_WAN6_CANDIDATES

	if [ "$wan6_stable_count" -eq 1 ]; then
		wan6_address=$wan6_stable_address
	elif [ "$wan6_stable_count" -eq 0 ] &&
	     [ "$wan6_temporary_count" -eq 1 ]; then
		wan6_address=$wan6_temporary_address
	else
		return 1
	fi
	aliddns_ipv6_public_valid "$wan6_address" || return 1
	printf '%s\n' "$wan6_address"
}

if [ "$IPv6" = "1" ] ; then
	arIpAddress=$(arIpAddress6)
else
	arIpAddress=$(arIpAddress)
fi
