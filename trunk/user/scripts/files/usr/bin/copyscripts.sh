#!/bin/sh
install_ca_trust_store() {
	copyscripts_cert_archive="${COPYSCRIPTS_CERT_ARCHIVE:-/etc_ro/certs.tgz}"
	copyscripts_cert_dir="${COPYSCRIPTS_CERT_DIR:-/etc/ssl}"
	copyscripts_ca_bundle="$copyscripts_cert_dir/certs/ca-certificates.crt"

	if ! mkdir -p "$copyscripts_cert_dir"; then
		logger -t "启动脚本" "CA 证书目录创建失败：$copyscripts_cert_dir"
		return 1
	fi
	if ! tar -xzf "$copyscripts_cert_archive" -C "$copyscripts_cert_dir"; then
		logger -t "启动脚本" "CA 证书归档解压失败：$copyscripts_cert_archive"
		return 1
	fi
	if [ ! -r "$copyscripts_ca_bundle" ]; then
		logger -t "启动脚本" "CA 证书包解压后不可读：$copyscripts_ca_bundle"
		return 1
	fi
	return 0
}

# Keep unrelated boot-time script seeding intact, but report a broken trust
# store to rc.c so the failure cannot be mistaken for successful preparation.
copyscripts_ca_status=0
if ! install_ca_trust_store; then
	copyscripts_ca_status=1
fi
if [ ! -f "/etc/storage/koolproxy_rules_script.sh" ] ; then
cp -rf /etc_ro/koolproxy_rules_script.sh /etc/storage/
chmod 755 "/etc/storage/koolproxy_rules_script.sh"
fi
if [ ! -f "/etc/storage/koolproxy_rules_list.sh" ] ; then
cp -rf /etc_ro/koolproxy_rules_list.sh /etc/storage/
chmod 755 "/etc/storage/koolproxy_rules_list.sh"
fi
if [ ! -f "/etc/storage/ad_config_script.sh" ] ; then
cp -rf /etc_ro/ad_config_script.sh /etc/storage/
chmod 755 "/etc/storage/ad_config_script.sh"
fi
# The persistent FRP script contains tokens and dashboard credentials.
# Keep it private on every boot, including operator-modified existing copies.
frp_script_path="/etc/storage/frp_script.sh"
if [ -L "$frp_script_path" ]; then
	logger -t "启动脚本" "拒绝符号链接形式的 FRP 配置脚本：$frp_script_path"
elif [ ! -e "$frp_script_path" ]; then
	if cp -f /etc_ro/frp_script.sh "$frp_script_path"; then
		chmod 700 "$frp_script_path" ||
			logger -t "启动脚本" "FRP 配置脚本权限设置失败：$frp_script_path"
	else
		logger -t "启动脚本" "FRP 默认配置脚本复制失败：$frp_script_path"
	fi
elif [ ! -f "$frp_script_path" ]; then
	logger -t "启动脚本" "FRP 配置脚本路径不是普通文件：$frp_script_path"
elif ! chmod 700 "$frp_script_path"; then
	logger -t "启动脚本" "FRP 配置脚本权限修复失败：$frp_script_path"
fi
if [ ! -f "/etc/storage/caddy_script.sh" ] ; then
cp -rf /etc_ro/caddy_script.sh /etc/storage/
chmod 755 "/etc/storage/caddy_script.sh"
fi
if [ ! -f "/etc/storage/smartdns_address.conf" ] ; then
cp -rf /etc_ro/smartdns_address.conf /etc/storage/
chmod 755 "/etc/storage/smartdns_address.conf"
fi
if [ ! -f "/etc/storage/smartdns_blacklist-ip.conf" ] ; then
cp -rf /etc_ro/smartdns_blacklist-ip.conf /etc/storage/
chmod 755 "/etc/storage/smartdns_blacklist-ip.conf"
fi
if [ ! -f "/etc/storage/smartdns_whitelist-ip.conf" ] ; then
cp -rf /etc_ro/smartdns_whitelist-ip.conf /etc/storage/
chmod 755 "/etc/storage/smartdns_whitelist-ip.conf"
fi
if [ ! -f "/etc/storage/smartdns_custom.conf" ] ; then
cp -rf /etc_ro/smartdns_custom.conf /etc/storage/
chmod 755 "/etc/storage/smartdns_custom.conf"
fi
ddns_script_path="/etc/storage/ddns_script.sh"
if [ -L "$ddns_script_path" ]; then
	logger -t "启动脚本" "拒绝符号链接形式的 DDNS 地址获取脚本：$ddns_script_path"
