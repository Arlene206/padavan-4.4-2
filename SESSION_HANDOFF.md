# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on main only. Every logical task MUST be one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix actual source defects with focused regressions; never weaken checks to obtain PASS.
- A PENDING Action does not block independent work. Diagnose exact-SHA failures from the earliest causal log.
- Before context exhaustion, finish a safe slice, commit it and leave an exact first action for the next GPT. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end inspect live main, verify frozen backup SHA, and inspect relevant exact-SHA Actions. Update this file only when task, source baseline, verification evidence or first next action materially changes. Do not create a no-op commit.

## Current repository state

- Repository Arlene206/padavan-4.4-2; parent for this injected IPTC errno regression slice: 4a7d3c3e3b1966d1574f703ffbfc498bce292542.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Product-source commit 4a7d3c3e3b1966d1574f703ffbfc498bce292542 fixes a real UDP peer-cleanup tuple defect: get_udp_match(rport, iport) stores the internal endpoint in spts[0], but the peer SNAT scan used dpts[0] and could miss the matching mangle DSCP entry. Source now uses spts[0] and a shared GNU89 port-match policy; C regression and dedicated source checker cover the tuple contract.
- On parent 4a, MiniUPnP #44808660926, AI #44808661003, Process #44808661091, NVRAM #44808660908 and XY-C1 #44808661058 were all in progress at the latest poll. XY L3 remains unverified until its Build, TRX and artifact steps pass on source 4a.
- Earlier confirmed XY-C1 L3 exists for 30af417 and eccdab only; those do not validate later product changes.
- This commit adds executable errno-clobber tests of the actual IPTC delete/insert helpers under both legacy and IPTABLES_143 handle APIs. Logging/string conversion/free mocks deliberately clobber errno.

## Active task

1. Check this commit's exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions. The new production-helper test must compile/run for both handle APIs under GNU89/Werror/ASan/UBSan.
2. Count L3 only if Build XY-C1, Verify TRX output and artifact upload all complete SUCCESS on the exact same product-source SHA. If the parent 4a build finishes after the test-only commit, it still validates the unchanged product code 4a; confirm the exact run and actual steps.
3. If both variants pass, extend injected coverage to transaction-level primary/secondary failure precedence and NAT/filter rollback state; otherwise diagnose the first causal compiler/test/source assertion.
4. Continue source → regression → atomic commit → exact-SHA Actions, without repeating completed audits or waiting on unrelated jobs.

## Verification boundary

- Parent 4a: exact-head MiniUPnP #44808660926, AI #44808661003, Process #44808661091, NVRAM #44808660908 and XY-C1 #44808661058 were in progress at last poll.
- Source 4a includes the UDP internal-source-port correction. No L3 claim is made until Build XY-C1, Verify TRX output and artifact upload are each successful for that source.
- Earlier source 30af417 and eccdab540 have separate exact-source L3 PASS; do not apply them to later SHAs.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP/AI/Process/NVRAM/XY Actions. Confirm the newly wired errno harness completes both legacy and IPTABLES_143 variants and all injected clobbering cases pass. Confirm the XY run for source 4a actually ran Build, TRX and artifact upload successfully; only then report L3. Continue lower-level transaction error/rollback regression work afterward.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
