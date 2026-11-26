#!/usr/bin/env python3
"""Check DHCPv6 IA publication retry and partial-prefix reconciliation contracts."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
BASE = "trunk/user/busybox/busybox-1.24.x/networking/udhcp/"


def read(path):
    return (ROOT / path).read_text()


def function_body(source, signature):
    # Prefer the definition over an earlier forward declaration.
    start = source.rindex(signature)
    brace = source.index("{", start)
    depth, state, pos = 0, "code", brace
    while pos < len(source):
        ch = source[pos]
        nxt = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and nxt == "*":
                state = "block_comment"
                pos += 1
            elif ch == "/" and nxt == "/":
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
            if ch == "*" and nxt == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)


ia = read(BASE + "dhcp6c_ia.c")
client = read(BASE + "dhcp6c.c")
socket = read(BASE + "dhcp6c_socket.c")
header = read(BASE + "dhcp6c.h")
policy = read(BASE + "dhcp6c_ia_policy.h")
config_header = read(BASE + "dhcp6c_config.h")
test = read(BASE + "tests/dhcp6c_ia_publication_policy.c")
config = read(BASE + "dhcp6c_config.c")
iaid_policy = read(BASE + "dhcp6c_iaid_policy.h")
iaid_test = read(BASE + "tests/dhcp6c_iaid_policy.c")
ia_source = read(BASE + "dhcp6c_ia.c")
sla_policy = read(BASE + "dhcp6c_pd_sla_policy.h")
sla_test = read(BASE + "tests/dhcp6c_pd_sla_policy.c")
config_cleanup = read(BASE + "dhcp6c_config_cleanup_policy.h")
config_cleanup_test = read(BASE + "tests/dhcp6c_config_cleanup_policy.c")
common = read(BASE + "dhcp6c_common.c")
bounds_policy = read(BASE + "dhcp6c_option_bounds_policy.h")
timer_policy = read(BASE + "dhcp6c_timer_policy.h")
timer_test = read(BASE + "tests/dhcp6c_timer_traversal_policy.c")
bounds_test = read(BASE + "tests/dhcp6c_option_bounds_policy.c")
xy_workflow = read(".github/workflows/xy-c1-firmware-validation.yml")
workflow = read(".github/workflows/dhcp6c-ia-publication-stability.yml")

assert '#include "dhcp6c_ia_policy.h"' in ia
assert '#include "dhcp6c_ia_policy.h"' in client
assert "int update_ia(struct dhcp6_list *iahead" in ia
assert "int update_ia(struct dhcp6_list *iahead" in header
assert "dhcp6c_ia_pd_target_needs_apply" in policy
assert "dhcp6c_ia_first_error" in policy
assert "dhcp6c_ia_retry_required" in policy
assert "dhcp6c_ia_ifaddr_change_complete" in policy
assert "dhcp6c_ia_prefix_expiry_timer_required" in policy
assert "dhcp6c_ia_preferred_lifetime_remaining" in policy
assert "test_preferred_lifetime_uses_monotonic_elapsed_time" in test
assert "unsigned long long updatetime_ms;" in ia
assert "time_t updatetime;" not in ia
pref_duration = function_body(ia, "static uint32_t pref_duration(struct iactl *iac)")
assert "monotonic_ms()" in pref_duration
assert "sp->updatetime_ms" in pref_duration
assert "dhcp6c_ia_preferred_lifetime_remaining(" in pref_duration
assert "time(NULL)" not in pref_duration
assert "sp->updatetime_ms = monotonic_ms();" in ia

assert '#include "dhcp6c_iaid_policy.h"' in config
assert "dhcp6c_iaid_parse" in iaid_policy
assert "0xffffffffUL" in iaid_policy
assert "atoi(iap->ptr)" not in config
configure_ia = function_body(config, "static int configure_ia(void)")
iaid_parse = configure_ia.index("dhcp6c_iaid_parse(iap->ptr, &iaid)")
iaid_alloc = configure_ia.index("iac = xzalloc(sizeof(struct ia_conf));")
iaid_publish = configure_ia.index("TAILQ_INSERT_TAIL(&client6_config.ia_allconflist, iac, link);")
assert iaid_parse < iaid_alloc < iaid_publish
assert "client6_config.conffile, iap->line" in configure_ia
assert "test_decimal_iaid_boundaries" in iaid_test
assert "test_malformed_iaid_is_rejected_without_partial_output" in iaid_test
assert "test_output_pointer_is_required" in iaid_test
assert "DHCPv6 IAID strict decimal parsing policy: PASS" in iaid_test
assert '#include "dhcp6c_pd_sla_policy.h"' in ia_source
assert '#include "dhcp6c_pd_sla_policy.h"' in config
assert "dhcp6c_pd_sla_id_valid" in sla_policy
assert "dhcp6c_pd_sla_insert" in sla_policy
assert "dhcp6c_pd_prefix_clear_host_bits" in sla_policy
add_ifprefix = function_body(ia_source, "static int add_ifprefix(struct iaprefix *iaprefix,")
assert "dhcp6c_pd_prefix_clear_host_bits(ifpfx->paddr.s6_addr," in add_ifprefix
assert add_ifprefix.index("dhcp6c_pd_prefix_clear_host_bits(") < add_ifprefix.index("dhcp6c_pd_sla_insert(")
assert "test_delegated_prefix_clears_host_bits_at_byte_boundaries" in sla_test
assert "test_sla_insertion_preserves_only_canonical_prefix_bits" in sla_test
assert "test_prefix_canonicalization_rejects_invalid_input_atomically" in sla_test
assert "dhcp6c_pd_sla_insert(" in add_ifprefix
assert "htonl(pconf->sla_id)" not in add_ifprefix
assert "sp = ((char *)&sla_id + 3)" not in add_ifprefix
add_pd_pif = function_body(config, "static int add_pd_pif(struct ia_conf *iac,")
assert "pif->sla_len > 32" in add_pd_pif
assert "dhcp6c_pd_sla_id_valid(pif->sla_len, pif->sla_id)" in add_pd_pif
assert "test_partial_sla_field_is_network_bit_aligned" in sla_test
assert "test_byte_aligned_and_full_width_sla_fields" in sla_test
assert "test_invalid_sla_configurations_are_rejected_atomically" in sla_test
assert "DHCPv6 delegated-prefix SLA bit placement policy: PASS" in sla_test
assert "tests/dhcp6c_pd_sla_policy.c" in workflow

assert '#include "dhcp6c_config_cleanup_policy.h"' in config
assert "dhcp6c_config_cleanup_prefix_list" in config_cleanup
assert "test_ia_na_prefix_list_is_released" in config_cleanup_test
assert "test_ia_pd_prefix_list_is_released" in config_cleanup_test
assert "test_unrelated_ia_type_does_not_release_unowned_list" in config_cleanup_test
clear_pd_pif = function_body(config, "static void clear_pd_pif(")
assert "dhcp6_clear_list(&iac->prefix_list)" not in clear_pd_pif
clear_prefix_adapter = function_body(config, "clear_dhcp6_config_prefix_list(void *context)")
assert "dhcp6_clear_list((struct dhcp6_list *)context);" in clear_prefix_adapter
clear_iaconf = function_body(config, "static void clear_iaconf(")
cleanup_call = clear_iaconf.index("dhcp6c_config_cleanup_prefix_list(iac->type")
assert clear_iaconf.index("switch (iac->type)") < cleanup_call < clear_iaconf.index("free(iac);")
assert "DHCPOPT_IA_NA, DHCPOPT_IA_PD" in clear_iaconf
assert "tests/dhcp6c_config_cleanup_policy.c" in workflow
assert "tests/dhcp6c_config_cleanup_policy.c" in xy_workflow
assert "tests/dhcp6c_iaid_policy.c" in workflow
assert "DHCP6C_IA_CLEANUP_RETRY_DELAY_MS == 5000ULL" in test

installed = function_body(ia, "static int ifprefix_is_installed(struct iaprefix *sp, struct prefix_ifconf *pif)")
assert "TAILQ_FOREACH(ip, &sp->u.ifprefix_list, plink)" in installed
assert "ip->ifconf == pif" in installed

update_prefix = function_body(ia, "static int update_prefix6(struct ia *ia,")
add_ifprefix = function_body(ia, "static int add_ifprefix(struct iaprefix *iaprefix,")
assert "if (lv->lvtype == DHCP6_LISTVAL_PREFIX6)" in update_prefix
assert "ifprefix_is_installed(sp, pif)" in update_prefix
assert "dhcp6c_ia_pd_target_needs_apply(" in update_prefix
assert "if (add_ifprefix(sp, &pinfo, pif) != 0)" in update_prefix
assert "&& spcreate" not in update_prefix
assert "errno = EINVAL;" in update_prefix
expiry_policy = update_prefix.index("dhcp6c_ia_prefix_expiry_timer_required(")
expiry_timer_add = update_prefix.index("dhcp6_timer_add(iapref_timo, sp)", expiry_policy)
expiry_timer_set = update_prefix.index("dhcp6_timer_set(sp->pa.vltime * 1000ULL, sp->timer)", expiry_timer_add)
assert expiry_policy < expiry_timer_add < expiry_timer_set
assert expiry_timer_set < update_prefix.index("add_ifprefix(sp, &pinfo, pif)")
assert expiry_timer_set < update_prefix.index("ifaddrconf(IFADDRCONF_ADD,")
zero_lifetime = update_prefix.index("if (sp->pa.vltime == 0)")
zero_cleanup = update_prefix.index("remove_iaprefix(sp,", zero_lifetime)
zero_return = update_prefix.index("return 0;", zero_cleanup)
reconcile_start = update_prefix.index("/* Reconcile all configured downstream interfaces", zero_return)
assert zero_lifetime < zero_cleanup < zero_return < reconcile_start

assert "dhcp6c_ia_t2_from_duration(duration)" in ia
assert "dhcp6c_ia_t1_from_t2(ia->t2)" in ia
assert "duration * 4 / 5" not in ia
assert "ia->t2 * 5 / 8" not in ia
assert "test_default_ia_timer_ratios_do_not_overflow" in test
assert "0xfffffffeU" in test and "2000000000U" in test

update_ia = function_body(ia, "int update_ia(struct dhcp6_list *iahead")
assert "int ia_error = 0, update_status;" in update_ia
assert "dhcp6c_ia_first_error(ia_error" in update_ia
assert "first_error = dhcp6c_ia_first_error(first_error, ia_error);" in update_ia
assert "if (ia_error != 0)" in update_ia
assert update_ia.index("if (ia_error != 0)") < update_ia.index("if (ia->ctl == NULL || !pref_isvalid(ia->ctl))")
assert update_ia.index("if (ia_error != 0)") < update_ia.index("dhcp6_timer_set(ia->t1 * 1000ULL, ia->timer)")
assert "return first_error;" in update_ia

# Both DHCPv6 option decoders validate lengths before forming the option-end
# pointer, and reject any truncated trailing header instead of silently
# accepting the packet/suboption list.
assert '#include "dhcp6c_option_bounds_policy.h"' in common
assert '#include "dhcp6c_option_bounds_policy.h"' in client
assert "dhcp6c_option_wire_length_valid" in bounds_policy
assert "DHCPv6 option bounds and NTP suboption policy: PASS" in bounds_test
assert "dhcp6c_datagram_length_fits" in bounds_policy
assert "dhcp6c_option_vbuf_is_unset" in bounds_policy
assert "Singleton option buffer policy" in bounds_test
outer_options = function_body(common, "int dhcp6_get_options(")
nested_options = function_body(common, "static int copyin_option(int type, struct dhcp6opt *p,")
outer_options_compact = "".join(outer_options.split())
for option_buffer in (
        "optinfo->clientID.dv_buf,optinfo->clientID.dv_len",
        "optinfo->serverID.dv_buf,optinfo->serverID.dv_len",
        "optinfo->relaymsg_msg,optinfo->relaymsg_len",
        "optinfo->ifidopt_id,optinfo->ifidopt_len"):
    assert ("dhcp6c_option_vbuf_is_unset(" + option_buffer + ")") in outer_options_compact, (
        "duplicate DHCPv6 singleton vbuf must be rejected before reallocation: " +
        option_buffer)
assert outer_options.count("dhcp6c_option_vbuf_is_unset(") == 4
assert "dhcp6_clear_options(optinfo);" in outer_options

reconf_accept_start = outer_options.index("case D6_OPT_RECONF_ACCEPT:")
reconf_accept_end = outer_options.index("default:", reconf_accept_start)
reconf_accept = outer_options[reconf_accept_start:reconf_accept_end]
assert ("dhcp6c_option_zero_payload_length_valid((size_t)optlen)" in
        "".join(reconf_accept.split()))
assert "Reconfigure Accept has a mandatory zero-length payload" in bounds_test
ntp_parser = function_body(common, "static int dhcp6_get_ntp_server(")
ntp_parser_compact = "".join(ntp_parser.split())
assert "[D6_OPT_NTP_SERVER]     = { 0, 0 }" in common
assert "D6_NTP_SUBOPT_SRV_ADDR" in ntp_parser
assert "D6_NTP_SUBOPT_SRV_MC_ADDR" in ntp_parser
assert "D6_NTP_SUBOPT_SRV_FQDN" in ntp_parser
assert "dhcp6c_ntp_suboption_wire_length_valid(remaining,sublen)" in ntp_parser_compact
assert "dhcp6c_ntp_fqdn_wire_valid((constunsignedchar*)p,sublen)" in ntp_parser_compact
assert "dhcp6c_ntp_fqdn_wire_valid" in bounds_policy
assert "multiple_names" in bounds_test and "idna_alabel" in bounds_test
assert "dhcp6c_ntp_suboption_payload_length_valid(suboption,sublen)" in ntp_parser_compact
assert "dhcp6c_ntp_address_kind_valid(suboption,IN6_IS_ADDR_MULTICAST(&address))" in ntp_parser_compact
assert "dhcp6c_ntp_time_source_claim(suboption,&source_seen)" in ntp_parser_compact
assert "returndhcp6c_ntp_time_source_present(source_seen)?0:-1;" in ntp_parser_compact
assert "dhcp6_get_domain((int)sublen,p,D6_OPT_NTP_SERVER,list)" in ntp_parser_compact
assert "dhcp6_add_listval(list,D6_OPT_NTP_SERVER,DHCP6_LISTVAL_ADDR6,&address,NULL)" in ntp_parser_compact
ntp_case = outer_options.index("case D6_OPT_NTP_SERVER:")
ntp_case_end = outer_options.index("case D6_OPT_SIP_SERVER_A:", ntp_case)
ntp_option_case = "".join(outer_options[ntp_case:ntp_case_end].split())
assert "dhcp6_get_ntp_server(optlen,cp,&optinfo->ad_list)" in ntp_option_case
assert ntp_option_case.index("dhcp6_get_ntp_server(") < ntp_option_case.index("gotomalformed")
assert "D6_OPT_NTP_SERVER" not in outer_options[outer_options.index("case D6_OPT_SIP_SERVER_A:"):outer_options.index("case D6_OPT_IA_PD:")]
assert "DHCPv6 option bounds and NTP suboption policy: PASS" in bounds_test
assert "dhcp6c_ntp_suboption_wire_length_valid" in bounds_policy
assert "dhcp6c_ntp_suboption_payload_length_valid" in bounds_policy
assert "dhcp6c_ntp_address_kind_valid" in bounds_policy
assert "dhcp6c_ntp_time_source_claim" in bounds_policy
assert "dhcp6c_ntp_time_source_present" in bounds_policy

for option_parser in (outer_options, nested_options):
    assert "for (; (char *)p < (char *)ep; p = np)" in option_parser
    assert "for (; p + 1 <= ep; p = np)" not in option_parser
    assert "dhcp6c_option_wire_length_valid(remaining," in option_parser
    assert "np > ep" not in option_parser
    assert option_parser.index("dhcp6c_option_wire_length_valid(remaining,") < option_parser.index("np = (struct dhcp6opt *)(cp + optlen);")
assert "truncated DHCP6 option header" in outer_options
nested_wire_check = nested_options.index("dhcp6c_option_wire_length_valid(remaining,")
nested_switch = nested_options.index("/* check option context & minimum known-option length */")
nested_skip = nested_options.index("goto nextoption;", nested_switch)
assert nested_wire_check < nested_switch < nested_skip

assert "update_status = reestablish_ia(ia);" in update_ia
assert "dhcp6c_ia_operation_error(update_status, errno)" in update_ia
assert "first_error = dhcp6c_ia_first_error(first_error, ia_error);" in update_ia
assert "dhcp6c_ia_nobinding_requires_request" in policy
assert "test_nobinding_is_detected_before_ia_suboption_application" in test
assert "if (siav->lvtype == DHCP6_LISTVAL_NUM16 &&" in update_ia
assert "siav->val_num16 == D6_OPT_STCODE_NOBINDING" in update_ia
nobinding_preflight = update_ia.index("if (dhcp6c_ia_nobinding_requires_request(")
main_suboption_loop = update_ia.index("/* update IA configuration information */")
first_prefix_update = update_ia.index("update_prefix6(ia, siav")
assert update_ia.index("TAILQ_FOREACH(siav, &iav->sublist, link)") < nobinding_preflight < main_suboption_loop < first_prefix_update
preflight_body = update_ia[nobinding_preflight:main_suboption_loop]
assert "is_renew_or_rebind" in preflight_body
assert "no_binding_found" in preflight_body
assert "update_status = reestablish_ia(ia);" in preflight_body
assert "goto nextia;" in preflight_body
ordinary_suboption_loop = update_ia[main_suboption_loop:]
assert "siav->val_num16 == D6_OPT_STCODE_NOBINDING" not in ordinary_suboption_loop
assert "test_operation_error_is_retryable" in test
assert update_ia.index("update_status = reestablish_ia(ia);") < update_ia.index("goto nextia;", update_ia.index("update_status = reestablish_ia(ia);"))

prepare_event = function_body(ia, "static int prepare_iaevent(struct ia *ia, int dhcpstate)")
renew_prepare = prepare_event.index("if (ia->ctl && iapref_renew(")
event_data_queue = prepare_event.index("TAILQ_INSERT_TAIL(&ev->data_list, evd, link)")
renew_failure_cleanup = prepare_event[renew_prepare:event_data_queue]
assert "free(evd);" in renew_failure_cleanup
assert renew_failure_cleanup.index("free(evd);") < renew_failure_cleanup.index("goto fail;")
assert event_data_queue > renew_prepare

reply = function_body(client, "static int client6_recvreply(struct dhcp6_if *ifp, struct dhcp6_event *ev,")
apply_begin = reply.index("ia_status = update_ia(")
script_begin = reply.index("if (ifp->scriptpath != NULL")
apply_block = reply[apply_begin:script_begin]
assert "ia_status = update_ia(" in apply_block
assert "dhcp6c_ia_retry_required(ia_status)" in apply_block
assert "DHCP6C_SCRIPT_RETRY_DELAY_MS" in apply_block
assert "return -1;" in apply_block
assert "script_status = dhcp6_script(" in reply

remove_prefix = function_body(ia, "static int remove_iaprefix(struct iaprefix *sp, int type)")
assert "TAILQ_NEXT(ip, plink)" in remove_prefix
assert "ret = dhcp6c_ia_first_error(" in remove_prefix
assert "ret, errno > 0 ? errno : EIO" in remove_prefix
assert "test_multiple_pd_cleanup_failures_preserve_first_error" in test
assert "goto retry;" in remove_prefix
assert "dhcp6_timer_add(iapref_timo, sp)" in remove_prefix
assert "dhcp6_timer_set(DHCP6C_IA_CLEANUP_RETRY_DELAY_MS, sp->timer)" in remove_prefix
assert "TAILQ_REMOVE(&sp->u.ifprefix_list, ip, plink);" in remove_prefix

cleanup = function_body(ia, "static int pref_cleanup(struct iactl *iac)")
assert "if (remove_iaprefix(sp, iac->type) != 0)" in cleanup
assert cleanup.index("if (remove_iaprefix") < cleanup.index("free(iac)")
assert "TAILQ_REMOVE(&iac->iaprefix_head, sp, link)" not in cleanup

remove_ia = function_body(ia, "static int remove_ia(struct ia *ia)")
assert "ia->cleanup_pending = 1;" in remove_ia
assert "if (ia->ctl && pref_cleanup(ia->ctl) != 0)" in remove_ia
assert remove_ia.index("pref_cleanup(ia->ctl)") < remove_ia.index("TAILQ_REMOVE(&iac->iadata, ia, link)")

expiry = function_body(ia, "static struct dhcp6_timer *iapref_timo(void *arg)")
assert "if (remove_iaprefix(sp, sp->ctl->type) != 0)" in expiry
assert "ia->cleanup_pending || ia->ctl == NULL || !pref_isvalid(ia->ctl)" in expiry

assert "int addr_owned;" in ia
assert "sp->addr_owned = 1;" in update_prefix
assert "dhcp6c_ia_na_address_needs_remove(sp->addr_owned)" in remove_prefix
assert "DHCP6_DURATION_INFINITE, DHCP6_DURATION_INFINITE, 0)" in add_ifprefix
assert "DHCP6_DURATION_INFINITE, DHCP6_DURATION_INFINITE, 1)" in remove_prefix
socket_body = function_body(socket, "int ifaddrconf(ifaddrconf_cmd_t cmd, const char *ifname, struct in6_addr *addr,")
assert "dhcp6c_ia_ifaddr_change_complete(cmd == IFADDRCONF_ADD, errno," in socket_body
assert "already_owned" in socket_body
assert "return error == EEXIST && already_owned;" in policy
assert "test_unowned_ia_na_collision_is_never_claimed_or_removed" in test
assert "if (errno == EEXIST)" not in socket_body

assert "test_failed_address_removal_is_retained_and_retried" in test
assert "test_address_change_absent_and_duplicate_errors" in test
assert "DHCPv6 IA delegated-prefix retry and removal policy: PASS" in test
assert "dhcp6c_ia_shutdown_ready" in policy
assert "dhcp6c_ia_restart_ready" in policy
assert "dhcp6c_ia_client_start_allowed" in policy
assert "test_shutdown_waits_for_ia_cleanup_and_release_events" in test
assert "test_config_reload_waits_for_pending_ia_and_release_cleanup" in test
assert "int shutting_down;" in config_header
assert "int restart_pending;" in config_header
client_start = function_body(client, "int client6_start(struct dhcp6_if *ifp)")
assert "dhcp6c_ia_client_start_allowed(ifp->shutting_down, ifp->restart_pending)" in client_start
assert client_start.index("dhcp6_timer_remove(&ifp->timer)") < client_start.index("dhcp6c_ia_client_start_allowed(") < client_start.index("dhcp6_create_event(ifp, DHCP6S_INIT)")
pending_cleanup = function_body(client, "static int client6_has_pending_ia_cleanup(void)")
assert "TAILQ_FOREACH(iac, &ifp->iaconf_list, link)" in pending_cleanup
assert "if (!TAILQ_EMPTY(&iac->iadata))" in pending_cleanup
shutdown_check = function_body(client, "static void client6_check_shutdown(void)")
assert "dhcp6c_ia_shutdown_ready(client6_has_pending_ia_cleanup(),!TAILQ_EMPTY(&client6_config.event_list))" in "".join(shutdown_check.split())
assert "check_exit(1);" in shutdown_check
release_setup = function_body(ia, "void release_all_ia(struct dhcp6_if *ifp)")
assert "prepare_iaevent(ia, DHCP6S_RELEASE);" in release_setup
free_resources = function_body(client, "static void free_resources(int force)")
assert "if (!force && ev->state == DHCP6S_RELEASE)" in free_resources
restart_check = function_body(client, "static void client6_check_restart(void)")
assert "dhcp6c_ia_restart_ready(client6_has_pending_ia_cleanup(),!TAILQ_EMPTY(&client6_config.event_list))" in "".join(restart_check.split())
assert restart_check.index("ifp->restart_pending = 0;") < restart_check.index("client6_startall(1);")
read_config_reload = function_body(config, "int FAST_FUNC read_config6(const char *file)")
assert "struct ia_conf *iac;" in read_config_reload
assert "TAILQ_FOREACH(iac, &client6_config.dhcp6c_if.iaconf_list, link)" in read_config_reload
assert "errno = EBUSY;" in read_config_reload
assert read_config_reload.index("if (!TAILQ_EMPTY(&iac->iadata))") < read_config_reload.index("cleanup_cflist(") < read_config_reload.index("clear_ifconf(0);")
clear_ifconf_body = function_body(config, "void clear_ifconf(int destroy)")
assert "clear_iaconf(&ifp->iaconf_list);" in clear_ifconf_body
mainloop = function_body(client, "static void client6_mainloop(void)")
assert "case SIGTERM:" in mainloop and "case SIGQUIT:" in mainloop
assert "restart_cleanup_started = ifp->restart_pending;" in mainloop
assert "ifp->shutting_down = 1;" in mainloop and "ifp->restart_pending = 0;" in mainloop
assert "if (!restart_cleanup_started)" in mainloop
sighup_start = mainloop.index("case SIGHUP:")
reload_flag = mainloop.index("ifp->restart_pending = 1;", sighup_start)
reload_free = mainloop.index("free_resources(0);", reload_flag)
assert reload_flag < reload_free
assert "client6_startall(1);" not in mainloop
assert mainloop.count("client6_check_restart();") == 3
timer_check = mainloop.index("w = dhcp6_timer_check();")
assert timer_check < mainloop.index("client6_check_shutdown();", timer_check)
assert timer_check < mainloop.index("client6_check_restart();", timer_check)

assert "test_partial_pd_failure_is_retried_without_reapplying_successes" in test
assert "test_first_error_and_retry_gate" in test


# recv_from_to() previously returned the copied buffer length for an oversized
# UDP datagram, so a packet truncated exactly at an option boundary could look
# complete. MSG_TRUNC exposes the original Linux datagram size; reject it before
# computing any packet-end pointer.
receive = function_body(client, "static void client6_recv(void)")
receive_call = receive.index("len = recv_from_to(")
receive_error = receive.index("if (len < 0)")
receive_call_end = receive.index(";", receive_call)
short_check = receive.index("if (len < (ssize_t)sizeof(struct dhcp6))")
datagram_check = receive.index("dhcp6c_datagram_length_fits(")
option_parse = receive.index("dhcp6_get_options(")
assert "MSG_TRUNC" in receive[receive_call:receive_call_end]
assert receive_call < receive_error < short_check < datagram_check < option_parse
assert "(size_t)len, sizeof(G.rbuf)" in receive[datagram_check:option_parse]
assert "return;" in receive[datagram_check:option_parse]

# stop_dhcp6c holds the session lock across the daemon stop. A failed
# kill must not stale callback tokens belonging to a still-live client.
wan6 = read("trunk/user/rc/src/net_wan6.c")
callback_test = read("trunk/user/rc/src/tests/dhcp6c_callback_generation.c")
callback_policy = read("trunk/user/rc/src/dhcp6c_callback_generation_policy.h")
stop_dhcp6c = function_body(wan6, "int stop_dhcp6c(void)")
assert "stop_status = kill_services(svcs, 16, 0);" in stop_dhcp6c
assert "stop_status = kill_services(svcs, 3, 1);" not in stop_dhcp6c
stop_call = stop_dhcp6c.index("stop_status = kill_services(svcs, 16, 0);")
generation_advance = stop_dhcp6c.index(
    "dhcp6c_callback_session_advance(session_fd, &generation)")
generation_guard = stop_dhcp6c.rindex("if (stop_status == 0 &&",
                                     stop_call, generation_advance)
final_unlock = stop_dhcp6c.rindex(
    "unlock_status = dhcp6c_callback_session_unlock(session_fd);")
assert stop_call < generation_guard < generation_advance < final_unlock
assert "test_failed_stop_preserves_waiting_old_callback_generation" in callback_test
assert "test_successful_stop_invalidates_waiting_old_callback_generation" in callback_test
assert "run_stop_fencing_case(EBUSY, 1, 1)" in callback_test
assert "run_stop_fencing_case(0, 0, 2)" in callback_test
assert "dhcp6c_callback_primary_errno(saved_errno, unlock_errno)" in stop_dhcp6c
assert "dhcp6c_callback_session_lock" in callback_policy

# Timer callbacks may free the saved-next node (for example, IA prefix
# cleanup removes sibling retry timers). The iterator must restart from the
# live head after any callback-side list mutation and skip visited/new nodes.
timer_add = function_body(common, "struct dhcp6_timer *dhcp6_timer_add(")
timer_remove = function_body(common, "void dhcp6_timer_remove(")
timer_check = function_body(common, "struct timeval *dhcp6_timer_check(void)")
timer_check_compact = "".join(timer_check.split())
assert '#include "dhcp6c_timer_policy.h"' in common
assert "unsigned int check_generation;" in header
assert "dhcp6c_timer_next_check_generation" in timer_policy
assert "dhcp6c_timer_seen_in_check" in timer_policy
assert "dhcp6c_timer_callback_requires_rescan" in timer_policy
assert "newtimer->check_generation = timer_check_generation;" in timer_add
assert timer_add.index("newtimer->check_generation = timer_check_generation;") < timer_add.index("LIST_INSERT_HEAD(")
assert "++timer_list_generation;" in timer_add
assert "++timer_list_generation;" in timer_remove
assert "timer_check_generation=dhcp6c_timer_next_check_generation(timer_check_generation);" in timer_check_compact
assert "tm->check_generation=0U;" in timer_check_compact
assert "dhcp6c_timer_seen_in_check(tm->check_generation,timer_check_generation)" in timer_check_compact
assert timer_check.index("tm->check_generation = timer_check_generation;") < timer_check.index("callback_result = expire(expire_data);")
rescan = timer_check.index("dhcp6c_timer_callback_requires_rescan(")
restart = timer_check.index("tm = LIST_FIRST(&client6_config.timer_head);", rescan)
continue_after = timer_check.index("continue;", rescan)
advance = timer_check.rindex("tm = LIST_NEXT(tm, link);")
assert rescan < restart < continue_after < advance
assert "tm_next" not in timer_check
assert timer_check.rindex("client6_config.tm_sentinel = ULLONG_MAX;") > timer_check.index("while (tm != NULL)")
assert "assert(dhcp6c_timer_next_check_generation(UINT_MAX) == 1U);" in timer_test
assert "test_callback_removal_restarts_without_revisiting_or_running_new_timer" in timer_test
assert "visited_second == 0" in timer_test and "visited_third == 1" in timer_test
assert "DHCPv6 timer generation and callback invalidation policy: PASS" in timer_test

# Keep the IA timer as the owner of T2/Rebind until the exchange is ready.
ia_timeout = function_body(ia, "static struct dhcp6_timer *ia_timo(void *arg)")
assert "DHCP6C_IA_EVENT_RETRY_DELAY_MS 5000ULL" in policy
assert "dhcp6c_ia_event_timer_action" in policy
assert "case IAS_REBIND:" in ia_timeout
assert "case DHCP6C_IA_EVENT_TIMER_KEEP_T2:" in ia_timeout
assert "case DHCP6C_IA_EVENT_TIMER_RETRY_REBIND:" in ia_timeout
assert "dhcp6_timer_set(DHCP6C_IA_EVENT_RETRY_DELAY_MS, ia->timer);" in ia_timeout
prepared = ia_timeout.index("event_prepared = (prepare_iaevent(ia, dhcpstate) == 0);")
selected = ia_timeout.index("dhcp6c_ia_event_timer_action(", prepared)
removed = ia_timeout.index("dhcp6_timer_remove(&ia->timer);", selected)
assert prepared < selected < removed
renew_case = ia_timeout.index("case IAS_RENEW:")
rebind_case = ia_timeout.index("case IAS_REBIND:")
default_case = ia_timeout.index("default:", rebind_case)
assert renew_case < rebind_case < default_case
assert "dhcp6_timer_remove(&ia->timer);" not in ia_timeout[renew_case:rebind_case]
retry_case = ia_timeout.index("case DHCP6C_IA_EVENT_TIMER_RETRY_REBIND:")
remove_case = ia_timeout.index("case DHCP6C_IA_EVENT_TIMER_REMOVE_REBIND:")
assert retry_case < remove_case
assert "dhcp6_timer_set(DHCP6C_IA_EVENT_RETRY_DELAY_MS, ia->timer);" in ia_timeout[retry_case:remove_case]
assert "if (ia->timer)" in ia_timeout[default_case:]
assert "test_ia_event_timer_is_retained_until_rebind_is_ready" in test
assert "DHCP6C_IA_EVENT_TIMER_RETRY_REBIND" in test

# NoBinding starts a bounded REQUEST exchange. Keep an IA watchdog beyond
# all MRC=10 retransmission intervals so server silence cannot orphan the IA.
ia_timeout = function_body(ia, "static struct dhcp6_timer *ia_timo(void *arg)")
reestablish = function_body(ia, "static int reestablish_ia(struct ia *ia)")
update_ia_body = function_body(ia, "int update_ia(struct dhcp6_list *iahead")
assert "typedef enum {IAS_ACTIVE, IAS_RENEW, IAS_REBIND, IAS_REQUEST}" in ia
assert "DHCP6C_IA_REQUEST_RETRY_DELAY_MS 240000ULL" in policy
assert "dhcp6c_ia_request_retry_delay" in policy
assert "dhcp6c_ia_request_nobinding_is_duplicate" in policy
assert "prepare_iaevent(ia, DHCP6S_REQUEST)" in ia_timeout
assert "dhcp6_timer_set(request_delay, ia->timer);" in ia_timeout
request_state = reestablish.index("ia->state = IAS_REQUEST;")
request_prepared = reestablish.index("ret = prepare_iaevent(ia, DHCP6S_REQUEST);")
watchdog_start = reestablish.index("if (ia->timer == NULL)", request_prepared)
delay_select = reestablish.index(
    "request_delay = dhcp6c_ia_request_retry_delay(ret == 0,", watchdog_start)
watchdog_set = reestablish.index("dhcp6_timer_set(request_delay, ia->timer);", delay_select)
assert request_state < request_prepared < watchdog_start < delay_select < watchdog_set
assert "dhcp6_timer_remove(&ia->timer);" not in reestablish
assert "return ret;" in reestablish[watchdog_set:]
assert "dhcp6c_ia_request_nobinding_is_duplicate(" in update_ia_body
assert update_ia_body.index("dhcp6c_ia_request_nobinding_is_duplicate(") < update_ia_body.index("dhcp6c_ia_nobinding_requires_request(")
assert "dhcp6_timer_set(dhcp6c_ia_request_retry_delay(1, 1)," in update_ia_body
assert "case IAS_REQUEST:" in ia
assert 'return "REQUEST";' in ia
assert "test_nobinding_request_has_a_bounded_retry_watchdog" in test
assert "dhcp6c_ia_request_retry_delay(1, 1) == 240000ULL" in test

required_paths = (
    ".github/workflows/dhcp6c-ia-publication-stability.yml",
    "scripts/check_dhcp6c_ia_publication.py",
    BASE + "dhcp6c_ia.c",
    BASE + "dhcp6c.c",
    BASE + "dhcp6c_socket.c",
    BASE + "dhcp6c.h",
    BASE + "dhcp6c_config.h",
    BASE + "dhcp6c_ia_policy.h",
    BASE + "tests/dhcp6c_ia_publication_policy.c",
    BASE + "dhcp6c_config.c",
    BASE + "dhcp6c_iaid_policy.h",
    BASE + "tests/dhcp6c_iaid_policy.c",
    BASE + "dhcp6c_pd_sla_policy.h",
    BASE + "tests/dhcp6c_pd_sla_policy.c",
    BASE + "dhcp6c_config_cleanup_policy.h",
    BASE + "tests/dhcp6c_config_cleanup_policy.c",
    BASE + "dhcp6c_common.c",
    BASE + "dhcp6c_timer_policy.h",
    BASE + "tests/dhcp6c_timer_traversal_policy.c",
    BASE + "dhcp6c_option_bounds_policy.h",
    BASE + "tests/dhcp6c_option_bounds_policy.c",
    "trunk/user/rc/src/net_wan6.c",
    "trunk/user/rc/src/dhcp6c_callback_generation_policy.h",
    "trunk/user/rc/src/tests/dhcp6c_callback_generation.c",
)
filters = workflow.split("jobs:", 1)[0]
push, pull = filters.split("  pull_request:", 1)
for path in required_paths:
    quoted = '"' + path + '"'
    assert push.count(quoted) == 1, "push filter missing/duplicate: " + path
    assert pull.count(quoted) == 1, "pull_request filter missing/duplicate: " + path

xy_push = xy_workflow.split("jobs:", 1)[0].split("  pull_request:", 1)[0]
for path in (BASE + "dhcp6c_config_cleanup_policy.h",
             BASE + "tests/dhcp6c_config_cleanup_policy.c"):
    quoted = '"' + path + '"'
    assert xy_push.count(quoted) == 1, "XY-C1 push filter missing/duplicate: " + path
print("DHCPv6 IA retry/publication and config-cleanup source contracts: PASS")
