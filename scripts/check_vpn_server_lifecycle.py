#!/usr/bin/env python3
"""Semantic source contract for VPN server lifecycle serialization."""
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[1]
def read(path): return (ROOT/path).read_text(encoding="utf-8")
def require(ok,msg):
    if not ok: raise AssertionError(msg)

def body(src,sig):
    start=src.find(sig); require(start>=0,f"missing function: {sig}")
    brace=src.find("{",start+len(sig)); require(brace>=0,f"missing body: {sig}")
    depth=0; state="code"; i=brace
    while i<len(src):
        ch=src[i]; nxt=src[i+1] if i+1<len(src) else ""
        if state=="code":
            if ch=="/" and nxt=="*": state="comment"; i+=2; continue
            if ch=="/" and nxt=="/": state="line"; i+=2; continue
            if ch=='"': state="string"
            elif ch=="'": state="char"
            elif ch=="{": depth+=1
            elif ch=="}":
                depth-=1
                if depth==0: return src[brace+1:i]
        elif state=="comment":
            if ch=="*" and nxt=="/": state="code"; i+=2; continue
        elif state=="line":
            if ch=="\n": state="code"
        elif state in ("string","char"):
            if ch=="\\": i+=2; continue
            if (state=="string" and ch=='"') or (state=="char" and ch=="'"): state="code"
        i+=1
    raise AssertionError(f"unterminated function: {sig}")

def ordered(txt,tokens,label):
    pos=-1
    for token in tokens:
        found=txt.find(token,pos+1); require(found>=0,f"{label}: missing/out of order {token!r}")
        pos=found

vpn=read("trunk/user/rc/src/vpn_server.c")
vpn_openvpn=read("trunk/user/rc/src/vpn_openvpn.c")
cert_script=read("trunk/user/openvpn/openvpn-cert.sh")
cert_input_policy=read("trunk/user/rc/src/openvpn_cert_input_policy.h")
cert_input_policy_test=read("trunk/user/rc/src/tests/openvpn_cert_input_policy.c")
config_policy=read("trunk/user/rc/src/vpn_server_config_policy.h")
config_policy_test=read("trunk/user/rc/src/tests/vpn_server_config_policy.c")
route_status_policy=read("trunk/user/rc/src/route_status_policy.h")
route_status_test=read("trunk/user/rc/src/tests/route_status_policy.c")
rc_header=read("trunk/user/rc/src/rc.h")
lock=read("trunk/user/rc/src/vpn_server_lifecycle_lock.h")
lease_lock=read("trunk/user/rc/src/vpn_server_lease_lock.h")
lease_lock_test=read("trunk/user/rc/src/tests/vpn_server_lease_lock.c")
rc=read("trunk/user/rc/src/rc.c")
services=read("trunk/user/rc/src/services.c")
boot_services=body(services,"int\nstart_services_once(int is_ap_mode)")
require("int vpn_start_status = start_vpn_server();" in boot_services and
        "if (vpn_start_status != 0)" in boot_services and
        'logmessage(LOGNAME, "VPN server boot startup failed (status=%d)",' in boot_services,
        "boot must log VPN server startup failures instead of discarding the status")
net_ppp=read("trunk/user/rc/src/net_ppp.c")
xl2tpd_policy=read("trunk/user/rc/src/xl2tpd_start_policy.h")
xl2tpd_test=read("trunk/user/rc/src/tests/xl2tpd_start_policy.c")
l2tp_config_file=read("trunk/user/rc/src/l2tp_config_file.h")
l2tp_config_file_test=read("trunk/user/rc/src/tests/l2tp_config_file.c")
mwan=read("trunk/user/pppoe/mwan.sh")

rpl2tp_start=body(net_ppp,"static int\nstart_rpl2tp(int unit)")
require('#include "l2tp_config_file.h"' in net_ppp and
        'l2tp_config_file_publish("/etc/l2tp", "l2tp.conf"' in rpl2tp_start and
        "rpl2tp_config_write" in rpl2tp_start and
        "mkdir_if_none(" not in rpl2tp_start and
        "fopen(" not in rpl2tp_start and
        "chmod(" not in rpl2tp_start,
        "rp-l2tp must use the no-follow atomic configuration publisher")
for token in ("O_DIRECTORY", "O_NOFOLLOW", "fstat(fd, &st)",
              "st.st_uid != geteuid()", "fchmod(fd, safe_mode)",
              "path_st.st_ino != st.st_ino", "mkstemp(temporary_path)",
              "st.st_nlink != 1", "rename(temporary_path, target_path)",
              "unlink(temporary_path)"):
    require(token in l2tp_config_file,f"rp-l2tp config helper missing safety invariant {token}")
for token in ("test_symlink_directory_is_rejected",
              "test_file_symlink_is_replaced_without_touching_target",
              "test_failed_publish_preserves_existing_file_and_cleans_temp",
              "umask(0000)", "ENOSPC", "(st.st_mode & 0777) != 0644"):
    require(token in l2tp_config_file_test,f"rp-l2tp config regression missing {token}")

net_wan=read("trunk/user/rc/src/net_wan.c")
vpn_client=read("trunk/user/rc/src/vpn_client.c")
makefile=read("trunk/user/rc/src/Makefile")
workflow=read(".github/workflows/vpn-server-stability.yml")
test=read("trunk/user/rc/src/tests/vpn_server_lifecycle_lock.c")
callback_policy=read("trunk/user/rc/src/vpn_server_callback_policy.h")
callback_test=read("trunk/user/rc/src/tests/vpn_server_callback_policy.c")
client_callback_policy=read("trunk/user/rc/src/vpn_client_callback_policy.h")
client_callback_test=read("trunk/user/rc/src/tests/vpn_client_callback_policy.c")
restart_policy=read("trunk/user/rc/src/vpn_server_restart_policy.h")
restart_policy_test=read("trunk/user/rc/src/tests/vpn_server_restart_policy.c")
process_pidfile=read("trunk/user/shared/src/process_pidfile.c")
pppd_ipcp=read("trunk/user/pppd/ppp-2.4.7/pppd/ipcp.c")
pppd_main=read("trunk/user/pppd/ppp-2.4.7/pppd/main.c")
for signature,label in (
    ("ipup_vpns_main(int argc, char **argv)", "up"),
    ("ipdown_vpns_main(int argc, char **argv)", "down"),
):
    callback=body(vpn,signature)
    ordered(callback,[
        "hook_status = _eval(hook_argv, NULL, 0, NULL);",
        "if (hook_status != 0)",
        "cleanup_failed = 1;",
        "return cleanup_failed ? -1 : 0;"
    ], "VPN "+label+" custom hook result")
require("test_vpn_server_custom_hook_failure_is_not_hidden" in route_status_test,
        "VPN server custom-hook failure regression missing")
require('#include "route_status_policy.h"' in vpn,
        "VPN server remote-LAN callbacks must use shared route-result policy")
require("int" + chr(10) + "vpns_route_to_remote_lan(" in vpn and
        "int vpns_route_to_remote_lan(const char *cname, char *ifname, char *gw, int add);" in rc_header,
        "VPN remote-LAN helper must expose operation status")
vpn_remote_route=body(vpn,"vpns_route_to_remote_lan(const char *cname, char *ifname, char *gw, int add)")
ordered(vpn_remote_route,[
    "if (!cname || !cname[0] || !ifname || !ifname[0]",
    "vpn_server_read_route_acl_snapshot(&snapshot)",
    "route_add(ifname, 0,",
    "route_del(ifname, 0,",
    "route_status_idempotent_add(route_status)",
    "route_status_idempotent_delete(route_status)",
    "if (route_status != 0)",
    "return route_status",
], "VPN server remote-LAN route status")
require("route_status_idempotent_add(EEXIST) == 0" in route_status_test and
        "route_status_idempotent_delete(ESRCH) == 0" in route_status_test and
        "test_vpn_server_remote_route_failure_survives_later_callback_work" in route_status_test,
        "VPN server route idempotence/error regression missing")
for signature,call,label in (
    ("ipup_vpns_main(int argc, char **argv)",
     "vpns_route_to_remote_lan(peer_name, argv[1], NULL, 1)", "up"),
    ("ipdown_vpns_main(int argc, char **argv)",
     "vpns_route_to_remote_lan(peer_name, argv[1], NULL, 0)", "down"),
):
    callback=body(vpn,signature)
    ordered(callback,[
        "route_status = "+call+";",
        "if (route_status != 0)",
        "cleanup_failed = 1;",
        "vpn_server_lifecycle_lock_release(lifecycle_fd)",
        "return cleanup_failed ? -1 : 0;"
    ], "VPN "+label+" callback route result")
require('#include "vpn_server_lifecycle_lock.h"' in vpn,"missing lifecycle lock include")
require("vpn_server.o" in makefile,"vpn_server.o missing from RC build")
for t in ("O_NOFOLLOW","O_CLOEXEC","fstat(fd, &st)","S_ISREG(st.st_mode)",
          "st.st_uid != geteuid()","st.st_nlink != 1","(st.st_mode & 077) != 0",
          "F_SETLK","VPN_SERVER_LIFECYCLE_LOCK_WAIT_SECONDS","EBUSY","EINTR","EAGAIN"):
    require(t in lock,f"lock helper missing safety property {t}")
require("unlink(" not in lock and "unlinkat(" not in lock,"stable lock inode may not be unlinked")
root_lease_clear=body(vpn,"static int\nclear_vpns_lease_file")
require('#define VPN_SERVER_OPENVPN_PID_FILE' in rc_header,
        "shared OpenVPN PID-file path is not declared in rc.h")
ordered(root_lease_clear, [
    "vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE)",
    "unlink(VPN_SERVER_OPENVPN_PID_FILE)",
    "unlink(VPN_SERVER_LEASE_FILE)",
    "vpn_server_lease_lock_release(lock_fd)",
], "root lease clear removes PID fence before snapshot")

require("#if defined(APP_OPENVPN)" in lease_lock and
        "#else /* !APP_OPENVPN */" in lease_lock,
        "PPP-only builds must retain a root-private lease lock")
for t in ('lstat("/tmp", &tmp_stat)', "tmp_stat.st_uid != (uid_t)0",
          "S_ISVTX", "O_WRONLY", "O_NOFOLLOW", "O_CLOEXEC"):
    require(t in lease_lock,f"shared lease lock directory/open safety missing {t}")
for t in ("getgrnam(SYS_GROUP_NOGROUP)", "fchown(fd, owner_uid, group_gid)",
          "VPN_SERVER_LEASE_LOCK_MODE", "O_NOFOLLOW", "O_CLOEXEC",
          "st.st_uid != owner_uid", "st.st_gid != group_gid", "F_SETLKW"):
    require(t in lease_lock,f"shared lease lock missing {t}")
require("fork()" in lease_lock_test and "select(" in lease_lock_test and
        "(st.st_mode & 07777) == 0620" in lease_lock_test and
        "setgid(vpn_group->gr_gid)" in lease_lock_test and
        "setuid(vpn_user->pw_uid)" in lease_lock_test and
        "#define APP_OPENVPN 1" in lease_lock_test and
        "mkfifo(path, 0600)" in lease_lock_test and
        "errno == ENXIO" in lease_lock_test,
        "shared lease lock ownership/contention regression incomplete")

require("attempt++ >= wait_seconds" in lock,"lock retry must be bounded")

s=body(vpn,"start_vpn_server(void)")
sp=body(vpn,"start_vpn_server_locked(void)")
t=body(vpn,"stop_vpn_server(void)")
start_gate=sp
enabled_reader=body(vpn,"vpn_server_read_enabled_status(int *enabled)")
type_reader=body(vpn,"vpn_server_read_type_status(int *type)")
ordered(start_gate,[
    "config_status = vpn_server_read_enabled_status(&enabled);",
    "if (config_status != 0)",
    "if (!enabled || get_ap_mode())",
    "config_status = vpn_server_read_type_status(&i_type);",
    "if (config_status != 0)",
    "if (clear_vpns_lease_file() != 0)"
],"VPN server fail-closed startup configuration")
require('nvram_invmatch("vpns_enable", "1")' not in start_gate and
        'nvram_get_int("vpns_type")' not in start_gate,
        "VPN startup must not collapse missing/failed NVRAM reads into implicit defaults")
require('nvram_get_checked("vpns_enable", &value)' in enabled_reader and
        "vpn_server_config_enable_status" in enabled_reader,
        "VPN startup must use the checked enable-state reader")
require('nvram_get_checked("vpns_type", &value)' in type_reader and
        "vpn_server_config_type_status" in type_reader,
        "VPN startup must distinguish a missing/default type from a backend error")
reapply_gate=body(vpn,"reapply_vpn_server_locked(void)")
ordered(reapply_gate,[
    "config_status = vpn_server_read_enabled_status(&enabled);",
    "if (config_status != 0)",
    "if (!enabled || get_ap_mode())",
    "config_status = vpn_server_read_type_status(&i_type);",
    "if (config_status != 0)"
],"VPN server fail-closed reapply configuration")
require('nvram_invmatch("vpns_enable", "1")' not in reapply_gate and
        'nvram_get_int("vpns_type")' not in reapply_gate,
        "VPN reapply must not collapse missing/failed NVRAM reads into implicit defaults")

acl_reader=body(vpn,"vpn_server_read_acl_snapshot(struct vpn_server_acl_entry *entries,")
acl_copy=body(vpn,"vpn_server_transaction_copy_text(struct nvram_transaction *transaction,")
ordered(acl_reader,[
    "nvram_transaction_begin(&transaction)",
    'nvram_transaction_get(&transaction, "vpns_num_x", &value)',
    "vpn_server_config_int_status(read_status, read_errno",
    "vpn_server_transaction_copy_text(&transaction, key",
    'nvram_transaction_get(&transaction, key, &value)',
    "nvram_transaction_end(&transaction)"
],"VPN CHAP credential transactional snapshot")
require("length >= buffer_size" in acl_copy and
        "*cursor < 0x20" in acl_copy and "*cursor == 0x7f" in acl_copy and
        "memcpy(buffer, value, length + 1)" in acl_copy,
        "VPN CHAP credentials must be bounded, copied and free of control characters")
