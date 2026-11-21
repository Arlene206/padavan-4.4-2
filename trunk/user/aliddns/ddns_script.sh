# Stock IPv4 discovery uses the shared HTTPS/CA-verifying helper.
arIpAddress () {
	pppoemwan=$(nvram get pppoemwan_enable)
	case "$pppoemwan" in
		""|0) /usr/bin/aliddns-ipv4.sh ;;
		*) /usr/bin/aliddns-ipv4.sh ppp0 ;;
	esac
}

arIpAddress6 () {
	wan6_ifname=$(nvram get wan0_ifname6) || return 1
	case "$wan6_ifname" in
		""|*[!A-Za-z0-9_.:-]*|-*) return 1 ;;
	esac
	[ "${#wan6_ifname}" -le 15 ] || return 1

	# Match RC's get_wan_addr6_host(): use the configured WAN6 interface.
	ifconfig_output=$(ifconfig "$wan6_ifname" 2>/dev/null) || return 1
	wan6_address=$(printf '%s\n' "$ifconfig_output" | awk '
		/Scope:Global/ {
			for (i = 1; i < NF; i++) {
				if ($i != "addr:")
					continue
				address = $(i + 1)
				split(address, address_parts, "/")
				address = address_parts[1]
				if (address ~ /^[23][[:xdigit:]:]+$/)
					addresses[++count] = address
			}
		}
		END {
			if (count != 1)
				exit 1
			print addresses[1]
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
