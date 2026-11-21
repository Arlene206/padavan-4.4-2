# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main SHA before current service-enable startup repair: `38bc699b31a449f9708daf57e114de02560f17cc`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.
- Recent WAN-up ZeroTier changes removed the unchecked NVRAM pre-gate so the checked service wrapper's read/launch status reaches the WAN-up accumulator.

## Active task

- Completed checked configuration fixes: PPPoE-MWAN WAN-up/down; rstats/LLTD service enable; DHCP gateway-ARP enable; PPP DNS snapshot preservation.
- Current WAN teardown task: `wan_down_internal()` now snapshots gateway/address/netmask with bounded checked NVRAM reads, accumulates/logs read errors, and only removes off-subnet gateway /32 route with a complete checked tuple. The checked previous WAN address is reused for conntrack flush and post-WAN hook fallback.
- GNU89 regression in `tests/wan_teardown_policy.c` exercises successful off-subnet host route, on-subnet/PPP/invalid gateway cases, and gateway/address/netmask read failures. `scripts/check_wan_dns_read_status.py` asserts the production call path.
- Exact-head main on checker-fix SHA `432005050395db4136cd80407f37000c5dede114`: WAN route #44925798017 PASS; gateway-ARP socket #44925798034 PASS; AI handoff #44925798009 PASS. Process integration #44925798100 failed only at stale LLTD enable-state assertions; its current implementation uses checked `service_nvram_match_status` and requires a matching contract update. XY-C1 on e100 SHA failed at the missing static-inline modem policy include; `net_wan.c` now includes `modem_wan_start_policy.h` and a fresh build is pending.
- First next task: align LLTD/rstats startup contracts with checked enable reads, repair exact-head failures, then continue WAN control-plane state audit.

## Verification boundary

- L1: checked-NVRAM policy and service-start helpers have host regression coverage; the current boot integration contract verifies all four optional-service entry points retain their checked-read boundary and feed status aggregation.
- L2: exact-head WAN route and gateway-ARP socket suites passed on 432005050395db4136cd80407f37000c5dede114; Process integration #44925798100 failed at the stale LLTD expectation. Exact-head Process and WAN-route results for this boot-gate repair are pending.
- L3: NOT VERIFIED until XY-C1 firmware Build, TRX verification and artifact upload all PASS on one exact product-source SHA. The services.c change should trigger the XY-C1 firmware validation workflow; the outstanding modem policy header fix also awaits same-SHA verification.
- L4: boot-time NVRAM error propagation, WAN route cleanup and resolver continuity remain NOT VERIFIED on hardware.

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

Fetch live main and confirm backup `backup/pre-plugin-overhaul-2026-11-14` is unchanged. Check exact-head WAN route, teardown, Process integration, USB modem and XY-C1 Build/TRX/artifact Actions for the checked gateway/address/netmask change; inspect failures and repair. Update LLTD/rstats integration assertions to expect `service_nvram_match_status()` instead of unchecked `nvram_invmatch()`. Then continue one independent P1 WAN/service reliability issue.

