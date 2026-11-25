#!/bin/sh
#20200426 chongshengB
#20210410 xumng123

# Keep persistent identity and derived network configuration private.
umask 077
PROG=/usr/bin/zerotier-one
PROGCLI=/usr/bin/zerotier-cli
PROGIDT=/usr/bin/zerotier-idtool
config_path="/etc/storage/zerotier-one"
PLANET="/etc/storage/planet"
ZT_READY_TIMEOUT=30
ZT_MAX_STATIC_ROUTES=64
ZT_PROC_ROOT=${ZT_PROC_ROOT:-/proc}
zt0=""
ip_segment=""
zerotier_prepare_config_directory() {
	if [ -L "$config_path" ] ||
	   { [ -e "$config_path" ] && [ ! -d "$config_path" ]; }; then
		logger -t "zerotier" "ZeroTier配置路径不是安全目录"
		return 1
	fi
	if ! mkdir -p "$config_path" || ! chmod 700 "$config_path"; then
		logger -t "zerotier" "无法创建/保护ZeroTier配置目录"
		return 1
	fi

	zt_network_dir="$config_path/networks.d"
	if [ -L "$zt_network_dir" ] ||
	   { [ -e "$zt_network_dir" ] && [ ! -d "$zt_network_dir" ]; }; then
		logger -t "zerotier" "ZeroTier networks.d 路径不是安全目录"
		return 1
	fi
	if ! mkdir -p "$zt_network_dir" || ! chmod 700 "$zt_network_dir"; then
		logger -t "zerotier" "无法创建/保护ZeroTier networks.d目录"
		return 1
	fi
	return 0
}

# Generate/derive identity material privately, then publish each verified file
# with a same-filesystem rename so neither destination is written in-place.
zerotier_prepare_identity() {
	(
		umask 077
		[ -d "$config_path" ] && [ ! -L "$config_path" ] || exit 1
		zt_identity_tmp_dir="$(mktemp -d "$config_path/.identity.XXXXXX")" || exit 1
		zt_identity_secret_tmp="$zt_identity_tmp_dir/identity.secret"
		zt_identity_public_tmp="$zt_identity_tmp_dir/identity.public"
		zt_identity_secret_backup="$zt_identity_tmp_dir/identity.secret.previous"
		zt_identity_public_backup="$zt_identity_tmp_dir/identity.public.previous"
		zt_identity_original_secret="$secret"
		zt_identity_secret_had_original=0
		zt_identity_public_had_original=0
		zt_identity_transaction_active=1
		zt_identity_preserve_backup=0

		# Recover either half of a partially published pair before discarding
		# the private staging directory. A missing original is restored by unlink.
		zerotier_identity_cleanup() {
			zt_identity_exit_status="$1"
			zt_identity_rollback_status=0
			if [ "$zt_identity_transaction_active" = "1" ]; then
				if [ ! -e "$zt_identity_secret_tmp" ]; then
					if [ "$zt_identity_secret_had_original" = "1" ]; then
						mv -f "$zt_identity_secret_backup" "$config_path/identity.secret" ||
							zt_identity_rollback_status=1
					else
						rm -f "$config_path/identity.secret" ||
							zt_identity_rollback_status=1
					fi
				fi
				if [ ! -e "$zt_identity_public_tmp" ]; then
					if [ "$zt_identity_public_had_original" = "1" ]; then
						mv -f "$zt_identity_public_backup" "$config_path/identity.public" ||
							zt_identity_rollback_status=1
					else
						rm -f "$config_path/identity.public" ||
							zt_identity_rollback_status=1
					fi
				fi
			fi

			if [ "$zt_identity_rollback_status" != "0" ]; then
				zt_identity_preserve_backup=1
				logger -t "zerotier" "身份文件回滚未完成；保留私有恢复目录: $zt_identity_tmp_dir"
			fi
			if [ "$zt_identity_preserve_backup" != "1" ]; then
				if ! rm -rf "$zt_identity_tmp_dir"; then
					logger -t "zerotier" "身份暂存目录清理失败，请检查: $zt_identity_tmp_dir"
					zt_identity_exit_status=1
				fi
			fi
			return "$zt_identity_exit_status"
		}
		trap 'zerotier_identity_cleanup "$?"' EXIT
		trap 'exit 1' HUP INT TERM

		if [ -z "$secret" ]; then
			logger -t "zerotier" "设备密匙为空,正在生成密匙,请稍后..."
			if ! "$PROGIDT" generate "$zt_identity_secret_tmp" "$zt_identity_public_tmp" >/dev/null; then
				logger -t "zerotier" "ZeroTier身份密匙生成失败"
				exit 1
			fi
			if [ ! -s "$zt_identity_secret_tmp" ] || [ ! -s "$zt_identity_public_tmp" ]; then
				logger -t "zerotier" "ZeroTier身份生成结果不完整"
				exit 1
			fi
			secret="$(cat "$zt_identity_secret_tmp")" || exit 1
			[ -n "$secret" ] || exit 1
		else
			logger -t "zerotier" "找到密匙,正在写入文件,请稍后..."
			if ! printf '%s\n' "$secret" > "$zt_identity_secret_tmp"; then
				exit 1
			fi
			if ! "$PROGIDT" getpublic "$zt_identity_secret_tmp" > "$zt_identity_public_tmp"; then
				logger -t "zerotier" "ZeroTier公钥派生失败"
				exit 1
			fi
		fi

		if [ ! -s "$zt_identity_secret_tmp" ] || [ ! -s "$zt_identity_public_tmp" ]; then
			logger -t "zerotier" "ZeroTier身份文件为空"
			exit 1
		fi
		for zt_identity_name in identity.secret identity.public; do
			zt_identity_dest="$config_path/$zt_identity_name"
			if [ -L "$zt_identity_dest" ] ||
			   { [ -e "$zt_identity_dest" ] && [ ! -f "$zt_identity_dest" ]; }; then
				logger -t "zerotier" "拒绝替换非普通身份文件: $zt_identity_name"
				exit 1
			fi
		done

		# Retain same-filesystem hard links to both old files before changing
		# either destination. If links are unavailable, fail closed pre-publish.
		if [ -e "$config_path/identity.secret" ]; then
			if ! ln "$config_path/identity.secret" "$zt_identity_secret_backup"; then
				logger -t "zerotier" "无法备份原有身份密匙"
				exit 1
			fi
			zt_identity_secret_had_original=1
		fi
		if [ -e "$config_path/identity.public" ]; then
			if ! ln "$config_path/identity.public" "$zt_identity_public_backup"; then
				logger -t "zerotier" "无法备份原有身份公钥"
				exit 1
			fi
			zt_identity_public_had_original=1
		fi
		if ! chmod 600 "$zt_identity_secret_tmp" "$zt_identity_public_tmp"; then
			logger -t "zerotier" "无法限制ZeroTier身份文件权限"
			exit 1
		fi
		if ! mv -f "$zt_identity_public_tmp" "$config_path/identity.public"; then
			logger -t "zerotier" "无法发布ZeroTier公钥文件"
			exit 1
		fi
		if ! mv -f "$zt_identity_secret_tmp" "$config_path/identity.secret"; then
			logger -t "zerotier" "无法发布ZeroTier密匙文件"
			exit 1
		fi

		# Persist a newly generated secret only after both files are published.
		# On NVRAM failure, restore the original value and roll the file pair back.
		if [ -z "$zt_identity_original_secret" ]; then
			if ! nvram set zerotier_secret="$secret"; then
				logger -t "zerotier" "ZeroTier身份密匙写入NVRAM失败"
				if ! nvram set zerotier_secret="$zt_identity_original_secret" ||
				   ! nvram commit; then
					logger -t "zerotier" "ZeroTier NVRAM身份值回滚失败"
				fi
				exit 1
			fi
			if ! nvram commit; then
				logger -t "zerotier" "ZeroTier身份密匙持久化失败"
				if ! nvram set zerotier_secret="$zt_identity_original_secret" ||
				   ! nvram commit; then
					logger -t "zerotier" "ZeroTier NVRAM身份值回滚失败"
				fi
				exit 1
			fi
		fi

		zt_identity_transaction_active=0
		exit 0
	)
}

