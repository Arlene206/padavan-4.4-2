#!/bin/sh

WG_IFACE=wg0
WG_KEY_FILE=
WG_INTERFACE_CREATED=0
WG_FIREWALL_ADDED=0
WG_FIREWALL_RULE="-t nat -A POSTROUTING -o wg0 -j MASQUERADE"
WG_FIREWALL_CHECK="-t nat -C POSTROUTING -o wg0 -j MASQUERADE"
WG_FIREWALL_DELETE="-t nat -D POSTROUTING -o wg0 -j MASQUERADE"

log()
{
	logger -t WIREGUARD "$*" 2>/dev/null || :
}

cleanup_private_key()
{
	if [ -n "$WG_KEY_FILE" ]; then
		if rm -f "$WG_KEY_FILE"; then
			WG_KEY_FILE=
		else
			log "failed to remove temporary private-key file"
			return 1
		fi
	fi
	return 0
}

trap 'cleanup_private_key' 0
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

require_commands()
{
	for command_name in "$@"; do
		if ! command -v "$command_name" >/dev/null 2>&1; then
			log "required command is unavailable: $command_name"
			return 127
		fi
	done
	return 0
}

required_setting()
{
	setting_value=$(nvram get "$1") || return $?
	case "$setting_value" in
		''|*[[:space:]]*)
			return 2
			;;
	esac
	printf '%s\n' "$setting_value"
}

rollback_start()
{
	if [ "$WG_FIREWALL_ADDED" -eq 1 ]; then
		if iptables -t nat -D POSTROUTING -o wg0 -j MASQUERADE; then
			WG_FIREWALL_ADDED=0
		else
			rollback_status=$?
			log "failed to roll back WireGuard masquerade rule (status=$rollback_status)"
		fi
	fi

	if [ "$WG_INTERFACE_CREATED" -eq 1 ]; then
		if ip link del dev "$WG_IFACE"; then
			WG_INTERFACE_CREATED=0
		else
			rollback_status=$?
			log "failed to remove newly-created WireGuard interface (status=$rollback_status)"
		fi
	fi
}

fail_start()
{
	failure_status=$1
	failure_reason=$2
	log "WireGuard startup failed: $failure_reason (status=$failure_status)"
	rollback_start
	cleanup_private_key || :
	return "$failure_status"
}

