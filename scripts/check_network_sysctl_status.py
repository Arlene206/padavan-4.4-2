#!/usr/bin/env python3
"""Check status propagation for WAN network sysctl operations."""
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
RC=ROOT/"trunk/user/rc/src"
NET=(RC/"net.c").read_text()
COMMON=(RC/"common_ex.c").read_text()
WAN=(RC/"net_wan.c").read_text()
HEADER=(RC/"network_sysctl.h").read_text()
RC_HEADER=(RC/"rc.h").read_text()
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
tcp=body(NET,"set_tcp_syncookies(int enabled, int *syncookies_status)")
require("rc_network_sysctl_apply(3, write_tcp_syncookies_operation, &context)" in tcp,
        "all three TCP/ICMP settings must be attempted")
tcp_writer=body(NET,"write_tcp_syncookies_operation(")
for token in ('/proc/sys/net/ipv4/tcp_syncookies',
              '/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses',
              '/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts'):
    require(token in tcp_writer,"missing TCP/ICMP sysctl write: "+token)
require("context.enabled = enabled ? 1 : 0;" in tcp and
        "context->syncookies_status" in tcp_writer and
        "*syncookies_status = context.syncookies_status" in tcp,
        "exact SYN-cookie operation status must be returned separately")
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
require("read_result < 0" in POLICY and "read_result > 1" in POLICY,
        "SYN-cookie policy parsing must reject failed/invalid NVRAM read results")
require("return connected" not in POLICY,"unrelated readiness policy must not be included")
require("mock_status[0] = EIO" in TEST and "mock_status[3] = EACCES" in TEST,
        "regression must verify first-error and continue-after-failure")
require("assert(mock_calls == 4)" in TEST,"regression must assert all independent operations ran")
require("test_configured_syncookies_gate" in TEST and
        "rc_network_sysctl_tcp_syncookies_allows_wan(1, EIO)" in TEST,
        "regression must cover explicitly enabled SYN-cookie failure")
for proto in ("int set_nf_conntrack(void);","int set_tcp_syncookies(int enabled, int *syncookies_status);","int set_igmp_mld_version(void);"):
    require(proto in HEADER,"missing status-returning prototype: "+proto)
for obsolete in ("void set_nf_conntrack(void);","void set_tcp_syncookies(void);","void set_igmp_mld_version(void);"):
    require(obsolete not in RC_HEADER,"stale void prototype remains in rc.h: "+obsolete)
sysctl_reload=body(COMMON,"restart_all_sysctl(void)")
syncookies_reload_compact = "".join(sysctl_reload.split())
require("int syncookies_enabled = 1;" in sysctl_reload and
        'syncookies_read_result=nvram_get_checked("fw_syn_cook",&syncookies_value);' in syncookies_reload_compact and
        "rc_network_sysctl_parse_syncookies_status(" in syncookies_reload_compact and
        "syncookies_status=operation_status;" in syncookies_reload_compact,
        "sysctl reload must distinguish NVRAM failure from an explicit SYN-cookie disable")
read_pos = syncookies_reload_compact.index('syncookies_read_result=nvram_get_checked("fw_syn_cook",&syncookies_value);')
parse_pos = syncookies_reload_compact.index("operation_status=rc_network_sysctl_parse_syncookies_status(", read_pos)
write_pos = syncookies_reload_compact.index("operation_status=set_tcp_syncookies(syncookies_enabled,&syncookies_status);", parse_pos)
require(read_pos < parse_pos < write_pos and
        syncookies_reload_compact.index("if(operation_status!=0){", parse_pos) < write_pos,
        "sysctl reload must gate writes on a successful checked policy read")
require('nvram_get_int("fw_syn_cook")' not in sysctl_reload,
        "unchecked SYN-cookie NVRAM reads must be removed from reload path")
require("rc_network_sysctl_parse_syncookies_status" in POLICY and
        "test_syncookies_checked_read_failure_does_not_disable_policy" in TEST,
        "policy tests must cover checked-read errors without lowering the WAN gate")
require("if (!rc_network_sysctl_tcp_syncookies_allows_wan(" in sysctl_reload and
        "syncookies_enabled, syncookies_status)" in sysctl_reload,
        "runtime sysctl reload must enforce the same configured SYN-cookie prerequisite")
require("stop_wan_status = stop_wan();" in sysctl_reload,
        "runtime SYN-cookie failure must stop the existing WAN")
require(sysctl_reload.index("stop_wan_status = stop_wan();") <
        sysctl_reload.index("passthrough_status = set_passthrough_pppoe(1);"),
        "runtime failure must not restart PPPoE passthrough after WAN stop")
wan_start=body(WAN,"start_wan(void)")
for name in ("set_nf_conntrack","set_igmp_mld_version"):
    require(name+"();" in wan_start and "= "+name+"();" in wan_start,
            "WAN startup must capture "+name+" result")
wan_start_compact = "".join(wan_start.split())
syncookie_read = 'nvram_status=get_nvram_int_checked_status("fw_syn_cook",&tcp_syncookies_enabled);'
syncookie_write = "sysctl_status=set_tcp_syncookies(tcp_syncookies_enabled,&tcp_syncookies_status);"
require(syncookie_read in wan_start_compact and syncookie_write in wan_start_compact,
        "WAN startup must use checked SYN-cookie reads and capture exact write status")
syncookie_read_pos = wan_start_compact.index(syncookie_read)
syncookie_error_pos = wan_start_compact.index("if(nvram_status!=0)", syncookie_read_pos)
syncookie_return_pos = wan_start_compact.index("returnnvram_status;", syncookie_error_pos)
syncookie_write_pos = wan_start_compact.index(syncookie_write, syncookie_read_pos)
require(syncookie_read_pos < syncookie_error_pos < syncookie_return_pos < syncookie_write_pos,
        "WAN startup must abort on SYN-cookie NVRAM read errors before applying sysctls")
require("int tcp_syncookies_status = 0;" in wan_start,
        "WAN startup must initialize dedicated SYN-cookie status")
wan_compact=" ".join(wan_start.split())
require("rc_network_sysctl_tcp_syncookies_allows_wan( tcp_syncookies_enabled, tcp_syncookies_status)" in wan_compact,
        "configured SYN-cookie failure must block WAN")
require(wan_start.index("sysctl_status = set_tcp_syncookies") < wan_start.index("if (create_cb_links() != 0)"),
        "SYN-cookie admission must run before callback links and WAN side effects")
require("return rc_first_service_start_status(wan_start_status, tcp_syncookies_status);" in wan_start,
        "WAN rejection must preserve exact SYN-cookie error")
require(wan_start.count("set_tcp_syncookies(")==1,
        "WAN startup must not duplicate or defer SYN-cookie setting")
require(wan_start.count("rc_first_service_start_status(")>=3,
        "WAN startup must preserve independent sysctl failures")
required=("trunk/user/rc/src/net.c","trunk/user/rc/src/common_ex.c",
 "trunk/user/rc/src/net_wan.c","trunk/user/rc/src/rc.h",
 "trunk/user/rc/src/network_sysctl.h",
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
