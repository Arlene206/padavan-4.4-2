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

- Current main before archive restoration: `3791719dff38276df830dafe1ffbb2f501f0b13b`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged.
- Product-source SHA `20e139d230b73eabc5f25a2b17d56a3a5768af11` has exact XY-C1 run `#44887656164` SUCCESS: Build XY-C1, TRX verification and artifact upload all passed. L3 is verified only for that exact source SHA. Physical router L4 remains NOT VERIFIED.
- PID-only helper hardening in `20e139d2` was validated by Process regression `#44887824016` and AI handoff `#44887824043` on docs-only child `94c0e2c`.
- Process inventory run `#44889271901` passed on source SHA `421042f3a436e82eb73305ebab77161e6c3d6f17`.
- XY-C1 run `#44889614321` is IN_PROGRESS on exact source SHA `3791719dff38276df830dafe1ffbb2f501f0b13b` at the last poll; do not infer a result until Build XY-C1, TRX verification and artifact upload complete on that same SHA.
- Storage Hotplug run `#44889614290` is IN_PROGRESS on exact SHA `3791719dff38276df830dafe1ffbb2f501f0b13b`; all shell syntax and pre-ash runtime steps had passed at the last poll, and the BusyBox 1.24.2 ash build/lifecycle regression was in progress. The previous failure on `421042f` was only a static assertion typo; commit `3791719d` corrected it.
- AI handoff run `#44889614300` failed on exact SHA `3791719dff38276df830dafe1ffbb2f501f0b13b`: the validator found that `docs/dev-log/2026/11/part-021.md` was indexed but absent from the Git tree. Its original complete 107,416-byte content was recovered from the existing Git blob `f80bf2d087fa1c313cb671f2108e3279092db234` and is restored at the indexed path in this commit.

## Active task

Restore the missing `docs/dev-log/2026/11/part-021.md` archive object. The index already referenced the correct blob SHA `f80bf2d087fa1c313cb671f2108e3279092db234`, but the path was omitted from the tree; the original complete 107,416-byte blob has now been reintroduced without rewriting its contents. Verify the exact-head AI handoff contract and continue monitoring Storage Hotplug and XY-C1 Actions on their recorded SHAs.


Completed process audit: full tracked `trunk/user` scan on `1ab5e53519d7316243f13cff4095dc1b3c13ce5d`: 9,725 entries, 6,478 text files, 37 hit files, zero missing candidates/read errors. `kill_pidfile_s()` has one in-tree invocation only from the `kill_pidfile()` compatibility wrapper; preserve exported numeric PID-only semantics for external consumers and do not repeat the scan absent new callers.

## Verification boundary

- Storage host regression success is not firmware L3.
- XY-C1 L3 applies only when Build XY-C1, Verify TRX output, and artifact upload all pass on one exact production-source SHA. Most recent fully verified source baseline is `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Process inventory PASS is exact only for its recorded SHA.
- Physical router L4 is NOT VERIFIED.
- No local checkout/toolchain was available in this session; only report test results observed in GitHub Actions.

## Exact first action for the next GPT

Fetch live `main` and confirm the frozen backup SHA. Inspect exact-head AI handoff and Storage Hotplug Actions created by this archive-restoration commit; the previous AI failure was caused only by the indexed `part-021.md` path being missing. Continue to monitor XY-C1 run `#44889614321` only for exact source SHA `3791719dff38276df830dafe1ffbb2f501f0b13b`; no descendant docs commit can inherit its firmware result. When the immediate governance checks are resolved, continue the WAN NVRAM error-propagation stability task in `trunk/user/rc/src/net_wan.c`: audit identified that `nvram_get_int()` returns zero for both absent keys and backend/read/close failures, affecting PPPoE manual/demand, MTU, EAPOL/Kabinet auth and MAN DNS route/resolver choices. Use `nvram_get_checked()`, preserve absent-key defaults, fail closed on backend errors, add GNU89 regression coverage and exact workflow checks.

## Handoff maintenance

- Root `dev_log.md` currently retains 21 newest-first records; this record makes 22. Archive the oldest 100 complete records only when the next addition would make the root reach 120, retaining the newest 20.
- The 100 historical records in `docs/dev-log/2026/11/part-021.md` are indexed with blob SHA `f80bf2d087fa1c313cb671f2108e3279092db234`.
- Update `REFACTOR_ROADMAP.md` only when whole-repository module priority/status materially changes; do not re-audit completed tasks.
- Keep the frozen backup permanently unchanged.