ordered(sp,[
    "vpn_server_read_enabled_status(&enabled)",
    "vpn_server_read_type_status(&i_type)",
    "vpn_server_read_acl_snapshot(acl_entries, &acl_count)",
    "clear_vpns_lease_file()",
    "ppp_options_file_open(vpns_sec)"
],"VPN credentials must be snapshotted before destructive publication")
require('nvram_get_int("vpns_num_x")' not in sp and
        'nvram_get_int(acl_addr_var)' not in sp and
        'nvram_safe_get(acl_user_var)' not in sp and
        'nvram_safe_get(acl_pass_var)' not in sp,
        "VPN CHAP publication must not use unchecked per-row NVRAM getters")

pool_reader=body(vpn,"int\nget_vpns_pool(unsigned int *ip_v,")
ordered(pool_reader,[
    "nvram_transaction_begin(&transaction)",
    '"vpns_vuse"',
    '"lan_ipaddr"',
    '"lan_netmask"',
    "vpn_server_pool_parse_ipv4(lan_ip_text",
    "nvram_transaction_end(&transaction)"
],"VPN address-pool transactional snapshot")
require("vpn_server_pool_resolve_lan" in pool_reader and
        "vpn_server_pool_resolve_virtual" in pool_reader and
        "vpn_server_transaction_get_int" in pool_reader,
        "VPN client pool must use checked numeric reads and validated pool policies")
require("nvram_safe_get" not in pool_reader and "nvram_get_int" not in pool_reader,
        "VPN client pool must not use compatibility NVRAM getters")
ordered(sp,[
    "if (get_vpns_pool(&vaddr, &vmask, &vp_b, &vp_e) != 0)",
    "if (i_type != 1)"
],"VPN startup must validate the client pool before publishing PPTP configuration")
require("int get_vpns_pool(unsigned int *ip_v" in rc_header,
        "VPN pool API must propagate validation/read failures")

ppp_options=body(vpn,"gen_vpns_pppd_options(int vpns_type)")
ppp_snapshot_start=vpn.rfind("static int\nvpn_server_read_ppp_options_snapshot(struct vpn_server_ppp_options *options)")
require(ppp_snapshot_start>=0,"missing PPP options snapshot definition")
ppp_snapshot=body(vpn[ppp_snapshot_start:],"vpn_server_read_ppp_options_snapshot(struct vpn_server_ppp_options *options)")
ordered(ppp_snapshot,[
    "is_dhcpd_enabled_status(0, &options->dhcp_enabled)",
    "nvram_transaction_begin(&transaction)",
    '"vpns_auth"',
    '"vpns_mppe"',
    '"vpns_vuse"',
    '"vpns_mtu"',
    '"vpns_mru"',
    '"lan_ipaddr"',
    "vpn_server_pool_parse_ipv4(options->lan_ip",
    "nvram_transaction_end(&transaction)"
],"VPN PPP options checked NVRAM snapshot")
ordered(ppp_options,[
    "vpn_server_read_ppp_options_snapshot(&options)",
    "ppp_options_file_open(vpns_opt)"
],"VPN PPP options must be snapshotted before opening the options file")
require('nvram_get_int("vpns_auth")' not in ppp_options and
        'nvram_get_int("vpns_mppe")' not in ppp_options and
        'nvram_get_int("vpns_vuse")' not in ppp_options and
        'is_dhcpd_enabled(0)' not in ppp_options and
        'nvram_safe_get("lan_ipaddr")' not in ppp_options and
        'nvram_safe_get("dhcp_dns1_x")' not in ppp_options and
        'nvram_safe_get("dhcp_dns2_x")' not in ppp_options and
        'nvram_safe_get("dhcp_wins_x")' not in ppp_options and
        'nvram_safe_get_int("vpns_mtu"' not in ppp_options and
        'nvram_safe_get_int("vpns_mru"' not in ppp_options and
        'nvram_get_int("wins_enable")' not in ppp_options,
        "VPN PPP options must not use unchecked compatibility NVRAM getters")
require('is_dhcpd_enabled_status(0, &options->dhcp_enabled)' in ppp_snapshot and
        "vpn_server_transaction_get_int" in ppp_snapshot and
        "vpn_server_transaction_copy_text" in ppp_snapshot,
        "VPN PPP options snapshot must fail on checked DHCP/NVRAM read errors")
require("acl_entries[i].username" in sp and "acl_entries[i].password" in sp and
        "acl_entries[i].address" in sp,
        "VPN CHAP secrets must be emitted from the validated credential snapshot")
for marker in (
    "test_enable_missing_key_is_disabled",
    "test_enable_requires_exact_one",
    "test_enable_backend_error_fails_closed",
    "test_type_missing_key_preserves_pptp_default",
    "test_type_accepts_only_supported_modes",
    "test_type_rejects_malformed_and_unknown_values",
    "test_type_backend_error_fails_closed",
    "test_int_missing_and_empty_values_use_default",
    "test_int_accepts_only_complete_bounded_decimal",
    "test_int_rejects_malformed_and_out_of_range_values",
    "test_int_backend_error_fails_closed",
    "test_pool_ipv4_parser_is_strict",
    "test_pool_lan_defaults_and_client_range",
    "test_pool_lan_small_subnet_is_clamped_safely",
    "test_pool_lan_rejects_invalid_masks_hosts_and_gateway_overlap",
    "test_pool_virtual_private_network_and_lan_separation",
):
    require(marker in config_policy_test,f"VPN config-read regression missing {marker}")
require("read_status < 0" in config_policy and "read_status == 0" in config_policy and
        'strcmp(value, "1") == 0' in config_policy and
        "strtol(value, &end, 10)" in config_policy and
        "parsed > 2" in config_policy,
        "VPN config policy must distinguish absent/error and validate type bounds")
require("vpn_server_config_int_status" in config_policy and
        "parsed < minimum || parsed > maximum" in config_policy,
        "VPN ACL integer policy must distinguish absent/error and enforce bounds")
require("vpn_server_pool_parse_ipv4" in config_policy and
        "vpn_server_pool_resolve_lan" in config_policy and
        "vpn_server_pool_resolve_virtual" in config_policy and
        "EADDRINUSE" in config_policy,
        "VPN client pool policy must validate subnets, ranges and overlap")
for path in (
    "trunk/user/rc/src/vpn_server_config_policy.h",
    "trunk/user/rc/src/tests/vpn_server_config_policy.c",
):
    push, pull_section = workflow.split("  pull_request:",1)
    pull = pull_section.split("  workflow_dispatch:",1)[0]
    require(push.count("      - '"+path+"'") == 1 and
            pull.count("      - '"+path+"'") == 1,
            f"VPN config policy workflow path missing or duplicated: {path}")
require("vpn-server-config-policy" in workflow and
        "tests/vpn_server_config_policy.c" in workflow,
        "VPN configuration gate unit test is not compiled by the VPN workflow")

tp=body(vpn,"stop_vpn_server_locked(void)")
r=body(vpn,"restart_vpn_server(void)")
a=body(vpn,"reapply_vpn_server(void)")
ap=body(vpn,"reapply_vpn_server_locked(void)")
ordered(s,["vpn_server_lifecycle_lock_acquire()","if (lock_fd < 0)","start_vpn_server_locked()","vpn_server_lifecycle_lock_release(lock_fd)"],"start")
require("return -1;" in s and "gen_vpns_pppd_options(i_type)" in sp,"start wrapper/private helper incomplete")
options_writer=body(vpn,"gen_vpns_pppd_options(int vpns_type)")
require('#include "ppp_options_file.h"' in vpn and
        "ppp_options_file_open(vpns_opt)" in options_writer and
        "ppp_options_file_finish(fp, vpns_opt)" in options_writer and
        "chmod(vpns_opt, 0644)" in options_writer and
        "return 0;" in options_writer,
        "PPP VPN options generation must use the private no-follow writer and report flush/close/permission failures")
require("if (gen_vpns_pppd_options(i_type) != 0)" in sp and
        "ppp_options_file_open(vpns_sec)" in sp and
        "ppp_options_file_finish(fp, vpns_sec)" in sp and
        "ppp_options_file_open(vpns_cfg)" in sp and
        "ppp_options_file_finish(fp, vpns_cfg)" in sp,
        "VPN server must refuse launch after any required PPP configuration publication failure")
require("test_stream_flush_reports_dev_full_failure" in
        read("trunk/user/rc/src/tests/file_write_status.c"),
        "PPP options stream close/write failure regression is missing")
ordered(t,["vpn_server_lifecycle_lock_acquire()","if (lock_fd < 0)","return -1;","stop_vpn_server_locked()","vpn_server_lifecycle_lock_release(lock_fd)"],"stop")
require("kill_services_with_pidfiles(svcs, pidfile_paths, \"pppd\", 1, 5, 1)" in tp and "kill_pidfile_s" not in tp,"private stop helper must preflight all PPP clients before signaling")
restore_helper=body(vpn,"restore_shared_xl2tpd_locked(const char *phase)")
ordered(restore_helper,[
    "if (!xl2tpd_stop_attempted_vpns)",
    "vpn_server_read_shared_xl2tpd_dependencies(",
    "vpn_server_restart_should_attempt_restore_shared_xl2tpd(",
    "safe_start_xl2tpd_locked()",
    "if (dependency_status != 0)",
    "return -1;",
    "return restore_status < 0 ? restore_status : 0;"
],"shared xl2tpd restore helper")
require("xl2tpd_stop_attempted_vpns = xl2tpd_active;" in tp and
        tp.index("xl2tpd_stop_attempted_vpns = xl2tpd_active;") < tp.index("kill_services_with_pidfiles("),
        "stop attempt must be recorded before a partial multi-target stop can fail")
ordered(r,[
    "vpn_server_lifecycle_lock_acquire()",
    "if (lock_fd < 0)",
    "stop_vpn_server_locked()",
    "if (stop_status != 0)",
    'restore_shared_xl2tpd_locked("after incomplete VPN teardown")',
    "vpn_server_lifecycle_lock_release(lock_fd)",
    "start_vpn_server_locked()",
    "restart_firewall()",
    "nfs_reload_status = reload_nfsd();",
    "if (nfs_reload_status != 0)",
    "vpn_server_restart_first_error(restart_status,",
    "nfs_reload_status)",
    'restore_shared_xl2tpd_locked("after VPN server restart")',
    "vpn_server_lifecycle_lock_release(lock_fd)",
    "return restart_status;"
],"restart")
require("return;" not in r and "return restart_status;" in r,
        "restart must return the accumulated stop/start/restore/unlock status")
marker_writer = body(vpn, "vpn_server_set_l2tp_server_marker(int enabled)")
require('nvram_set_int_temp("l2tp_srv_t", enabled)' in marker_writer and
        "vpn_server_config_write_status(api_result, saved_errno)" in marker_writer,
        "L2TP server ownership marker must normalize NVRAM write/close failures")
require(vpn.count('nvram_set_int_temp("l2tp_srv_t"') == 1,
        "L2TP marker writes must all pass through the checked setter")
require("test_runtime_marker_write_status_requires_success" in config_policy_test and
        "vpn_server_config_write_status(-1, EACCES) == EACCES" in config_policy_test and
        "vpn_server_config_write_status(-1, 0) == EIO" in config_policy_test,
        "L2TP runtime marker write failure regression is missing")

l2tp_start_marker = sp.split("if (i_type == 1) {", 1)[1]
ordered(l2tp_start_marker, [
    "marker_status = vpn_server_set_l2tp_server_marker(1);",
    "if (marker_status != 0)",
    "rollback_status = vpn_server_set_l2tp_server_marker(0);",
    "xl2tpd_status = safe_start_xl2tpd_locked();",
    "if (xl2tpd_status < 0)",
    "rollback_status = vpn_server_set_l2tp_server_marker(0);",
    "marker_status = vpn_server_set_l2tp_server_marker(0);",
    "return eval(\"/usr/sbin/pptpd\""
], "L2TP/PPTP server ownership marker publication and rollback")
require("refusing L2TP server startup: cannot publish ownership marker" in l2tp_start_marker and
        "refusing PPTP server startup: cannot clear L2TP server ownership marker" in l2tp_start_marker,
        "VPN server must refuse launch when runtime ownership marker publication fails")

ordered(tp, [
    "marker_status = vpn_server_set_l2tp_server_marker(0);",
    "if (marker_status != 0)",
    "unlink(VPNS_PPP_UP_SCRIPT)",
    "unlink(VPNS_PPP_DW_SCRIPT)",
    "clear_vpns_lease_file()",
    "if (saved_errno != 0)",
    "return -1;"
], "VPN stop must report marker failure after attempting independent cleanup")

require("int restart_vpn_server(void);" in rc_header,
        "restart status must be part of the public RC declaration")
require('#include "vpn_server_restart_policy.h"' in vpn,
        "restart error policy helper is not integrated")
require("vpn_server_restart_first_error" in restart_policy and
        "vpn_server_restart_first_error" in restart_policy_test and
        "vpn_server_restart_should_restore_shared_xl2tpd" in restart_policy and
        "test_shared_xl2tpd_restoration_decision" in restart_policy_test and
        "vpn_server_restart_should_attempt_restore_shared_xl2tpd" in restart_policy and
        "test_shared_xl2tpd_restore_is_attempted_when_dependency_read_fails" in restart_policy_test,
        "restart first-error/shared-xl2tpd read-error restore policy or regression is missing")
notifications=body(rc,"handle_notifications(void)")
require("strcmp(entry->d_name, RCN_RESTART_VPNSVR)" in notifications and
        "action_result = rc_notify_merge_action_result(action_result, restart_vpn_server());" in notifications,
        "PID1 restart entry must retain failed VPN restart notifications")
require("stop_vpn_server();" not in r and "start_vpn_server();" not in r,"restart must use private non-recursive helpers")
require("reload_nfsd();" in r,"restart lost conditional NFS reload")
ordered(a,["vpn_server_lifecycle_lock_acquire()","if (lock_fd < 0)","return -1;","result = reapply_vpn_server_locked()","vpn_server_lifecycle_lock_release(lock_fd)","return result;"],"reapply")
require("gen_vpns_pppd_options(i_type)" in ap or "restart_openvpn_server()" in ap,"reapply lost update behavior")
require("reapply_vpn_server();" not in ap,"private reapply must not recurse")

