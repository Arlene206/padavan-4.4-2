#!/usr/bin/env python3
"""Compile the real UPnP delete caller with injected firewall/lease backends."""

from pathlib import Path
import os
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATH = ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/upnpredirect.c"
SOURCE = SOURCE_PATH.read_text()


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    state = "code"
    index = brace
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "/" and next_char == "*":
                state = "block_comment"
                index += 1
            elif char == "/" and next_char == "/":
                state = "line_comment"
                index += 1
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[brace:index + 1]
        elif state in ("string", "char"):
            if char == "\\\\":
                index += 1
            elif state == "string" and char == '"':
                state = "code"
            elif state == "char" and char == "'":
                state = "code"
        elif state == "block_comment":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state == "line_comment" and char == "\\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


DELETE_CALLER = function_body(SOURCE, "_upnp_delete_redir(unsigned short eport, int proto)")

PREAMBLE = r"""
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>

#define ENABLE_LEASEFILE 1

static int mock_aggregate_status;
static int mock_primary_removed;
static int mock_lease_status;
static int mock_lease_errno;
static int firewall_calls;
static int lease_calls;
static unsigned short observed_eport;
static int observed_proto;

int delete_redirect_and_filter_rules_ex(unsigned short eport, int proto,
                                        int *primary_removed)
{
    ++firewall_calls;
    observed_eport = eport;
    observed_proto = proto;
    *primary_removed = mock_primary_removed;
    return mock_aggregate_status;
}

int lease_file_remove(unsigned short eport, int proto)
{
    ++lease_calls;
    assert(eport == observed_eport);
    assert(proto == observed_proto);
    if (mock_lease_status < 0)
        errno = mock_lease_errno;
    return mock_lease_status;
}

static void reset_case(int aggregate_status, int primary_removed,
                       int lease_status, int lease_errno)
{
    mock_aggregate_status = aggregate_status;
    mock_primary_removed = primary_removed;
    mock_lease_status = lease_status;
    mock_lease_errno = lease_errno;
    firewall_calls = 0;
    lease_calls = 0;
    observed_eport = 0;
    observed_proto = 0;
}
"""

TESTS = r"""
int main(void)
{
    int result;

    /* Peer/DSCP cleanup can fail after the primary DNAT/filter mapping is gone.
     * The lease must still be removed, while the aggregate firewall error stays
     * visible to the caller. */
    reset_case(-1, 1, 0, 0);
    result = _upnp_delete_redir(4567, 6);
    assert(result == -1);
    assert(firewall_calls == 1);
    assert(lease_calls == 1);
    assert(observed_eport == 4567 && observed_proto == 6);

    /* An incomplete or failed primary delete must preserve the lease. */
    reset_case(-1, 0, 0, 0);
    result = _upnp_delete_redir(4567, 6);
    assert(result == -1);
    assert(firewall_calls == 1);
    assert(lease_calls == 0);

    /* A clean secondary path does not change the no-primary-removal decision. */
    reset_case(0, 0, 0, 0);
    result = _upnp_delete_redir(4567, 17);
    assert(result == 0);
    assert(firewall_calls == 1);
    assert(lease_calls == 0);

    /* Lease persistence failure overrides a successful firewall deletion. */
    reset_case(0, 1, -1, EACCES);
    errno = 0;
    result = _upnp_delete_redir(7654, 17);
    assert(result == -1);
    assert(errno == EACCES);
    assert(firewall_calls == 1);
    assert(lease_calls == 1);

    /* Fully successful primary and secondary cleanup removes the lease once. */
    reset_case(0, 1, 0, 0);
    result = _upnp_delete_redir(7654, 6);
    assert(result == 0);
    assert(firewall_calls == 1);
    assert(lease_calls == 1);

    puts("MiniUPnP primary delete/lease integration with injected backends: PASS");
    return 0;
}
"""


def main() -> None:
    source = PREAMBLE + "\\n" + DELETE_CALLER + "\\n" + TESTS
    with tempfile.TemporaryDirectory(prefix="miniupnp-delete-integration-") as temp:
        c_path = Path(temp) / "delete_integration.c"
        binary_path = Path(temp) / "delete_integration"
        c_path.write_text(source)
        subprocess.run(
            [
                os.environ.get("CC", "cc"),
                "-std=gnu89",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-fsanitize=address,undefined",
                "-fno-omit-frame-pointer",
                str(c_path),
                "-o",
                str(binary_path),
            ],
            check=True,
            cwd=ROOT,
        )
        env = os.environ.copy()
        env["ASAN_OPTIONS"] = "detect_leaks=1"
        subprocess.run([str(binary_path)], check=True, cwd=ROOT, env=env)


if __name__ == "__main__":
    main()
