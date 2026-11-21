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

- Current main before this archive-index and workflow-check correction: `d397166be2fcd3582492a8004eac5e8dec216929`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only and unchanged.
- Product-source SHA `20e139d230b73eabc5f25a2b17d56a3a5768af11` has exact XY-C1 run `#44887656164` SUCCESS: Build XY-C1, TRX verification and artifact upload all passed. L3 is verified only for that exact source SHA. Physical router L4 remains NOT VERIFIED.
- PID-only helper hardening in `20e139d2` was validated by Process regression `#44887824016` and AI handoff `#44887824043` on docs-only child `94c0e2c`.
- Process inventory run `#44889271901` passed on source SHA `421042f3a436e82eb73305ebab77161e6c3d6f17`.
- XY-C1 run `#44889614321` completed SUCCESS on exact SHA `3791719dff38276df830dafe1ffbb2f501f0b13b`, but production-input detection marked the commit docs/checker-only, so Build XY-C1, TRX verification and artifact upload were SKIPPED. This is not new L3 evidence; the last fully verified product-source baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Storage Hotplug run `#44889614290` FAILED on exact SHA `3791719dff38276df830dafe1ffbb2f501f0b13b` only at the final static checker. BusyBox 1.24.2 ash lifecycle regression and every prior shell/compile step passed. The checker counted path literals across the entire workflow YAML and included extra references in embedded Python; the three Optware paths are already present exactly once in each `push` and `pull_request` filter. This commit scopes the check to those filter blocks.
- AI handoff run `#44889732024` FAILED on exact SHA `d397166be2fcd3582492a8004eac5e8dec216929`: the restored archive file exists, but its index row used basename `part-021.md` instead of repository-relative path `docs/dev-log/2026/11/part-021.md`; the recorded byte size was also 107,416 instead of the actual 107,650 UTF-8 bytes. The path and byte count are corrected here; the existing content hash `f80bf2d087fa1c313cb671f2108e3279092db234` and 100-record count are unchanged.

## Active task

Close the two exact-head CI contract failures without changing production storage behavior:
1. The Storage Hotplug checker must compare Optware paths inside the dedicated `push_filters` and `pull_filters` sections, not count references throughout the workflow's embedded test script. Each of the three paths already appears exactly once in each path filter.
2. The November dev-log archive index must use a repository-relative path and its actual UTF-8 byte size. The archive file has been restored unchanged with Git blob SHA `f80bf2d087fa1c313cb671f2108e3279092db234`, 107,650 bytes and 100 records.

The exact BusyBox 1.24.2 ash mount-owner regression passed on the previous Storage run. Re-run Storage Hotplug and AI handoff on this corrected commit; do not weaken coverage.


Completed process audit: full tracked `trunk/user` scan on `1ab5e53519d7316243f13cff4095dc1b3c13ce5d`: 9,725 entries, 6,478 text files, 37 hit files, zero missing candidates/read errors. `kill_pidfile_s()` has one in-tree invocation only from the `kill_pidfile()` compatibility wrapper; preserve exported numeric PID-only semantics for external consumers and do not repeat the scan absent new callers.

## Verification boundary

- Storage host regression success is not firmware L3.
- XY-C1 L3 applies only when Build XY-C1, Verify TRX output, and artifact upload all pass on one exact production-source SHA. Most recent fully verified source baseline is `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Process inventory PASS is exact only for its recorded SHA.
- Physical router L4 is NOT VERIFIED.
- No local checkout/toolchain was available in this session; only report test results observed in GitHub Actions.

## Exact first action for the next GPT

Fetch live `main` and confirm the frozen backup SHA. Inspect the exact-head AI handoff and Storage Hotplug runs created by this index/checker correction. If they pass, start WAN NVRAM error propagation in `trunk/user/rc/src/net_wan.c`: replace ambiguous `nvram_get_int()` reads for checked internal decisions with `nvram_get_checked()` and a strict errno-style integer parser, preserve absent-key defaults, fail closed on backend/read/close errors, add GNU89 policy regression tests and exact workflow coverage. Keep XY-C1 L3 attached only to a source SHA on which Build, TRX and artifact all ran and passed; a detector-skipped build is not L3.

## Handoff maintenance

- Root `dev_log.md` currently retains 22 newest-first records; this record makes 23. Archive the oldest 100 complete records only when the next addition would make the root reach 120, retaining the newest 20.
- The 100 historical records in `docs/dev-log/2026/11/part-021.md` are indexed with blob SHA `f80bf2d087fa1c313cb671f2108e3279092db234`.
- Update `REFACTOR_ROADMAP.md` only when whole-repository module priority/status materially changes; do not re-audit completed tasks.
- Keep the frozen backup permanently unchanged.
