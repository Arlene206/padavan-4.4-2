#!/bin/sh
# Copyright (C) 2018 Nick Peng (pymumu@gmail.com)
# Copyright (C) 2019 chongshengB (bkye@vip.qq.com)
# Copyright (C) 2022 TurBoTse (860018505@qq.com)
#
action="$1"

smartdns_process_lifecycle_lib="/usr/bin/smartdns_process_lifecycle.sh"
if [ ! -r "$smartdns_process_lifecycle_lib" ]; then
    echo "SmartDNS process lifecycle helper is missing: $smartdns_process_lifecycle_lib" >&2
    exit 1
fi
. "$smartdns_process_lifecycle_lib"
smartdns_dnsmasq_config_lib="/usr/bin/smartdns_dnsmasq_config.sh"
if [ ! -r "$smartdns_dnsmasq_config_lib" ]; then
    echo "SmartDNS DNSMasq configuration helper is missing: $smartdns_dnsmasq_config_lib" >&2
    exit 1
fi
. "$smartdns_dnsmasq_config_lib"
smartdns_config_file_lib="/usr/bin/smartdns_config_file.sh"
if [ ! -r "$smartdns_config_file_lib" ]; then
    echo "SmartDNS atomic config helper is missing: $smartdns_config_file_lib" >&2
    exit 1
fi
. "$smartdns_config_file_lib"
smartdns_lifecycle_lock_lib="/usr/bin/smartdns_lifecycle_lock.sh"
if [ ! -r "$smartdns_lifecycle_lock_lib" ]; then
    echo "SmartDNS lifecycle lock helper is missing: $smartdns_lifecycle_lock_lib" >&2
    exit 1
fi
. "$smartdns_lifecycle_lock_lib"
storage_Path="/etc/storage"
smartdns_Bin="/usr/bin/smartdns"
smartdns_Ini="$storage_Path/smartdns_conf.ini"
smartdns_Conf="$storage_Path/smartdns.conf"
smartdns_address_Conf="$storage_Path/smartdns_address.conf"
smartdns_blacklist_Conf="$storage_Path/smartdns_blacklist-ip.conf"
smartdns_whitelist_Conf="$storage_Path/smartdns_whitelist-ip.conf"
smartdns_custom_Conf="$storage_Path/smartdns_custom.conf"
dnsmasq_Conf="$storage_Path/dnsmasq/dnsmasq.conf"

smartdns_nvram_read_checked()
{
    smartdns_nvram_value=$(nvram get_checked "$1")
    smartdns_nvram_status=$?
    if [ "$smartdns_nvram_status" -ne 0 ]; then
        logger -t "SmartDNS" "Failed to read NVRAM key $1 (status=$smartdns_nvram_status)"
        return "$smartdns_nvram_status"
    fi
    return 0
}

# sdns_enable is a binary UI selector. An unexpected value must not be
# treated as "disabled" by restart, which would stop a healthy SmartDNS daemon.
smartdns_validate_enable_selector()
{
    [ "$#" -eq 1 ] || return 2
    case "$1" in
        ''|0|1)
            return 0
            ;;
        *)
            logger -t "SmartDNS" "Invalid sdns_enable selector; refusing to change service state"
            return 22
            ;;
    esac
}

smartdns_load_lifecycle_nvram_state()
{
    # An explicit stop must not depend on transient NVRAM availability. The
    # saved SmartDNS state file is the authority for reversing DNSMasq changes.
    if [ "$action" = "stop" ]; then
        sdns_enable=0
        sdns_port=
        sdns_redirect=
        sdnse_enable=
        sdnse_port=
        return 0
    fi

    smartdns_nvram_read_checked sdns_enable || return $?
    sdns_enable=$smartdns_nvram_value
    case "$action" in
        start|restart|reset)
            smartdns_validate_enable_selector "$sdns_enable" || return $?
            ;;
    esac
    smartdns_nvram_read_checked sdns_port || return $?
    sdns_port=$smartdns_nvram_value
    smartdns_nvram_read_checked sdns_redirect || return $?
    sdns_redirect=$smartdns_nvram_value
    smartdns_nvram_read_checked sdnse_enable || return $?
    sdnse_enable=$smartdns_nvram_value
    smartdns_nvram_read_checked sdnse_port || return $?
    sdnse_port=$smartdns_nvram_value
    return 0
}

chn_Route="$storage_Path/chinadns/chnroute.txt"

smartdns_load_runtime_nvram_state()
{
    smartdns_load_lifecycle_nvram_state || return $?
    if [ "$action" != "stop" ]; then
        smartdns_nvram_read_checked sdns_name || return $?
        sdns_name=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_tcp_server || return $?
        sdns_tcp_server=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_ipv6_server || return $?
        sdns_ipv6_server=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_cache || return $?
        sdns_cache=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_cache_persist || return $?
        sdns_cache_persist=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_tcp_idle_time || return $?
        sdns_tcp_idle_time=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_rr_ttl || return $?
        sdns_rr_ttl=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_rr_ttl_min || return $?
        sdns_rr_ttl_min=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_rr_ttl_max || return $?
        sdns_rr_ttl_max=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_rr_ttl_reply_max || return $?
        sdns_rr_ttl_reply_max=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_max_reply_ip_num || return $?
        sdns_max_reply_ip_num=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_speed || return $?
        sdns_speed=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_speed_mode || return $?
        sdns_speed_mode=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_address || return $?
        sdns_address=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_ipset || return $?
        sdns_ipset=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_ipset_timeout || return $?
        sdns_ipset_timeout=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_as || return $?
        sdns_as=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_ip_change || return $?
        sdns_ip_change=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_ip_change_time || return $?
        sdns_ip_change_time=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_dualstack_ip_allow_force_aaaa || return $?
        sdns_dualstack_ip_allow_force_aaaa=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_force_aaaa_soa || return $?
        sdns_force_aaaa_soa=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_force_qtype_soa || return $?
        sdns_force_qtype_soa=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_prefetch_domain || return $?
        sdns_prefetch_domain=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_exp || return $?
        sdns_exp=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_exp_ttl || return $?
        sdns_exp_ttl=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_exp_ttl_max || return $?
        sdns_exp_ttl_max=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_exp_prefetch_time || return $?
        sdns_exp_prefetch_time=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_tcp || return $?
        sdnse_tcp=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_speed || return $?
        sdnse_speed=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_name || return $?
        sdnse_name=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_address || return $?
        sdnse_address=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_ns || return $?
        sdnse_ns=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_ns || return $?
        sdns_ns=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_ipset || return $?
        sdnse_ipset=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_as || return $?
        sdnse_as=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_ipv6_server || return $?
        sdnse_ipv6_server=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_ipc || return $?
        sdnse_ipc=$smartdns_nvram_value
        smartdns_nvram_read_checked sdnse_cache || return $?
        sdnse_cache=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_adblock || return $?
        sdns_adblock=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_white || return $?
        sdns_white=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_black || return $?
        sdns_black=$smartdns_nvram_value
        smartdns_nvram_read_checked sdns_coredump || return $?
        sdns_coredump=$smartdns_nvram_value
    fi

    IPS4="$(ifconfig br0 | grep "inet addr" | grep -v ":127" | grep "Bcast" | awk '{print $2}' | awk -F : '{print $2}')"
    IPS6="$(ifconfig br0 | grep "inet6 addr" | grep -v "fe80::" | grep "Global" | awk '{print $3}')"
    dnsmasq_md5=$(md5sum "$dnsmasq_Conf" | awk '{ print $1 }')
    return 0
}

