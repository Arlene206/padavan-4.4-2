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

# Copy a factory default only into a regular, non-symlink storage path.
# Stage beside the target and link without replacing a concurrent destination;
# this prevents a dangling destination symlink from redirecting a root write.
seed_storage_file() {
	copyscripts_source_path=$1
	copyscripts_destination_path=$2
	copyscripts_file_mode=$3
	copyscripts_storage_root=/etc/storage

	case "$copyscripts_destination_path" in
		"$copyscripts_storage_root"/*) ;;
		*)
			logger -t "启动脚本" "拒绝存储目录之外的默认文件目标：$copyscripts_destination_path"
			copyscripts_seed_status=1
			return 1
			;;
	esac
	if [ -L "$copyscripts_storage_root" ] || [ ! -d "$copyscripts_storage_root" ]; then
		logger -t "启动脚本" "存储目录不是可信目录：$copyscripts_storage_root"
		copyscripts_seed_status=1
		return 1
	fi

	copyscripts_parent_dir=${copyscripts_destination_path%/*}
	if [ -L "$copyscripts_parent_dir" ] || [ ! -d "$copyscripts_parent_dir" ]; then
		logger -t "启动脚本" "拒绝非目录或符号链接父路径：$copyscripts_parent_dir"
		copyscripts_seed_status=1
		return 1
	fi
	if [ -L "$copyscripts_destination_path" ]; then
		logger -t "启动脚本" "拒绝符号链接形式的默认文件目标：$copyscripts_destination_path"
		copyscripts_seed_status=1
		return 1
	fi
	if [ -e "$copyscripts_destination_path" ]; then
		if [ -f "$copyscripts_destination_path" ]; then
			return 0
		fi
		logger -t "启动脚本" "拒绝非普通文件默认目标：$copyscripts_destination_path"
		copyscripts_seed_status=1
		return 1
	fi

	copyscripts_staged_path=$(mktemp "${copyscripts_destination_path}.tmp.XXXXXX") || {
		logger -t "启动脚本" "无法创建默认文件暂存：$copyscripts_destination_path"
		copyscripts_seed_status=1
		return 1
	}
	if ! cat "$copyscripts_source_path" > "$copyscripts_staged_path"; then
		rm -f "$copyscripts_staged_path"
		logger -t "启动脚本" "默认文件复制失败：$copyscripts_destination_path"
		copyscripts_seed_status=1
		return 1
	fi
	if ! chmod "$copyscripts_file_mode" "$copyscripts_staged_path"; then
		rm -f "$copyscripts_staged_path"
		logger -t "启动脚本" "默认文件权限设置失败：$copyscripts_destination_path"
		copyscripts_seed_status=1
		return 1
	fi
	if ! ln "$copyscripts_staged_path" "$copyscripts_destination_path"; then
		if [ -L "$copyscripts_destination_path" ] || [ ! -f "$copyscripts_destination_path" ]; then
			rm -f "$copyscripts_staged_path"
			logger -t "启动脚本" "默认文件目标在发布时发生变化：$copyscripts_destination_path"
			copyscripts_seed_status=1
			return 1
		fi
		# A concurrent regular file wins; never replace operator-managed data.
	fi
	if ! rm -f "$copyscripts_staged_path"; then
		logger -t "启动脚本" "默认文件暂存清理失败：$copyscripts_staged_path"
		copyscripts_seed_status=1
		return 1
	fi
	return 0
}

# Keep unrelated boot-time script seeding intact, but report a broken trust
# store to rc.c so the failure cannot be mistaken for successful preparation.
copyscripts_ca_status=0
copyscripts_seed_status=0
if ! install_ca_trust_store; then
	copyscripts_ca_status=1
fi
seed_storage_file "/etc_ro/koolproxy_rules_script.sh" "/etc/storage/koolproxy_rules_script.sh" 755

seed_storage_file "/etc_ro/koolproxy_rules_list.sh" "/etc/storage/koolproxy_rules_list.sh" 755

seed_storage_file "/etc_ro/ad_config_script.sh" "/etc/storage/ad_config_script.sh" 755

# The persistent FRP script contains tokens and dashboard credentials.
# Keep it private on every boot, including operator-modified existing copies.
frp_script_path="/etc/storage/frp_script.sh"
if [ -L "$frp_script_path" ]; then
	logger -t "启动脚本" "拒绝符号链接形式的 FRP 配置脚本：$frp_script_path"
	copyscripts_seed_status=1
elif [ ! -e "$frp_script_path" ]; then
	if cp -f /etc_ro/frp_script.sh "$frp_script_path"; then
		if ! chmod 700 "$frp_script_path"; then
			logger -t "启动脚本" "FRP 配置脚本权限设置失败：$frp_script_path"
			copyscripts_seed_status=1
		fi
	else
		logger -t "启动脚本" "FRP 默认配置脚本复制失败：$frp_script_path"
		copyscripts_seed_status=1
	fi
elif [ ! -f "$frp_script_path" ]; then
	logger -t "启动脚本" "FRP 配置脚本路径不是普通文件：$frp_script_path"
	copyscripts_seed_status=1
elif ! chmod 700 "$frp_script_path"; then
	logger -t "启动脚本" "FRP 配置脚本权限修复失败：$frp_script_path"
	copyscripts_seed_status=1
fi
seed_storage_file "/etc_ro/caddy_script.sh" "/etc/storage/caddy_script.sh" 755

seed_storage_file "/etc_ro/smartdns_address.conf" "/etc/storage/smartdns_address.conf" 755

seed_storage_file "/etc_ro/smartdns_blacklist-ip.conf" "/etc/storage/smartdns_blacklist-ip.conf" 755

seed_storage_file "/etc_ro/smartdns_whitelist-ip.conf" "/etc/storage/smartdns_whitelist-ip.conf" 755

seed_storage_file "/etc_ro/smartdns_custom.conf" "/etc/storage/smartdns_custom.conf" 755

ddns_script_path="/etc/storage/ddns_script.sh"
if [ -L "$ddns_script_path" ]; then
	logger -t "启动脚本" "拒绝符号链接形式的 DDNS 地址获取脚本：$ddns_script_path"
	copyscripts_seed_status=1
elif [ ! -e "$ddns_script_path" ]; then
	if cp -f /etc_ro/ddns_script.sh "$ddns_script_path"; then
		if ! chmod 755 "$ddns_script_path"; then
			logger -t "启动脚本" "DDNS 默认脚本权限设置失败"
			copyscripts_seed_status=1
		fi
	else
		logger -t "启动脚本" "DDNS 默认脚本复制失败"
		copyscripts_seed_status=1
	fi
elif [ ! -f "$ddns_script_path" ]; then
	logger -t "启动脚本" "DDNS 地址获取路径不是普通文件：$ddns_script_path"
	copyscripts_seed_status=1
fi
seed_storage_file "/etc_ro/adbyby_adblack.sh" "/etc/storage/adbyby_adblack.sh" 755

seed_storage_file "/etc_ro/adbyby_adesc.sh" "/etc/storage/adbyby_adesc.sh" 755

seed_storage_file "/etc_ro/adbyby_adhost.sh" "/etc/storage/adbyby_adhost.sh" 755

seed_storage_file "/etc_ro/adbyby_host.sh" "/etc/storage/adbyby_host.sh" 755

seed_storage_file "/etc_ro/adbyby_blockip.sh" "/etc/storage/adbyby_blockip.sh" 755

seed_storage_file "/etc_ro/adbyby_rules.sh" "/etc/storage/adbyby_rules.sh" 755

seed_storage_file "/etc_ro/ss_dlink.sh" "/etc/storage/ss_dlink.sh" 755

seed_storage_file "/etc_ro/ss_dom.sh" "/etc/storage/ss_dom.sh" 755

seed_storage_file "/etc_ro/uss_dom.sh" "/etc/storage/uss_dom.sh" 755

seed_storage_file "/etc_ro/ss_ip.sh" "/etc/storage/ss_ip.sh" 755

seed_storage_file "/etc_ro/ss_lan_bip.sh" "/etc/storage/ss_lan_bip.sh" 755

seed_storage_file "/etc_ro/ss_lan_gmip.sh" "/etc/storage/ss_lan_gmip.sh" 755

seed_storage_file "/etc_ro/ss_lan_ip.sh" "/etc/storage/ss_lan_ip.sh" 755

seed_storage_file "/etc_ro/ss_wan_ip.sh" "/etc/storage/ss_wan_ip.sh" 755

mkdir -p /etc/storage/dnsmasq.oversea
seed_storage_file "/etc_ro/oversea_list.conf" "/etc/storage/dnsmasq.oversea/oversea_list.conf" 755
mkdir -p /etc/storage/gfwlist
seed_storage_file "/etc_ro/gfwlist_list.conf" "/etc/storage/gfwlist/gfwlist_list.conf" 755

if [ "$copyscripts_ca_status" -ne 0 ] || [ "$copyscripts_seed_status" -ne 0 ]; then
	exit 1
fi
exit 0