require(re.search(r'RCN_RESTART_VPNSVR[\s\S]{0,220}restart_vpn_server\s*\(\)',rc),"PID1 restart entry missing")
require(re.search(r'RCN_REAPPLY_VPNSVR[\s\S]{0,220}reapply_vpn_server\s*\(\)',rc),"PID1 reapply entry missing")
require(re.search(r'RCN_RESTART_WINS[\s\S]{0,260}reapply_vpn_server\s*\(\)',rc),"WINS reapply entry missing")
require(re.search(r'!strcmp\(base,\s*"restart_vpn_server"\)[\s\S]{0,160}restart_vpn_server\s*\(\)',rc),"direct RC applet entry missing")
require("start_vpn_server();" in body(services,"start_services_once(int is_ap_mode)"),"service start entry missing")
require('record_service_stop_status(&stop_status, "VPN server", stop_vpn_server());' in body(services,"stop_services(int stopall)"),"service stop result must be aggregated")
require("return stop_status;" in body(services,"stop_services(int stopall)"),"service stop failure must reach caller")
for p in ("trunk/user/rc/src/vpn_server.c","trunk/user/rc/src/vpn_openvpn.c",
          "trunk/user/openvpn/openvpn-cert.sh",
          "trunk/user/rc/src/vpn_server_lifecycle_lock.h",
          "trunk/user/rc/src/vpn_server_lease_lock.h",
          "trunk/user/rc/src/tests/vpn_server_lease_lock.c",
          "trunk/user/rc/src/tests/vpn_server_lifecycle_lock.c",
          "trunk/user/rc/src/vpn_server_restart_policy.h",
          "trunk/user/rc/src/tests/vpn_server_restart_policy.c","trunk/user/rc/src/rc.c",
          "trunk/user/rc/src/services.c","trunk/user/rc/src/rc.h",
          "trunk/user/pppoe/mwan.sh",
          "trunk/user/rc/src/net_ppp.c",
          "trunk/user/rc/src/vpn_client.c",
          "trunk/user/rc/src/xl2tpd_start_policy.h",
          "trunk/user/rc/src/tests/xl2tpd_start_policy.c",
          "scripts/check_vpn_server_lifecycle.py",".github/workflows/vpn-server-stability.yml"):
    require(p in workflow,f"workflow path filter missing {p}")
push_paths=workflow.split("  push:",1)[1].split("  pull_request:",1)[0]
pull_paths=workflow.split("  pull_request:",1)[1].split("  workflow_dispatch:",1)[0]
openvpn_filter="      - 'trunk/user/rc/src/vpn_openvpn.c'"
require(push_paths.count(openvpn_filter) == 1 and pull_paths.count(openvpn_filter) == 1,
        "OpenVPN production source path must appear exactly once in push and pull_request filters")
cert_filter="      - 'trunk/user/openvpn/openvpn-cert.sh'"
require(push_paths.count(cert_filter) == 1 and pull_paths.count(cert_filter) == 1,
        "OpenVPN certificate generator path must appear exactly once in push and pull_request filters")
require("fork()" in test and "errno==EBUSY" in test.replace(" ",""),"cross-process contention regression missing")
require("test_first_failure_survives_later_restore_and_unlock_errors" in restart_policy_test and
        "rc_notify_merge_action_result(0, status) == -1" in restart_policy_test,
        "VPN restart policy regression must preserve first error through notification aggregation")

# Every RC path that starts/stops the shared PPP/L2TP transport must
# participate in the same lifecycle barrier as server callbacks.
safe_start=body(net_ppp,"safe_start_xl2tpd(void)")
safe_start_locked = body(net_ppp, "safe_start_xl2tpd_locked(void)")
runtime_marker_writer = body(
    net_ppp, "ppp_l2tp_runtime_marker_set_status(const char *name, int enabled)"
)
require('nvram_set_int_temp(name, enabled)' in runtime_marker_writer and
        "xl2tpd_start_policy_write_status(api_result, saved_errno)" in runtime_marker_writer and
        '"l2tp_cli_t"' in runtime_marker_writer and
        '"l2tp_wan_t"' in runtime_marker_writer,
        "shared xl2tpd runtime marker writes must use the checked allow-listed setter")
require('nvram_set_int_temp("l2tp_cli_t"' not in net_ppp and
        'nvram_set_int_temp("l2tp_wan_t"' not in net_ppp,
        "shared xl2tpd markers must not bypass checked writes")
require("test_runtime_marker_write_status" in xl2tpd_test and
        "xl2tpd_start_policy_write_status(-1, EACCES) == EACCES" in xl2tpd_test and
        "xl2tpd_start_policy_write_status(-1, 0) == EIO" in xl2tpd_test,
        "shared xl2tpd NVRAM marker status regression is missing")
vpnc_marker = safe_start_locked.index(
    'ppp_l2tp_runtime_marker_set_status("l2tp_cli_t", 1)'
)
vpnc_launch = safe_start_locked.index('eval("/usr/sbin/xl2tpd"')
require(vpnc_marker < vpnc_launch and
        "refusing xl2tpd launch: cannot publish VPN-client ownership marker" in safe_start_locked,
        "xl2tpd must not launch without publishing VPN-client ownership")

# If the common daemon start fails, roll back a client marker published by
# this invocation even when a WAN/server caller owns the outer start request.
vpnc_start_compact = "".join(safe_start_locked.split())
vpnc_published = vpnc_start_compact.index("vpnc_marker_published=1;")
vpnc_completion = vpnc_start_compact.index(
    "presence_status=xl2tpd_start_policy_completion_result("
)
vpnc_start_failure = vpnc_start_compact.index(
    "if(presence_status<0)", vpnc_completion
)
vpnc_rollback_gate = vpnc_start_compact.index(
    "xl2tpd_start_policy_vpnc_marker_rollback_required(", vpnc_start_failure
)
vpnc_rollback_call = vpnc_start_compact.index(
    'ppp_l2tp_runtime_marker_set_status("l2tp_cli_t",0)',
    vpnc_rollback_gate
)
vpnc_failure_return = vpnc_start_compact.index(
    "return-1;", vpnc_rollback_call
)
require(vpnc_marker < vpnc_launch and
        vpnc_published < vpnc_completion < vpnc_start_failure <
        vpnc_rollback_gate < vpnc_rollback_call < vpnc_failure_return and
        "errno=saved_errno;" in
        vpnc_start_compact[vpnc_rollback_gate:vpnc_failure_return],
        "failed shared-daemon start must clear the published VPN-client marker and preserve its error")
require("test_vpnc_marker_rolls_back_only_after_failed_start" in xl2tpd_test and
        "xl2tpd_start_policy_vpnc_marker_rollback_required(1, -1)" in xl2tpd_test,
        "VPN-client marker rollback policy regressions missing")

wan_start = body(net_ppp, "launch_wan_pppd_locked(int unit, int wan_proto,")
# Compare every source-order offset in the same whitespace-normalized view.
# Mixing raw and compact offsets can invert a correct source order when tabs,
# indentation and line breaks are removed from only part of the expression.
wan_start_compact = "".join(wan_start.split())
wan_marker = wan_start_compact.index(
    'ppp_l2tp_runtime_marker_set_status("l2tp_wan_t",1)'
)
wan_published = wan_start_compact.index("wan_marker_published=1;")
wan_launch = wan_start_compact.index("safe_start_xl2tpd_locked()")
wan_launch_failure = wan_start_compact.index("if(launch_status!=0)")
wan_rollback_gate = wan_start_compact.index(
    "xl2tpd_start_policy_wan_marker_rollback_required("
)
wan_rollback_call = wan_start_compact.index(
    'ppp_l2tp_runtime_marker_set_status("l2tp_wan_t",0)', wan_rollback_gate
)
wan_failure_return = wan_start_compact.index("return-1;", wan_launch_failure)
require(wan_marker < wan_published < wan_launch < wan_launch_failure <
        wan_rollback_gate < wan_rollback_call < wan_failure_return,
        "published WAN xl2tpd dependency must be rolled back only after a failed launch")
require("refusingxl2tpdWANlaunch:cannotpublishWANownershipmarker" in
        wan_start_compact and
        "marker_rollback_status=ppp_l2tp_runtime_marker_set_status(" in
        wan_start_compact[wan_rollback_gate:wan_failure_return] and
        "errno=saved_errno;" in
        wan_start_compact[wan_rollback_gate:wan_failure_return],
        "WAN marker cleanup must preserve the original launch errno")
for marker in ("xl2tpd_start_policy_wan_marker_rollback_required(0, -1)",
               "xl2tpd_start_policy_wan_marker_rollback_required(1, 0)",
               "xl2tpd_start_policy_wan_marker_rollback_required(1, -1)"):
    require(marker in xl2tpd_test,
            "WAN marker rollback policy regression missing: " + marker)

ordered(safe_start,["vpn_server_lifecycle_lock_acquire()",
                    "safe_start_xl2tpd_locked()",
                    "vpn_server_lifecycle_lock_release(lock_fd)"],"public xl2tpd start")
require('eval("/usr/sbin/xl2tpd"' in safe_start_locked,
        "locked xl2tpd starter lost daemon launch")
require('#include <process_inventory.h>' in net_ppp and
        '#include "xl2tpd_start_policy.h"' in net_ppp,
        "xl2tpd start must use tri-state process inventory policy")

# Every NVRAM selector and optional VPN-client credential consumed by
# xl2tpd must come from one checked snapshot before process scan/config I/O.
xl2tpd_snapshot = body(
    net_ppp, "xl2tpd_read_config_snapshot(struct xl2tpd_config_snapshot *snapshot)"
)
for key in ('"vpns_enable"', '"vpns_type"', '"vpnc_enable"', '"vpnc_type"',
            '"vpnc_dgw"', '"wan_l2tpd"', '"vpnc_peer"', '"vpnc_user"', '"l2tp_cli_t'):
    require(key in xl2tpd_snapshot,
            "xl2tpd selector/credential snapshot is missing " + key)
for token in ("rc_wan_nvram_copy_snapshot(",):
    require(token in xl2tpd_snapshot,
            "xl2tpd configuration must use checked NVRAM transaction: " + token)
for adapter, token in (
    ("xl2tpd_snapshot_transaction_begin(void *context)", "nvram_transaction_begin("),
    ("xl2tpd_snapshot_transaction_get(void *context,", "nvram_transaction_get("),
    ("xl2tpd_snapshot_transaction_end(void *context)", "nvram_transaction_end("),
):
    require(token in body(net_ppp, adapter),
            "xl2tpd transaction adapter is missing " + token)
xl2tpd_writer = body(net_ppp, "xl2tpd_config_write(FILE *fp, void *opaque)")
require('#include "l2tp_config_file.h"' in net_ppp and
        'l2tp_config_file_publish("/etc", "xl2tpd.conf"' in safe_start_locked and
        "xl2tpd_config_write" in safe_start_locked and
        'fopen(l2tp_conf, "w")' not in safe_start_locked and
        "unlink(l2tp_conf)" not in safe_start_locked,
        "shared xl2tpd config must use same-directory atomic publication")
for token in ('"[global]\\n"', '"[lns default]\\n"', '"[lac %s]\\n"',
              '"[lac VPNC]\\n"', "VPN_SERVER_PPPD_OPTIONS",
              "VPN_CLIENT_PPPD_OPTIONS", "config->wan_peer",
              "config->vpnc_peer", "config->vpnc_default_route"):
    require(token in xl2tpd_writer,
            "xl2tpd atomic writer missing existing config section/value: " + token)
require("return -1;" in xl2tpd_writer and "fprintf(fp," in xl2tpd_writer,
        "xl2tpd writer must propagate formatting/write failures")
require("xl2tpd_read_config_snapshot(&config_snapshot)" in safe_start_locked and
        safe_start_locked.index("xl2tpd_read_config_snapshot(&config_snapshot)") <
        safe_start_locked.index('process_inventory_scan("xl2tpd", &process_snapshot)') <
        safe_start_locked.index('l2tp_config_file_publish("/etc", "xl2tpd.conf"'),
        "xl2tpd snapshot failures must stop before process scan and atomic config publication")
require("xl2tpd_start_policy_config_flags(" in safe_start_locked and
        "config_snapshot.vpns_enable" in safe_start_locked and
        "config_snapshot.vpnc_enable" in safe_start_locked and
        "config_snapshot.wan_l2tpd" in safe_start_locked,
        "xl2tpd mode flags must derive only from the checked snapshot")
require("names[9]" in xl2tpd_snapshot and "fields[9]" in xl2tpd_snapshot and
        "destinations[9]" in xl2tpd_snapshot and "fields, 9," in xl2tpd_snapshot,
        "xl2tpd must snapshot the existing VPN-client marker with the selectors")
for forbidden in ('nvram_match("vpns_enable"',
                  'nvram_match("vpns_type"',
                  'nvram_match("vpnc_enable"',
                  'nvram_match("vpnc_type"',
                  'nvram_invmatch("wan_l2tpd"',
                  'nvram_match("vpnc_dgw"',
                  'nvram_safe_get("vpnc_peer"',
                  'nvram_safe_get("vpnc_user"'):
    require(forbidden not in safe_start_locked,
            "xl2tpd config must not use lossy NVRAM getter: " + forbidden)
for case in ("test_config_flags",
             "xl2tpd_start_policy_config_flags",
             "nvram_invmatch", "xl2tpd process-presence and launch-result policy: PASS"):
    require(case in xl2tpd_policy + xl2tpd_test,
            "xl2tpd checked configuration policy regression is missing: " + case)
require('process_inventory_scan("xl2tpd", &process_snapshot)' in safe_start_locked and
        "process_snapshot_release(&process_snapshot)" in safe_start_locked,
        "xl2tpd must obtain and release a complete process snapshot")
require(safe_start_locked.index('process_inventory_scan("xl2tpd", &process_snapshot)') <
        safe_start_locked.index('l2tp_config_file_publish("/etc", "xl2tpd.conf"'),
        "unknown process state must be rejected before atomic config publication")
