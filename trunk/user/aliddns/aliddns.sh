#!/bin/sh
#copyright by hiboy
#source /etc/storage/init.sh
#ACTION=$1
ALIDDNS_HELPER_DIR="/home/root/.aliddns"
ALIDDNS_HELPER="$ALIDDNS_HELPER_DIR/_aliddns"
export PATH="/etc/storage/bin:$ALIDDNS_HELPER_DIR:/tmp/script:/etc/storage/script:/opt/usr/sbin:/opt/usr/bin:/opt/sbin:/opt/bin:/usr/local/sbin:/usr/sbin:/usr/bin:/sbin:/bin"
export LD_LIBRARY_PATH=/lib:/opt/lib
ACTION=$1
scriptfilepath=$(cd "$(dirname "$0")"; pwd)/$(basename $0)
#echo $scriptfilepath
scriptpath=$(cd "$(dirname "$0")"; pwd)
#echo $scriptpath
scriptname=$(basename $0)

aliddns_enable=`nvram get aliddns_enable`
[ -z $aliddns_enable ] && aliddns_enable=0 && nvram set aliddns_enable=0
if [ "$aliddns_enable" != "0" ] ; then
#nvramshow=`nvram showall | grep '=' | grep aliddns | awk '{print gensub(/'"'"'/,"'"'"'\"'"'"'\"'"'"'","g",$0);}'| awk '{print gensub(/=/,"='\''",1,$0)"'\'';";}'` && eval $nvramshow

aliddns_interval=`nvram get aliddns_interval`
aliddns_ak=`nvram get aliddns_ak`
aliddns_sk=`nvram get aliddns_sk`
aliddns_domain=`nvram get aliddns_domain`
aliddns_name=`nvram get aliddns_name`
aliddns_domain2=`nvram get aliddns_domain2`
aliddns_name2=`nvram get aliddns_name2`
aliddns_domain6=`nvram get aliddns_domain6`
aliddns_name6=`nvram get aliddns_name6`
aliddns_ttl=`nvram get aliddns_ttl`

if [ "$aliddns_domain"x != "x" ] && [ "$aliddns_name"x = "x" ] ; then
	aliddns_name="www"
	nvram set aliddns_name="www"
fi
if [ "$aliddns_domain2"x != "x" ] && [ "$aliddns_name2"x = "x" ] ; then
	aliddns_name2="www"
	nvram set aliddns_name2="www"
fi
if [ "$aliddns_domain6"x != "x" ] && [ "$aliddns_name6"x = "x" ] ; then
	aliddns_name6="www"
	nvram set aliddns_name6="www"
fi

IPv6=0
domain_type=""
hostIP=""
domain=""
name=""
name1=""
timestamp=`date -u "+%Y-%m-%dT%H%%3A%M%%3A%SZ"`
aliddns_record_id=""
[ -z $aliddns_interval ] && aliddns_interval=600 && nvram set aliddns_interval=$aliddns_interval
[ -z $aliddns_ttl ] && aliddns_ttl=600 && nvram set aliddns_ttl=$aliddns_ttl
aliddns_renum=`nvram get aliddns_renum`

fi

if [ ! -z "$(echo $scriptfilepath | grep -v "/tmp/script/" | grep aliddns)" ] && [ ! -s "$ALIDDNS_HELPER" ]; then
	if [ ! -d "$ALIDDNS_HELPER_DIR" ] && [ ! -e "$ALIDDNS_HELPER_DIR" ] && [ ! -L "$ALIDDNS_HELPER_DIR" ]; then
		(umask 077; mkdir "$ALIDDNS_HELPER_DIR") 2>/dev/null
	fi
	if [ -d "$ALIDDNS_HELPER_DIR" ] && [ ! -L "$ALIDDNS_HELPER_DIR" ]; then
		chmod 700 "$ALIDDNS_HELPER_DIR"
		if [ ! -L "$ALIDDNS_HELPER" ]; then
			(umask 077; printf '#!/bin/sh\n"%s" "$@" &\n' "$scriptfilepath" > "$ALIDDNS_HELPER") 2>/dev/null
		fi
		if [ -f "$ALIDDNS_HELPER" ] && [ ! -L "$ALIDDNS_HELPER" ]; then
			chmod 700 "$ALIDDNS_HELPER"
		fi
	fi
