#!/usr/bin/env python3
"""Bind indirect RAETH MMD operations to one uninterrupted MDIO lock."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RAETH = ROOT / "trunk/linux-4.4.x/drivers/net/ethernet/raeth"
SOURCE = (RAETH / "mii_mgr.c").read_text()
POLICY = (RAETH / "mii_mgr_indirect_policy.h").read_text()
TEST = (RAETH / "tests/mii_mgr_indirect_policy.c").read_text()
WORKFLOW = (ROOT / ".github/workflows/switch-cli-stability.yml").read_text()


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError(f"unterminated function: {signature}")


assert '#include "mii_mgr_indirect_policy.h"' in SOURCE

read_locked = function_body(SOURCE, "static u32 __mii_mgr_read_locked(")
write_locked = function_body(SOURCE, "static u32 __mii_mgr_write_locked(")
read_wrapper = function_body(SOURCE, "static u32 __mii_mgr_read(")
write_wrapper = function_body(SOURCE, "static u32 __mii_mgr_write(")
read_cb = function_body(SOURCE, "mii_mgr_indirect_locked_read(")
write_cb = function_body(SOURCE, "mii_mgr_indirect_locked_write(")
read_public = function_body(SOURCE, "u32 mii_mgr_read(")
write_public = function_body(SOURCE, "u32 mii_mgr_write(")

for label, body in (("locked read primitive", read_locked),
                    ("locked write primitive", write_locked)):
    assert "spin_lock_irqsave" not in body, label
    assert "spin_unlock_irqrestore" not in body, label
    assert "mii_mgr_busy_timeout_expired" in body, label

for label, body, primitive in (
    ("single read wrapper", read_wrapper, "__mii_mgr_read_locked("),
    ("single write wrapper", write_wrapper, "__mii_mgr_write_locked("),
):
    lock_at = body.index("spin_lock_irqsave(&ei_local->mdio_lock")
    call_at = body.index(primitive)
    unlock_at = body.index("spin_unlock_irqrestore(&ei_local->mdio_lock")
    assert lock_at < call_at < unlock_at, label

assert "__mii_mgr_read_locked(" in read_cb and "spin_lock_irqsave" not in read_cb
assert "__mii_mgr_write_locked(" in write_cb and "spin_lock_irqsave" not in write_cb

for label, body, policy_call in (
    ("indirect MMD read", read_public, "mii_mgr_indirect_read("),
    ("indirect MMD write", write_public, "mii_mgr_indirect_write("),
):
    lock_at = body.index("spin_lock_irqsave(&ei_local->mdio_lock")
    disable_at = body.index("set_an_polling(0)")
    transaction_at = body.index(policy_call)
    restore_at = body.index("set_an_polling(1)")
    unlock_at = body.index("spin_unlock_irqrestore(&ei_local->mdio_lock")
    assert lock_at < disable_at < transaction_at < restore_at < unlock_at, label
    assert body.index("return status;", transaction_at) > unlock_at, label

assert "mii_mgr_indirect_locked_write" in read_public
assert "mii_mgr_indirect_locked_read" in read_public
assert "mii_mgr_indirect_locked_write" in write_public
assert "0x1fU" in POLICY and "0x10U" in POLICY
assert POLICY.index("0x1fU") < POLICY.index("(phy_register >> 2)") < POLICY.index("0x10U")
for signature in ("u32 mii_mgr_read_cl45(", "u32 mii_mgr_write_cl45("):
    body = function_body(SOURCE, signature)
    lock_at = body.index("spin_lock_irqsave(&ei_local->mdio_lock")
    address_at = body.index("mii_mgr_cl45_set_address(")
    unlock_at = body.index("spin_unlock_irqrestore(&ei_local->mdio_lock")
    assert lock_at < address_at < unlock_at, signature

for case in (
    "test_read_sequence_is_one_locked_transaction",
    "test_read_failures_clear_output_and_stop_sequence",
    "test_write_sequence_uses_low_and_high_words",
    "test_write_failures_stop_before_later_registers",
):
    assert case in TEST, case

filters = WORKFLOW.split("jobs:", 1)[0]
push, pull_request = filters.split("  pull_request:", 1)
for path in (
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/mii_mgr_indirect_policy.h",
    "trunk/linux-4.4.x/drivers/net/ethernet/raeth/tests/mii_mgr_indirect_policy.c",
    "scripts/check_mii_mgr_indirect_locking.py",
):
    assert push.count('"' + path + '"') == 1, ("push", path)
    assert pull_request.count('"' + path + '"') == 1, ("pull_request", path)

print("RAETH indirect MMD page/data operations are serialized as one MDIO transaction: PASS")
