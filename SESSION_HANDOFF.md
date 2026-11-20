# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- PID 1 degraded-readiness implementation is committed on main as `9ad1570aa0bc8a9e1c5ff26a85266d53043c526a`. Its GNU89 ASan/UBSan policy binary and existing `dev_init.sh` shell regression both passed in the Process job; the job then failed in the new Python source contract because its C-function parser still indexed `init.c` when extracting `rc.c`.
- Exact-head Process run `44642485690` therefore failed only at that parser; HTTPD run `44642485613` failed at stale source assertions expecting the old `return -1` and `init_router() == 0` contracts. The follow-up changes align both contracts with FATAL/READY/DEGRADED results and ensure the policy header itself triggers HTTPD/XY-C1 validation.
- AI handoff docs run `44642485702` PASS on `9ad1570`. XY-C1 firmware run `44642485538` was still building at the last checkpoint; Build/TRX/artifact status must be re-queried by exact source SHA.
- Earlier rp-L2TP product SHA `2b646cccdcb24da5d447586893a500fdcf3cfe48` passed XY-C1 Build, TRX verification and artifact upload in run `44639411601`; artifact `14584429568`. This does not replace the build of the PID 1 source change.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, archival and untouched.

## Active task

- The P1 PID 1 boot-result slice is implemented: `init_router()` reports FATAL for the four mandatory NVRAM/configuration aborts, READY for a clean boot and DEGRADED for accumulated module/service failures. PID 1 keeps notification/deferred-hotplug processing enabled for READY and DEGRADED; mandatory `dev_init.sh`/NVRAM failures and FATAL remain in console recovery.
- The GNU89/Werror/ASan/UBSan policy binary passed on `9ad1570`; the first Process and HTTPD CI attempts exposed two stale source-contract problems, not a C regression failure. This follow-up fixes the generic C-function parser in `check_dev_init_boot.py`, updates HTTPD's boot-state assertions, and adds the policy header to the HTTPD and XY-C1 workflow path filters. No production C behavior changes in this follow-up.
- Exact-head Process, HTTPD and AI docs reruns are pending; XY-C1 Build/TRX/artifact for production-source SHA `9ad1570` must be checked separately.

## Exact first action for the next GPT

1. Query exact-head Process lifecycle, HTTPD security and AI handoff docs results for the CI-contract follow-up; inspect the precise failed step if either source contract still fails.
2. Re-query XY-C1 run `44642485538` for production-source SHA `9ad1570`; report L3 only if Build, TRX verification and artifact upload all pass on that exact SHA.
3. Continue the open P1 RC startup-result propagation queue at the next unclosed `start_services_once()`/WAN service caller; do not repeat the closed void-wrapper audit or completed rp-L2TP/netmask/MTD slices.

## Verification boundary

- L1: PID 1 now distinguishes fatal prerequisite failure, fully READY and DEGRADED service startup; only READY/DEGRADED enable the router event loop.
- L2: local and CI GNU89/Werror/ASan/UBSan policy regression PASS on `9ad1570`; existing shell test PASS. Process source checker failed from its wrong C-function buffer reference, and HTTPD failed from stale pre-three-state assertions; corrected contracts await exact-head reruns.
- L3: XY-C1 run `44642485538` for `9ad1570` was IN_PROGRESS at the last check (Build step running); TRX/artifact are not yet claimed.
- L4: target MIPS/uClibc runtime and physical-router startup/recovery behavior remain NOT VERIFIED.

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
