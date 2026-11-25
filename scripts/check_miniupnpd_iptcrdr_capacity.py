#!/usr/bin/env python3
"""Guard MiniUPnP port-mapping scans against fixed-capacity reallocation."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr.c").read_text()
POLICY = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptcrdr_capacity_policy.h").read_text()


def function_body(source, signature):
    position = source.index(signature)
    while True:
        brace = source.find("{", position)
        semicolon = source.find(";", position)
        assert brace >= 0, "function definition missing: " + signature
        if semicolon >= 0 and semicolon < brace:
            position = source.find(signature, position + len(signature))
            assert position >= 0, "function definition missing: " + signature
            continue
        depth = 0
        for index in range(brace, len(source)):
            if source[index] == "{":
                depth += 1
            elif source[index] == "}":
                depth -= 1
                if depth == 0:
                    return source[position:index + 1]
        raise AssertionError("unterminated function: " + signature)


body = " ".join(function_body(
    SOURCE,
    "get_portmappings_in_range(unsigned short startport, unsigned short endport,"
).split())
policy = " ".join(POLICY.split())

assert "current > UINT_MAX / 2" in policy
assert "maximum_elements = ((size_t)-1) / sizeof(unsigned short);" in policy
assert "(size_t)current > maximum_elements / 2" in policy
assert "*next = (size_t)current * 2;" in policy

growth = body.index("if (*number >= capacity)")
policy_call = body.index(
    "miniupnpd_portmapping_capacity_next( capacity, &next_capacity)", growth
)
realloc_call = body.index(
    "tmp = realloc(array, sizeof(unsigned short) * next_capacity);", policy_call
)
realloc_guard = body.index("if (!tmp)", realloc_call)
realloc_end = body.index("break;", realloc_guard)
realloc_failure = body[realloc_guard:realloc_end]

assert "size_t next_capacity;" in body
assert "capacity = 128;" in body
assert growth < policy_call < realloc_call < realloc_guard
assert "array = tmp;" in body[realloc_guard:]
assert "capacity = (unsigned int)next_capacity;" in body[realloc_guard:]
assert "*number = 0;" in realloc_failure
assert "free(array);" in realloc_failure and "array = NULL;" in realloc_failure
assert "errno = saved_errno;" in realloc_failure
assert "realloc(array, sizeof(unsigned short)*capacity)" not in body

overflow_guard = body.index(
    "if (miniupnpd_portmapping_capacity_next( capacity, &next_capacity) != 0)", growth
)
overflow_end = body.index("break;", overflow_guard)
overflow_failure = body[overflow_guard:overflow_end]
assert "*number = 0;" in overflow_failure
assert "free(array);" in overflow_failure and "array = NULL;" in overflow_failure
assert "errno = saved_errno;" in overflow_failure

print("MiniUPnP port-mapping dynamic capacity source contract: PASS")
