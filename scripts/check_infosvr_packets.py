#!/usr/bin/env python3
"""Check infosvr UDP receive initialization and request-prefix bounds."""
from pathlib import Path
import re

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


source = read("trunk/user/infosvr/src/infosvr.c")
asus = read("trunk/user/infosvr/src/asus.c")
policy = read("trunk/user/infosvr/src/infosvr_packet_policy.h")
test = read("trunk/user/infosvr/src/tests/infosvr_packet_policy.c")
ibox = read("trunk/user/shared/src/include/ibox.h")
workflow = read(".github/workflows/infosvr-packet-stability.yml")
handoff = read("SESSION_HANDOFF.md")

for required_heading in (
    "## Current repository state",
    "## Active task",
    "## Verification boundary",
    "## Exact first action for the next GPT",
):
    assert required_heading in handoff, required_heading

for token in ("memset(packet, 0, size)", "errno = EINVAL",
              "received >= required", "required != 0"):
    assert token in policy, token

reader = function_body(source, "read_packet(int fd, void *packet, int size)")
prepare = reader.index("infosvr_packet_buffer_prepare(packet, (size_t)size)")
receive = reader.index("recvfrom(fd, packet, (size_t)size")
assert prepare < receive
assert "sizeof(*packet)" not in reader
assert "return res;" in reader
assert "return (res == size);" not in reader

processor = function_body(
    source, "process_packet(int fd, void *packet, int packet_len, char *ifname)"
)
base_guard = processor.index("infosvr_packet_length_valid((size_t)packet_len, sizeof(IBOX_COMM_PKT_HDR))")
service_read = processor.index("hdr->ServiceID")
assert base_guard < service_read
extended_guard = processor.index("offsetof(IBOX_COMM_PKT_HDR_EX, Password)")
mac_read = processor.index("hdr->MacAddress")
assert service_read < extended_guard < mac_read
assert "packet_len < 0" in processor

main = function_body(source, "main(int argc, char *argv[])")
receive_call = main.index("res = read_packet(g_fd4, packet, sizeof(packet))")
minimum_check = main.index("res < (int)sizeof(IBOX_COMM_PKT_HDR)", receive_call)
process_call = main.index("process_packet(g_fd4, packet, res, ifname)", minimum_check)
assert receive_call < minimum_check < process_call

mac_source = function_body(asus, "get_hwaddr(char *ifname, void *buf, int size)")
assert "size < 6" in mac_source
assert "!ether_atoe(macaddr, buf)" in mac_source
use = processor.index("if (!get_hwaddr(ifname, hwaddr, sizeof(hwaddr)))")
compare = processor.index("memcmp(hdr->MacAddress, hwaddr")
assert use < compare

assert "BYTE Password[32]" in ibox
assert "BYTE MacAddress[6]" in ibox
for token in (
    "full receive buffer is initialized",
    "unused tail are zero",
    "truncated base IBOX header is rejected",
    "truncated extended MAC prefix is rejected",
    "NULL receive buffer is rejected",
    "infosvr packet initialization and length policy: PASS",
):
    assert token in test, token

paths = (
    ".github/workflows/infosvr-packet-stability.yml",
    "SESSION_HANDOFF.md",
    "scripts/check_infosvr_packets.py",
    "trunk/user/infosvr/src/infosvr.c",
    "trunk/user/infosvr/src/asus.c",
    "trunk/user/infosvr/src/infosvr_packet_policy.h",
    "trunk/user/infosvr/src/tests/infosvr_packet_policy.c",
    "trunk/user/shared/src/include/ibox.h",
)
pre_jobs = workflow.split("jobs:", 1)[0]
push, pull = pre_jobs.split("  pull_request:", 1)
for path in paths:
    item = '"' + path + '"'
    assert push.count(item) == 1, "push path filter: " + path
    assert pull.count(item) == 1, "pull_request path filter: " + path

print("infosvr packet receive and bounds source contracts: PASS")
