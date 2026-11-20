# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue main-only development; every logical slice is one atomic commit on `main`, built as one complete Git tree and advanced with a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real defects and run focused regressions; do not weaken checks to obtain green CI.
- A pending Action does not block independent work. Inspect exact-SHA job logs and repair the earliest causal failure.
- Finish a safe slice and leave a factual next action before context exhaustion.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end, inspect live `main`, verify the frozen backup remains unchanged, and inspect relevant exact-SHA Actions. Update this document only when the active task, verification evidence or first next action materially changes; do not create no-op document commits.

## Repository state

- Repository Arlene206/padavan-4.4-2; live main parent: cbe06d311a01a6da73891df2c7a5587e03f312fc. Frozen backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, unchanged.
- Primary redirect deletion separates the main NAT/filter transaction from optional peer-SNAT/DSCP cleanup. Missing NAT chain fails closed; lease and description cleanup follow primary state rather than the merged auxiliary result.
- Exact-head MiniUPnP #44803703734 on 72b914095e14c099f60d6ac8f20150b4ca5d29e9 PASS after updating the writer contract to transactional append semantics.
- Review found the pass-validating source contract accepted nat_scan_completed set before the NAT loop. THIS COMMIT moves it after loop termination and updates the checker to enforce loop-close-before-completion, so only an inspected chain can prove a primary rule is absent.
- Parent XY-C1 #44803257077 on product source 35e319a57af04e113fc9a3ccaa31628764ce4cfd was still in Build XY-C1 at last poll; it does not validate this correction. Process inventory #44803257124, AI handoff #44803257093 and NVRAM audit #44803257169 all passed on 35e.
- Physical-router L4 and separate MIPS/uClibc runtime remain NOT VERIFIED.

## Active task

1. Verify this commit's exact-head MiniUPnP regression and post-loop NAT scan source contract.
2. Verify exact-head XY-C1 Build, TRX validation and artifact upload.
3. Add injected primary-delete state tests for missing NAT chain, no matching rule, NAT/filter delete errors and optional peer/DSCP failures.
4. Continue a separate whole-userland reliability/security defect while Actions are pending; do not repeat completed audits.

## Exact verification boundary

- Parent 72b MiniUPnP #44803703734 PASS; this commit changes production iptcrdr.c and its source checker.
- Parent XY source SHA 35e has #44803257077 still in progress at Build XY-C1 in the last poll. It does not validate the post-loop correction.
- Process inventory #44803257124 PASS; AI handoff #44803257093 PASS; NVRAM audit #44803257169 PASS.
- No physical-router L4 or standalone MIPS/uClibc runtime has been performed.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP and XY-C1 runs triggered by this commit. Confirm nat_scan_completed occurs after the actual NAT loop close and before auxiliary cleanup. If the regression or build fails, diagnose the earliest causal source or assertion. Continue injected delete-state testing or another whole-userland reliability/security defect while unrelated Actions are pending.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at the repository's 120-record rotation threshold, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only for material module-priority or verification changes.
- Host tests, same-SHA firmware Build/TRX/artifact L3, and physical-router L4 are distinct evidence levels.