# 函数

smartdns_state_saved=0
Read_ini () {
    if [ -L "$smartdns_Ini" ]; then
        logger -t "SmartDNS" "SmartDNS saved state is a symlink; refusing resolver cleanup"
        return 1
    fi
    if [ -f "$smartdns_Ini" ]; then
        [ -r "$smartdns_Ini" ] && [ -s "$smartdns_Ini" ] || return 1
        if hosts_type=$(sed -n '1p' "$smartdns_Ini") &&
           sdns_redirected=$(sed -n '2p' "$smartdns_Ini") &&
           sdns_ported=$(sed -n '3p' "$smartdns_Ini") &&
           sdnse_ported=$(sed -n '4p' "$smartdns_Ini") &&
           smartdns_state_version=$(sed -n '5p' "$smartdns_Ini"); then
            :
        else
            return 1
        fi
        case "$hosts_type" in SmartDNS|dnsmasq|0) ;; *) return 1 ;; esac
        case "$sdns_redirected" in ''|0) sdns_redirected=0 ;; 1|2) ;; *) return 1 ;; esac
        sdns_ported=$(smartdns_dnsmasq_normalize_port "$sdns_ported") || return 1
        case "$smartdns_state_version" in
            ''|legacy) smartdns_state_version=legacy ;;
            managed-v1) ;;
            *) return 1 ;;
        esac
        smartdns_state_saved=1
    elif [ -e "$smartdns_Ini" ]; then
        return 1
    else
        hosts_type=0
        sdns_redirected=0
        sdns_ported="$sdns_port"
        sdnse_ported="$sdnse_port"
        smartdns_state_version=legacy
        smartdns_state_saved=0
    fi

    case "$action" in
        start|restart|reset)
            case "$sdns_redirect" in ''|0) sdns_redirect=0 ;; 1|2) ;; *) return 1 ;; esac
            sdns_port=$(smartdns_dnsmasq_normalize_port "$sdns_port") || return 1
            ;;
    esac
    return 0
}

Check_md5 () {
    # 【检测某些文件是否变动】
    echo "smartdns：" "Enter Check_md5"
    
    local files="$storage_Path/smartdns_*.sh"
    local md5="$storage_Path/smartdns.md5"
    local new_md5="/tmp/smartdns.md5"
    local status=0
    
    md5sum -b "$files" > $new_md5
    if [ -s "$md5" ] ; then    # 存在，则比较
        diff $md5 $new_md5 >/dev/null 2>&1
        [ $? -eq 1 ] && status=1    # 不同，则存储并更新
    else
        status=1    # 不存在，则存储并更新
    fi
    [ "$status" = 1 ] && cat $new_md5 > $md5  \
    && mtd_storage.sh save >/dev/null 2>&1
    
    echo "smartdns：" "Leave Check_md5"
}

Check_ss()
{
    if [ -s /etc_ro/ss_ip.sh ]; then
        smartdns_nvram_read_checked ss_enable || return $?
        ss_enable=$smartdns_nvram_value
        smartdns_nvram_read_checked ss_run_mode || return $?
        ss_run_mode=$smartdns_nvram_value
        smartdns_nvram_read_checked pdnsd_enable || return $?
        pdnsd_enable=$smartdns_nvram_value
        if [ "$ss_enable" = 1 ] && [ "$ss_run_mode" = "router" ] && [ "$pdnsd_enable" = 0 ]; then
            logger -t "SmartDNS" "系统检测到 SS 模式为绕过大陆模式，并且启用了 pdnsd 请先调整 SS 解析使用 SmartDNS +手动配置模式！程序将退出..."
            nvram set sdns_enable=0
            exit 0
        fi
    fi
    return 0
}

