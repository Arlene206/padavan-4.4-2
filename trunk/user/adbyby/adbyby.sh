#!/bin/sh
#2019/08/30 by bkye
ipt_n="iptables -t nat"
#adbyby_dir="/tmp/adbyby"
PROG_PATH="/tmp/adbyby"
DATA_PATH="$PROG_PATH/data"
ADBYBY_CA_BUNDLE="/etc/ssl/certs/ca-certificates.crt"
ADBYBY_ANTI_AD_FILE="/etc/storage/dnsmasq-adbyby.d/anti-ad-for-dnsmasq.conf"
WAN_FILE="/etc/storage/dnsmasq-adbyby.d/03-adbyby-ipset.conf"
ADBYBY_HOST_LIST_FILE="/etc/storage/adbyby_host.sh"
ADBYBY_HOSTS_CONFIG_FILE="/etc/storage/dnsmasq/dnsmasq.conf"
ADBYBY_HOST_SOURCE_MAX_BYTES=4194304
ADBYBY_HOST_TOTAL_MAX_BYTES=16777216
ADBYBY_HOST_MAX_CONFIG_BYTES=314571
ADBYBY_HOST_MAX_SOURCES=16

# Read mutable settings only after the lifecycle lock is held.
adbyby_refresh_nvram_state()
{
	adbyby_enable=$(nvram get adbyby_enable) || return 1
	adbyby_ip_x=$(nvram get adbyby_ip_x) || return 1
	adbyby_rules_x=$(nvram get adbyby_rules_x) || return 1
	adbyby_set=$(nvram get adbyby_set) || return 1
	http_username=$(nvram get http_username) || return 1
	adbyby_update=$(nvram get adbyby_update) || return 1
	adbyby_update_hour=$(nvram get adbyby_update_hour) || return 1
	adbyby_update_min=$(nvram get adbyby_update_min) || return 1
	wan_mode="$adbyby_set"
}
adbyby_restart_dhcpd()
{
	# The helper may detach dnsmasq; it must not retain the lifecycle lock FD.
	/sbin/restart_dhcpd 9>&-
}
adbyby_start()
{
	addscripts
	if [ ! -f "$PROG_PATH/adbyby" ]; then
	logger -t "adbyby" "adbyby程序文件不存在，正在解压..."
	tar -xzvf "/etc_ro/adbyby.tar.gz" -C "/tmp"
	logger -t "adbyby" "成功解压至：$PROG_PATH"
	fi
	#if [ $abp_mode -eq 1 ]; then
	#/tmp/adbyby/adblock.sh &
	#fi
	#/tmp/adbyby/adupdate.sh &
	if ! add_rules; then
		logger -t "adbyby" "规则准备失败，拒绝启动Adbyby。"
		return 1
	fi
	"$PROG_PATH/adbyby" 9>&- >/dev/null 2>&1 &
	add_dns
	iptables-save | grep ADBYBY >/dev/null || \
	add_rule
	hosts_ads
	adbyby_restart_dhcpd
	add_cron
	logger -t "adbyby" "Adbyby启动完成。"
}

adbyby_close()
{
	del_rule
	del_cron
	del_dns
	killall -q adbyby
	if [ $mem_mode -eq 1 ]; then
	echo "stop mem mode"
	fi
	kill -9 $(ps | grep admem.sh | grep -v grep | awk '{print $1}') >/dev/null 2>&1 
	adbyby_restart_dhcpd
	logger -t "adbyby" "Adbyby已关闭。"

}

adbyby_valid_md5()
{
	case "$1" in
		''|*[!0123456789abcdefABCDEF]*) return 1 ;;
	esac
	[ "${#1}" -eq 32 ]
}

adbyby_md5_file()
{
	[ -f "$1" ] && [ ! -L "$1" ] && [ -s "$1" ] || return 1
	adbyby_md5_output=$(md5sum "$1" 2>/dev/null) || return 1
	adbyby_md5_value=${adbyby_md5_output%% *}
	adbyby_valid_md5 "$adbyby_md5_value" || return 1
	printf '%s\n' "$adbyby_md5_value" | tr 'A-F' 'a-f'
}

adbyby_manifest_digest()
{
	adbyby_manifest_key="$1"
	adbyby_manifest_file="$2"
	sed -n "s/.*\"$adbyby_manifest_key\"[[:space:]]*:[[:space:]]*\"\\([[:xdigit:]]\\{32\\}\\)\".*/\\1/p" "$adbyby_manifest_file" |
		tr 'A-F' 'a-f'
}

adbyby_valid_https_url()
{
	adbyby_url="$1"
	[ -n "$adbyby_url" ] && [ "${#adbyby_url}" -le 2048 ] || return 1
	adbyby_url_lf='
'
	adbyby_url_cr=$(printf '\r')
	case "$adbyby_url" in
		https://*) ;;
		*) return 1 ;;
	esac
	case "$adbyby_url" in
		*' '*|*"$adbyby_url_lf"*|*"$adbyby_url_cr"*) return 1 ;;
	esac
	printf '%s\n' "$adbyby_url" | LC_ALL=C grep -Eq \
		'^https://([A-Za-z0-9]([A-Za-z0-9.-]*[A-Za-z0-9])?|\[[0-9A-Fa-f:]+\])(:[0-9]{1,5})?([/?#][!-~]*)?$' || return 1
	adbyby_url_authority="${adbyby_url#https://}"
	adbyby_url_authority="${adbyby_url_authority%%[/?#]*}"
	case "$adbyby_url_authority" in
		*..*|*.-*|*-.|.*|*.) return 1 ;;
	esac
}

adbyby_download_https_200()
{
	adbyby_download_url="$1"
	adbyby_download_file="$2"
	adbyby_download_max_bytes="${3:-16777216}"
	adbyby_valid_https_url "$adbyby_download_url" || return 1
	case "$adbyby_download_max_bytes" in
		''|*[!0123456789]*) return 1 ;;
	esac
	[ "$adbyby_download_max_bytes" -gt 0 ] &&
	[ "$adbyby_download_max_bytes" -le 16777216 ] || return 1
	[ -r "$ADBYBY_CA_BUNDLE" ] && [ -s "$ADBYBY_CA_BUNDLE" ] || return 1
	adbyby_download_status=$(curl --cacert "$ADBYBY_CA_BUNDLE" --proto '=https' \
		--fail --silent --show-error --connect-timeout 5 --max-time 30 \
		--retry 3 --limit-rate 512k --max-filesize "$adbyby_download_max_bytes" \
		--output "$adbyby_download_file" --write-out '%{http_code}' \
		"$adbyby_download_url") || return 1
	[ "$adbyby_download_status" = 200 ] || return 1
	adbyby_download_size=$(wc -c < "$adbyby_download_file" | awk '{print $1}') || return 1
	case "$adbyby_download_size" in ''|*[!0123456789]*) return 1 ;; esac
	[ "$adbyby_download_size" -le "$adbyby_download_max_bytes" ] &&
	[ -s "$adbyby_download_file" ]
}

adbyby_rules_cache_is_usable()
{
	[ -d "$PROG_PATH" ] && [ ! -L "$PROG_PATH" ] &&
	[ -d "$DATA_PATH" ] && [ ! -L "$DATA_PATH" ] &&
	[ -f "$DATA_PATH/lazy.txt" ] && [ ! -L "$DATA_PATH/lazy.txt" ] && [ -s "$DATA_PATH/lazy.txt" ] &&
	[ -f "$DATA_PATH/video.txt" ] && [ ! -L "$DATA_PATH/video.txt" ] && [ -s "$DATA_PATH/video.txt" ]
}

