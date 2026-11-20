#!/usr/bin/env python3
"""Prevent lease replay from reading records it appends back to an undeleted file."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/upnpredirect.c").read_text()


def function_body(source, signature):
    position = source.index(signature)
    brace = source.index("{", position)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[position:index + 1]
    raise AssertionError("unterminated function: " + signature)


replay = function_body(SOURCE, "reload_from_lease_file()")
unlink_at = replay.index("if(unlink(lease_file) < 0)")
error_test = replay.index("if (saved_errno != ENOENT)", unlink_at)
close_at = replay.index("(void)fclose(fd)", error_test)
return_at = replay.index("return -1;", close_at)
read_at = replay.index("while(fgets(line, sizeof(line), fd))")
persist_at = replay.index("lease_file_add(eport, iaddr, iport, proto_atoi(proto)")
assert unlink_at < error_test < close_at < return_at < read_at
assert read_at < persist_at
assert "saved_errno = errno ? errno : EIO;" in replay
assert "errno = saved_errno;" in replay
assert "saved_errno != ENOENT" in replay

print("MiniUPnP lease replay unlink-failure guard: PASS")
