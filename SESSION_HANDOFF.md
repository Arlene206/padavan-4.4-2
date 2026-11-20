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

- Repository Arlene206/padavan-4.4-2; parent main SHA for the UDP peer DSCP tuple fix: 062d09b646f294d09947c986aadcfc9e6619e6c9.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- Exact-head MiniUPnP #44808409986 and AI handoff #44808409401 PASS on docs/workflow contract commit 062d09b646f294d09947c986aadcfc9e6619e6c9. Parent-source Process inventory #44807976713 PASS and NVRAM audit #44807976901 PASS on 5ca967f2dd9da9ee20408dde1b413cfaeac9265c; the production code is unchanged by the later docs/workflow-only 4e/062 commits.
- XY-C1 #44807976845 on 5ca was still running: Build XY-C1 IN_PROGRESS, TRX and artifact upload PENDING. XY run #44808194761 on 4e skipped the firmware steps. Therefore no L3 evidence covers the current production source; prior verified L3 is limited to 30af417 and eccdab.
- Confirmed UDP peer-cleanup defect: addpeernatrule() and addpeerdscprule() call get_udp_match(rport, iport), placing internal iport in spts[0] and remote rport in dpts[0]. The peer SNAT scan incorrectly copied UDP dpts[0] into iport, then the DSCP scan compared spts[0], leaving an orphaned mangle entry after SNAT removal.
- THIS COMMIT fixes extraction to spts[0], routes TCP/UDP DSCP port checks through a small GNU89 policy helper, adds an ASan/UBSan unit regression and extends the dedicated source checker to bind production writers, cleanup and tests.

## Active task

1. Inspect exact-head MiniUPnP, AI, Process, NVRAM and XY-C1 Actions for THIS COMMIT. The new peer-port unit test and dedicated source checker must pass.
2. Count L3 only when Build XY-C1, Verify TRX output and artifact upload all complete SUCCESS on this exact product-source SHA.
3. After the UDP slice validates, continue injected backend failure coverage for NAT/filter init, delete, insert and rollback operations, checking errno and cross-table state for IPTABLES_143 and legacy handles.
4. Continue source → regression → atomic commit → exact-SHA Actions. PENDING jobs do not block independent work.

## Verification boundary

- Parent 062d09b646f294d09947c986aadcfc9e6619e6c9: MiniUPnP #44808409986 PASS and AI #44808409401 PASS; Process #44807976713 and NVRAM #44807976901 PASS on parent 5ca (source unchanged through workflow/docs-only commits).
- XY-C1 #44807976845 on 5ca: Build IN_PROGRESS, TRX PENDING, artifact upload PENDING at last poll. XY-C1 #44808194761 on 4e skipped the firmware steps. L3 for the current product source is NOT VERIFIED.
- Earlier exact-source L3: 30af417 (#44804291706) and eccdab (#44805637821), each Build/TRX/artifact PASS. These results do not validate subsequent changes.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and inspect this commit's exact-head MiniUPnP/AI/Process/NVRAM/XY-C1 runs. Confirm the UDP source-port regression and peer cleanup checker pass. Verify Build XY-C1, TRX and artifact upload individually on the same SHA. Then extend the injected IPTC backend test matrix without repeating finished CI/audits.

## Handoff maintenance

- Keep dev_log.md newest-first; archive only at the 120-record threshold, retaining the newest 20.
- Update REFACTOR_ROADMAP.md only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are distinct verification levels.
