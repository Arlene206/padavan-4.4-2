#!/usr/bin/env python3
"""Prevent stale-pidfile signals and plaintext MentoHUST credential exposure."""
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
    if "pidfile" in content or re.search(r"\bkill\s+-9\b", content):
        raise SystemExit(
            "legacy wrapper pidfile safety: FAIL: unsafe stale PID handling remains in "
            + str(source.relative_to(ROOT))
        )

mentohust = SOURCES[1].read_text()
for token in (
    "umask 077",
    'mktemp "$conf_file.XXXXXX"',
    'chmod 600 "$conf_tmp"',
    'mv -f "$conf_tmp" "$conf_file"',
    'func_restrict_private_config "$bin_conf_file"',
    'if [ -L "$conf_file" ] || [ -d "$conf_file" ]; then',
    'case "$value" in',
    'newline=${newline%x}',
):
    if token not in mentohust:
        raise SystemExit(f"MentoHUST credential security: FAIL: missing private publication contract {token}")
if "sed -i" in mentohust:
    raise SystemExit("MentoHUST credential security: FAIL: unsafe sed-based config interpolation remains")

test = TEST.read_text()
for marker in (
    "2147483647",
    "scutclient wrapper consumed or rewrote",
    "mentohust wrapper consumed or rewrote",
    "generated MentoHUST credentials are not stored mode 0600",
    "existing MentoHUST config permissions were not restricted",
    "MentoHUST accepted a symlink config destination",
    "MentoHUST accepted a line-break-bearing setting",
    "MentoHUST accepted a carriage-return-bearing setting",
    "Legacy wrappers preserve stale-pidfile safety and MentoHUST credential confidentiality",
):
    if marker not in test:
        raise SystemExit(f"legacy wrapper security: FAIL: missing regression marker {marker}")

print("Legacy wrapper stale-pidfile and MentoHUST credential security contracts: PASS")
