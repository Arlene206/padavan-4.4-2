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
    'restore_shared_xl2tpd_locked("after VPN server restart")',
    "vpn_server_lifecycle_lock_release(lock_fd)",
    "return restart_status;"
],"restart")
require("return;" not in r and "return restart_status;" in r,
        "restart must return the accumulated stop/start/restore/unlock status")
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
require("fork()" in test and "errno==EBUSY" in test.replace(" ",""),"cross-process contention regression missing")
require("test_first_failure_survives_later_restore_and_unlock_errors" in restart_policy_test and
        "rc_notify_merge_action_result(0, status) == -1" in restart_policy_test,
        "VPN restart policy regression must preserve first error through notification aggregation")

# Every RC path that starts/stops the shared PPP/L2TP transport must
# participate in the same lifecycle barrier as server callbacks.
safe_start=body(net_ppp,"safe_start_xl2tpd(void)")
safe_start_locked=body(net_ppp,"safe_start_xl2tpd_locked(void)")
ordered(safe_start,["vpn_server_lifecycle_lock_acquire()",
                    "safe_start_xl2tpd_locked()",
                    "vpn_server_lifecycle_lock_release(lock_fd)"],"public xl2tpd start")
require('eval("/usr/sbin/xl2tpd"' in safe_start_locked,
        "locked xl2tpd starter lost daemon launch")
require('#include <process_inventory.h>' in net_ppp and
        '#include "xl2tpd_start_policy.h"' in net_ppp,
        "xl2tpd start must use tri-state process inventory policy")
require('process_inventory_scan("xl2tpd", &process_snapshot)' in safe_start_locked and
        "process_snapshot_release(&process_snapshot)" in safe_start_locked,
        "xl2tpd must obtain and release a complete process snapshot")
require(safe_start_locked.index('process_inventory_scan("xl2tpd", &process_snapshot)') <
        safe_start_locked.index('fopen(l2tp_conf, "w")'),
        "unknown process state must be rejected before live config truncation")
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
wan_launch_locked=body(net_ppp,"launch_wan_pppd_locked(int unit, int wan_proto)")
ordered(wan_launch,["if (unit < 0)",
                    "if (wan_proto != IPV4_WAN_PROTO_L2TP &&",
                    '!nvram_match("pppoemwan_enable", "1"))',
                    "return launch_wan_pppd_locked(unit, wan_proto)",
                    "vpn_server_lifecycle_lock_acquire()",
                    "launch_wan_pppd_locked(unit, wan_proto)",
                    "vpn_server_lifecycle_lock_release(lock_fd)"],"WAN L2TP/multi-WAN launch")
require("kill_services(svcs, 5, 1)" in wan_launch_locked and
        "xl2tpd_start_policy_wan_launch_result(" in wan_launch_locked,
        "WAN L2TP stop/config/start must remain in one locked helper")
require("xl2tpd_start_policy_wan_launch_result(\n\t\t\t\tsafe_start_xl2tpd_locked())" in wan_launch_locked,
        "WAN L2TP launch must accept already-running xl2tpd as success")
ordered(wan_launch_locked,["ppp_wan_session_generation_change(&callback_generation)",
                           'nvram_match("pppoemwan_enable", "1")',
                           "kill_services(svcs_pppd, 5, 1)",
                           'eval("/usr/bin/mwan.sh", "--rc-lifecycle-locked")'],
        "multi-WAN PPP stop before helper launch")
require('doSystem("/usr/bin/mwan.sh --rc-lifecycle-locked")' not in wan_launch_locked,
        "multi-WAN lifecycle helper must be launched without a shell")
require("refusing multi-WAN PPP launch: pppd did not stop" in wan_launch_locked and
        "EBUSY" in wan_launch_locked,
        "multi-WAN PPP launch must fail closed if the previous pppd processes remain")
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
                               "safe_start_xl2tpd_locked()"],"VPN client shared-daemon restart")
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
    "vpn_client_restart_should_restore_shared_xl2tpd(",
    "safe_start_xl2tpd_locked()",
    "if (restore_status < 0)",
    "errno = saved_errno;",
    "return stop_status;"
], "VPN client incomplete-stop shared-daemon rollback")
require("cannot restore shared xl2tpd after incomplete VPN client teardown" in
        client_stop_failure,
        "VPN client rollback must log an unsuccessful shared-daemon restore")

