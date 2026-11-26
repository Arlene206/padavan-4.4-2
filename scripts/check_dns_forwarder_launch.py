#!/usr/bin/env python3
"""Check DNS Forwarder launch-path independence from the caller's CWD."""
from __future__ import annotations

import errno
import os
import re
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def function_body(source: str, signature: str) -> str:
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


services = read("trunk/user/rc/src/services.c")
rc = read("trunk/user/rc/src/rc.c")
rc_header = read("trunk/user/rc/src/rc.h")
init = read("trunk/user/rc/src/init.c")
shutils = read("trunk/user/shared/src/shutils.c")
shutils_header = read("trunk/user/shared/src/shutils.h")
package_makefile = read("trunk/user/dns-forwarder/Makefile")
launcher_script = read("trunk/user/dns-forwarder/dns-forwarder.sh")
forwarder_core = read("trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-dns-forwarder.c")
event_source_core = read("trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-event-source.c")
event_loop_test = read("trunk/user/dns-forwarder/tests/hev_event_loop_lifecycle.c")
session_core = read("trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-dns-session.c")
main_core = read("trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-main.c")
forwarder_policy = read("trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-dns-forwarder-policy.h")
forwarder_policy_test = read("trunk/user/dns-forwarder/tests/hev_dns_forwarder_policy.c")
forwarder_init_test = read("trunk/user/dns-forwarder/tests/hev_dns_forwarder_init_failure.c")
forwarder_init_hooks = read("trunk/user/dns-forwarder/tests/hev_dns_forwarder_init_test_hooks.h")
core_dir = "trunk/user/dns-forwarder/dns-forwarder-1.2.1/"
event_loop = read(core_dir + "hev-event-loop.c")
event_source = read(core_dir + "hev-event-source.c")
timeout_source = read(core_dir + "hev-event-source-timeout.c")
signal_source = read(core_dir + "hev-event-source-signal.c")
event_loop_test = read("trunk/user/dns-forwarder/tests/hev_event_loop_lifecycle.c")
lifecycle_test = read("trunk/user/dns-forwarder/tests/dns_forwarder_lifecycle.sh")
shared_process_helper = read("trunk/user/shared/scripts/process_lifecycle.sh")
shared_makefile = read("trunk/user/shared/Makefile")
assert r"*[!0123456789\ ]*" in launcher_script
subprocess.run(["/bin/sh", "-n", str(ROOT / "trunk/user/dns-forwarder/dns-forwarder.sh")], check=True)
workflow = read(".github/workflows/dns-forwarder-launch-path.yml")
busybox_config = read("trunk/configs/boards/busybox.config")
assert "CONFIG_FLOCK=y" in busybox_config
assert "CONFIG_PIDOF=y" in busybox_config
assert "CONFIG_KILLALL=y" in busybox_config
assert "CONFIG_START_STOP_DAEMON=y" in busybox_config

start = function_body(services, "int start_dnsforwarder(void)")
stop = function_body(services, "int stop_dnsforwarder(void)")
restart = function_body(services, "int restart_dnsforwarder(void)")
assert 'return eval("/usr/bin/dns-forwarder.sh","start");' in start
assert 'return eval("/usr/bin/dns-forwarder.sh","stop");' in stop
assert 'return eval("/usr/bin/dns-forwarder.sh","restart");' in restart
assert "stop_dnsforwarder();" not in restart
assert "start_dnsforwarder();" in function_body(
    services, "start_services_once(int is_ap_mode)"
)
notify = function_body(rc, "handle_notifications(void)")
assert "action_result = rc_notify_merge_action_result(action_result, restart_dnsforwarder());" in notify
assert "RCN_RESTART_DNSFORWARDER" in notify
assert "int start_dnsforwarder(void);" in rc_header
assert "int stop_dnsforwarder(void);" in rc_header
assert "int restart_dnsforwarder(void);" in rc_header

