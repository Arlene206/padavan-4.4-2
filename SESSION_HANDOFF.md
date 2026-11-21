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
- Parent main for this watchdog workflow-filter repair: `fee2d5d2d016853fc6d35c4f4247c3fc451f2323`; current product source is in `fee2d5d2`.
- The checked `watchdog_cpu` NVRAM reader now protects both `start_watchdog_cpu()` and `restart_watchdog_cpu()` from backend, malformed and overflow failures; absent key remains disabled default. Module load/unload is not attempted when config is unreadable.
- Exact-head watchdog run #44900457471: GNU89 signal regression PASS; NTP/service-lifecycle regression PASS; CPU watchdog module restart policy compile + GNU89/Werror/ASan/UBSan regression PASS; NTP/service integration PASS. Embedded contract failed only because `wan_nvram_status_policy.h` was missing from the pull_request path filter. This docs/workflow-only commit adds the missing PR path; the push filter and contract inventory already contain it.
- Exact-head XY-C1 #44900457461 is IN_PROGRESS (Build job active; verify Build XY-C1, TRX and artifact before L3 claim). Process lifecycle inventory #44900457378, Process fail-closed umbrella #44900457513, AI docs #44900457357, and several independent runs are at mixed PASS/PENDING states.
- Samba parent host regressions #44900045476, #44900045461, #44900045444, #44900045350, #44900045396 and #44900045349 passed; Samba source exact XY-C1 #44900045362 remained pending at last poll.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; legacy PR #1 closed/unmerged.

## Active task
- Verify this workflow-filter repair's exact-head watchdog Action; it should now pass the previously missing PR filter assertion.
- Keep the watchdog source read/parse failure fail-closed; do not relax C tests or source contracts.
- Inspect XY-C1 #44900457461 for the CPU watchdog production-source SHA `fee2d5d2`; L3 only after Build, TRX verification and artifact all pass.
- Continue independent reliability/security work while other Action runs remain pending.

## Verification boundary
- CPU watchdog C policy regression on `fee2d5d2`: PASS on exact-head host Action before the integration checker failure.
- Watchdog integration contract: failed only because the workflow PR filter omitted the new NVRAM header; corrected here, exact-head rerun pending.
- Same-source XY-C1 #44900457461: IN_PROGRESS; no L3 claim yet.
- Hardware/router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect the new exact-head watchdog Action; confirm all tests and filter contract PASS.
2. Inspect #44900457461 for Build XY-C1, TRX verification and artifact upload.
3. Continue with an independent P1 reliability/security defect if CI is pending; retain fail-closed watchdog config semantics.
4. Update the handoff when exact-head statuses or the next task materially change.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
