#!/usr/bin/env python3
"""Verify VPN PPP secrets and callback paths are safely published before launch."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def function_body(source, signature):
    marker = source.find(signature)
    assert marker >= 0, f"missing function: {signature}"
    opening = source.find("{", marker)
    assert opening >= 0
    depth = 0
    quote = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and nxt == "/":
                block_comment = False
                index += 1
        elif quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and nxt == "/":
            line_comment = True
            index += 1
        elif char == "/" and nxt == "*":
            block_comment = True
            index += 1
        elif char in ("'", '"'):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


client_src = (ROOT / "trunk/user/rc/src/vpn_client.c").read_text()
server_src = (ROOT / "trunk/user/rc/src/vpn_server.c").read_text()
helper = (ROOT / "trunk/user/rc/src/ppp_options_file.h").read_text()
client = function_body(client_src, "start_vpn_client_locked(void)")
server = function_body(server_src, "start_vpn_server_locked(void)")
server_options = function_body(server_src, "gen_vpns_pppd_options(int vpns_type)")

for source in (client_src, server_src):
    assert '#include "ppp_options_file.h"' in source
for token in ("O_NOFOLLOW", "fchmod(fd, private_mode)", "ftruncate(fd, 0)",
              "readlink(path, actual, sizeof(actual))",
              "memcmp(actual, target, target_length)", "unlink(path)"):
    assert token in helper

for body, up, down in (
    (client, "VPNC_PPP_UP_SCRIPT", "VPNC_PPP_DW_SCRIPT"),
    (server, "VPNS_PPP_UP_SCRIPT", "VPNS_PPP_DW_SCRIPT"),
):
    assert 'ppp_options_directory_ready("/tmp/ppp")' in body
    assert f'ppp_options_symlink_ready({up}, "/sbin/rc")' in body
    assert f'ppp_options_symlink_ready({down}, "/sbin/rc")' in body
    assert 'mkdir("/tmp/ppp", 0777)' not in body

assert "ppp_options_file_open(vpnc_opt)" in client
assert "ppp_options_file_finish(fp, vpnc_opt)" in client
assert "fopen(vpnc_opt" not in client
assert client.index("ppp_options_file_finish(fp, vpnc_opt)") < client.index('eval("/usr/sbin/pppd"')

assert "ppp_options_file_open(vpns_opt)" in server_options
assert "ppp_options_file_finish(fp, vpns_opt)" in server_options
assert "chmod(vpns_opt, 0644)" in server_options
assert "ppp_options_file_open(vpns_cfg)" in server
assert "ppp_options_file_finish(fp, vpns_cfg)" in server
assert "chmod(vpns_cfg, 0644)" in server
assert "if (gen_vpns_pppd_options(i_type) != 0)" in server
assert server.index("gen_vpns_pppd_options(i_type) != 0") < server.index("ppp_options_file_open(vpns_sec)")
assert "ppp_options_file_open(vpns_sec)" in server
assert "ppp_options_file_finish(fp, vpns_sec)" in server
assert "fopen(vpns_sec" not in server
assert "if (fp) {" not in server
assert server.index("ppp_options_file_finish(fp, vpns_sec)") < server.index('eval("/usr/sbin/pptpd"')

print("VPN client/server private PPP credential publication contract: PASS")