# Decode the NVRAM planet payload to a private same-directory stage and
# replace the live planet only after the decoder and file checks succeed.
zerotier_publish_planet_base64() {
	(
		umask 077
		zt_planet_target="$config_path/planet"
		if [ -L "$zt_planet_target" ] ||
		   { [ -e "$zt_planet_target" ] && [ ! -f "$zt_planet_target" ]; }; then
			logger -t "zerotier" "自定义planet目标不是安全普通文件"
			exit 1
		fi
		zt_planet_tmp="$(mktemp "$config_path/.planet.XXXXXX")" || exit 1
		zerotier_planet_cleanup() {
			if [ -n "$zt_planet_tmp" ]; then
				rm -f "$zt_planet_tmp"
			fi
			return 0
		}
		trap 'zerotier_planet_cleanup' EXIT
		trap 'exit 1' HUP INT TERM
		if ! printf '%s' "$1" | base64 -d > "$zt_planet_tmp"; then
			logger -t "zerotier" "NVRAM中的planet数据不是有效base64"
			exit 1
		fi
		if [ ! -s "$zt_planet_tmp" ]; then
			logger -t "zerotier" "解码后的planet文件为空"
			exit 1
		fi
		if [ -L "$zt_planet_target" ] ||
		   { [ -e "$zt_planet_target" ] && [ ! -f "$zt_planet_target" ]; }; then
			logger -t "zerotier" "拒绝替换非普通planet目标"
			exit 1
		fi
		if ! chmod 600 "$zt_planet_tmp"; then
			logger -t "zerotier" "无法限制planet文件权限"
			exit 1
		fi
		if ! mv -f "$zt_planet_tmp" "$zt_planet_target"; then
			logger -t "zerotier" "planet文件原子发布失败"
			exit 1
		fi
		zt_planet_tmp=""
		exit 0
	)
}

# Import an uploaded planet file with a rollback point for the persistent
# file and NVRAM value. Empty uploads explicitly remove the configured planet.
zerotier_import_planet_file() {
	(
		umask 077
		zt_planet_source="$1"
		zt_planet_target="$config_path/planet"
		if [ -L "$zt_planet_source" ] || [ ! -f "$zt_planet_source" ]; then
			logger -t "zerotier" "上传的planet必须是普通文件，且不能是符号链接"
			exit 1
		fi
		if [ -L "$zt_planet_target" ] ||
		   { [ -e "$zt_planet_target" ] && [ ! -f "$zt_planet_target" ]; }; then
			logger -t "zerotier" "持久planet目标不是安全普通文件"
			exit 1
		fi
		zt_planet_import_dir="$(mktemp -d "$config_path/.planet-import.XXXXXX")" || exit 1
		zt_planet_stage="$zt_planet_import_dir/planet.new"
		zt_planet_backup="$zt_planet_import_dir/planet.previous"
		zt_planet_encoded="$zt_planet_import_dir/planet.base64"
		zt_planet_old_value=""
		zt_planet_had_original=0
		zt_planet_delete_pending=0
		zt_planet_transaction_active=1
		zt_planet_preserve_backup=0

		zerotier_planet_import_cleanup() {
			zt_planet_exit_status="$1"
			zt_planet_rollback_status=0
			if [ "$zt_planet_transaction_active" = "1" ] &&
			   [ "$zt_planet_delete_pending" = "1" ]; then
				if [ "$zt_planet_had_original" = "1" ]; then
					if [ -e "$zt_planet_backup" ]; then
						ln -f "$zt_planet_backup" "$zt_planet_target" ||
							zt_planet_rollback_status=1
					else
						zt_planet_rollback_status=1
					fi
				else
					rm -f "$zt_planet_target" || zt_planet_rollback_status=1
				fi
			fi
			if [ "$zt_planet_rollback_status" != "0" ]; then
				zt_planet_preserve_backup=1
				logger -t "zerotier" "planet文件回滚未完成；保留私有恢复目录: $zt_planet_import_dir"
			fi
			if [ "$zt_planet_preserve_backup" != "1" ]; then
				if ! rm -rf "$zt_planet_import_dir"; then
					logger -t "zerotier" "planet恢复目录清理失败，请检查: $zt_planet_import_dir"
					zt_planet_exit_status=1
				fi
			fi
			return "$zt_planet_exit_status"
		}
		trap 'zerotier_planet_import_cleanup "$?"' EXIT
		trap 'exit 1' HUP INT TERM

		# Register cleanup before the first fallible operation after mktemp.
		zt_planet_old_value="$(nvram get_checked zerotier_planet)" || exit 1
		# Preserve the old NVRAM value for recovery if both flash commits fail.
		if ! printf '%s' "$zt_planet_old_value" > "$zt_planet_import_dir/planet.nvram.previous" ||
		   ! chmod 600 "$zt_planet_import_dir/planet.nvram.previous"; then
			logger -t "zerotier" "无法保存原有planet NVRAM恢复值"
			exit 1
		fi
		if [ -e "$zt_planet_target" ]; then
			if ! ln "$zt_planet_target" "$zt_planet_backup"; then
				logger -t "zerotier" "无法保留原有planet文件"
				exit 1
			fi
			zt_planet_had_original=1
		fi

		if [ -s "$zt_planet_source" ]; then
			# Snapshot the upload once; both the published file and NVRAM
			# payload must describe the same bytes even if the upload changes.
			if ! cp "$zt_planet_source" "$zt_planet_stage" ||
			   [ ! -s "$zt_planet_stage" ]; then
				logger -t "zerotier" "无法暂存上传的planet文件"
				exit 1
			fi
			if ! base64 "$zt_planet_stage" > "$zt_planet_encoded" ||
			   [ ! -s "$zt_planet_encoded" ]; then
				logger -t "zerotier" "无法编码暂存的planet文件"
				exit 1
			fi
			if ! chmod 600 "$zt_planet_stage"; then
				logger -t "zerotier" "无法限制planet暂存文件权限"
				exit 1
			fi
			if [ -L "$zt_planet_target" ] ||
			   { [ -e "$zt_planet_target" ] && [ ! -f "$zt_planet_target" ]; }; then
				logger -t "zerotier" "拒绝替换非普通planet目标"
				exit 1
			fi
			zt_planet_delete_pending=1
			if ! mv -f "$zt_planet_stage" "$zt_planet_target"; then
				logger -t "zerotier" "上传的planet文件发布失败"
				exit 1
			fi
			zt_planet_new_value="$(cat "$zt_planet_encoded")" || exit 1
		else
			zt_planet_delete_pending=1
			if ! rm -f "$zt_planet_target"; then
				logger -t "zerotier" "无法删除旧planet文件"
				exit 1
			fi
			zt_planet_new_value=""
		fi

		# Keep NVRAM and the file in sync; a failed set/commit restores both.
		if ! nvram set zerotier_planet="$zt_planet_new_value" ||
		   ! nvram commit; then
			logger -t "zerotier" "上传的planet配置持久化失败"
			if ! nvram set zerotier_planet="$zt_planet_old_value" ||
			   ! nvram commit; then
				# The flash value is uncertain. Keep the NVRAM snapshot and file
				# recovery link even if the EXIT trap restores the file.
				zt_planet_preserve_backup=1
				logger -t "zerotier" "planet NVRAM值回滚失败；保留私有恢复目录: $zt_planet_import_dir"
			fi
			exit 1
		fi

		zt_planet_transaction_active=0
		if ! rm -f "$zt_planet_source"; then
			logger -t "zerotier" "planet已导入，但无法删除上传暂存文件: $zt_planet_source"
		fi
		exit 0
	)
}

# Read every startup selector before allowing identity or service side effects.
# Missing keys keep the legacy empty default; backend/read/close errors abort.
zerotier_validate_network_id() {
	[ "$#" -eq 1 ] || return 1
	[ -n "$1" ] || return 0
	case "$1" in
		*[!0123456789abcdefABCDEF]*) return 1 ;;
	esac
	[ "${#1}" -eq 16 ]
}


