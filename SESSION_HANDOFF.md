## Current repository state

- Live `main` HEAD at this recovery: `6dba16674ed6ee0b0babcf65905a3722fac23876`; active branch: `main`. This SHA is the newest commit returned by the repository-scoped commit listing and is confirmed as the tip represented by the current `main` contents. The immediately preceding source checkpoint recorded in the handoff was `935a38ae44ece9243015e3a9c3dcae054e561097`; this tip commit is documentation-only (`dev_log.md` update).
- Product-source comparison from L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` to this tip reports 36 commits ahead, zero behind; only `REFACTOR_ROADMAP.md`, `SESSION_HANDOFF.md`, and `dev_log.md` differ. No product-source delta since the recorded L3 source tree.
- Remote branch inventory: only `main`; no remote checkpoint branch exists. Legacy PR #1 remains CLOSED / UNMERGED per the existing verified repository record; do not use it.
- Exact-head push Actions for `6dba16674ed6ee0b0babcf65905a3722fac23876` completed 6/6 SUCCESS: AI handoff docs [43529987739], AliDDNS [43529987764], AliyunDrive [43529987660], AdGuardHome [43529987814], infosvr [43529987788], and NVRAM [43529987716]. All six runs have event=push and exact head_sha=`6dba16674ed6ee0b0babcf65905a3722fac23876`; all reported job steps succeeded. Full job step summaries were inspected.
- Root `dev_log.md` has 54 complete newest-first records; no rotation or INDEX update is due.

## Active task

- Priority P1 — complete the shared process-inventory fail-closed contract in `trunk/user/shared/src/pids.c`, `trunk/user/shared/src/shutils.h`, `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/services_ex.c`, and `trunk/user/rc/src/service_lifecycle_policy.h`.
- Confirmed failure modes: scan-context allocation failure, `opendir("/proc")` failure, mid-scan `readdir()` error, and PID-list allocation/growth failure can be collapsed into NULL/empty/partial inventory. Boolean consumers cannot represent UNKNOWN; lifecycle stop can signal before authoritative inventory.
- Required contract: RUNNING / ABSENT / UNKNOWN. UNKNOWN must abort before TERM/KILL, stop-success, restart, or destructive cleanup. Do not change the shared API until every process-scan API caller and lifecycle/readiness/stop consumer is enumerated and classified.
- Exhaustive inventory is still BLOCKED: six GitHub code-search queries returned zero indexed matches (not proof of no callers), recursive tree enumeration was truncated, and a fresh shallow clone failed because this runtime cannot resolve `github.com`. No safe complete checkout is available in this execution.

## Verification boundary

- L1: recovery and partial source review only; no new product-source changes. No exhaustive caller inventory was obtained.
- L2: process-inventory host regressions NOT RUN; local GNU89/`-Werror` tests NOT RUN. Local `scripts/check_ai_handoff_docs.py` NOT RUN because no checkout is available.
- Exact-HEAD Actions on current `main`: six push workflows PASS by exact head SHA; all job steps reported success (run IDs listed above).
- L3: XY-C1 MIPSel build, TRX verification and artifact upload evidence applies to product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254`, job `132107102854`, artifact `14069572898`). Current main differs from that source only in the three documentation paths listed above.
- L4: physical router flash/boot NOT VERIFIED.

## Exact first action for the next GPT

1. Re-fetch `refs/heads/main` and verify the exact SHA before any write; current exact-head Actions for `6dba16674ed6ee0b0babcf65905a3722fac23876` are reconciled (6/6 SUCCESS).
2. Obtain a complete checkout at that SHA, or a demonstrably exhaustive hierarchical Git-tree/content enumeration. Do not treat GitHub code search results or a truncated recursive tree as exhaustive.
3. Enumerate and classify every `pids()`, `pids_main()`, `find_pid_by_name()`, and `procps_scan()` caller, plus all lifecycle/readiness/stop consumers, init/shutdown paths, build manifests, and test integration.
4. Implement RUNNING / ABSENT / UNKNOWN propagation for scan allocation failure, `/proc` open failure, mid-scan `readdir` error, PID-list allocation/growth failure, normal empty inventory, matched process, and per-PID disappearance. UNKNOWN must abort before signals, successful absence, restart, or cleanup.
5. Add injectable GNU89/`-Werror` regressions; migrate only confirmed consumers; run local docs validation when possible; commit directly to `main`; verify exact-head push Actions. Do not start another module or perform a partial API migration.
