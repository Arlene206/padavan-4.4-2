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

- Repository Arlene206/padavan-4.4-2. Parent for this first-error precedence slice: b6c1519c69cedefba6483e51a810106f185bef1a.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Product source 4a7d3c3e3b1966d1574f703ffbfc498bce292542 fixes the UDP peer-cleanup tuple mismatch by matching the internal UDP source port in the mangle DSCP scan. It has dedicated source and policy regressions. Its exact-source XY status was pending at the earlier checkpoint.
- The preceding test commit adds scripts/test_miniupnpd_iptc_errno.py. It extracts the real delete_rule_and_commit() and insert_rule_and_commit() implementations, compiles and executes both the legacy and IPTABLES_143 handle APIs, and uses iptc_strerror/syslog/iptc_free mocks that deliberately clobber errno.
- This commit prevents later failure sites in the same transaction phase from overwriting the first causal errno. The primary and secondary phases each preserve their first error, and the final selector returns primary error before secondary error.
- Product L3 is unverified for this current source until the same-SHA Build XY-C1, TRX and artifact gates pass. Older verified SHA results do not apply. Physical-router L4 is not verified.

## Active task

1. Inspect the exact-head MiniUPnP, AI, Process and NVRAM Actions for this commit. The first-error policy regression and primary/secondary source contracts must pass.
2. Inspect exact-head XY-C1 and count L3 only when Build XY-C1, Verify TRX output and artifact upload each complete SUCCESS on the identical source SHA.
3. If host checks pass, expand the injected IPTC harness to exercise transaction-level error precedence and rollback state, or choose another concrete userland reliability/security defect.
4. Continue source → regression → atomic commit → exact-SHA Actions; PENDING work does not block independent implementation.

## Verification boundary

- The preceding injected errno helper runner is expected to execute both API variants; check its exact-head MiniUPnP step before relying on it.
- This commit changes cleanup policy and production netfilter code and needs fresh exact-SHA validation.
- Previous source 30af417 and eccdab540 have valid but distinct XY-C1 L3 evidence; do not attribute those builds to this commit.
- Frozen backup remains unchanged.

## Exact first action for the next GPT

Fetch live main and inspect exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions. Confirm that the injected helper harness compiles/runs the legacy and IPTABLES_143 APIs, and the new first-error test and source contracts pass. Confirm Build XY-C1, TRX verification and artifact upload separately on this exact source SHA. Then continue transaction-level rollback injection or another demonstrated userland bug.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
