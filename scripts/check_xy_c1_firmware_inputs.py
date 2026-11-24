#!/usr/bin/env python3
"""Fail-safe XY-C1 production-input detection using the complete Git tree diff."""
from __future__ import annotations

import os
import re
import subprocess
from pathlib import Path, PurePosixPath
from typing import Callable, Optional

ROOT = Path(__file__).resolve().parents[1]
ZERO_SHA = "0" * 40
SHA_RE = re.compile(r"^[0-9a-f]{40}$")
DOCUMENTATION_FILES = {
    "AI_BOOTSTRAP_PROMPT.md",
    "AI_DEVELOPMENT_PROTOCOL.md",
    "SESSION_HANDOFF.md",
    "REFACTOR_ROADMAP.md",
    "dev_log.md",
}


def is_non_firmware_input(path: str) -> bool:
    """Host-only and documentation paths skip the build; unknown paths build."""
    if not path or path.startswith("/"):
        return False
    # Changes to the firmware workflow itself require validating its build path.
    if path == ".github/workflows/xy-c1-firmware-validation.yml":
        return False
    parts = PurePosixPath(path).parts
    return (
        path.startswith(".github/")
        or "tests" in parts
        or path.startswith("scripts/check_")
        or path.startswith("scripts/tests/")
        or path.startswith("docs/")
        or path in DOCUMENTATION_FILES
    )


def run_git(args: list[str]) -> subprocess.CompletedProcess[str]:
    """Run one bounded Git operation; failures become a nonzero result."""
    command = ["git", *args]
    try:
        return subprocess.run(
            command, cwd=ROOT, text=True, encoding="utf-8",
            errors="surrogateescape", stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, timeout=60, check=False,
        )
    except (OSError, subprocess.TimeoutExpired, UnicodeError) as error:
        return subprocess.CompletedProcess(command, 1, "", str(error))


def collect_changed_paths(
    before: str,
    after: str,
    git_runner: Callable[[list[str]], subprocess.CompletedProcess[str]] = run_git,
) -> Optional[list[str]]:
    """Return complete changed paths, or None if the comparison is untrustworthy."""
    if not SHA_RE.fullmatch(before or "") or before == ZERO_SHA:
        return None
    if not SHA_RE.fullmatch(after or ""):
        return None
    try:
        head = git_runner(["rev-parse", "HEAD"])
        if head.returncode != 0 or head.stdout.strip() != after:
            return None
        # Push webhook commits[] paths can be absent or truncated; compare Git trees.
        fetched = git_runner(["fetch", "--no-tags", "--depth=1", "origin", before])
        if fetched.returncode != 0:
            return None
        compared = git_runner(["diff", "--name-only", "--no-renames", "-z", before, after])
        if compared.returncode != 0:
            return None
    except (OSError, subprocess.TimeoutExpired, UnicodeError):
        return None
    return [path for path in compared.stdout.split("\0") if path]


def decide_firmware_build(
    event_name: str,
    before: str,
    after: str,
    git_runner: Callable[[list[str]], subprocess.CompletedProcess[str]] = run_git,
) -> tuple[bool, list[str], str]:
    """Manual/PR/unknown events build; valid push events use a complete tree diff."""
    if event_name != "push":
        return True, [], f"{event_name or 'unknown'} event: fail-safe full build"
    paths = collect_changed_paths(before, after, git_runner)
    if paths is None:
        return True, [], "Git before/after comparison unavailable: fail-safe full build"
    if any(not is_non_firmware_input(path) for path in paths):
        return True, paths, "firmware production inputs changed"
    return False, paths, "no firmware production input changed"


def main() -> None:
    changed, paths, reason = decide_firmware_build(
        os.environ.get("EVENT_NAME", ""),
        os.environ.get("BEFORE_SHA", ""),
        os.environ.get("AFTER_SHA", ""),
    )
    output_path = os.environ.get("GITHUB_OUTPUT")
    if not output_path:
        raise SystemExit("GITHUB_OUTPUT is required")
    with open(output_path, "a", encoding="utf-8") as output:
        output.write(f"changed={'true' if changed else 'false'}\n")
    print(f"Firmware production inputs changed: {changed} ({reason})")
    for path in paths:
        print(f"  {path}")


if __name__ == "__main__":
    main()
