#!/usr/bin/env python3
"""Verify DHCPv6 Reply script failures retain and retry their event."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    pos = brace
    state = "code"
    while pos < len(source):
        ch = source[pos]
        nxt = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and nxt == "*":
                state = "block"
                pos += 1
            elif ch == "/" and nxt == "/":
                state = "line"
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
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        elif state == "block" and ch == "*" and nxt == "/":
            state = "code"
            pos += 1
        elif state == "line" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)

base = "trunk/user/busybox/busybox-1.24.x/networking/udhcp/"
dhcp6c = (ROOT / (base + "dhcp6c.c")).read_text()
common = (ROOT / (base + "dhcp6c_common.c")).read_text()
dhcp4c = (ROOT / (base + "dhcpc.c")).read_text()
shared_policy = (ROOT / (base + "dhcp_script_env_policy.h")).read_text()
script_runner = (ROOT / (base + "dhcp6c_script.c")).read_text()
policy = (ROOT / (base + "dhcp6c_script_retry_policy.h")).read_text()
test = (ROOT / (base + "tests/dhcp6c_script_retry_policy.c")).read_text()
workflow = (ROOT / ".github/workflows/dhcp6c-script-retry-stability.yml").read_text()

assert '#include "dhcp6c_script_retry_policy.h"' in dhcp6c

# ifp->ifname is not initialized until after if_nametoindex() succeeds.
# The fatal diagnostic must use the original argument, not the null field.
if6init_signature = "static void if6init(struct dhcp6_if *ifp, const char *ifname)"
if6init_prototype = dhcp6c.index(if6init_signature)
if6init_definition = dhcp6c.index(if6init_signature, if6init_prototype + len(if6init_signature))
if6init = function_body(dhcp6c[if6init_definition:], if6init_signature)
ifindex_read = if6init.index("ifp->ifid = if_nametoindex(ifname);")
ifindex_guard = if6init.index("if (ifp->ifid == 0)", ifindex_read)
ifindex_die = if6init.index('bb_perror_msg_and_die("invalid interface(%s)", ifname);', ifindex_guard)
ifname_publish = if6init.index("ifp->ifname = xstrdup(ifname);")
assert ifindex_read < ifindex_guard < ifindex_die < ifname_publish
assert 'bb_perror_msg_and_die("invalid interface(%s)", ifp->ifname);' not in if6init
assert '#include "dhcp6c_script_retry_policy.h"' in script_runner
env_builder = function_body(script_runner, "char **fill_envp_client6(")
assert "dhcp6c_script_env_publish(envp, putenv, client6_unsetenv)" in env_builder
assert "putenv(*curr++)" not in env_builder
assert "return NULL;" in env_builder
assert "int saved_errno = errno ? errno : ENOMEM;" in env_builder
assert "for (curr = envp; *curr != NULL; ++curr)" in env_builder
assert "free(envp);" in env_builder
# The compatibility wrapper delegates to the shared publisher below.
assert "dhcp_script_env_publish(envp, publish, unset_value)" in policy
shared_publication = function_body(shared_policy, "dhcp_script_env_publish(")
assert "if (!envp || !publish || !unset_value)" in shared_publication
assert "while (published > 0)" in shared_publication
assert "unset_value(envp[--published]);" in shared_publication
assert "errno = saved_errno;" in shared_publication
runner = function_body(script_runner, "int dhcp6_script(const char *scriptpath, char **envp)")
assert "if (envp == NULL)" in runner
assert runner.index("if (envp == NULL)") < runner.index("status = spawn_and_wait(argv);")
assert "status = spawn_and_wait(argv);" in runner
assert "return status;" in runner
assert "free(envp);" in runner

assert '#include "dhcp_script_env_policy.h"' in dhcp4c
dhcp4_builder = function_body(dhcp4c, "static char **fill_envp(struct dhcp_packet *packet)")
assert "putenv(*curr++)" not in dhcp4_builder
assert "dhcp_script_env_publish(envp, putenv, udhcpc_unsetenv)" in dhcp4_builder
assert "goto publish_env;" in dhcp4_builder and "publish_env:" in dhcp4_builder
assert "for (curr = envp; *curr != NULL; ++curr)" in dhcp4_builder
assert "free(envp);" in dhcp4_builder
dhcp4_runner = function_body(dhcp4c, "static int udhcp_run_script(struct dhcp_packet *packet, const char *name)")
assert "if (envp == NULL)" in dhcp4_runner
assert dhcp4_runner.index("if (envp == NULL)") < dhcp4_runner.index("status = spawn_and_wait(argv);")
assert "status = spawn_and_wait(argv);" in dhcp4_runner

reply = function_body(dhcp6c, "static int client6_recvreply(struct dhcp6_if *ifp, struct dhcp6_event *ev,")
assert "int state, script_status = 0;" in reply
script_call = reply.index("script_status = dhcp6_script(")
retry_gate = reply.index("if (dhcp6c_script_retry_required(script_status))")
assert script_call < retry_gate
retry_budget = reply.index("ev->timeouts = 0;", retry_gate)
retry_params = reply.index("dhcp6_set_timeoparam(ev);", retry_budget)
retry_timer = reply.index("dhcp6_timer_set(DHCP6C_SCRIPT_RETRY_DELAY_MS, ev->timer);", retry_params)
retry_return = reply.index("return -1;", retry_timer)
refresh_timer = reply.index("if (state == DHCP6S_INFOREQ)", retry_return)
remove_event = reply.index("dhcp6_remove_event(ev);", refresh_timer)
assert script_call < retry_gate < retry_budget < retry_params < retry_timer < retry_return < refresh_timer < remove_event
assert "if (script_status < 0)" in reply[retry_gate:retry_return]
assert "retrying event" in reply[retry_gate:retry_return]
assert "DHCPv6 configuration script exited with status %d; retrying event" in reply[retry_gate:retry_return]
assert "#define DHCP6C_SCRIPT_RETRY_DELAY_MS 5000ULL" in policy
assert "return script_status != 0;" in policy
assert "DHCP6C_SCRIPT_RETRY_DELAY_MS == 5000ULL" in test
assert "dhcp6_timer_remaining_ms(1500ULL, 1000ULL) == 500ULL" in test
assert "dhcp6_timer_remaining_ms(1000ULL, 1000ULL) == 0ULL" in test
assert "dhcp6_timer_remaining_ms(999ULL, 1000ULL) == 0ULL" in test
assert "DHCPv4/v6 callback environment and retry policy: PASS" in test

timer_rest = function_body(common, "unsigned long long dhcp6_timer_rest(struct dhcp6_timer *timer)")
assert '#include "dhcp6c_script_retry_policy.h"' in common
assert "dhcp6_timer_remaining_ms(timer->t, now)" in timer_rest
assert "timer->t - now <= 0" not in timer_rest

for marker in (
    "test_environment_publish_rolls_back_partial_set",
    "test_environment_publish_failure_without_errno_uses_enomem",
    "test_environment_publish_success_keeps_all_entries",
    "test_environment_publish_null_callback_is_rejected",
    "dhcp6c_script_env_publish",
    "test_success_does_not_retry",
    "test_positive_exit_status_retries",
    "test_spawn_failure_retries",
    "test_retry_delay_is_bounded_and_nonzero",
    "DHCPv4/v6 callback environment and retry policy: PASS",
):
    assert marker in test, marker

# recv_from_to() receives sizeof(sockaddr_in6) as msg_namelen. The old
# struct sockaddr local was only 16 bytes, so recvmsg() could overwrite the
# stack when it copied the 28-byte IPv6 peer address.
recv_signature = "static void client6_recv(void)"
recv_prototype = dhcp6c.index(recv_signature)
recv_definition = dhcp6c.index(
    recv_signature, recv_prototype + len(recv_signature)
)
recv_body = function_body(dhcp6c[recv_definition:], recv_signature)
assert "struct sockaddr_storage from;" in recv_body
assert "struct sockaddr from;" not in recv_body
recv_compact = "".join(recv_body.split())
assert "recv_from_to(csock,G.rbuf,sizeof(G.rbuf),MSG_TRUNC,(struct sockaddr *)&from,(struct sockaddr *)&to,sizeof(struct sockaddr_in6));" in recv_compact
assert "memset(&from,0,sizeof(from));" in recv_compact
assert "xmalloc_sockaddr2dotted_noport((struct sockaddr *)&from)" in recv_compact
assert "test_ipv6_source_address_storage_fits_kernel_sockaddr" in test
assert "sizeof(from) >= sizeof(peer)" in test

workflow_push, pr_part = workflow.split("  pull_request:", 1)
workflow_pr = pr_part.split("  workflow_dispatch:", 1)[0]
paths = (
    ".github/workflows/dhcp6c-script-retry-stability.yml",
    "scripts/check_dhcp6c_script_retry.py",
    base + "dhcp6c.c",
    base + "dhcp6c_script.c",
    base + "dhcp6c_common.c",
    base + "dhcp6c_script_retry_policy.h",
    base + "tests/dhcp6c_script_retry_policy.c",
)
for path in paths:
    quoted = '"' + path + '"'
    assert workflow_push.count(quoted) == 1, "push path: " + path
    assert workflow_pr.count(quoted) == 1, "pull_request path: " + path

print("DHCPv6 Reply callback failure and event retry source contract: PASS")
