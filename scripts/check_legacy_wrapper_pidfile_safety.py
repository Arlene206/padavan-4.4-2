#!/usr/bin/env python3
"""Prevent short-lived RC wrappers from killing stale pidfile PIDs."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCES = (
    ROOT / "trunk/user/scutclient/scripts/scutclient.sh",
    ROOT / "trunk/user/mentohust/scripts/mentohust.sh",
)
TEST = ROOT / "scripts/tests/legacy_wrapper_pidfile_safety.sh"

for source in SOURCES:
    content = source.read_text()
    if "pidfile" in content or re.search(r"\\bkill\\s+-9\\b", content):
        raise SystemExit(f"legacy wrapper pidfile safety: FAIL: unsafe stale PID handling remains in {source.relative_to(ROOT)}")

test = TEST.read_text()
for marker in ("2147483647", "scutclient wrapper consumed or rewrote",
               "mentohust wrapper consumed or rewrote",
               "never signal stale pidfile PIDs"):
    if marker not in test:
        raise SystemExit(f"legacy wrapper pidfile safety: FAIL: missing regression marker {marker}")

print("Legacy scutclient/mentohust stale-pidfile safety contracts: PASS")
