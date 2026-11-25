#!/bin/sh

# This helper is an RC-internal continuation: the caller owns the shared VPN
# lifecycle barrier and has already stopped PPPD with a checked postcondition.
if [ "$#" -ne 1 ] || [ "$1" != "--rc-lifecycle-locked" ]; then
	logger -t "MWAN" "refusing direct launch; use the RC WAN PPP lifecycle path"
	exit 1
fi

umask 077
LC_ALL=C
export LC_ALL
MWAN_STATUS=0

mwan_log() { logger -t MWAN "$*" >/dev/null 2>&1 || :; }
mwan_fail() {
	mwan_rc="$1"
	shift
	[ "$mwan_rc" -ne 0 ] || return 0
	[ "$MWAN_STATUS" -ne 0 ] || MWAN_STATUS="$mwan_rc"
	mwan_log "operation failed (status=$mwan_rc): $*"
	return 0
}
mwan_die() {
	mwan_rc="$1"
	shift
	mwan_fail "$mwan_rc" "$*"
	exit "$MWAN_STATUS"
}
mwan_uint_normalize() {
	mwan_value="$1"
	mwan_maxlen="$2"
	case "$mwan_value" in ''|*[!0-9]*) return 1;; esac
	[ "${#mwan_value}" -le "$mwan_maxlen" ] || return 1
	while [ "${mwan_value#0}" != "$mwan_value" ]; do
		mwan_value="${mwan_value#0}"
	done
	[ -n "$mwan_value" ] || mwan_value=0
	printf '%s' "$mwan_value"
}
mwan_mtu_normalize() {
	mwan_value="$1"
	case "$mwan_value" in ''|*[!0-9]*) printf '1492'; return 0;; esac
	[ "${#mwan_value}" -le 4 ] || { printf '1492'; return 0; }
	while [ "${mwan_value#0}" != "$mwan_value" ]; do
		mwan_value="${mwan_value#0}"
	done
	[ -n "$mwan_value" ] || mwan_value=0
	if [ "$mwan_value" -lt 1000 ] || [ "$mwan_value" -gt 1492 ]; then
		mwan_value=1492
	fi
	printf '%s' "$mwan_value"
}
# Match ppp_options_write_quoted(): single-quote the value and escape both
# backslashes and apostrophes. Reject line/control characters and overflow.
mwan_pppd_quote() {
	case "$1" in *'
'*) return 1;; esac
	mwan_quoted=$(printf '%s' "$1" | awk '
		BEGIN { ORS = "" }
		{
			for (i = 1; i <= length($0); ++i) {
				ch = substr($0, i, 1)
				if (ch ~ /[[:cntrl:]]/) exit 2
				if (ch == sprintf("%c", 39) || ch == "\\") printf "\\"
				printf "%s", ch
			}
		}
	') || return 1
	[ "${#mwan_quoted}" -le 254 ] || return 1
	printf "'%s'" "$mwan_quoted"
}

for mwan_command in nvram ip ifconfig cat awk chmod mv rm; do
	command -v "$mwan_command" >/dev/null 2>&1 ||
		mwan_die 127 "required command not found: $mwan_command"
done

if sync="$(nvram get pppoesync_enable)"; then :; else
	mwan_rc=$?; mwan_die "$mwan_rc" "cannot read pppoesync_enable"
fi
if PPP_NUM="$(nvram get pppoe_num)"; then :; else
	mwan_rc=$?; mwan_die "$mwan_rc" "cannot read pppoe_num"
fi
if vlanen="$(nvram get vlan_filter)"; then :; else
	mwan_rc=$?; mwan_die "$mwan_rc" "cannot read vlan_filter"
fi
if vlanid="$(nvram get vlan_vid_cpu)"; then :; else
	mwan_rc=$?; mwan_die "$mwan_rc" "cannot read vlan_vid_cpu"
fi
if userid="$(nvram get wan_pppoe_username)"; then :; else
	mwan_rc=$?; mwan_die "$mwan_rc" "cannot read wan_pppoe_username"
fi
if userpw="$(nvram get wan_pppoe_passwd)"; then :; else
	mwan_rc=$?; mwan_die "$mwan_rc" "cannot read wan_pppoe_passwd"
fi
if raw_mtu="$(nvram get wan_pppoe_mtu)"; then :; else
	mwan_rc=$?; mwan_die "$mwan_rc" "cannot read wan_pppoe_mtu"
fi
if raw_mru="$(nvram get wan_pppoe_mru)"; then :; else
	mwan_rc=$?; mwan_die "$mwan_rc" "cannot read wan_pppoe_mru"
fi

case "$sync" in ''|0) sync=0;; 1) ;; *) mwan_die 2 "invalid pppoesync_enable value";; esac
case "$vlanen" in ''|0) vlanen=0;; 1) ;; *) mwan_die 2 "invalid vlan_filter value";; esac
if PPP_COUNT="$(mwan_uint_normalize "$PPP_NUM" 3)"; then :; else
	mwan_die 2 "invalid pppoe_num value"
fi
if [ "$PPP_COUNT" -lt 1 ] || [ "$PPP_COUNT" -gt 255 ]; then
	mwan_die 2 "pppoe_num is outside 1..255"
fi

if [ "$vlanen" = 1 ]; then
	if vlanid="$(mwan_uint_normalize "$vlanid" 4)"; then :; else
		mwan_die 2 "invalid vlan_vid_cpu value"
	fi
	if [ "$vlanid" -lt 1 ] || [ "$vlanid" -gt 4094 ]; then
		mwan_die 2 "vlan_vid_cpu is outside 1..4094"
	fi
	ethname="eth3.$vlanid"
