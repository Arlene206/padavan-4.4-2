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
	# stable global address so privacy-address churn cannot make DDNS fail
	# merely because an interface has more than one global IPv6 address.
	ip_output=$(ip -6 addr show dev "$wan6_ifname" 2>/dev/null) || return 1
	wan6_address=$(printf '%s\n' "$ip_output" | awk '
		/^[[:space:]]*inet6[[:space:]]/ {
			address = $2
			sub(/\/.*/, "", address)
			global = 0
			secondary = 0
			unusable = 0
			for (i = 3; i <= NF; i++) {
				if ($i == "scope" && $(i + 1) == "global")
					global = 1
				if ($i == "secondary")
					secondary = 1
				if ($i == "tentative" || $i == "deprecated")
					unusable = 1
			}
			if (global && !unusable) {
				if (secondary) {
					secondary_address = address
					secondary_count++
				} else {
					stable_address = address
					stable_count++
				}
			}
		}
		END {
			if (stable_count == 1) {
				print stable_address
			} else if (stable_count == 0 && secondary_count == 1) {
				print secondary_address
			} else {
				exit 1
		}
	') || return 1
	aliddns_ipv6_public_valid "$wan6_address" || return 1
	printf '%s\n' "$wan6_address"
}

if [ "$IPv6" = "1" ] ; then
	arIpAddress=$(arIpAddress6)
else
	arIpAddress=$(arIpAddress)
fi
