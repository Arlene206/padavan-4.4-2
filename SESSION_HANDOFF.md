## Current repository state

- Live `main` HEAD at recovery: `935a38ae44ece9243015e3a9c3dcae054e561097`; active branch: `main`. This is the exact current SHA fetched from `refs/heads/main`.
- Product-source comparison from L3 source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` to this HEAD reports 34 commits ahead, zero behind, and only `REFACTOR_ROADMAP.md`, `SESSION_HANDOFF.md`, and `dev_log.md` changed. No product-source delta since the recorded L3 source tree.
- Remote branch inventory: only `main`. Legacy PR #1 is CLOSED / UNMERGED (head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`, base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74`); do not use it. No checkpoint branch exists.
- Exact-HEAD `push` Actions for `935a38ae44ece9243015e3a9c3dcae054e561097`: AliyunDrive `43528744171`, AdGuardHome `43528744049`, AI handoff docs `43528744087`, infosvr `43528744070`, AliDDNS `43528744042`, and NVRAM `43528744152` — all COMPLETED / SUCCESS with exact matching `head_sha`. Job details were not yet inspected during this recovery; workflow conclusions are SUCCESS.
- Root `dev_log.md` has 52 complete newest-first records before this checkpoint; adding one yields 53/119. No rotation or INDEX update is due.

## Active task

- Priority P1 — complete the shared process-inventory fail-closed contract in `trunk/user/shared/src/pids.c`, `trunk/user/shared/src/shutils.h`, `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/services_ex.c`, and `trunk/user/rc/src/service_lifecycle_policy.h`.
- Confirmed failure modes: scan-context allocation failure, `opendir("/proc")` failure, mid-scan `readdir()` error, and PID-list allocation/growth failure can be collapsed into NULL/empty/partial inventory. Boolean consumers cannot represent UNKNOWN; lifecycle stop can signal before authoritative inventory.
- Required contract: RUNNING / ABSENT / UNKNOWN. UNKNOWN must abort before TERM/KILL, stop-success, restart, or destructive cleanup. Do not change the shared API until every process-scan API caller and lifecycle/readiness/stop consumer is enumerated and classified.
- Exhaustive inventory is still BLOCKED: six GitHub code-search queries returned zero indexed matches (not proof of no callers), recursive tree enumeration was truncated, and a fresh shallow clone failed because this runtime cannot resolve `github.com`. No safe complete checkout is available in this execution.

## Verification boundary

- L1: partial source review and recovery only; no new product-source changes.
- L2: process-inventory host regressions NOT RUN; local GNU89/`-Werror` tests NOT RUN. Local `scripts/check_ai_handoff_docs.py` NOT RUN because no checkout is available.
- Exact-HEAD Actions on current `main`: six visible workflows PASS by exact head SHA as listed above; job-step detail not yet inspected.
- L3: XY-C1 MIPSel build, TRX verification and artifact upload evidence applies to product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254`, job `132107102854`, artifact `14069572898`). Current main differs from that source only in the three documentation paths listed above.
- L4: physical router flash/boot NOT VERIFIED.

## Exact first action for the next GPT

1. Re-fetch `refs/heads/main` and verify the exact SHA before any write; inspect new exact-head `push` Actions.
2. Obtain a complete checkout at that SHA, or a demonstrably exhaustive hierarchical Git-tree/content enumeration. Do not treat GitHub code search results or a truncated recursive tree as exhaustive.
3. Enumerate and classify every `pids()`, `pids_main()`, `find_pid_by_name()`, and `procps_scan()` caller, plus all lifecycle/readiness/stop consumers, init/shutdown paths, build manifests, and test integration.
4. Implement RUNNING / ABSENT / UNKNOWN propagation for scan allocation failure, `/proc` open failure, mid-scan `readdir` error, PID-list allocation/growth failure, normal empty inventory, matched process, and per-PID disappearance. UNKNOWN must abort before signals, successful absence, restart, or cleanup.
5. Add injectable GNU89/`-Werror` regressions; migrate only confirmed consumers; run local docs validation when possible; commit directly to `main`; verify exact-head push Actions. Do not start another module or perform a partial API migration.