elif [ ! -e "$ddns_script_path" ]; then
	if cp -f /etc_ro/ddns_script.sh "$ddns_script_path"; then
		chmod 755 "$ddns_script_path" || logger -t "启动脚本" "DDNS 默认脚本权限设置失败"
	else
		logger -t "启动脚本" "DDNS 默认脚本复制失败"
	fi
elif [ ! -f "$ddns_script_path" ]; then
	logger -t "启动脚本" "DDNS 地址获取路径不是普通文件：$ddns_script_path"
fi
if [ ! -f "/etc/storage/adbyby_adblack.sh" ] ; then
cp -rf /etc_ro/adbyby_adblack.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_adblack.sh"
fi
if [ ! -f "/etc/storage/adbyby_adesc.sh" ] ; then
cp -rf /etc_ro/adbyby_adesc.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_adesc.sh"
fi
if [ ! -f "/etc/storage/adbyby_adhost.sh" ] ; then
cp -rf /etc_ro/adbyby_adhost.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_adhost.sh"
fi
if [ ! -f "/etc/storage/adbyby_host.sh" ] ; then
cp -rf /etc_ro/adbyby_host.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_host.sh"
fi
if [ ! -f "/etc/storage/adbyby_blockip.sh" ] ; then
cp -rf /etc_ro/adbyby_blockip.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_blockip.sh"
fi
if [ ! -f "/etc/storage/adbyby_rules.sh" ] ; then
cp -rf /etc_ro/adbyby_rules.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_rules.sh"
fi
if [ ! -f "/etc/storage/ss_dlink.sh" ] ; then
cp -rf /etc_ro/ss_dlink.sh /etc/storage/
chmod 755 "/etc/storage/ss_dlink.sh"
fi
if [ ! -f "/etc/storage/ss_dom.sh" ] ; then
cp -rf /etc_ro/ss_dom.sh /etc/storage/
chmod 755 "/etc/storage/ss_dom.sh"
fi
if [ ! -f "/etc/storage/uss_dom.sh" ] ; then
cp -rf /etc_ro/uss_dom.sh /etc/storage/
chmod 755 "/etc/storage/uss_dom.sh"
fi
if [ ! -f "/etc/storage/ss_ip.sh" ] ; then
cp -rf /etc_ro/ss_ip.sh /etc/storage/
chmod 755 "/etc/storage/ss_ip.sh"
fi
if [ ! -f "/etc/storage/ss_lan_bip.sh" ] ; then
cp -rf /etc_ro/ss_lan_bip.sh /etc/storage/
chmod 755 "/etc/storage/ss_lan_bip.sh"
fi
if [ ! -f "/etc/storage/ss_lan_gmip.sh" ] ; then
cp -rf /etc_ro/ss_lan_gmip.sh /etc/storage/
chmod 755 "/etc/storage/ss_lan_gmip.sh"
fi
if [ ! -f "/etc/storage/ss_lan_ip.sh" ] ; then
cp -rf /etc_ro/ss_lan_ip.sh /etc/storage/
chmod 755 "/etc/storage/ss_lan_ip.sh"
fi
if [ ! -f "/etc/storage/ss_wan_ip.sh" ] ; then
cp -rf /etc_ro/ss_wan_ip.sh /etc/storage/
chmod 755 "/etc/storage/ss_wan_ip.sh"
fi
if [ ! -f "/etc/storage/dnsmasq.oversea/oversea_list.conf" ] ; then
mkdir -p /etc/storage/dnsmasq.oversea
cp -rf /etc_ro/oversea_list.conf /etc/storage/dnsmasq.oversea/
chmod 755 "/etc/storage/dnsmasq.oversea/oversea_list.conf"
fi
if [ ! -f "/etc/storage/gfwlist/gfwlist_list.conf" ] ; then
mkdir -p /etc/storage/gfwlist
cp -rf /etc_ro/gfwlist_list.conf /etc/storage/gfwlist/
chmod 755 "/etc/storage/gfwlist/gfwlist_list.conf"
fi

exit "$copyscripts_ca_status"