Get_sdns_conf () {
    # 【基本设置】
    :>"$smartdns_tmp_Conf"
    echo "server-name $sdns_name" >> "$smartdns_tmp_Conf"
    ARGS_1=""
    if [ "$sdns_address" = "1" ] ; then
     ARGS_1="$ARGS_1 -no-rule-addr"
    fi
    if [ "$sdns_ns" = "1" ] ; then
        ARGS_1="$ARGS_1 -no-rule-nameserver"
    fi
    if [ "$sdns_ipset" = "1" ] ; then
        ARGS_1="$ARGS_1 -no-rule-ipset"
    fi
    if [ "$sdns_speed" = "1" ] ; then
        ARGS_1="$ARGS_1 -no-speed-check"
    fi
    if [ "$sdns_as" = "1" ] ; then
        ARGS_1="$ARGS_1 -no-rule-soa"
    fi
    if [ "$sdns_ipv6_server" = "1" ] ; then
        echo "bind" "[::]:$sdns_port $ARGS_1" >> "$smartdns_tmp_Conf"
    else
        echo "bind" ":$sdns_port $ARGS_1" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_tcp_server" = "1" ] ; then
        if [ "$sdns_ipv6_server" = "1" ] ; then
            echo "bind-tcp" "[::]:$sdns_port $ARGS_1" >> "$smartdns_tmp_Conf"
        else
            echo "bind-tcp" ":$sdns_port $ARGS_1" >> "$smartdns_tmp_Conf"
        fi
    fi
    # 【读取配置】
    Get_sdnse_conf || return $?
    echo "cache-size $sdns_cache" >> "$smartdns_tmp_Conf"
    echo "rr-ttl $sdns_rr_ttl" >> "$smartdns_tmp_Conf"
    echo "rr-ttl-min $sdns_rr_ttl_min" >> "$smartdns_tmp_Conf"
    echo "rr-ttl-max $sdns_rr_ttl_max" >> "$smartdns_tmp_Conf"
    echo "tcp-idle-time $sdns_tcp_idle_time" >> "$smartdns_tmp_Conf"
    echo "rr-ttl-reply-max $sdns_rr_ttl_reply_max" >> "$smartdns_tmp_Conf"
    echo "max-reply-ip-num $sdns_max_reply_ip_num" >> "$smartdns_tmp_Conf"
    echo "serve-expired-ttl $sdns_exp_ttl" >> "$smartdns_tmp_Conf"
    echo "serve-expired-reply-ttl $sdns_exp_ttl_max" >> "$smartdns_tmp_Conf"
    echo "serve-expired-prefetch-time $sdns_exp_prefetch_time" >> "$smartdns_tmp_Conf"
    echo "force-qtype-SOA $sdns_force_qtype_soa" >> "$smartdns_tmp_Conf"
    echo "speed-check-mode $sdns_speed_mode" >> "$smartdns_tmp_Conf"
    if [ "$sdns_ip_change" -eq 1 ] ;then
        echo "dualstack-ip-selection yes" >> "$smartdns_tmp_Conf"
        echo "dualstack-ip-selection-threshold $sdns_ip_change_time" >> "$smartdns_tmp_Conf"
    elif [ "$sdns_force_aaaa_soa" -eq 1 ] ;then
        echo "force-AAAA-SOA yes" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_dualstack_ip_allow_force_aaaa" -eq 1 ] && [ " $sdns_cache" -gt 0 ] ;then
        echo "dualstack-ip-allow-force-AAAA yes" >> "$smartdns_tmp_Conf"
    else
        echo "dualstack-ip-allow-force-AAAA no" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_cache_persist" -eq 1 ] && [ "$sdns_cache" -gt 0 ] ;then
        echo "cache-persist yes" >> "$smartdns_tmp_Conf"
        echo "cache-file /tmp/smartdns.cache" >> "$smartdns_tmp_Conf"
    else
        echo "cache-persist no" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_prefetch_domain" -eq 1 ] && [ " $sdns_cache" -gt 0 ] ;then
        echo "prefetch-domain yes" >> "$smartdns_tmp_Conf"
    else
        echo "prefetch-domain no" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_ipset_timeout" -eq 1 ] && [ " $sdns_cache" -gt 0 ] ;then
        echo "ipset-timeout yes" >> "$smartdns_tmp_Conf"
    else
        echo "ipset-timeout no" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_force_aaaa_soa" -eq 1 ] && [ " $sdns_cache" -gt 0 ] ;then
        echo "force-AAAA-SOA yes" >> "$smartdns_tmp_Conf"
    else
        echo "force-AAAA-SOA no" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_exp" -eq 1 ] && [ "$sdns_cache" -gt 0 ] ;then
        echo "serve-expired yes" >> "$smartdns_tmp_Conf"
    else
        echo "serve-expired no" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_adblock" -eq 1 ] && [ "$sdns_cache" -gt 0 ] ;then
        echo "conf-file /tmp/anti-ad-for-smartdns.conf" >> "$smartdns_tmp_Conf"
    fi
    echo "log-level error" >> "$smartdns_tmp_Conf"
    smartdns_nvram_read_checked sdns_staticnum_x || return $?
    listnum=$smartdns_nvram_value
    case "$listnum" in
        '') listnum=0 ;;
        *[!0-9]*) return 1 ;;
    esac
    while [ "${listnum#0}" != "$listnum" ]; do
        listnum=${listnum#0}
    done
    [ -n "$listnum" ] || listnum=0
    [ "${#listnum}" -le 2 ] && [ "$listnum" -le 64 ] 2>/dev/null || return 1
    j=0
    while [ "$j" -lt "$listnum" ]; do
        smartdns_nvram_read_checked "sdnss_enable_x$j" || return $?
        sdnss_enable=$smartdns_nvram_value
        case "$sdnss_enable" in
        1)
            smartdns_nvram_read_checked "sdnss_name_x$j" || return $?
            sdnss_name=$smartdns_nvram_value
            smartdns_nvram_read_checked "sdnss_ip_x$j" || return $?
            sdnss_ip=$smartdns_nvram_value
            smartdns_nvram_read_checked "sdnss_port_x$j" || return $?
            sdnss_port=$smartdns_nvram_value
            smartdns_nvram_read_checked "sdnss_type_x$j" || return $?
            sdnss_type=$smartdns_nvram_value
            smartdns_nvram_read_checked "sdnss_ipc_x$j" || return $?
            sdnss_ipc=$smartdns_nvram_value
            smartdns_nvram_read_checked "sdnss_named_x$j" || return $?
            sdnss_named=$smartdns_nvram_value
            smartdns_nvram_read_checked "sdnss_non_x$j" || return $?
            sdnss_non=$smartdns_nvram_value
            smartdns_nvram_read_checked "sdnss_ipset_x$j" || return $?
            sdnss_ipset=$smartdns_nvram_value
            ipc=""
            named=""
            non=""
            ipset=""
            if [ "$sdnss_ipc" = "whitelist" ] ; then
                ipc="-whitelist-ip"
            elif [ "$sdnss_ipc" = "blacklist" ] ; then
                ipc="-blacklist-ip"
            fi
            if [ "$sdnss_named"x != x ] ; then
                named="-group $sdnss_named"
            fi
            if [ "$sdnss_non" = "1" ] ; then
                non="-exclude-default-group"
            fi
            if [ "$sdnss_type" = "tcp" ] ; then
                if [ "$sdnss_port" = "default" ] ; then
                    echo "server-tcp $sdnss_ip:53 $ipc $named $non" >> "$smartdns_tmp_Conf"
                else
                    echo "server-tcp $sdnss_ip:$sdnss_port $ipc $named $non" >> "$smartdns_tmp_Conf"
                fi
            elif [ "$sdnss_type" = "udp" ] ; then
                if [ "$sdnss_port" = "default" ] ; then
                    echo "server $sdnss_ip:53 $ipc $named $non" >> "$smartdns_tmp_Conf"
                else
                    echo "server $sdnss_ip:$sdnss_port $ipc $named $non" >> "$smartdns_tmp_Conf"
                fi
            elif [ "$sdnss_type" = "tls" ] ; then
                if [ "$sdnss_port" = "default" ] ; then
                    echo "server-tls $sdnss_ip:853 $ipc $named $non" >> "$smartdns_tmp_Conf"
                else
                    echo "server-tls $sdnss_ip:$sdnss_port $ipc $named $non" >> "$smartdns_tmp_Conf"
                fi
            elif [ "$sdnss_type" = "https" ] ; then
                if [ "$sdnss_port" = "default" ] ; then
                    echo "server-https $sdnss_ip:443 $ipc $named $non" >> "$smartdns_tmp_Conf"
                else
                    echo "server-https $sdnss_ip:$sdnss_port $ipc $named $non" >> "$smartdns_tmp_Conf"
                fi
            fi
            if [ "$sdnss_ipset"x != x ] ; then
                Check_ip_addr "$sdnss_ipset"
                if [ "$?" = "1" ] ;then
                    echo "ipset /$sdnss_ipset/smartdns" >> "$smartdns_tmp_Conf"
                else
                    ipset add smartdns "$sdnss_ipset" 2>/dev/null
                fi
            fi
            ;;
        ''|0) ;;
        *) return 1 ;;
        esac
        j=$((j + 1))
    done
    if [ "$sdns_white" = "1" ] && [ -f "$chn_Route" ] ; then
        :>/tmp/whitelist.conf
        logger -t "SmartDNS" "开始处理白名单IP..."
        awk '{printf("whitelist-ip %s\n", $1, $1 )}' "$chn_Route" >> /tmp/whitelist.conf
        echo "conf-file /tmp/whitelist.conf" >> "$smartdns_tmp_Conf"
    fi
    if [ "$sdns_black" = "1" ] && [ -f "$chn_Route" ] ; then
        :>/tmp/blacklist.conf
        logger -t "SmartDNS" "开始处理黑名单IP..."
        awk '{printf("blacklist-ip %s\n", $1, $1 )}' "$chn_Route" >> /tmp/blacklist.conf
        echo "conf-file /tmp/blacklist.conf" >> "$smartdns_tmp_Conf"
    fi
}