else
	ethname="eth3"
fi

if quoted_user="$(mwan_pppd_quote "$userid")"; then :; else
	mwan_die 2 "refusing unsafe or oversized PPP username"
fi
if quoted_pass="$(mwan_pppd_quote "$userpw")"; then :; else
	mwan_die 2 "refusing unsafe or oversized PPP password"
fi
usermtu="$(mwan_mtu_normalize "$raw_mtu")"
usermru="$(mwan_mtu_normalize "$raw_mru")"

# RC creates and validates this private directory before invoking this helper.
if [ ! -d /tmp/ppp ] || [ -L /tmp/ppp ]; then
	mwan_die 1 "private PPP options directory is unavailable"
fi

# Validate the parent address before creating interfaces. The generated
# macvlan suffix sequence must never reuse the parent interface MAC.
if base_mac="$(cat "/sys/class/net/$ethname/address")"; then :; else
	mwan_rc=$?
	mwan_die "$mwan_rc" "read MAC address for $ethname"
fi
case "$base_mac" in
	[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]) ;;
	*) mwan_die 2 "invalid base MAC address for $ethname" ;;
esac
base_mac_suffix=$((0x${base_mac##*:}))
mac_suffix_value=16

i=1
while [ "$i" -le "$PPP_COUNT" ]; do
	macvlan="macvlan$i"
	count=$((i - 1))
	ip link del "$macvlan" >/dev/null 2>&1 || :
	if ip link add link "$ethname" "$macvlan" type macvlan; then :; else
		mwan_rc=$?
		mwan_fail "$mwan_rc" "ip link add $macvlan"
		i=$((i + 1))
		continue
	fi
	mac_prefix="${base_mac%:*}"
	mac_suffix_value=$(((mac_suffix_value + 1) % 256))
	if [ "$mac_suffix_value" -eq "$base_mac_suffix" ]; then
		mac_suffix_value=$(((mac_suffix_value + 1) % 256))
	fi
	mac_suffix=$(printf '%02X' "$mac_suffix_value") || {
		mwan_rc=$?
		mwan_fail "$mwan_rc" "cannot calculate MAC suffix for $macvlan"
		i=$((i + 1))
		continue
	}
	if ifconfig "$macvlan" hw ether "$mac_prefix:$mac_suffix"; then :; else
		mwan_rc=$?
		mwan_fail "$mwan_rc" "set hardware address for $macvlan"
		i=$((i + 1))
		continue
	fi
	if ifconfig "$macvlan" up; then :; else
		mwan_rc=$?
		mwan_fail "$mwan_rc" "bring up $macvlan"
		i=$((i + 1))
		continue
	fi

	options_file="/tmp/ppp/options.wan$count"
	options_tmp="/tmp/ppp/.options.wan$count.$$"
	if [ -L "$options_file" ]; then
		mwan_fail 1 "refusing symlink PPP options file $options_file"
		i=$((i + 1))
		continue
	fi
	# mv treats an existing directory as a container, not a file to replace.
	# Reject every pre-existing non-regular path before staging or launching PPPD.
	if [ -e "$options_file" ] && [ ! -f "$options_file" ]; then
		mwan_fail 1 "refusing non-regular PPP options path $options_file"
		i=$((i + 1))
		continue
	fi
	if ( set -C; : > "$options_tmp" ) 2>/dev/null; then :; else
		mwan_rc=$?
		mwan_fail "$mwan_rc" "cannot create temporary PPP options file for $macvlan"
		i=$((i + 1))
		continue
	fi
	if (
		if [ "$sync" = 1 ]; then
			printf 'syncppp %s\n' "$PPP_COUNT" || exit 1
		fi
		printf 'user %s\n' "$quoted_user" || exit 1
		printf 'password %s\n' "$quoted_pass" || exit 1
		printf '+ipv6\nrefuse-eap\nplugin rp-pppoe.so\n' || exit 1
		printf 'nic-%s\n' "$macvlan" || exit 1
		printf 'persist\nmaxfail 0\nholdoff 10\n' || exit 1
		printf 'ipcp-accept-remote\nipcp-accept-local\nnoipdefault\n' || exit 1
		printf 'mtu %s\nmru %s\n' "$usermtu" "$usermru" || exit 1
		printf 'usepeerdns\ndefault-asyncmap\nnopcomp\nnoaccomp\nnovj\nnobsdcomp\nnodeflate\nnomppe\nnomppc\n' || exit 1
		printf 'lcp-echo-interval 20\nlcp-echo-failure 6\nminunit 0\n' || exit 1
		printf 'linkname wan%s\nktune\n' "$count" || exit 1
	) > "$options_tmp"; then :; else
		mwan_rc=$?
		rm -f "$options_tmp"
		mwan_fail "$mwan_rc" "write PPP options for $macvlan"
		i=$((i + 1))
		continue
	fi
	if chmod 600 "$options_tmp"; then :; else
		mwan_rc=$?
		rm -f "$options_tmp"
		mwan_fail "$mwan_rc" "protect PPP options for $macvlan"
		i=$((i + 1))
		continue
	fi
	if mv -f "$options_tmp" "$options_file"; then :; else
		mwan_rc=$?
		rm -f "$options_tmp"
		mwan_fail "$mwan_rc" "publish PPP options for $macvlan"
		i=$((i + 1))
		continue
	fi
	if /usr/sbin/pppd file "$options_file"; then :; else
		mwan_rc=$?
		mwan_fail "$mwan_rc" "launch pppd for $macvlan"
	fi
	i=$((i + 1))
done

exit "$MWAN_STATUS"
