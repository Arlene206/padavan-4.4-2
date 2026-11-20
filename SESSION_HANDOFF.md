# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Current main before this warning-cleanup commit: `46d79504f70236b403ac3153ca155715d9c65bb0`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- NAT/filter redirect deletion compensation was implemented in `b7a803dcc95ae88566ffe01ff8fd8ea4c5cefd33`; exact-head MiniUPnP #44793663690 passed the delete-rollback policy test and source contract.
- Peer SNAT/DSCP compensation was added in the current parent slice: it snapshots the peer SNAT rule before deletion and restores it if the subsequent mangle/DSCP operation fails. This follow-up removes a redundant local flag that was written but never read, avoiding a potential `-Wunused-but-set-variable` build warning.
- Exact-head actions observed on current parent: #44794517934 NVRAM transaction boundary audit queued/pending; #44794518071 AI handoff documentation contract in_progress/pending; #44794517908 XY-C1 Firmware Validation in_progress/pending; #44794518177 Process lifecycle inventory - 46d79504f70236b403ac3153ca155715d9c65bb0 in_progress/pending; #44794517972 miniupnpd atomic configuration publication regression in_progress/pending.
- Prior XY-C1 #44792212585 was host-only: Build/TRX/artifact were SKIPPED after no eligible firmware inputs were detected. No firmware L3 claim.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Finish exact ownership ordering for the peer SNAT deletion helper:

- `delete_rule_and_commit()` consumes and frees the by-value `IPTC_HANDLE`.
- Set the caller's `h = NULL` immediately after the peer SNAT helper call; only then copy the scalar result into `peer_delete_status`.
- Keep the regression contract strict across every delete helper call site. Do not weaken the test to accommodate an ownership gap.

After the MiniUPnP rerun passes, continue with the independently confirmed redirect-description/finite-lease metadata OOM defect: stage descriptor allocation before NAT updates and publish only after success.

## Verification boundary

- NAT/filter delete compensation `b7a803d`: exact-head MiniUPnP #44793663690 PASS, including sanitizer policy regression and source contract.
- Peer SNAT/DSCP compensation and this unused-flag cleanup are covered by the exact-head MiniUPnP run on the current parent; confirm the run's final conclusion before declaring this slice verified.
- XY-C1's latest completed check remained host-only, with Build XY-C1, TRX verification and artifact upload skipped. Firmware L3 is NOT verified. Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and confirm the frozen backup SHA remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Check exact-head MiniUPnP peer SNAT/DSCP regression and source-contract run after the warning cleanup; inspect any failed step's log.
3. Continue the HTTPD/service lifecycle and UPnP teardown audit across userland. Keep host-only XY-C1 runs distinct from firmware L3; physical-router L4 remains unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
