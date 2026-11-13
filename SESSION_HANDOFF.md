## Current repository state

- Live `main` baseline after the documentation recovery: `cd18661e46ab7a684666e0998dcd687adbc9a9d8`; its parent is `0a43156b1ef399804e9c93d3d51e86c16a8552e2`, which restored the required heading `## Current repository state` in this handoff. Both commits are documentation-only; product source remains at L3-recorded SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Live branch inventory: only `main`. Legacy PR #1 is CLOSED / UNMERGED (head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`, base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74`). No PR or non-main branch is used.
- Exact-head `push` Actions on prior SHA `435913a1244da38ccb722f89df0eb2a5d2648b10`: NVRAM `43525514810`, AdGuardHome `43525514772`, AliyunDrive `43525514717`, AliDDNS `43525514693` SUCCESS; AI handoff docs `43525514688` FAILED because this required heading was absent. Prior infosvr run `43525494041` on `c430f079d867d95dcb5c18193bc6ba5829948370` failed on the same stale heading after all three GNU89/ASan/UBSan binaries passed. The heading is now restored; exact-head results for `a33f8c4be90158c516c1e4d688d000600e6d62a4` are NOT YET INSPECTED.
- Recursive Git tree enumeration returned `truncated=true` with 64,005 entries. This is not an exhaustive checkout or caller inventory. The active P1 process-inventory tri-state task remains unfinished and must not be partially migrated.
- Root `dev_log.md` has 51 complete newest-first records / 119; no rotation or INDEX update is due. `REFACTOR_ROADMAP.md` remains the whole-repository dashboard; no global module priority change was established in this documentation-only recovery.

## Live recovery checkpoint — 2026-11-13 15:23 PST (UTC-8)

> Documentation CI recovery only. The exact live baseline is `cd18661e46ab7a684666e0998dcd687adbc9a9d8`. Product source and tests are unchanged. The active P1 shared process-inventory defect remains open because a complete checkout and exhaustive caller classification are prerequisites to a safe API change.

- Commit `07caeb90a6ab5707cb8c19e978e09eddd1aad857` restored the required handoff heading; commit `a33f8c4be90158c516c1e4d688d000600e6d62a4` records the CI failure and correction in `dev_log.md`; commit `cd18661e46ab7a684666e0998dcd687adbc9a9d8` normalizes its timestamp to required Beijing time. Both are direct `main` commits and documentation-only.
- The latest verified exact-head outcomes are those on parent SHA `435913a1244da38ccb722f89df0eb2a5d2648b10`; no success is inferred for the new SHA. Check all runs whose `head_sha` exactly equals `cd18661e46ab7a684666e0998dcd687adbc9a9d8` and `event=push`.
- Existing product-source L3 evidence is limited to `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (XY-C1 MIPSel/TRX run `43489646254`, job `132107102854`). L4 router flash/boot is NOT VERIFIED. Current source changes are absent; no new L2 host tests or local documentation validator were run.

## Exact first action for the next GPT

1. Re-fetch live `main`, branch inventory, PR #1 and exact-head `push` Actions; reconcile the SHA and pending documentation workflow before any source write.
2. Obtain a complete checkout at that exact SHA (or another demonstrably exhaustive repository-content enumeration). Partial code search and truncated tree output are not acceptable substitutes.
3. Enumerate and classify every `pids()`, `pids_main()`, `find_pid_by_name()`, and `procps_scan()` caller, plus every service lifecycle/readiness/stop consumer, init/shutdown path, build manifest and test integration.
4. Implement explicit RUNNING / ABSENT / UNKNOWN propagation for scan allocation failure, `/proc` open failure, mid-scan `readdir` error, PID-list allocation/growth failure, normal empty inventory, matched process and per-PID disappearance. UNKNOWN must abort before TERM/KILL, successful absence, restart or destructive cleanup.
5. Add injectable GNU89/`-Werror` regressions and migrate only confirmed consumers. Run local documentation validation when a checkout is available, commit directly to `main`, then verify exact-head push Actions. Do not select another module until this P1 is safely resolved.