pool_compact = re.sub(r"\s+", " ", safe_start_locked)
require("get_vpns_pool(&vpn_server_address, &vpn_server_mask, &vpn_pool_start, &vpn_pool_end) != 0" in pool_compact,
        "xl2tpd must consume the checked four-output VPN pool API")
pool_gate = safe_start_locked.index("if (use_vpn_server &&")
pool_call = safe_start_locked.index("get_vpns_pool(", pool_gate)
config_publish = safe_start_locked.index('l2tp_config_file_publish("/etc", "xl2tpd.conf"')
require(pool_gate < pool_call < config_publish,
        "VPN address-pool snapshot must complete before xl2tpd config publication")
pool_failure = safe_start_locked[pool_call:config_publish]
require("saved_errno = errno ? errno : EIO;" in pool_failure and
        "errno = saved_errno;" in pool_failure and "return -1;" in pool_failure,
        "VPN address-pool read errors must fail closed before xl2tpd config publication")
require('nvram_get_int("vpns_vuse")' not in safe_start_locked and
        "get_vpns_pool(nvram_get_int" not in safe_start_locked,
        "xl2tpd must not bypass the checked VPN address-pool snapshot")
writer_compact = "".join(xl2tpd_writer.split())
require("(config->vpn_server_address&config->vpn_server_mask)|config->vpn_pool_start" in writer_compact and
        "(config->vpn_server_address&config->vpn_server_mask)|config->vpn_pool_end" in writer_compact,
        "xl2tpd address ranges must use the validated snapshot outputs")
marker_parse = safe_start_locked.index(
    "rc_wan_nvram_parse_int_value(config_snapshot.vpnc_marker, &vpnc_marker_active)")
marker_validation = safe_start_locked.index(
    "(vpnc_marker_active != 0 && vpnc_marker_active != 1)", marker_parse)
marker_policy = safe_start_locked.index(
    "xl2tpd_start_policy_vpnc_marker_publish_required(")
marker_write = safe_start_locked.index(
    'ppp_l2tp_runtime_marker_set_status("l2tp_cli_t", 1)')
marker_owned = safe_start_locked.index("vpnc_marker_published = 1;", marker_write)
require(marker_parse < marker_validation < marker_policy < marker_write < marker_owned,
        "xl2tpd must validate the snapshot marker before acquiring rollback ownership")
require("return use_vpnc && marker_active == 0;" in xl2tpd_policy and
        "vpnc_marker_publish_required(1, 1)" in xl2tpd_test,
        "existing VPNC marker must suppress redundant publication")
require("presence_status < 0" in safe_start_locked and
        "scan_error ? scan_error : EIO" in safe_start_locked,
        "unknown xl2tpd presence must not look like already-running success")
require("if (presence_status == 0)" in safe_start_locked and
        'if (!pids("xl2tpd"))' not in safe_start_locked,
        "xl2tpd must launch only after confirmed absence")
require("xl2tpd_start_policy_completion_result(presence_status" in safe_start_locked and
        "return eval(" not in safe_start_locked,
        "xl2tpd child exit status must not be confused with already-running state")
for marker in ("PROCESS_PRESENCE_RUNNING", "PROCESS_PRESENCE_ABSENT",
               "PROCESS_PRESENCE_UNKNOWN",
               "xl2tpd_start_policy_vpnc_marker_publish_required",
               "test_vpnc_marker_is_published_only_when_unowned",
               "xl2tpd_start_policy_completion_result(0, 1)",
               "xl2tpd_start_policy_completion_result(0, 127)",
               "xl2tpd_start_policy_wan_launch_result(1)",
               "xl2tpd process-presence and launch-result policy: PASS"):
    require(marker in xl2tpd_policy + xl2tpd_test,
            f"xl2tpd lifecycle result test marker missing: {marker}")
client_start=body(vpn_client,"start_vpn_client_locked(void)")
require("xl2tpd_status == 1" in client_start and "xl2tpd_status < 0" in client_start,
        "VPNC control must only target a confirmed running daemon")
server_start=body(vpn,"start_vpn_server_locked(void)")
require("xl2tpd_status < 0" in server_start,
        "L2TP server start must propagate unknown daemon state")
server_restart=body(vpn,"restart_vpn_server(void)")
shared_restore=body(vpn,"restore_shared_xl2tpd_locked(const char *phase)")
require("vpn_server_read_shared_xl2tpd_dependencies(" in shared_restore and
        "vpn_server_restart_should_attempt_restore_shared_xl2tpd(" in shared_restore and
        "restore_status = safe_start_xl2tpd_locked();" in shared_restore and
        "if (dependency_status != 0)" in shared_restore and
        "return -1;" in shared_restore and
        "return restore_status < 0 ? restore_status : 0;" in shared_restore,
        "shared xl2tpd restore must accept the already-running success sentinel and propagate dependency read errors")
require('restore_shared_xl2tpd_locked("after incomplete VPN teardown")' in server_restart and
        'restore_shared_xl2tpd_locked("after VPN server restart")' in server_restart,
        "VPN server restart must restore shared clients after abort and normal restart")
client_restart=body(vpn_client,"restart_vpn_client_locked(void)")
require("restore_status < 0" in client_restart and "start_status = -1" in client_restart,
        "VPNC restart must propagate failed shared daemon restoration")

wan_launch=body(net_ppp,"launch_wan_pppd(int unit, int wan_proto)")
wan_launch_locked=body(net_ppp,"launch_wan_pppd_locked(int unit, int wan_proto,")
snapshot_read=wan_launch.index("ppp_wan_launch_mode_snapshot_read(wan_proto, &launch_mode)")
snapshot_error=wan_launch.index("if (mode_status != 0)", snapshot_read)
single_wan=wan_launch.index("if (wan_proto != IPV4_WAN_PROTO_L2TP && !launch_mode.multiwan_enabled)")
single_fastpath=wan_launch.index("return launch_wan_pppd_locked(unit, wan_proto, &launch_mode)", single_wan)
lifecycle_lock=wan_launch.index("vpn_server_lifecycle_lock_acquire()")
locked_launch=wan_launch.index("result = launch_wan_pppd_locked(unit, wan_proto, &launch_mode)")
lifecycle_release=wan_launch.index("vpn_server_lifecycle_lock_release(lock_fd)")
require(snapshot_read < snapshot_error < single_wan < single_fastpath < lifecycle_lock < locked_launch < lifecycle_release,
        "WAN PPP selector transaction must fail closed before lock selection and all locked paths reuse its snapshot")
require("kill_services(svcs, 5, 1)" in wan_launch_locked and
        "xl2tpd_start_policy_wan_launch_result(" in wan_launch_locked,
        "WAN L2TP stop/config/start must remain in one locked helper")
require("xl2tpd_start_policy_wan_launch_result(" in wan_launch_locked and
        "safe_start_xl2tpd_locked()" in wan_launch_locked,
        "WAN L2TP launch must accept already-running xl2tpd as success")
ordered(wan_launch_locked,["launch_mode->multiwan_enabled",
                           "kill_services(svcs_pppd, 5, 1)",
                           "ppp_wan_session_generation_change(&callback_generation)",
                           'eval("/usr/bin/mwan.sh", "--rc-lifecycle-locked")'],
        "multi-WAN PPP stop before generation advance and helper launch")
require("launch_mode->use_rpl2tp" in wan_launch_locked and
        'nvram_match("pppoemwan_enable"' not in wan_launch_locked and
        'nvram_match("wan_l2tpd"' not in wan_launch_locked,
        "WAN PPP L2TP daemon and multi-WAN decisions must use the checked immutable snapshot")
require("killall pppd" not in mwan and "pidof pppd" not in mwan and
        'kill -9 "$pppd_process"' not in mwan,
        "mwan.sh must not kill pppd outside the lifecycle-locked RC caller")
require('"$1" != "--rc-lifecycle-locked"' in mwan,
        "mwan.sh must reject direct startup without the RC lifecycle handoff")

client_start=body(vpn_client,"start_vpn_client(void)")
client_start_locked=body(vpn_client,"start_vpn_client_locked(void)")
ordered(client_start,["vpn_server_lifecycle_lock_acquire()",
                      "start_vpn_client_locked()",
                      "vpn_server_lifecycle_lock_release(lock_fd)"],"VPN client start")
require("safe_start_xl2tpd_locked()" in client_start_locked,
        "L2TP client must use the non-recursive locked daemon starter")
client_restart=body(vpn_client,"restart_vpn_client(void)")
client_restart_locked=body(vpn_client,"restart_vpn_client_locked(void)")
ordered(client_restart,["vpn_server_lifecycle_lock_acquire()",
                        "restart_vpn_client_locked()",
                        "vpn_server_lifecycle_lock_release(lock_fd)"],"VPN client restart")
ordered(client_restart_locked,["stop_vpn_client_force()",
                               "if (stop_status != 0)",
                               "stop_vpn_client_locked()",
                               "if (stop_status != 0)",
                               "start_vpn_client_locked()",
                               "vpn_client_restore_shared_xl2tpd_locked("],"VPN client shared-daemon restart")
client_stop=body(vpn_client,"stop_vpn_client(void)")
client_stop_locked=body(vpn_client,"stop_vpn_client_locked(void)")
client_force_stop=body(vpn_client,"stop_vpn_client_force(void)")
client_pid_plan=body(vpn_client,"stop_vpn_client_ppp(int stop_xl2tpd)")
client_control=body(vpn_client,"control_xl2tpd(const char *cmd, const char *lac)")
ordered(client_stop,["vpn_server_lifecycle_lock_acquire()",
                     "stop_vpn_client_locked()",
                     "vpn_server_lifecycle_lock_release(lock_fd)"],"VPN client stop")
require("process_inventory_scan(\"xl2tpd\", &snapshot)" in client_stop_locked and
        "scan_error = snapshot.error;" in client_stop_locked and
        "process_snapshot_release(&snapshot);" in client_stop_locked and
        "process_presence_start_action_for_state(presence)" in client_stop_locked,
        "VPN client stop must use a complete xl2tpd tri-state inventory")
ordered(client_stop_locked,["scan_error = snapshot.error;",
                            "process_snapshot_release(&snapshot);",
                            "process_presence_start_action_for_state(presence)",
                            "stop_vpn_client_ppp(0)"],"VPN client stop preflight")
require("PROCESS_PRESENCE_START_ABORT" in client_stop_locked and
        "PROCESS_PRESENCE_START_NOTIFY" in client_stop_locked and
        "pids(" not in client_stop_locked,
        "VPN client stop must fail closed and remove lossy xl2tpd pids gate")
require('control_xl2tpd("d", "VPNC")' in client_stop_locked and
        "if (control_status != 0)" in client_stop_locked,
        "VPN client stop must propagate xl2tpd disconnect failures")
require("written = write(control_fd, buf, (size_t)length);" in client_control and
        "written != length" in client_control and
        "close(control_fd) != 0" in client_control,
        "xl2tpd control helper must propagate short-write and close errors")
require('control_xl2tpd("c", "VPNC")' in client_start_locked and
        "cannot connect VPNC tunnel through xl2tpd control FIFO" in client_start_locked,
        "VPN client start must report control FIFO command failures")
require("stop_vpn_client_ppp(0)" in client_stop_locked and "if (stop_status != 0)" in client_stop_locked,
        "VPN client stop must fail closed on unknown PPP link identity")
require("kill_services_with_pidfiles" in client_pid_plan and
        'pidfile_paths, "pppd", 0, 5, 1' in client_pid_plan,
        "VPN client stop must bind pppd link PID file to process identity")
require("stop_vpn_client_ppp(1)" in client_force_stop and "if (stop_status != 0)" in client_force_stop,
        "forced VPN client stop must preflight PPP link PID and xl2tpd together")
require("stop_vpn_client_locked()" in client_restart_locked,
        "VPN client restart must use the non-recursive locked stop helper")

client_stop_failure=client_restart_locked.split(
    "stop_status = stop_vpn_client_locked();", 1)[1].split(
    "restore_dns_from_vpnc();", 1)[0]
ordered(client_stop_failure, [
    "if (stop_status != 0)",
    "saved_errno = errno ? errno : EIO;",
    "vpn_client_restore_shared_xl2tpd_locked(",
    "if (restore_status < 0)",
    "errno = saved_errno;",
    "return stop_status;"
], "VPN client incomplete-stop shared-daemon rollback")
require("cannot restore shared xl2tpd after incomplete VPN client teardown" in
        client_stop_failure,
        "VPN client rollback must log an unsuccessful shared-daemon restore")

client_active = body(vpn_client, "static int\nget_xl2tpd_vpnc_active(int *active)")
ordered(client_active, [
    "nvram_transaction_begin(&transaction)",
    '"vpnc_enable"',
    "vpn_client_read_transaction_int(",
    '"vpnc_type"',
    '"l2tp_cli_t"',
    "nvram_transaction_end(&transaction)",
    "*active = (enabled && type == 1) || l2tp_client;"
], "VPN client xl2tpd stop-set checked NVRAM snapshot")
require('nvram_match("vpnc_enable"' not in client_active and
        'nvram_match("vpnc_type"' not in client_active and
        'nvram_match("l2tp_cli_t"' not in client_active,
        "VPN client xl2tpd stop-set must not use error-collapsing match getters")

client_dependencies = body(vpn_client, "static int\nvpn_client_read_shared_xl2tpd_dependencies(")
ordered(client_dependencies, [
    "nvram_transaction_begin(&transaction)",
    '"l2tp_wan_t"',
    '"l2tp_srv_t"',
    "nvram_transaction_end(&transaction)"
], "VPN client shared xl2tpd dependency snapshot")
client_shared_restore = body(vpn_client, "static int\nvpn_client_restore_shared_xl2tpd_locked(")
ordered(client_shared_restore, [
    "if (!xl2tpd_killed_vpnc)",
    "vpn_client_read_shared_xl2tpd_dependencies(",
    "vpn_client_restart_should_attempt_restore_shared_xl2tpd(",
    "safe_start_xl2tpd_locked()",
    "if (dependency_status != 0)",
    "return -1"
], "VPN client shared xl2tpd fail-safe restoration")
require("vpn_client_nvram_read_int_status" in
        (ROOT / "trunk/user/rc/src/vpn_client_nvram_policy.h").read_text() and
        "test_integer_read_status_preserves_defaults_and_checks_bounds" in
        (ROOT / "trunk/user/rc/src/tests/vpn_client_nvram_policy.c").read_text(),
        "VPN client transaction selector parser lacks bounded integer regression")
