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

- Current main at checkpoint: `2cb3ee3d27446aca096b14f9ac0324110c0afa8b` (`fix(miniupnpd): reject unresolved IPv6 pinhole sources`).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; do not modify).
- Exact-head Actions on `2cb3ee3d`: MiniUPnP #44763259688 PASS; Process inventory #44763259657 PASS; NVRAM #44763259725 PASS; AI handoff docs #44763259666 FAIL; XY-C1 #44763259637 PENDING.
- AI handoff failure root cause: the current handoff had lost the mandatory end-of-session accuracy checkpoint wording. This documentation repair restores that contract; check the new exact-SHA AI run after the commit.
- Parent source slice `84199a2f` added IPv6 pinhole SOAP validation; its MiniUPnP #44762854186 and AI docs #44762854261 passed. The later `2cb3ee3d` change rejects unresolved remote IPv6 source names.
- Do not infer firmware L3 from host CI. Hardware L4 remains NOT VERIFIED.

## Active task

Continue stability/security work in the MiniUPnP IPv6 netfilter pinhole backend (`trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptpinhole.c`).

Confirmed review findings to address with a single focused implementation slice:
- SOAP RemoteHost=`*` currently reaches the netfilter backend, where unchecked `inet_pton` leaves a zero source address but sets an all-ones mask; that creates ::/128 instead of the required wildcard ::/0.
- `find_pinhole()` passes `*` through unchecked `inet_pton` into an uninitialized `struct in6_addr`, making duplicate pinhole lookup nondeterministic.
- UID collision search wraps indefinitely if every nonzero 16-bit UID is occupied.
- Match/target allocations are dereferenced without NULL checks; the successful `ip6tc_commit()` path leaks its libip6tc handle.
- Registry bookkeeping is allocated after installing the kernel rule; allocation failure can leave an active but untracked firewall rule.

Plan for this slice: add a shared tested IPv6/UID policy helper; normalize NULL/empty/`*` to a zero remote address with no source mask; reject invalid literal addresses; bound UID allocation; allocate the registry record before firewall mutation; check all allocation failures; release the ip6tc handle on success. Integrate the regression into MiniUPnP stability CI and include the production backend/header in XY-C1 firmware input detection.

## Verification boundary

- L2 host regression and static source-contract checks: required for this slice.
- L3 firmware validation: NOT VERIFIED on `2cb3ee3d`; require Build XY-C1, TRX verification and artifact upload to pass on the same product-source SHA.
- L4 physical hardware validation: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and verify the backup SHA; inspect the new exact-head AI handoff run and XY-C1 run without waiting on pending work.
2. Implement the MiniUPnP netfilter pinhole backend slice above against the live main tree, run the GNU89/Werror/ASan/UBSan regression and source-contract tests, and commit one atomic main tree.
3. Check MiniUPnP, AI, Process/NVRAM as triggered, and XY-C1 Actions for that exact commit SHA; diagnose any FAIL from job logs.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