Get_sdnse_conf () {
    # 【读取第二服务器设置】
    if [ "$sdnse_enable" -eq 1 ] ; then
    ARGS_2=""
    ADDR=""
    if [ "$sdnse_speed" = "1" ] ; then
        ARGS_2="$ARGS_2 -no-speed-check"
    fi
    if [ -n "$sdnse_name" ] ; then
        ARGS_2="$ARGS_2-group $sdnse_name"
    fi
    if [ "$sdnse_address" = "1" ] ; then
        ARGS_2="$ARGS_2-no-rule-addr"
    fi
    if [ "$sdnse_ns" = "1" ] ; then
        ARGS_2="$ARGS_2-no-rule-nameserver"
    fi
    if [ "$sdnse_ipset" = "1" ] ; then
        ARGS_2="$ARGS_2-no-rule-ipset"
    fi
    if [ "$sdnse_as" = "1" ] ; then
        ARGS_2="$ARGS_2-no-rule-soa"
    fi
    if [ "$sdnse_ipc" = "1" ] ; then
        ARGS_2="$ARGS_2-no-dualstack-selection"
    fi
    if [ "$sdnse_cache" = "1" ] ; then
        ARGS_2="$ARGS_2-no-cache"
    fi
    if [ "$sdnse_ipv6_server" = "1" ] ; then
        ADDR="[::]"
    else
        ADDR=""
    fi
    printf '%s\n' "bind $ADDR:$sdnse_port $ARGS_2" >> "$smartdns_tmp_Conf" || return $?
    if [ "$sdnse_tcp_server" = "1" ] ; then
        printf '%s\n' "bind-tcp $ADDR:$sdnse_port $ARGS_2" >> "$smartdns_tmp_Conf" || return $?
    fi
fi
}

Check_ip_addr () {
    echo "$1"|grep "^[0-9]\{1,3\}\.\([0-9]\{1,3\}\.\)\{2\}[0-9]\{1,3\}$" >/dev/null
    # IP地址必须为全数字
    if [ $? -ne 0 ] ; then
        return 1
    fi
    ipaddr=$1
    a=$(echo "$ipaddr"|awk -F . '{ print $1 }')  # 以"."分隔，取出每个列的值
    b=$(echo "$ipaddr"|awk -F . '{ print $2 }')
    c=$(echo "$ipaddr"|awk -F . '{ print $3 }')
    d=$(echo "$ipaddr"|awk -F . '{ print $4 }')
    for num in $a $b $c $d
    do
        if [ "$num" -gt 255 ] || [ "$num" -lt 0 ] ; then   # 每个数值必须在0-255之间
            return 1
        fi
    done
    return 0
}

