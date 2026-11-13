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
rc_header=read("trunk/user/rc/src/rc.h")
lock=read("trunk/user/rc/src/vpn_server_lifecycle_lock.h")
lease_lock=read("trunk/user/rc/src/vpn_server_lease_lock.h")
lease_lock_test=read("trunk/user/rc/src/tests/vpn_server_lease_lock.c")
rc=read("trunk/user/rc/src/rc.c")
services=read("trunk/user/rc/src/services.c")
net_ppp=read("trunk/user/rc/src/net_ppp.c")
mwan=read("trunk/user/pppoe/mwan.sh")
net_wan=read("trunk/user/rc/src/net_wan.c")
vpn_client=read("trunk/user/rc/src/vpn_client.c")
makefile=read("trunk/user/rc/src/Makefile")
workflow=read(".github/workflows/vpn-server-stability.yml")
test=read("trunk/user/rc/src/tests/vpn_server_lifecycle_lock.c")
callback_policy=read("trunk/user/rc/src/vpn_server_callback_policy.h")
callback_test=read("trunk/user/rc/src/tests/vpn_server_callback_policy.c")
pppd_ipcp=read("trunk/user/pppd/ppp-2.4.7/pppd/ipcp.c")
pppd_main=read("trunk/user/pppd/ppp-2.4.7/pppd/main.c")
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
tp=body(vpn,"stop_vpn_server_locked(void)")
r=body(vpn,"restart_vpn_server(void)")
a=body(vpn,"reapply_vpn_server(void)")
ap=body(vpn,"reapply_vpn_server_locked(void)")
ordered(s,["vpn_server_lifecycle_lock_acquire()","if (lock_fd < 0)","start_vpn_server_locked()","vpn_server_lifecycle_lock_release(lock_fd)"],"start")
require("return -1;" in s and "gen_vpns_pppd_options(i_type)" in sp,"start wrapper/private helper incomplete")
ordered(t,["vpn_server_lifecycle_lock_acquire()","if (lock_fd < 0)","return;","stop_vpn_server_locked()","vpn_server_lifecycle_lock_release(lock_fd)"],"stop")
require("kill_services(svcs, 5, 1)" in tp,"private stop helper lost daemon stop")
ordered(r,["vpn_server_lifecycle_lock_acquire()","if (lock_fd < 0)","return;","stop_vpn_server_locked()","start_vpn_server_locked()","restart_firewall()","safe_start_xl2tpd_locked()","vpn_server_lifecycle_lock_release(lock_fd)"],"restart")
require("stop_vpn_server();" not in r and "start_vpn_server();" not in r,"restart must use private non-recursive helpers")
require("reload_nfsd();" in r,"restart lost conditional NFS reload")
ordered(a,["vpn_server_lifecycle_lock_acquire()","if (lock_fd < 0)","return;","reapply_vpn_server_locked()","vpn_server_lifecycle_lock_release(lock_fd)"],"reapply")
require("gen_vpns_pppd_options(i_type)" in ap or "restart_openvpn_server()" in ap,"reapply lost update behavior")
require("reapply_vpn_server();" not in ap,"private reapply must not recurse")

require(re.search(r'RCN_RESTART_VPNSVR[\s\S]{0,220}restart_vpn_server\s*\(\)',rc),"PID1 restart entry missing")
require(re.search(r'RCN_REAPPLY_VPNSVR[\s\S]{0,220}reapply_vpn_server\s*\(\)',rc),"PID1 reapply entry missing")
require(re.search(r'RCN_RESTART_WINS[\s\S]{0,260}reapply_vpn_server\s*\(\)',rc),"WINS reapply entry missing")
require(re.search(r'!strcmp\(base,\s*"restart_vpn_server"\)[\s\S]{0,160}restart_vpn_server\s*\(\)',rc),"direct RC applet entry missing")
require("start_vpn_server();" in body(services,"start_services_once(int is_ap_mode)"),"service start entry missing")
require("stop_vpn_server();" in body(services,"stop_services(int stopall)"),"service stop entry missing")
for p in ("trunk/user/rc/src/vpn_server.c","trunk/user/rc/src/vpn_openvpn.c",
          "trunk/user/rc/src/vpn_server_lifecycle_lock.h",
          "trunk/user/rc/src/vpn_server_lease_lock.h",
          "trunk/user/rc/src/tests/vpn_server_lease_lock.c",
          "trunk/user/rc/src/tests/vpn_server_lifecycle_lock.c","trunk/user/rc/src/rc.c",
          "trunk/user/rc/src/services.c","trunk/user/rc/src/rc.h",
          "trunk/user/pppoe/mwan.sh",
          "scripts/check_vpn_server_lifecycle.py",".github/workflows/vpn-server-stability.yml"):
    require(p in workflow,f"workflow path filter missing {p}")