fi

aliddns_restart () {

relock="/var/lock/aliddns_restart.lock"
if [ "$1" = "o" ] ; then
	nvram set aliddns_renum="0"
	[ -f $relock ] && rm -f $relock
	return 0
fi
if [ "$1" = "x" ] ; then
	if [ -f $relock ] ; then
		logger -t "【aliddns】" "多次尝试启动失败，等待【"`cat $relock`"分钟】后自动尝试重新启动"
		exit 0
	fi
	aliddns_renum=${aliddns_renum:-"0"}
	aliddns_renum=`expr $aliddns_renum + 1`
	nvram set aliddns_renum="$aliddns_renum"
	if [ "$aliddns_renum" -gt "2" ] ; then
		I=19
		echo $I > $relock
		logger -t "【aliddns】" "多次尝试启动失败，等待【"`cat $relock`"分钟】后自动尝试重新启动"
		while [ $I -gt 0 ]; do
			I=$(($I - 1))
			echo $I > $relock
			sleep 60
			[ "$(nvram get aliddns_renum)" = "0" ] && exit 0
			[ $I -lt 0 ] && break
		done
		nvram set aliddns_renum="0"
	fi
	[ -f $relock ] && rm -f $relock
fi
nvram set aliddns_status=0
eval "$scriptfilepath &"
exit 0
}

aliddns_get_status () {

A_restart=`nvram get aliddns_status`
B_restart="$aliddns_enable$aliddns_interval$aliddns_ak$aliddns_sk$aliddns_domain$aliddns_name$aliddns_domain2$aliddns_name2$aliddns_domain6$aliddns_name6$aliddns_ttl$(cat /etc/storage/ddns_script.sh | grep -v '^#' | grep -v "^$")"
B_restart=`echo -n "$B_restart" | md5sum | sed s/[[:space:]]//g | sed s/-//g`
if [ "$A_restart" != "$B_restart" ] ; then
	nvram set aliddns_status=$B_restart
	needed_restart=1
else
	needed_restart=0
fi
}

aliddns_check () {

aliddns_get_status
if [ "$aliddns_enable" != "1" ] && [ "$needed_restart" = "1" ] ; then
	[ ! -z "$(ps -w | grep "$scriptname keep" | grep -v grep )" ] && logger -t "【aliddns动态域名】" "停止 aliddns" && aliddns_close
	{ kill_ps "$scriptname" exit0; exit 0; }
fi
if [ "$aliddns_enable" = "1" ] ; then
	if [ "$needed_restart" = "1" ] ; then
		aliddns_close
		eval "$scriptfilepath keep &"
		exit 0
	else
		[ -z "$(ps -w | grep "$scriptname keep" | grep -v grep )" ] || [ ! -s "`which curl`" ] && aliddns_restart
	fi
fi
}

aliddns_keep () {
aliddns_start
logger -t "【AliDDNS动态域名】" "守护进程启动"
while true; do
sleep $aliddns_interval
[ ! -s "`which curl`" ] && aliddns_restart
#nvramshow=`nvram showall | grep '=' | grep aliddns | awk '{print gensub(/'"'"'/,"'"'"'\"'"'"'\"'"'"'","g",$0);}'| awk '{print gensub(/=/,"='\''",1,$0)"'\'';";}'` && eval $nvramshow
aliddns_enable=`nvram get aliddns_enable`
[ "$aliddns_enable" = "0" ] && aliddns_close && exit 0;
if [ "$aliddns_enable" = "1" ] ; then
	aliddns_start
fi
done
}

kill_ps () {

COMMAND="$1"
if [ ! -z "$COMMAND" ] ; then
	eval $(ps -w | grep "$COMMAND" | grep -v $$ | grep -v grep | awk '{print "kill "$1";";}')
	eval $(ps -w | grep "$COMMAND" | grep -v $$ | grep -v grep | awk '{print "kill -9 "$1";";}')
fi
if [ "$2" == "exit0" ] ; then
	exit 0
fi
}

