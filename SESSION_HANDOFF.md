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

- Repository Arlene206/padavan-4.4-2; parent for this source-scope correction: e3578d8b09eb5555aa726726184d21eafa25dbb8.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and read-only.
- Product source 5b9fcaa09b621e880517232b24a0f35f9b296827 preserves errno across iptc_free() inside delete_rule_and_commit() and insert_rule_and_commit(). Its MiniUPnP #44807130506, AI #44807130434 and Process #44807130342 PASS; NVRAM #44807130385 PASS; XY #44807130410 was still building at the last poll.
- Product source e3578d8b09eb5555aa726726184d21eafa25dbb8 additionally captures primary/secondary errors across the delete transaction and restores primary-first errno. Its MiniUPnP #44807553369 failed the source contract because the ENOENT marker was in get_redirect_rule_by_index() instead of delete_redirect_and_filter_rules_ex(). The correction in this commit moves it to the correct function and removes the undeclared reference.
- Exact-head AI #44807553503 and Process #44807553198 PASS on e357. NVRAM #44807553409 and XY-C1 #44807553718 were still pending/in progress at last observation.
- Earlier product source eccdab has exact-source XY-C1 L3 PASS (Build, TRX, artifact); this does not verify later commits. Firmware L3 for the current product source remains NOT VERIFIED.

## Active task

1. Inspect exact-head MiniUPnP, AI, Process and NVRAM Actions for THIS COMMIT. The unchanged delete source checker must now find ENOENT in the actual delete transaction. All C policy regressions must pass.
2. Inspect exact-head XY-C1 for THIS COMMIT; count L3 only when Build XY-C1, Verify TRX output and artifact upload all complete SUCCESS on this same source SHA.
3. If host tests pass, add an injected backend regression for failure errno clobbering in both IPTABLES_143 and legacy handle paths, then verify NAT/filter/peer first-error precedence.
4. Continue source → regression → atomic commit → exact-SHA Actions; pending jobs do not block independent work.

## Verification boundary

- Parent e357: MiniUPnP #44807553369 FAIL at the standalone assertion requiring primary_failure_errno = ENOENT inside the extracted delete function; compiled policy tests before that step passed. AI #44807553503 PASS; Process #44807553198 PASS; NVRAM #44807553409 and XY-C1 #44807553718 were in progress at last poll.
- Parent 5b: MiniUPnP #44807130506 PASS, AI #44807130434 PASS, Process #44807130342 PASS, NVRAM #44807130385 PASS. XY #44807130410 still needed an explicit Build/TRX/artifact result.
- Product source eccdab has valid L3: Build PASS, TRX PASS, artifact upload PASS. Earlier 30af also has valid L3.
- Current source commit changes product C and needs a fresh exact-source firmware build. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and poll the new exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 runs. Confirm the ENOENT marker belongs only to delete_redirect_and_filter_rules_ex(), and check the standalone delete checker and policy tests. Inspect the first causal log for any failure; require Build, TRX and artifact upload to pass on the identical current source SHA before reporting L3. Then continue fault-injected IPTC errno coverage.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
