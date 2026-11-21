# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and necessary tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Never weaken regression contracts.
- Exact-head Actions validate only the matching SHA. Do not wait on PENDING; continue independent work. Fix a real FAIL at its cause.
- Keep root `dev_log.md` newest-first with 20–119 entries. Preserve `docs/dev-log/2026/11/part-021.md` and its index. No no-op commits or handoff snapshot archives.

## Mandatory handoff checkpoint

Inspect live repository state before ending. Update this handoff only if the source baseline, active task, exact-head validation, blocker, or first next action has materially changed. If the existing checkpoint is still correct, explicitly conclude it without a file edit or no-op commit.

## Current repository state
- Parent before WAN protocol read hardening: `4a02532fb9082af0872240cd7b70330426332b2a`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and is immutable.
- Firewall global-enable checked-read commit `ee01141a`: exact Firewall #44900939770 PASS; AI docs #44900939767 PASS; Process lifecycle inventory #44900939740 PASS. Its follow-up MAC/DoS fail-closed commit is `4a02532fb9082af0872240cd7b70330426332b2a`: Firewall #44901320517 PASS, AI docs #44901320562 PASS, Process inventory #44901320491 PASS; WAN DHCP #44901320527, WAN route #44901320473, HTTPD #44901320508 and FULLCONENAT #44901320450 also PASS.
- On `4a02532fb9082af0872240cd7b70330426332b2a`, XY-C1 #44901320499 remains IN_PROGRESS in dependency setup; Build XY-C1, TRX verification and artifact upload have not completed. NVRAM transaction boundary #44901320454 also remains IN_PROGRESS.
- Firewall now checks `fw_enable_x`, `fw_dos_x`, `macfilter_enable_x` and `fw_mac_drop` before generating policy; absent-key behavior remains disabled/legacy, backend/invalid/malformed values abort staging and the existing core restore transaction rolls back.
- This new slice fixes a separate WAN config publication issue: `reset_wan_vars_status()` previously used `nvram_safe_get("wan_proto")` then wrote `wan0_proto`, collapsing backend errors into empty protocol values. It now uses the existing checked transactional `set_wan_unit_param_status(unit, "proto")`.

## Active task
- Verify this commit's exact-head WAN route regression, NVRAM boundary audit and XY-C1 firmware build.
- Confirm the WAN source contract detects the transactional read → result gate → per-unit write ordering and preserves missing-key empty semantics.
- Then continue with a separate confirmed P1 defect in the wider userland; do not resume repetitive firewall review unless an actual test/bug requires it.

## Verification boundary
- Latest firewall/MAC/DoS host regression: GNU89 `-Wall -Wextra -Werror`, ASan/UBSan PASS locally.
- Exact-head Firewall #44901320517 PASS, AI docs #44901320562 PASS, Process inventory #44901320491 PASS on `4a02532`.
- XY-C1 #44901320499 IN_PROGRESS on `4a02532`; Build, TRX and artifact pending.
- NVRAM audit #44901320454 IN_PROGRESS on `4a02532`.
- WAN checked protocol source regression and same-source XY-C1 pending for the new commit.
- No complete MIPS build or hardware/router L4 claim.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head WAN-route, NVRAM transaction audit and XY-C1 Actions for the current WAN protocol read fix.
2. Confirm the checked transactional `wan_proto` → `wan0_proto` contract passes, and inspect Build XY-C1, TRX verification and artifact upload as separate steps.
3. Diagnose any failing exact-SHA step without reverting to `nvram_safe_get` or weakening read-status handling.
4. Continue one independent P1 stability/security fix outside the WAN/firewall slice.

Before context exhaustion, update this handoff only when the active task, exact-head validation, or first next action materially changes.
