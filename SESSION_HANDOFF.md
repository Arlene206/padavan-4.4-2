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
- Parent main before this firewall NVRAM slice: `6dc8c0bb9759e63624b65a8285a86a9a408cc9cf`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- CPU watchdog checked NVRAM source was committed on `fee2d5d2`; the watchdog workflow PR path contract was repaired on `6dc8c0bb`. Exact watchdog run #44900574845 and AI handoff docs #44900574762 passed on `6dc8c0bb`.
- Same-source XY-C1 run #44900457461 for CPU watchdog production source `fee2d5d2` remained IN_PROGRESS at last poll (Build/TRX/artifact not yet verified).
- New P1 firewall defect fixed in this slice: five `firewall_ex.c` builders previously collapsed `nvram_match("fw_enable_x", "1")` backend errors to false, which can render IPv4/IPv6 filter base policies ACCEPT. A checked reader now preserves absence as the historical disabled default, but treats backend/invalid status as an error and aborts the existing core restore transaction before ruleset publication.
- Frozen backup remains read-only; legacy PR #1 remains closed/unmerged.

## Active task
- Verify exact-head Firewall Startup Stability regression and XY-C1 Build/TRX/artifact for this firewall source change.
- Preserve the restore transaction rollback path on every checked-read failure; do not map UNKNOWN to disabled or change the legacy missing-key default.
- If exact-head tests fail, inspect the first causal step and fix source/contracts without weakening coverage. Pending XY-C1 does not block independent host-level work.

## Verification boundary
- Standalone firewall NVRAM policy test compiled locally with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan: PASS.
- Full `firewall_ex.c` RC compile, MIPS/uClibc firmware build, TRX verification and artifact upload: pending exact-head Actions; no L3 claim.
- Exact watchdog and AI-doc checks on `6dc8c0bb` passed; watchdog-source XY-C1 #44900457461 was still IN_PROGRESS at the last poll.
- Hardware/router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head Firewall Startup Stability and XY-C1 Actions.
2. Confirm the checked-reader policy binary, five source gates, workflow contracts and Build/TRX/artifact status.
3. Diagnose the first failing step without weakening transaction rollback or the UNKNOWN-vs-absent distinction.
4. Continue a separate P1 stability/security issue across the wider Padavan userland.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
