# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main HEAD at this handoff refresh: `285f00cc0b4918670afbeed98e6848de2c9b93d0`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.
- Recent WAN-up ZeroTier changes removed the unchecked NVRAM pre-gate so the checked service wrapper's read/launch status reaches the WAN-up accumulator.

## Active task

- WAN-down uses bounded checked NVRAM copies for gateway, address and netmask. This repair adds semantic validation: dhcp_lease_ipv4_is_valid() for the previous host address and dhcp_lease_netmask_is_valid() for a contiguous nonzero mask before subnet classification or gateway /32 deletion.
- Exact-head 387591: infosvr #44927382658 PASS; crond #44927382660 PASS; AI handoff #44927382685 PASS; Process inventory #44927382674 failed on a stale restart-function signature lookup; WAN route #44927382675 failed on an incorrectly escaped newline assertion. Both checker defects were corrected in 285f00cc0b4918670afbeed98e6848de2c9b93d0.
- On 285f, AI handoff #44927593523 PASS; WAN route #44927593606 and Process inventory #44927593544 were IN_PROGRESS when last checked.
- Invalid address/netmask values now fail closed for gateway /32 ownership and report EINVAL, while independent default/static-route/conntrack cleanup remains reachable.
- Next: inspect exact-head WAN route, Process inventory and XY-C1 Build/TRX/artifact for this commit. Then determine whether gateway/address/netmask need one transaction snapshot to avoid mixing values across concurrent writers.

## Verification boundary

- L1: checked NVRAM and teardown policy have GNU89 regressions. WAN route workflow compiles/runs wan_teardown_policy.c under ASan/UBSan and statically checks validators precede subnet classification.
- L2: infosvr, crond and AI handoff passed on 387591. On 285f, AI handoff passed; WAN route and Process inventory were pending when this semantic-validation slice began. New policy tests and source assertions await exact-head Actions.
- L3: NOT VERIFIED until XY-C1 firmware Build, TRX verification and artifact upload all PASS on one exact product-source SHA.
- L4: host-route ownership and concurrent WAN reconfiguration behavior remain NOT VERIFIED on hardware.

## Workflow and preservation rules

- Main only. One logical slice, one atomic main commit: create all blobs, one tree based on current main, one commit with current main as parent, then non-forced update_ref using the exact expected_sha. No force push, amend, history rewrite, branches, PR-based development or partial per-file commits.
- Re-fetch main immediately before writing. On a moved ref, discard the stale prepared tree and rebuild from the new parent.
- Inspect push Actions only when head_sha exactly matches the commit. Diagnose failures from their logs. PENDING does not block independent work.
- Preserve security, architecture, ABI, GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Never weaken useful tests or focused workflow path coverage.
- Do not touch GPT Session Rotator workflow/state or replay SHA 9d5a5cb. The backup branch is permanent immutable recovery data.

## Cross-conversation documentation governance

- GitHub is the durable cross-conversation record; the one-line user entrypoint remains sufficient.
- dev_log.md stays newest-first with 20–119 complete records. Before record 120, archive the oldest 100 complete records verbatim into docs/dev-log/YYYY/MM/part-NNN.md in one batch, leaving 20 newest entries.
- Rotate early only above the 384 KiB target and never exceed 512 KiB. Preserve archive/index bytes. Git history is the version archive for handoff and roadmap; routine snapshots and SHA-chasing commits are prohibited.
- Update dev_log.md for real milestones, this handoff when first-next-action facts materially change, and REFACTOR_ROADMAP.md only when repository-wide module status/priorities materially change.
- Before context exhaustion, finish the safe slice, commit atomically, inspect exact-head Actions and leave the next precise task.

## Exact first action for the next GPT

Check exact-head WAN route, infosvr, crond, Process integration, Process inventory and XY-C1 Build/TRX/artifact Actions for this regression-alignment commit. Repair any remaining failures from exact logs. Then harden WAN teardown host-route classification against missing/invalid address or netmask values while retaining independent cleanup, and continue the WAN control-plane audit.