require("vpn_client_restart_should_attempt_restore_shared_xl2tpd" in
        (ROOT / "trunk/user/rc/src/vpn_client_restart_policy.h").read_text() and
        "test_client_shared_xl2tpd_restore_is_attempted_when_dependency_read_fails" in
        (ROOT / "trunk/user/rc/src/tests/vpn_server_restart_policy.c").read_text(),
        "VPN client unknown-dependency restore policy regression missing")

wan_stop_lock=body(net_wan,"run_wan_stop_under_vpn_lifecycle_lock(int (*operation)(void), const char *name)")
ordered(wan_stop_lock,["vpn_server_lifecycle_lock_acquire()",
                       "result = operation()",
                       "vpn_server_lifecycle_lock_release(lock_fd)"],"WAN stop lock")
wan_ppp_stop=body(net_wan,"stop_wan_ppp_locked(void)")
wan_full_stop=body(net_wan,"stop_wan_locked(void)")
client_stop_under_lock=body(vpn_client,"stop_vpn_client_under_lifecycle_lock(void)")
fence_call = "stop_status = invalidate_wan_ppp_callback_session();"
vpn_stop_call = "stop_status = stop_vpn_client_under_lifecycle_lock();"
ppp_kill_call = "stop_status = kill_services(svcs_ppp, 6, 1);"
require(vpn_stop_call in wan_ppp_stop and ppp_kill_call in wan_ppp_stop and
        fence_call in wan_ppp_stop and "stop_vpn_client() != 0" not in wan_ppp_stop,
        "WAN PPP stop must preserve VPN/PPP/fence statuses inside the lifecycle lock")
ppp_kill = wan_ppp_stop.index(ppp_kill_call)
ppp_gate = wan_ppp_stop.index("if (stop_status != 0)", ppp_kill)
ppp_abort = wan_ppp_stop.index("goto abort_resume_detect;", ppp_gate)
ppp_fence = wan_ppp_stop.index(fence_call)
ppp_clear_state = wan_ppp_stop.index("stop_status = clear_wan_state_status();")
require(wan_ppp_stop.index(vpn_stop_call) < ppp_kill < ppp_gate < ppp_abort < ppp_fence < ppp_clear_state,
        "WAN PPP stop must fence old callbacks only after the previous daemon stop succeeds")
require(fence_call in wan_full_stop and vpn_stop_call in wan_full_stop and
        ppp_kill_call in wan_full_stop and "if (stop_status != 0)" in wan_full_stop and
        "stop_vpn_client() != 0" not in wan_full_stop,
        "WAN full stop must preserve callback, VPN and PPP stop statuses inside the lock-owning wrapper")
full_kill = wan_full_stop.index(ppp_kill_call)
full_gate = wan_full_stop.index("if (stop_status != 0)", full_kill)
full_abort = wan_full_stop.index("goto abort_resume_detect;", full_gate)
full_fence = wan_full_stop.index(fence_call)
require(wan_full_stop.index(vpn_stop_call) < full_kill < full_gate < full_abort < full_fence,
        "WAN full stop must fence old callbacks only after the previous daemon stop succeeds")
require("return stop_vpn_client_locked();" in client_stop_under_lock and
        "vpn_server_lifecycle_lock_acquire" not in client_stop_under_lock and
        "vpn_server_lifecycle_lock_release" not in client_stop_under_lock,
        "WAN's under-lock client helper must not recursively acquire or release the process-associated lock")
require("stop_vpn_client_under_lifecycle_lock" in rc_header,
        "the caller-holds-lock VPN client entry point must be declared")
require("run_wan_stop_under_vpn_lifecycle_lock(stop_wan_ppp_locked" in net_wan and
        "run_wan_stop_under_vpn_lifecycle_lock(stop_wan_locked" in net_wan,
        "both WAN teardown entry points must use the lifecycle barrier")

firmware=body(rc,"flash_firmware(void)")
ordered(firmware,["vpn_server_lifecycle_lock_acquire()",
                  "kill_services(svcs, 6, 1)",
                  "vpn_server_lifecycle_lock_release(lock_fd)",
                  "if (stop_status != 0)",
                  "stop_misc()"],"firmware PPP stop")
require("firmware update aborted: cannot lock VPN server lifecycle" in firmware,
        "firmware shutdown must fail closed on lifecycle lock failure")
require("safe_start_xl2tpd_locked();" in body(vpn,"start_vpn_server_locked(void)"),
        "server start must use non-recursive locked L2TP helper")

# In-flight PPP callbacks must share the lifecycle critical section and prove
# that their pppd parent and per-interface pidfile still identify one session.
for t in ("PPPD_PID", "getppid()", "/var/run/%s.pid",
          "vpn_server_callback_pids_match",
          'process_pidfile_read_identity(pidfile, "pppd", &identity)'):
    require(t in vpn,f"callback identity gate missing {t}")
for t in ("O_NOFOLLOW", "O_CLOEXEC", "O_NONBLOCK", "fstat(fd, &st)",
          "S_ISREG(st.st_mode)", "st.st_uid != 0", "st.st_nlink != 1",
          "(st.st_mode & 022) != 0"):
    require(t in process_pidfile,f"central PID-file reader missing safety property {t}")
for t in ("vpn_server_callback_pid_parse", "vpn_server_callback_pids_match",
          "callback_pid == parent_pid", "parent_pid == pidfile_pid"):
    require(t in callback_policy,f"callback policy missing {t}")
up=body(vpn,"ipup_vpns_main(int argc, char **argv)")
down=body(vpn,"ipdown_vpns_main(int argc, char **argv)")
require('#include <process_inventory.h>' in vpn and
        '#include "process_presence_start_policy.h"' in vpn,
        "VPN server callbacks missing tri-state process inventory dependencies")
for callback, label in ((up, "VPN up bcrelay gate"), (down, "VPN down bcrelay gate")):
    require('process_inventory_scan("bcrelay", &snapshot)' in callback,
            f"{label}: missing complete process inventory scan")
    require("scan_error = snapshot.error;" in callback and
            "process_snapshot_release(&snapshot);" in callback and
            "process_presence_start_action_for_state(presence)" in callback,
            f"{label}: snapshot error/release/action order incomplete")
    ordered(callback, ["scan_error = snapshot.error;",
                       "process_snapshot_release(&snapshot);",
                       "process_presence_start_action_for_state(presence)"],
            label)
    require("pids(svcs[0])" not in callback,
            f"{label}: lossy pids() gate remains")
require("PROCESS_PRESENCE_START_ABORT" in up and
        "PROCESS_PRESENCE_START_NOTIFY" in up and
        "PROCESS_PRESENCE_START_NEW" in up,
        "VPN up bcrelay start/stop tri-state branches incomplete")
require('eval("/usr/sbin/bcrelay", "-d", "-i", IFNAME_BR, "-o", "ppp[1-5][0-9]", "-n")' in up,
        "VPN bcrelay bridge-to-client relay direction changed")
require('eval("/usr/sbin/bcrelay", "-d", "-i", "ppp[1-5][0-9]", "-o", IFNAME_BR, "-n")' in up,
        "VPN bcrelay client-to-bridge relay direction changed")
require("PROCESS_PRESENCE_START_ABORT" in down and
        "PROCESS_PRESENCE_START_NOTIFY" in down,
        "VPN down bcrelay UNKNOWN/stop branches incomplete")
require("cleanup_failed = 1;" in up and "cleanup_failed = 1;" in down,
        "VPN bcrelay failure must propagate to callback status")
ordered(up, ["if (relay_action == PROCESS_PRESENCE_START_ABORT)",
             "else if (relay_action == PROCESS_PRESENCE_START_NOTIFY)",
             "else if (relay_action == PROCESS_PRESENCE_START_NEW && i_vuse == 0)"],
        "VPN up bcrelay branch order")
ordered(down, ["if (relay_action == PROCESS_PRESENCE_START_ABORT)",
               "else if (relay_action == PROCESS_PRESENCE_START_NOTIFY)",
               "kill_services(svcs, 3, 1)"], "VPN down bcrelay branch order")
ordered(up,[
            "vpns_callback_lifecycle_enter(argv[1], &lifecycle_fd)",
            "vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE)",
            "vpns_firewall_permission(argv[1], 1)",
            "append_vpns_lease_record",
            "vpns_route_to_remote_lan(peer_name, argv[1], NULL, 1)",
            "set_vpn_balancing(argv[1], 1)",
            "_eval(hook_argv, NULL, 0, NULL)",
            "vpn_server_lifecycle_lock_release(lifecycle_fd)"],"VPN up callback")
ordered(down,["vpns_callback_lifecycle_enter(argv[1], &lifecycle_fd)",
              "vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE)",
              "vpns_firewall_permission(argv[1], 0)",
              "vpns_prune_lease_file",
              "vpns_route_to_remote_lan(peer_name, argv[1], NULL, 0)",
              "_eval(hook_argv, NULL, 0, NULL)",
              "vpn_server_lifecycle_lock_release(lifecycle_fd)"],"VPN down callback")
require("ipcp_script(path_ipup, 0)" in pppd_ipcp and
        "ipcp_script(path_ipdown, 0)" in pppd_ipcp,
        "pppd callback scripts must remain asynchronous when taking lifecycle lock")
require("argv[6] = ipparam" in pppd_ipcp,"pppd ipparam ABI changed")
pppd_pidfile_policy=read("trunk/user/pppd/ppp-2.4.7/pppd/pppd_pidfile_policy.h")
pppd_base_pid_writer=body(pppd_main,"static void\ncreate_pidfile(pid)")
pppd_link_pid_writer=body(pppd_main,"void\ncreate_linkpidfile(pid)")
require('fatal("Failed to create identity-bound pid file' in pppd_base_pid_writer and
        "error(" not in pppd_base_pid_writer,
        "pppd must exit rather than continue without a process-identity PID file")
require('fatal("Failed to create identity-bound link pid file' in pppd_link_pid_writer and
        "error(" not in pppd_link_pid_writer,
        "pppd must exit rather than continue without a link-identity PID file")
require('script_setenv("PPPD_PID", numbuf, 1)' in pppd_main and
        "create_pidfile(getpid())" in pppd_main and
        "_PATH_VARRUN, ifname" in pppd_main and
        "write_pidfile_record(pidfilename, (pid_t)pid)" in pppd_main and
        "pppd_pidfile_parse_start_time" in pppd_pidfile_policy,
        "pppd parent/start-time pidfile identity source changed")
require("expect_invalid" in callback_test and
        "vpn_server_callback_pids_match" in callback_test,
        "callback regression lacks parser/session coverage")

# VPN-client PPP callbacks must be fenced before route, DNS or NVRAM effects.
for token in ("vpn_client_callback_interface_is_valid",
              "vpn_client_callback_session_matches",
              "VPN_CLIENT_CALLBACK_MAX_PPP_UNITS"):
    require(token in client_callback_policy,
            f"VPN client callback policy missing {token}")
for token in ("ppp_callback_parse_index", "vpn_server_callback_pid_parse",
              "vpn_server_callback_pids_match"):
    require(token in client_callback_policy,
            f"VPN client callback identity invariant missing {token}")
for token in ("ppp0", "ppp255", "ppp00", "ppp256",
              "VPN client PPP callback identity policy: PASS"):
    require(token in client_callback_test,
            f"VPN client callback regression missing {token}")
client_session=body(vpn_client,
    "static int\nvpn_client_callback_session_is_current(const char *ifname)")
ordered(client_session, [
    "vpn_client_callback_interface_is_valid(ifname)",
    'safe_getenv("PPPD_PID")',
    "getppid()",
    '"/var/run/%s.pid"',
    'process_pidfile_read_identity(pidfile, "pppd", &identity)',
    "vpn_client_callback_session_matches("
], "VPN client callback parent and pidfile authentication")
client_enter=body(vpn_client,
    "static int\nvpn_client_callback_lifecycle_enter(const char *ifname, int *lifecycle_fd)")
ordered(client_enter, [
    "vpn_server_lifecycle_lock_acquire()",
    "vpn_client_callback_session_is_current(ifname)",
    "vpn_server_lifecycle_lock_release(fd)"
], "VPN client callback lifecycle gate")
require('#include "vpn_client_callback_policy.h"' in vpn_client and
        '#include <process_pidfile.h>' in vpn_client,
        "VPN client callback implementation missing identity dependencies")
for signature,label,side_effect in (
    ("ipup_vpnc_main(int argc, char **argv)", "up",
     'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "1")'),
    ("ipdown_vpnc_main(int argc, char **argv)", "down",
     'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "0")')
):
    callback=body(vpn_client,signature)
    ordered(callback, [
        "if (argc < 2 || !argv || !argv[1] || !ifname",
        "vpn_client_callback_lifecycle_enter(\n\t\targv[1], &lifecycle_fd)",
        "umask(0000)",
        side_effect,
        "vpn_server_lifecycle_lock_release(lifecycle_fd)",
        "return status;"
    ], "VPN client "+label+" callback fencing")
    require("ignoring stale VPN client "+label+" callback" in callback,
            "VPN client "+label+" callback must ignore stale sessions")
client_stop=body(vpn_client,"static int\nstop_vpn_client_locked(void)")
ordered(client_stop, [
    "stop_vpn_client_ppp(0)",
    "restore_dns_from_vpnc()",
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "0")',
    "unlink(VPNC_PPP_UP_SCRIPT)"
], "VPN client stop must restore DNS after stopping the PPP peer")
client_restart=body(vpn_client,"static int\nrestart_vpn_client_locked(void)")
require("restore_dns_from_vpnc()" not in client_restart,
        "VPN client restart must not repeat the DNS restoration already performed by stop")


# OpenVPN callbacks run after privilege drop and must serialize against the
# root RC lease clear on the same inode, with a PID fence checked after lock.
openvpn=read("trunk/user/rc/src/vpn_openvpn.c")
openvpn_stop_policy=read("trunk/user/rc/src/openvpn_process_stop_policy.h")
openvpn_stop_test=read("trunk/user/rc/src/tests/openvpn_process_stop_policy.c")
require('#include "openvpn_process_stop_policy.h"' in openvpn,
        "OpenVPN process stop policy header is not integrated")
