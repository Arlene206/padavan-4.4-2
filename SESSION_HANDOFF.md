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

- Repository Arlene206/padavan-4.4-2; parent for this top-level IPTC error preservation slice: 5b9fcaa09b621e880517232b24a0f35f9b296827.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Current product parent 5b9fcaa09b621e880517232b24a0f35f9b296827 adds local errno restoration after IPTC handle release in delete_rule_and_commit() and insert_rule_and_commit(). Its MiniUPnP #44807130506 PASS, AI #44807130434 PASS, Process #44807130342 PASS; NVRAM #44807130385 and XY-C1 #44807130410 were still running at last poll.
- This commit closes a second errno-loss window in delete_redirect_and_filter_rules_ex(): snapshot primary/secondary failures before logging or follow-up operations and restore the first primary errno at the final aggregate return; secondary errno is returned only if primary status succeeded.
- Product source eccdab54081a0f28c072025a55e5e2f6a8159b88 has MiniUPnP #44805637861 PASS, AI #44805637957 PASS, Process #44805637806 PASS, NVRAM #44805637802 PASS, and XY-C1 #44805637821 Build/TRX/artifact all PASS, confirming L3 for eccdab. The newer 5b commit then changes production C and requires its own exact-source build.
- Product source 30af417ed8d7665e5da0f630e79b38bbb9f92296 also has valid L3 (#44804291706 Build/TRX/artifact all PASS); earlier source SHA only.

## Active task

1. Inspect exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions for THIS COMMIT. The new error-precedence policy regression and source contract must pass.
2. Count firmware L3 only if Build XY-C1, Verify TRX output and artifact upload all complete SUCCESS on this exact source SHA.
3. If the suite passes, continue with a fault-injected lower-level backend test proving delete/insert helper errno remains stable when mock free/logging callbacks clobber errno, including IPTABLES_143 and legacy handle APIs.
4. Continue source → regression → atomic commit → exact-SHA Actions. Do not wait on PENDING jobs before doing independent work.

## Verification boundary

- Parent 5b: MiniUPnP #44807130506 PASS; AI #44807130434 PASS; Process #44807130342 PASS; NVRAM #44807130385 and XY-C1 #44807130410 were in progress at the last poll.
- Parent 5b XY build/TRX/artifact must not be attributed to this new source commit.
- Parent eccdab has valid exact-source L3: Build PASS, TRX PASS, artifact PASS. Parent 30af has valid exact-source L3 as well.
- This commit changes production C and shared policy, so its own target build and full artifact gate are required. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions. Confirm top-level primary/secondary errno precedence assertions and the policy regression. Verify Build XY-C1, TRX and artifact upload individually on this exact SHA. If a job fails, inspect the first causal log and fix it without weakening the contract. Then add injected backend errno-clobber tests for both IPTABLES_143 and legacy branches.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
