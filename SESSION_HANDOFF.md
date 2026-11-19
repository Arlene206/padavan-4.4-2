# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Parent main before this checker slice: `b4b46dea0d4288c9f46f7a7b8f15072291f51eba` (`test(rc): align APCLI and storage completion checks`).
- Product-source SHA `3488f129babe376bccf27e132776c45910614dd7` contains PPP physical-WAN fail-closed recovery (`5878c9d`), APCLI WAN recovery propagation (`de73223`) and checked SD/MMC PID-1 event queue. Its exact XY-C1 firmware run `44625809599` PASSED: Build XY-C1, TRX output verification and artifact upload all completed successfully on this exact product-source SHA. This is L3 evidence for those source changes; it is not hardware/L4 evidence.
- Exact b4b46 checker runs: Wireless radio `44627098078` PASS; Process lifecycle `44627098093` failed on an obsolete first-occurrence IAPP aggregation-order assertion; Storage `44627098102` and HTTPD security `44627098106` failed on stale legacy `notify_rc()` unplug assertions after the source switched to `queue_storage_hotplug_notification()`; AI handoff docs `44627098108` failed because the mandatory end-of-session checkpoint statement had been removed from this file. THIS commit corrects those four contracts.
- Earlier exact `3488f12` checks: NVRAM audit `44625809632`, Process inventory `44625809765`, and AI docs `44625809759` PASS. Its Storage/HTTPD/Process/Wireless failures have since been traced to stale checker assertions and repaired at b4b46 or THIS commit.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is archival only. PR #1 remains closed/unmerged.
- Root `dev_log.md` has 31 dated entries before THIS commit, safely inside its 20–119 record window. No physical-router test has been performed.

## Active task

### Close exact-head CI contracts for the result-propagation changes

- PPP, APCLI and SD/MMC production changes are already committed. The exact XY-C1 run `44625809599` for product SHA `3488f129...` passed Build, TRX verification and artifact upload.
- This checker-only slice aligns storage and HTTPD unplug assertions with the new checked `queue_storage_hotplug_notification()` call, corrects Process integration ordering to inspect the IAPP status merge after its own failure check (the earlier radio merge is for WAN recovery), and restores the mandatory end-of-session checkpoint wording required by `check_ai_handoff_docs.py`.
- No production-source modifications are included in this slice. Exact-head Actions for this commit must be inspected before claiming L2 validation.

## Exact first action for the next GPT

1. Inspect exact-head Actions for THIS commit: Process lifecycle, Storage hotplug, HTTPD security and AI handoff docs. On any FAIL, inspect the job log and correct the specific source contract. Wireless radio was already PASS on parent b4b46; verify no new regression is introduced.
2. Preserve L3 evidence for product SHA `3488f129babe376bccf27e132776c45910614dd7`: XY-C1 Build, TRX verification and artifact upload `44625809599` all PASSED. Checker-only commits after that source SHA do not require a new firmware build unless firmware inputs change.
3. Continue an independently evidenced P1 stability/security issue across Padavan userland. Do not reopen PPP, APCLI, SD/MMC notification, or closed SCUT/OpenVPN/USB modem notifier paths without new evidence; keep L1/L2/L3/L4 claims separate.

## Verification boundary

- L1: PPP physical-WAN fail-closed recovery, APCLI WAN error propagation and SD/MMC checked PID-1 notification queue are implemented on product SHA `3488f129...`.
- L2: exact product SHA `3488f12` XY host checks, Storage/HTTPD/Process inventory and AI handoff checks were partially validated; follow-up checker contract fixes are in THIS commit. Exact-head L2 outcomes remain pending.
- L3: XY-C1 Build `44625809599`, TRX verification and artifact upload all PASSED on exact product-source SHA `3488f129babe376bccf27e132776c45910614dd7`.
- L4: physical WAN recovery, APCLI/WISP reconnection and SD/MMC/PID-1 hotplug behavior on a router remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
