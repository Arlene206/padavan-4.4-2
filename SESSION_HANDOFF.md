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

- Parent `main` for the redirect-metadata source slice: `de521606d8b1ccfabbb67cb0e2e006ec8bbc5939`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and read-only.
- IPTC ownership-order correction `46d79504f70236b403ac3153ca155715d9c65bb0`: exact-head MiniUPnP #44794517972 PASS; AI handoff #44794518071 PASS; process inventory #44794518177 PASS.
- NAT/filter delete compensation `b7a803dcc95ae88566ffe01ff8fd8ea4c5cefd33`: exact-head MiniUPnP #44793663690 PASS.
- NAT/filter update rollback `bfc1c1663f9d3dc6cdb4349ba1b3601f175e27ad` plus checker correction `ea745da6e423a778a65c0aba6ce85424e6a642a3`: MiniUPnP #44792212170 PASS; AI #44792212195 PASS; exact-source XY-C1 #44791960769 on bfc1c16 PASS (Build, TRX verification, artifact upload).
- Peer SNAT/DSCP helper ordering correction `46d79504` is covered by MiniUPnP #44794517972 PASS. The metadata OOM source slice `de521606d8b1ccfabbb67cb0e2e006ec8bbc5939` has exact-head MiniUPnP #44795187554 PASS and Process lifecycle inventory #44795187466/#44795187342 PASS.
- AI handoff run #44795187315 failed because the new handoff accidentally dropped the required cross-session mandate and end-of-session checkpoint preamble. This document-only correction restores the original mandatory preamble and preserves the updated task state.
- XY-C1 #44795187275 and NVRAM #44795187414 were in progress at last poll. Firmware L3 for the metadata SHA is not yet verified.
- MIPS/uClibc runtime and physical-router L4 remain NOT VERIFIED.

## Active task

Complete the documentation-contract repair following the redirect-metadata OOM fix:

- Preserve this file's mandatory `# Cross-session execution mandate` and `## Mandatory handoff checkpoint` preamble on every edit.
- The checkpoint is mandatory; a document edit is conditional. Restore the required text from the prior committed handoff, then keep the current exact source SHA, CI evidence and first next action.
- Do not weaken the validator or create a no-op document commit; exact-head AI handoff Action must verify the restored text.

After the AI handoff check passes, continue auditing MiniUPnP teardown and other userland stability/security issues. Do not repeat the descriptor OOM fix or IPTC/NAT-filter work already covered by passing host tests.

## Verification boundary

- Metadata OOM source fix `de521606d8b1ccfabbb67cb0e2e006ec8bbc5939`: MiniUPnP exact-head #44795187554 PASS, including the GNU89 ASan/UBSan allocator binary and new source-order contract. Process inventory #44795187466 and lifecycle regression #44795187342 PASS.
- AI handoff docs #44795187315 FAILED at the mandatory end-of-session checkpoint requirement because the handoff preamble was omitted; this commit restores the exact required mandate/checkpoint section from parent history. Rerun required.
- XY-C1 #44795187275 and NVRAM #44795187414 were PENDING at last poll. Do not claim Build/TRX/artifact L3 until XY exact-source success.
- Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`, confirm frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, and inspect the exact-head AI handoff rerun for the restored mandatory preamble.
2. Poll XY-C1 #44795187275 and NVRAM #44795187414 for their final exact-SHA conclusions. Require Build, TRX verification and artifact upload before claiming L3.
3. Continue MiniUPnP teardown and whole-userland stability/security work. Keep MIPS/uClibc runtime and physical-router L4 explicitly unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
