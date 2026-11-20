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

- Parent `main` SHA immediately before this atomic source slice: `a6a0175208fcbd073d1bbdb424de501a5885cf4b`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; preservation branch must remain unchanged).
- Exact-head push Actions on parent `a6a01752`: VPN server lifecycle #44766404316 PASS; AI handoff documentation #44766404187 PASS.
- The prior MiniUPnP IPv6 wildcard/UID/resource-lifecycle slice is implemented. On product-source SHA `84199a2fe17d774b8bcdba0a1918e7fa10a33160`, MiniUPnP regression #44762854186 and XY-C1 firmware validation #44762854295 passed. This does not validate the new counter-default source change.
- Local GNU89/`-Wall -Wextra -Werror` ASan/UBSan policy regression for this slice passed. New exact-head MiniUPnP and XY-C1 runs are required after this commit.
- Do not infer hardware validation from host CI or firmware automation. Physical-router L4 remains NOT VERIFIED.

## Active task

The pinhole wildcard/UID/allocation/resource-lifecycle task recorded in the previous checkpoint has been implemented. This commit fixes a follow-on reliability defect in `trunk/user/miniupnpd/miniupnpd-2.x/netfilter/iptpinhole.c`:

- If a pinhole registry entry survives after its kernel rule disappears, `get_pinhole_info()` previously returned success without initializing packet/byte outputs. The caller could copy an uninitialized `packets_tmp` value and misreport the pinhole as working.
- A shared null-safe counter initializer now sets requested counters to zero before querying netfilter; a matching rule overwrites those defaults. Regression tests cover both counters and optional NULL arguments, while the MiniUPnP workflow pins initialization-before-lookup ordering.

After exact-head CI verification, continue the repository-wide stability/security queue. Prioritize concrete unfinished high-impact WAN/service lifecycle issues in `REFACTOR_ROADMAP.md`; do not repeat the completed pinhole wildcard/UID audit.

## Verification boundary

- L2 host policy regression: GNU89, `-Wall -Wextra -Werror`, ASan and UBSan PASS locally.
- Static source contract: added to `.github/workflows/miniupnpd-config-stability.yml`; exact-head result pending until the new `main` push completes.
- XY-C1 detector already watches the production pinhole backend and policy header. Exact new-SHA Build XY-C1 / TRX / artifact result must still be checked; do not transfer the preceding `84199a2` result.
- Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and confirm the frozen backup still equals `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Check push-triggered MiniUPnP and XY-C1 Actions whose `head_sha` exactly equals the new `main` commit. Diagnose any failure from its causal job log; a pending run must not block independent work.
3. Once the slice is verified, resume the highest-priority unfinished WAN/service-lifecycle stability defect in the live roadmap. Keep source, regression and required integration changes in one atomic `main` commit.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
