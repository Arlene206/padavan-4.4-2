## Current repository state

- Live `main` HEAD at this recovery: `eda03f83812c86e81a6f8d67130ac30f54ce4b18`; active branch: `main`. Parent: `6dba16674ed6ee0b0babcf65905a3722fac23876`. The latest commit is documentation-only; product source remains identical to L3-verified tree `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`.
- Remote branch inventory: only `main`; no checkpoint branch. Legacy PR #1 is CLOSED / UNMERGED; it was inspected for state only and is not a development target.
- Exact-head Actions for `eda03f83812c86e81a6f8d67130ac30f54ce4b18` completed 6/6 SUCCESS on `push`, each with exact matching `head_sha`: AliyunDrive [43530505638], AI handoff docs [43530505416], AdGuardHome [43530505408], AliDDNS [43530505376], infosvr [43530505319], NVRAM [43530505307]. Combined commit status has no status entries; the six Actions runs are the positive CI evidence.
- Root `dev_log.md` has 55 complete newest-first records before this recovery record; no rotation or INDEX update is due.
- Product-source L3 evidence remains on `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`; L4 router flash/boot remains NOT VERIFIED.

## Active task

- Priority P1 — complete the shared process-inventory fail-closed contract in `trunk/user/shared/src/pids.c`, `trunk/user/shared/src/shutils.h`, `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/services_ex.c`, and `trunk/user/rc/src/service_lifecycle_policy.h`.
- Confirmed failure modes: scan-context allocation failure, `opendir("/proc")` failure, mid-scan `readdir()` error, and PID-list allocation/growth failure can be collapsed into NULL/empty/partial inventory. Boolean consumers cannot represent UNKNOWN; lifecycle stop can signal before authoritative inventory.
- Required contract: RUNNING / ABSENT / UNKNOWN. UNKNOWN must abort before TERM/KILL, stop-success, restart, or destructive cleanup. Do not change the shared API until every process-scan API caller and lifecycle/readiness/stop consumer is enumerated and classified.
- The complete hierarchical Git tree for `trunk/user` was enumerated at this SHA: 10,203 entries, 764 directories, and 4,409 C/H files; the subtree response was not truncated. Five repository code-search queries (`pids_main`, `procps_scan`, `find_pid_by_name`, `pids(`, `service_lifecycle_policy`) returned zero indexed matches, which is not proof of no callers. The full 229-file RC/shared fetch attempt exceeded the connector execution call budget before completing; therefore caller enumeration remains INCOMPLETE. No shared API migration is safe yet.

## Verification boundary

- L1: complete subtree path inventory and partial source review only; no new product-source changes. Caller inventory remains incomplete.
- L2: process-inventory host regressions NOT RUN; local GNU89/`-Werror` tests NOT RUN. Local `scripts/check_ai_handoff_docs.py` NOT RUN because no checkout is available.
- Exact-HEAD Actions on current `main`: six push workflows PASS by exact head SHA (run IDs listed above). Job-step detail was not re-fetched in this recovery.
- L3: XY-C1 MIPSel build, TRX verification and artifact upload evidence applies to product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254`, job `132107102854`, artifact `14069572898`). Current main differs from that source only in the three documentation paths listed above.
- L4: physical router flash/boot NOT VERIFIED.

## Exact first action for the next GPT

1. Re-fetch `refs/heads/main` and verify the exact SHA before any write; current exact-head push Actions for `eda03f83812c86e81a6f8d67130ac30f54ce4b18` are reconciled (6/6 SUCCESS).
2. Complete the caller inventory using the now-enumerated `trunk/user` subtree: fetch and inspect all relevant C/H and shell sources for every process-scan API caller, lifecycle/readiness/stop consumer, init/shutdown path, build manifest, and test integration. Do not treat zero indexed code-search matches as exhaustive.
4. Implement RUNNING / ABSENT / UNKNOWN propagation for scan allocation failure, `/proc` open failure, mid-scan `readdir` error, PID-list allocation/growth failure, normal empty inventory, matched process, and per-PID disappearance. UNKNOWN must abort before signals, successful absence, restart, or cleanup.
5. Add injectable GNU89/`-Werror` regressions; migrate only confirmed consumers; run local docs validation when possible; commit directly to `main`; verify exact-head push Actions. Do not start another module or perform a partial API migration.
