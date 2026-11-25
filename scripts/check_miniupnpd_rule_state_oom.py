#!/usr/bin/env python3
"""Prevent NULL dereferences and partial cleanup decisions on UPnP snapshot OOM."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "trunk/user/miniupnpd/miniupnpd-2.x/upnpredirect.c").read_text()


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


scan = function_body(SOURCE, "get_upnp_rules_state_list(int max_rules_number_target)")
assert scan.index("nextruletoclean_timestamp = 0") < scan.index(
    "tmp = malloc(sizeof(struct rule_state))"
)
assert scan.count("allocation_failed = 1") == 2
assert scan.index("if (!allocation_failed)") < scan.index("get_peer_rule_by_index(")
assert scan.index("free(tmp);") < scan.index("if (allocation_failed)")
failure = scan.index("if (allocation_failed)")
cleanup = scan.index("while (list)", failure)
early_return = scan.index("return 0;", cleanup)
expiry_scan = scan.index("/* remove the redirections that need to be removed */")
assert cleanup < early_return < expiry_scan
assert "list = tmp->next;" in scan[cleanup:early_return]
assert "free(tmp);" in scan[cleanup:early_return]
assert "skipping cleanup" in scan[failure:early_return]

print("MiniUPnP rule-snapshot allocation fail-closed contract: PASS")