zerotier_load_start_settings() {
	if ! zt_start_moonid="$(nvram get_checked zerotier_moonid)"; then
		logger -t "zerotier" "无法读取启动配置 zerotier_moonid"
		return 1
	fi
	if ! zt_start_secret="$(nvram get_checked zerotier_secret)"; then
		logger -t "zerotier" "无法读取启动配置 zerotier_secret"
		return 1
	fi
	if ! zt_start_moon_enable="$(nvram get_checked zerotiermoon_enable)"; then
		logger -t "zerotier" "无法读取启动配置 zerotiermoon_enable"
		return 1
	fi
	if ! zt_start_planet="$(nvram get_checked zerotier_planet)"; then
		logger -t "zerotier" "无法读取启动配置 zerotier_planet"
		return 1
	fi
	if ! zt_start_network_id="$(nvram get_checked zerotier_id)"; then
		logger -t "zerotier" "无法读取启动配置 zerotier_id"
		return 1
	fi

	# Reject invalid selectors before publishing the snapshot or stopping
	# an already-running daemon. Only the UI's explicit Moon values are valid.
	case "$zt_start_moon_enable" in
		""|0|1) ;;
		*)
			logger -t "zerotier" "ZeroTier Moon启用配置无效"
			return 1
			;;
	esac
	if ! zerotier_validate_network_id "$zt_start_network_id"; then
		logger -t "zerotier" "ZeroTier网络ID必须为空或16位十六进制"
		return 1
	fi

	# Publish only after all five reads and selector validations succeed.
	moonid="$zt_start_moonid"
	secret="$zt_start_secret"
	enablemoonserv="$zt_start_moon_enable"
	planet="$zt_start_planet"
	zt_network_id="$zt_start_network_id"
	return 0
}

start_instance() {
	cfg="$1"
	echo $cfg
	port=""
	args=""
	
	if ! zerotier_prepare_config_directory; then
		logger -t "zerotier" "无法安全准备ZeroTier配置目录"
		return 1
	fi
	if ! zerotier_prepare_identity; then
		logger -t "zerotier" "ZeroTier身份文件准备失败"
		return 1
	fi
	if [ -n "$port" ]; then
		args="$args -p$port"
	fi

	
	if [ -L "$PLANET" ] ||
	   { [ -e "$PLANET" ] && [ ! -f "$PLANET" ]; }; then
		logger -t "zerotier" "上传的planet路径不是安全普通文件"
		return 1
	fi
	if [ -f "$PLANET" ]; then
		logger -t "zerotier" "检测到上传的planet文件，正在安全导入..."
		if ! zerotier_import_planet_file "$PLANET"; then
			logger -t "zerotier" "上传的planet导入失败，取消本次启动"
			return 1
		fi
	elif [ -n "$planet" ]; then
		logger -t "zerotier" "找到NVRAM planet,正在安全写入文件,请稍后..."
		if ! zerotier_publish_planet_base64 "$planet"; then
			logger -t "zerotier" "planet文件写入失败，取消本次启动"
			return 1
		fi
	fi

	if [ -n "$enablemoonserv" ]; then
		if [ "$enablemoonserv" -eq "1" ]; then
			logger -t "zerotier" "creat moon start"
			if ! creat_moon; then
				logger -t "zerotier" "ZeroTier Moon创建失败"
				return 1
			fi
		else
			logger -t "zerotier" "remove moon start"
			if ! remove_moon; then
				logger -t "zerotier" "ZeroTier Moon清理失败"
				return 1
			fi
		fi
	fi

	if ! add_join "$zt_network_id"; then
		logger -t "zerotier" "无效的ZeroTier网络ID或无法创建加入文件"
		return 1
	fi

	$PROG $args $config_path >/dev/null 2>&1 &

	if ! rules; then
		logger -t "zerotier" "ZeroTier接口/路由未在限定时间内就绪，停止本次启动"
		if ! kill_z; then
			logger -t "zerotier" "启动失败后仍无法确认ZeroTier进程退出"
		fi
		return 1
	fi

	if [ -n "$moonid" ]; then
		if "$PROGCLI" -D"$config_path" orbit "$moonid" "$moonid"; then
			logger -t "zerotier" "orbit moonid $moonid ok!"
		else
			zt_orbit_status=$?
			logger -t "zerotier" "ZeroTier Moon orbit失败 (status=$zt_orbit_status)"
			if ! zerotier_rollback_failed_start; then
				logger -t "zerotier" "Moon orbit失败后网络清理未完成，保留恢复状态"
			fi
			return 1
		fi
	fi


}

zerotier_rollback_failed_start() {
	# The daemon must be confirmed stopped before removing its published network state.
	if ! kill_z; then
		logger -t "zerotier" "Moon orbit失败后无法确认ZeroTier进程退出"
		return 1
	fi

	zt_start_rollback_status=0
	if ! del_rules; then
		logger -t "zerotier" "Moon orbit失败后防火墙规则清理未完成"
		zt_start_rollback_status=1
	fi
	if ! zero_route "del"; then
		logger -t "zerotier" "Moon orbit失败后静态路由清理未完成"
		zt_start_rollback_status=1
	fi
	if [ "$zt_start_rollback_status" -eq 0 ] &&
	   ! zerotier_firewall_state_clear; then
		logger -t "zerotier" "Moon orbit失败后无法清除网络恢复检查点"
		zt_start_rollback_status=1
	fi
	return "$zt_start_rollback_status"
}

add_join() {
	zt_network_id="$1"
	[ -n "$zt_network_id" ] || return 0
	if ! zerotier_validate_network_id "$zt_network_id"; then
		logger -t "zerotier" "ZeroTier网络ID必须为16位十六进制"
		return 1
	fi
	zt_network_dir="$config_path/networks.d"
	if [ ! -d "$zt_network_dir" ] || [ -L "$zt_network_dir" ]; then
		logger -t "zerotier" "ZeroTier networks.d目录不可用"
		return 1
	fi
	zt_network_file="$zt_network_dir/$zt_network_id.conf"
	if [ -L "$zt_network_file" ] ||
	   { [ -e "$zt_network_file" ] && [ ! -f "$zt_network_file" ]; }; then
		logger -t "zerotier" "拒绝跟随非普通ZeroTier加入文件"
		return 1
	fi
	if ! touch "$zt_network_file" || ! chmod 600 "$zt_network_file"; then
		logger -t "zerotier" "无法创建/保护ZeroTier加入文件"
		return 1
	fi
	return 0
}


zerotier_interface_name() {
	ifconfig | awk '$1 ~ /^zt/ { sub(/:$/, "", $1); print $1; exit }'
}

zerotier_kernel_route_segment() {
	[ -n "$1" ] || return 1
	ip route | awk -v zt_if="$1" '
	{
		dev = ""
		proto = ""
		for (i = 1; i < NF; i++) {
			if ($i == "dev")
				dev = $(i + 1)
			else if ($i == "proto")
				proto = $(i + 1)
		}
		if (dev == zt_if && proto == "kernel") {
			print $1
			exit
		}
	}'
}

zerotier_wait_interface() {
	zerotier_wait=0
	while :; do
		zt0="$(zerotier_interface_name)"
		if [ -n "$zt0" ]; then
			printf '%s\n' "$zt0"
			return 0
		fi
		if [ "$zerotier_wait" -ge "$ZT_READY_TIMEOUT" ]; then
			break
		fi
		sleep 1
		zerotier_wait=$((zerotier_wait + 1))
	done
	return 1
}

zerotier_wait_route() {
	zerotier_wait=0
	while :; do
		ip_segment="$(zerotier_kernel_route_segment "$1")"
		if [ -n "$ip_segment" ]; then
			printf '%s\n' "$ip_segment"
			return 0
		fi
		if [ "$zerotier_wait" -ge "$ZT_READY_TIMEOUT" ]; then
			break
		fi
		sleep 1
		zerotier_wait=$((zerotier_wait + 1))
	done
	return 1
}

# Keep the table selector before the iptables operation. iptables accepts
# "-t nat -C CHAIN ..."; placing -t after -C is not parsed as the nat table.
zerotier_firewall_rule_action() {
	[ "$#" -ge 2 ] || return 2
	zerotier_firewall_action="$1"
	zerotier_firewall_action_table="$2"
	shift 2
	case "$zerotier_firewall_action" in
		-C|-D) ;;
		*) return 2 ;;
	esac
	if [ -n "$zerotier_firewall_action_table" ]; then
		iptables -t "$zerotier_firewall_action_table" "$zerotier_firewall_action" "$@"
	else
		iptables "$zerotier_firewall_action" "$@"
	fi
}

