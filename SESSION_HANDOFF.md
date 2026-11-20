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

- Repository: `Arlene206/padavan-4.4-2`; parent `main` for this slice: `72b914095e14c099f60d6ac8f20150b4ca5d29e9`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- Product source fix `35e319a57af04e113fc9a3ccaa31628764ce4cfd` moves the NAT-scan-completion marker into `delete_redirect_and_filter_rules_ex()`, makes a missing NAT chain fail, and adds the new cleanup policy header to `netfilter/iptcrdr.o` dependencies. Parent XY run #44803257077 was still building at last poll.
- Workflow-only commit `72b914095e14c099f60d6ac8f20150b4ca5d29e9` corrected an inline source contract still demanding direct stdio writes inside `lease_file_add()`, although production now uses the transactional append helper. MiniUPnP #44803703734 PASS and AI handoff #44803703663 PASS on 72b.
- Prior MiniUPnP regressions on source 35: all policy, rollback, ownership, peer SNAT/DSCP, SOAP, pinhole, lease serialization, lease cleanup/replay steps passed; the only failure was the duplicate embedded stdio assertion, now corrected by 72b.
- Prior source 8f884 had an XY build error from misplaced `nat_scan_completed`; fixed in 35e. Exact-source XY Build/TRX/artifact is still not verified until run #44803257077 completes.

## Active task

1. Validate the new injected production-caller regression in this commit's MiniUPnP Action. It compiles the actual extracted `_upnp_delete_redir()` body with injected firewall and lease backends under GNU89, Werror, ASan and UBSan.
2. Check XY-C1 run #44803257077 on source SHA 35e. Count L3 only when Build XY-C1, TRX verification and artifact upload all ran and passed on that SHA.
3. The new test should prove that a peer/DSCP aggregate failure does not suppress lease cleanup when primary DNAT/filter removal succeeded, while lease removal failure still propagates errno. Diagnose and fix any test or workflow failure without weakening the contract.
4. Once this integration test passes, continue with injected low-level backend coverage or a separate concrete stability/security defect across Padavan userland. Do not repeat completed audits.

## Exact verification boundary

- Parent `72b914095e14c099f60d6ac8f20150b4ca5d29e9`: MiniUPnP #44803703734 PASS; AI handoff #44803703663 PASS.
- Product source SHA 35e: NVRAM audit #44803257169 PASS; Process inventory #44803257124 PASS; AI handoff #44803257093 PASS; MiniUPnP #44803257122 failed only at the stale inline transactional lease-writer contract (all preceding tests passed; corrected in 72b); XY-C1 #44803257077 was still in progress at last poll.
- Host verification and firmware L3 are separate. Firmware L3 remains NOT VERIFIED until the same-source Build, TRX and artifact steps pass. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`, then inspect this commit's MiniUPnP and AI handoff Actions. The integration test must compile the extracted production caller and pass every injected backend case. Separately poll XY-C1 #44803257077 on source 35e and inspect actual Build/TRX/artifact step conclusions. Continue fixing source/test failures before moving to another userland issue.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at the repository's 120-record rotation threshold, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only for material module-priority or verification changes.
- Host tests, same-SHA firmware Build/TRX/artifact L3, and physical-router L4 are distinct evidence levels.
