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

- Repository Arlene206/padavan-4.4-2; parent for this evidence update: eccdab54081a0f28c072025a55e5e2f6a8159b88.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Product source SHA 30af417ed8d7665e5da0f630e79b38bbb9f92296 has complete XY-C1 L3 PASS: run #44804291706, Build XY-C1 PASS, TRX verification PASS, artifact upload PASS. This validates only source through 30af417.
- Later product changes are not yet covered by that L3: 7d9b12c46927b2fbb3aba6ea0346de0cc1b7d0a1 adds the primary-metadata cleanup finalizer; current product source eccdab54081a0f28c072025a55e5e2f6a8159b88 gates peer SNAT/DSCP cleanup on confirmed primary redirect removal.
- Exact-head MiniUPnP #44805637861 PASS, AI handoff #44805637957 PASS, process inventory #44805637806 PASS, and NVRAM transaction audit #44805637802 PASS on eccdab.
- Exact-head XY-C1 #44805637821 on eccdab is still IN_PROGRESS: Build XY-C1 IN_PROGRESS; Verify TRX output and artifact upload PENDING. Do not claim L3 for eccdab until all three actual steps pass.
- The peer-cleanup gate fixes a discovered state-corruption bug: the old code could delete peer SNAT/DSCP after primary NAT deletion failed, or after a successful NAT rollback following filter deletion failure, leaving an existing DNAT without supporting peer rules. The complete secondary transaction is now gated on primary_mapping_removed. Its C policy regression and source checker pass in the MiniUPnP workflow.

## Active task

1. Check exact-head XY-C1 run #44805637821 for eccdab. Count L3 only if Build XY-C1, Verify TRX output and artifact upload each complete SUCCESS on this exact SHA.
2. If Build fails, inspect its earliest causal compiler/error line and make a targeted GNU89/MIPS/uClibc-safe source fix. If Build passes, confirm TRX and artifact steps individually.
3. Then add fault-injected backend coverage for NAT init/delete failure, filter init/delete failure, successful/failed NAT restoration and missing/no-match primary chain; keep peer state unchanged whenever primary mapping remains.
4. Continue source → regression → atomic commit → exact-SHA Actions. The already passing host, process and NVRAM jobs do not need repeated audits.

## Verification boundary

- XY-C1 run #44804291706, exact source SHA 30af417: Build PASS, TRX PASS, artifact upload PASS. L3 verified for 30af417 only.
- On exact source SHA eccdab: MiniUPnP #44805637861 PASS; AI handoff #44805637957 PASS; process inventory #44805637806 PASS; NVRAM audit #44805637802 PASS.
- XY-C1 #44805637821 on eccdab: Build IN_PROGRESS, TRX PENDING, artifact PENDING at the last check. L3 for eccdab NOT VERIFIED.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect run #44805637821. Verify the actual Build XY-C1, TRX and artifact-upload conclusions for exact SHA eccdab. If a failure exists, inspect the first causal log line and fix it. If it passes, move to injected low-level backend coverage or another concrete userland stability/security bug without repeating the completed main/backup recovery or host audits.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