aliddns_close () {

kill_ps "$ALIDDNS_HELPER"
kill_ps "_aliddns.sh"
kill_ps "$scriptname"

}

aliddns_start () {
IPv6=0
if [ "$aliddns_domain"x != "x" ] && [ "$aliddns_name"x != "x" ] ; then
	sleep 1
	timestamp=`date -u "+%Y-%m-%dT%H%%3A%M%%3A%SZ"`
	aliddns_record_id=""
	domain="$aliddns_domain"
	name="$aliddns_name"
	arDdnsCheck $aliddns_domain $aliddns_name
fi
if [ "$aliddns_domain2"x != "x" ] && [ "$aliddns_name2"x != "x" ] ; then
	sleep 1
	timestamp=`date -u "+%Y-%m-%dT%H%%3A%M%%3A%SZ"`
	aliddns_record_id=""
	domain="$aliddns_domain2"
	name="$aliddns_name2"
	arDdnsCheck $aliddns_domain2 $aliddns_name2
fi
if [ "$aliddns_domain6"x != "x" ] && [ "$aliddns_name6"x != "x" ] ; then
	IPv6=1
	sleep 1
	timestamp=`date -u "+%Y-%m-%dT%H%%3A%M%%3A%SZ"`
	aliddns_record_id=""
	domain="$aliddns_domain6"
	name="$aliddns_name6"
	arDdnsCheck $aliddns_domain6 $aliddns_name6
fi

}

urlencode() {
	# urlencode <string>
	out=""
	while read -n1 c
	do
		case $c in
			[a-zA-Z0-9._-]) out="$out$c" ;;
			*) out="$out`printf '%%%02X' "'$c"`" ;;
		esac
	done
	echo -n $out
}

enc() {
	echo -n "$1" | urlencode
}

ALIDDNS_CA_BUNDLE="/etc/ssl/certs/ca-certificates.crt"

send_request() {
	args="AccessKeyId=$aliddns_ak&Action=$1&Format=json&$2&Version=2015-01-09"
	hash=$(echo -n "GET&%2F&$(enc "$args")" | openssl dgst -sha1 -hmac "$aliddns_sk&" -binary | openssl base64)

	# The signed query contains credentials. Require a readable trust bundle,
	# verified HTTPS, a bounded transfer and a 2xx response. Do not follow
	# redirects: the request must stay on the official AliDNS endpoint.
	if [ ! -r "$ALIDDNS_CA_BUNDLE" ]; then
		logger -t "【AliDDNS动态域名】" "CA 证书文件不可读：$ALIDDNS_CA_BUNDLE"
		return 1
	fi

	request_url="https://alidns.aliyuncs.com/?$args&Signature=$(enc "$hash")"
	# curl's process argv must not contain AccessKeyId or the signed URL.
	# Reject config delimiters/control bytes rather than allowing a value to
	# inject additional curl config directives through stdin.
	carriage_return=$(printf '\r')
	tab=$(printf '\t')
	case "$request_url" in
		*'"'*|*'\'*|*'
'*|*"$carriage_return"*|*"$tab"*)
			logger -t "【AliDDNS动态域名】" "AliDNS 请求参数包含非法控制字符"
			return 1
			;;
	esac

	if response=$(curl --fail --silent --show-error \
		--connect-timeout 5 --max-time 20 --proto '=https' \
		--cacert "$ALIDDNS_CA_BUNDLE" --write-out '\n%{http_code}' \
		--config - <<EOF
url = "$request_url"
EOF
	); then
		:
	else
		return $?
	fi

	http_status=$(printf '%s\n' "$response" | tail -n 1)
	case "$http_status" in
		2[0-9][0-9]) ;;
		*)
			logger -t "【AliDDNS动态域名】" "AliDNS HTTPS 请求返回 HTTP $http_status"
			return 22
			;;
	esac

	printf '%s\n' "$response" | sed '$d'
}