# iptables 1.8.7 supports -C. Treat only exit status 1 as "absent";
# command/xtables failures must not be mistaken for completed cleanup.
ZEROTIER_RULE_DELETE_LIMIT=32
zerotier_delete_firewall_rule() {
	zerotier_rule_table=""
	if [ "${1:-}" = "-t" ]; then
		[ "$#" -ge 4 ] || return 1
		zerotier_rule_table="$2"
		case "$zerotier_rule_table" in
			filter|nat|mangle|raw|security) ;;
			*) return 1 ;;
		esac
		shift 2
	fi
	[ "$#" -ge 1 ] || return 1

	zerotier_rule_delete_count=0
	while [ "$zerotier_rule_delete_count" -lt "$ZEROTIER_RULE_DELETE_LIMIT" ]; do
		if zerotier_firewall_rule_action -C "$zerotier_rule_table" "$@" >/dev/null 2>&1; then
			if ! zerotier_firewall_rule_action -D "$zerotier_rule_table" "$@" >/dev/null 2>&1; then
				logger -t "zerotier" "防火墙规则删除失败: iptables -D $*"
				return 1
			fi
			zerotier_rule_delete_count=$((zerotier_rule_delete_count + 1))
		else
			zerotier_rule_check_status=$?
			case "$zerotier_rule_check_status" in
				1) return 0 ;;
				*)
					logger -t "zerotier" "防火墙规则状态检查失败: iptables -C $* (status=$zerotier_rule_check_status)"
					return 1
					;;
			esac
		fi
	done

	# Bound duplicate cleanup. Refuse to claim success if more copies remain.
	if zerotier_firewall_rule_action -C "$zerotier_rule_table" "$@" >/dev/null 2>&1; then
		logger -t "zerotier" "防火墙规则重复数量超过清理上限: $*"
		return 1
	else
		zerotier_rule_check_status=$?
		[ "$zerotier_rule_check_status" -eq 1 ] && return 0
		logger -t "zerotier" "防火墙规则最终状态检查失败: $* (status=$zerotier_rule_check_status)"
		return 1
	fi
}

zerotier_firewall_state_save() {
	[ -n "$zt0" ] && [ -d "$config_path" ] && [ ! -L "$config_path" ] || return 1
	case "$zt0" in
		zt*) ;;
		*) return 1 ;;
	esac
	case "$zt0" in
		*[!A-Za-z0-9_.:-]*) return 1 ;;
	esac
	case "$ip_segment" in
		*[!0-9./]*) return 1 ;;
	esac

	zt_state_file="$config_path/.firewall-state"
	if [ -e "$zt_state_file" ] || [ -L "$zt_state_file" ]; then
		[ -f "$zt_state_file" ] && [ ! -L "$zt_state_file" ] || return 1
	fi
	zt_state_tmp="$(mktemp "$config_path/.firewall-state.XXXXXX")" || return 1
	if ! printf '%s\n%s\n' "$zt0" "$ip_segment" > "$zt_state_tmp"; then
		rm -f "$zt_state_tmp"
		return 1
	fi
	if ! chmod 600 "$zt_state_tmp" || ! mv -f "$zt_state_tmp" "$zt_state_file"; then
		rm -f "$zt_state_tmp"
		return 1
	fi
	return 0
}

zerotier_firewall_state_load() {
	[ -d "$config_path" ] && [ ! -L "$config_path" ] || return 1
	zt_state_file="$config_path/.firewall-state"
	[ -f "$zt_state_file" ] && [ ! -L "$zt_state_file" ] || return 1
	{
		IFS= read -r zt_state_saved_interface || return 1
		IFS= read -r zt_state_saved_segment || return 1
		if IFS= read -r zt_state_extra; then
			return 1
		fi
	} < "$zt_state_file"
	case "$zt_state_saved_interface" in
		zt*) ;;
		*) return 1 ;;
	esac
	case "$zt_state_saved_interface" in
		*[!A-Za-z0-9_.:-]*) return 1 ;;
	esac
	case "$zt_state_saved_segment" in
		*[!0-9./]*) return 1 ;;
	esac
	return 0
}

# Clear only a regular, non-symlink checkpoint after cleanup has been verified.
zerotier_firewall_state_clear() {
	[ -d "$config_path" ] && [ ! -L "$config_path" ] || return 1
	zt_state_file="$config_path/.firewall-state"
	if [ -e "$zt_state_file" ] || [ -L "$zt_state_file" ]; then
		[ -f "$zt_state_file" ] && [ ! -L "$zt_state_file" ] || return 1
		rm -f "$zt_state_file" || return 1
	fi
	return 0
}

# Replay the old interface/segment before a replacement daemon may publish
# any new network state. Failed cleanup leaves the checkpoint for the next try.
zerotier_replay_firewall_state() {
	zt_state_file="$config_path/.firewall-state"
	if [ ! -e "$zt_state_file" ] && [ ! -L "$zt_state_file" ]; then
		return 0
	fi
	if ! zerotier_firewall_state_load; then
		logger -t "zerotier" "防火墙清理状态无效，拒绝启动替代守护进程"
		return 1
	fi
	zt0="$zt_state_saved_interface"
	ip_segment="$zt_state_saved_segment"
	if ! del_rules; then
		logger -t "zerotier" "旧ZeroTier防火墙规则清理失败，保留恢复状态"
		return 1
	fi
	if ! zero_route "del"; then
		logger -t "zerotier" "旧ZeroTier静态路由清理失败，保留恢复状态"
		return 1
	fi
	if ! zerotier_firewall_state_clear; then
		logger -t "zerotier" "旧ZeroTier清理状态无法移除，拒绝启动替代守护进程"
		return 1
	fi
	zt0=""
	ip_segment=""
	return 0
}

zerotier_add_firewall_rule() {
	if iptables "$@"; then
		return 0
	fi

	logger -t "zerotier" "防火墙规则安装失败: iptables $*"
	# Remove every rule from this transaction; the rejected append may follow
	# one or more successful appends, so cleanup must cover the whole set.
	if ! del_rules; then
		logger -t "zerotier" "防火墙规则安装失败后回滚不完整"
	elif ! zerotier_firewall_state_clear; then
		logger -t "zerotier" "防火墙规则回滚后恢复状态无法移除"
	fi
	return 1
}


zerotier_read_nat_enabled() {
	if ! nat_enable="$(nvram get_checked zerotier_nat)"; then
		logger -t "zerotier" "无法读取ZeroTier NAT配置"
		return 1
	fi
	case "$nat_enable" in
		""|0) nat_enable=0 ;;
		1) ;;
		*)
			logger -t "zerotier" "无效的ZeroTier NAT配置"
			return 1
			;;
	esac
	return 0
}

zerotier_validate_static_route_count() {
	zt_route_count="$1"
	case "$zt_route_count" in
		"") zt_route_count=0 ;;
		*[!0-9]*)
			return 1
			;;
	esac
	# Normalize leading zeroes before using the shell's numeric test. Reject
	# long values before arithmetic so corrupted NVRAM cannot overflow or
	# drive an unbounded seq loop.
	while [ "${zt_route_count#0}" != "$zt_route_count" ]; do
		zt_route_count=${zt_route_count#0}
	done
	[ -n "$zt_route_count" ] || zt_route_count=0
	[ "${#zt_route_count}" -le 2 ] || return 1
	[ "$zt_route_count" -le "$ZT_MAX_STATIC_ROUTES" ] 2>/dev/null
}


zerotier_preflight_static_routes() {
	if ! zt_preflight_route_count="$(nvram get_checked zero_staticnum_x)"; then
		logger -t "zerotier" "无法读取静态路由数量配置"
		return 1
	fi
	if ! zerotier_validate_static_route_count "$zt_preflight_route_count"; then
		logger -t "zerotier" "静态路由数量配置超出允许范围 (0-$ZT_MAX_STATIC_ROUTES)"
		return 1
	fi
	[ "$zt_route_count" -gt 0 ] || return 0
	for zt_preflight_i in $(seq 1 "$zt_route_count"); do
		zt_preflight_j=$((zt_preflight_i - 1))
		if ! zt_preflight_enable="$(nvram get_checked zero_enable_x$zt_preflight_j)"; then
			logger -t "zerotier" "无法读取静态路由启用标志: $zt_preflight_j"
			return 1
		fi
		case "$zt_preflight_enable" in
			""|0) continue ;;
			1) ;;
			*)
				logger -t "zerotier" "静态路由启用标志无效: $zt_preflight_j"
				return 1
				;;
		esac
		if ! zt_preflight_ip="$(nvram get_checked zero_ip_x$zt_preflight_j)"; then
			logger -t "zerotier" "无法读取静态路由目标: $zt_preflight_j"
			return 1
		fi
		if ! zt_preflight_gateway="$(nvram get_checked zero_route_x$zt_preflight_j)"; then
			logger -t "zerotier" "无法读取静态路由网关: $zt_preflight_j"
			return 1
		fi
		if [ -z "$zt_preflight_ip" ] || [ -z "$zt_preflight_gateway" ]; then
			logger -t "zerotier" "静态路由配置不完整: $zt_preflight_j"
			return 1
		fi
	done
	return 0
}

