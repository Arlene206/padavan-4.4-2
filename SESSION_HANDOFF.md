## Current repository state

- Live `main` HEAD at this recovery: `f40aba8106e24993cbc64fd9a23dee4756620790`; active branch: `main`. Parent: `eda03f83812c86e81a6f8d67130ac30f54ce4b18`. This is a handoff/log-only commit; product-source tree remains unchanged from the recorded L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Remote branch inventory: only `main`. Legacy PR #1 is CLOSED / UNMERGED (head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`, base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74`); no PR or non-main branch used for development.
- Exact-head Actions for `f40aba8106e24993cbc64fd9a23dee4756620790`: six `push` runs, all completed SUCCESS and all `head_sha` values exactly match: NVRAM [43531298250], AdGuardHome [43531298340], infosvr [43531298263], AliDDNS [43531298235], AI handoff docs [43531298253], AliyunDrive [43531298258]. These are host/source/documentation workflows, not firmware/hardware proof.
- Root `dev_log.md` currently has 56 complete newest-first records (142,625 characters); no rotation due (20–119 records; below 384 KiB trigger). Archive index unchanged.
- L3 evidence remains limited to product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (XY-C1 MIPSel build/TRX verification/artifact upload run `43489646254`, job `132107102854`, artifact `14069572898`). L4 router flash/boot NOT VERIFIED.

## Active task

- Priority P1: finish the shared process-inventory fail-closed contract centered on `trunk/user/shared/src/pids.c`, public declarations in `trunk/user/shared/src/shutils.h`, and lifecycle consumers in `trunk/user/rc/src/common_ex.c`, `services_ex.c`, and `service_lifecycle_policy.h`.
- Source-confirmed defects: `alloc_procps_scan()` returns NULL for allocation and `opendir("/proc")` failures; `procps_scan()` treats every NULL `readdir()` as normal EOF without checking errno; `find_pid_by_name()` returns NULL for both scan failure and allocation failure, while callers can interpret NULL as no process. PID-list growth failure also collapses into NULL. This can produce false ABSENT/partial inventory and unsafe lifecycle decisions.
- Required semantics: RUNNING / ABSENT / UNKNOWN. UNKNOWN must prevent TERM/KILL, stop-success, restart, and destructive cleanup. Do not change the shared API or migrate consumers until all API callers and lifecycle/readiness/stop paths are exhaustively enumerated and classified.
- Enumeration progress: current-main Git tree previously enumerated hierarchically at `eda03f83812c86e81a6f8d67130ac30f54ce4b18`: `trunk/user` contained 10,203 entries, 764 directories, 4,409 C/H files, not truncated. RC and shared source directory listings have now been inspected. GitHub code search for `pids(`, `pids_main(`, `find_pid_by_name(`, `procps_scan(`, `service_lifecycle_stop`, and `kill_pidfile_s` returned zero indexed matches; this is NOT exhaustive. Bulk retrieval of all RC/shared source files exceeded connector execution budget in the previous attempt. Caller inventory remains INCOMPLETE; no safe API migration yet.

## Verification boundary

- L1: partial source review confirms the fail-open paths; directory inventories available; exhaustive cross-userland caller classification NOT COMPLETE.
- L2: process inventory GNU89/`-Werror` host regressions NOT RUN; local `scripts/check_ai_handoff_docs.py` NOT RUN (no local checkout).
- Exact-head Actions for current main: 6/6 push workflows SUCCESS as listed above.
- L3: only the source SHA and run/artifact listed above are verified. Current main's latest commits change documentation only; no newer product-source L3 claim.
- L4: physical router flash/boot NOT VERIFIED.

## Exact first action for the next GPT

1. Re-fetch `refs/heads/main`; expected baseline after this handoff update is the new direct child of `f40aba8106e24993cbc64fd9a23dee4756620790`. Reconcile exact-head push Actions before further writes.
2. Obtain complete source content in bounded batches, starting with all RC/shared C/H files and then every other `trunk/user` subtree. Search locally or by content for all declarations/definitions/callers of `pids()`, `pids_main()`, `find_pid_by_name()`, `procps_scan()`, plus lifecycle consumers and signal/cleanup paths. Record exact paths and classify each caller; zero indexed search results are not proof of absence.
3. Do not implement partial tri-state API changes until that inventory is complete. Then implement RUNNING / ABSENT / UNKNOWN, distinguish normal EOF from `readdir` errors, propagate scan and allocation failures, and define safe per-PID disappearance semantics.
4. Add injectable GNU89/`-Werror` host regressions for allocation, `/proc` open, readdir error/EOF, list growth, empty/matched process, and PID disappearance; migrate every confirmed consumer fail-closed. Run relevant source tests and docs validation when a local checkout is available.
5. Commit directly to `main`, append a concise log record, and verify exact-head push Actions. Do not create branches/PRs or start another module before this P1 task is safely resolved.
