# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Parent main SHA at this handoff checkpoint: `9bdc5821ada33a0983a3c4c0c5fda1999e1df68f`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.
- Recent WAN-up ZeroTier changes removed the unchecked NVRAM pre-gate so the checked service wrapper's read/launch status reaches the WAN-up accumulator.

## Active task

- Production source slices: preserve indexed demand-PPP DNS on WAN teardown when global MAN DNS cannot be read; use checked exact-string PPPoE-MWAN enable reads in both WAN-up and WAN-down; check rstats/LLTD enable state before daemon startup.
- Current task: repair three stale regression assertions. The WAN route checker must accept the checked PPPoE-MWAN read/error/launch sequence after the WAN post-down hook; the PPPoE-MWAN checker must test that same checked read in both event handlers; the Process checker must normalize whitespace across multi-line C rollback calls.
- Earlier route/ZeroTier/firewall contracts now pass on the 855d ancestry; WAN route #44925135400 failed later at check_wan_detect_wan_status.py. XY-C1 #44925135581 passed the PPPoE shell tests and failed at the old PPPoE-MWAN caller assertion. Process #44925135670 failed only at the whitespace-sensitive rollback assertion. Exact-head reruns pending.
- After these contracts pass, continue the WAN event NVRAM audit with the DHCP gateway-ARP enable decision.

## Verification boundary

- L1: the code path now retains indexed PPP DNS on checked global-DNS read errors; the commit-specific GNU89 host test and caller contract are part of the commit.
- L2: exact-head WAN-route and WAN-teardown checks are pending until queried after push.
- L3: NOT VERIFIED for this source commit until XY-C1 Build, TRX verification and artifact upload all PASS on one exact product-source SHA.
- L4: physical PPP demand-DNS retention, resolver continuity and router teardown behavior remain NOT VERIFIED on hardware.

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

Fetch live main and verify the frozen backup remains unchanged. Check exact-head WAN route, XY-C1 Build/TRX/artifact, and Process Actions for the checker repair; inspect any failure logs and fix only the true failing contract/implementation. Then implement the DHCP gateway-ARP enable check using the checked exact-string NVRAM reader and add regression coverage.

