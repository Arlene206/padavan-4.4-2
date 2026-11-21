# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then use a non-forced expected-SHA compare-and-swap update. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and required tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Do not weaken tests.
- Exact-head push Actions only validate the matching main SHA. PENDING is not a gate on independent work; inspect a relevant FAIL, fix its cause, and move forward.
- Before context exhaustion, finish the safe slice, publish the commit, inspect exact-SHA Actions and leave a truthful next action.
- Keep root `dev_log.md` at 20–119 newest-first entries; preserve `docs/dev-log/2026/11/part-021.md` and its index unchanged. Do not create handoff/roadmap snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending, verify this handoff's source baseline, active task, exact-head Actions, validation boundary, blockers and first next action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no no-op commit. Update this handoff only when next-session-critical facts materially change.

## Current repository state
- Live main parent for this watchdog config-read slice: `47335ed2ce9e949aded6c361e7164ac51851ff95`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- Samba config-mode fix on parent `47335ed2ce9e949aded6c361e7164ac51851ff95`: Samba host regression #44900045476 PASS; Process inventory #44900045461 PASS; USB modem identity #44900045444 PASS; vsftpd #44900045350 PASS; NFS #44900045396 PASS; AI docs #44900045349 PASS. XY-C1 #44900045362 pending (Build/TRX/artifact not yet verified).
- New watchdog defect fixed in current commit: unchecked `nvram_get_int("watchdog_cpu")` converted backend errors to zero, skipping CPU watchdog boot startup and potentially unloading a live module on restart. Checked NVRAM + strict integer parse now distinguishes absent-key default zero from read/parse failures. Both start and restart return the read error without changing module state.
- Existing watchdog GNU89/Werror/ASan/UBSan fixture now covers missing default, enabled values, backend/malformed/overflow error-status preservation and invalid output. Workflow source contracts and XY-C1 input test cover the checked reader/header.
- Radio source SHA `60ad278e7056ace912066b090fdf20d0fd874f3e` previously L3 VERIFIED; USB source `c58272b9` and OpenVPN source `c9eff6c6` host checks passed but exact-source XY-C1 remained pending at their last recorded poll.
- Frozen backup immutable; legacy PR #1 closed/unmerged.

## Active task
- Verify exact-head watchdog-signal, Process inventory/lifecycle, AI handoff and XY-C1 Actions for this commit.
- Inspect parent XY-C1 #44900045362 separately; it validates the preceding Samba production input.
- Keep watchdog configuration UNKNOWN fail-closed: do not translate backend or parse errors into disabled or perform a load/unload operation.
- Continue an independent P1 reliability/security issue once this host regression passes; do not wait on pending firmware builds.

## Verification boundary
- Watchdog checked-config reader and test committed; exact-head host Actions pending. No local run claimed.
- Samba parent `47335ed2ce9e949aded6c361e7164ac51851ff95`: dedicated Samba + Process inventory + USB + vsftpd + NFS + AI host Actions PASS; XY-C1 #44900045362 pending.
- Radio source `60ad278e`: L3 VERIFIED; USB/OpenVPN exact-source XY-C1 L3 pending at last documented poll.
- Hardware/router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head watchdog-signal, Process inventory/lifecycle, AI handoff and XY-C1 runs.
2. Triage the first causal failing step; retain strict checked NVRAM and the no-module-side-effect-on-error rule.
3. Inspect parent XY-C1 #44900045362 as Samba source validation only, and the new run for watchdog source validation.
4. Continue with another distinct P1 reliability/security issue outside the just-closed watchdog config slice.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
