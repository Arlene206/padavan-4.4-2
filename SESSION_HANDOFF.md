# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- PID 1 degraded-readiness implementation is committed on main as `9ad1570aa0bc8a9e1c5ff26a85266d53043c526a`. Its GNU89 ASan/UBSan policy binary and existing `dev_init.sh` shell regression passed in the Process job; the subsequent first-attempt Python source contract failed on the C-function parser buffer reference.
- CI-contract follow-up `e8fb566f3a122f75c4f648543c2a7aa516b00316` corrected that parser and the HTTPD three-state boot-result source assertions. Exact-head Process `44643316833`, HTTPD `44643316983` and AI handoff docs `44643316847` all PASS on this SHA.
- XY-C1 firmware run `44643316929` on the e8 checker/workflow SHA remained IN_PROGRESS at the last query. The prior production-source SHA `9ad1570` XY run `44642485538` was also still IN_PROGRESS; re-query exact source SHA before reporting L3.
- Earlier rp-L2TP product SHA `2b646cccdcb24da5d447586893a500fdcf3cfe48` passed XY-C1 Build/TRX/artifact in `44639411601`; artifact `14584429568`. This does not verify the newer PID 1 source.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, archival and untouched.

## Active task

- P1 RC startup-result propagation: `init_router()` carries bridge initialization failures into the PID 1 READY/DEGRADED decision; `init_bridge()` had one remaining caller-side gap.
- Confirmed defect: `gen_ralink_config_2g/5g()` returns `-1` when atomic wireless configuration publication fails, but six calls in `init_bridge()` discarded those results (four compile-time preparation branches and two post-start configuration restores). The radio-restart path already treats generation failure as fatal to that restart.
- This source slice captures and records every preparation/restore result in `rc_bridge_init_status_t`, preserving first failure while continuing independent bridge setup. The bridge host regression pins failure propagation; the Process workflow source contract checks every callsite and compiles the policy test under GNU89 + ASan/UBSan.
- Main baseline: `bd36a1c9da40c6f58aa810f1f1d38f70b06cf50d`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and is untouched.

## Exact first action for the next GPT

1. Query exact-head Process lifecycle, AI handoff docs and XY-C1 Validation for the wireless-configuration-result commit. Check the complete job logs and confirm Build XY-C1, TRX verification and artifact upload actually ran (not merely that the workflow succeeded).
2. Repair any real exact-SHA regression without weakening the config-publication result assertions; if any run is pending, proceed with independent P1 work.
3. Continue the next confirmed RC boot/service caller that discards a required configuration or launch result. Preserve GNU89/MIPS/uClibc compatibility and existing fail-closed/rollback invariants.

## Active task

- Latest parent at task start: `bd36a1c9da40c6f58aa810f1f1d38f70b06cf50d`. Main-only development; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Implemented in this slice: all six `gen_ralink_config_2g/5g()` calls inside `init_bridge()` now store their return value in `operation_status` and immediately record the operation in `bridge_status`. A failed atomic wireless config publish now makes bridge initialization return failure and feeds PID 1's existing DEGRADED status path; independent bridge startup steps still run.
- Added a GNU89-compatible bridge status regression for config-generation failure and a Process workflow source contract that verifies every textual callsite is captured and recorded. Added the regression source to Process and XY-C1 path filters.
- Local host command passed on the extracted policy/test: `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer .../bridge_init_policy.c`, then `ASAN_OPTIONS=detect_leaks=1 bridge-init-policy` (PASS).
- Exact-head Actions for the new commit must be queried after push. The previous production-source L3 evidence is only for `61cbc6b842fdaaafc3f2fcab269bb11292fa8514` / run `44644448761`; the checker-only `d246ddf` XY run `44646517924` succeeded with Build/TRX/artifact steps SKIPPED because no firmware input changed. A new production-source build is required for this wireless source change.

## Exact first action for the next GPT

1. Check Process, AI handoff docs and XY-C1 exact-head Actions for the current `main` SHA; inspect Build XY-C1, TRX verification and artifact upload individually.
2. Continue cross-userland RC stability fixes from the remaining P1 caller-result gaps. Do not re-audit closed SCUT/CA, SD/MMC queue, NVRAM gate, or ZeroTier teardown slices without a new failing case.
3. Record exact L2/L3 evidence separately. No physical-router L4 validation is established.

## Verification boundary

- L1: PID 1 distinguishes FATAL/READY/DEGRADED and continues event processing after degraded service boot. Late CA preparation, persistent user-script and autostart launcher failures are now included in the boot aggregate.
- L2: PID 1 policy binary and `dev_init` shell regression passed on `9ad1570`; the new CA status shell regression passed on `61cbc6`. Exact-head Process, AliDDNS and Storage jobs initially failed only at three stale source-contract assertions; their corrections are in this checker follow-up. HTTPD, AI docs and all other completed current-head regression runs were PASS.
- L3: XY-C1 `44644448761` on `61cbc6` was IN_PROGRESS at last poll; no Build/TRX/artifact result is claimed until all three exact-head steps succeed.
- L4: target MIPS/uClibc runtime and physical-router boot/recovery behavior remain NOT VERIFIED.

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