# Stage and verify both upstream lists privately. Each published file uses an
# atomic same-filesystem rename; a failed second rename rolls back the first.
adbyby_refresh_managed_rules()
(
	umask 077
	[ -d "$PROG_PATH" ] && [ ! -L "$PROG_PATH" ] &&
	[ -d "$DATA_PATH" ] && [ ! -L "$DATA_PATH" ] || return 1
	for adbyby_target in "$DATA_PATH/lazy.txt" "$DATA_PATH/video.txt"; do
		if [ -e "$adbyby_target" ] || [ -L "$adbyby_target" ]; then
			[ -f "$adbyby_target" ] && [ ! -L "$adbyby_target" ] || return 1
		fi
	done

	adbyby_stage_dir=$(mktemp -d /tmp/adbyby-rules.XXXXXX) || return 1
	adbyby_keep_stage=0
	trap 'if [ "$adbyby_keep_stage" -eq 0 ]; then rm -rf "$adbyby_stage_dir"; fi' 0

	adbyby_lazy_local=$(adbyby_md5_file "$DATA_PATH/lazy.txt" 2>/dev/null) || adbyby_lazy_local=""
	adbyby_video_local=$(adbyby_md5_file "$DATA_PATH/video.txt" 2>/dev/null) || adbyby_video_local=""
	adbyby_cache_ok=0
	if adbyby_rules_cache_is_usable &&
	   adbyby_valid_md5 "$adbyby_lazy_local" && adbyby_valid_md5 "$adbyby_video_local"; then
		adbyby_cache_ok=1
	fi

	if ! adbyby_download_https_200 \
		"https://adbyby.coding.net/p/xwhyc-rules/d/xwhyc-rules/git/raw/master/md5.json" \
		"$adbyby_stage_dir/md5.json"; then
		if [ "$adbyby_cache_ok" -eq 1 ]; then
			logger -t "adbyby" "规则清单下载或HTTPS校验失败，保留现有规则。"
			return 0
		fi
		logger -t "adbyby" "规则清单不可用且本地规则不完整，拒绝继续。"
		return 1
	fi

	adbyby_lazy_online=$(adbyby_manifest_digest lazy "$adbyby_stage_dir/md5.json")
	adbyby_video_online=$(adbyby_manifest_digest video "$adbyby_stage_dir/md5.json")
	if ! adbyby_valid_md5 "$adbyby_lazy_online" ||
	   ! adbyby_valid_md5 "$adbyby_video_online"; then
		if [ "$adbyby_cache_ok" -eq 1 ]; then
			logger -t "adbyby" "规则清单格式或摘要无效，保留现有规则。"
			return 0
		fi
		logger -t "adbyby" "规则清单格式无效且本地规则不完整，拒绝继续。"
		return 1
	fi

	if [ "$adbyby_lazy_online" = "$adbyby_lazy_local" ] &&
	   [ "$adbyby_video_online" = "$adbyby_video_local" ]; then
		logger -t "adbyby" "规则摘要一致，无需下载。"
		return 0
	fi

	if ! adbyby_download_https_200 \
		"https://adbyby.coding.net/p/xwhyc-rules/d/xwhyc-rules/git/raw/master/lazy.txt" \
		"$adbyby_stage_dir/lazy.next" ||
	   ! adbyby_download_https_200 \
		"https://adbyby.coding.net/p/xwhyc-rules/d/xwhyc-rules/git/raw/master/video.txt" \
		"$adbyby_stage_dir/video.next"; then
		if [ "$adbyby_cache_ok" -eq 1 ]; then
			logger -t "adbyby" "规则文件下载失败，保留现有规则。"
			return 0
		fi
		logger -t "adbyby" "规则文件下载失败且本地规则不完整，拒绝继续。"
		return 1
	fi

	adbyby_lazy_downloaded=$(adbyby_md5_file "$adbyby_stage_dir/lazy.next" 2>/dev/null) || adbyby_lazy_downloaded=""
	adbyby_video_downloaded=$(adbyby_md5_file "$adbyby_stage_dir/video.next" 2>/dev/null) || adbyby_video_downloaded=""
	if [ "$adbyby_lazy_downloaded" != "$adbyby_lazy_online" ] ||
	   [ "$adbyby_video_downloaded" != "$adbyby_video_online" ]; then
		if [ "$adbyby_cache_ok" -eq 1 ]; then
			logger -t "adbyby" "规则下载摘要不匹配，拒绝发布并保留旧规则。"
			return 0
		fi
		logger -t "adbyby" "规则下载摘要不匹配且无可用旧规则，拒绝继续。"
		return 1
	fi

	adbyby_had_lazy=0
	if [ -e "$DATA_PATH/lazy.txt" ]; then
		cp -p "$DATA_PATH/lazy.txt" "$adbyby_stage_dir/lazy.previous" || return 1
		adbyby_had_lazy=1
	fi
	if ! mv -f "$adbyby_stage_dir/lazy.next" "$DATA_PATH/lazy.txt"; then
		logger -t "adbyby" "lazy规则原子发布失败，保留旧规则。"
		return 1
	fi
	if ! mv -f "$adbyby_stage_dir/video.next" "$DATA_PATH/video.txt"; then
		if [ "$adbyby_had_lazy" -eq 1 ]; then
			if ! mv -f "$adbyby_stage_dir/lazy.previous" "$DATA_PATH/lazy.txt"; then
				adbyby_keep_stage=1
				logger -t "adbyby" "video规则发布及lazy回滚均失败，保留恢复文件：$adbyby_stage_dir"
				return 1
			fi
		else
			rm -f "$DATA_PATH/lazy.txt"
		fi
		logger -t "adbyby" "video规则原子发布失败，已回滚lazy规则。"
		return 1
	fi

	if ! printf '%s\n' "$(date '+%Y-%m-%d %H:%M:%S')" > "$PROG_PATH/adbyby.updated"; then
		logger -t "adbyby" "规则已发布，但更新时间文件写入失败。"
	fi
	logger -t "adbyby" "规则文件已通过HTTPS和MD5校验并发布。"
	return 0
)

adbyby_user_rules_cache_is_usable()
{
	[ -d "$PROG_PATH" ] && [ ! -L "$PROG_PATH" ] &&
	[ -d "$DATA_PATH" ] && [ ! -L "$DATA_PATH" ] &&
	[ -f "$DATA_PATH/user.txt" ] && [ ! -L "$DATA_PATH/user.txt" ] &&
	[ -s "$DATA_PATH/user.txt" ]
}

adbyby_prepare_user_rules()
{
	(
		umask 077
		adbyby_prepare_user_rules_transaction
	)
}

adbyby_prepare_user_rules_transaction()
{
	[ -d "$PROG_PATH" ] && [ ! -L "$PROG_PATH" ] &&
	[ -d "$DATA_PATH" ] && [ ! -L "$DATA_PATH" ] || return 1
	if [ -e "$DATA_PATH/user.txt" ] || [ -L "$DATA_PATH/user.txt" ]; then
		[ -f "$DATA_PATH/user.txt" ] && [ ! -L "$DATA_PATH/user.txt" ] || return 1
	fi

	adbyby_user_stage_dir=$(mktemp -d /tmp/adbyby-user-rules.XXXXXX) || return 1
	trap 'rm -rf "$adbyby_user_stage_dir"' 0
	trap 'exit 1' HUP INT TERM
	: > "$adbyby_user_stage_dir/user.next" || return 1

	if [ "$adbyby_rules_x" = "1" ]; then
		adbyby_user_rules_count=$(nvram get adbybyrules_staticnum_x 2>/dev/null) || adbyby_user_rules_count=""
		case "$adbyby_user_rules_count" in
			'') adbyby_user_rules_count=0 ;;
			*[!0123456789]*)
				logger -t "adbyby" "第三方规则数量无效，拒绝覆盖现有规则。"
				if adbyby_user_rules_cache_is_usable; then return 0; else return 1; fi
				;;
		esac
		if [ "$adbyby_user_rules_count" -gt 32 ]; then
			logger -t "adbyby" "第三方规则数量超过安全上限，拒绝覆盖现有规则。"
			if adbyby_user_rules_cache_is_usable; then return 0; else return 1; fi
		fi

		adbyby_user_rule_index=0
		while [ "$adbyby_user_rule_index" -lt "$adbyby_user_rules_count" ]; do
			adbyby_user_rule_url=$(nvram get "adbybyrules_x$adbyby_user_rule_index" 2>/dev/null) || adbyby_user_rule_url=""
			adbyby_user_rule_enabled=$(nvram get "adbybyrules_road_x$adbyby_user_rule_index" 2>/dev/null) || adbyby_user_rule_enabled=0
			case "$adbyby_user_rule_enabled" in
				''|0) ;;
				*)
					adbyby_user_rule_raw="$adbyby_user_stage_dir/rule-$adbyby_user_rule_index.raw"
					adbyby_user_rule_filtered="$adbyby_user_stage_dir/rule-$adbyby_user_rule_index.filtered"
					if ! adbyby_download_https_200 "$adbyby_user_rule_url" "$adbyby_user_rule_raw" 1048576; then
						logger -t "adbyby" "第三方规则下载或校验失败，保留上一次完整规则。"
						if adbyby_user_rules_cache_is_usable; then return 0; else return 1; fi
					fi
					grep -v '^!' "$adbyby_user_rule_raw" | grep -E '^(@@\\||\\||[[:alnum:]])' | sort -u > "$adbyby_user_rule_filtered"
					if [ ! -s "$adbyby_user_rule_filtered" ]; then
						logger -t "adbyby" "第三方规则内容为空或格式不受支持，保留上一次完整规则。"
						if adbyby_user_rules_cache_is_usable; then return 0; else return 1; fi
					fi
					cat "$adbyby_user_rule_filtered" >> "$adbyby_user_stage_dir/user.next" || return 1
					;;
			esac
			adbyby_user_rule_index=$((adbyby_user_rule_index + 1))
		done
	fi

	if [ -f "$PROG_PATH/rules.txt" ] && [ ! -L "$PROG_PATH/rules.txt" ]; then
		grep -v '^!' "$PROG_PATH/rules.txt" | grep -v '^$' >> "$adbyby_user_stage_dir/user.next"
	fi
	chmod 0644 "$adbyby_user_stage_dir/user.next" || return 1
	if ! mv -f "$adbyby_user_stage_dir/user.next" "$DATA_PATH/user.txt"; then
		logger -t "adbyby" "第三方规则发布失败，保留上一次完整规则。"
		return 1
	fi
	rm -f "$DATA_PATH/user.bin"
	return 0
}