smartdns_prepare_adblock_filter()
{
    _smartdns_adblock_input=$1
    _smartdns_adblock_output=$2
    _smartdns_adblock_snapshot=$(mktemp "${_smartdns_adblock_output}.input.XXXXXX") || return 1
    _smartdns_adblock_tmp=$(mktemp "${_smartdns_adblock_output}.tmp.XXXXXX") || {
        rm -f "$_smartdns_adblock_snapshot"
        return 1
    }

    # Snapshot with a checked read: awk's END block can otherwise mask an
    # input-open error and make an incomplete list look like a valid empty one.
    if ! cat "$_smartdns_adblock_input" > "$_smartdns_adblock_snapshot"; then
        rm -f "$_smartdns_adblock_snapshot" "$_smartdns_adblock_tmp"
        return 1
    fi

    if grep -Eq '^[[:space:]]*address=' "$_smartdns_adblock_snapshot"; then
        if ! cp "$_smartdns_adblock_snapshot" "$_smartdns_adblock_tmp"; then
            rm -f "$_smartdns_adblock_snapshot" "$_smartdns_adblock_tmp"
            return 1
        fi
    else
        _smartdns_adblock_grep_status=$?
        if [ "$_smartdns_adblock_grep_status" -ne 1 ]; then
            rm -f "$_smartdns_adblock_snapshot" "$_smartdns_adblock_tmp"
            return "$_smartdns_adblock_grep_status"
        fi

        awk '
            /^[[:space:]]*!/ || /^[[:space:]]*$/ { next }
            {
                rule = $0
                sub(/\r$/, "", rule)
                if (index(rule, "||") != 1 || substr(rule, length(rule), 1) != "^")
                    next

                domain = substr(rule, 3, length(rule) - 3)
                if (length(domain) <= 253 &&
                    domain ~ /^[A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?(\.[A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?)*$/) {
                    printf "address=/%s/0.0.0.0\n", domain
                    converted++
                }
            }
            END {
                if (converted == 0)
                    exit 1
            }
        ' "$_smartdns_adblock_snapshot" > "$_smartdns_adblock_tmp"
        _smartdns_adblock_status=$?
        if [ "$_smartdns_adblock_status" -ne 0 ]; then
            rm -f "$_smartdns_adblock_snapshot" "$_smartdns_adblock_tmp"
            return "$_smartdns_adblock_status"
        fi
    fi

    rm -f "$_smartdns_adblock_snapshot"
    if ! mv -f "$_smartdns_adblock_tmp" "$_smartdns_adblock_output"; then
        rm -f "$_smartdns_adblock_tmp"
        return 1
    fi
    return 0
}

Start_AD () {
# Download and normalize the optional Adblock Plus/SmartDNS rule list.
    _smartdns_adblock_output=/tmp/anti-ad-for-smartdns.conf
    _smartdns_adblock_empty=$(mktemp "${_smartdns_adblock_output}.empty.XXXXXX") || {
        logger -t "SmartDNS" "无法创建广告过滤临时文件，跳过广告过滤"
        return 1
    }
    if ! mv -f "$_smartdns_adblock_empty" "$_smartdns_adblock_output"; then
        rm -f "$_smartdns_adblock_empty"
        logger -t "SmartDNS" "无法初始化广告过滤文件，跳过广告过滤"
        return 1
    fi

    _smartdns_adblock_download=$(mktemp /tmp/sdnsadnew.conf.XXXXXX) || {
        logger -t "SmartDNS" "无法创建广告过滤下载文件，跳过广告过滤"
        return 1
    }
    if [ -z "$sdns_adblock_url" ] ||
        ! curl -fsS -o "$_smartdns_adblock_download" --connect-timeout 10 --retry 3 "$sdns_adblock_url"; then
        logger -t "SmartDNS" "广告过滤文件下载失败，继续使用空过滤列表"
        rm -f "$_smartdns_adblock_download"
        return 1
    fi
    if [ ! -s "$_smartdns_adblock_download" ]; then
        logger -t "SmartDNS" "广告过滤文件为空，继续使用空过滤列表"
        rm -f "$_smartdns_adblock_download"
        return 1
    fi
    if ! smartdns_prepare_adblock_filter "$_smartdns_adblock_download" "$_smartdns_adblock_output"; then
        logger -t "SmartDNS" "广告过滤文件格式无效，继续使用空过滤列表"
        rm -f "$_smartdns_adblock_download"
        return 1
    fi

    logger -t "SmartDNS" "去广告文件下载并处理成功..."
    rm -f "$_smartdns_adblock_download"
    return 0
}

Change_adbyby () {
# 【】
    # DNS ownership must not change unless Adbyby process state is known.
    smartdns_named_process_state adbyby
    adbyby_process_status=$?
    case "$adbyby_process_status" in
        0) ;;
        1) return 0 ;;
        *)
            logger -t "SmartDNS" "Cannot determine Adbyby process state; refusing DNS ownership transition"
            return 2
            ;;
    esac
    smartdns_nvram_read_checked adbyby_enable || return $?
    [ "$smartdns_nvram_value" = "1" ] || return 0

    case "$sdns_enable" in
    0)
        smartdns_nvram_read_checked adbyby_add || return $?
        if [ "$smartdns_nvram_value" = "1" ] && [ "$hosts_type" != "dnsmasq" ]; then
            if nvram set adbyby_add=0; then
                :
            else
                adbyby_nvram_status=$?
                logger -t "SmartDNS" "Failed to persist adbyby_add=0 (status=$adbyby_nvram_status)"
                return "$adbyby_nvram_status"
            fi
            # The Adbyby init script has no switch action; persist DNS ownership locally.
            logger -t "SmartDNS" "DNS 去广告规则: SmartDNS ⇒ DNSmasq"
            hosts_type="dnsmasq"
        fi
        ;;
    1)
        if [ "$hosts_type" != "SmartDNS" ] && [ "$action" = "start" ]; then
            smartdns_adbyby_update_target=0
            if [ "$sdns_port" = "53" ]; then
                smartdns_adbyby_update_target=1
            else
                smartdns_nvram_read_checked adbyby_add || return $?
                if [ "$smartdns_nvram_value" = "1" ] || [ "$sdns_redirect" = "2" ]; then
                    smartdns_adbyby_update_target=1
                fi
            fi
            if [ "$smartdns_adbyby_update_target" = "1" ]; then
                if nvram set adbyby_add=1; then
                    :
                else
                    adbyby_nvram_status=$?
                    logger -t "SmartDNS" "Failed to persist adbyby_add=1 (status=$adbyby_nvram_status)"
                    return "$adbyby_nvram_status"
                fi
                # The Adbyby init script has no switch action; persist DNS ownership locally.
                logger -t "SmartDNS" "DNS 去广告规则: DNSmasq ⇒ SmartDNS"
                hosts_type="SmartDNS"
            fi
        fi
        ;;
    esac
}

Change_dnsmasq () {
    smartdns_dnsmasq_action="${1:-$action}"
    case "$smartdns_dnsmasq_action" in
    stop)
        if [ "$smartdns_state_saved" = "1" ]; then
            if smartdns_dnsmasq_remove_saved_directives "$dnsmasq_Conf" "$sdns_redirected" "$sdns_ported" "$smartdns_state_version"; then
                :
            else
                smartdns_dnsmasq_status=$?
                logger -t "SmartDNS" "Could not remove the previously managed DNSMasq directives (status=$smartdns_dnsmasq_status)"
                return "$smartdns_dnsmasq_status"
            fi
        fi
        if [ "$sdns_enable" = 0 ] ; then
            [ "$sdns_ported" = "53" ] && logger -t "SmartDNS" "已启用 DNSmasq 域名解析（DNS）功能"
            [ "$sdns_redirected" = "1" ] && logger -t "SmartDNS" "删除 DNSmasq 上游服务器：127.0.0.1:$sdns_ported"
        fi
        ;;
    start)
        set -- "$dnsmasq_Conf"
        if [ "$sdns_port" = "53" ] ; then
            if [ "$sdns_redirect" = "1" ] ; then
                nvram set sdns_redirect=0
                sdns_redirect=0
                logger -t "SmartDNS" "因此，自动修改重定向为：无"
            fi
            set -- "$@" port=0
            logger -t "SmartDNS" "已关闭 DNSmasq 域名解析（DNS）功能..."
        fi
        if [ "$sdns_redirect" = "1" ] ; then
            set -- "$@" no-resolv "server=127.0.0.1#$sdns_port"
            logger -t "SmartDNS" "作为 DNSmasq 上游服务器：127.0.0.1#$sdns_port"
            if [ "$sdnse_enable" = 1 ] ; then
                logger -t "SmartDNS" "作为 DNSmasq 上游服务器：127.0.0.1:$sdnse_port"
            fi
        fi
        if [ "$#" -gt 1 ]; then
            if smartdns_dnsmasq_publish_managed_block "$@"; then
                :
            else
                smartdns_dnsmasq_status=$?
                logger -t "SmartDNS" "Could not atomically publish DNSMasq resolver directives (status=$smartdns_dnsmasq_status)"
                return "$smartdns_dnsmasq_status"
            fi
        fi
        ;;
    *)
        return 2
        ;;
    esac
    return 0
}
smartdns_write_ini_state () {
    [ ! -L "$smartdns_Ini" ] || return 1
    smartdns_ini_tmp=$(mktemp "${smartdns_Ini}.tmp.XXXXXX") || return 1
    if printf '%s\n' "$hosts_type" "$sdns_redirected" "$sdns_ported" "$sdnse_port" "$smartdns_state_version" > "$smartdns_ini_tmp"; then
        :
    else
        smartdns_ini_status=$?
        rm -f "$smartdns_ini_tmp"
        return "$smartdns_ini_status"
    fi
    if chmod 600 "$smartdns_ini_tmp"; then :; else
        smartdns_ini_status=$?
        rm -f "$smartdns_ini_tmp"
        return "$smartdns_ini_status"
    fi
    if mv -f "$smartdns_ini_tmp" "$smartdns_Ini"; then
        return 0
    else
        smartdns_ini_status=$?
        rm -f "$smartdns_ini_tmp"
        return "$smartdns_ini_status"
    fi
}

