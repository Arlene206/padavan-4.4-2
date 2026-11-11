#!/usr/bin/env python3
"""Source contract for private, atomic OpenVPN client credential publication."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1 : pos]
    raise AssertionError(f"unterminated function: {signature}")


def ordered(source: str, tokens: tuple[str, ...], label: str) -> None:
    position = -1
    for token in tokens:
        found = source.find(token, position + 1)
        assert found >= 0, f"{label}: missing or out-of-order {token!r}"
        position = found


source = read("trunk/user/rc/src/vpn_openvpn.c")
helper = read("trunk/user/rc/src/openvpn_client_secret_file.h")
test = read("trunk/user/rc/src/tests/openvpn_client_secret_file.c")
workflow = read(".github/workflows/openvpn-client-secret.yml")

assert '#include "openvpn_client_secret_file.h"' in source
secret_writer = body(source, "static int\nopenvpn_create_client_secret")
assert "openvpn_client_secret_publish(" in secret_writer
assert "fopen(" not in secret_writer and "chmod(" not in secret_writer
assert 'nvram_safe_get("vpnc_user")' in secret_writer
assert 'nvram_safe_get("vpnc_pass")' in secret_writer

client_conf = body(source, "static int\nopenvpn_create_client_conf")
secret_gate = 'if (i_auth == 1 && openvpn_create_client_secret("secret") != 0)'
assert secret_gate in client_conf
assert client_conf.index(secret_gate) < client_conf.index('fp = fopen(conf_file, "w+")')
assert client_conf.index("return 1;", client_conf.index(secret_gate)) < client_conf.index('fp = fopen(conf_file, "w+")')
assert 'fprintf(fp, "auth-user-pass %s\\n", "secret");' in client_conf
assert client_conf.count('openvpn_create_client_secret("secret")') == 1

ordered(helper, [
    "fd = mkstemp(staged_path)",
    "fchmod(fd, S_IRUSR | S_IWUSR)",
    'fp = fdopen(fd, "w")',
    'fprintf(fp, "%s\\n%s\\n", username, password)',
    "fflush(fp)",
    "fsync(fileno(fp))",
    "fclose(fp)",
    "rename(staged_path, path)",
], "private staging and atomic credential publication")
assert "unlink(staged_path)" in helper and "errno = saved_errno" in helper
assert "O_TRUNC" not in helper
assert "assert((before.st_mode & 0777) == 0666)" in test
assert "assert((after.st_mode & 0777) == 0600)" in test
assert 'count_staged_files(directory, "blocked.tmp.") == 0' in test

paths = (
    "trunk/user/rc/src/vpn_openvpn.c",
    "trunk/user/rc/src/openvpn_client_secret_file.h",
    "trunk/user/rc/src/tests/openvpn_client_secret_file.c",
    "scripts/check_openvpn_client_secret.py",
    ".github/workflows/openvpn-client-secret.yml",
)
push, remaining = workflow.split("  pull_request:", 1)
pr = remaining.split("  workflow_dispatch:", 1)[0]
for path in paths:
    quoted = "'" + path + "'"
    assert push.count(quoted) == 1, f"push path filter must contain exactly one {path}"
    assert pr.count(quoted) == 1, f"pull_request path filter must contain exactly one {path}"

print("OpenVPN client credential source and workflow contracts: PASS")