# The package installs the exact absolute script path used by the RC service.
assert (
    "$(ROMFSINST) -p +x $(THISDIR)/dns-forwarder.sh /usr/bin/dns-forwarder.sh"
    in package_makefile
)
assert "dns_forwarder_lifecycle_lock_prepare" in launcher_script
assert "dns_forwarder_lifecycle_lock_execute" in launcher_script
assert "flock -n 9" in launcher_script
assert "pidof dns-forwarder" in launcher_script
assert "dns_forwarder_process_state()" in launcher_script
assert 'PROCESS_LIFECYCLE_LIB="${PROCESS_LIFECYCLE_LIB:-/usr/lib/process_lifecycle.sh}"' in launcher_script
assert '. "$PROCESS_LIFECYCLE_LIB" || exit 127' in launcher_script
process_state_body = function_body(launcher_script, "dns_forwarder_process_state()")
assert "pl_process_state dns-forwarder" in process_state_body
assert "dns_forwarder_inventory_status" in process_state_body
assert "pl_process_state()" in shared_process_helper
assert "$(INSTALL_BIN) $(CURDIR)/scripts/process_lifecycle.sh $(ROMFSDIR)/usr/lib/process_lifecycle.sh" in shared_makefile
assert "cannot verify existing daemon state" in launcher_script
assert "cannot verify daemon stop postcondition" in launcher_script
stop_body = function_body(launcher_script, "func_stop()")
assert 'kill -TERM "$dns_forwarder_pid"' in stop_body
assert "killall -q dns-forwarder" not in stop_body
assert 'dns_forwarder_pids="$*"' in function_body(
    launcher_script, "dns_forwarder_process_state()"
)
assert '123 456' in lifecycle_test
assert '-TERM 123' in lifecycle_test and '-TERM 456' in lifecycle_test
assert "FAKE_PIDOF_STATUS=2" in lifecycle_test
assert 'PROCESS_LIFECYCLE_LIB="$script_dir/../../shared/scripts/process_lifecycle.sh"' in lifecycle_test
assert 'PL_PROC_ROOT="$tmp/proc"' in lifecycle_test
assert "4242 (dns-forwarder)" in lifecycle_test
assert 'PL_PROC_ROOT="$tmp/missing-proc"' in lifecycle_test
assert "start-stop-daemon -S -b -x /usr/bin/dns-forwarder --" in launcher_script
assert "launcher returned success but no daemon appeared" in launcher_script
assert "restart aborted because stop did not complete" in launcher_script
assert "nvram get dns_forwarder_enable" in launcher_script
assert "bind_address=$(nvram get" not in launcher_script[:launcher_script.index("func_start()")]
assert '[ -s "$FAKE_RUNNING_FILE" ]' in lifecycle_test
assert lifecycle_test.count('printf \'%s\\n\' 123 > "$FAKE_RUNNING_FILE"') >= 3

# eval() delegates argv[0] to execvp(). A pathname containing a slash is
# resolved from CWD; PATH search is not performed for this relative pathname.
assert "#define eval(cmd, args...)" in shutils_header
assert "_eval(argv, NULL, 0, NULL)" in shutils_header
eval_body = function_body(
    shutils, "_eval(char *const argv[], char *path, int timeout, int *ppid)"
)
assert "execvp(argv[0], argv);" in eval_body
assert "chdir(" not in eval_body
assert 'chdir("/")' in function_body(init, "init_main_loop(void)")