Change_iptable () {
    # 【端口转发】
    local statu=0
    case $action in
    stop)
        if [ "$sdns_redirected" = 2 ] ; then
            iptables -t nat -D PREROUTING -p tcp -d "$IPS4" --dport 53 -j REDIRECT --to-ports "$sdns_ported" >/dev/null 2>&1
            iptables -t nat -D PREROUTING -p udp -d "$IPS4" --dport 53 -j REDIRECT --to-ports "$sdns_ported" >/dev/null 2>&1
            ip6tables -t nat -D PREROUTING -p tcp -d "$IPS6" --dport 53 -j REDIRECT --to-ports "$sdns_ported" >/dev/null 2>&1
            ip6tables -t nat -D PREROUTING -p udp -d "$IPS6" --dport 53 -j REDIRECT --to-ports "$sdns_ported" >/dev/null 2>&1
            [ "$sdns_enable" = 0 ] && logger -t "SmartDNS" "恢复重定向 $IPS4:$sdns_ported 至 xxx.xxx.xxx:53"
        fi
        if [ "$sdns_redirected" = 1 ] ; then
            iptables -t nat -D PREROUTING -p udp -d "$IPS4" --dport 53 -j REDIRECT --to-ports 53 >/dev/null 2>&1
        fi
        ;;
    start)
        if [ "$sdns_redirected" != 2 ] && [ "$sdns_redirect" = 2 ] ; then
            statu=1
            logger -t "SmartDNS" "重定向 xxx.xxx.xxx:53 至 $IPS4:$sdns_port"
            if [ "$sdnse_enable" = 1 ] ; then
                logger -t "SmartDNS" "重定向 xxx.xxx.xxx:53 至 $IPS4:$sdnse_port"
            fi
        fi
        ;;
    reset)
        if [ "$sdns_redirect" = 2 ] ; then
            statu=1
        fi
        if [ "$sdns_redirect" = 1 ] ; then
            iptables -t nat -A PREROUTING -p udp -d "$IPS4" --dport 53 -j REDIRECT --to-ports 53 >/dev/null 2>&1
        fi
        ;;
    esac
    if [ "$statu" = 1 ] ; then
        [ "$sdns_tcp_server" = "1" ] && iptables -t nat -A PREROUTING -p tcp -d "$IPS4" --dport 53 -j REDIRECT --to-ports "$sdns_port" >/dev/null 2>&1
        iptables -t nat -A PREROUTING -p udp -d "$IPS4" --dport 53 -j REDIRECT --to-ports "$sdns_port" >/dev/null 2>&1
        if [ "$sdns_ipv6_server" = "1" ] ; then
            [ "$sdns_tcp_server" = "1" ] && ip6tables -t nat -A PREROUTING -p tcp -d "$IPS6" --dport 53 -j REDIRECT --to-ports "$sdns_port" >/dev/null 2>&1
            ip6tables -t nat -A PREROUTING -p udp -d "$IPS6" --dport 53 -j REDIRECT --to-ports "$sdns_port" >/dev/null 2>&1
        fi
    fi
}

smartdns_restore_previous_instance()
{
    [ "${smartdns_previous_running:-0}" = 1 ] || return 0
    smartdns_process_state
    _smartdns_restore_state=$?
    case "$_smartdns_restore_state" in
        0) return 0 ;;
        1) ;;
        *) return 2 ;;
    esac
    if [ -L "$smartdns_Conf" ] || [ ! -f "$smartdns_Conf" ] || [ ! -r "$smartdns_Conf" ]; then
        logger -t "SmartDNS" "原 SmartDNS 配置不可安全读取，无法恢复旧进程"
        return 1
    fi
    "$smartdns_Bin" -f -c "$smartdns_Conf" "${args:-}" >/dev/null 2>&1 &
    sleep 1
    smartdns_process_state
    _smartdns_restore_state=$?
    if [ "$_smartdns_restore_state" -eq 0 ]; then
        logger -t "SmartDNS" "已恢复原 SmartDNS 进程以维持 DNSMasq 当前上游"
        return 0
    fi
    logger -t "SmartDNS" "无法恢复原 SmartDNS 进程 (status=$_smartdns_restore_state)"
    return 1
}

smartdns_start_config_rollback()
{
    _smartdns_failure_status=$1
    if [ -n "$smartdns_tmp_Conf" ]; then
        rm -f "$smartdns_tmp_Conf"
    fi

    if nvram set sdns_enable=0; then
        _smartdns_nvram_status=0
    else
        _smartdns_nvram_status=$?
        logger -t "SmartDNS" "无法持久化停用标记 (status=$_smartdns_nvram_status)"
    fi
    sdns_enable=0
    Stop_smartdns
    _smartdns_cleanup_status=$?
    if [ "$_smartdns_cleanup_status" -ne 0 ]; then
        logger -t "SmartDNS" "配置生成失败后的 DNSMasq 回滚未完成 (status=$_smartdns_cleanup_status)"
        smartdns_restore_previous_instance || :
        return "$_smartdns_cleanup_status"
    fi
    if [ "$_smartdns_nvram_status" -ne 0 ]; then
        return "$_smartdns_nvram_status"
    fi
    return "$_smartdns_failure_status"
}

