#!/usr/bin/env python3
"""Validate compact AI handoff records and lossless archives."""
from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HANDOFF_LIMIT = 64 * 1024
ROADMAP_LIMIT = 128 * 1024
LOG_LIMIT = 128 * 1024
ARCHIVE_LIMIT = 512 * 1024
LOG_WINDOW = 20


def fail(message: str) -> None:
    print(f"AI handoff documentation check: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def read_bytes(rel: str) -> bytes:
    path = ROOT / rel
    if not path.is_file():
        fail(f"missing required file: {rel}")
    return path.read_bytes()


def git_blob_sha(data: bytes) -> str:
    header = f"blob {len(data)}\0".encode("ascii")
    return hashlib.sha1(header + data).hexdigest()


def check_limit(rel: str, limit: int) -> bytes:
    data = read_bytes(rel)
    if len(data) > limit:
        fail(f"{rel} is {len(data)} bytes; hard limit is {limit}")
    return data


def check_archive_index(index_rel: str, archive_root: str, heading_pattern: bytes) -> int:
    index_data = read_bytes(index_rel).decode("utf-8")
    rows = {}
    for line in index_data.splitlines():
        if not line.startswith("| " + chr(96)):
            continue
        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) < 5:
            fail(f"malformed archive index row in {index_rel}: {line}")
        rel = cells[0].strip(chr(96))
        try:
            count = int(cells[1])
            size = int(cells[3])
        except ValueError:
            fail(f"invalid count/size in {index_rel}: {line}")
        sha = cells[4].strip(chr(96))
        data = read_bytes(rel)
        if len(data) != size:
            fail(f"indexed byte count for {rel} is {size}, actual {len(data)}")
        if len(data) > ARCHIVE_LIMIT:
            fail(f"archive shard exceeds 512 KiB: {rel} ({len(data)} bytes)")
        actual_sha = git_blob_sha(data)
        if actual_sha != sha:
            fail(f"Git blob SHA mismatch for {rel}: index {sha}, actual {actual_sha}")
        actual_count = len(re.findall(heading_pattern, data, flags=re.MULTILINE))
        if actual_count != count:
            fail(f"heading/record count mismatch for {rel}: index {count}, actual {actual_count}")
        rows[rel] = True

    root = ROOT / archive_root
    actual_files = {p.relative_to(ROOT).as_posix() for p in root.rglob("*.md") if p.name != "INDEX.md"}
    if actual_files != set(rows):
        missing = sorted(actual_files - set(rows))
        stale = sorted(set(rows) - actual_files)
        fail(f"archive inventory mismatch in {index_rel}; unindexed={missing}, missing={stale}")
    if not rows:
        fail(f"no archive entries found in {index_rel}")
    return len(rows)


def main() -> None:
    bootstrap = read_bytes("AI_BOOTSTRAP_PROMPT.md").decode("utf-8")
    protocol = read_bytes("AI_DEVELOPMENT_PROTOCOL.md").decode("utf-8")
    handoff = check_limit("SESSION_HANDOFF.md", HANDOFF_LIMIT).decode("utf-8")
    roadmap = check_limit("REFACTOR_ROADMAP.md", ROADMAP_LIMIT).decode("utf-8")
    log = check_limit("dev_log.md", LOG_LIMIT).decode("utf-8")

    if "docs/dev-log/INDEX.md" not in bootstrap or "scripts/check_ai_handoff_docs.py" not in bootstrap:
        fail("bootstrap does not point to canonical archive index and validator")
    for required in ("cross-conversation", "512 KiB", "docs/dev-log/YYYY/MM/part-NNN.md"):
        if required not in protocol:
            fail(f"protocol is missing mandatory governance rule: {required}")
    for heading in ("## Current repository state", "## Active task", "## Verification boundary", "## Exact first action for the next GPT"):
        if heading not in handoff:
            fail(f"handoff missing required section: {heading}")
    if "## Whole-repository module status" not in roadmap or "AUDITING" not in roadmap:
        fail("roadmap does not contain a current module status dashboard")

    log_count = len(re.findall(r"(?m)^## \d{4}-\d{2}-\d{2}\b", log))
    if log_count != LOG_WINDOW:
        fail(f"dev_log.md must contain exactly {LOG_WINDOW} complete entries, found {log_count}")

    log_files = check_archive_index(
        "docs/dev-log/INDEX.md", "docs/dev-log", rb"^## \d{4}-\d{2}-\d{2}\b"
    )
    hand_files = check_archive_index(
        "docs/handoffs/INDEX.md", "docs/handoffs", rb"^## "
    )
    road_files = check_archive_index(
        "docs/roadmap-history/INDEX.md", "docs/roadmap-history", rb"^## "
    )
    print(
        "AI handoff documentation check: PASS "
        f"(root log={log_count} entries; archive shards: dev-log={log_files}, "
        f"handoff={hand_files}, roadmap={road_files})"
    )


if __name__ == "__main__":
    main()
