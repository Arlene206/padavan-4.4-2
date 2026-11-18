#!/usr/bin/env python3
"""Validate current AI handoff records and batched, lossless history archives."""
from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HANDOFF_LIMIT = 64 * 1024
ROADMAP_LIMIT = 128 * 1024
LOG_TARGET = 384 * 1024
LOG_LIMIT = 512 * 1024
ARCHIVE_LIMIT = 512 * 1024
LOG_RETAIN = 20
LOG_ARCHIVE_BATCH = 100
LOG_ROTATE_AT = LOG_RETAIN + LOG_ARCHIVE_BATCH


def fail(message: str) -> None:
    print(f"AI handoff documentation check: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def read_bytes(rel: str) -> bytes:
    path = ROOT / rel
    if not path.is_file():
        fail(f"missing required file: {rel}")
    return path.read_bytes()


def git_blob_sha(data: bytes) -> str:
    header = b"blob " + str(len(data)).encode("ascii") + bytes((0,))
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
    actual_files = {
        p.relative_to(ROOT).as_posix()
        for p in root.rglob("*.md")
        if p.name != "INDEX.md"
    }
    if actual_files != set(rows):
        missing = sorted(actual_files - set(rows))
        stale = sorted(set(rows) - actual_files)
        fail(f"archive inventory mismatch in {index_rel}; unindexed={missing}, missing={stale}")
    if not rows:
        fail(f"no archive entries found in {index_rel}")
    return len(rows)



def workflow_paths(workflow: str, event: str) -> list[str]:
    """Return the literal paths list for one workflow event."""
    event_match = re.search(
        rf"(?ms)^  {re.escape(event)}:\s*\n(.*?)(?=^  [A-Za-z_][A-Za-z0-9_-]*:|^[A-Za-z_][A-Za-z0-9_-]*:)",
        workflow,
    )
    if not event_match:
        return []
    paths_match = re.search(
        r"(?m)^    paths:\s*\n((?:^      - .*\n?)*)",
        event_match.group(1),
    )
    if not paths_match:
        return []
    result = []
    for line in paths_match.group(1).splitlines():
        value = line.strip()[2:].strip()
        if len(value) >= 2 and value[0] in ("'", '"') and value[-1] == value[0]:
            value = value[1:-1]
        result.append(value)
    return result


def check_ci_trigger_governance() -> None:
    """Protect test coverage while rejecting known broad-trigger regressions."""
    workflow_root = ROOT / ".github/workflows"
    inventory = (workflow_root / "process-lifecycle-inventory.yml").read_text()
    if '"trunk/user/**"' not in inventory:
        fail("whole-userland Process lifecycle inventory trigger was removed")

    for workflow_path in sorted(workflow_root.glob("*.yml")):
        source = workflow_path.read_text()
        if re.search(r"""(?m)^\s*-\s*['"]trunk/user/\*\*['"]\s*$""", source):
            if workflow_path.name != "process-lifecycle-inventory.yml":
                fail(
                    f"blanket trunk/user/** trigger is forbidden in focused workflow: "
                    f"{workflow_path.name}"
                )

    process_workflow = (workflow_root / "process-lifecycle-regression.yml").read_text()
    push_paths = workflow_paths(process_workflow, "push")
    pr_paths = workflow_paths(process_workflow, "pull_request")
    if not push_paths or not pr_paths:
        fail("Process lifecycle regression must define push and pull_request path filters")
    if len(push_paths) != len(set(push_paths)) or len(pr_paths) != len(set(pr_paths)):
        fail("Process lifecycle regression contains duplicate path-filter entries")
    if set(push_paths) != set(pr_paths):
        fail("Process lifecycle regression push/pull_request path sets must match")
    if "trunk/user/**" in push_paths or "trunk/user/**" in pr_paths:
        fail("Process lifecycle regression must use an explicit allowlist")

    xy_workflow = (workflow_root / "xy-c1-firmware-validation.yml").read_text()
    for step_marker in (
        "- name: Build XY-C1",
        "- name: Verify TRX output",
        "- uses: actions/upload-artifact@",
    ):
        start = xy_workflow.find(step_marker)
        if start < 0:
            fail(f"XY-C1 workflow is missing required step: {step_marker}")
        next_step = re.search(r"(?m)^      - ", xy_workflow[start + len(step_marker):])
        end = start + len(step_marker) + next_step.start() if next_step else len(xy_workflow)
        step = xy_workflow[start:end]
        if not re.search(
            r"""(?m)^        if: steps\.firmware_inputs\.outputs\.changed == ['"]true['"]$""",
            step,
        ):
            fail(f"XY-C1 expensive step is not gated by production-input detection: {step_marker}")

    detector = (ROOT / "scripts/check_xy_c1_firmware_inputs.py").read_text()
    for required in (
        'if event_name != "push":',
        'if paths is None:',
        'if any(not is_non_firmware_input(path) for path in paths):',
        'return True, paths, "firmware production inputs changed"',
    ):
        if required not in detector:
            fail(f"XY-C1 production-input detector lost fail-safe behavior: {required}")
    print("CI trigger governance and XY-C1 firmware build gate: PASS")



def main() -> None:
    bootstrap = read_bytes("AI_BOOTSTRAP_PROMPT.md").decode("utf-8")
    protocol = read_bytes("AI_DEVELOPMENT_PROTOCOL.md").decode("utf-8")
    handoff = check_limit("SESSION_HANDOFF.md", HANDOFF_LIMIT).decode("utf-8")
    roadmap = check_limit("REFACTOR_ROADMAP.md", ROADMAP_LIMIT).decode("utf-8")
    log_bytes = check_limit("dev_log.md", LOG_LIMIT)
    log = log_bytes.decode("utf-8")

    if "docs/dev-log/INDEX.md" not in bootstrap or "scripts/check_ai_handoff_docs.py" not in bootstrap:
        fail("bootstrap does not point to the canonical log archive index and validator")
    if "do not claim tests, commits, builds, firmware artifacts or hardware checks that were not actually performed." not in bootstrap:
        fail("bootstrap is missing the complete no-fabrication rule")
    if re.search(r"(?m)^aim tests, commits", bootstrap):
        fail("bootstrap contains a truncated/orphaned no-fabrication sentence")
    if "Every GPT must update the handoff in Git before ending" in protocol:
        fail("protocol unconditionally requires handoff churn and contradicts its materiality gate")
    if "resolve its pending/failed relevant CI before selecting another task" in protocol:
        fail("protocol blocks independent work while relevant CI is merely pending")
    if "pending/failed CI first; once it is closed" in protocol or "failed/pending exact-head `push` CI before choosing another task" in protocol:
        fail("protocol contains a legacy pending-CI prerequisite that can block independent work")
    if "failed/pending `push` CI" in bootstrap:
        fail("bootstrap incorrectly treats pending CI as a prerequisite to new work")
    if "A PENDING run is not a gate on independent work" not in protocol:
        fail("protocol must explicitly allow independent work while CI is pending")
    if "a pending check is not a stop condition" not in bootstrap.lower():
        fail("bootstrap must explicitly allow independent work while CI is pending")
    if "If it remains accurate, leave it untouched." not in protocol:
        fail("protocol must explicitly preserve an accurate handoff without a no-op rewrite")
    for required in (
        "Every session MUST perform a handoff checkpoint before ending",
        "A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information",
        "If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit",
    ):
        if required not in protocol:
            fail(f"protocol is missing mandatory end-of-session checkpoint rule: {required}")
    for required in (
        "Mandatory end-of-session handoff checkpoint",
        "Every session MUST inspect the live repository state",
        "do not create a no-op commit",
    ):
        if required not in bootstrap:
            fail(f"bootstrap is missing mandatory handoff checkpoint rule: {required}")
    if "Mandatory handoff checkpoint" not in handoff or "The checkpoint is mandatory; a document edit is conditional." not in handoff:
        fail("current handoff does not require an end-of-session accuracy checkpoint")
    for required in (
        "cross-conversation",
        "512 KiB",
        "docs/dev-log/YYYY/MM/part-NNN.md",
        "120",
        "oldest 100",
        "384 KiB",
        "Git history",
    ):
        if required not in protocol:
            fail(f"protocol is missing mandatory governance rule: {required}")
    if "20–119" not in protocol or "20–119" not in bootstrap:
        fail("protocol/bootstrap must define the 20–119 root log window")
    if "routine archive copies are prohibited" not in protocol:
        fail("protocol must prohibit routine handoff/roadmap archive snapshots")
    if "do not create new handoff or roadmap snapshot archives" not in protocol:
        fail("protocol must prohibit new routine handoff/roadmap snapshots")
    if "do not add new snapshots for ordinary edits" not in bootstrap:
        fail("bootstrap must prohibit routine handoff/roadmap snapshots")

    for required in (
        "## Documentation decision gate and anti-churn",
        "Recovery is read-only by default",
        "Do not commit merely to synchronize a documented SHA with live main",
        "Allowed standalone documentation-only commits",
        "Do not create a second documentation-only commit for the same logical event",
    ):
        if required not in protocol:
            fail(f"protocol is missing anti-churn governance rule: {required}")

    for required in (
        "Continuous execution across GPT conversations",
        "Recover once, then execute",
        "An audit is a means, not the default endpoint",
        "One logical slice, one atomic commit",
        "Checkpoint before context exhaustion",
        "non-forced compare-and-swap",
    ):
        if required not in protocol:
            fail(f"protocol is missing continuous-execution rule: {required}")

    for required in (
        "## Continuous execution mandate",
        "Git data APIs to create all blobs, one tree, one commit",
        "Before context exhaustion",
        "Do not ask the user what to do next",
    ):
        if required not in bootstrap:
            fail(f"bootstrap is missing continuous-execution rule: {required}")

    for required in (
        "# Cross-session execution mandate",
        "one atomic main commit",
        "Before context exhaustion",
    ):
        if required not in handoff:
            fail(f"handoff is missing cross-session execution rule: {required}")

    for required in (
        "## Documentation anti-churn guard",
        "Recovery is read-only by default",
        "Never commit merely because a GPT has taken over",
        "This is not a ban on useful documentation-only work",
        "If the current handoff remains accurate, leave it untouched",
    ):
        if required not in bootstrap:
            fail(f"bootstrap is missing anti-churn recovery rule: {required}")

    for heading in (
        "## Current repository state",
        "## Active task",
        "## Verification boundary",
        "## Exact first action for the next GPT",
    ):
        if heading not in handoff:
            fail(f"handoff missing required section: {heading}")
    if "## Whole-repository module status" not in roadmap or "AUDITING" not in roadmap:
        fail("roadmap does not contain a current module status dashboard")

    log_count = len(re.findall(r"(?m)^## [0-9]{4}-[0-9]{2}-[0-9]{2}", log))
    if log_count < LOG_RETAIN:
        fail(f"dev_log.md must retain at least {LOG_RETAIN} complete newest entries; found {log_count}")
    if log_count >= LOG_ROTATE_AT:
        fail(
            f"dev_log.md has {log_count} entries; before committing record {LOG_ROTATE_AT}, "
            f"archive the oldest {LOG_ARCHIVE_BATCH} complete records and retain {LOG_RETAIN}"
        )
    if len(log_bytes) > LOG_TARGET and log_count > LOG_RETAIN:
        fail(
            f"dev_log.md is {len(log_bytes)} bytes (target {LOG_TARGET}); "
            f"batch-rotate old records and retain the newest {LOG_RETAIN}"
        )

    check_ci_trigger_governance()

    log_files = check_archive_index(
        "docs/dev-log/INDEX.md", "docs/dev-log", rb"^## [0-9]{4}-[0-9]{2}-[0-9]{2}"
    )
    hand_files = check_archive_index(
        "docs/handoffs/INDEX.md", "docs/handoffs", rb"^## "
    )
    road_files = check_archive_index(
        "docs/roadmap-history/INDEX.md", "docs/roadmap-history", rb"^## "
    )
    print(
        "AI handoff documentation check: PASS "
        f"(root log={log_count}/{LOG_ROTATE_AT - 1} entries, "
        f"batch={LOG_ARCHIVE_BATCH}, retain={LOG_RETAIN}, "
        f"legacy/current archives: dev-log={log_files}, handoff={hand_files}, roadmap={road_files})"
    )


if __name__ == "__main__":
    main()