Start_smartdns () {
    smartdns_previous_running=0
    # Do not modify DNS, firewall or persisted state until the old daemon is
    # positively classified and, when running, confirmed stopped.
    smartdns_process_state
    smartdns_state=$?
    case "$smartdns_state" in
        0)
            smartdns_previous_running=1
            smartdns_stop_process
            stop_status=$?
            if [ "$stop_status" -ne 0 ]; then
                logger -t "SmartDNS" "Existing SmartDNS instance did not stop cleanly (status=$stop_status)"
                return "$stop_status"
            fi
            ;;
        1) smartdns_previous_running=0 ;;
        *)
            logger -t "SmartDNS" "Cannot determine SmartDNS process state; startup aborted"
            return 2
            ;;
    esac

    if [ "$smartdns_state_saved" = "1" ]; then
        if smartdns_dnsmasq_remove_saved_directives "$dnsmasq_Conf" "$sdns_redirected" "$sdns_ported" "$smartdns_state_version"; then
            :
        else
            smartdns_cleanup_status=$?
            logger -t "SmartDNS" "Previous DNSMasq resolver state could not be removed (status=$smartdns_cleanup_status)"
            smartdns_restore_previous_instance || :
            return "$smartdns_cleanup_status"
        fi
    fi
    [ "$sdns_enable" -eq 0 ] && nvram set sdns_enable=1 && sdns_enable=1
    if Change_adbyby; then
        :
    else
        smartdns_adbyby_status=$?
        logger -t "SmartDNS" "Adbyby DNS target transition failed; restoring previous SmartDNS instance (status=$smartdns_adbyby_status)"
        smartdns_restore_previous_instance || :
        return "$smartdns_adbyby_status"
    fi
    sdns_redirected="$sdns_redirect"
    sdns_ported="$sdns_port"
    smartdns_state_version=managed-v1
    if [ "$sdns_port" = "53" ] && [ "$sdns_redirect" = "1" ]; then
        sdns_redirected=0
    fi
    if smartdns_write_ini_state; then
        smartdns_state_saved=1
    else
        smartdns_ini_status=$?
        logger -t "SmartDNS" "Could not atomically publish resolver state (status=$smartdns_ini_status)"
        smartdns_restore_previous_instance || :
        return "$smartdns_ini_status"
    fi
    if Change_dnsmasq start; then
        :
    else
        smartdns_dnsmasq_status=$?
        logger -t "SmartDNS" "Could not publish DNSMasq resolver directives (status=$smartdns_dnsmasq_status)"
        smartdns_restore_previous_instance || :
        return "$smartdns_dnsmasq_status"
    fi
    if [ "$sdns_redirect" = 0 ] ; then
        logger -t "SmartDNS" "SmartDNS 使用 $sdns_port 端口"
        if [ "$sdnse_enable" = 1 ] ; then
            logger -t "SmartDNS" "SmartDNS 使用 $sdnse_port 端口"
        fi
    fi
    Change_iptable
    rm -f /tmp/sdnsipset.conf
    args=""
    logger -t "SmartDNS" "SmartDNS 创建配置文件..."
    ipset -N smartdns hash:net >/dev/null
    smartdns_tmp_Conf=$(mktemp "${smartdns_Conf}.input.XXXXXX") || {
        logger -t "SmartDNS" "无法创建 SmartDNS 配置 staging 文件"
        smartdns_start_config_rollback 1
        return $?
    }
    if Get_sdns_conf; then
        :
    else
        smartdns_config_status=$?
        logger -t "SmartDNS" "SmartDNS 配置生成失败 (status=$smartdns_config_status)"
        smartdns_start_config_rollback "$smartdns_config_status"
        return $?
    fi
    for smartdns_source_conf in "$smartdns_address_Conf" "$smartdns_blacklist_Conf" "$smartdns_whitelist_Conf" "$smartdns_custom_Conf"; do
        if smartdns_config_append_filtered "$smartdns_source_conf" "$smartdns_tmp_Conf"; then
            :
        else
            smartdns_config_status=$?
            logger -t "SmartDNS" "SmartDNS 自定义配置读取失败 (file=$smartdns_source_conf, status=$smartdns_config_status)"
            smartdns_start_config_rollback "$smartdns_config_status"
            return $?
        fi
    done
    if sed -i '/my.router/d' "$smartdns_tmp_Conf"; then
        :
    else
        smartdns_config_status=$?
        logger -t "SmartDNS" "SmartDNS 配置规范化失败 (status=$smartdns_config_status)"
        smartdns_start_config_rollback "$smartdns_config_status"
        return $?
    fi
    if echo "domain-rules " "/my.router/ -c none -a $IPS4 -d no" >> "$smartdns_tmp_Conf"; then
        :
    else
        smartdns_config_status=$?
        logger -t "SmartDNS" "SmartDNS 配置写入失败 (status=$smartdns_config_status)"
        smartdns_start_config_rollback "$smartdns_config_status"
        return $?
    fi
    if smartdns_config_publish_deduplicated "$smartdns_tmp_Conf" "$smartdns_Conf"; then
        :
    else
        smartdns_config_status=$?
        logger -t "SmartDNS" "SmartDNS 配置原子发布失败 (status=$smartdns_config_status)"
        smartdns_start_config_rollback "$smartdns_config_status"
        return $?
    fi
    rm -f "$smartdns_tmp_Conf"
    if [ "$sdns_coredump" = "1" ] ; then
        args="$args -S"
    fi
    if [ "$dnsmasq_md5" != $(md5sum "$dnsmasq_Conf" | awk '{ print $1 }') ] ; then
        logger -t "SmartDNS" "正在重启 DNSmasq 进程..."
        /sbin/restart_dhcpd >/dev/null 2>&1
        dnsmasq_restart_status=$?
        if [ "$dnsmasq_restart_status" -ne 0 ]; then
            logger -t "SmartDNS" "DNSMasq 重启失败，回滚 SmartDNS 启动 (status=$dnsmasq_restart_status)"
            smartdns_start_config_rollback "$dnsmasq_restart_status"
            return $?
        fi
        logger -t "SmartDNS" "DNSmasq 进程已重启..."
    fi
    "$smartdns_Bin" -f -c "$smartdns_Conf" "$args"  &>/dev/null &
    sleep 1

    smartdns_process_state
    smartdns_state=$?
    case "$smartdns_state" in
        0) ;;
        1)
            if [ "$hosts_type" = "SmartDNS" ] ; then
                logger -t "SmartDNS" "SmartDNS 启动失败..."
                logger -t "SmartDNS" "删除 $smartdns_Conf 中 conf-file 附加去广告设置，再次启动..."
                logger -t "SmartDNS" "若启动成功，则请检查相关去广告规则格式是否符合 SmartDNS 要求？"
                sed -i '/conf-file /d' "$smartdns_Conf"
                "$smartdns_Bin" -f -c "$smartdns_Conf" "$args"  &>/dev/null &
            fi
            ;;
        *)
            logger -t "SmartDNS" "SmartDNS process state is unknown after launch; refusing cleanup"
            return 2
            ;;
    esac

    sleep 1
    smartdns_process_state
    smartdns_state=$?
    case "$smartdns_state" in
        0)
            logger -t "SmartDNS" "SmartDNS 进程已启动..."
            return 0
            ;;
        1)
            logger -t "SmartDNS" "SmartDNS 启动失败..."
            logger -t "SmartDNS" "停用 SmartDNS！请检查其端口配置及自定义配置是否匹配？"
            logger -t "SmartDNS" "恢复 DNSmasq 提供 DNS 解析..."
            nvram set sdns_enable=0
            sdns_enable=0
            Stop_smartdns
            stop_status=$?
            if [ "$stop_status" -ne 0 ]; then
                logger -t "SmartDNS" "SmartDNS cleanup could not confirm daemon absence (status=$stop_status)"
                return "$stop_status"
            fi
            if [ "$dnsmasq_md5" != $(md5sum "$dnsmasq_Conf" | awk '{ print $1 }') ] ; then
                logger -t "SmartDNS" "正在重启 DNSmasq 进程..."
                /sbin/restart_dhcpd >/dev/null 2>&1
                logger -t "SmartDNS" "DNSmasq 进程已重启..."
            fi
            return 1
            ;;
        *)
            logger -t "SmartDNS" "Cannot verify SmartDNS after launch; preserving configuration (status unknown)"
            return 2
            ;;
    esac
}