add_rules()
{
	logger -t "adbyby" "正在检查规则是否需要更新!"
	rm -f /tmp/adbyby/data/*.bak

	if ! adbyby_refresh_managed_rules; then
		logger -t "adbyby" "规则校验或发布失败，停止规则处理。"
		return 1
	fi
	logger -t "adbyby" "Adbyby规则更新检查完成"
	nvram set adbyby_ltime=$(head -1 "$DATA_PATH/lazy.txt" | awk -F' ' '{print $3,$4}')
	nvram set adbyby_vtime=$(head -1 "$DATA_PATH/video.txt" | awk -F' ' '{print $3,$4}')

	#nvram set adbyby_rules=`grep -v '^!' /tmp/adbyby/data/rules.txt | wc -l`

	#nvram set adbyby_utime=`cat /tmp/adbyby.updated 2>/dev/null`
	grep -v '^!' /etc/storage/adbyby_rules.sh | grep -v "^$" > $PROG_PATH/rules.txt
	grep -v '^!' /etc/storage/adbyby_blockip.sh | grep -v "^$" > $PROG_PATH/blockip.conf
	grep -v '^!' /etc/storage/adbyby_adblack.sh | grep -v "^$" > $PROG_PATH/adblack.conf
	grep -v '^!' /etc/storage/adbyby_adesc.sh | grep -v "^$" > $PROG_PATH/adesc.conf
	grep -v '^!' /etc/storage/adbyby_adhost.sh | grep -v "^$" > $PROG_PATH/adhost.conf
	logger -t "adbyby" "正在处理规则..."
	if ! adbyby_prepare_user_rules; then
		logger -t "adbyby" "第三方规则无法安全更新且没有可用缓存，停止规则处理。"
		return 1
	fi
	nvram set adbyby_user="$(wc -l < "$DATA_PATH/user.txt")"
}


add_cron()
{
	if [ $adbyby_update -eq 0 ]; then
	sed -i '/adbyby/d' /etc/storage/cron/crontabs/$http_username
	cat >> /etc/storage/cron/crontabs/$http_username << EOF
$adbyby_update_min $adbyby_update_hour * * * /bin/sh /usr/bin/adbyby.sh G >/dev/null 2>&1
EOF
	logger -t "adbyby" "设置每天$adbyby_update_hour时$adbyby_update_min分，自动更新规则！"
	fi
if [ $adbyby_update -eq 1 ]; then

	sed -i '/adbyby/d' /etc/storage/cron/crontabs/$http_username
	cat >> /etc/storage/cron/crontabs/$http_username << EOF
*/$adbyby_update_min */$adbyby_update_hour * * * /bin/sh /usr/bin/adbyby.sh G >/dev/null 2>&1
EOF
	logger -t "adbyby" "设置每隔$adbyby_update_hour时$adbyby_update_min分，自动更新规则！"
	fi
	if [ $adbyby_update -eq 2 ]; then
	sed -i '/adbyby/d' /etc/storage/cron/crontabs/$http_username
	fi
}

del_cron()
{
	sed -i '/adbyby/d' /etc/storage/cron/crontabs/$http_username
}

adbyby_cleanup_blockip_stage()
{
	if [ -n "$adbyby_blockip_file" ]; then
		rm -f "$adbyby_blockip_file"
		adbyby_blockip_file=""
	fi
	if [ -n "$adbyby_blockip_set" ]; then
		ipset -! destroy "$adbyby_blockip_set" >/dev/null 2>&1
	fi
}

# Build a complete replacement off-line, then atomically exchange the set
# referenced by the live iptables rules. A failed parse/restore never mutates
# the current blockip inventory.
adbyby_refresh_blockip_set()
{
	adbyby_blockip_set=""
	adbyby_blockip_file=$(mktemp /tmp/adbyby-blockip.XXXXXX) || return 1
	adbyby_blockip_set=$(basename "$adbyby_blockip_file" | sed 's/^adbyby-blockip\./adbyby_/') || {
		rm -f "$adbyby_blockip_file"
		return 1
	}
	[ -n "$adbyby_blockip_set" ] || {
		rm -f "$adbyby_blockip_file"
		return 1
	}
	if ! chmod 600 "$adbyby_blockip_file" ||
	   ! awk -v set="$adbyby_blockip_set" '!/^$/&&!/^#/{printf("add %s %s\n",set,$0)}' "$PROG_PATH/blockip.conf" > "$adbyby_blockip_file"; then
		adbyby_cleanup_blockip_stage
		return 1
	fi
	if ! ipset -! destroy "$adbyby_blockip_set" >/dev/null 2>&1 ||
	   ! ipset create "$adbyby_blockip_set" hash:net family inet hashsize 1024 maxelem 65536; then
		adbyby_cleanup_blockip_stage
		return 1
	fi
	if ! ipset -! restore < "$adbyby_blockip_file" 2>/dev/null; then
		adbyby_cleanup_blockip_stage
		return 1
	fi
	if ! rm -f "$adbyby_blockip_file"; then
		adbyby_cleanup_blockip_stage
		return 1
	fi
	adbyby_blockip_file=""

	if ipset list blockip >/dev/null 2>&1; then
		# ipset swap preserves existing kernel references to the stable name.
		if ! ipset swap "$adbyby_blockip_set" blockip; then
			adbyby_cleanup_blockip_stage
			return 1
		fi
		# The staging name now owns the old, unreferenced set.
		if ! ipset destroy "$adbyby_blockip_set"; then
			logger -t "adbyby" "blockip已刷新，但旧集合清理失败：$adbyby_blockip_set"
		fi
	else
		# Cold boot: no stable set exists yet, so publish the complete staged set.
		if ! ipset rename "$adbyby_blockip_set" blockip; then
			adbyby_cleanup_blockip_stage
			return 1
		fi
	fi
	return 0
}

ip_rule()
{
	ipset -! -N adbyby_esc hash:ip || return 1
	$ipt_n -A ADBYBY -m set --match-set adbyby_esc dst -j RETURN || return 1
	num=$(nvram get adbybyip_staticnum_x)
	if [ "$adbyby_ip_x" -eq 1 ]; then
		if [ "$num" -ne 0 ]; then
			logger -t "adbyby" "设置内网IP过滤控制"
			for i in $(seq 1 "$num")
			do
				j=$(expr "$i" - 1)
				ip=$(nvram get adbybyip_ip_x$j)
				mode=$(nvram get adbybyip_ip_road_x$j)
				case "$mode" in
				0)
					$ipt_n -A ADBYBY -s "$ip" -j RETURN || return 1
					logger -t "adbyby" "忽略$ip走AD过滤。"
					;;
				1)
					$ipt_n -A ADBYBY -s "$ip" -p tcp -j REDIRECT --to-ports 8118 || return 1
					$ipt_n -A ADBYBY -s "$ip" -j RETURN || return 1
					logger -t "adbyby" "设置$ip走全局过滤。"
					;;
				2)
					ipset -! -N adbyby_wan hash:ip || return 1
					$ipt_n -A ADBYBY -m set --match-set adbyby_wan dst -s "$ip" -p tcp -j REDIRECT --to-ports 8118 || return 1
					awk '!/^$/&&!/^#/{printf("ipset=/%s/'"'"'\"adbyby_wan\"'"'"'\\n",$0)}' "$PROG_PATH/adhost.conf" > "$WAN_FILE" || return 1
					logger -t "adbyby" "设置$ip走Plus+过滤。"
					;;
				*)
					return 1
					;;
				esac
			done
		fi
	fi

	case "$wan_mode" in
	0)
		$ipt_n -A ADBYBY -p tcp -j REDIRECT --to-ports 8118 || return 1
		;;
	1)
		ipset -! -N adbyby_wan hash:ip || return 1
		$ipt_n -A ADBYBY -m set --match-set adbyby_wan dst -p tcp -j REDIRECT --to-ports 8118 || return 1
		;;
	2)
		$ipt_n -A ADBYBY -d 0.0.0.0/24 -j RETURN || return 1
		;;
	*)
		return 1
		;;
	esac

	# Refresh the stable set through an off-line replacement so removed
	# entries do not survive a firewall rebuild.
	adbyby_refresh_blockip_set || return 1
	iptables -I FORWARD -m set --match-set blockip dst -j DROP || return 1
	iptables -I OUTPUT -m set --match-set blockip dst -j DROP || return 1
}

