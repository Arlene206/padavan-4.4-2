#!/bin/bash

# Reconcile PPPoE multi-WAN firewall and policy routes. A missing pppN device
# is normal during WAN reconvergence, so command failures are tracked explicitly.
LOCKFILE="${MWAN_LOCKFILE:-/tmp/lock}"
MWAN_STATUS=0
MWAN_LOCK_OWNED=0

mwan_log() { logger -t MWAN "$*" >/dev/null 2>&1 || :; }
mwan_fail() {
	local rc="$1"; shift
	[ "$rc" -ne 0 ] || return 0
	[ "$MWAN_STATUS" -ne 0 ] || MWAN_STATUS="$rc"
	mwan_log "operation failed (status=$rc): $*"
	return 0
}
mwan_run() { "$@"; local rc=$?; mwan_fail "$rc" "$*"; return 0; }
mwan_die() { local rc="$1"; shift; mwan_fail "$rc" "$*"; exit "$MWAN_STATUS"; }
mwan_exit() {
	local rc=$?
	trap - EXIT
	if [ "$MWAN_LOCK_OWNED" -eq 1 ] && ! rm -f "$LOCKFILE"; then
		mwan_log "failed to remove lock file: $LOCKFILE"
		[ "$rc" -ne 0 ] || rc=1
	fi
	exit "$rc"
}
trap 'mwan_exit' EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

mwan_uint_valid() { case "$1" in ''|*[!0-9]*) return 1;; *) return 0;; esac; }
mwan_uint_max_valid() {
	local value="$1" max="$2"
	mwan_uint_valid "$value" && [ "${#value}" -le 3 ] && [ "$((10#$value))" -le "$max" ]
}
mwan_ipv4_valid() {
	local address="$1" octet
	local -a octets
	[[ "$address" =~ ^[0-9]{1,3}(\.[0-9]{1,3}){3}$ ]] || return 1
	IFS=. read -r -a octets <<< "$address"
	[ "${#octets[@]}" -eq 4 ] || return 1
	for octet in "${octets[@]}"; do [ "$octet" -le 255 ] 2>/dev/null || return 1; done
}
mwan_ipv4_from_ifconfig() {
	printf '%s\n' "$1" | awk '
		/inet addr:/ { sub(/^.*inet addr:/, ""); sub(/[[:space:]].*/, ""); print; exit }
		$1 == "inet" && $2 ~ /^addr:/ { sub(/^addr:/, "", $2); print $2; exit }
	'
}

# Parse iptables-save output into argv; never evaluate its shell metacharacters.
mwan_parse_rule() {
	local line="$1" char quote="" token="" escaped=0 started=0 i
	MWAN_RULE_ARGS=()
	for ((i=0; i<${#line}; i++)); do
		char="${line:i:1}"
		if [ "$escaped" -eq 1 ]; then token+="$char"; escaped=0; started=1; continue; fi
		case "$quote" in
			"'") if [ "$char" = "'" ]; then quote=""; else token+="$char"; fi ;;
			'"') case "$char" in '"') quote="";; \\) escaped=1;; *) token+="$char";; esac ;;
			"") case "$char" in
				"'"|'"') quote="$char"; started=1;;
				\\) escaped=1; started=1;;
				' '|$'\t') if [ "$started" -eq 1 ]; then MWAN_RULE_ARGS+=("$token"); token=""; started=0; fi;;
				*) token+="$char"; started=1;;
			esac ;;
		esac
	done
	[ "$escaped" -eq 0 ] && [ -z "$quote" ] || return 2
	[ "$started" -eq 0 ] || MWAN_RULE_ARGS+=("$token")
	[ "${#MWAN_RULE_ARGS[@]}" -ge 2 ] && [ "${MWAN_RULE_ARGS[0]}" = -A ]
}
mwan_flush_iptables() {
	local table="$1" tag="$2" saved rule rc
	if saved=$(iptables-save -t "$table"); then :; else rc=$?; mwan_fail "$rc" "iptables-save -t $table"; return 0; fi
	while IFS= read -r rule || [ -n "$rule" ]; do
		case "$rule" in -A\ *) ;; *) continue;; esac
		case "$rule" in *"$tag"*) ;; *) continue;; esac
		if ! mwan_parse_rule "$rule"; then mwan_fail 2 "cannot safely parse $table iptables-save rule"; continue; fi
		iptables -t "$table" -D "${MWAN_RULE_ARGS[@]:1}"
		rc=$?; mwan_fail "$rc" "iptables -t $table rule deletion"
	done <<< "$saved"
}

# Preserve the shared /tmp/lock convention but create it atomically and only
# remove a lock owned by this invocation.
while :; do
	if ( set -C; : > "$LOCKFILE" ) 2>/dev/null; then MWAN_LOCK_OWNED=1; break; fi
	if [ -f "$LOCKFILE" ] && [ ! -L "$LOCKFILE" ]; then
		sleep 1 || mwan_die 1 "sleep failed waiting for $LOCKFILE"
	else
		mwan_die 1 "cannot acquire lock file: $LOCKFILE"
	fi