pid_identity=body(openvpn,"static enum process_presence\nopenvpn_read_process_identity")
ordered(pid_identity,["open(pidfile, O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK)",
    "fstat(fd, &before)","openvpn_process_stop_parse_pid(pid_text, used, &pid)",
    'process_inventory_scan("openvpn", &snapshot)',
    "snapshot.items[i].pid != pid","process_identity_validate(\"openvpn\", identity)"],
    "numeric OpenVPN PID mapping")
for token in ("before.st_uid != (uid_t)0","before.st_nlink != 1",
              "(before.st_mode & 022) != 0","before.st_size != after.st_size"):
    require(token in pid_identity,f"OpenVPN numeric PID file validation missing {token}")
signal_identity=body(openvpn,"static int\nopenvpn_signal_process_identity")
ordered(signal_identity,["process_identity_validate(\"openvpn\", identity)",
    "kill(identity->pid, signal_number)"],"OpenVPN identity-bound signal")
stop_process=body(openvpn,"static int\nopenvpn_stop_pidfile_process")
ordered(stop_process,["openvpn_read_process_identity(pidfile, &identity)",
    "openvpn_signal_process_identity(&identity, SIGTERM)",
    "openvpn_wait_process_stopped(&identity)",
    "openvpn_signal_process_identity(&identity, SIGKILL)",
    "openvpn_wait_process_stopped(&identity)"],"OpenVPN bounded stop escalation")
require("ETIMEDOUT" in openvpn and "process_identity_validate(\"openvpn\", identity)" in openvpn,
        "OpenVPN stop must wait for a verified process exit")
tap_start=body(openvpn,"static int\nopenvpn_tapif_start")
ordered(tap_start,[
    "if (!interface_created)",
    "*interface_created = 0;",
    'doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname)',
    "if (status != 0)",
    "created = 1;",
    "*interface_created = created;",
    "if (insert_to_bridge)",
    "br_add_del_if(IFNAME_BR, ifname, 1)",
    'doSystem("ifconfig %s %s %s", ifname, "0.0.0.0", "promisc up")',
    "if (status != 0)",
    "return status;",
    "set_vpn_balancing(ifname, is_server)",
], "OpenVPN TAP setup with deferred, ownership-gated rollback")
tun_start=body(openvpn,"static int\nopenvpn_tunif_start")
require("*interface_created = created;" in tap_start and
        tap_start.index("*interface_created = created;") <
        tap_start.index('doSystem("ifconfig %s %s %s"') and
        '"--rmtun"' not in tap_start and
        "br_add_del_if(IFNAME_BR, ifname, 0)" not in tap_start,
        "TAP setup failure must defer destructive cleanup to the process-inventory-gated caller")
require('doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname)' in tun_start and
        "if (status != 0)" in tun_start and
        "set_vpn_balancing(ifname, is_server)" in tun_start,
        "OpenVPN TUN creation failure must be returned before balancing")
for signature,label in (("int\nstart_openvpn_server(void)","server"),
                         ("int\nstart_openvpn_client(void)","client")):
    start_fn=body(openvpn,signature)
    require("interface_status = openvpn_tunif_start(" in start_fn and
            "interface_status = openvpn_tapif_start(" in start_fn and
            "&interface_created" in start_fn and
            "if (interface_status != 0)" in start_fn and
            start_fn.index("openvpn_script_link_ensure(") <
            start_fn.index("openvpn_tunif_start(") <
            start_fn.index("if (interface_status != 0)") <
            start_fn.index("launch_status = _eval(openvpn_argv"),
            f"OpenVPN {label} startup must validate callback and report interface ownership before launch")
    ordered(start_fn, [
        "launch_status = _eval(openvpn_argv, NULL, 0, NULL)",
        "detached_daemon_start_postcondition_with_ops(",
        "if (launch_status != 0)",
        "openvpn_start_rollback_failed(",
        "return 0;",
    ], f"OpenVPN {label} detached daemon identity postcondition and rollback")

rollback_fn=body(openvpn,"static int\nopenvpn_start_rollback_failed")
ordered(rollback_fn, [
    "openvpn_start_rollback_presence(NULL, &scan_error)",
    "openvpn_start_rollback_cleanup_owned(process_state,",
    "openvpn_tunif_stop(ifname)",
    "openvpn_tapif_stop(ifname)",
    "openvpn_script_link_remove_created(script_path,",
    "if (process_state != PROCESS_PRESENCE_ABSENT)",
    "retaining interface and callback link",
], "OpenVPN failed-start cleanup is identity-safe and ownership-gated")
for signature,label in (("int\nopenvpn_tapif_stop","TAP"),("int\nopenvpn_tunif_stop","TUN")):
    iface_stop=body(openvpn,signature)
    require("ifconfig(ifname, 0, NULL, NULL)" in iface_stop and
            'doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname)' in iface_stop,
            f"OpenVPN {label} teardown must check interface-down and device-removal status")
server_openvpn_stop=body(openvpn,"int\nstop_openvpn_server(void)")
client_openvpn_stop=body(openvpn,"int\nstop_openvpn_client(void)")
ordered(server_openvpn_stop,["openvpn_stop_pidfile_process(SERVER_PID_FILE)",
    "openvpn_tapif_stop(IFNAME_SERVER_TAP)","openvpn_tunif_stop(IFNAME_SERVER_TUN)",
    "unlink(vpns_scr)"],"OpenVPN server checked stop")
ordered(client_openvpn_stop,["openvpn_stop_pidfile_process(CLIENT_PID_FILE)",
    "openvpn_tapif_stop(IFNAME_CLIENT_TAP)","openvpn_tunif_stop(IFNAME_CLIENT_TUN)",
    "unlink(vpnc_scr)"],"OpenVPN client checked stop")
require("openvpn_process_stop_parse_pid" in openvpn_stop_policy and
        "openvpn_process_stop_result" in openvpn_stop_policy and
        "PROCESS_PRESENCE_UNKNOWN" not in openvpn_stop_policy,
        "OpenVPN stop policy must keep numeric parsing and fail-closed state contract")
require('expect_pid("4242\\n", 5, (pid_t)4242)' in openvpn_stop_test and
        "PROCESS_PRESENCE_UNKNOWN" in openvpn_stop_test and
        "ETIMEDOUT" not in openvpn_stop_test,
        "OpenVPN stop policy regression missing numeric/UNKNOWN cases")
require("openvpn_process_stop_result(PROCESS_PRESENCE_RUNNING, 0) == -1" in openvpn_stop_test,
        "OpenVPN RUNNING state must not satisfy stop postcondition")

xy_workflow=read(".github/workflows/xy-c1-firmware-validation.yml")
for path in ("trunk/user/rc/src/l2tp_config_file.h",
             "trunk/user/rc/src/tests/l2tp_config_file.c",
             "trunk/user/rc/src/vpn_openvpn.c",
             "trunk/user/rc/src/openvpn_client_secret_file.h",
             "trunk/user/rc/src/openvpn_script_link.h",
             "trunk/user/rc/src/openvpn_start_rollback_policy.h",
             "trunk/user/openvpn/openvpn-cert.sh",
             "trunk/user/rc/src/openvpn_process_stop_policy.h"):
    require(xy_workflow.count('      - "{}"'.format(path)) == 1,
            f"XY-C1 firmware workflow must rebuild OpenVPN source dependency: {path}")
vpn_server_stop=body(vpn,"static int\nstop_vpn_server_locked(void)")
require("if (stop_openvpn_server() != 0)" in vpn_server_stop and
        vpn_server_stop.index("if (stop_openvpn_server() != 0)") < vpn_server_stop.index("vpn_server_set_l2tp_server_marker(0)") and
        vpn_server_stop.index("if (stop_openvpn_server() != 0)") < vpn_server_stop.index("unlink(VPNS_PPP_UP_SCRIPT)"),
        "VPN server cleanup must wait for OpenVPN stop")
vpn_client_stop=body(vpn_client,"static int\nstop_vpn_client_locked(void)")
require("stop_status = stop_openvpn_client();" in vpn_client_stop and
        "if (stop_status != 0)" in vpn_client_stop and
        vpn_client_stop.index("if (stop_status != 0)") <
        vpn_client_stop.index('vpn_client_nvram_set_temp_status(\n\t\tNULL, "l2tp_cli_t", "0")'),
        "VPN client NVRAM/script cleanup must wait for OpenVPN stop")
ordered(vpn_client_stop, [
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "l2tp_cli_t", "0")',
    "cleanup_status = route_status_accumulate(cleanup_status, route_status);",
    'vpn_client_nvram_set_temp_status(\n\t\tNULL, "vpnc_state_t", "0")',
    "cleanup_status = route_status_accumulate(cleanup_status, route_status);",
    "unlink(VPNC_PPP_UP_SCRIPT)",
    "return cleanup_status;"
], "VPN client checked NVRAM cleanup")
require("nvram_set_int_temp(" not in vpn_client,
        "VPN client must not retain unchecked temporary NVRAM setters")

require('#include "vpn_server_lease_lock.h"' in openvpn,
        "OpenVPN callbacks do not include the shared lease lock")
parent_fence=body(openvpn,"static int\nopenvpn_server_lease_parent_is_current(void)")
ordered(parent_fence,["parent_pid = getppid()",
    "open(SERVER_PID_FILE, O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC)",
    "fstat(fd, &st)","S_ISREG(st.st_mode)",
    "read(fd, pid_text, sizeof(pid_text))","openvpn_server_lease_pid_matches"],
    "OpenVPN callback daemon identity fence")
for token in ("st.st_uid != (uid_t)0","st.st_nlink != 1","(st.st_mode & 022) != 0"):
    require(token in parent_fence,f"OpenVPN PID marker trust validation missing {token}")
lease_update=body(openvpn,"static int\nopenvpn_server_lease_update(")
ordered(lease_update,["vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE)",
    "openvpn_server_lease_parent_is_current()",
    "open(path, O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC)",
    "fstat(source_fd, &source_stat)","openvpn_server_lease_copy(" ,
    "openvpn_config_file_publish(" ,
    "vpn_server_lease_lock_release(lock_fd)"],"OpenVPN shared-lock lease publication")
require("stale_callback = 1" in lease_update and "return stale_callback ? -2 : -1" in lease_update,
        "OpenVPN lease update must distinguish late callbacks from I/O errors")
for signature,label in (("static void\non_server_client_connect(int is_tun)","connect"),
                        ("static void\non_server_client_disconnect(int is_tun)","disconnect")):
    callback=body(openvpn,signature)
    ordered(callback,["lease_status = openvpn_server_lease_update(",
        "if (lease_status == -2)","check_if_file_exist(script_name)",
        "_eval(hook_argv, NULL, 0, NULL)"],f"OpenVPN {label} stale callback hook gate")
require("(is_add && !ifname[0])" in lease_update and
        "(ifname[0] && (strlen(ifname) >= 16" in lease_update,
        "OpenVPN lease disconnect must allow an unknown interface while add must require one")
for signature,label in (("static void\non_server_client_connect(int is_tun)","connect"),
                        ("static void\non_server_client_disconnect(int is_tun)","disconnect")):
    callback=body(openvpn,signature)
    require("if (strlen(dev_ifname) == 0 && is_tun >= 0)" in callback,
            f"OpenVPN {label} callback must not invent a device after a mode read failure")
    ordered(callback,["lease_status = openvpn_server_lease_update(",
        "if (lease_status == -2)","check_if_file_exist(script_name)",
        "_eval(hook_argv, NULL, 0, NULL)"],f"OpenVPN {label} stale callback hook gate")
server_entry=body(openvpn,"int\novpn_server_script_main(int argc, char **argv)")
require('nvram_get_checked("vpns_ov_mode", &mode_value)' in server_entry and
        "vpn_server_config_int_status(" in server_entry and
        'nvram_get_int("vpns_ov_mode")' not in server_entry,
        "OpenVPN server callback mode fallback must use checked NVRAM reads")
ordered(server_entry,[
    'char *dev_ifname = safe_getenv("dev");',
    'if ((strcmp(script_type, "client-connect") == 0 ||',
    'api_result = nvram_get_checked("vpns_ov_mode", &mode_value);',
    "if (mode_status != 0)",
    "on_server_client_connect(i_mode_tun)",
    "on_server_client_disconnect(i_mode_tun)"
],"OpenVPN server callback device fallback and mandatory event dispatch")
ovpn_clear=body(openvpn,"static int\nopenvpn_server_lease_clear(void)")
ordered(ovpn_clear,["vpn_server_lease_lock_acquire(VPN_SERVER_LEASE_LOCK_FILE)",
    "unlink(SERVER_PID_FILE)","unlink(VPN_SERVER_LEASE_FILE)",
    "vpn_server_lease_lock_release(lock_fd)"],"OpenVPN restart lease clear")
ovpn_restart=body(openvpn,"int\nrestart_openvpn_server(void)")
require('#include "vpn_server_restart_policy.h"' in openvpn,
        "OpenVPN restart status policy is not integrated")
ordered(ovpn_restart,["vpn_openvpn_server_restart_run(stop_openvpn_server,",
    "openvpn_server_lease_clear","start_openvpn_server)",
    "if (restart_status != 0)","return restart_status"],
    "OpenVPN restart fail-closed stop/fence/start and result")
reapply_locked=body(vpn,"static int\nreapply_vpn_server_locked(void)")
reapply=body(vpn,"int\nreapply_vpn_server(void)")
require("return restart_openvpn_server();" in reapply_locked and
        "return gen_vpns_pppd_options(i_type);" in reapply_locked,
        "VPN reapply must return OpenVPN and PPP configuration failures")
ordered(reapply,["vpn_server_lifecycle_lock_acquire()",
    "result = reapply_vpn_server_locked()",
    "vpn_server_lifecycle_lock_release(lock_fd)","return result"],
    "VPN reapply lifecycle result propagation")
require("int reapply_vpn_server(void);" in rc_header and
        "int restart_openvpn_server(void);" in rc_header,
        "VPN reapply/OpenVPN restart declarations must expose status")
