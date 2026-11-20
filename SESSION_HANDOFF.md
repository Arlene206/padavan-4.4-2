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

- Repository Arlene206/padavan-4.4-2; parent for this IPTC errno preservation fix: d7a937541226425de4254d90498c35f3c5859994.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and read-only.
- Product source eccdab54081a0f28c072025a55e5e2f6a8159b88 gates the entire peer SNAT/DSCP deletion and peer rollback transaction on confirmed primary DNAT/filter removal. Its exact-head MiniUPnP #44805637861, AI #44805637957, Process #44805637806 and NVRAM #44805637802 all PASS.
- Exact-source XY-C1 #44805637821 on eccdab was still IN_PROGRESS at last observation: Build XY-C1 IN_PROGRESS; TRX verification and artifact upload PENDING. This product SHA has no L3 result until those exact steps pass.
- Earlier source 30af417ed8d7665e5da0f630e79b38bbb9f92296 has valid L3: XY-C1 #44804291706 Build, TRX and artifact upload all PASS. This does not verify later production changes.
- Newly identified error-propagation gap: delete_rule_and_commit() and insert_rule_and_commit() could lose the causal IPTC errno during syslog formatting or iptc_free(). This commit adds a shared errno-restoration policy, uses it after all handle cleanup, and adds compiled regression/source contracts.

## Active task

1. Inspect exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions for THIS COMMIT. The cleanup errno unit test plus the two IPTC helper source contracts must pass.
2. Count L3 only if Build XY-C1, Verify TRX output and artifact upload all complete SUCCESS for this exact product-source SHA.
3. If exact-head tests pass, continue with fault-injected coverage for primary NAT/filter backend failure and rollback cases; focus next on preserving the original errno through delete_rule_and_commit() and insert_rule_and_commit() in all IPTABLES_143/non-143 branches, as well as NAT/filter initialization failures.
4. Continue source → regression → atomic commit → exact-SHA Actions; pending unrelated work does not block independent fixes.

## Verification boundary

- Parent eccdab: MiniUPnP PASS, AI handoff PASS, Process inventory PASS, NVRAM transaction audit PASS.
- Parent eccdab XY-C1 #44805637821: Build IN_PROGRESS, TRX PENDING, artifact PENDING at last poll; not L3 yet.
- Product source 30af: XY-C1 #44804291706 Build PASS, TRX PASS, artifact upload PASS; L3 verified for 30af only.
- Current commit changes product C/header and requires its own exact-source XY-C1 build. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP/AI/Process/NVRAM/XY-C1 runs. Confirm the errno policy test and both source contracts pass. Verify Build XY-C1, TRX and artifact upload individually on the exact new source SHA. If a job fails, inspect its first causal log and repair the actual defect without weakening assertions. Then continue injected NAT/filter delete and rollback fault coverage.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
