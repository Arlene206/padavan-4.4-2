#!/usr/bin/env python3
"""Check status propagation for WAN network sysctl operations."""
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
RC=ROOT/"trunk/user/rc/src"
NET=(RC/"net.c").read_text()
COMMON=(RC/"common_ex.c").read_text()
WAN=(RC/"net_wan.c").read_text()
HEADER=(RC/"rc.h").read_text()
POLICY=(RC/"network_sysctl_policy.h").read_text()
TEST=(RC/"tests/network_sysctl_policy.c").read_text()
WF=(ROOT/".github/workflows/wan-route-status.yml").read_text()
XY=(ROOT/".github/workflows/xy-c1-firmware-validation.yml").read_text()

def body(source, signature):
    start=source.index(signature)
    opening=source.index("{",start)
    depth=0
    state="code"
    quote=""
    i=opening
    while i<len(source):
        c=source[i]
        n=source[i+1] if i+1<len(source) else ""
        if state=="code":
            if c=="/" and n=="/": state="line"; i+=2; continue
            if c=="/" and n=="*": state="block"; i+=2; continue
            if c in ("'",'"'): quote=c; state="quote"
            elif c=="{": depth+=1
            elif c=="}":
                depth-=1
                if depth==0: return source[opening+1:i]
        elif state=="quote":
            if c==chr(92): i+=2; continue
            if c==quote: state="code"
        elif state=="line" and c==chr(10): state="code"
        elif state=="block" and c=="*" and n=="/": state="code"; i+=2; continue
        i+=1
    raise AssertionError("unterminated: "+signature)

def require(ok,msg):
    if not ok: raise SystemExit("Network sysctl status contract: FAIL: "+msg)

nf=body(NET,"set_nf_conntrack(void)")
require('return fput_int("/proc/sys/net/nf_conntrack_max", i_nf_val);' in nf,
        "conntrack-limit write result must be returned")
tcp=body(NET,"set_tcp_syncookies(void)")
require("rc_network_sysctl_apply(3, write_tcp_syncookies_operation, &context)" in tcp,
        "all three TCP/ICMP settings must be attempted")
tcp_writer=body(NET,"write_tcp_syncookies_operation(")
for token in ('/proc/sys/net/ipv4/tcp_syncookies',
              '/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses',
              '/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts'):
    require(token in tcp_writer,"missing TCP/ICMP sysctl write: "+token)
igmp=body(NET,"set_igmp_mld_version(void)")
igmp_writer=body(NET,"write_igmp_mld_operation(")
require("rc_network_sysctl_apply(3, write_igmp_mld_operation, &context)" in igmp
        and "rc_network_sysctl_apply(6, write_igmp_mld_operation, &context)" in igmp,
        "IPv4 writes and optional IPv6 writes must be attempted")
for token in ('set_interface_conf_int("ipv4", "all"',
              'set_interface_conf_int("ipv4", context->ifname',
              'set_interface_conf_int("ipv4", IFNAME_BR',
              'set_interface_conf_int("ipv6", "all"',
              'set_interface_conf_int("ipv6", context->ifname',
              'set_interface_conf_int("ipv6", IFNAME_BR'):
    require(token in igmp_writer,"missing multicast sysctl write: "+token)
require("first failure" in POLICY,"policy must document first-error semantics")
require("return first_status;" in POLICY,"policy must preserve first failure")
require("return connected" not in POLICY,"unrelated readiness policy must not be included")
require("mock_status[0] = EIO" in TEST and "mock_status[3] = EACCES" in TEST,
        "regression must verify first-error and continue-after-failure")
require("assert(mock_calls == 4)" in TEST,"regression must assert all independent operations ran")
for proto in ("int set_nf_conntrack(void);","int set_tcp_syncookies(void);","int set_igmp_mld_version(void);"):
    require(proto in HEADER,"missing status-returning prototype: "+proto)
for obsolete in ("void set_nf_conntrack(void);","void set_tcp_syncookies(void);","void set_igmp_mld_version(void);"):
    require(obsolete not in HEADER,"stale void prototype: "+obsolete)
for source,signature,label in ((COMMON,"restart_all_sysctl(void)","sysctl reload"),
                               (WAN,"start_wan(void)","WAN startup")):
    f=body(source,signature)
    for name in ("set_nf_conntrack","set_tcp_syncookies","set_igmp_mld_version"):
        require(name+"();" in f and "= "+name+"();" in f,label+" must capture "+name+" result")
    require(f.count("rc_first_service_start_status(")>=3,label+" must aggregate all three failures")
required=("trunk/user/rc/src/net.c","trunk/user/rc/src/common_ex.c",
 "trunk/user/rc/src/net_wan.c","trunk/user/rc/src/rc.h",
 "trunk/user/rc/src/network_sysctl_policy.h",
 "trunk/user/rc/src/tests/network_sysctl_policy.c","scripts/check_network_sysctl_status.py")
spl=WF.split("jobs:",1)[0].split("  pull_request:",1)
require(len(spl)==2,"WAN workflow must have push and PR filters")
for trigger,label in ((spl[0],"push"),(spl[1],"pull_request")):
    for path in required:
        require('"'+path+'"' in trigger,label+" filter missing "+path)
require("network-sysctl-policy" in WF,"WAN workflow must compile/run the policy regression")
require("python3 scripts/check_network_sysctl_status.py" in WF,"WAN workflow must run source checker")
xy=XY.split("  workflow_dispatch:",1)[0]
for path in ("trunk/user/rc/src/net.c","trunk/user/rc/src/rc.h",
             "trunk/user/rc/src/common_ex.c","trunk/user/rc/src/net_wan.c",
             "trunk/user/rc/src/network_sysctl_policy.h"):
    require('"'+path+'"' in xy,"XY-C1 filter missing "+path)
print("WAN startup/reload network sysctl result propagation: PASS")
