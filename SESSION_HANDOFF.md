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

- Current main before this NAT/filter transaction slice: f5807d72e7b0e240660650432a9af846f61e94c7; frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Port-mapping enumeration heap-overflow fix is committed as c5e41697bfeb593223a8c7c46b9e1e972fdc7be9. Exact-head MiniUPnP regression #44791232803 PASS, including the 4096-entry capacity-growth sanitizer test and new source contract.
- Exact-head XY-C1 run #44791232838 on c5e4169 was IN_PROGRESS at last poll; Build XY-C1 had not started and TRX/artifact remained pending. Do not claim L3.
- Confirmed a cross-table partial-transaction defect in update_portmapping(): NAT is committed with the new internal port before filter-table open/chain lookup/rule allocation/commit. Any later filter failure returned immediately, leaving the DNAT and forwarding ACCEPT rules with different internal ports.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Make update_portmapping() fail transactionally across NAT and filter tables:

- Retain the changed NAT entry and its original internal port until the corresponding filter update commits.
- Use a separate filter-entry buffer. If filter open, chain lookup, matching rule, buffer allocation or filter commit fails after NAT commit, restore the NAT entry to its original port at the saved NAT index.
- Preserve the primary failure status/errno if compensating rollback also fails, and emit a clear rollback diagnostic.
- Add GNU89 ASan/UBSan policy regression tests and a source contract requiring all post-NAT filter failure paths to reach the rollback.

After exact-head MiniUPnP and XY-C1 validation, continue the netfilter rule update/delete transaction audit. Do not repeat the completed capacity/allocation fixes.

## Verification boundary

- Port-mapping enumeration capacity fix c5e4169: exact-head MiniUPnP #44791232803 PASS; the C regression stored and verified 4096 entries after multiple capacity doublings under ASan/UBSan, and its source contract passed.
- XY-C1 #44791232838 on c5e4169 was IN_PROGRESS; Build, TRX and artifact had not yet completed.
- This commit adds a rollback policy regression/source contract; exact-head MiniUPnP Action must pass. XY-C1 triggers on iptcrdr.c and new policy/test paths; exact-source firmware Build/TRX/artifact remain mandatory for L3.
- Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main, verify frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807, and check exact-head MiniUPnP rollback policy/source-contract Actions.
2. Inspect XY-C1 Build, TRX verification and artifact upload for the exact source SHA; diagnose the first causal failure from its logs.
3. Continue auditing MiniUPnP NAT/filter rule updates and deletes for partial commits and stale state. Physical-router L4 remains unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
