#!/usr/bin/env python3
"""Verify udhcpc event-script status propagation and lease-state gating."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative):
    return (ROOT / relative).read_text()


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    pos = brace
    state = "code"
    while pos < len(source):
        ch = source[pos]
        next_ch = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and next_ch == "*":
                state = "block_comment"
                pos += 1
            elif ch == "/" and next_ch == "/":
                state = "line_comment"
                pos += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:pos]
        elif state in ("string", "char"):
            if ch == "\\":
                pos += 1
            elif state == "string" and ch == '"':
                state = "code"
            elif state == "char" and ch == "'":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and next_ch == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)


udhcp_dir = "trunk/user/busybox/busybox-1.24.x/networking/udhcp/"
dhcpc = read(udhcp_dir + "dhcpc.c")
policy = read(udhcp_dir + "udhcpc_script_policy.h")
test = read(udhcp_dir + "tests/udhcpc_script_policy.c")
arpping_source = read(udhcp_dir + "arpping.c")
spawn = read("trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c")
libbb = read("trunk/user/busybox/busybox-1.24.x/include/libbb.h")
workflow = read(".github/workflows/udhcpc-script-status-stability.yml")

assert '#include "udhcpc_script_policy.h"' in dhcpc
assert 'udhcpc_subnet_mask_prefix_length(uint32_t mask, unsigned int *prefix_length)' in policy
assert 'test_subnet_mask_requires_contiguous_network_bits' in test
assert 'test_subnet_mask_requires_contiguous_network_bits() != 0' in test
assert '#define BMASK(i) (udhcpc_option_bitmap_mask((unsigned)(i)))' in dhcpc
assert '#define BITMAP unsigned' in dhcpc
fill_envp = function_body(dhcpc, "static char **fill_envp(struct dhcp_packet *packet)")
static_routes_guard = fill_envp.index(
    "udhcpc_static_routes_option_is_well_formed(temp, route_option_length)"
)
static_routes_invalid = fill_envp.index('xasprintf("%s=!invalid!", opt_name)')
static_routes_convert = fill_envp.index(
    "xmalloc_optname_optval(temp, &dhcp_optflags[i], opt_name)",
    static_routes_guard
)
assert "dhcp_optflags[i].flags & OPTION_STATIC_ROUTES" in fill_envp
assert "temp[-OPT_DATA + OPT_LEN]" in fill_envp
assert static_routes_guard < static_routes_invalid < static_routes_convert
assert "test_static_routes_reject_malformed_trailing_tuples" in test
assert "test_static_routes_reject_malformed_trailing_tuples() != 0" in test
assert 'return 1U << (option_code % (sizeof(unsigned int) * 8U));' in policy
assert 'test_option_bitmap_mask_uses_unsigned_high_bit' in test
assert 'test_option_bitmap_mask_uses_unsigned_high_bit() != 0' in test
runner = function_body(dhcpc, "static int udhcp_run_script")
assert "int status;" in runner
assert "status = spawn_and_wait(argv);" in runner
assert "if (status != 0)" in runner
assert "DHCP event script %s failed with status %d" in runner
assert "return status;" in runner
assert "static void udhcp_run_script" not in dhcpc
assert 'opt_complementary = "O::x::T+:t+:A+"' in dhcpc

spawn_body = function_body(spawn, "int FAST_FUNC spawn_and_wait(char **argv)")
assert "return wait4pid(rc);" in spawn_body
assert "int spawn_and_wait(char **argv) FAST_FUNC;" in libbb
assert "Returns sig + 0x180 if child is killed by signal." in libbb

ack_start = dhcpc.index("if (*message == DHCPACK)")
nak_start = dhcpc.index("if (*message == DHCPNAK)", ack_start)
ack = dhcpc[ack_start:nak_start]
subnet_validation = function_body(dhcpc, "static int udhcpc_subnet_mask_option_valid")
assert "udhcp_get_option(packet, DHCP_SUBNET)" in subnet_validation
assert "option[-OPT_DATA + OPT_LEN] != sizeof(subnet)" in subnet_validation
assert "udhcpc_subnet_mask_prefix_length(ntohl(subnet)" in subnet_validation
subnet_gate = ack.index("if (!udhcpc_subnet_mask_option_valid(&packet))")
subnet_retry = ack.index("udhcpc_script_failure_prepare_retry", subnet_gate)
subnet_continue = ack.index("continue;", subnet_retry)
lease_time_read = ack.index("udhcp_get_option32(&packet, DHCP_LEASE_TIME)")
assert subnet_gate < subnet_retry < subnet_continue < lease_time_read
script_call = ack.index("script_status = udhcp_run_script")
failure_gate = ack.index("if (script_status != 0)", script_call)
retry = ack.index("udhcpc_script_failure_prepare_retry", failure_gate)
retry_continue = ack.index("continue;", retry)
bound_publish = ack.index("state = BOUND;", retry_continue)
assert script_call < failure_gate < retry < retry_continue < bound_publish
assert "script_status != 0" in ack[failure_gate:retry_continue]
assert "state == REQUESTING" in ack[retry:retry_continue]
assert "tryagain_timeout" in ack[retry:retry_continue]
assert "script_elapsed" in ack[retry:retry_continue]
assert ack.index("requested_ip = packet.yiaddr;", retry_continue) > retry_continue
assert "already_waited_sec = script_elapsed;" in ack
assert "DHCPACK for %s" in ack

# The exported lease duration and internal timer must share one normalization.
fill_envp = function_body(dhcpc, "static char **fill_envp")
assert "if (code == DHCP_SUBNET)" in fill_envp
assert "udhcpc_subnet_mask_prefix_length(ntohl(subnet)" in fill_envp
assert 'xasprintf("mask=%u", subnet_prefix_length)' in fill_envp
assert "mton(subnet)" not in fill_envp
assert "if (code == DHCP_LEASE_TIME)" in fill_envp
assert "udhcp_get_option32(packet, code)" in fill_envp
assert "udhcpc_lease_time_effective(" in fill_envp
assert 'xasprintf("%s=%u", opt_name' in fill_envp
lease_ack_start = ack.index("temp = udhcp_get_option32(&packet, DHCP_LEASE_TIME);")
lease_ack = ack[lease_ack_start:ack.index("#if ENABLE_FEATURE_UDHCPC_ARPING", lease_ack_start)]
assert "udhcpc_lease_time_effective(0U, 0)" in lease_ack
assert "udhcpc_lease_time_effective(" in lease_ack
assert "lease_seconds < 0x10" not in lease_ack
assert "lease_seconds >= 0x10000000" not in lease_ack
assert "UDHCP_LEASE_TIME_DEFAULT_SECONDS 3600U" in policy
assert "UDHCP_LEASE_TIME_MIN_SECONDS 0x10U" in policy
assert "UDHCP_LEASE_TIME_MAX_SECONDS 0x0fffffffU" in policy
assert "test_effective_lease_time_bounds_match_client_timer" in test
assert "test_effective_lease_time_bounds_match_client_timer() != 0" in test

# A failed deconfig blocks NAK/expiry/release/manual-renew state transitions.
assert "#define DECONFIG_PENDING 7" in dhcpc
assert "static smallint deconfig_resume_state;" in dhcpc
scheduler = function_body(dhcpc, "udhcpc_schedule_deconfig_retry(")
assert "struct udhcpc_deconfig_retry_plan plan;" in scheduler
assert "udhcpc_deconfig_retry_plan_init(&plan, resume_state, release_on_usr2," in scheduler
assert "change_listen_mode(LISTEN_NONE)" in scheduler
assert "state = DECONFIG_PENDING;" in scheduler
assert "deconfig_resume_state = plan.resume_state;" in scheduler
assert "deconfig_release_on_usr2 = plan.release_on_usr2;" in scheduler
assert "*timeout = plan.timeout;" in scheduler
assert "*packet_num = plan.packet_num;" in scheduler
assert "*already_waited_sec = plan.already_waited_sec;" in scheduler
assert "static smallint deconfig_release_on_usr2;" in dhcpc
manual_renew = dhcpc[dhcpc.index("case SIGUSR1:"):dhcpc.index("case SIGUSR2:")]
assert "udhcpc_schedule_deconfig_retry(INIT_SELECTING, state == RENEW_REQUESTED," in " ".join(manual_renew.split())
pending = dhcpc.rindex("case DECONFIG_PENDING:")
pending_end = dhcpc.index("/* case RELEASED:", pending)
pending_body = dhcpc[pending:pending_end]
assert 'udhcp_run_script(NULL, "deconfig")' in pending_body
assert "udhcpc_deconfig_retry_required(deconfig_status)" in pending_body
assert "state = deconfig_resume_state;" in pending_body
assert "change_listen_mode(LISTEN_RAW)" in pending_body
nak_start = dhcpc.index('bb_info_msg("Received DHCP NAK")')
nak_end = dhcpc.index("change_listen_mode(LISTEN_RAW);", nak_start)
nak = dhcpc[nak_start:nak_end]
assert "udhcpc_deconfig_retry_required(deconfig_status)" in nak
assert "udhcpc_schedule_deconfig_retry(INIT_SELECTING" in nak


# A NAK during initial REQUESTING does not trigger deconfig; a NAK after an
# acquired lease must deconfigure before returning to INIT_SELECTING.
nak_branch_start = dhcpc.index("if (*message == DHCPNAK)")
nak_branch_end = dhcpc.index("/* case BOUND: - ignore all packets */", nak_branch_start)
nak_branch = dhcpc[nak_branch_start:nak_branch_end]
assert "udhcpc_nak_requires_deconfig(state == REQUESTING)" in nak_branch
assert "udhcpc_nak_requires_deconfig(int requesting)" in policy
assert "test_nak_only_deconfigures_an_active_lease" in test
assert "test_nak_only_deconfigures_an_active_lease() != 0" in test
nak_branch_compact = "".join(nak_branch.split())
policy_compact = "".join(policy.split())
test_compact = "".join(test.split())
nak_id_read = nak_branch_compact.index("temp=udhcp_get_option32(&packet,DHCP_SERVER_ID)")
nak_id_check = nak_branch_compact.index("udhcpc_nak_server_id_rejected(state==REBINDING,server_addr,temp!=NULL,svid)")
nak_event = nak_branch_compact.index('udhcp_run_script(&packet,"nak")')
nak_deconfig = nak_branch_compact.index('udhcp_run_script(NULL,"deconfig")')
nak_retry = nak_branch_compact.index("udhcpc_schedule_deconfig_retry(INIT_SELECTING", nak_deconfig)
assert nak_id_read < nak_id_check < nak_event < nak_deconfig < nak_retry
assert "received DHCP NAK with wrong" in nak_branch
assert ("staticinlineintudhcpc_nak_server_id_rejected(intrebinding,uint32_texpected_server_id,"
        "intserver_id_present,uint32_treceived_server_id)") in policy_compact
assert "if(rebinding)return!server_id_present;returnexpected_server_id!=0&&(!server_id_present||received_server_id!=expected_server_id)" in policy_compact
assert "test_nak_server_id_filter_respects_rebinding_broadcast" in test
assert "test_nak_server_id_filter_respects_rebinding_broadcast()!=0" in test_compact
assert "udhcpc_nak_requires_deconfig(state==REQUESTING)" in nak_branch_compact[nak_event:nak_retry]

# A REBINDING request is broadcast to any server; don't pin its DHCPNAK to
# the server that issued the original lease. Missing server IDs stay invalid.
assert "state == REBINDING, server_addr" in nak_branch
assert "if (rebinding)" in policy
assert "return !server_id_present;" in policy
assert "test_nak_server_id_filter_respects_rebinding_broadcast" in test

# leasefail is advisory: its exit status must not suppress retry/background/
# abort behavior after DHCP discovery is exhausted.
leasefail_start = dhcpc.index("\n leasefail:")
leasefail_end = dhcpc.index("\n\t\t\tcase REQUESTING:", leasefail_start)
leasefail = dhcpc[leasefail_start:leasefail_end]
assert 'udhcp_run_script(NULL, "leasefail");' in leasefail
assert 'script_status = udhcp_run_script(NULL, "leasefail")' not in leasefail
assert "timeout = tryagain_timeout;" in leasefail
assert "packet_num = 0;" in leasefail
assert "continue;" in leasefail
assert leasefail.index('udhcp_run_script(NULL, "leasefail");') < leasefail.index("timeout = tryagain_timeout;") < leasefail.index("continue;")

arpping_body = function_body(arpping_source, "int FAST_FUNC arpping(")
assert "Returns 1 for no conflict, 0 for conflict, and -1 on probe I/O error." in arpping_source
assert "rv = -1;" in arpping_body
assert "send_result != (int)sizeof(arp)" in arpping_body
assert "if (r < 0)" in arpping_body
assert "if (r <= 0)" in arpping_body

arp_start = dhcpc.index("arp_probe_status = arpping(")
arp_end = dhcpc.index("temp_addr.s_addr = packet.yiaddr", arp_start)
arp_conflict = dhcpc[arp_start:arp_end]
assert "udhcpc_arp_probe_failed(arp_probe_status)" in arp_conflict
assert "udhcpc_script_failure_prepare_retry(state == REQUESTING" in arp_conflict
assert arp_conflict.index("udhcpc_arp_probe_failed(arp_probe_status)") < arp_conflict.index("if (arp_probe_status == 0)") < arp_conflict.index("send_decline(server_addr, packet.yiaddr)")
assert "udhcpc_arp_probe_failed(int probe_status)" in policy
assert "test_arp_probe_errors_do_not_publish_a_dhcp_lease" in test
assert "test_arp_probe_errors_do_not_publish_a_dhcp_lease() != 0" in test

assert "decline_status = send_decline(server_addr, packet.yiaddr);" in arp_conflict
assert "udhcpc_packet_send_failed(decline_status)" in arp_conflict
assert "decline_server_addr = server_addr;" in arp_conflict
assert "decline_requested_ip = packet.yiaddr;" in arp_conflict
assert "DECLINE_PENDING : INIT_SELECTING" in " ".join(arp_conflict.split())
assert "state = DECLINE_PENDING;" in arp_conflict
assert "udhcpc_decline_retry_delay(tryagain_timeout)" in arp_conflict
assert "udhcpc_packet_send_failed(int send_status)" in policy
assert "udhcpc_decline_retry_delay(int retry_timeout)" in policy
assert "test_failed_packet_send_blocks_dhcp_reselection_until_retry" in test
assert "if (state != REQUESTING)" in arp_conflict
assert "udhcpc_deconfig_retry_required(deconfig_status)" in arp_conflict
assert "udhcpc_schedule_deconfig_retry(resume_state, 0," in arp_conflict

decline_start = dhcpc.index("case DECLINE_PENDING:")
decline_end = dhcpc.index("/* case RELEASED:", decline_start)
decline_pending = dhcpc[decline_start:decline_end]
assert "send_decline(decline_server_addr" in decline_pending
assert "decline_requested_ip" in decline_pending
assert "udhcpc_packet_send_failed(decline_status)" in decline_pending
assert "timeout = udhcpc_decline_retry_delay(tryagain_timeout);" in decline_pending
assert decline_pending.index("send_decline(") < decline_pending.index("state = INIT_SELECTING;")
resume_pending = pending_body[pending_body.index("state = deconfig_resume_state;"):]
assert "else if (state == DECLINE_PENDING)" in resume_pending
assert "timeout = 0;" in resume_pending
assert arp_conflict.index("udhcpc_schedule_deconfig_retry") < arp_conflict.index("change_listen_mode(LISTEN_RAW)")
expiry_start = dhcpc.index('bb_info_msg("Lease lost, entering init state")')
expiry_end = dhcpc.index("case DECONFIG_PENDING:", expiry_start)
expiry = dhcpc[expiry_start:expiry_end]
assert "udhcpc_deconfig_retry_required(deconfig_status)" in expiry
assert "udhcpc_schedule_deconfig_retry(INIT_SELECTING" in expiry
renew = function_body(dhcpc, "static int perform_renew")
assert "return script_status;" in renew
release = function_body(dhcpc, "static int perform_release")
assert "udhcpc_deconfig_pending_release_required(deconfig_release_on_usr2, 1)" in release
assert "release_status = send_release(server_addr, requested_ip);" in release
assert "udhcpc_packet_send_failed(release_status)" in release
assert "if (release_status != 0)" not in release
assert "DHCP release packet send failed with status %d" in release
assert "deconfig_release_on_usr2 = 0;" in release
assert "return script_status;" in release
assert "if (!udhcpc_deconfig_retry_required(script_status))" in release
signal_release = dhcpc[dhcpc.index("case SIGUSR2:"):dhcpc.index("/* Is it a packet? */")]
assert "if (state == DECONFIG_PENDING)" in signal_release
assert "script_status = perform_release(server_addr, requested_ip)" in signal_release
assert "udhcpc_schedule_deconfig_retry(RELEASED, 0," in " ".join(signal_release.split())
pending_signal = signal_release[signal_release.index("if (state == DECONFIG_PENDING)"): ]
assert pending_signal.index("script_status = perform_release(server_addr, requested_ip)") < pending_signal.index("udhcpc_schedule_deconfig_retry(RELEASED, 0,") < pending_signal.index("continue;")
assert "udhcpc_deconfig_pending_release_required" in policy
assert "test_pending_deconfig_manual_stop_releases_existing_lease" in test

exit_cleanup = dhcpc[dhcpc.index(" ret0:"):]

# One tested retry-delay policy is shared by scheduler, pending cleanup,
# decline retry, and process-exit cleanup.
assert "udhcpc_retry_delay(int retry_timeout)" in policy
assert "returnretry_timeout>0?retry_timeout:1;" in policy_compact
assert "udhcpc_deconfig_retry_plan_init(&plan, resume_state, release_on_usr2," in scheduler
assert "timeout=udhcpc_retry_delay(tryagain_timeout);" in "".join(pending_body.split())
assert "intretry_delay=udhcpc_retry_delay(tryagain_timeout);" in "".join(exit_cleanup.split())
assert "returnudhcpc_retry_delay(retry_timeout);" in policy_compact
assert "test_cleanup_retry_delay_has_one_second_floor" in test
assert "test_cleanup_retry_delay_has_one_second_floor()!=0" in test_compact

scheduler_compact = "".join(scheduler.split())
assert (scheduler_compact.index("udhcpc_deconfig_retry_plan_init(&plan,resume_state,release_on_usr2,retry_timeout);") <
        scheduler_compact.index("change_listen_mode(LISTEN_NONE);") <
        scheduler_compact.index("deconfig_resume_state=plan.resume_state;") <
        scheduler_compact.index("deconfig_release_on_usr2=plan.release_on_usr2;") <
        scheduler_compact.index("state=DECONFIG_PENDING;") <
        scheduler_compact.index("*timeout=plan.timeout;") <
        scheduler_compact.index("*packet_num=plan.packet_num;") <
        scheduler_compact.index("*already_waited_sec=plan.already_waited_sec;"))
assert "structudhcpc_deconfig_retry_plan" in policy_compact
assert "plan->resume_state=resume_state;" in policy_compact
assert "plan->release_on_usr2=release_on_usr2!=0;" in policy_compact
assert "plan->timeout=udhcpc_retry_delay(retry_timeout);" in policy_compact
assert "plan->packet_num=0;" in policy_compact
assert "plan->already_waited_sec=0;" in policy_compact
assert "test_deconfig_retry_plan_resets_state_and_timer" in test
assert "test_deconfig_retry_plan_resets_state_and_timer()!=0" in test_compact

pending_compact = "".join(pending_body.split())
pending_fail = pending_compact.index("if(udhcpc_deconfig_retry_required(deconfig_status))")
pending_continue = pending_compact.index("continue;", pending_fail)
pending_resume = pending_compact.index("state=deconfig_resume_state;", pending_continue)
pending_clear = pending_compact.index("deconfig_release_on_usr2=0;", pending_resume)
assert pending_fail < pending_continue < pending_resume < pending_clear
assert "timeout=udhcpc_retry_delay(tryagain_timeout);" in pending_compact[pending_fail:pending_continue]
resume_init = pending_compact.index("if(state==INIT_SELECTING)", pending_clear)
assert "change_listen_mode(LISTEN_RAW);" in pending_compact[resume_init:]
assert "client_config.first_secs=0;" in pending_compact[resume_init:]
assert "requested_ip=0;" in pending_compact[resume_init:]
assert "packet_num=0;" in pending_compact[resume_init:]
assert "timeout=0;" in pending_compact[resume_init:]
resume_decline = pending_compact.index("elseif(state==DECLINE_PENDING)", resume_init)
resume_other = pending_compact.index("else{", resume_decline)
assert "change_listen_mode(LISTEN_NONE);" in pending_compact[resume_other:]
assert "timeout=INT_MAX;" in pending_compact[resume_other:]

expiry_compact = "".join(expiry.split())
expiry_deconfig = expiry_compact.index('udhcp_run_script(NULL,"deconfig")')
expiry_retry = expiry_compact.index("udhcpc_schedule_deconfig_retry(INIT_SELECTING", expiry_deconfig)
expiry_continue = expiry_compact.index("continue;", expiry_retry)
expiry_resume = expiry_compact.index("state=INIT_SELECTING;", expiry_continue)
assert expiry_deconfig < expiry_retry < expiry_continue < expiry_resume
assert "change_listen_mode(LISTEN_RAW);" in expiry_compact[expiry_resume:]

manual_renew_compact = "".join(manual_renew.split())
manual_renew_call = manual_renew_compact.index("script_status=perform_renew();")
manual_renew_retry = manual_renew_compact.index(
    "udhcpc_schedule_deconfig_retry(INIT_SELECTING,state==RENEW_REQUESTED,",
    manual_renew_call
)
manual_renew_continue = manual_renew_compact.index("continue;", manual_renew_retry)
manual_renew_success = manual_renew_compact.index("if(state==RENEW_REQUESTED)", manual_renew_continue)
assert manual_renew_call < manual_renew_retry < manual_renew_continue < manual_renew_success

release_compact = "".join(release.split())
pending_release_gate = release_compact.index(
    "udhcpc_deconfig_pending_release_required(deconfig_release_on_usr2,1)"
)
release_send = release_compact.index("release_status=send_release(server_addr,requested_ip);")
release_flag_clear = release_compact.index("deconfig_release_on_usr2=0;")
release_deconfig = release_compact.index('script_status=udhcp_run_script(NULL,"deconfig");')
assert release_compact.index("if(state==DECONFIG_PENDING)") < pending_release_gate < release_send
assert release_send < release_flag_clear < release_deconfig
assert "if(script_status){intrelease_status;" in release_compact[:release_send]
assert release_compact.index("if(!udhcpc_deconfig_retry_required(script_status))") < release_compact.index("state=RELEASED;")
assert release_compact.index("if(!udhcpc_deconfig_retry_required(script_status))") < release_compact.index("state=RELEASED;")

signal_release_compact = "".join(signal_release.split())
pending_signal_start = signal_release_compact.index("if(state==DECONFIG_PENDING)")
pending_signal_branch = signal_release_compact[pending_signal_start:]
assert pending_signal_branch.index("script_status=perform_release(server_addr,requested_ip);") < pending_signal_branch.index(
    "udhcpc_schedule_deconfig_retry(RELEASED,0,"
) < pending_signal_branch.index("continue;")
assert "timeout=INT_MAX;" in pending_signal_branch

assert "if ((opt & OPT_R) || state == DECONFIG_PENDING)" in exit_cleanup
assert "if (opt & OPT_R)" in exit_cleanup
assert "release_status = perform_release(server_addr, requested_ip);" in exit_cleanup
assert "while (udhcpc_deconfig_retry_required(release_status))" in exit_cleanup
assert "sleep(retry_delay);" in exit_cleanup
assert 'release_status = udhcp_run_script(NULL, "deconfig");' in exit_cleanup
exit_compact = "".join(exit_cleanup.split())
assert (exit_compact.index("if(opt&OPT_R)") <
        exit_compact.index("release_status=perform_release(server_addr,requested_ip);") <
        exit_compact.index("while(udhcpc_deconfig_retry_required(release_status))") <
        exit_compact.index("sleep(retry_delay);") <
        exit_compact.rindex('release_status=udhcp_run_script(NULL,"deconfig");'))

assert "if (acquiring)" in policy
assert "*timeout = retry_timeout > 0 ? retry_timeout : 1;" in policy
assert "A+" in dhcpc
assert "*packet_num = 0;" in policy
assert "script_elapsed >= remaining" in policy
assert "*already_waited_sec += script_elapsed;" in policy
assert "return script_status != 0;" in policy
assert "test_failed_deconfig_blocks_state_transition" in test

for marker in (
    "test_initial_acquisition_retries_selected_request",
    "test_nonpositive_retry_interval_is_throttled",
    "test_failed_renewal_preserves_existing_lease_deadline",
    "test_script_runtime_cannot_extend_expired_lease",
    "test_exact_lease_deadline_is_expired",
    "test_already_expired_lease_is_not_resurrected",
    "udhcpc script failure and deconfig retry policy: PASS",
):
    assert marker in test, marker

paths = (
    ".github/workflows/udhcpc-script-status-stability.yml",
    "scripts/check_udhcpc_script_status.py",
    udhcp_dir + "dhcpc.c",
    udhcp_dir + "arpping.c",
    udhcp_dir + "udhcpc_script_policy.h",
    udhcp_dir + "tests/udhcpc_script_policy.c",
    "trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c",
    "trunk/user/busybox/busybox-1.24.x/include/libbb.h",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

print("udhcpc event-script status and lease-state source contracts: PASS")
