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

- Repository Arlene206/padavan-4.4-2; parent for this checker repair: 5ca967f2dd9da9ee20408dde1b413cfaeac9265c.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Product source e3578d8b09eb5555aa726726184d21eafa25dbb8 preserves primary/secondary errno but initially put the missing-chain ENOENT marker into get_redirect_rule_by_index(); 5ca967f2dd9da9ee20408dde1b413cfaeac9265c moved that marker into the owning delete transaction and removed the undeclared lookup reference.
- Exact-head MiniUPnP #44807976684 on 5ca passed the compiled cleanup policy tests and preceding allocation/ownership/capacity/update/description checks, then failed only because the standalone checker still required the older raw return statement for NAT snapshot allocation failure.
- The production path now correctly restores the captured primary failure errno via miniupnpd_restore_errno_on_failure(). This commit updates standalone and inline contracts to require the restored return path.
- Parent 5ca AI #44807976939, Process #44807976713 and NVRAM #44807976901 were running at the last poll; XY-C1 #44807976845 queued. Exact-source L3 for this revision remains NOT VERIFIED.
- Previous product source eccdab54081a0f28c072025a55e5e2f6a8159b88 has verified L3 (Build, TRX, artifact PASS); that does not validate later changes.

## Active task

1. Validate this commit's exact-head MiniUPnP, AI, Process and NVRAM Actions. Confirm the new NAT snapshot errno assertion passes.
2. Inspect the exact-head XY-C1 run and count L3 only when Build XY-C1, Verify TRX output and artifact upload all complete SUCCESS on the same source SHA.
3. If the suite passes, add injected backend errno-clobber coverage for both IPTABLES_143 and legacy handle APIs, including delete, insert, init and rollback failures.
4. Continue source → regression → atomic commit → exact-SHA Actions; PENDING workflows do not block independent tasks.

## Verification boundary

- Parent 5ca MiniUPnP #44807976684 FAIL only at the stale NAT snapshot early-return assertion; all preceding compiled policy and source checks passed.
- Parent 5ca AI #44807976939, Process #44807976713 and NVRAM #44807976901 were in progress; XY-C1 #44807976845 queued.
- Earlier product source eccdab has exact-source L3 PASS (Build, TRX, artifact). Current product C commits require fresh XY-C1 verification. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 runs. Confirm the new early-return checker passes. Verify Build XY-C1, TRX and artifact upload individually for this exact source SHA. If any failure occurs, inspect the first causal log and fix the real contract/source mismatch without weakening coverage. Then add injected IPTC errno-clobber tests for both handle API variants.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
