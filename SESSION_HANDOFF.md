# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main HEAD at this handoff refresh: `43fb2c87c144efe369970cd3ad6e0b43a0ba174a`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.
- Recent WAN-up ZeroTier changes removed the unchecked NVRAM pre-gate so the checked service wrapper's read/launch status reaches the WAN-up accumulator.

## Active task

- WAN teardown uses bounded checked gateway/address/netmask reads, accumulates backend errors, and gates off-subnet gateway /32 removal on the checked-read policy helper. The production change is committed at cb3a9460635a3754e3317d6340ac6b6749aef4ff.
- Latest main 43fb2c87c144efe369970cd3ad6e0b43a0ba174a also fixes LLTD/rstats restart wrappers to preserve their checked start result. Process inventory integration passed on this SHA.
- Exact-head Actions for 43fb: Process inventory #44926550385 PASS; WAN gateway ARP #44926550439 PASS; WAN route #44926550424 FAIL at a stale assertion requiring pre-refactor WAN-down pointer variables; infosvr #44926550285 and crond #44926550347 FAIL because their checkers still expected raw nvram_invmatch gates; Process integration #44926550298 FAIL because it expected the postcondition helper to be returned directly rather than tested as a predicate. XY-C1 #44926550475 was still in progress when checked.
- Current slice aligns those four source contracts with the checked-reader/postcondition implementation and wires the WAN teardown policy regression into the WAN route workflow, including push/PR filters and an ASan/UBSan build.
- Next production check: WAN teardown must not classify an invalid or empty IPv4 address/netmask as off-subnet. Keep gateway /32 cleanup fail-closed until both are valid IPv4 strings; preserve default-route deletion and independent conntrack cleanup.

## Verification boundary

- L1: checked NVRAM policy helpers and WAN teardown host-route selection have GNU89 regression coverage; the WAN teardown regression is now compiled with ASan/UBSan in the WAN route workflow.
- L2: exact-head Process inventory and gateway-ARP suites passed on 43fb2c87c144efe369970cd3ad6e0b43a0ba174a. WAN route, infosvr, crond and Process integration failures were stale source-checker expectations; corrected checks and WAN teardown test wiring await validation on the next exact SHA.
- L3: NOT VERIFIED until XY-C1 firmware Build, TRX verification and artifact upload all PASS on one exact product-source SHA. XY-C1 #44926550475 was in progress at the last check.
- L4: WAN route ownership, daemon status propagation and resolver continuity remain NOT VERIFIED on hardware.

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