rules() {
	zt0="$(zerotier_wait_interface)" || {
		logger -t "zerotier" "等待ZeroTier接口超时"
		return 1
	}
	if ! zerotier_read_nat_enabled; then
		return 1
	fi
	ip_segment=""
	if [ "$nat_enable" = "1" ]; then
		ip_segment="$(zerotier_wait_route "$zt0")" || {
			logger -t "zerotier" "等待ZeroTier内核路由超时"
			return 1
		}
	fi
	logger -t "zerotier" "zt interface $zt0 is started!"
	if ! zerotier_firewall_state_save; then
		logger -t "zerotier" "无法持久化ZeroTier网络清理状态，拒绝发布防火墙规则"
		return 1
	fi
	if ! del_rules; then
		logger -t "zerotier" "旧防火墙规则清理未完成，保留恢复状态并拒绝发布新规则"
		return 1
	fi
	if ! zerotier_add_firewall_rule -A INPUT -i "$zt0" -j ACCEPT; then
		return 1
	fi
	if ! zerotier_add_firewall_rule -A FORWARD -i "$zt0" -o "$zt0" -j ACCEPT; then
		return 1
	fi
	if ! zerotier_add_firewall_rule -A FORWARD -i "$zt0" -j ACCEPT; then
		return 1
	fi
	if [ "$nat_enable" = "1" ]; then
		if ! zerotier_add_firewall_rule -t nat -A POSTROUTING -o "$zt0" -j MASQUERADE; then
			return 1
		fi
		if ! zerotier_add_firewall_rule -t nat -A POSTROUTING -s "$ip_segment" -j MASQUERADE; then
			return 1
		fi
		if ! zero_route "add"; then
			zerotier_rollback_status=0
			logger -t "zerotier" "ZeroTier静态路由安装失败，回滚网络规则"
			if ! del_rules; then
				logger -t "zerotier" "静态路由安装失败后防火墙回滚不完整"
				zerotier_rollback_status=1
			fi
			if ! zero_route "del"; then
				logger -t "zerotier" "静态路由回滚未完全确认"
				zerotier_rollback_status=1
			fi
			if [ "$zerotier_rollback_status" -eq 0 ] &&
			   ! zerotier_firewall_state_clear; then
				logger -t "zerotier" "静态路由失败回滚后恢复状态无法移除"
			fi
			return 1
		fi
	fi
	return 0
}


del_rules() {
	del_rules_status=0
	if [ -z "$zt0" ] || [ -z "$ip_segment" ]; then
		if zerotier_firewall_state_load; then
			[ -n "$zt0" ] || zt0="$zt_state_saved_interface"
			[ -n "$ip_segment" ] || ip_segment="$zt_state_saved_segment"
		fi
	fi
	if [ -z "$zt0" ]; then
		logger -t "zerotier" "无法确定ZeroTier接口，跳过无目标的防火墙删除"
		return 0
	fi
	if [ -z "$ip_segment" ]; then
		ip_segment="$(zerotier_kernel_route_segment "$zt0")"
	fi

	if ! zerotier_delete_firewall_rule INPUT -i "$zt0" -j ACCEPT; then
		del_rules_status=1
	fi
	if ! zerotier_delete_firewall_rule FORWARD -i "$zt0" -o "$zt0" -j ACCEPT; then
		del_rules_status=1
	fi
	if ! zerotier_delete_firewall_rule FORWARD -i "$zt0" -j ACCEPT; then
		del_rules_status=1
	fi
	if ! zerotier_delete_firewall_rule -t nat POSTROUTING -o "$zt0" -j MASQUERADE; then
		del_rules_status=1
	fi
	if [ -n "$ip_segment" ]; then
		if ! zerotier_delete_firewall_rule -t nat POSTROUTING -s "$ip_segment" -j MASQUERADE; then
			del_rules_status=1
		fi
	fi
	return "$del_rules_status"
}


zero_route_entry_matches() {
	awk -v destination="$1" -v gateway="$2" -v interface="$3" '
		$1 == destination {
			via = ""
			dev = ""
			for (i = 2; i < NF; i++) {
				if ($i == "via")
					via = $(i + 1)
				else if ($i == "dev")
					dev = $(i + 1)
			}
			if (via == gateway && dev == interface)
				found = 1
		}
		END { exit found ? 0 : 1 }
	'
}

zero_route() {
	if ! rulesnum="$(nvram get_checked zero_staticnum_x)"; then
		logger -t "zerotier" "无法读取静态路由数量配置"
		return 1
	fi
	case "$1" in
		add|del) ;;
		*) return 2 ;;
	esac
	if ! zerotier_validate_static_route_count "$rulesnum"; then
		logger -t "zerotier" "静态路由数量配置超出允许范围 (0-$ZT_MAX_STATIC_ROUTES)"
		return 1
	fi
	[ "$zt_route_count" -gt 0 ] || return 0
	route_status=0
	for i in $(seq 1 "$zt_route_count")
	do
		j=$((i - 1))
		if ! route_enable="$(nvram get_checked zero_enable_x$j)"; then
			logger -t "zerotier" "无法读取静态路由启用标志: $j"
			if [ "$1" = "add" ]; then
				return 1
			fi
			route_status=1
			continue
		fi
		case "$route_enable" in
			""|0) continue ;;
			1) ;;
			*)
				logger -t "zerotier" "静态路由启用标志无效: $j"
				if [ "$1" = "add" ]; then
					return 1
				fi
				route_status=1
				continue
				;;
		esac
		if ! zero_ip="$(nvram get_checked zero_ip_x$j)"; then
			logger -t "zerotier" "无法读取静态路由目标: $j"
			if [ "$1" = "add" ]; then
				return 1
			fi
			route_status=1
			continue
		fi
		if ! zero_route="$(nvram get_checked zero_route_x$j)"; then
			logger -t "zerotier" "无法读取静态路由网关: $j"
			if [ "$1" = "add" ]; then
				return 1
			fi
			route_status=1
			continue
		fi
		if [ -z "$zero_ip" ] || [ -z "$zero_route" ] || [ -z "$zt0" ]; then
			logger -t "zerotier" "静态路由配置不完整: $j"
			if [ "$1" = "add" ]; then
				return 1
			fi
			route_status=1
			continue
		fi
		if [ "$1" = "add" ]; then
			if ! ip route add "$zero_ip" via "$zero_route" dev "$zt0"; then
				logger -t "zerotier" "静态路由安装失败: $zero_ip via $zero_route"
				return 1
			fi
			echo "$zt0"
			continue
		fi

		# Treat an already-absent route as an idempotent success, but do not
		# discard inspection or deletion failures: stop must retain identity
		# until every configured route is confirmed absent.
		if ! route_output="$(ip route show "$zero_ip" 2>/dev/null)"; then
			logger -t "zerotier" "无法检查静态路由: $zero_ip"
			route_status=1
			continue
		fi
		if printf '%s\n' "$route_output" | zero_route_entry_matches "$zero_ip" "$zero_route" "$zt0"; then
			if ! ip route del "$zero_ip" via "$zero_route" dev "$zt0" >/dev/null 2>&1; then
				logger -t "zerotier" "静态路由删除失败: $zero_ip via $zero_route"
				route_status=1
				continue
			fi
			route_output="$(ip route show "$zero_ip" 2>/dev/null)" || {
				logger -t "zerotier" "无法确认静态路由已删除: $zero_ip"
				route_status=1
				continue
			}
			if printf '%s\n' "$route_output" | zero_route_entry_matches "$zero_ip" "$zero_route" "$zt0"; then
				logger -t "zerotier" "静态路由删除后仍然存在: $zero_ip via $zero_route"
				route_status=1
			else
				route_match_status=$?
				if [ "$route_match_status" -ne 1 ]; then
					logger -t "zerotier" "无法解析静态路由删除后的状态: $zero_ip"
					route_status=1
				fi
			fi
		else
			route_match_status=$?
			if [ "$route_match_status" -ne 1 ]; then
				logger -t "zerotier" "无法解析静态路由状态: $zero_ip"
				route_status=1
			fi
		fi
	done
	return "$route_status"
}