aliddns_response_ok() {
	[ -n "$1" ] || return 1
	case "$1" in
		\{*) ;;
		*) return 1 ;;
	esac
	printf '%s\n' "$1" | grep -q '"RequestId"[[:space:]]*:'
	[ $? -eq 0 ] || return 1
	if printf '%s\n' "$1" | grep -q '"Code"[[:space:]]*:'; then
		return 1
	fi
	return 0
}

aliddns_update_failed() {
	nvram set aliddns_last_act="$(date '+%Y-%m-%d %H:%M:%S')   更新失败"
	logger -t "【AliDDNS动态域名】" "更新失败"
	return 1
}

aliddns_ipv4_valid() {
	case "$1" in
		""|*[!0-9.]*)
			return 1
			;;
	esac
	printf '%s\n' "$1" | awk -F. '
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
	'
}

aliddns_ipv6_valid() {
	case "$1" in
		""|*[!0-9A-Fa-f:]*)
			return 1
			;;
	esac
	printf '%s\n' "$1" | awk '
		function valid_part(part, groups, count, i) {
			if (part == "")
				return 0
			count = split(part, groups, ":")
			for (i = 1; i <= count; i++) {
				if (length(groups[i]) < 1 || length(groups[i]) > 4 ||
				    groups[i] !~ /^[[:xdigit:]]+$/)
					return 0
			}
			return count
		}
		{
			if (NR != 1 || $0 !~ /^[[:xdigit:]:]+$/ || index($0, ":::"))
				exit 1
			compression = index($0, "::")
			count = 0
			if (compression) {
				left = substr($0, 1, compression - 1)
				right = substr($0, compression + 2)
				if (index(right, "::"))
					exit 1
				if (left != "") {
					n = valid_part(left, left_groups)
					if (!n)
						exit 1
					count += n
				}
				if (right != "") {
					n = valid_part(right, right_groups)
					if (!n)
						exit 1
					count += n
				}
				if (count >= 8)
					exit 1
			} else {
				count = valid_part($0, all_groups)
				if (count != 8)
					exit 1
			}
			valid = 1
		}
		END {
			if (NR != 1 || !valid)
				exit 1
		}
	'
}

get_recordid() {
	grep -Eo '"RecordId":"[0-9]+"' | cut -d':' -f2 | tr -d '"' |head -n1
}

get_recordIP() {
	sed -e "s/"'"TTL":'"/"' \n '"/g" | grep '"Type":"'$domain_type'"' | grep -Eo '"Value":"[^"]*"' | awk -F 'Value":"' '{print $2}' | tr -d '"' |head -n1
}

query_recordInfo() {
	send_request "DescribeDomainRecordInfo" "RecordId=$1&SignatureMethod=HMAC-SHA1&SignatureNonce=$timestamp&SignatureVersion=1.0&Timestamp=$timestamp"
}

query_recordid() {
	send_request "DescribeSubDomainRecords" "SignatureMethod=HMAC-SHA1&SignatureNonce=$timestamp&SignatureVersion=1.0&SubDomain=$name1.$domain&Timestamp=$timestamp&Type=$domain_type"
}

update_record() {
	hostIP_tmp=$(enc "$hostIP")
	send_request "UpdateDomainRecord" "RR=$name1&RecordId=$1&SignatureMethod=HMAC-SHA1&SignatureNonce=$timestamp&SignatureVersion=1.0&TTL=$aliddns_ttl&Timestamp=$timestamp&Type=$domain_type&Value=$hostIP_tmp"
}

add_record() {
	hostIP_tmp=$(enc "$hostIP")
	send_request "AddDomainRecord&DomainName=$domain" "RR=$name1&SignatureMethod=HMAC-SHA1&SignatureNonce=$timestamp&SignatureVersion=1.0&TTL=$aliddns_ttl&Timestamp=$timestamp&Type=$domain_type&Value=$hostIP_tmp"
}