Stop_smartdns () {
    # Remove the resolver override and reload DNSMasq while SmartDNS is still
    # available. A failed reload must leave both the daemon and its retry state.
    if Change_dnsmasq stop; then
        smartdns_dnsmasq_status=0
    else
        smartdns_dnsmasq_status=$?
    fi
    if [ "$smartdns_dnsmasq_status" -ne 0 ]; then
        logger -t "SmartDNS" "DNSMasq resolver cleanup incomplete; preserving running daemon and saved state (status=$smartdns_dnsmasq_status)"
        return "$smartdns_dnsmasq_status"
    fi

    if [ "$sdns_enable" = 0 ] &&
       { [ "$smartdns_state_saved" = 1 ] ||
         [ "$dnsmasq_md5" != $(md5sum "$dnsmasq_Conf" | awk '{ print $1 }') ]; }; then
        logger -t "SmartDNS" "正在重启 DNSmasq 进程..."
        /sbin/restart_dhcpd >/dev/null 2>&1
        dnsmasq_restart_status=$?
        if [ "$dnsmasq_restart_status" -ne 0 ]; then
            logger -t "SmartDNS" "DNSMasq 重启失败；保留 SmartDNS 和状态文件以便重试 (status=$dnsmasq_restart_status)"
            return "$dnsmasq_restart_status"
        fi
        logger -t "SmartDNS" "DNSmasq 进程已重启..."
    fi

    smartdns_stop_process
    stop_status=$?
    if [ "$stop_status" -ne 0 ]; then
        logger -t "SmartDNS" "SmartDNS stop was not verified; preserving state for retry (status=$stop_status)"
        return "$stop_status"
    fi

    logger -t "SmartDNS" "结束 SmartDNS 进程..."
    if Change_adbyby; then
        :
    else
        smartdns_adbyby_status=$?
        logger -t "SmartDNS" "Adbyby DNS target transition failed; preserving SmartDNS retry state (status=$smartdns_adbyby_status)"
        return "$smartdns_adbyby_status"
    fi
    Change_iptable
    if [ "$sdns_enable" = 0 ] ; then
        rm -f "$smartdns_Ini"
        smartdns_state_saved=0
        smartdns_state_version=legacy
        logger -t "SmartDNS" "SmartDNS 服务器已停用..."
    fi
    return 0
}

Main () {
# 【调用各子函数】
    case $action in
    start)
        if [ ! -s "$smartdns_Ini" ] ; then
            logger -t "SmartDNS" "SmartDNS 正在启动..."
        fi
        Check_ss || return $?
        if [ "$sdns_adblock" = "1" ]; then
            smartdns_nvram_read_checked sdns_adblock_url || return $?
            sdns_adblock_url=$smartdns_nvram_value
            Start_AD
        fi
        Start_smartdns
        start_status=$?
        if [ "$start_status" -ne 0 ]; then
            logger -t "SmartDNS" "SmartDNS startup failed or could not be verified (status=$start_status)"
            return "$start_status"
        fi
        logger -t "SmartDNS" "SmartDNS 服务器已启动..."
        sleep 2
        echo 3 > /proc/sys/vm/drop_caches
        ;;
    stop)
        Stop_smartdns
        stop_status=$?
        if [ "$stop_status" -ne 0 ]; then
            logger -t "SmartDNS" "SmartDNS shutdown failed or could not be verified (status=$stop_status)"
            return "$stop_status"
        fi
        sleep 2
        echo 3 > /proc/sys/vm/drop_caches
        ;;
    restart)
        if [ "$sdns_enable" != "1" ]; then
            Stop_smartdns
            restart_status=$?
            if [ "$restart_status" -ne 0 ]; then
                logger -t "SmartDNS" "SmartDNS disabled restart cleanup failed (status=$restart_status)"
                return "$restart_status"
            fi
            return 0
        fi
        smartdns_nvram_read_checked adbyby_enable || return $?
        adbyby_enable=$smartdns_nvram_value
        case "$adbyby_enable" in
            1)
                smartdns_nvram_read_checked adbyby_add || return $?
                adbyby_add=$smartdns_nvram_value
                case "$adbyby_add" in
                    1) hosts_type="SmartDNS" ;;
                    0) hosts_type="DNSmasq" ;;
                    *) return 22 ;;
                esac
                ;;
            ''|0) hosts_type="0" ;;
            *) return 22 ;;
        esac
        Check_ss || return $?
        Start_smartdns
        start_status=$?
        if [ "$start_status" -ne 0 ]; then
            logger -t "SmartDNS" "SmartDNS restart failed or could not be verified (status=$start_status)"
            return "$start_status"
        fi
        logger -t "SmartDNS" "SmartDNS 服务器已重启完成.."
        sleep 2
        echo 3 > /proc/sys/vm/drop_caches
        ;;
    reset)
        [ "$sdns_enable" = "1" ] && Change_iptable
        ;;
    *)
        echo "check"
        ;;
    esac
}


if smartdns_lifecycle_lock_acquire; then
    :
else
    logger -t "SmartDNS" "Another lifecycle operation is active or its lock is unsafe"
    exit 1
fi
trap 'smartdns_lifecycle_lock_release >/dev/null 2>&1 || :' EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

if smartdns_load_runtime_nvram_state; then
    :
else
    smartdns_runtime_nvram_status=$?
    logger -t "SmartDNS" "SmartDNS runtime NVRAM snapshot failed before lifecycle transition (status=$smartdns_runtime_nvram_status)"
    exit "$smartdns_runtime_nvram_status"
fi

if Read_ini; then
    Main
else
    smartdns_ini_status=$?
    logger -t "SmartDNS" "Saved SmartDNS resolver state is invalid; preserving DNSMasq configuration (status=$smartdns_ini_status)"
    exit "$smartdns_ini_status"
fi