start_zero() {
	logger -t "zerotier" "正在启动zerotier"
	if ! zerotier_load_start_settings; then
		logger -t "zerotier" "无法读取ZeroTier启动配置，取消本次启动"
		return 1
	fi
	if ! zerotier_read_nat_enabled; then
		logger -t "zerotier" "无法读取NAT策略，保留当前ZeroTier服务"
		return 1
	fi
	if ! zerotier_preflight_static_routes; then
		logger -t "zerotier" "无法确认静态路由配置，保留当前ZeroTier服务"
		return 1
	fi
	if ! kill_z; then
		logger -t "zerotier" "旧进程未能确认退出，取消启动"
		return 1
	fi
	if ! zerotier_replay_firewall_state; then
		logger -t "zerotier" "旧网络状态尚未清理，取消新守护进程启动"
		return 1
	fi
	start_instance 'zerotier'
}
# Match only the daemon executable started by this launcher. pidof/killall
# operate on the mutable comm name and can signal unrelated same-name processes.
zerotier_process_matches() {
	zt_match_pid="$1"
	case "$zt_match_pid" in
		''|0|1|*[!0-9]*) return 1 ;;
	esac
	zt_match_proc_dir="$ZT_PROC_ROOT/$zt_match_pid"
	[ -d "$zt_match_proc_dir" ] || return 1
	zt_match_exe_path="$zt_match_proc_dir/exe"
	if [ ! -L "$zt_match_exe_path" ]; then
		[ -d "$zt_match_proc_dir" ] || return 1
		if zt_match_comm=$(cat "$zt_match_proc_dir/comm" 2>/dev/null); then
			[ "$zt_match_comm" = "zerotier-one" ] && return 2
			return 1
		fi
		return 2
	fi
	if zt_match_exe=$(readlink "$zt_match_exe_path" 2>/dev/null); then
		:
	else
		[ -d "$zt_match_proc_dir" ] || return 1
		return 2
	fi
	case "$zt_match_exe" in
		"$PROG"|"$PROG (deleted)") ;;
		*) return 1 ;;
	esac
	zt_match_cmdline="$zt_match_proc_dir/cmdline"
	if [ ! -f "$zt_match_cmdline" ] || [ ! -r "$zt_match_cmdline" ]; then
		[ -d "$zt_match_proc_dir" ] || return 1
		return 2
	fi
	zt_match_cmdline_text=$(tr '\000' '\n' < "$zt_match_cmdline" 2>/dev/null) || {
		[ -d "$zt_match_proc_dir" ] || return 1
		return 2
	}
	[ -n "$zt_match_cmdline_text" ] || return 2
	zt_match_argv0=$(printf '%s\n' "$zt_match_cmdline_text" | sed -n '1p') || return 2
	zt_match_argv1=$(printf '%s\n' "$zt_match_cmdline_text" | sed -n '2p') || return 2
	[ "$zt_match_argv0" = "$PROG" ] && [ "$zt_match_argv1" = "$config_path" ] || return 1
	return 0
}

# Return newline-separated exact executable PIDs. Incomplete procfs metadata
# is UNKNOWN (2), never proof that the managed daemon is absent.
zerotier_process_pids() {
	[ -d "$ZT_PROC_ROOT" ] && [ ! -L "$ZT_PROC_ROOT" ] &&
		[ -r "$ZT_PROC_ROOT" ] && [ -x "$ZT_PROC_ROOT" ] || return 2
	for zt_proc_dir in "$ZT_PROC_ROOT"/[0-9]*; do
		[ -d "$zt_proc_dir" ] || continue
		zt_scan_pid=$(printf '%s\n' "$zt_proc_dir" | sed 's|.*/||')
		case "$zt_scan_pid" in
			''|*[!0-9]*) continue ;;
		esac
		[ "$zt_scan_pid" -gt 1 ] 2>/dev/null || continue
		if zerotier_process_matches "$zt_scan_pid"; then
			printf '%s\n' "$zt_scan_pid"
		else
			zt_scan_status=$?
			case "$zt_scan_status" in
				1) ;;
				*) return 2 ;;
			esac
		fi
	done
	return 0
}

zerotier_process_state() {
	if zt_process_pids=$(zerotier_process_pids); then
		:
	else
		return $?
	fi
	[ -n "$zt_process_pids" ] && return 0
	return 1
}

# Re-enumerate and revalidate every PID immediately before signalling it.
# A stale PID or unreadable executable identity is never signalled.
zerotier_signal_all() {
	zt_signal="$1"
	case "$zt_signal" in TERM|KILL) ;; *) return 1 ;; esac
	zt_signal_pids=$(zerotier_process_pids) || return $?
	for zt_signal_pid in $zt_signal_pids; do
		if zerotier_process_matches "$zt_signal_pid"; then
			if ! kill "-$zt_signal" "$zt_signal_pid" >/dev/null 2>&1; then
				[ -d "$ZT_PROC_ROOT/$zt_signal_pid" ] || continue
				return 2
			fi
		else
			zt_signal_match_status=$?
			case "$zt_signal_match_status" in
				1) continue ;;
				*) return 2 ;;
			esac
		fi
	done
	return 0
}

zerotier_wait_stopped() {
	zerotier_wait=0
	while [ "$zerotier_wait" -lt 5 ]; do
		if zerotier_process_state; then
			zerotier_state=0
		else
			zerotier_state=$?
		fi
		case "$zerotier_state" in
			0)
				sleep 1
				zerotier_wait=$((zerotier_wait + 1))
				;;
			1)
				return 0
				;;
			*)
				return 2
				;;
		esac
	done
	if zerotier_process_state; then
		return 1
	else
		zerotier_state=$?
	fi
	case "$zerotier_state" in
		1) return 0 ;;
		2) return 2 ;;
		*) return 1 ;;
	esac
}

kill_z() {
	if zerotier_process_state; then
		zerotier_state=0
	else
		zerotier_state=$?
	fi
	case "$zerotier_state" in
		1) return 0 ;;
		2)
			logger -t "zerotier" "无法读取ZeroTier进程身份，拒绝发送信号"
			return 1
			;;
	esac

	logger -t "zerotier" "关闭进程..."
	zerotier_signal_all TERM || {
		logger -t "zerotier" "TERM阶段ZeroTier进程身份变为未知，停止升级信号。"
		return 1
	}
	if zerotier_wait_stopped; then
		return 0
	else
		zerotier_stop_state=$?
	fi
	if [ "$zerotier_stop_state" -ne 1 ]; then
		logger -t "zerotier" "停止期间进程身份变为未知，拒绝升级信号"
		return 1
	fi

	logger -t "zerotier" "进程未响应TERM，尝试强制停止..."
	zerotier_signal_all KILL || {
		logger -t "zerotier" "SIGKILL阶段ZeroTier进程身份变为未知。"
		return 1
	}
	if zerotier_wait_stopped; then
		return 0
	else
		zerotier_stop_state=$?
	fi
	case "$zerotier_stop_state" in
		1)
			logger -t "zerotier" "ZeroTier守护进程在SIGKILL后仍运行"
			return 1
			;;
		*)
			logger -t "zerotier" "SIGKILL后无法确认ZeroTier进程状态"
			return 1
			;;
	esac
}