arDdnsInfo() {
case "$name" in
	  \*)
		name1=%2A
		;;
	  \@)
		name1=%40
		;;
	  *)
		name1=$name
		;;
esac

	if [ "$IPv6" = "1" ]; then
		domain_type="AAAA"
	else
		domain_type="A"
	fi
	timestamp=$(date -u "+%Y-%m-%dT%H%%3A%M%%3A%SZ")

	response=$(query_recordid) || {
		logger -t "【AliDDNS动态域名】" "查询记录失败，检查 HTTPS、CA 证书和网络"
		return 1
	}
	if ! aliddns_response_ok "$response"; then
		logger -t "【AliDDNS动态域名】" "AliDNS 返回无效或错误的查询响应"
		return 1
	fi
	aliddns_record_id=$(printf '%s\n' "$response" | get_recordid)
	if [ -z "$aliddns_record_id" ]; then
		# A successful, valid empty result is distinct from a failed query.
		return 2
	fi

	sleep 1
	timestamp=$(date -u "+%Y-%m-%dT%H%%3A%M%%3A%SZ")
	response=$(query_recordInfo "$aliddns_record_id") || {
		logger -t "【AliDDNS动态域名】" "查询记录详情失败"
		return 1
	}
	if ! aliddns_response_ok "$response"; then
		logger -t "【AliDDNS动态域名】" "AliDNS 返回无效或错误的记录详情"
		return 1
	fi
	recordIP=$(printf '%s\n' "$response" | get_recordIP)
	if [ "$IPv6" = "1" ]; then
		aliddns_ipv6_valid "$recordIP" || return 1
	else
		aliddns_ipv4_valid "$recordIP" || return 1
	fi
	printf '%s\n' "$recordIP"
	return 0
}

# Resolve DNS answers with private per-call state and a bounded child lifecycle.
# Atomic mkdir allocation avoids $$ collisions without depending on the target
# BusyBox build's optional mktemp applet.
aliddns_nslookup() {
	(
		lookup_family="$1"
		lookup_name="$2"
		[ -n "$lookup_name" ] || exit 1

		lookup_old_umask=$(umask)
		umask 077
		lookup_dir=""
		lookup_index=0
		while [ "$lookup_index" -lt 100 ]; do
			lookup_candidate_dir="/tmp/arNslookup.$$.${lookup_index}"
			if mkdir "$lookup_candidate_dir" 2>/dev/null; then
				lookup_dir="$lookup_candidate_dir"
				break
			fi
			lookup_index=$((lookup_index + 1))
		done
		umask "$lookup_old_umask"
		[ -n "$lookup_dir" ] || exit 1

		trap 'rm -rf "$lookup_dir"' 0

		lookup_file="$lookup_dir/output"
		nslookup "$lookup_name" > "$lookup_file" 2>&1 &
		lookup_pid=$!
		trap 'trap "" HUP INT TERM; kill "$lookup_pid" 2>/dev/null || :; sleep 1; if kill -0 "$lookup_pid" 2>/dev/null; then kill -9 "$lookup_pid" 2>/dev/null || :; fi; wait "$lookup_pid" 2>/dev/null || :; exit 1' HUP INT TERM
		lookup_remaining=5
		lookup_timed_out=0
		while [ "$lookup_remaining" -gt 0 ]; do
			if ! kill -0 "$lookup_pid" 2>/dev/null; then
				break
			fi
			sleep 1
			lookup_remaining=$((lookup_remaining - 1))
		done

		if kill -0 "$lookup_pid" 2>/dev/null; then
			lookup_timed_out=1
			kill "$lookup_pid" 2>/dev/null || :
			lookup_grace=1
			while [ "$lookup_grace" -gt 0 ]; do
				if ! kill -0 "$lookup_pid" 2>/dev/null; then
					break
				fi
				sleep 1
				lookup_grace=$((lookup_grace - 1))
			done
			if kill -0 "$lookup_pid" 2>/dev/null; then
				kill -9 "$lookup_pid" 2>/dev/null || :
			fi
		fi

		lookup_status=0
		wait "$lookup_pid" 2>/dev/null || lookup_status=$?
		trap - HUP INT TERM
		lookup_pid=""
		if [ "$lookup_timed_out" -ne 0 ] || [ "$lookup_status" -ne 0 ]; then
			exit 1
		fi

		case "$lookup_family" in
			4|6) ;;
			*) exit 1 ;;
		esac
		tail -n +3 "$lookup_file" | awk '$1 == "Address" && NF >= 3 { print $3 }' > "$lookup_dir/candidates"
		lookup_result=""
		while IFS= read -r lookup_candidate; do
			if [ "$lookup_family" = "4" ]; then
				aliddns_ipv4_valid "$lookup_candidate" || continue
			else
				aliddns_ipv6_valid "$lookup_candidate" || continue
			fi
			lookup_result="$lookup_candidate"
			break
		done < "$lookup_dir/candidates"

		[ -n "$lookup_result" ] || exit 1
		printf '%s\n' "$lookup_result"
	)
}

