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

- Current main before this peer-SNAT transaction slice: b7a803dcc95ae88566ffe01ff8fd8ea4c5cefd33; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- NAT/filter redirect-delete compensation fix is `b7a803dcc95ae88566ffe01ff8fd8ea4c5cefd33`. Exact-head MiniUPnP #44793663690 PASS, including the new GNU89 policy regression and source contract for NAT snapshot/restore. The existing allocation, capacity and update-rollback checks also passed.
- XY-C1 did not run a firmware build for these MiniUPnP-only changes; the prior run #44792212585 was host-only with Build/TRX/artifact SKIPPED. No L3 claim.
- New confirmed peer-side transaction gap: peer SNAT in nat POSTROUTING was committed for deletion before its paired DSCP rule in mangle. Mangle table initialization or matched DSCP delete/commit failure left an orphan DSCP rule; retry could no longer discover it because the peer SNAT lookup had already disappeared.
- Physical-router L4 remains NOT VERIFIED.

## Active task

Make peer SNAT and DSCP deletion transactional:

- Snapshot the complete peer SNAT rule and original POSTROUTING index before deleting it; if allocation fails, do not mutate either table.
- If peer SNAT deletion succeeds but mangle initialization or a matched DSCP delete/commit fails, restore the saved SNAT rule through the existing IPTC insert/commit helper.
- Do not restore SNAT if no matching DSCP rule exists; preserve primary DSCP-side status/errno if compensation fails.
- Extend the GNU89 ASan/UBSan policy test and add a source contract for snapshot, commit ordering, restoration chain/index, and both IPTABLES_143 ABI forms.

After exact-head MiniUPnP validation, continue the HTTPD/service lifecycle and MiniUPnP redirect teardown audit; do not repeat completed NAT/filter deletion or update-rollback work.

## Verification boundary

- NAT/filter deletion compensation `b7a803d`: exact-head MiniUPnP #44793663690 PASS, including sanitizer policy binary and source contract. Prior XY-C1 #44792212585 was host-only, with firmware Build/TRX/artifact SKIPPED; L3 remains NOT verified.
- This peer SNAT/DSCP compensation commit adds another policy regression and source contract; exact-head MiniUPnP Action required after push. Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and verify frozen backup stays at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Check exact-head MiniUPnP peer SNAT/DSCP sanitizer and source-contract steps; inspect the first causal log on any failure.
3. Continue HTTPD/service lifecycle and redirect teardown auditing. Keep host-only XY-C1 checks separate from firmware L3; physical-router L4 remains unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