start_wg()
{
	localip=
	privatekey=
	peerkey=
	peerip=
	start_status=0
	interface_status=0
	firewall_status=0
	WG_INTERFACE_CREATED=0
	WG_FIREWALL_ADDED=0

	require_commands nvram ip wg iptables mktemp chmod rm || return $?

	if localip=$(required_setting wireguard_localip); then
		:
	else
		start_status=$?
		fail_start "$start_status" "invalid local interface address"
		return $?
	fi
	if privatekey=$(required_setting wireguard_localkey); then
		:
	else
		start_status=$?
		fail_start "$start_status" "private key is unavailable"
		return $?
	fi
	if peerkey=$(required_setting wireguard_peerkey); then
		:
	else
		start_status=$?
		fail_start "$start_status" "peer public key is unavailable"
		return $?
	fi
	if peerip=$(required_setting wireguard_peerip); then
		:
	else
		start_status=$?
		fail_start "$start_status" "peer endpoint is unavailable"
		return $?
	fi

	# A private key must never be exposed through a predictable, world-readable
	# /tmp/privatekey file. Create a unique 0600 file and remove it on every path.
	umask 077
	if WG_KEY_FILE=$(mktemp /tmp/wireguard-privatekey.XXXXXX); then
		:
	else
		start_status=$?
		fail_start "$start_status" "cannot create private-key staging file"
		return $?
	fi
	if chmod 600 "$WG_KEY_FILE"; then
		:
	else
		start_status=$?
		fail_start "$start_status" "cannot secure private-key staging file"
		return $?
	fi
	if printf '%s\n' "$privatekey" > "$WG_KEY_FILE"; then
		:
	else
		start_status=$?
		fail_start "$start_status" "cannot write private-key staging file"
		return $?
	fi

	if ip link show dev "$WG_IFACE" >/dev/null 2>&1; then
		WG_INTERFACE_CREATED=0
	else
		interface_status=$?
		if [ "$interface_status" -ne 1 ]; then
			fail_start "$interface_status" "cannot inspect interface state"
			return $?
		fi
		if ip link add dev "$WG_IFACE" type wireguard; then
			WG_INTERFACE_CREATED=1
		else
			start_status=$?
			fail_start "$start_status" "cannot create interface"
			return $?
		fi
	fi

	if ip link set dev "$WG_IFACE" mtu 1420; then
		:
	else
		start_status=$?
		fail_start "$start_status" "cannot set interface MTU"
		return $?
	fi
	if ip address replace "$localip" dev "$WG_IFACE"; then
		:
	else
		start_status=$?
		fail_start "$start_status" "cannot configure interface address"
		return $?
	fi
	if wg set "$WG_IFACE" private-key "$WG_KEY_FILE"; then
		:
	else
		start_status=$?
		fail_start "$start_status" "cannot configure private key"
		return $?
	fi
	if wg set "$WG_IFACE" peer "$peerkey" persistent-keepalive 25 allowed-ips 0.0.0.0/0 endpoint "$peerip"; then
		:
	else
		start_status=$?
		fail_start "$start_status" "cannot configure peer"
		return $?
	fi
	if cleanup_private_key; then
		:
	else
		start_status=$?
		fail_start "$start_status" "cannot remove private-key staging file"
		return $?
	fi

	if iptables $WG_FIREWALL_CHECK >/dev/null 2>&1; then
		:
	else
		firewall_status=$?
		if [ "$firewall_status" -ne 1 ]; then
			fail_start "$firewall_status" "cannot inspect masquerade rule"
			return $?
		fi
		if iptables $WG_FIREWALL_RULE; then
			WG_FIREWALL_ADDED=1
		else
			start_status=$?
			fail_start "$start_status" "cannot install masquerade rule"
			return $?
		fi
	fi

	if ip link set dev "$WG_IFACE" up; then
		log "WireGuard interface is running"
		return 0
	else
		start_status=$?
		fail_start "$start_status" "cannot bring interface up"
		return $?
	fi
}

remove_firewall_rules()
{
	removed_count=0
	while :; do
		if iptables $WG_FIREWALL_CHECK >/dev/null 2>&1; then
			:
		else
			firewall_check_status=$?
			if [ "$firewall_check_status" -eq 1 ]; then
				return 0
			fi
			return "$firewall_check_status"
		fi

		# Older releases appended the same MASQUERADE rule on every start.
		# Remove all exact matches, but bound the loop against unexpected
		# firewall behaviour or concurrent rule insertion.
		if [ "$removed_count" -ge 64 ]; then
			log "WireGuard masquerade cleanup exceeded its 64-rule safety bound"
			return 1
		fi
		if iptables $WG_FIREWALL_DELETE; then
			removed_count=$((removed_count + 1))
		else
			return $?
		fi
	done
}

stop_wg()
{
	stop_status=0
	interface_status=0
	firewall_status=0
	require_commands ip iptables || return $?

	if remove_firewall_rules; then
		:
	else
		firewall_status=$?
		stop_status=$firewall_status
		log "WireGuard masquerade cleanup incomplete (status=$firewall_status)"
	fi

	if ip link show dev "$WG_IFACE" >/dev/null 2>&1; then
		if ip link del dev "$WG_IFACE"; then
			:
		else
			interface_status=$?
			[ "$stop_status" -ne 0 ] || stop_status=$interface_status
			log "failed to delete WireGuard interface (status=$interface_status)"
		fi
	else
		interface_status=$?
		if [ "$interface_status" -ne 1 ]; then
			[ "$stop_status" -ne 0 ] || stop_status=$interface_status
			log "cannot inspect WireGuard interface (status=$interface_status)"
		fi
	fi

	if [ "$stop_status" -eq 0 ]; then
		log "WireGuard interface stopped"
	else
		log "WireGuard stop incomplete (status=$stop_status)"
	fi
	return "$stop_status"
}

case "$1" in
	start)
		start_wg
		;;
	stop)
		stop_wg
		;;
	*)
		echo "Usage: $0 { start | stop }" >&2
		exit 2
		;;
esac