# 查询域名地址
# 参数: 待查询域名
arNslookup() {
	aliddns_nslookup 4 "$1"
}

arNslookup6() {
	aliddns_nslookup 6 "$1"
}

# 更新记录信息
# 参数: 主域名 子域名
arDdnsUpdate() {
case "$name" in
	  \*)
		name1=%2A
		;;
	  \@)
		name1=%40
		;;
	  *)
		name1=$name
		;;
esac
	if [ "$IPv6" = "1" ]; then
		domain_type="AAAA"
		aliddns_ipv6_valid "$hostIP" || {
			logger -t "【AliDDNS动态域名】" "拒绝更新无效的 IPv6 地址"
			aliddns_update_failed
			return 1
		}
	else
		domain_type="A"
		aliddns_ipv4_valid "$hostIP" || {
			logger -t "【AliDDNS动态域名】" "拒绝更新无效的 IPv4 地址"
			aliddns_update_failed
			return 1
		}
	fi

	I=3
	aliddns_record_id=""
	query_ok=0
	while [ "$I" -gt 0 ]; do
		I=$((I - 1))
		timestamp=$(date -u "+%Y-%m-%dT%H%%3A%M%%3A%SZ")
		response=$(query_recordid)
		request_status=$?
		query_ok=0
		if [ "$request_status" -eq 0 ] && aliddns_response_ok "$response"; then
			query_ok=1
			aliddns_record_id=$(printf '%s\n' "$response" | get_recordid)
			echo "recordID $aliddns_record_id"
			[ -n "$aliddns_record_id" ] && break
		else
			logger -t "【AliDDNS动态域名】" "查询记录失败，拒绝在查询错误时添加新记录"
		fi
		[ "$I" -gt 0 ] && sleep 1
	done

	timestamp=$(date -u "+%Y-%m-%dT%H%%3A%M%%3A%SZ")
	if [ "$query_ok" != "1" ]; then
		aliddns_update_failed
		return 1
	fi

	if [ -z "$aliddns_record_id" ]; then
		response=$(add_record)
		request_status=$?
		if [ "$request_status" -ne 0 ] || ! aliddns_response_ok "$response"; then
			logger -t "【AliDDNS动态域名】" "添加记录请求失败"
			aliddns_update_failed
			return 1
		fi
		aliddns_record_id=$(printf '%s\n' "$response" | get_recordid)
		if [ -z "$aliddns_record_id" ]; then
			logger -t "【AliDDNS动态域名】" "添加记录响应缺少记录ID"
			aliddns_update_failed
			return 1
		fi
		echo "added record $aliddns_record_id"
		logger -t "【AliDDNS动态域名】" "添加的记录  $aliddns_record_id"
	else
		response=$(update_record "$aliddns_record_id")
		request_status=$?
		if [ "$request_status" -ne 0 ] || ! aliddns_response_ok "$response"; then
			logger -t "【AliDDNS动态域名】" "更新记录请求失败"
			aliddns_update_failed
			return 1
		fi
		echo "updated record $aliddns_record_id"
		logger -t "【AliDDNS动态域名】" "更新的记录  $aliddns_record_id"
	fi

	nvram set aliddns_record_id="$aliddns_record_id"
	nvram set aliddns_last_act="$(date '+%Y-%m-%d %H:%M:%S')   成功更新：$hostIP"
	logger -t "【AliDDNS动态域名】" "成功更新： $hostIP"
	return 0
}