require(rc.count("action_result = rc_notify_merge_action_result(action_result, reapply_vpn_server());") == 2,
        "WINS and VPN reapply notifications must merge the reapply status")

for path in ("trunk/user/rc/src/l2tp_config_file.h",
             "trunk/user/rc/src/tests/l2tp_config_file.c"):
    push_paths, pull_request_section = workflow.split("  pull_request:", 1)
    pull_paths = pull_request_section.split("  workflow_dispatch:", 1)[0]
    require(push_paths.count(path) == 1 and pull_paths.count(path) == 1,
            f"VPN workflow must watch rp-L2TP config dependency exactly once: {path}")
require("l2tp-config-file" in workflow and
        "tests/l2tp_config_file.c" in workflow,
        "VPN workflow does not compile and run rp-L2TP config regression")


route_snapshot=body(vpn,"vpn_server_read_route_acl_snapshot(struct vpn_server_route_acl_snapshot *snapshot)")
route_callback=body(vpn,"vpns_route_to_remote_lan(const char *cname, char *ifname, char *gw, int add)")
ordered(route_snapshot,[
    "nvram_transaction_begin(&transaction)",
    '"vpns_num_x"',
    '"lan_ipaddr"',
    '"lan_netmask"',
    '"vpns_user_x%d"',
    '"vpns_rnet_x%d"',
    '"vpns_rmsk_x%d"',
    "vpn_server_route_netmask_status(remote_mask)",
    "nvram_transaction_end(&transaction)"
],"VPN remote-LAN ACL transactional snapshot")
require("vpn_server_transaction_copy_text" in route_snapshot and
        "vpn_server_pool_validate_lan" in route_snapshot and
        "vpn_server_pool_parse_ipv4" in route_snapshot,
        "VPN remote-LAN ACL snapshot must use copied checked NVRAM and validated LAN/subnet addresses")
ordered(route_callback,[
    "vpn_server_read_route_acl_snapshot(&snapshot)",
    "for (i = 0; i < snapshot.entry_count; ++i)",
    "strcmp(snapshot.entries[i].username, cname)",
    "is_same_subnet2(",
    "route_add(ifname, 0",
],"VPN remote-LAN routes must use one completed ACL snapshot before mutation")
require("nvram_safe_get" not in route_callback and
        "nvram_get_int" not in route_callback and
        "nvram_safe_get" not in route_snapshot and
        "nvram_get_int" not in route_snapshot,
        "VPN remote-LAN route handling must not use unchecked NVRAM compatibility getters")
require("vpn_server_route_netmask_status" in config_policy and
        "test_route_netmask_validation" in config_policy_test,
        "VPN remote-LAN ACL mask validation needs unit regression")

shared_dependency_reader=body(vpn,"vpn_server_read_shared_xl2tpd_dependencies(int *wan_l2tp_enabled,")
shared_restore=body(vpn,"restore_shared_xl2tpd_locked(const char *phase)")
ordered(shared_dependency_reader,[
    "nvram_transaction_begin(&transaction)",
    '"l2tp_wan_t"',
    '"l2tp_cli_t"',
    "vpn_server_config_int_status",
    "nvram_transaction_end(&transaction)"
],"VPN shared xl2tpd dependency snapshot")
ordered(shared_restore,[
    "if (!xl2tpd_stop_attempted_vpns)",
    "vpn_server_read_shared_xl2tpd_dependencies(",
    "vpn_server_restart_should_attempt_restore_shared_xl2tpd(",
    "safe_start_xl2tpd_locked()",
    "if (dependency_status != 0)",
    "return -1"
],"VPN shared xl2tpd fail-safe restoration")
require("nvram_match(\"l2tp_wan_t\"" not in shared_restore and
        "nvram_match(\"l2tp_cli_t\"" not in shared_restore,
        "VPN shared xl2tpd restoration must not treat NVRAM read errors as disabled dependencies")
require("test_shared_xl2tpd_restore_is_attempted_when_dependency_read_fails" in restart_policy_test and
        "vpn_server_restart_should_attempt_restore_shared_xl2tpd" in restart_policy_test,
        "VPN shared xl2tpd read-error restoration regression missing")

xl2tpd_stop_set=body(vpn,"vpn_server_read_xl2tpd_stop_set_status(int *xl2tpd_active)")
ordered(xl2tpd_stop_set,[
    "nvram_transaction_begin(&transaction)",
    '"vpns_enable"',
    "vpn_server_config_enable_status",
    '"vpns_type"',
    "vpn_server_config_type_status",
    '"l2tp_srv_t"',
    "nvram_transaction_end(&transaction)",
    "vpn_server_xl2tpd_should_be_in_stop_set"
],"VPN shared xl2tpd stop-set ownership snapshot")
stop_server=body(vpn,"stop_vpn_server_locked(void)")
ordered(stop_server,[
    "vpn_server_read_xl2tpd_stop_set_status(&xl2tpd_active)",
    "if (config_status != 0)",
    'svcs[2] = "xl2tpd";',
    "xl2tpd_stop_attempted_vpns = xl2tpd_active;",
    "kill_services_with_pidfiles"
],"VPN stop must resolve shared xl2tpd dependencies before any daemon mutation")
require("nvram_match" not in xl2tpd_stop_set and
        "nvram_match" not in stop_server and
        "test_xl2tpd_stop_set_ownership_policy" in config_policy_test,
        "VPN shared xl2tpd stop-set must use checked ownership reads and tested dependency policy")

server_options_snapshot=body(vpn_openvpn,"openvpn_read_server_options_snapshot(struct openvpn_server_options_config *options)")
ordered(server_options_snapshot,[
    "is_dhcpd_enabled_status(0, &pending.dhcp_enabled)",
    "nvram_transaction_begin(&transaction)",
    '"vpns_ov_atls"',
    '"vpns_ov_tcv2"',
    '"vpns_ov_prot"',
    '"vpns_ov_rdgw"',
    '"vpns_ov_port"',
    '"vpns_ov_mdig"',
    '"vpns_ov_ciph"',
    '"vpns_ov_compress"',
    '"wins_enable"',
    '"vpns_ov_ncp_clist"',
    '"dhcp_dns1_x"',
    '"dhcp_dns2_x"',
    '"dhcp_wins_x"',
    "nvram_transaction_end(&transaction)",
    "*options = pending"
],"OpenVPN server option snapshot must check every scalar and copied DHCP option before publication")
server_export=body(vpn_openvpn,"ovpn_server_expcli_main(int argc, char **argv)")
ordered(server_export,[
    "openvpn_cert_days_valid_parse(",
    "if (days_status != 0)",
    "return 1;",
    "openvpn_read_server_network_snapshot(&network)",
    "openvpn_read_server_options_snapshot(&options)",
    "openvpn_check_key",
    "unlink(tmp_ovpn_conf)",
    "mkdtemp(tmp_ovpn_path)",
    "openvpn_config_file_open(tmp_ovpn_conf, staged_ovpn_conf",
    "network.is_tun",
    "options.port"
],"OpenVPN exported client profile must use checked server snapshots before filesystem/key-generation side effects")
require("atoi(argv[3])" not in server_export and
        "OPENVPN_CERT_DAYS_MIN 1" in cert_input_policy and
        "OPENVPN_CERT_DAYS_MAX 36500" in cert_input_policy and
        "test_malformed_and_out_of_range_lifetimes" in cert_input_policy_test and
        "test_invalid_api_arguments" in cert_input_policy_test,
        "OpenVPN client certificate export must reject malformed/overflowing lifetime arguments before side effects")
require("options.auth_digest" in server_export and
        "options.cipher" in server_export and
        "options.compression" in server_export,
        "exported profile must carry validated auth, cipher and compression settings")
ordered(server_export,[
    "openvpn_add_auth(fp, options.auth_digest)",
    "openvpn_add_cipher(fp, options.cipher, options.ncp_cipher_list)",
    "openvpn_add_compress(fp, options.compression, 0)",
    'fprintf(fp, "remote-cert-tls server\\n");'
],"OpenVPN exported client profile must preserve crypto settings and verify server certificate purpose")
require("nvram_get_int" not in server_export and
        "nvram_safe_get_int" not in server_export and
        "nvram_get(" not in server_export and
        "nvram_safe_get(" not in server_export and
        "nvram_match" not in server_export,
        "OpenVPN export must not bypass the checked server settings snapshot")
protocol_resolver=body(vpn_openvpn,"openvpn_resolve_protocol_family(")
ordered(protocol_resolver,[
    "get_ipv6_type_status(&ipv6_type)",
    "openvpn_ipv6_protocol_apply_service_state(",
    "if (policy_status != 0)",
    "return policy_status;"
],"OpenVPN protocol resolver must preserve requested IPv6 transport on service-read failure")
protocol_policy=read("trunk/user/rc/src/openvpn_ipv6_protocol_policy.h")
protocol_policy_test=read("trunk/user/rc/src/tests/openvpn_ipv6_protocol_policy.c")
require("if (service_status != 0)" in protocol_policy and
        "return service_status;" in protocol_policy and
        "assert(status == EIO && protocol == 2)" in protocol_policy_test and
        "assert(status == EACCES && protocol == 3)" in protocol_policy_test,
        "OpenVPN IPv6 policy must not downgrade transport when the service-state read fails")
wan6_source=read("trunk/user/rc/src/net_wan6.c")
wan6_identity_policy=read("trunk/user/rc/src/wan6_identity_policy.h")
wan6_identity_test=read("trunk/user/rc/src/tests/wan6_identity_policy.c")
wan6_host_status=body(wan6_source,"get_wan_addr6_host_status(char *p_addr6s, const char **p_host)")
ordered(wan6_host_status,[
    "if (!p_addr6s || !p_host)",
    "*p_host = NULL",
    "get_wan_unit_value_copy_status(",
    "return wan6_identity_resolve_addr6_host("
],"WAN IPv6 host lookup must use a bounded checked interface identity")
wan6_lookup=body(wan6_source,"wan6_lookup_ifaddr6_status(")
ordered(wan6_lookup,[
    "errno = 0;",
    "get_ifaddr6(ifname, 0, address);",
    "if (!result)",
    "return errno ? errno : 0;",
    "*found = 1;"
],"WAN IPv6 address enumeration must preserve getifaddrs errors")
wan6_resolver=body(wan6_identity_policy,"wan6_identity_resolve_addr6_host(")
ordered(wan6_resolver,[
    "if (ifname_status != 0)",
    "return ifname_status;",
    "if (!ifname[0])",
    "status = get_addr6(context, ifname, address, &found);",
    "if (status != 0)",
    "return status;",
    "if (!found)",
    "*host = address;"
],"WAN IPv6 endpoint policy must distinguish errors from clean misses")
for token in (
    "test_wan6_address_host_read_error_fails_closed",
    "test_wan6_address_host_missing_interface_keeps_fallback_eligible",
    "test_wan6_address_host_extracts_prefix_free_public_address",
    "test_wan6_address_host_does_not_hide_address_enumeration_error",
    "test_wan6_address_host_no_address_is_a_non_error",
    "test_wan6_address_host_rejects_invalid_arguments"
):
    require(token in wan6_identity_test, f"WAN IPv6 identity regression is missing {token}")
dualstack_endpoint=server_export
ordered(dualstack_endpoint,[
    "openvpn_resolve_protocol_family(",
    "wan_addr6_status = get_wan_addr6_host_status(addr6s, &wan_addr);",
    "if (wan_addr6_status != 0)",
    "goto failed;",
    "if (!wan_addr)",
    "wan_addr = get_lan_addr6_host(addr6s);",
    "if (!wan_addr && i_prot > 3)",
    'wan_addr = get_wan_unit_value(0, "ipaddr");',
    "if (!is_valid_ipv4(wan_addr))",
    'wan_addr = "{wan_address}";'
],"Dual-stack profiles must fail on IPv6 lookup errors and fall back only on clean misses")
require("wan_addr = get_wan_addr6_host(addr6s);" not in server_export,
        "OpenVPN export must not collapse WAN IPv6 lookup errors into a clean miss")
server_cert_flow=body(cert_script,"server()")
client_cert_flow=body(cert_script,"client()")
sign_cert_helper=body(cert_script,"sign_cert()")
require("extendedKeyUsage=serverAuth" in cert_script and
        "extendedKeyUsage=clientAuth" in cert_script and
        'sign_cert "$CA_KEY" "$CA_CRT" server.csr "$SERVER_CRT" "$CERT_DAYS" "$DGST_ALG" server || return $?' in server_cert_flow and
        "client_csr || return $?" in client_cert_flow and
        "openssl x509 -req" in sign_cert_helper and
        ' "$6" -extfile "$SSL_EXT_FILE"' in sign_cert_helper and
        ' "-$6" ' not in sign_cert_helper,
        "certificate generation must preserve EKU roles, fail fast and use one digest-option dash")
require('fprintf(fp, ";remote-cert-tls %s\\n", "server");' not in server_export,
        "exported profile must not leave server certificate purpose validation commented out")

client_config_snapshot=body(vpn_openvpn,"openvpn_read_client_config_snapshot(struct openvpn_client_config_snapshot *config)")
ordered(client_config_snapshot,[
    "nvram_transaction_begin(&transaction)",
    '"vpnc_ov_mode"',
    '"vpnc_ov_auth"',
    '"vpnc_ov_atls"',
    '"vpnc_ov_prot"',
    '"vpnc_ov_port"',
    '"vpnc_ov_mdig"',
    '"vpnc_ov_ciph"',
    '"vpnc_ov_compress"',
    '"vpnc_dgw"',
    '"vpnc_ov_cnat"',
    '"vpnc_peer"',
    '"vpnc_ov_ncp_clist"',
    "openvpn_safe_remote_host(pending.peer)",
    "nvram_transaction_end(&transaction)",
    "*config = pending"
],"OpenVPN client settings must be validated as one transactionally copied snapshot")
client_writer=body(vpn_openvpn,"openvpn_create_client_conf(const char *conf_file,")
require("config->peer" in client_writer and
        "config->port" in client_writer and
        "config->auth_mode" in client_writer and
        "config->tls_mode" in client_writer and
        "config->ncp_cipher_list" in client_writer and
        "nvram_get_int" not in client_writer and
        "nvram_safe_get" not in client_writer and
        "nvram_get(" not in client_writer and
        "nvram_match" not in client_writer and
        "nvram_set_int" not in client_writer,
        "OpenVPN client config writer must consume only the checked client snapshot")
