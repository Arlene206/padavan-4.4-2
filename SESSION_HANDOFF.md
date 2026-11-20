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

- Current main before this checker-only correction: bfc1c1663f9d3dc6cdb4349ba1b3601f175e27ad; frozen backup backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Port-mapping enumeration capacity fix c5e4169 passed exact-head MiniUPnP #44791232803, including the 4096-entry ASan/UBSan test and source contract.
- NAT/filter rollback source slice is bfc1c1663f9d3dc6cdb4349ba1b3601f175e27ad. Exact-head MiniUPnP #44791960712 compiled and passed all policy binaries, allocation/capacity sanitizer tests and source contracts, but the new rollback checker failed because the rollback call is split across lines after the opening parenthesis.
- XY-C1 run #44791960769 on bfc1c16 was IN_PROGRESS; Build XY-C1 had not started at last poll. TRX and artifact were pending.
- The checker correction will accept normalized C whitespace without loosening the rollback index, original-port restore or failure-status assertions.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Make update_portmapping() fail transactionally across NAT and filter tables:

- Retain the changed NAT entry and its original internal port until the corresponding filter update commits.
- Use a separate filter-entry buffer. If filter open, chain lookup, matching rule, buffer allocation or filter commit fails after NAT commit, restore the NAT entry to its original port at the saved NAT index.
- Preserve the primary failure status/errno if compensating rollback also fails, and emit a clear rollback diagnostic.
- Add GNU89 ASan/UBSan policy regression tests and a source contract requiring all post-NAT filter failure paths to reach the rollback.

After exact-head MiniUPnP and XY-C1 validation, continue the netfilter rule update/delete transaction audit. Do not repeat the completed capacity/allocation fixes.

## Verification boundary

- Port-mapping capacity c5e4169: exact-head MiniUPnP #44791232803 PASS, including ASan/UBSan and source contract.
- NAT/filter rollback C policy binary on bfc1c16: PASS. All existing MiniUPnP regressions, allocation guards and capacity tests also PASS.
- New rollback source contract on #44791960712: FAILED only on a whitespace-sensitive lookup of the wrapped rollback commit call; checker-only correction included in this commit. Exact-head MiniUPnP rerun required.
- XY-C1 #44791960769 on bfc1c16 was IN_PROGRESS before Build; TRX/artifact not yet verified. Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main; verify backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
2. Check exact-head MiniUPnP rerun after rollback checker whitespace correction. Inspect XY-C1 Build, TRX and artifact steps on the corresponding production-source SHA.
3. Continue MiniUPnP NAT/filter transaction audit, especially compensating rollback failures and other two-table rule mutations. Keep physical-router L4 unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