# 动态检查更新
# 参数: 主域名 子域名
arDdnsCheck() {
	#local postRS
	#local lastIP
	source /etc/storage/ddns_script.sh
	hostIP=$arIpAddress

	if [ "$IPv6" = "1" ]; then
		if ! aliddns_ipv6_valid "$hostIP"; then
			logger -t "【AliDDNS动态域名】" "错误！获取到的 IPv6 地址无效或为空"
			return 1
		fi
	else
		if [ -z "$hostIP" ]; then
			hostIP=$(/usr/bin/aliddns-ipv4.sh) || {
				logger -t "【AliDDNS动态域名】" "错误！HTTPS 公网 IPv4 获取失败，请检查 CA 证书和网络"
				return 1
			}
		fi
		if ! aliddns_ipv4_valid "$hostIP"; then
			logger -t "【AliDDNS动态域名】" "错误！获取到的 IPv4 地址无效"
			return 1
		fi
	fi

	echo "Updating Domain: $2.$1"
	echo "hostIP: $hostIP"
	lastIP=$(arDdnsInfo "$1 $2")
	info_status=$?
	if [ "$info_status" -eq 2 ]; then
		# Valid empty AliDNS result: a new record may be created.
		lastIP=""
	elif [ "$info_status" -ne 0 ]; then
		# API failed; compare with the current DNS answer, but never infer
		# that an unresolved hostname has no record.
		if [ "$IPv6" != "1" ]; then
			lastIP=$(arNslookup "$2.$1") || return 1
			aliddns_ipv4_valid "$lastIP" || return 1
		else
			lastIP=$(arNslookup6 "$2.$1") || return 1
			aliddns_ipv6_valid "$lastIP" || return 1
		fi
	fi
	echo "lastIP: $lastIP"
	if [ "$lastIP" != "$hostIP" ] ; then
		logger -t "【AliDDNS动态域名】" "开始更新 $2.$1 域名 IP 指向"
		logger -t "【AliDDNS动态域名】" "目前 IP: $hostIP"
		logger -t "【AliDDNS动态域名】" "上次 IP: $lastIP"
		sleep 1
		postRS=$(arDdnsUpdate "$1" "$2")
		update_status=$?
		if [ "$update_status" -eq 0 ]; then
			echo "postRS: $postRS"
			logger -t "【AliDDNS动态域名】" "更新动态DNS记录成功！"
			return 0
		fi
		echo "$postRS"
		logger -t "【AliDDNS动态域名】" "更新动态DNS记录失败！请检查您的网络。"
		return 1
	fi
	echo "$lastIP"
	echo "Last IP is the same as current IP!"
	return 1
}


initconfig () {

if [ ! -s "/etc/storage/ddns_script.sh" ] ; then
cat > "/etc/storage/ddns_script.sh" <<-\EEE
arIpAddress () {
	pppoemwan=$(nvram get pppoemwan_enable)
	case "$pppoemwan" in
		""|0) /usr/bin/aliddns-ipv4.sh ;;
		*) /usr/bin/aliddns-ipv4.sh ppp0 ;;
	esac
}
arIpAddress6 () {
ifconfig $(nvram get wan0_ifname_t) | awk '/Global/{print $3}' | awk -F/ '{print $1}'
}
if [ "$IPv6" = "1" ] ; then
arIpAddress=$(arIpAddress6)
else
arIpAddress=$(arIpAddress)
fi
EEE
	chmod 755 "$ddns_script"
fi

}

initconfig

case $ACTION in
start)
	aliddns_close
	aliddns_check
	;;
check)
	aliddns_check
	;;
stop)
	aliddns_close
	;;
keep)
	aliddns_keep
	;;
*)
	aliddns_check
	;;
esac