push_paths=workflow.split("  push:",1)[1].split("  pull_request:",1)[0]
pull_paths=workflow.split("  pull_request:",1)[1].split("  workflow_dispatch:",1)[0]
openvpn_filter="      - 'trunk/user/rc/src/vpn_openvpn.c'"
require(push_paths.count(openvpn_filter) == 1 and pull_paths.count(openvpn_filter) == 1,
        "OpenVPN production source path must appear exactly once in push and pull_request filters")
require("fork()" in test and "errno==EBUSY" in test.replace(" ",""),"cross-process contention regression missing")

# Every RC path that starts/stops the shared PPP/L2TP transport must
# participate in the same lifecycle barrier as server callbacks.
safe_start=body(net_ppp,"safe_start_xl2tpd(void)")
safe_start_locked=body(net_ppp,"safe_start_xl2tpd_locked(void)")
ordered(safe_start,["vpn_server_lifecycle_lock_acquire()",
                    "safe_start_xl2tpd_locked()",
                    "vpn_server_lifecycle_lock_release(lock_fd)"],"public xl2tpd start")
require('eval("/usr/sbin/xl2tpd"' in safe_start_locked,
        "locked xl2tpd starter lost daemon launch")
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
        "safe_start_xl2tpd_locked()" in wan_launch_locked,
        "WAN L2TP stop/config/start must remain in one locked helper")
ordered(wan_launch_locked,["ppp_wan_session_generation_change(&callback_generation)",
                           'nvram_match("pppoemwan_enable", "1")',
                           "kill_services(svcs_pppd, 5, 1)",
                           'doSystem("/usr/bin/mwan.sh --rc-lifecycle-locked")'],
        "multi-WAN PPP stop before helper launch")
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
                               "stop_vpn_client()",
                               "start_vpn_client_locked()",
                               "safe_start_xl2tpd_locked()"],"VPN client shared-daemon restart")

wan_stop_lock=body(net_wan,"run_wan_stop_under_vpn_lifecycle_lock(int (*operation)(void), const char *name)")
ordered(wan_stop_lock,["vpn_server_lifecycle_lock_acquire()",
                       "result = operation()",
                       "vpn_server_lifecycle_lock_release(lock_fd)"],"WAN stop lock")
wan_ppp_stop=body(net_wan,"stop_wan_ppp_locked(void)")
wan_full_stop=body(net_wan,"stop_wan_locked(void)")
for name,part in (("WAN PPP stop",wan_ppp_stop),("WAN full stop",wan_full_stop)):
    require("stop_vpn_client();" in part and "kill_services(svcs_ppp, 6, 1)" in part,
            f"{name} must stop shared PPP services inside the lock-owning wrapper")
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
for t in ("PPPD_PID", "getppid()", "/var/run/%s.pid", "O_NOFOLLOW",
          "O_CLOEXEC", "S_ISREG(st.st_mode)", "st.st_uid != 0",
          "st.st_nlink != 1", "(st.st_mode & 022) != 0",
          "vpn_server_callback_pids_match"):
    require(t in vpn,f"callback identity gate missing {t}")
for t in ("vpn_server_callback_pid_parse", "vpn_server_callback_pids_match",
          "callback_pid == parent_pid", "parent_pid == pidfile_pid"):
    require(t in callback_policy,f"callback policy missing {t}")
up=body(vpn,"ipup_vpns_main(int argc, char **argv)")
down=body(vpn,"ipdown_vpns_main(int argc, char **argv)")
ordered(up,["vpns_callback_lifecycle_enter(argv[1], &lifecycle_fd)",
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
require('script_setenv("PPPD_PID", numbuf, 1)' in pppd_main and
        "create_pidfile(getpid())" in pppd_main and
        "_PATH_VARRUN, ifname" in pppd_main,
        "pppd parent/pidfile identity source changed")
require("expect_invalid" in callback_test and
        "vpn_server_callback_pids_match" in callback_test,
        "callback regression lacks parser/session coverage")

# OpenVPN callbacks run after privilege drop and must serialize against the
# root RC lease clear on the same inode, with a PID fence checked after lock.
openvpn=read("trunk/user/rc/src/vpn_openvpn.c")
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
ovpn_restart=body(openvpn,"void\nrestart_openvpn_server(void)")
ordered(ovpn_restart,["stop_openvpn_server()","if (openvpn_server_lease_clear() != 0)",
    "return;","start_openvpn_server()"],"OpenVPN restart stop/fence/start")
print("VPN server lifecycle and callback source contract: PASS")
