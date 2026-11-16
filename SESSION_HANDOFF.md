# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Parent main: `f0294dee852b0d9bcdd68693436f3fe83633b612`. It includes generic firewall/UPnP result propagation through storage, IPv6, VPN server and the corresponding policy/source checks.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- Parent exact-head Process `43935227981` PASS and AI docs `43935227749` PASS. Storage hotplug `43935227891` FAIL at `scripts/check_storage_safe_remove.py:302`: its count of `rc_first_service_start_status()` in `start_stor_apps()` still expected 7 after firewall status propagation added an eighth aggregation. The storage workflow's earlier full caller-graph contract passed. NVRAM audit `43935227925` was IN_PROGRESS at last poll.
- Parent `cb0e8e5` Firewall Startup `43934499510` and VPN server `43934499223` PASS; XY-C1 `43934498984` was IN_PROGRESS on that production-source SHA. Parent `f6d4192` AI docs `43935073720` PASS; its Process run was later repaired and passed at `43935227981`.
- PR #1 remains historical closed/unmerged work; do not resume it.

## Active task

The storage implementation now propagates firewall/UPnP refresh failure from `start_stor_apps()`, but the second independent source-contract checker `scripts/check_storage_safe_remove.py` still expected seven first-error merges and did not account for the new firewall aggregation. This commit updates that checker to expect the eighth status merge and assert it occurs before the function returns. The storage workflow already had a matching eighth-merge assertion and its caller-graph validation step passed.

After this checker repair, continue the direct-call audit at `restart_vpn_client_locked()`: propagate the newly int-returning `restart_firewall()` status and preserve the first VPN failure's errno. Then resume the P1 `reload_nat_modules()` / `start_firewall_ex()` result audit.

## Verification boundary

- Parent Process `43935227981` and AI docs `43935227749` PASS. Storage hotplug `43935227891` failed only at the stale count-7 assertion in `scripts/check_storage_safe_remove.py:302`; the preceding storage caller-graph checker passed. This commit fixes the expected count and verifies the firewall aggregation/return ordering.
- NVRAM audit `43935227925` was IN_PROGRESS at last poll. XY-C1 `43934498984` is a prior production-source run on `cb0e8e5`; a later VPN source fix will require a new exact-SHA Build/TRX/artifact run.
- No full local firmware build or physical-device validation is claimed. Physical-router L4 remains NOT VERIFIED.
- Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect exact-head Storage hotplug and NVRAM Actions for THIS COMMIT; confirm `check_storage_safe_remove.py` now accepts all eight first-error merges without suppressing any earlier service failure.
2. Fix the remaining VPN client `restart_firewall()` result/errno propagation and extend the WAN semantic checker plus Firewall Startup workflow triggers.
3. Continue the P1 audit at `reload_nat_modules()` and `start_firewall_ex()`; keep backup read-only and physical-router L4 NOT VERIFIED.