add_dns()
{
	mkdir -p /etc/storage/dnsmasq-adbyby.d || return 1
	[ -d /etc/storage/dnsmasq-adbyby.d ] && [ ! -L /etc/storage/dnsmasq-adbyby.d ] || {
		logger -t "adbyby" "Adbyby dnsmasq配置目录不安全。"
		return 1
	}
	mkdir -p /tmp/dnsmasq.d
	if ! anti_ad; then
		# The optional anti-AD list must not suppress unrelated Adbyby DNS rules.
		logger -t "adbyby" "anti_AD不可用，继续处理其他dnsmasq规则。"
	fi
	block_ios=`nvram get block_ios`
	block_douyin=`nvram get block_douyin`
	awk '!/^$/&&!/^#/{printf("ipset=/%s/'"adbyby_esc"'\n",$0)}' $PROG_PATH/adesc.conf > /etc/storage/dnsmasq-adbyby.d/06-dnsmasq.esc
	awk '!/^$/&&!/^#/{printf("address=/%s/'"0.0.0.0"'\n",$0)}' $PROG_PATH/adblack.conf > /etc/storage/dnsmasq-adbyby.d/07-dnsmasq.black
	[ $block_ios -eq 1 ] && echo 'address=/mesu.apple.com/0.0.0.0' >> /etc/storage/dnsmasq-adbyby.d/07-dnsmasq.black
	if [ $block_douyin -eq 1 ]; then
  cat <<-EOF >/etc/storage/dnsmasq-adbyby.d/08-dnsmasq.douyin
address=/api.amemv.com/0.0.0.0
address=/.snssdk.com/0.0.0.0
address=/.douyin.com/0.0.0.0
		EOF
	fi
	sed -i '/dnsmasq-adbyby/d' /etc/storage/dnsmasq/dnsmasq.conf
	cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
conf-dir=/etc/storage/dnsmasq-adbyby.d
EOF
	if [ $wan_mode -eq 1 ]; then
	awk '!/^$/&&!/^#/{printf("ipset=/%s/'"adbyby_wan"'\n",$0)}' $PROG_PATH/adhost.conf > $WAN_FILE
	fi
	if ls /etc/storage/dnsmasq-adbyby.d/* >/dev/null 2>&1; then
	mkdir -p /tmp/dnsmasq.d
	#if [ $abp_mode -eq 1 ]; then
	#cp $PROG_PATH/dnsmasq.adblock /etc/storage/dnsmasq-adbyby.d/04-dnsmasq.adblock
	#sed -i '/youku.com/d' $PROG_PATH/dnsmasq.ads
	#cp $PROG_PATH/dnsmasq.ads /etc/storage/dnsmasq-adbyby.d/05-dnsmasq.ads
	#fi
	fi
	#sed -i '/mesu.apple.com/d' /etc/dnsmasq.conf && [ $block_ios -eq 1 ] && echo 'address=/mesu.apple.com/0.0.0.0' >> /etc/dnsmasq.conf
	#处理hosts文件
}

del_dns()
{
	sed -i '/dnsmasq-adbyby/d' /etc/storage/dnsmasq/dnsmasq.conf
	#sed -i '/tvhosts/d' /etc/storage/dnsmasq/dnsmasq.conf
	if ! adbyby_remove_generated_hosts_config; then
		logger -t "adbyby" "移除Adbyby hosts配置失败。"
	fi
	rm -f /tmp/dnsmasq.d/dnsmasq-adbyby.conf
	if ! adbyby_cleanup_dnsmasq_ad_dir "/etc/storage/dnsmasq-adbyby.d"; then
		logger -t "adbyby" "清理Adbyby dnsmasq规则失败。"
	fi
	rm -f /tmp/adbyby_host.conf
}


save_adbyby_rule_snapshot()
(
	# /home/root is a 0700 tmpfs directory created by dev_init.sh.
	save_dir="${1:-/home/root}"
	[ -d "$save_dir" ] && [ ! -L "$save_dir" ] || return 1
	umask 077
	raw_file="$save_dir/.adbyby-save.raw.$$"
	next_file="$save_dir/.adbyby-save.next.$$"
	final_file="$save_dir/.adbyby.save"
	( set -C; : > "$raw_file" ) 2>/dev/null || return 1
	if ! iptables-save > "$raw_file"; then
		rm -f "$raw_file" "$next_file"; return 1
	fi
	if ! grep -E "ADBYBY|--match-set blockip|^\\*|^COMMIT" "$raw_file" >/dev/null; then
		rm -f "$raw_file" "$next_file"; return 1
	fi
	( set -C; : > "$next_file" ) 2>/dev/null || { rm -f "$raw_file"; return 1; }
	if ! grep -E "ADBYBY|--match-set blockip|^\\*|^COMMIT" "$raw_file" | sed -e "s/^-A \\(FORWARD\\|OUTPUT\\|PREROUTING\\)/-I \\1 1/" > "$next_file"; then
		rm -f "$raw_file" "$next_file"; return 1
	fi
	if [ ! -s "$next_file" ] || ! chmod 600 "$next_file" ||
	   ! mv -f "$next_file" "$final_file"; then
		rm -f "$raw_file" "$next_file"; return 1
	fi
	rm -f "$raw_file"
)

adbyby_remove_rule_snapshot()
(
	# A deleted add-on must not leave a stale privileged iptables-restore input.
	# Do not follow a path that redirects cleanup outside the private directory.
	save_dir="${1:-/home/root}"
	[ ! -L "$save_dir" ] || return 1
	if [ ! -e "$save_dir" ]; then
		return 0
	fi
	[ -d "$save_dir" ] || return 1
	rm -f "$save_dir/.adbyby.save" "$save_dir"/.adbyby-save.raw.* "$save_dir"/.adbyby-save.next.*
)

adbyby_abort_rule_setup()
{
	reason="$1"
	# Best-effort cleanup: PREROUTING is added only after every prerequisite succeeds.
	del_rule >/dev/null 2>&1
	logger -t "adbyby" "防火墙规则安装失败（$reason），已尝试清理部分规则。"
}

add_rule()
{
	if ! $ipt_n -N ADBYBY; then
		logger -t "adbyby" "防火墙规则安装失败：无法创建 ADBYBY 链。"
		return 1
	fi
	if ! $ipt_n -A ADBYBY -d 0.0.0.0/8 -j RETURN ||
	   ! $ipt_n -A ADBYBY -d 10.0.0.0/8 -j RETURN ||
	   ! $ipt_n -A ADBYBY -d 127.0.0.0/8 -j RETURN ||
	   ! $ipt_n -A ADBYBY -d 169.254.0.0/16 -j RETURN ||
	   ! $ipt_n -A ADBYBY -d 172.16.0.0/12 -j RETURN ||
	   ! $ipt_n -A ADBYBY -d 192.168.0.0/16 -j RETURN ||
	   ! $ipt_n -A ADBYBY -d 224.0.0.0/4 -j RETURN ||
	   ! $ipt_n -A ADBYBY -d 240.0.0.0/4 -j RETURN; then
		adbyby_abort_rule_setup "NAT chain base rules"
		return 1
	fi
	if ! ip_rule; then
		adbyby_abort_rule_setup "ipset or filter rules"
		return 1
	fi
	logger -t "adbyby" "添加8118透明代理端口。"
	if ! $ipt_n -I PREROUTING -p tcp --dport 80 -j ADBYBY; then
		adbyby_abort_rule_setup "PREROUTING redirect"
		return 1
	fi
	if save_adbyby_rule_snapshot; then
		logger -t "adbyby" "保存adbyby防火墙规则成功！"
	else
		# Runtime rules are already active; retain existing behavior but report
		# that the startup snapshot could not be refreshed.
		logger -t "adbyby" "保存adbyby防火墙规则失败！可能会造成重启后过滤广告失效，需要手动关闭再打开ADBYBY！"
	fi
	return 0
}

del_rule()
{
	if ! adbyby_remove_rule_snapshot; then
		logger -t "adbyby" "清理防火墙规则快照失败；后续防火墙重建可能仍尝试恢复旧规则。"
	fi
	$ipt_n -D PREROUTING -p tcp --dport 80 -j ADBYBY 2>/dev/null
	$ipt_n -F ADBYBY 2>/dev/null
	$ipt_n -X ADBYBY 2>/dev/null
	iptables -D FORWARD -m set --match-set blockip dst -j DROP 2>/dev/null
	iptables -D OUTPUT -m set --match-set blockip dst -j DROP 2>/dev/null
	ipset -F adbyby_esc 2>/dev/null
	ipset -X adbyby_esc 2>/dev/null
	ipset -F adbyby_wan 2>/dev/null
	ipset -X adbyby_wan 2>/dev/null
	ipset -F blockip 2>/dev/null
	ipset -X blockip 2>/dev/null
	logger -t "adbyby" "已关闭全部8118透明代理端口。"
}

reload_rule()
{
	config_load adbyby
	config_foreach get_config adbyby
	del_rule
	iptables-save | grep ADBYBY >/dev/null || \
	add_rule
}

adbyby_uprules()
{
	adbyby_close
	addscripts
	if [ ! -f "$PROG_PATH/adbyby" ]; then
	logger -t "adbyby" "adbyby程序文件不存在，正在解压..."
	tar -xzvf "/etc_ro/adbyby.tar.gz" -C "/tmp"
	logger -t "adbyby" "成功解压至：$PROG_PATH"
	fi
	#if [ $abp_mode -eq 1 ]; then
	#/tmp/adbyby/adblock.sh &
	#fi
	#/tmp/adbyby/adupdate.sh &
	if ! add_rules; then
		logger -t "adbyby" "规则准备失败，保持Adbyby关闭。"
		return 1
	fi
	"$PROG_PATH/adbyby" 9>&- >/dev/null 2>&1 &
	add_dns
	iptables-save | grep ADBYBY >/dev/null || \
	add_rule
	hosts_ads
	adbyby_restart_dhcpd
	#add_cron
}

#updateadb()
#{
#	/tmp/adbyby/adblock.sh &
#}
adbyby_validate_dnsmasq_fragment()
{
	adbyby_fragment_file="$1"
	[ -f "$adbyby_fragment_file" ] && [ ! -L "$adbyby_fragment_file" ] &&
	[ -s "$adbyby_fragment_file" ] || return 1
	adbyby_fragment_bytes=$(wc -c < "$adbyby_fragment_file" | awk '{print $1}') || return 1
	case "$adbyby_fragment_bytes" in ''|*[!0123456789]*) return 1 ;; esac
	[ "$adbyby_fragment_bytes" -le 16777216 ] || return 1
	awk '
		length($0) > 512 { invalid = 1; next }
		/^#/ { next }
		/^$/ { next }
		{
			if ($0 !~ /^address=\/[A-Za-z0-9_-]+(\.[A-Za-z0-9_-]+)*\/$/) {
				invalid = 1
				next
			}
			domain = substr($0, 10, length($0) - 10)
			if (length(domain) > 253 || domain ~ /\.\./ ||
			    domain ~ /(^|\.)-/ || domain ~ /-(\.|$)/ || domain ~ /\.$/) {
				invalid = 1
				next
			}
			label_count = split(domain, labels, /\./)
			for (i = 1; i <= label_count; i++) {
				if (length(labels[i]) > 63) {
					invalid = 1
				}
			}
			count++
		}
		END {
			if (invalid || count < 1) exit 1
			print count
		}
	' "$adbyby_fragment_file"
}

adbyby_anti_ad_cache_is_usable()
{
	adbyby_cache_file="$1"
	[ -f "$adbyby_cache_file" ] && [ ! -L "$adbyby_cache_file" ] &&
	[ -s "$adbyby_cache_file" ] || return 1
	adbyby_validate_dnsmasq_fragment "$adbyby_cache_file" >/dev/null
}

adbyby_prepare_anti_ad()
{
	(
		umask 077
		adbyby_prepare_anti_ad_transaction "$1" "${2:-$ADBYBY_ANTI_AD_FILE}"
	)
}

adbyby_prepare_anti_ad_transaction()
{
	adbyby_anti_ad_url="$1"
	adbyby_anti_ad_file="${2:-$ADBYBY_ANTI_AD_FILE}"
	adbyby_anti_ad_dir="${adbyby_anti_ad_file%/*}"
	adbyby_valid_https_url "$adbyby_anti_ad_url" || return 1
	[ -d "$adbyby_anti_ad_dir" ] && [ ! -L "$adbyby_anti_ad_dir" ] || return 1
	if [ -e "$adbyby_anti_ad_file" ] || [ -L "$adbyby_anti_ad_file" ]; then
		[ -f "$adbyby_anti_ad_file" ] && [ ! -L "$adbyby_anti_ad_file" ] || return 1
	fi
	# Stage on the same filesystem as the persistent conf-dir target.
	adbyby_anti_ad_stage_dir=$(mktemp -d "$adbyby_anti_ad_dir/.anti-ad.XXXXXX") || return 1
	trap 'rm -rf "$adbyby_anti_ad_stage_dir"' 0
	trap 'exit 1' HUP INT TERM
	adbyby_anti_ad_download="$adbyby_anti_ad_stage_dir/anti-ad.raw"
	adbyby_download_https_200 "$adbyby_anti_ad_url" "$adbyby_anti_ad_download" 16777216 || return 1
	adbyby_anti_ad_count=$(adbyby_validate_dnsmasq_fragment "$adbyby_anti_ad_download") || return 1
	chmod 0644 "$adbyby_anti_ad_download" || return 1
	[ -d "$adbyby_anti_ad_dir" ] && [ ! -L "$adbyby_anti_ad_dir" ] || return 1
	if [ -e "$adbyby_anti_ad_file" ] || [ -L "$adbyby_anti_ad_file" ]; then
		[ -f "$adbyby_anti_ad_file" ] && [ ! -L "$adbyby_anti_ad_file" ] || return 1
	fi
	mv -f "$adbyby_anti_ad_download" "$adbyby_anti_ad_file" || return 1
	printf '%s\n' "$adbyby_anti_ad_count"
}

anti_ad()
{
	adbyby_anti_ad_enabled=$(nvram get anti_ad 2>/dev/null) || adbyby_anti_ad_enabled=0
	if [ "$adbyby_anti_ad_enabled" != "1" ]; then
		nvram set anti_ad_count=0 || return 1
		if ! adbyby_remove_anti_ad_fragment "$ADBYBY_ANTI_AD_FILE"; then
			logger -t "adbyby" "anti_AD已关闭，但无法安全移除旧规则文件。"
			return 1
		fi
		return 0
	fi
	adbyby_anti_ad_url=$(nvram get anti_ad_link 2>/dev/null) || adbyby_anti_ad_url=""
	if adbyby_anti_ad_count=$(adbyby_prepare_anti_ad "$adbyby_anti_ad_url" "$ADBYBY_ANTI_AD_FILE"); then
		logger -t "adbyby" "anti_AD下载成功，已安全发布规则。"
	else
		if adbyby_anti_ad_cache_is_usable "$ADBYBY_ANTI_AD_FILE"; then
			adbyby_anti_ad_count=$(adbyby_validate_dnsmasq_fragment "$ADBYBY_ANTI_AD_FILE") || return 1
			logger -t "adbyby" "anti_AD更新失败，保留上一次有效规则。"
		else
			adbyby_anti_ad_count=0
			if ! adbyby_remove_anti_ad_fragment "$ADBYBY_ANTI_AD_FILE"; then
				logger -t "adbyby" "无法移除不可信的anti_AD规则目标。"
			fi
			logger -t "adbyby" "anti_AD下载或格式校验失败，未启用无效规则。"
		fi
	fi
	nvram set anti_ad_count="$adbyby_anti_ad_count"
}

adbyby_remove_anti_ad_fragment()
{
	adbyby_remove_file="$1"
	[ -n "$adbyby_remove_file" ] || return 1
	if [ -L "$adbyby_remove_file" ]; then
		# unlink the link itself; never follow its referent.
		rm -f "$adbyby_remove_file"
		return $?
	fi
	[ -e "$adbyby_remove_file" ] || return 0
	[ ! -d "$adbyby_remove_file" ] || return 1
	rm -f "$adbyby_remove_file"
}

adbyby_cleanup_dnsmasq_ad_dir()
{
	adbyby_cleanup_dir="$1"
	[ -n "$adbyby_cleanup_dir" ] || return 1
	[ ! -L "$adbyby_cleanup_dir" ] || return 1
	[ -d "$adbyby_cleanup_dir" ] || {
		[ ! -e "$adbyby_cleanup_dir" ] && return 0
		return 1
	}

	adbyby_cleanup_enabled=$(nvram get anti_ad 2>/dev/null) || adbyby_cleanup_enabled=0
	adbyby_keep_anti_ad=0
	if [ "$adbyby_cleanup_enabled" = "1" ] &&
	   adbyby_anti_ad_cache_is_usable "$ADBYBY_ANTI_AD_FILE"; then
		adbyby_cleanup_count=$(adbyby_validate_dnsmasq_fragment "$ADBYBY_ANTI_AD_FILE") || return 1
		nvram set anti_ad_count="$adbyby_cleanup_count" || return 1
		adbyby_keep_anti_ad=1
	else
		if ! adbyby_remove_anti_ad_fragment "$ADBYBY_ANTI_AD_FILE"; then
			logger -t "adbyby" "无法安全移除不可用的anti_AD缓存。"
			return 1
		fi
		nvram set anti_ad_count=0 || return 1
	fi

	for adbyby_cleanup_entry in "$adbyby_cleanup_dir"/*; do
		[ -e "$adbyby_cleanup_entry" ] || [ -L "$adbyby_cleanup_entry" ] || continue
		if [ "$adbyby_cleanup_entry" = "$ADBYBY_ANTI_AD_FILE" ] &&
		   [ "$adbyby_keep_anti_ad" = "1" ]; then
			continue
		fi
		rm -f "$adbyby_cleanup_entry" || return 1
	done
	# Remove abandoned private staging areas left by an unclean process exit.
	for adbyby_cleanup_stage in "$adbyby_cleanup_dir"/.anti-ad.*; do
		[ -e "$adbyby_cleanup_stage" ] || [ -L "$adbyby_cleanup_stage" ] || continue
		if [ -L "$adbyby_cleanup_stage" ]; then
			rm -f "$adbyby_cleanup_stage" || return 1
		elif [ -d "$adbyby_cleanup_stage" ]; then
			rm -rf "$adbyby_cleanup_stage" || return 1
		else
			rm -f "$adbyby_cleanup_stage" || return 1
		fi
	done
	return 0
}


adbyby_remove_generated_hosts_config()
{
	adbyby_hosts_config_file="${1:-/etc/storage/dnsmasq/dnsmasq.conf}"
	adbyby_hosts_file="${2:-$PROG_PATH/hosts}"
	[ -f "$adbyby_hosts_config_file" ] || return 0
	adbyby_hosts_entry="addn-hosts=$adbyby_hosts_file"
	adbyby_escaped_hosts_entry=$(printf '%s\n' "$adbyby_hosts_entry" | sed 's/[][\\/.^$*]/\\&/g') || return 1
	sed -i "/^$adbyby_escaped_hosts_entry\$/d" "$adbyby_hosts_config_file"
}

adbyby_validate_hosts_file()
{
	adbyby_hosts_input="$1"
	adbyby_hosts_output="$2"
	adbyby_hosts_max_bytes="$3"
	[ -f "$adbyby_hosts_input" ] && [ ! -L "$adbyby_hosts_input" ] && [ -s "$adbyby_hosts_input" ] || return 1
	case "$adbyby_hosts_max_bytes" in ''|*[!0123456789]*) return 1 ;; esac
	[ "$adbyby_hosts_max_bytes" -gt 0 ] && [ "$adbyby_hosts_max_bytes" -le 16777216 ] || return 1
	adbyby_hosts_input_bytes=$(wc -c < "$adbyby_hosts_input" | awk '{print $1}') || return 1
	case "$adbyby_hosts_input_bytes" in ''|*[!0123456789]*) return 1 ;; esac
	[ "$adbyby_hosts_input_bytes" -le "$adbyby_hosts_max_bytes" ] || return 1
	if [ -n "$adbyby_hosts_output" ]; then
		[ "$adbyby_hosts_output" != "$adbyby_hosts_input" ] || return 1
		( set -C; : > "$adbyby_hosts_output" ) 2>/dev/null || return 1
	else
		adbyby_hosts_output="/dev/null"
	fi
	awk -v output="$adbyby_hosts_output" '
		function valid_ipv4(address, octets, count, i, value) {
			if (address !~ /^[0-9][0-9.]*$/) return 0
			count = split(address, octets, /\./)
			if (count != 4) return 0
			for (i = 1; i <= 4; i++) {
				if (octets[i] !~ /^[0-9]+$/ || length(octets[i]) > 3) return 0
				if (length(octets[i]) > 1 && substr(octets[i], 1, 1) == "0") return 0
				value = octets[i] + 0
				if (value > 255) return 0
			}
			return 1
		}
		function valid_ipv6(address, parts, count, i, compressed, groups, normalized) {
			if (address !~ /^[0-9A-Fa-f:]+$/ || index(address, ":") == 0 || address ~ /:::/) return 0
			compressed = (index(address, "::") > 0)
			normalized = address
			if (compressed) {
				sub(/::/, "X", normalized)
				if (index(normalized, "::") > 0) return 0
			} else if (substr(address, 1, 1) == ":" || substr(address, length(address), 1) == ":") return 0
			count = split(address, parts, /:/)
			groups = 0
			for (i = 1; i <= count; i++) {
				if (parts[i] == "") continue
				if (length(parts[i]) > 4 || parts[i] !~ /^[0-9A-Fa-f]+$/) return 0
				groups++
			}
			if (compressed) return groups < 8
			return groups == 8
		}
		function valid_address(address) {
			if (index(address, ":") > 0) return valid_ipv6(address)
			return valid_ipv4(address)
		}
		function valid_hostname(name, labels, count, i, label) {
			if (length(name) > 254 || name !~ /^[A-Za-z0-9_.-]+$/) return 0
			if (substr(name, length(name), 1) == ".") name = substr(name, 1, length(name) - 1)
			if (name == "" || length(name) > 253) return 0
			count = split(name, labels, /\./)
			for (i = 1; i <= count; i++) {
				label = labels[i]
				if (length(label) < 1 || length(label) > 63) return 0
				if (length(label) == 1) {
					if (label !~ /^[A-Za-z0-9_]$/) return 0
				} else if (label !~ /^[A-Za-z0-9_][A-Za-z0-9_-]*[A-Za-z0-9_]$/) return 0
			}
			return 1
		}
		{
			line = $0
			sub(/\r$/, "", line)
			if (length(line) > 1024) { invalid = 1; next }
			sub(/^[ \t]+/, "", line)
			sub(/[ \t]+$/, "", line)
			if (line == "" || line ~ /^#/) next
			fields_count = split(line, fields, /[ \t]+/)
			if (fields_count < 2 || !valid_address(fields[1])) { invalid = 1; next }
			normalized = fields[1]
			names = 0
			record_invalid = 0
			for (i = 2; i <= fields_count; i++) {
				if (substr(fields[i], 1, 1) == "#") break
				if (!valid_hostname(fields[i])) { invalid = 1; record_invalid = 1; break }
				normalized = normalized " " fields[i]
				names++
			}
			if (names < 1) { invalid = 1; record_invalid = 1 }
			if (!record_invalid) { print normalized >> output; records++ }
		}
		END { close(output); if (invalid || records == 0) exit 1 }
	' "$adbyby_hosts_input"
}

adbyby_prepare_hosts_transaction()
(
	umask 077
	adbyby_hosts_destination="$PROG_PATH/hosts"
	[ -d "$PROG_PATH" ] && [ ! -L "$PROG_PATH" ] || return 1
	if [ -e "$adbyby_hosts_destination" ] || [ -L "$adbyby_hosts_destination" ]; then
		[ -f "$adbyby_hosts_destination" ] && [ ! -L "$adbyby_hosts_destination" ] || return 1
	fi
	[ -f "$ADBYBY_HOST_LIST_FILE" ] && [ ! -L "$ADBYBY_HOST_LIST_FILE" ] && [ -r "$ADBYBY_HOST_LIST_FILE" ] || return 1
	adbyby_hosts_config_bytes=$(wc -c < "$ADBYBY_HOST_LIST_FILE" | awk '{print $1}') || return 1
	case "$adbyby_hosts_config_bytes" in ''|*[!0123456789]*) return 1 ;; esac
	[ "$adbyby_hosts_config_bytes" -le "$ADBYBY_HOST_MAX_CONFIG_BYTES" ] || return 1
	adbyby_hosts_stage=$(mktemp -d "$PROG_PATH/.adbyby-hosts.XXXXXX") || return 1
	trap 'rm -rf "$adbyby_hosts_stage"' 0
	trap 'exit 1' HUP INT TERM
	adbyby_hosts_aggregate="$adbyby_hosts_stage/all.hosts"
	( set -C; : > "$adbyby_hosts_aggregate" ) 2>/dev/null || return 1
	adbyby_hosts_source_count=0
	while IFS= read -r adbyby_hosts_url || [ -n "$adbyby_hosts_url" ]; do
		case "$adbyby_hosts_url" in ''|'#'*) continue ;; esac
		adbyby_hosts_source_count=$((adbyby_hosts_source_count + 1))
		[ "$adbyby_hosts_source_count" -le "$ADBYBY_HOST_MAX_SOURCES" ] || return 1
		adbyby_hosts_source="$adbyby_hosts_stage/source.$adbyby_hosts_source_count"
		adbyby_hosts_validated="$adbyby_hosts_stage/validated.$adbyby_hosts_source_count"
		if ! adbyby_download_https_200 "$adbyby_hosts_url" "$adbyby_hosts_source" "$ADBYBY_HOST_SOURCE_MAX_BYTES"; then
			logger -t "adbyby" "Hosts列表第 $adbyby_hosts_source_count 个来源下载或HTTPS校验失败。"
			return 1
		fi
		if ! adbyby_validate_hosts_file "$adbyby_hosts_source" "$adbyby_hosts_validated" "$ADBYBY_HOST_SOURCE_MAX_BYTES"; then
			logger -t "adbyby" "Hosts列表第 $adbyby_hosts_source_count 个来源格式无效。"
			return 1
		fi
		adbyby_hosts_source_bytes=$(wc -c < "$adbyby_hosts_validated" | awk '{print $1}') || return 1
		adbyby_hosts_aggregate_bytes=$(wc -c < "$adbyby_hosts_aggregate" | awk '{print $1}') || return 1
		case "$adbyby_hosts_source_bytes:$adbyby_hosts_aggregate_bytes" in *[!0123456789:]*) return 1 ;; esac
		[ $((adbyby_hosts_source_bytes + adbyby_hosts_aggregate_bytes)) -le "$ADBYBY_HOST_TOTAL_MAX_BYTES" ] || return 1
		cat "$adbyby_hosts_validated" >> "$adbyby_hosts_aggregate" || return 1
	done < "$ADBYBY_HOST_LIST_FILE"
	[ "$adbyby_hosts_source_count" -gt 0 ] && [ -s "$adbyby_hosts_aggregate" ] || return 1
	LC_ALL=C sort "$adbyby_hosts_aggregate" > "$adbyby_hosts_stage/hosts.sorted" || return 1
	uniq "$adbyby_hosts_stage/hosts.sorted" > "$adbyby_hosts_stage/hosts.next" || return 1
	[ -s "$adbyby_hosts_stage/hosts.next" ] || return 1
	adbyby_hosts_final_bytes=$(wc -c < "$adbyby_hosts_stage/hosts.next" | awk '{print $1}') || return 1
	case "$adbyby_hosts_final_bytes" in ''|*[!0123456789]*) return 1 ;; esac
	[ "$adbyby_hosts_final_bytes" -le "$ADBYBY_HOST_TOTAL_MAX_BYTES" ] || return 1
	adbyby_validate_hosts_file "$adbyby_hosts_stage/hosts.next" "" "$ADBYBY_HOST_TOTAL_MAX_BYTES" || return 1
	if [ -e "$adbyby_hosts_destination" ] || [ -L "$adbyby_hosts_destination" ]; then
		[ -f "$adbyby_hosts_destination" ] && [ ! -L "$adbyby_hosts_destination" ] || return 1
	fi
	chmod 0644 "$adbyby_hosts_stage/hosts.next" || return 1
	mv -f "$adbyby_hosts_stage/hosts.next" "$adbyby_hosts_destination" || return 1
)

adbyby_hosts_cache_is_usable()
{
	[ -d "$PROG_PATH" ] && [ ! -L "$PROG_PATH" ] &&
		[ -f "$PROG_PATH/hosts" ] && [ ! -L "$PROG_PATH/hosts" ] &&
		[ -s "$PROG_PATH/hosts" ] &&
		adbyby_validate_hosts_file "$PROG_PATH/hosts" "" "$ADBYBY_HOST_TOTAL_MAX_BYTES"
}

hosts_ads()
{
	adbyby_hosts_enabled=$(nvram get hosts_ad) || return 1
	if [ "$adbyby_hosts_enabled" != "1" ]; then
		nvram set adbyby_hostsad=0 || return 1
		return 0
	fi
	if ! adbyby_prepare_hosts_transaction; then
		if ! adbyby_hosts_cache_is_usable; then
			logger -t "adbyby" "Hosts列表更新失败且不存在有效的上一次完整列表，拒绝发布。"
			nvram set adbyby_hostsad=0 || return 1
			if ! adbyby_remove_generated_hosts_config "$ADBYBY_HOSTS_CONFIG_FILE" "$PROG_PATH/hosts"; then
				logger -t "adbyby" "移除无效Hosts配置失败。"
				return 1
			fi
			return 1
		fi
		logger -t "adbyby" "Hosts列表更新失败，保留并继续使用上一次完整列表。"
	fi
	if ! adbyby_remove_generated_hosts_config "$ADBYBY_HOSTS_CONFIG_FILE" "$PROG_PATH/hosts"; then
		logger -t "adbyby" "移除Adbyby Hosts配置失败，未追加新的Hosts配置。"
		return 1
	fi
	if ! printf 'addn-hosts=%s\n' "$PROG_PATH/hosts" >> "$ADBYBY_HOSTS_CONFIG_FILE"; then
		logger -t "adbyby" "写入Adbyby Hosts配置失败。"
		return 1
	fi
	adbyby_hosts_count=$(wc -l < "$PROG_PATH/hosts" | awk '{print $1}') || return 1
	case "$adbyby_hosts_count" in ''|*[!0123456789]*) return 1 ;; esac
	nvram set adbyby_hostsad="$adbyby_hosts_count"
}


addscripts()
{

	adbyby_rules="/etc/storage/adbyby_rules.sh"
	if [ ! -f "$adbyby_rules" ] || [ ! -s "$adbyby_rules" ] ; then
	cat > "$adbyby_rules" <<-\EEE
!  ------------------------------ ADByby 自定义过滤语法简表---------------------------------
!  --------------  规则基于abp规则，并进行了字符替换部分的扩展-----------------------------
!  ABP规则请参考https://adblockplus.org/zh_CN/filters，下面为大致摘要
!  "!" 为行注释符，注释行以该符号起始作为一行注释语义，用于规则描述
!  "*" 为字符通配符，能够匹配0长度或任意长度的字符串，该通配符不能与正则语法混用。
!  "^" 为分隔符，可以是除了字母、数字或者 _ - . % 之外的任何字符。
!  "|" 为管线符号，来表示地址的最前端或最末端
!  "||" 为子域通配符，方便匹配主域名下的所有子域。
!  "~" 为排除标识符，通配符能过滤大多数广告，但同时存在误杀, 可以通过排除标识符修正误杀链接。
!  "##" 为元素选择器标识符，后面跟需要隐藏元素的CSS样式例如 #ad_id  .ad_class
!!  元素隐藏暂不支持全局规则和排除规则
!! 字符替换扩展
!  文本替换选择器标识符，后面跟需要替换的文本数据，格式：$s@模式字符串@替换后的文本@
!  支持通配符*和？
!  -------------------------------------------------------------------------------------------

EEE
	chmod 755 "$adbyby_rules"
	fi

	adbyby_blockip="/etc/storage/adbyby_blockip.sh"
	if [ ! -f "$adbyby_blockip" ] || [ ! -s "$adbyby_blockip" ] ; then
	cat > "$adbyby_blockip" <<-\EEE
2.2.2.2

EEE
	chmod 755 "$adbyby_blockip"
	fi

	adbyby_adblack="/etc/storage/adbyby_adblack.sh"
	if [ ! -f "$adbyby_adblack" ] || [ ! -s "$adbyby_adblack" ] ; then
	cat > "$adbyby_adblack" <<-\EEE
gvod.aiseejapp.atianqi.com
stat.pandora.xiaomi.com
upgrade.mishop.pandora.xiaomi.com
logonext.tv.kuyun.com
config.kuyun.com
mishop.pandora.xiaomi.com
dvb.pandora.xiaomi.com
api.ad.xiaomi.com
de.pandora.xiaomi.com
data.mistat.xiaomi.com
jellyfish.pandora.xiaomi.com
gallery.pandora.xiaomi.com
o2o.api.xiaomi.com
bss.pandora.xiaomi.com

EEE
	chmod 755 "$adbyby_adblack"
	fi

	adbyby_adesc="/etc/storage/adbyby_adesc.sh"
	if [ ! -f "$adbyby_adesc" ] || [ ! -s "$adbyby_adesc" ] ; then
	cat > "$adbyby_adesc" <<-\EEE
weixin.qq.com
qpic.cn
imtt.qq.com

EEE
	chmod 755 "$adbyby_adesc"
	fi

	adbyby_adhost="/etc/storage/adbyby_adhost.sh"
	if [ ! -f "$adbyby_adhost" ] || [ ! -s "$adbyby_adhost" ] ; then
	cat > "$adbyby_adhost" <<-\EEE
cbjs.baidu.com
list.video.baidu.com
nsclick.baidu.com
play.baidu.com
sclick.baidu.com
tieba.baidu.com
baidustatic.com
bdimg.com
bdstatic.com
share.baidu.com
hm.baidu.com
v.baidu.com
cpro.baidu.com
1000fr.net
atianqi.com
56.com
v-56.com
acfun.com
acfun.tv
baofeng.com
baofeng.net
cntv.cn
hoopchina.com.cn
funshion.com
fun.tv
hitvs.cn
hljtv.com
iqiyi.com
qiyi.com
agn.aty.sohu.com
itc.cn
kankan.com
ku6.com
letv.com
letvcloud.com
letvimg.com
pplive.cn
pps.tv
ppsimg.com
pptv.com
www.qq.com
l.qq.com
v.qq.com
video.sina.com.cn
tudou.com
wasu.cn
analytics-union.xunlei.com
kankan.xunlei.com
youku.com
hunantv.com
ifeng.com
renren.com
mediav.com
cnbeta.com
mydrivers.com
168f.info
doubleclick.net
126.net
sohu.com
right.com.cn
50bang.org
you85.cn
jiuzhilan.com
googles.com
cnbetacdn.com
ptqy.gitv.tv
admaster.com.cn
serving-sys.com

EEE
	chmod 755 "$adbyby_adhost"
	fi
}

adbyby_lifecycle_lock_path() {
	printf '%s\n' '/tmp/.adbyby.lifecycle.lock'
}

# Keep a stable inode; unlinking a flock file can split active locks.
adbyby_lifecycle_lock_prepare() {
	adbyby_lock_file="$1"
	[ -n "$adbyby_lock_file" ] || return 1
	adbyby_lock_dir=$(dirname "$adbyby_lock_file") || return 1
	[ -n "$adbyby_lock_dir" ] || adbyby_lock_dir=/
	if [ -L "$adbyby_lock_dir" ] || [ ! -d "$adbyby_lock_dir" ]; then
		logger -t "adbyby" "生命周期锁目录不是普通目录"
		return 1
	fi
	adbyby_lock_uid=$(id -u) || return 1
	adbyby_dir_info=$(stat -c '%u %a' "$adbyby_lock_dir") || return 1
	set -- $adbyby_dir_info
	if [ "$#" -ne 2 ] || [ "$1" != "$adbyby_lock_uid" ]; then
		logger -t "adbyby" "生命周期锁目录所有者不安全"
		return 1
	fi
	adbyby_dir_mode=$((0$2))
	if [ $((adbyby_dir_mode & 0022)) -ne 0 ] && [ $((adbyby_dir_mode & 01000)) -eq 0 ]; then
		logger -t "adbyby" "生命周期锁目录可被其他用户写入且没有sticky保护"
		return 1
	fi
	if [ -L "$adbyby_lock_file" ]; then
		logger -t "adbyby" "生命周期锁文件不能是符号链接"
		return 1
	fi
	if [ ! -e "$adbyby_lock_file" ]; then
		(umask 077; set -C; : > "$adbyby_lock_file") 2>/dev/null || :
	fi
	if [ -L "$adbyby_lock_file" ] || [ ! -f "$adbyby_lock_file" ]; then
		logger -t "adbyby" "生命周期锁文件不是普通文件"
		return 1
	fi
	adbyby_file_info=$(stat -c '%u %a %h' "$adbyby_lock_file") || return 1
	set -- $adbyby_file_info
	if [ "$#" -ne 3 ] || [ "$1" != "$adbyby_lock_uid" ] || [ "$2" != 600 ] || [ "$3" != 1 ]; then
		logger -t "adbyby" "生命周期锁文件权限、所有者或链接数不安全"
		return 1
	fi
	return 0
}

adbyby_lifecycle_lock_execute() {
	adbyby_lock_file="$1"
	adbyby_wait_seconds="$2"
	shift 2
	case "$adbyby_wait_seconds" in ''|*[!0-9]*) return 1 ;; esac
	[ "$#" -gt 0 ] || return 1
	if ( : <&9 ) 2>/dev/null || ( : >&9 ) 2>/dev/null; then
		logger -t "adbyby" "生命周期锁FD 9已被占用，拒绝运行"
		return 1
	fi
	adbyby_old_umask=$(umask)
	umask 077
	if ! exec 9>>"$adbyby_lock_file"; then
		umask "$adbyby_old_umask"
		logger -t "adbyby" "无法打开生命周期锁文件"
		return 1
	fi
	umask "$adbyby_old_umask"
	adbyby_lock_attempt=0
	while ! flock -n 9; do
		if [ "$adbyby_lock_attempt" -ge "$adbyby_wait_seconds" ]; then
			exec 9>&-
			logger -t "adbyby" "等待生命周期锁超时"
			return 1
		fi
		adbyby_lock_attempt=$((adbyby_lock_attempt + 1))
		sleep 1
	done
	if "$@"; then adbyby_action_status=0; else adbyby_action_status=$?; fi
	exec 9>&-
	return "$adbyby_action_status"
}

adbyby_lifecycle_locked_action() {
	adbyby_action="$1"
	if [ "$adbyby_action" != "B" ]; then
		adbyby_refresh_nvram_state || return 1
		nvram set adbyby_adb=0
		nvram set adbybyip_mac_x_0=""
		nvram set adbybyip_ip_x_0=""
		nvram set adbybyip_name_x_0=""
		nvram set adbybyip_ip_road_x_0=""
		nvram set adbybyrules_x_0=""
		nvram set adbybyrules_road_x_0=""
	fi
	case "$adbyby_action" in
		B) adbyby_refresh_blockip_set ;;
		start) adbyby_start ;;
		stop) adbyby_close ;;
		restart)
			adbyby_close || return 1
			if [ "$adbyby_enable" = "1" ]; then adbyby_start; fi
			return 0
			;;
		A) add_rules ;;
		C) add_rule ;;
		D) add_dns ;;
		E) addscripts ;;
		F) hosts_ads ;;
		G|updateadb) adbyby_uprules ;;
		*) return 2 ;;
	esac
}

adbyby_lifecycle_lock_run() {
	adbyby_action="$1"
	adbyby_wait_seconds="${2:-30}"
	case "$adbyby_action" in B|start|stop|restart|A|C|D|E|F|G|updateadb) ;; *) return 2 ;; esac
	adbyby_lock_file=$(adbyby_lifecycle_lock_path) || return 1
	if ! adbyby_lifecycle_lock_prepare "$adbyby_lock_file"; then
		logger -t "adbyby" "生命周期锁准备失败，取消服务操作"
		return 1
	fi
	if ! command -v flock >/dev/null 2>&1; then
		logger -t "adbyby" "系统缺少flock，拒绝无锁执行生命周期操作"
		return 1
	fi
	adbyby_lifecycle_lock_execute "$adbyby_lock_file" "$adbyby_wait_seconds" \
		adbyby_lifecycle_locked_action "$adbyby_action"
}

case "${1:-}" in
	B|start|stop|restart|A|C|D|E|F|G|updateadb) adbyby_lifecycle_lock_run "$1" ;;
	*) echo "check" ;;
esac
