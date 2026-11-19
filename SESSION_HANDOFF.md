# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- Current main before this source-contract repair: `40eb3ad8bde1b728c449d907208886fa751f8f55`. Product-source commit `2b646cccdcb24da5d447586893a500fdcf3cfe48` added the rp-L2TP private atomic config writer; its GNU89 ASan/UBSan filesystem test passed on the original source run.
- Checker follow-up `09dfc4be938591b9e9e99f15ce7992c124080d73` fixed the mode expectation and HTTPD checker; checker follow-up `40eb3ad8bde1b728c449d907208886fa751f8f55` fixed the second HTTPD source-body extraction mismatch. Exact-head HTTPD `44640170448` PASS.
- Exact-head VPN run `44640170448` fails only at the source checker’s trigger-path count for `tests/l2tp_config_file.c`. The PR path filter contains the file exactly once; the checker incorrectly counts the later compiler command occurrence as a second path because it scans the entire remainder of the YAML. This commit narrows the scan to the pull_request paths block.
- The original `2b646cc` XY-C1 run `44639411601` was pending at the last confirmed checkpoint. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.

## Active task

- rp-L2TP root config publication is hardened by `l2tp_config_file.h`: it rejects symlinked/foreign-owned config directories, enforces non-writable directory mode despite PID 1’s `umask(0000)`, stages in the same directory and atomically renames, replacing a leaf symlink without following it. GNU89 ASan/UBSan filesystem regression passed in original source run `44639411367`.
- Fixed the VPN source checker’s false positive: it previously counted every occurrence after the YAML `pull_request:` key, including the `cc ... tests/l2tp_config_file.c` command under jobs. It now restricts its count to the actual `pull_request.paths` section before `workflow_dispatch:`. No production code or test behavior changed.
- No local MIPS build, target uClibc test, or physical-router behavior is claimed.

## Exact first action for the next GPT

1. Check this checker-only commit’s exact-head VPN lifecycle and AI handoff docs results. The VPN source contract must pass while continuing to require exactly one push and PR path entry for both the config header and regression file.
2. Recheck the original product SHA `2b646cccdcb24da5d447586893a500fdcf3cfe48` XY-C1 run `44639411601`; checker-only commits do not replace same-source firmware verification.
3. Continue the P1 RC startup-result propagation item: trace any remaining launch path whose real failure is discarded, preserving PID 1 fatal prerequisites and degraded notification behavior.

## Verification boundary

- L1: rp-L2TP config directory/leaf handling uses owner/mode/symlink validation and same-directory atomic publication; final config mode is 0644.
- L2: original GNU89 ASan/UBSan filesystem regression passed on source run `44639411367`. VPN exact-head run `44640170448` failed only in a workflow path-count checker bug; this scoped checker correction awaits a fresh exact-head rerun. HTTPD exact-head run `44640170448` PASS.
- L3: original product SHA `2b646cc` XY-C1 run `44639411601` was pending at last confirmed check; re-query its status. No checker-only SHA substitutes for that product-source result.
- L4: target MIPS/uClibc behavior, concurrent WAN transitions and physical-router validation remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, production-source baseline, exact-head Actions, module state and the next task. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit; do not create a no-op commit.

## Before context exhaustion

- Finish the current safe source/test slice and commit it atomically before handoff; do not publish related multi-file changes as sequential partial commits.
- Re-fetch live `main`, create one complete Git tree and one commit with that exact parent, then update main only with `force=false` and the exact parent as `expected_sha`. Never force-push.
- Query relevant Actions by exact source SHA and record PASS/FAIL/PENDING with run IDs. Pending validation does not block independent work and must not be reported as complete.
- Record the production-source baseline, test commands/results, exact CI state, unresolved limitations and the next concrete task. Distinguish host L2, firmware L3 and device L4 evidence.
- Confirm all work is committed and remotely present before claiming it is preserved across conversations.

## Development invariants

- Main-only; never alter the frozen backup or develop on a legacy PR.
- One complete tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Keep L1/L2/L3/L4 distinct. Host tests are not firmware or hardware validation.
