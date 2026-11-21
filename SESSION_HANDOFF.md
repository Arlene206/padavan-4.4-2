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
- Parent main before this firewall access-control slice: `ee01141a9761cd702ef71b6f8dbae2ad48604f6a`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- Firewall global-enable NVRAM fail-closed fix is committed on `ee01141a`; dedicated Firewall Startup Stability run #44900939770 PASS. AI handoff docs #44900939767 and Process lifecycle inventory #44900939740 also PASS on that SHA.
- Same-source XY-C1 #44900939788 for `ee01141a` was IN_PROGRESS at last poll (cross-build job; Build/TRX/artifact not yet verified). NVRAM audit #44900939734 was IN_PROGRESS.
- This slice closes three related firewall gaps: `macfilter_enable_x` errors previously collapsed to zero, `fw_mac_drop` errors to false, and `fw_dos_x` errors to false. Those could omit configured MAC filtering or DoS rules during a core firewall rebuild. Checked integer/boolean reads now preserve absent-key defaults, abort staging on backend/parse errors and use the existing core restore rollback.
- Frozen backup remains read-only; legacy PR #1 remains closed/unmerged.

## Active task
- Verify exact-head Firewall Startup Stability, Process/AI and XY-C1 Actions for this MAC/DoS checked-read slice.
- Preserve legacy absent-key behavior (MAC mode 0, drop false, DoS false) and exact integer mode semantics; errors must not produce partial or less restrictive rule tables.
- If a run fails, inspect its first failing step and repair source/contract without weakening checks. Pending XY-C1 does not block other independent userland work.

## Verification boundary
- Standalone firewall NVRAM policy fixture extended with integer parsing; GNU89, `-Wall -Wextra -Werror`, ASan and UBSan: PASS locally.
- Full `firewall_ex.c` RC cross-build, MIPS/uClibc firmware build, TRX/artifact and router behavior are pending exact-head Actions; no L3 claim.
- Prior firewall global-enable slice `ee01141a`: dedicated Firewall run #44900939770 PASS; same-SHA XY-C1 #44900939788 still IN_PROGRESS at last poll.
- Hardware/router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect this slice's exact-head Firewall Startup Stability regression, Process/AI checks and XY-C1 workflow.
2. Confirm the integer/boolean policy tests and source call-order assertions pass; inspect Build XY-C1, TRX verification and artifact upload separately.
3. Fix any causal failure while preserving checked-read rollback and legacy missing-key defaults.
4. Continue a distinct P1 stability/security issue outside the firewall subsystem, covering the broader Padavan userland.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
