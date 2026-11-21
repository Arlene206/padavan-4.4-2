# Cross-session execution mandate — 2026-11-21 checkpoint

## Execution rules

- Continue on `main` only. One complete Git tree and one atomic commit per logical task, then non-forced update using exact expected parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add regression tests; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the first causal failure and fix it.
- Before context exhaustion, finish the current safe slice, publish atomically and leave the exact first next action here.

## Repository state

- Main parent before the BusyBox mount-status correction: `065837e4dcc383ebe2d5276b8165ffa834f9dd3f`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- Product-source SHA `20e139d230b73eabc5f25a2b17d56a3a5768af11` has exact XY-C1 run `#44887656164` SUCCESS: Build XY-C1, TRX verification and artifact upload all passed. L3 VERIFIED only for that source SHA; physical router L4 remains NOT VERIFIED.
- PID-only helper hardening in `20e139d2` was validated by Process regression `#44887824016` and AI handoff `#44887824043` on docs-only child `94c0e2c`.
- Exact-head Process inventory `#44888827681` PASS on `065837e4dcc383ebe2d5276b8165ffa834f9dd3f`; AI handoff `#44888827646` PASS on `065837e4dcc383ebe2d5276b8165ffa834f9dd3f`.
- Storage Hotplug run `#44888827697` FAILED on `065837e4dcc383ebe2d5276b8165ffa834f9dd3f` only in the BusyBox 1.24.2 ash lifecycle step: five-field truncated mount row expected tri-state UNKNOWN (2), got 1. Other pre-ash checks passed.
- XY-C1 run `#44888827678` was IN_PROGRESS on `065837e4dcc383ebe2d5276b8165ffa834f9dd3f` at the last poll; poll it, but do not transfer its result to a later source SHA.

## Storage Hotplug active fix

`optware_lifecycle_mount_matches()` already validates full six-field mount rows and exact source/mountpoint fields, rejects duplicate matches, and treats malformed/unreadable inventories as UNKNOWN. The regression exposed that carrying UNKNOWN through AWK `exit 2` did not retain exit value 2 under the BusyBox 1.24.2 ash path. The current correction emits explicit `match` / `different` / `unknown` tokens from AWK, then maps them to shell return values 0/1/2; AWK process/read errors still map to UNKNOWN. The static checker was updated, and the full regression must now pass on the new exact source SHA.

## Completed repository audits

- Full tracked `trunk/user` process inventory on `1ab5e53519d7316243f13cff4095dc1b3c13ce5d`: 9,725 tree entries, 6,478 text files, 37 hit files, zero missing candidates/read errors. `kill_pidfile_s()` had one in-tree call only from `kill_pidfile()` compatibility wrapper; retain exported PID-only semantics for unknown external consumers. Do not repeat absent new callers.
- NVRAM transaction-boundary audit `#44887656246` PASS on `20e139d2`.

## Next engineering target after Storage CI is green

Continue WAN stability work in `trunk/user/rc/src/net_wan.c`. Audit identified that `nvram_get_int()` returns zero for both absent keys and checked backend/read/close failures; internal WAN decisions use it for PPPoE manual mode/demand, MTU, EAPOL/Kabinet auth and MAN DNS-route/resolver choices. Use `nvram_get_checked()` plus an errno-style strict integer parser; preserve absent-key zero compatibility, fail closed on backend errors, add GNU89 policy tests and exact workflow checks. A prior patch-generation attempt aborted before GitHub writes; no WAN change is in main.

## Exact first action

Fetch live `main` and backup SHA; inspect Storage Hotplug run for the correction's exact SHA. If PASS, inspect the remaining completed/skipped Actions on that SHA and then begin the WAN checked-integer NVRAM fix. If FAIL, read the first causal log and fix that specific issue. Continue independent tasks while XY-C1 or any other run is pending.

## Verification boundaries

- Host regression PASS is not firmware L3.
- XY-C1 L3 applies only when Build, TRX verification and artifact upload all pass on one exact source SHA.
- Physical router L4 is NOT VERIFIED.
- No local checkout/toolchain is available in this session; report only CI results actually observed.

## Handoff maintenance

- Root `dev_log.md` is now rotated to 20 newest-first records; the previous 100 oldest records are preserved in `docs/dev-log/2026/11/part-021.md`.
- Keep newest-first root log; archive the oldest 100 only when the next entry would make it reach 120, retaining 20.
- Update `REFACTOR_ROADMAP.md` only when module status/priority materially changes.
- Keep frozen backup permanently unchanged.