client_start=body(vpn_openvpn,"start_openvpn_client(void)")
ordered(client_start,[
    "openvpn_read_client_config_snapshot(&config)",
    'doSystem("mkdir -p -m %s %s", "755", CLIENT_ROOT_DIR)',
    "openvpn_create_client_conf(vpnc_cfg, &config)",
    "config.is_tun",
    "config.client_nat"
],"OpenVPN client startup must validate and reuse one snapshot before filesystem/interface side effects")
require("test_openvpn_client_option_bounds" in config_policy_test,
        "OpenVPN client option bounds regression missing")
server_network_snapshot=body(vpn_openvpn,"openvpn_read_server_network_snapshot(struct openvpn_server_network_config *network)")
ordered(server_network_snapshot,[
    "nvram_transaction_begin(&transaction)",
    '"vpns_ov_mode"',
    '"lan_ipaddr"',
    '"lan_netmask"',
    "vpn_server_pool_parse_ipv4",
    "vpn_server_pool_validate_lan",
    '"vpns_vnet"',
    "vpn_server_pool_resolve_virtual",
    '"vpns_cli0"',
    '"vpns_cli1"',
    "nvram_transaction_end(&transaction)",
    "*network = pending"
],"OpenVPN server network snapshot must read and validate mode/LAN/TUN-or-TAP settings atomically")
openvpn_server_start=body(vpn_openvpn,"start_openvpn_server(void)")
ordered(openvpn_server_start,[
    "openvpn_read_server_network_snapshot(&network)",
    "openvpn_read_server_options_snapshot(&options)",
    "mkdir_if_none(SERVER_ROOT_DIR, \"755\")",
    "openvpn_create_server_conf(vpns_cfg, &network, &options)",
    "if (network.is_tun)",
    "openvpn_tunif_start(IFNAME_SERVER_TUN, 1, &interface_created)",
    "openvpn_tapif_start(IFNAME_SERVER_TAP, 1, 1, &interface_created)"
],"OpenVPN must validate topology and service options before filesystem/config/interface side effects")
openvpn_server_writer=body(vpn_openvpn,"openvpn_create_server_conf(const char *conf_file,")
require("network->lan_ip" in openvpn_server_writer and
        "network->lan_netmask" in openvpn_server_writer and
        "network->virtual_network" in openvpn_server_writer and
        "network->tap_client_start" in openvpn_server_writer and
        "options->port" in openvpn_server_writer and
        "options->auth_digest" in openvpn_server_writer and
        "options->cipher" in openvpn_server_writer and
        "options->compression" in openvpn_server_writer and
        "options->dns1" in openvpn_server_writer and
        "options->wins" in openvpn_server_writer and
        "nvram_get_int" not in openvpn_server_writer and
        "nvram_safe_get" not in openvpn_server_writer and
        "nvram_get(" not in openvpn_server_writer and
        "nvram_set_int" not in openvpn_server_writer,
        "OpenVPN writer must consume checked topology/options snapshots without compatibility NVRAM rereads or writes")
require("test_pool_virtual_private_network_and_lan_separation" in config_policy_test and
        "test_openvpn_server_mode_bounds" in config_policy_test and
        "test_openvpn_server_option_bounds" in config_policy_test,
        "OpenVPN topology/mode/options policy regression coverage is missing")

server_acl_snapshot=body(vpn_openvpn,"openvpn_server_acl_read_snapshot(struct openvpn_server_acl_entry *entries,")
ordered(server_acl_snapshot,[
    "nvram_transaction_begin(&transaction)",
    '"vpns_num_x"',
    "vpn_server_config_int_status",
    '"vpns_user_x%d"',
    "openvpn_server_acl_copy_text",
    '"vpns_rnet_x%d"',
    '"vpns_rmsk_x%d"',
    "vpn_server_pool_parse_ipv4",
    "vpn_server_route_netmask_status",
    '"vpns_addr_x%d"',
    "nvram_transaction_end(&transaction)",
    "memcpy(entries, pending, sizeof(pending))"
],"OpenVPN server ACL atomic snapshot")
server_acl_writer=body(vpn_openvpn,"openvpn_create_server_acl(FILE *fp, const char *ccd, unsigned int vnet, unsigned int vmsk)")
ordered(server_acl_writer,[
    "openvpn_server_acl_read_snapshot(entries, &entry_count)",
    "mkdir(vpns_ccd, 0755)",
    "lstat(vpns_ccd, &ccd_stat)",
    "for (i = 0; i < entry_count; ++i)",
    "openvpn_config_file_open(ccf, staged_ccf",
    "openvpn_config_file_publish(&fp_ccf, staged_ccf, ccf, 0644)"
],"OpenVPN CCD publication must consume a complete validated ACL snapshot")
require("nvram_get_int" not in server_acl_writer and
        "nvram_safe_get" not in server_acl_writer and
        "test_route_netmask_validation" in config_policy_test,
        "OpenVPN server CCD writer must not use unchecked per-row NVRAM reads")
callback_modes=body(vpn,"vpn_server_read_callback_mode_snapshot(int *virtual_mode, int *cast_mode)")
ordered(callback_modes,[
    "nvram_transaction_begin(&transaction)",
    '"vpns_vuse"',
    '"vpns_cast"',
    "nvram_transaction_end(&transaction)"
],"VPN callback mode transactional snapshot")
ipup_callback=body(vpn,"ipup_vpns_main(int argc, char **argv)")
ordered(ipup_callback,[
    "vpn_server_read_callback_mode_snapshot(&i_vuse, &i_cast)",
    "vpn_server_lease_lock_acquire",
    "vpns_firewall_permission(argv[1], 1)",
    "if (i_vuse)",
    "if (i_cast == 0 || i_vuse == 0)"
],"VPN up callback must check modes before firewall/lease side effects")
require("ifconfig_status = _eval(ifconfig_argv, NULL, 0, NULL);" in ipup_callback and
        "if (ifconfig_status != 0)" in ipup_callback and
        "cannot disable multicast on VPN interface" in ipup_callback and
        "cleanup_failed = 1;" in ipup_callback,
        "VPN up callback must report multicast-interface configuration failures")
ipdown_callback=body(vpn,"ipdown_vpns_main(int argc, char **argv)")
ordered(ipdown_callback,[
    "vpn_server_read_callback_mode_snapshot(&i_vuse, &i_cast)",
    "vpn_server_lease_lock_acquire",
    "vpns_firewall_permission(argv[1], 0)",
    "vpns_route_to_remote_lan(peer_name, argv[1], NULL, 0)",
    "mode_status == 0 && i_cast == 0"
],"VPN down callback must continue mandatory cleanup after optional mode-read failure")
require('nvram_get_int("vpns_vuse")' not in ipup_callback and
        'nvram_get_int("vpns_cast")' not in ipup_callback and
        'nvram_get_int("vpns_cast")' not in ipdown_callback,
        "VPN callbacks must not use unchecked mode getters")
require("test_vpn_callback_mode_bounds" in config_policy_test,
        "VPN callback mode bounds regression missing")

print("VPN server lifecycle, identity-bound OpenVPN stop and callback source contract: PASS")

# The bcrelay presence policy and its shared scanner are VPN callback dependencies.
for path in (
    "trunk/user/rc/src/vpn_server.c",
    "trunk/user/shared/src/process_inventory.c",
    "trunk/user/shared/src/process_inventory.h",
    "trunk/user/rc/src/process_presence_start_policy.h",
    "trunk/user/rc/src/tests/process_presence_start_policy.c",
):
    push, pull_request = workflow.split("  pull_request:", 1)
    require(push.count(path) == 1 and pull_request.count(path) == 1,
            f"VPN workflow missing/duplicated process-presence dependency: {path}")


# OpenVPN detached startup requires the exact daemon writepid identity. Rollback
# is permitted only for owned resources after two complete ABSENT inventories.
openvpn_rollback_policy = read("trunk/user/rc/src/openvpn_start_rollback_policy.h")
openvpn_rollback_test = read("trunk/user/rc/src/tests/openvpn_start_rollback_policy.c")
require('#include "detached_daemon_start_policy.h"' in vpn_openvpn and
        '#include "openvpn_start_rollback_policy.h"' in vpn_openvpn,
        "OpenVPN startup readiness/rollback policies must be included")
for token in ("OPENVPN_START_ROLLBACK_CONFIRM_SCANS 2",
              "OPENVPN_START_ROLLBACK_CONFIRM_WAIT_SECONDS 1",
              "state == PROCESS_PRESENCE_ABSENT",
              "state == PROCESS_PRESENCE_UNKNOWN",
              "state == PROCESS_PRESENCE_RUNNING",
              "errno = EIO"):
    require(token in openvpn_rollback_policy,
            f"OpenVPN rollback policy missing fail-closed rule {token}")
for token in ("test_two_absent_scans_are_required",
              "test_second_running_scan_revokes_absence_confirmation",
              "test_unknown_inventory_propagates_error",
              "test_invalid_inventory_state_fails_closed",
              "test_interrupted_wait_fails_closed",
              "test_cleanup_requires_absence_and_ownership"):
    require(token in openvpn_rollback_test,
            f"OpenVPN rollback regression missing {token}")
pidfile_scan = body(vpn_openvpn,
    "static enum process_presence\nopenvpn_start_pidfile_presence")
require("openvpn_read_process_identity((const char *)context, &identity)" in pidfile_scan,
        "OpenVPN startup readiness must bind to its exact writepid process identity")
rollback_scan = body(vpn_openvpn,
    "static enum process_presence\nopenvpn_start_rollback_scan")
require('process_inventory_scan("openvpn", &snapshot)' in rollback_scan and
        "process_snapshot_release(&snapshot)" in rollback_scan,
        "OpenVPN rollback must use and release a complete process inventory")
rollback_presence = body(vpn_openvpn,
    "static enum process_presence\nopenvpn_start_rollback_presence")
require("openvpn_start_rollback_confirm_absent_with_ops" in rollback_presence,
        "OpenVPN rollback must use the two-snapshot absence policy")
rollback_fn = body(vpn_openvpn,
    "static int\nopenvpn_start_rollback_failed")
ordered(rollback_fn, [
    "openvpn_start_rollback_presence(NULL, &scan_error)",
    "openvpn_start_rollback_cleanup_owned(process_state,",
    "openvpn_tunif_stop(ifname)",
    "openvpn_tapif_stop(ifname)",
    "openvpn_script_link_remove_created(script_path,",
    "if (process_state != PROCESS_PRESENCE_ABSENT)",
    "retaining interface and callback link"
], "OpenVPN startup rollback cleanup ownership/order")
require("resource_created == 1" in openvpn_rollback_policy and
        "process_state == PROCESS_PRESENCE_ABSENT" in openvpn_rollback_policy,
        "OpenVPN cleanup requires this attempt's ownership and confirmed absence")
for helper_signature, label in (
    ("static int\nopenvpn_tapif_start", "TAP"),
    ("static int\nopenvpn_tunif_start", "TUN"),
):
    require(helper_signature in vpn_openvpn,
            f"OpenVPN {label} interface helper declaration is missing")
    helper_start = vpn_openvpn.index(helper_signature)
    helper_brace = vpn_openvpn.index("{", helper_start)
    helper_declaration = vpn_openvpn[helper_start:helper_brace]
    helper_body = body(vpn_openvpn, helper_signature)
    require("int *interface_created" in helper_declaration and
            "*interface_created = 0;" in helper_body and
            "*interface_created = created;" in helper_body,
            f"OpenVPN {label} helper must report only resources created by this attempt")
    if label == "TAP":
        require(helper_body.index("*interface_created = created;") <
                helper_body.index('status = doSystem("ifconfig %s %s %s"') and
                '"--rmtun"' not in helper_body and
                "br_add_del_if(IFNAME_BR, ifname, 0)" not in helper_body,
                "TAP preparation failure must defer cleanup to the process-inventory-gated caller")
for signature, pidfile, label in (
    ("start_openvpn_server(void)", "SERVER_PID_FILE", "server"),
    ("start_openvpn_client(void)", "CLIENT_PID_FILE", "client"),
):
    start_fn = body(vpn_openvpn, signature)
    ordered(start_fn, [
        "openvpn_script_link_ensure",
        "&interface_created",
        "if (interface_status != 0)",
        "openvpn_start_rollback_failed(",
        "_eval(openvpn_argv, NULL, 0, NULL)",
        "detached_daemon_start_postcondition_with_ops(",
        pidfile,
        "if (launch_status != 0)"
    ], f"OpenVPN {label} startup/readiness/rollback sequence")
    require("launch_errno" in start_fn and "launch_status" in start_fn,
            f"OpenVPN {label} startup must preserve the primary error")
require(vpn_openvpn.count("openvpn_start_rollback_failed(") >= 5,
        "both OpenVPN start paths must gate interface and daemon failures")
for path in (
    "trunk/user/rc/src/openvpn_start_rollback_policy.h",
    "trunk/user/rc/src/tests/openvpn_start_rollback_policy.c",
):
    push, pull_request = workflow.split("  pull_request:", 1)
    require(push.count("      - '" + path + "'") == 1 and
            pull_request.count("      - '" + path + "'") == 1,
            f"VPN workflow missing/duplicated OpenVPN rollback dependency: {path}")
require("openvpn_start_rollback_policy.c" in workflow and
        "ASAN_OPTIONS=detect_leaks=1 /tmp/openvpn-start-rollback-policy" in workflow,
        "VPN workflow must compile and run the rollback regression")
xy_workflow = read(".github/workflows/xy-c1-firmware-validation.yml")
xy_test = read("scripts/tests/test_xy_c1_firmware_inputs.py")
xy_policy_path = "trunk/user/rc/src/openvpn_start_rollback_policy.h"
require(xy_workflow.count('      - "' + xy_policy_path + '"') == 1 and
        "test_openvpn_start_rollback_policy_header_triggers_xy_c1_build" in xy_test,
        "XY-C1 detector/workflow must treat the rollback policy as production input")
print("OpenVPN detached startup readiness and safe rollback source contract: PASS")