stop_zero() {
	# Capture the virtual interface while zerotier-one still owns it. The
	# interface can disappear as soon as the daemon exits, but both firewall
	# and static-route cleanup must use the original interface name.
	if [ -z "$zt0" ]; then
		zt0="$(zerotier_interface_name)"
	fi
	ip_segment="$(zerotier_kernel_route_segment "$zt0")"
	if [ -z "$zt0" ] || [ -z "$ip_segment" ]; then
		zt_state_file="$config_path/.firewall-state"
		if [ -e "$zt_state_file" ] || [ -L "$zt_state_file" ]; then
			# A saved source-NAT segment may be the only remaining way to
			# remove a rule after the kernel route has disappeared. Never
			# overwrite an unreadable checkpoint with an empty live snapshot.
			if ! zerotier_firewall_state_load; then
				logger -t "zerotier" "ZeroTier防火墙恢复检查点无效，拒绝停止并保留现有网络状态"
				return 1
			fi
			[ -n "$zt0" ] || zt0="$zt_state_saved_interface"
			[ -n "$ip_segment" ] || ip_segment="$zt_state_saved_segment"
		fi
	fi
	if ! zerotier_read_nat_enabled; then
		logger -t "zerotier" "无法读取NAT策略，保留当前ZeroTier服务"
		return 1
	fi
	if ! zerotier_preflight_static_routes; then
		logger -t "zerotier" "无法确认静态路由配置，保留当前ZeroTier服务"
		return 1
	fi
	if [ "$nat_enable" = "1" ] && [ -z "$ip_segment" ]; then
		logger -t "zerotier" "无法在停止前读取ZeroTier内核路由，保留守护进程与网络配置"
		return 1
	fi
	if [ -n "$zt0" ] && ! zerotier_firewall_state_save; then
		logger -t "zerotier" "无法安全保存防火墙清理状态，拒绝停止ZeroTier"
		return 1
	fi

	# Keep the persistent identity/configuration and live network state intact
	# unless the daemon is confirmed absent.
	if ! kill_z; then
		logger -t "zerotier" "无法确认ZeroTier已停止，保留配置与网络规则"
		return 1
	fi
	if ! del_rules; then
		logger -t "zerotier" "无法确认ZeroTier防火墙规则已清理，保留清理状态与持久配置"
		return 1
	fi
	if ! zero_route "del"; then
		logger -t "zerotier" "无法确认ZeroTier静态路由已清理，保留持久身份与配置"
		return 1
	fi
	rm -rf "$config_path"
}

#创建moon节点
zerotier_moon_ipv4_is_valid() {
	[ "$#" -eq 1 ] || return 1
	printf '%s\n' "$1" | awk -F. '
		NF != 4 { exit 1 }
		{
			for (i = 1; i <= 4; ++i) {
				if ($i !~ /^[0-9][0-9]?[0-9]?$/ || $i + 0 > 255 ||
				    ($i ~ /^0[0-9]+$/))
					exit 1
			}
			if ($1 + 0 == 0 || $4 + 0 == 0)
				exit 1
		}
	'
}

zerotier_rollback_moon_create() {
	zt_moon_create_rollback_status=0
	if [ "$zt_moon_create_moon_publish_attempted" -eq 1 ]; then
		if [ "$zt_moon_create_old_moon_present" -eq 1 ]; then
			ln -f "$zt_moon_create_moon_backup" "$zt_moon_create_moon_file" || zt_moon_create_rollback_status=1
		else
			rm -f "$zt_moon_create_moon_file" || zt_moon_create_rollback_status=1
		fi
	fi
	if [ "$zt_moon_create_json_publish_attempted" -eq 1 ]; then
		if [ "$zt_moon_create_old_json_present" -eq 1 ]; then
			ln -f "$zt_moon_create_json_backup" "$zt_moon_create_json_file" || zt_moon_create_rollback_status=1
		else
			rm -f "$zt_moon_create_json_file" || zt_moon_create_rollback_status=1
		fi
	fi
	if [ "$zt_moon_create_nvram_attempted" -eq 1 ]; then
		if ! nvram set zerotiermoon_id="$zt_moon_create_old_id" || ! nvram commit; then
			logger -t "zerotier" "Moon身份NVRAM回滚失败"
			zt_moon_create_rollback_status=1
		fi
	fi
	if [ "$zt_moon_create_rollback_status" -eq 0 ]; then
		rm -rf "$zt_moon_create_stage_dir" || zt_moon_create_rollback_status=1
	fi
	if [ "$zt_moon_create_rollback_status" -ne 0 ]; then
		logger -t "zerotier" "Moon创建事务回滚不完整，保留私有备份目录: $zt_moon_create_stage_dir"
	fi
	return "$zt_moon_create_rollback_status"
}

creat_moon() {
	if ! moonip="$(nvram get_checked zerotiermoon_ip)"; then
		logger -t "zerotier" "无法读取ZeroTier Moon地址配置"
		return 1
	fi
	if ! zt_moon_create_old_id="$(nvram get_checked zerotiermoon_id)"; then
		logger -t "zerotier" "无法读取原有ZeroTier Moon身份配置"
		return 1
	fi

	if [ -z "$moonip" ]; then
		ip_addr="$(ifconfig -a ppp0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}')"
	else
		ip_addr="$moonip"
	fi
	if ! zerotier_moon_ipv4_is_valid "$ip_addr"; then
		logger -t "zerotier" "ZeroTier Moon地址不是有效IPv4地址"
		return 1
	fi

	zt_moon_create_identity_public="$config_path/identity.public"
	zt_moon_create_moon_dir="$config_path/moons.d"
	zt_moon_create_json_file="$config_path/moon.json"
	if [ -L "$config_path" ] || [ ! -d "$config_path" ] ||
	   [ -L "$zt_moon_create_identity_public" ] ||
	   [ ! -f "$zt_moon_create_identity_public" ]; then
		logger -t "zerotier" "ZeroTier Moon身份文件或配置目录不安全"
		return 1
	fi
	if [ -L "$zt_moon_create_moon_dir" ] ||
	   { [ -e "$zt_moon_create_moon_dir" ] && [ ! -d "$zt_moon_create_moon_dir" ]; }; then
		logger -t "zerotier" "ZeroTier Moon签名目录不安全"
		return 1
	fi
	if [ -L "$zt_moon_create_json_file" ] ||
	   { [ -e "$zt_moon_create_json_file" ] && [ ! -f "$zt_moon_create_json_file" ]; }; then
		logger -t "zerotier" "ZeroTier Moon JSON目标不安全"
		return 1
	fi
	if ! mkdir -p "$zt_moon_create_moon_dir" || ! chmod 700 "$zt_moon_create_moon_dir"; then
		logger -t "zerotier" "无法创建或保护ZeroTier Moon签名目录"
		return 1
	fi

	zt_moon_create_stage_dir="$(mktemp -d "$config_path/.moon-create.XXXXXX")" || return 1
	zt_moon_create_json_stage="$zt_moon_create_stage_dir/moon.json"
	if ! "$PROGIDT" initmoon "$zt_moon_create_identity_public" > "$zt_moon_create_json_stage"; then
		logger -t "zerotier" "ZeroTier Moon JSON初始化失败"
		rm -rf "$zt_moon_create_stage_dir"
		return 1
	fi
	# The endpoint is restricted to numeric IPv4 dotted-quad above, so it
	# cannot inject sed replacement syntax. Check sed's status directly.
	if ! sed -i "s/\"stableEndpoints\"[[:space:]]*:[[:space:]]*\[\]/\"stableEndpoints\": [ \"$ip_addr\/9993\" ]/" "$zt_moon_create_json_stage"; then
		logger -t "zerotier" "ZeroTier Moon端点写入失败"
		rm -rf "$zt_moon_create_stage_dir"
		return 1
	fi
	if ! grep -F "\"$ip_addr/9993\"" "$zt_moon_create_json_stage" >/dev/null 2>&1; then
		logger -t "zerotier" "ZeroTier Moon JSON缺少已配置端点"
		rm -rf "$zt_moon_create_stage_dir"
		return 1
	fi

	zt_moon_create_id="$(awk -F '"' '/"id"[[:space:]]*:/ { print $4; exit }' "$zt_moon_create_json_stage")"
	case "$zt_moon_create_id" in
		*[!0123456789abcdefABCDEF]*)
			logger -t "zerotier" "ZeroTier Moon初始化结果包含无效身份地址"
			rm -rf "$zt_moon_create_stage_dir"
			return 1
			;;
	esac
	if [ "${#zt_moon_create_id}" -ne 10 ]; then
		logger -t "zerotier" "ZeroTier Moon初始化结果身份地址长度无效"
		rm -rf "$zt_moon_create_stage_dir"
		return 1
	fi

	zt_moon_create_moon_stage="$zt_moon_create_stage_dir/000000$zt_moon_create_id.moon"
	if ! (cd "$zt_moon_create_stage_dir" && "$PROGIDT" genmoon "$zt_moon_create_json_stage"); then
		logger -t "zerotier" "ZeroTier Moon签名生成失败"
		rm -rf "$zt_moon_create_stage_dir"
		return 1
	fi
	if [ ! -s "$zt_moon_create_moon_stage" ]; then
		logger -t "zerotier" "ZeroTier Moon签名输出不存在或为空"
		rm -rf "$zt_moon_create_stage_dir"
		return 1
	fi
	if ! chmod 600 "$zt_moon_create_json_stage" "$zt_moon_create_moon_stage"; then
		logger -t "zerotier" "无法保护ZeroTier Moon生成文件"
		rm -rf "$zt_moon_create_stage_dir"
		return 1
	fi

	zt_moon_create_moon_file="$zt_moon_create_moon_dir/000000$zt_moon_create_id.moon"
	if [ -L "$zt_moon_create_moon_file" ] ||
	   { [ -e "$zt_moon_create_moon_file" ] && [ ! -f "$zt_moon_create_moon_file" ]; }; then
		logger -t "zerotier" "ZeroTier Moon签名目标不安全"
		rm -rf "$zt_moon_create_stage_dir"
		return 1
	fi
	zt_moon_create_moon_backup="$zt_moon_create_stage_dir/moon-file.previous"
	zt_moon_create_json_backup="$zt_moon_create_stage_dir/moon-json.previous"
	zt_moon_create_old_moon_present=0
	zt_moon_create_old_json_present=0
	zt_moon_create_moon_publish_attempted=0
	zt_moon_create_json_publish_attempted=0
	zt_moon_create_nvram_attempted=0
	if [ -e "$zt_moon_create_moon_file" ]; then
		if ! ln "$zt_moon_create_moon_file" "$zt_moon_create_moon_backup"; then
			logger -t "zerotier" "无法备份现有ZeroTier Moon签名"
			rm -rf "$zt_moon_create_stage_dir"
			return 1
		fi
		zt_moon_create_old_moon_present=1
	fi
	if [ -e "$zt_moon_create_json_file" ]; then
		if ! ln "$zt_moon_create_json_file" "$zt_moon_create_json_backup"; then
			logger -t "zerotier" "无法备份现有ZeroTier Moon JSON"
			rm -rf "$zt_moon_create_stage_dir"
			return 1
		fi
		zt_moon_create_old_json_present=1
	fi

	zt_moon_create_moon_publish_attempted=1
	if ! mv -f "$zt_moon_create_moon_stage" "$zt_moon_create_moon_file"; then
		logger -t "zerotier" "ZeroTier Moon签名发布失败"
		zerotier_rollback_moon_create
		return 1
	fi
	zt_moon_create_json_publish_attempted=1
	if ! mv -f "$zt_moon_create_json_stage" "$zt_moon_create_json_file"; then
		logger -t "zerotier" "ZeroTier Moon JSON发布失败"
		zerotier_rollback_moon_create
		return 1
	fi

	zt_moon_create_nvram_attempted=1
	if ! nvram set zerotiermoon_id="$zt_moon_create_id" || ! nvram commit; then
		logger -t "zerotier" "ZeroTier Moon身份配置持久化失败"
		zerotier_rollback_moon_create
		return 1
	fi
	if ! rm -rf "$zt_moon_create_stage_dir"; then
		logger -t "zerotier" "ZeroTier Moon创建成功但临时备份目录清理失败"
		return 1
	fi
	logger -t "zerotier" "ZeroTier Moon节点创建完成: $zt_moon_create_id"
	return 0
}

