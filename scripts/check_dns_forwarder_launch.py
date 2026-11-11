#!/usr/bin/env python3
"""Check DNS Forwarder launch-path independence from the caller's CWD."""
from __future__ import annotations

import errno
import os
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
workflow = read(".github/workflows/dns-forwarder-launch-path.yml")

start = function_body(services, "void start_dnsforwarder(void)")
stop = function_body(services, "void stop_dnsforwarder(void)")
restart = function_body(services, "void restart_dnsforwarder(void)")
assert 'nvram_get_int("dns_forwarder_enable")' in start
assert "if (dnsforwarder_mode == 1)" in start
assert 'eval("/usr/bin/dns-forwarder.sh","start");' in start
assert 'eval("usr/bin/dns-forwarder.sh","start");' not in start
assert 'eval("/usr/bin/dns-forwarder.sh","stop");' in stop
assert restart.index("stop_dnsforwarder();") < restart.index("start_dnsforwarder();")
assert "start_dnsforwarder();" in function_body(
    services, "start_services_once(int is_ap_mode)"
)
assert "restart_dnsforwarder();" in rc
assert "RCN_RESTART_DNSFORWARDER" in rc
assert "void start_dnsforwarder(void);" in rc_header
assert "void stop_dnsforwarder(void);" in rc_header
assert "void restart_dnsforwarder(void);" in rc_header

# The package installs the exact absolute script path used by the RC service.
assert (
    "$(ROMFSINST) -p +x $(THISDIR)/dns-forwarder.sh /usr/bin/dns-forwarder.sh"
    in package_makefile
)
assert (
    'start-stop-daemon -S -b -x dns-forwarder -- -b "$bind_address" '
    '-p "$bind_port" -s "$server"'
    in launcher_script
)
assert 'killall -q dns-forwarder' in launcher_script

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
    "trunk/user/dns-forwarder/Makefile",
    "trunk/user/dns-forwarder/dns-forwarder.sh",
    "scripts/check_dns_forwarder_launch.py",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

print("DNS Forwarder push/pull_request path filters: PASS")