wan_stop_lock=body(net_wan,"run_wan_stop_under_vpn_lifecycle_lock(int (*operation)(void), const char *name)")
ordered(wan_stop_lock,["vpn_server_lifecycle_lock_acquire()",
                       "result = operation()",
                       "vpn_server_lifecycle_lock_release(lock_fd)"],"WAN stop lock")
wan_ppp_stop=body(net_wan,"stop_wan_ppp_locked(void)")
wan_full_stop=body(net_wan,"stop_wan_locked(void)")
client_stop_under_lock=body(vpn_client,"stop_vpn_client_under_lifecycle_lock(void)")
require("stop_status = stop_vpn_client_under_lifecycle_lock();" in wan_ppp_stop and
        "stop_status = kill_services(svcs_ppp, 6, 1);" in wan_ppp_stop and
        wan_ppp_stop.index("stop_status = stop_vpn_client_under_lifecycle_lock();") <
        wan_ppp_stop.index("stop_status = kill_services(svcs_ppp, 6, 1);"),
        "WAN PPP stop must preserve the under-lock VPN stop result before stopping PPP services")
require("stop_status = invalidate_wan_ppp_callback_session();" in wan_full_stop and
        "stop_status = stop_vpn_client_under_lifecycle_lock();" in wan_full_stop and
        "stop_status = kill_services(svcs_ppp, 6, 1);" in wan_full_stop and
        "if (stop_status != 0)" in wan_full_stop and
        "stop_vpn_client() != 0" not in wan_full_stop,
        "WAN full stop must preserve callback, VPN and PPP stop statuses inside the lock-owning wrapper")
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
    'doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname)',
    "if (status != 0)",
    'doSystem("ifconfig %s %s %s", ifname, "0.0.0.0", "promisc up")',
    "if (status != 0)",
    "if (created)",
    'doSystem("%s %s --dev %s", OPENVPN_EXE, "--rmtun", ifname)',
    "set_vpn_balancing(ifname, is_server)",
], "OpenVPN TAP setup and rollback")
tun_start=body(openvpn,"static int\nopenvpn_tunif_start")
require('doSystem("%s %s --dev %s", OPENVPN_EXE, "--mktun", ifname)' in tun_start and
        "if (status != 0)" in tun_start and
        "set_vpn_balancing(ifname, is_server)" in tun_start,
        "OpenVPN TUN creation failure must be returned before balancing")
for signature,interface in (("int\nstart_openvpn_server(void)","IFNAME_SERVER_TUN"),
                            ("int\nstart_openvpn_client(void)","IFNAME_CLIENT_TUN")):
    start_fn=body(openvpn,signature)
    require("interface_status = openvpn_tunif_start(" in start_fn and
            "interface_status = openvpn_tapif_start(" in start_fn and
            "if (interface_status != 0)" in start_fn and
            start_fn.index("if (interface_status != 0)") < start_fn.index("symlink(\"/sbin/rc\"") <
            start_fn.index("_eval(openvpn_argv"),
            f"OpenVPN start must abort before script link/daemon launch when interface setup fails ({interface})")
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
             "trunk/user/rc/src/openvpn_process_stop_policy.h"):
    require(xy_workflow.count('      - "{}"'.format(path)) == 1,
            f"XY-C1 firmware workflow must rebuild OpenVPN source dependency: {path}")
vpn_server_stop=body(vpn,"static int\nstop_vpn_server_locked(void)")
require("if (stop_openvpn_server() != 0)" in vpn_server_stop and
        vpn_server_stop.index("if (stop_openvpn_server() != 0)") < vpn_server_stop.index('nvram_set_int_temp("l2tp_srv_t", 0)') and
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