zerotier_rollback_moon_remove() {
	zt_moon_remove_rollback_status=0
	if [ "$zt_moon_remove_old_file_present" -eq 1 ]; then
		ln -f "$zt_moon_remove_file_backup" "$zt_moon_remove_file" || zt_moon_remove_rollback_status=1
	else
		rm -f "$zt_moon_remove_file" || zt_moon_remove_rollback_status=1
	fi
	if [ "$zt_moon_remove_old_json_present" -eq 1 ]; then
		ln -f "$zt_moon_remove_json_backup" "$zt_moon_remove_json" || zt_moon_remove_rollback_status=1
	else
		rm -f "$zt_moon_remove_json" || zt_moon_remove_rollback_status=1
	fi
	if [ "$zt_moon_remove_nvram_attempted" -eq 1 ]; then
		if ! nvram set zerotiermoon_id="$zt_moon_remove_old_id" || ! nvram commit; then
			logger -t "zerotier" "Moon身份NVRAM回滚失败"
			zt_moon_remove_rollback_status=1
		fi
	fi
	if [ "$zt_moon_remove_rollback_status" -eq 0 ]; then
		rm -rf "$zt_moon_remove_stage_dir" || zt_moon_remove_rollback_status=1
	else
		logger -t "zerotier" "Moon清理事务回滚不完整，保留临时备份目录: $zt_moon_remove_stage_dir"
	fi
	return "$zt_moon_remove_rollback_status"
}

remove_moon() {
	if ! zt_moon_remove_old_id="$(nvram get_checked zerotiermoon_id)"; then
		logger -t "zerotier" "无法读取ZeroTier Moon身份配置"
		return 1
	fi
	[ -n "$zt_moon_remove_old_id" ] || return 0
	case "$zt_moon_remove_old_id" in
		*[!0123456789abcdefABCDEF]*)
			logger -t "zerotier" "ZeroTier Moon身份地址无效"
			return 1
			;;
	esac
	if [ "${#zt_moon_remove_old_id}" -ne 10 ]; then
		logger -t "zerotier" "ZeroTier Moon身份地址长度无效"
		return 1
	fi

	zt_moon_remove_dir="$config_path/moons.d"
	zt_moon_remove_file="$zt_moon_remove_dir/000000$zt_moon_remove_old_id.moon"
	zt_moon_remove_json="$config_path/moon.json"
	if [ -L "$config_path" ] || [ ! -d "$config_path" ] ||
	   [ -L "$zt_moon_remove_dir" ] ||
	   { [ -e "$zt_moon_remove_dir" ] && [ ! -d "$zt_moon_remove_dir" ]; } ||
	   [ -L "$zt_moon_remove_file" ] ||
	   { [ -e "$zt_moon_remove_file" ] && [ ! -f "$zt_moon_remove_file" ]; } ||
	   [ -L "$zt_moon_remove_json" ] ||
	   { [ -e "$zt_moon_remove_json" ] && [ ! -f "$zt_moon_remove_json" ]; }; then
		logger -t "zerotier" "Moon清理目标路径不安全"
		return 1
	fi

	zt_moon_remove_stage_dir="$(mktemp -d "$config_path/.moon-remove.XXXXXX")" || return 1
	chmod 700 "$zt_moon_remove_stage_dir" || { rm -rf "$zt_moon_remove_stage_dir"; return 1; }
	zt_moon_remove_file_backup="$zt_moon_remove_stage_dir/moon-file.previous"
	zt_moon_remove_json_backup="$zt_moon_remove_stage_dir/moon-json.previous"
	zt_moon_remove_old_file_present=0
	zt_moon_remove_old_json_present=0
	zt_moon_remove_nvram_attempted=0
	if [ -e "$zt_moon_remove_file" ]; then
		if ! ln "$zt_moon_remove_file" "$zt_moon_remove_file_backup"; then
			logger -t "zerotier" "无法备份Moon签名文件"
			rm -rf "$zt_moon_remove_stage_dir"
			return 1
		fi
		zt_moon_remove_old_file_present=1
	fi
	if [ -e "$zt_moon_remove_json" ]; then
		if ! ln "$zt_moon_remove_json" "$zt_moon_remove_json_backup"; then
			logger -t "zerotier" "无法备份Moon JSON"
			rm -rf "$zt_moon_remove_stage_dir"
			return 1
		fi
		zt_moon_remove_old_json_present=1
	fi

	zt_moon_remove_nvram_attempted=1
	if ! nvram set zerotiermoon_id="" || ! nvram commit; then
		logger -t "zerotier" "ZeroTier Moon身份配置清理失败"
		zerotier_rollback_moon_remove
		return 1
	fi
	if ! rm -f "$zt_moon_remove_file" "$zt_moon_remove_json"; then
		logger -t "zerotier" "ZeroTier Moon文件清理失败"
		zerotier_rollback_moon_remove
		return 1
	fi
	rm -rf "$zt_moon_remove_stage_dir" || logger -t "zerotier" "Moon清理临时目录删除失败"
	return 0
}

case $1 in
start)
	start_zero
	;;
stop)
	stop_zero
	;;
*)
	echo "check"
	#exit 0
	;;
esac