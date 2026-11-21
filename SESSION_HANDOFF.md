# Cross-session execution mandate — 2026-11-21 checkpoint

## Execution rules

- Continue on `main` only. Use one atomic main commit for each logical task: create all blobs, one complete Git tree and one commit, then update the ref non-forcibly with the exact expected parent SHA.
- Never amend, force-push, create development branches or PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add regression tests; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the first causal failure and fix it.
- Before context exhaustion, finish the current safe slice, publish atomically, and leave the exact first next action here.

## Mandatory handoff checkpoint — end-of-session accuracy

The checkpoint is mandatory; a document edit is conditional. At every session end, compare live `main`, relevant exact-SHA Actions, the product-source baseline, verification boundary, blockers and the first next action with this record. Update it only when one of those resumable facts materially changes; do not create a no-op commit.

## Current repository state

- Current main parent before this checkpoint: `fa888e9c9323a555d63b2d84a9bd4bd82a181764`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged.
- Product-source SHA `20e139d230b73eabc5f25a2b17d56a3a5768af11` has exact XY-C1 run `#44887656164` SUCCESS: Build XY-C1, TRX verification and artifact upload all passed. L3 is verified only for that exact source SHA. Physical router L4 remains NOT VERIFIED.
- PID-only helper hardening in `20e139d2` was validated by Process regression `#44887824016` and AI handoff `#44887824043` on docs-only child `94c0e2c`.
- Process inventory run `#44889271901` passed on source SHA `421042f3a436e82eb73305ebab77161e6c3d6f17`.
- XY-C1 run `#44889271879` was still IN_PROGRESS on exact source SHA `421042f3a436e82eb73305ebab77161e6c3d6f17` at the last poll: all host checks passed, Build XY-C1 was running, TRX verification and artifact upload were pending. Do not infer a result for a later SHA.
- Storage Hotplug run `#44889271838` on `421042f3` completed with failure only in the final Python static checker. All preceding shell/compile tests passed, including the BusyBox 1.24.2 ash mount-inventory regression. The static checker expected the wrong literal `case _mount_match_result in`; actual helper uses `case $_mount_match_result in`. That checker assertion is corrected in the current commit.
- AI handoff run `#44889426235` failed on exact docs-only SHA `fa888e9c9323a555d63b2d84a9bd4bd82a181764`: the handoff lacked the required phrase `one atomic main commit` and the required section headings. The handoff is now reshaped to the checker contract in the current commit; confirm exact-head CI.

## Active task

The immediate task is to close two CI contract failures without changing production storage behavior:
1. The Optware tri-state source assertion now matches the exact shell syntax `case $_mount_match_result in`; rerun Storage Hotplug on this commit and verify all earlier ash regression coverage remains successful.
2. The live handoff now has the exact required section structure and mandatory wording; rerun the AI handoff documentation contract on this commit.

Storage ownership behavior retained in `optware_lifecycle_mount_matches()`: validate complete six-field mount rows, compare source and `/opt` mountpoint exactly, reject duplicate matches, classify malformed/unreadable/empty inventory as UNKNOWN, and remove only on one exact owner match. The shell emits explicit `match` / `different` / `unknown` AWK tokens and maps them to 0/1/2 so BusyBox awk/ash does not collapse UNKNOWN.

Completed process audit: full tracked `trunk/user` scan on `1ab5e53519d7316243f13cff4095dc1b3c13ce5d`: 9,725 entries, 6,478 text files, 37 hit files, zero missing candidates/read errors. `kill_pidfile_s()` has one in-tree invocation only from the `kill_pidfile()` compatibility wrapper; preserve exported numeric PID-only semantics for external consumers and do not repeat the scan absent new callers.

## Verification boundary

- Storage host regression success is not firmware L3.
- XY-C1 L3 applies only when Build XY-C1, Verify TRX output, and artifact upload all pass on one exact production-source SHA. Most recent fully verified source baseline is `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Process inventory PASS is exact only for its recorded SHA.
- Physical router L4 is NOT VERIFIED.
- No local checkout/toolchain was available in this session; only report test results observed in GitHub Actions.

## Exact first action for the next GPT

Fetch live `main` and the frozen backup SHA, then inspect the exact-head Storage Hotplug and AI handoff runs created by this commit. If either fails, read its first causal log and make a focused correction without weakening tests. Poll XY-C1 run `#44889271879` only for its exact source SHA `421042f3a436e82eb73305ebab77161e6c3d6f17`; do not attribute that result to this docs-only commit. Once both governance checks are green (or pending without blocking independent work), continue the WAN NVRAM error-propagation stability task in `trunk/user/rc/src/net_wan.c`: `nvram_get_int()` returns zero for both absent keys and checked backend/read/close failures, and several internal WAN decisions incorrectly treat zero as valid PPPoE, MTU, auth and DNS mode. Use `nvram_get_checked()`, preserve absent-key defaults, test the error path, and integrate a focused exact-head CI regression. The earlier patch-generation attempts aborted before remote writes; no WAN change has landed.

## Handoff maintenance

- Root `dev_log.md` currently retains 20 newest-first records; the new record makes 21. Archive the oldest 100 complete records only when the next addition would make the root reach 120, retaining the newest 20.
- The 100 historical records in `docs/dev-log/2026/11/part-021.md` are indexed with blob SHA `f80bf2d087fa1c313cb671f2108e3279092db234`.
- Update `REFACTOR_ROADMAP.md` only when whole-repository module priority/status materially changes; do not re-audit completed tasks.
- Keep the frozen backup permanently unchanged.