# The old slash-containing relative name fails from an unrelated CWD, while
# the absolute equivalent succeeds, matching the contract above.
with tempfile.TemporaryDirectory(prefix="dns-forwarder-cwd-") as tmp:
    root = Path(tmp)
    fake_script = root / "usr" / "bin" / "dns-forwarder.sh"
    fake_script.parent.mkdir(parents=True)
    marker = root / "invoked"
    fake_script.write_text(
        '#!/bin/sh\nprintf "%s" "$1" > "$MARKER"\n',
        encoding="utf-8",
    )
    fake_script.chmod(0o755)
    caller_cwd = root / "caller-cwd"
    caller_cwd.mkdir()
    env = os.environ.copy()
    env["MARKER"] = str(marker)

    try:
        subprocess.run(
            ["usr/bin/dns-forwarder.sh", "start"],
            cwd=caller_cwd, env=env, check=False,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
    except OSError as exc:
        assert exc.errno == errno.ENOENT, exc
    else:
        raise AssertionError("relative slash-containing launch unexpectedly worked")

    result = subprocess.run(
        [str(fake_script.resolve()), "start"],
        cwd=caller_cwd, env=env, check=False,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    assert marker.read_text(encoding="utf-8") == "start"

print("DNS Forwarder launch path working-directory regression: PASS")
print("DNS Forwarder service, installer and execvp source contracts: PASS")

paths = (
    ".github/workflows/dns-forwarder-launch-path.yml",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/init.c",
    "trunk/user/shared/src/shutils.c",
    "trunk/user/shared/src/shutils.h",
    "trunk/user/shared/Makefile",
    "trunk/user/shared/scripts/process_lifecycle.sh",
    "trunk/user/dns-forwarder/Makefile",
    "trunk/user/dns-forwarder/dns-forwarder.sh",
    "trunk/user/dns-forwarder/tests/dns_forwarder_lifecycle.sh",
    "trunk/configs/boards/busybox.config",
    "scripts/check_dns_forwarder_launch.py",
    "trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-event-loop.c",
    "trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-event-source.c",
    "trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-event-source-timeout.c",
    "trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-event-source-signal.c",
    "trunk/user/dns-forwarder/tests/hev_event_loop_lifecycle.c",
    "trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-dns-forwarder-policy.h",
    "trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-dns-forwarder.c",
    "trunk/user/dns-forwarder/dns-forwarder-1.2.1/hev-main.c",
    "trunk/user/dns-forwarder/tests/hev_dns_forwarder_policy.c",
    "trunk/user/dns-forwarder/tests/hev_dns_forwarder_init_failure.c",
    "trunk/user/dns-forwarder/tests/hev_dns_forwarder_init_test_hooks.h",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

# Event-loop/source lifetime and registration errors must be fail-closed.
loop_ctor = function_body(event_loop, "hev_event_loop_new (void)")
assert "self->epoll_fd < 0" in loop_ctor
assert "return NULL;" in loop_ctor
add_source = function_body(event_loop, "hev_event_loop_add_source (HevEventLoop *self, HevEventSource *source)")
assert "source->loop" in add_source
assert "!_hev_event_loop_add_fd (self, fd)" in add_source
assert "if (!source->funcs.prepare (source))" in add_source
assert "_hev_event_source_set_loop (source, NULL)" in add_source
assert "hev_event_source_unref (source);" in add_source
source_setter = function_body(event_source, "_hev_event_source_set_loop (HevEventSource *self, HevEventLoop *loop)")
assert "self->loop = loop;" in source_setter
source_add_fd = function_body(event_source, "hev_event_source_add_fd (HevEventSource *self, int fd, uint32_t events)")
assert "new_fds = hev_slist_append (self->fds, efd);" in source_add_fd
assert "hev_slist_remove (self->fds, efd)" in source_add_fd
assert "_hev_event_loop_add_fd (self->loop, efd)" in source_add_fd
assert "!hev_event_source_add_fd (source, self->timer_fd" in timeout_source
assert "return (0 == timerfd_settime" in timeout_source
assert "!hev_event_source_add_fd (source, self->signal_fd" in signal_source
assert "hev_event_loop_add_source (first, source)" in event_loop_test
assert "hev_event_loop_add_source (second, source)" in event_loop_test
assert "hev_event_source_add_fd (source, -1, EPOLLIN)" in event_loop_test
assert "hev_event_source_get_loop (source)" in event_loop_test

forwarder_constructor = function_body(
    forwarder_core, "hev_dns_forwarder_new (HevEventLoop *loop"
)
assert "if (!loop || !addr || !*addr || !port || !*port || !upstream)" in forwarder_constructor
assert "hev_dns_forwarder_parse_port (" in forwarder_constructor

# Constructor dependency hooks must exercise the real production constructor.
assert "#ifdef HEV_DNS_FORWARDER_TESTING" in forwarder_core
for operation in ("GETADDRINFO", "SOCKET", "IOCTL", "SETSOCKOPT", "BIND",
                  "FREEADDRINFO", "CLOSE", "EVENT_SOURCE_FDS_NEW",
                  "EVENT_SOURCE_TIMEOUT_NEW", "EVENT_SOURCE_ADD_FD",
                  "EVENT_LOOP_ADD_SOURCE", "EVENT_LOOP_DEL_SOURCE",
                  "EVENT_SOURCE_UNREF"):
    assert "DNS_FORWARDER_" + operation in forwarder_core
for failure_case in ("FAIL_GETADDRINFO", "FAIL_SOCKET", "FAIL_IOCTL",
                     "FAIL_SETSOCKOPT", "FAIL_BIND", "FAIL_LISTENER_SOURCE_NEW",
                     "FAIL_LISTENER_SOURCE_ADD_FD",
                     "FAIL_LISTENER_SOURCE_REGISTER", "FAIL_TIMEOUT_SOURCE_NEW",
                     "FAIL_TIMEOUT_SOURCE_REGISTER"):
    assert failure_case in forwarder_init_test
assert "open_fd_count()" in forwarder_init_test
assert "dns_forwarder_test_event_loop_del_source" in forwarder_init_hooks
assert "inet_aton (upstream, &upstream_addr)" in forwarder_constructor
assert forwarder_constructor.index("DNS_FORWARDER_GETADDRINFO (") < forwarder_constructor.index("DNS_FORWARDER_SOCKET (")
assert "if (0 > DNS_FORWARDER_IOCTL (self->listen_fd, FIONBIO" in forwarder_constructor
assert "if (0 > DNS_FORWARDER_SETSOCKOPT (self->listen_fd, SOL_SOCKET, SO_REUSEADDR" in forwarder_constructor
assert "if (0 != DNS_FORWARDER_BIND (self->listen_fd" in forwarder_constructor
assert "if (!DNS_FORWARDER_EVENT_SOURCE_ADD_FD (" in forwarder_constructor
assert forwarder_constructor.count("if (!DNS_FORWARDER_EVENT_LOOP_ADD_SOURCE (") == 2
assert forwarder_constructor.count("DNS_FORWARDER_EVENT_SOURCE_GET_LOOP (") == 2
assert "DNS_FORWARDER_EVENT_LOOP_DEL_SOURCE (loop, self->timeout_source)" in forwarder_constructor
assert "DNS_FORWARDER_EVENT_LOOP_DEL_SOURCE (loop, self->listener_source)" in forwarder_constructor
assert "DNS_FORWARDER_FREEADDRINFO (addr_ip);" in forwarder_constructor
assert "DNS_FORWARDER_CLOSE (self->listen_fd);" in forwarder_constructor
assert "HEV_MEMORY_ALLOCATOR_FREE (self);" in forwarder_constructor

assert '#include "hev-dns-forwarder-policy.h"' in forwarder_core
assert re.search(r"hev_dns_forwarder_parse_port\s*\(\s*upstream_port\s*,\s*&parsed_upstream_port\s*\)", forwarder_core)
assert "htons (parsed_upstream_port)" in forwarder_core
assert "atoi (upstream_port)" not in forwarder_core
event_source_constructor = function_body(event_source_core, "hev_event_source_new (")
assert "memset (&self->funcs, 0, sizeof (self->funcs));" in event_source_constructor
assert event_source_constructor.index("memset (&self->funcs") < event_source_constructor.index("if (funcs)")
assert "test_default_event_source_functions_are_initialized" in event_loop_test

event_source_name_setter = function_body(event_source_core, "hev_event_source_set_name (")
assert "strlen (name) + 1" in event_source_name_setter
assert "HEV_MEMORY_ALLOCATOR_ALLOC (name_length)" in event_source_name_setter
assert "memcpy (new_name, name, name_length)" in event_source_name_setter
assert event_source_name_setter.index("memcpy (new_name") < event_source_name_setter.index("HEV_MEMORY_ALLOCATOR_FREE (old_name)")
assert "test_event_source_name_storage_is_safe" in event_loop_test

listener = function_body(
    forwarder_core,
    "listener_source_handler (HevEventSourceFD *fd, void *data)\n{"
)
session_new = function_body(
    session_core, "hev_dns_session_new (int fd, struct sockaddr_in *upstream,"
)
assert "for (;;)" in listener
assert "MSG_PEEK | MSG_DONTWAIT" in listener
assert "hev_dns_forwarder_udp_drain_action" in listener
assert "HEV_DNS_FORWARDER_UDP_DRAIN_RETRY" in listener
assert "fd->revents &= ~EPOLLIN" in listener
assert listener.index("hev_slist_append") < listener.index("hev_event_loop_add_source")
assert "if (!session)" in listener
assert "if (!source)" in listener
assert "if (!new_sessions)" in listener
assert "hev_slist_remove" in listener
assert "hev_dns_session_start (session)" in listener
assert "if (!self->forward_buffer || !self->backward_buffer)" in session_new
assert "hev_dns_session_unref (self)" in session_new
assert "hev_dns_forwarder_udp_drain_action(-1, EINTR)" in forwarder_policy_test
assert "hev_dns_forwarder_udp_drain_action(0, 0)" in forwarder_policy_test

assert "value > 65535" in forwarder_policy
for port_case in ('"1"', '"53"', '"00053"', '"65535"', '"0"', '"65536"', '"-1"', '"+53"', '"53x"'):
    assert port_case in forwarder_policy_test

# CLI startup failures must be visible to init/supervisor and release all owned args.
assert "if (!loop)" in main_core
assert "if (!source)" in main_core
assert "if (!hev_event_loop_add_source (loop, source))" in main_core
assert "if (!forwarder)" in main_core
assert "exit_status = 1;" in main_core
assert "dns_port_allocated = 1;" in main_core
assert "dns_port = dns_delimiter + 1;" in main_core
assert "if (dns_port_allocated)" in main_core
assert "free (dns_port);" in main_core
assert "hev_event_source_unref (source);" in main_core
assert "hev_event_loop_unref (loop);" in main_core
assert "hev_dns_forwarder_unref (forwarder);" in main_core

print("DNS Forwarder push/pull_request path filters: PASS")
