# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main HEAD at this handoff refresh: `387591fe0a0bdc20507f8853e70d033ffa3661c4`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.
- Recent WAN-up ZeroTier changes removed the unchecked NVRAM pre-gate so the checked service wrapper's read/launch status reaches the WAN-up accumulator.

## Active task

- WAN teardown uses bounded checked gateway/address/netmask reads and gates off-subnet gateway /32 cleanup on a complete checked tuple. Production fix: cb3a9460635a3754e3317d6340ac6b6749aef4ff.
- Latest main 387591fe0a0bdc20507f8853e70d033ffa3661c4 aligns infosvr/crond enable checks and LLTD/rstats postcondition assertions and adds ASan/UBSan coverage for the WAN teardown policy.
- Exact-head Actions for 387591: infosvr #44927382658 PASS; crond #44927382660 PASS; AI handoff #44927382685 PASS; Process inventory #44927382674 FAIL at a checker signature mismatch for restart_lltd(void); WAN route #44927382675 FAIL because the workflow's Python string used a literal escaped backslash-n instead of the C function-definition newline. XY-C1 #44927382664 was still in progress at last check.
- Current narrow repair corrects both checker mistakes; no production code changes in this slice.
- Next production item: require a valid previous WAN IPv4 address and a valid contiguous netmask before WAN-down classifies the gateway as off-subnet. Continue default-route deletion and independent cleanup, and report invalid configuration states.

## Verification boundary

- L1: checked NVRAM route snapshot policy has GNU89 regression tests; WAN route workflow now compiles/runs wan_teardown_policy.c under ASan/UBSan and has exact push/PR trigger paths.
- L2: infosvr, crond and AI handoff suites passed on 387591fe0a0bdc20507f8853e70d033ffa3661c4. Process inventory and WAN route failed only at checker-source mismatches described above; corrected checks await exact-head validation.
- L3: NOT VERIFIED until XY-C1 firmware Build, TRX verification and artifact upload all PASS on one exact product-source SHA. XY-C1 #44927382664 was still in progress.
- L4: WAN teardown and daemon error propagation remain NOT VERIFIED on hardware.

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

