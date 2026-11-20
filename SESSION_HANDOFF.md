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

- Current main before this deletion-transaction slice: ea745da6e423a778a65c0aba6ce85424e6a642a3; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Port-mapping enumeration capacity fix `c5e41697bfeb593223a8c7c46b9e1e972fdc7be9`: exact-head MiniUPnP #44791232803 PASS, including 4096-entry ASan/UBSan growth test.
- NAT/filter update rollback fix `bfc1c1663f9d3dc6cdb4349ba1b3601f175e27ad`, checker correction `ea745da6e423a778a65c0aba6ce85424e6a642a3`: MiniUPnP #44792212170 PASS; AI handoff docs #44792212195 PASS.
- XY-C1 #44792212585 on `ea745da` completed host-only. The firmware-input detector found no eligible firmware build inputs, so Build XY-C1, TRX verification and artifact upload were SKIPPED. This is not firmware L3.
- Confirmed deletion-side cross-table inconsistency in `delete_redirect_and_filter_rules()`: NAT deletion commits before filter-table open/removal. Filter init or matched filter delete/commit failure leaves an orphan ACCEPT rule; retry may no longer find the NAT rule used to identify it.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Make `delete_redirect_and_filter_rules()` transactional across NAT and filter tables:

- Snapshot the complete NAT rule and its 0-based chain index before mutation; if snapshot allocation fails, abort before deleting anything.
- If NAT deletion commits but filter initialization or a matched filter-rule delete/commit fails, reinsert the saved NAT rule at its original index and commit.
- Do not restore NAT when the paired filter rule is genuinely absent; preserve the primary filter-side status/errno if restoration also fails.
- Cover rollback decisions/error precedence in the GNU89 ASan/UBSan policy test and add a source contract for snapshot-before-delete, insert/commit, both IPTABLES_143 handle signatures and failure routing.

After exact-head MiniUPnP verification, continue the peer SNAT/DSCP multi-rule deletion cleanup audit. Do not repeat completed allocation, capacity or update-rollback slices.

## Verification boundary

- Exact-head MiniUPnP #44792212170 on `ea745da6e423a778a65c0aba6ce85424e6a642a3`: PASS, including GNU89 ASan/UBSan allocation/capacity/update-rollback tests and source contracts.
- AI handoff docs #44792212195 on the same SHA: PASS.
- XY-C1 #44792212585 on `ea745da`: host-only success; Build XY-C1, TRX and artifact were SKIPPED because no eligible production firmware inputs were detected. Do not claim L3.
- New NAT/filter delete compensation binary/source contract require exact-head MiniUPnP Actions after push. Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main; verify frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Check exact-head MiniUPnP delete-compensation regression and source contract; diagnose first causal failure from logs. Keep XY-C1 host-only distinct from L3.
3. Continue auditing peer SNAT/DSCP multi-rule deletion cleanup and retry semantics. Physical-router L4 remains unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