done

for cmd in nvram ifconfig iptables iptables-save ip awk; do
	command -v "$cmd" >/dev/null 2>&1 || mwan_die 127 "required command not found: $cmd"
done

mwan_log "flush the rules"
if PPP_NUM=$(nvram get pppoe_num); then :; else mwan_die "$?" "cannot read pppoe_num"; fi
mwan_uint_max_valid "$PPP_NUM" 255 || mwan_die 2 "invalid pppoe_num: $PPP_NUM"
PPP_COUNT=$((10#$PPP_NUM))
if bridge=$(ifconfig br0); then lanip=$(mwan_ipv4_from_ifconfig "$bridge"); else mwan_die "$?" "cannot inspect br0"; fi
mwan_ipv4_valid "$lanip" || mwan_die 2 "invalid br0 IPv4 address: $lanip"

mwan_flush_iptables mangle ctstate
mwan_flush_iptables mangle br0
mwan_flush_iptables nat ppp

last=1
route_replace=(route replace default)
for ((i=1; i<=PPP_COUNT; i++)); do
	count=$((i-1))
	if ppp_output=$(ifconfig "ppp$count"); then ppp_ip=$(mwan_ipv4_from_ifconfig "$ppp_output"); else continue; fi
	if mwan_ipv4_valid "$ppp_ip"; then
		mwan_run iptables -t mangle -A PREROUTING -s "$lanip/24" -m conntrack --ctstate NEW -m statistic --mode nth --every "$PPP_COUNT" --packet "$count" -j CONNMARK --set-mark "2$i"
		last=$i
		route_replace+=(nexthop via "$ppp_ip" dev "ppp$count" weight 1)
	fi
done

if webrule=$(nvram get pppoemwan_443); then :; else mwan_die "$?" "cannot read pppoemwan_443"; fi
mwan_uint_valid "$webrule" || mwan_die 2 "invalid pppoemwan_443 value: $webrule"
if [ "$webrule" -ne 0 ]; then
	mwan_run iptables -t mangle -A PREROUTING -p tcp --dport 443 -m conntrack --ctstate NEW -j CONNMARK --set-mark "2$last"
	mwan_run iptables -t mangle -A PREROUTING -p udp --dport 443 -m conntrack --ctstate NEW -j CONNMARK --set-mark "2$last"
fi
if rulesenable=$(nvram get pppoemwan_rules_x); then :; else mwan_die "$?" "cannot read pppoemwan_rules_x"; fi
mwan_uint_valid "$rulesenable" || mwan_die 2 "invalid pppoemwan_rules_x value: $rulesenable"
if [ "$rulesenable" -ne 0 ]; then
	if num=$(nvram get pppoemwan_staticnum_x); then :; else mwan_die "$?" "cannot read pppoemwan_staticnum_x"; fi
	mwan_uint_max_valid "$num" 255 || mwan_die 2 "invalid pppoemwan_staticnum_x value: $num"
	static_count=$((10#$num))
	for ((i=0; i<static_count; i++)); do
		ipname="pppoemwan_ip_x$i"; ifname="pppoemwan_interface_x$i"
		if ip_value=$(nvram get "$ipname"); then :; else mwan_die "$?" "cannot read $ipname"; fi
		if if_value=$(nvram get "$ifname"); then :; else mwan_die "$?" "cannot read $ifname"; fi
		mwan_ipv4_valid "$ip_value" || mwan_die 2 "invalid $ipname value: $ip_value"
		mwan_uint_max_valid "$if_value" 255 || mwan_die 2 "invalid $ifname value: $if_value"
		mwan_run iptables -t mangle -A PREROUTING -s "$ip_value/32" -m conntrack --ctstate NEW -j CONNMARK --set-mark "2$if_value"
	done
fi

for ((i=1; i<=PPP_COUNT; i++)); do
	count=$((i-1))
	if ppp_output=$(ifconfig "ppp$count"); then ppp_ip=$(mwan_ipv4_from_ifconfig "$ppp_output"); else continue; fi
	if mwan_ipv4_valid "$ppp_ip"; then
		mwan_run iptables -t mangle -A PREROUTING -i br0 -m connmark --mark "2$i" -j MARK --set-mark "2$i"
		mwan_run iptables -t nat -A POSTROUTING -o "ppp$count" -s "$lanip/24" -j FULLCONENAT
		mwan_run iptables -t nat -A PREROUTING -i "ppp$count" -j FULLCONENAT
		mwan_run ip rule add fwmark "2$i" table "2${i}0"
		mwan_run ip route add default via "$ppp_ip" dev "ppp$count" table "2${i}0"
	fi
done
mwan_run ip "${route_replace[@]}"
mwan_run ip route flush cache
exit "$MWAN_STATUS"
