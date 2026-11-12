#!/bin/sh
# Fetch exactly one public IPv4 address over verified HTTPS.
if [ -z "$ALIDDNS_CA_BUNDLE" ]; then
	ALIDDNS_CA_BUNDLE="/etc/ssl/certs/ca-certificates.crt"
fi

interface="$1"
if [ "$#" -gt 1 ]; then
	echo "usage: $0 [interface]" >&2
	exit 2
fi
case "$interface" in
	*[!A-Za-z0-9_.:-]*|-*)
		echo "invalid network interface name" >&2
		exit 2
		;;
esac

if [ ! -r "$ALIDDNS_CA_BUNDLE" ]; then
	logger -t "【AliDDNS动态域名】" "CA 证书文件不可读：$ALIDDNS_CA_BUNDLE" 2>/dev/null || :
	exit 1
fi

set -- --fail --silent --show-error --ipv4 \
	--connect-timeout 5 --max-time 15 --proto '=https' \
	--cacert "$ALIDDNS_CA_BUNDLE" --write-out '\n%{http_code}'
if [ -n "$interface" ]; then
	set -- "$@" --interface "$interface"
fi

if response=$(curl "$@" "https://api.ipify.org"); then
	:
else
	request_status=$?
	logger -t "【AliDDNS动态域名】" "HTTPS 公网 IPv4 查询失败" 2>/dev/null || :
	exit "$request_status"
fi

http_status=$(printf '%s\n' "$response" | tail -n 1)
[ "$http_status" = "200" ] || {
	logger -t "【AliDDNS动态域名】" "公网 IPv4 服务返回 HTTP $http_status" 2>/dev/null || :
	exit 1
}

ip=$(printf '%s\n' "$response" | sed '$d')
case "$ip" in
	""|*[!0-9.]*)
		logger -t "【AliDDNS动态域名】" "公网 IPv4 服务返回非 IPv4 内容" 2>/dev/null || :
		exit 1
		;;
esac
printf '%s\n' "$ip" | awk -F. '
	NR != 1 { exit 1 }
	{
		if (NF != 4)
			exit 1
		for (i = 1; i <= 4; i++) {
			if ($i !~ /^(0|[1-9][0-9]?[0-9]?)$/ || ($i + 0) > 255)
				exit 1
		}
		valid = 1
	}
	END {
		if (NR != 1 || !valid)
			exit 1
	}
' || {
	logger -t "【AliDDNS动态域名】" "公网 IPv4 服务返回无效地址" 2>/dev/null || :
	exit 1
}

printf '%s\n' "$ip"
