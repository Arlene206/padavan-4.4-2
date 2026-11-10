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
lock=read("trunk/user/rc/src/vpn_server_lifecycle_lock.h")
rc=read("trunk/user/rc/src/rc.c")
services=read("trunk/user/rc/src/services.c")
makefile=read("trunk/user/rc/src/Makefile")
workflow=read(".github/workflows/vpn-server-stability.yml")
test=read("trunk/user/rc/src/tests/vpn_server_lifecycle_lock.c")
require('#include "vpn_server_lifecycle_lock.h"' in vpn,"missing lifecycle lock include")
require("vpn_server.o" in makefile,"vpn_server.o missing from RC build")
for t in ("O_NOFOLLOW","O_CLOEXEC","fstat(fd, &st)","S_ISREG(st.st_mode)",
          "st.st_uid != geteuid()","st.st_nlink != 1","(st.st_mode & 077) != 0",
          "F_SETLK","VPN_SERVER_LIFECYCLE_LOCK_WAIT_SECONDS","EBUSY","EINTR","EAGAIN"):
    require(t in lock,f"lock helper missing safety property {t}")
require("unlink(" not in lock and "unlinkat(" not in lock,"stable lock inode may not be unlinked")
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
ordered(r,["vpn_server_lifecycle_lock_acquire()","if (lock_fd < 0)","return;","stop_vpn_server_locked()","start_vpn_server_locked()","restart_firewall()","safe_start_xl2tpd()","vpn_server_lifecycle_lock_release(lock_fd)"],"restart")
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
for p in ("trunk/user/rc/src/vpn_server.c","trunk/user/rc/src/vpn_server_lifecycle_lock.h",
          "trunk/user/rc/src/tests/vpn_server_lifecycle_lock.c","trunk/user/rc/src/rc.c",
          "trunk/user/rc/src/services.c","trunk/user/rc/src/rc.h",
          "scripts/check_vpn_server_lifecycle.py",".github/workflows/vpn-server-stability.yml"):
    require(p in workflow,f"workflow path filter missing {p}")
require("fork()" in test and "errno==EBUSY" in test.replace(" ",""),"cross-process contention regression missing")
print("VPN server lifecycle source contract: PASS")
