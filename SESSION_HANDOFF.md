## Current repository state

- Repository: \`Arlene206/padavan-4.4-2\`; development branch: \`main\` only. Current checkpoint commit is this documentation update, based on parent \`d0404d5e33d4239e19c92db9b9ab4fb7529cf26c\`. Re-fetch live \`refs/heads/main\` before any subsequent write.
- The last source/audit commit was \`d0404d5e33d4239e19c92db9b9ab4fb7529cf26c\`; its \`trunk/user\` tree is \`daafdae4e97b401a428a0ac65d83a94ebeab0ff5\` (9,439 blobs, complete recursive tree).
- Remote refs must remain as governed: \`main\` plus the immutable archival branch \`backup/pre-plugin-overhaul-2026-11-14\` (\`72f4f157a084f8184b9597ca5bf334b8080e5807\`). Never use, modify, rename, prune, rebase, reset, merge or delete that archive.
- Legacy PR #1 remains CLOSED / UNMERGED. Its valid CA/HMAC changes were already ported directly to main; do not use the PR head or reopen it.
- Latest exact-head source inventory workflow: run [43581078509](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43581078509), \`push\` on \`main\`, exact head \`d0404d5e33d4239e19c92db9b9ab4fb7529cf26c\`, conclusion SUCCESS. Artifact [14108820679](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43581078509/artifacts/14108820679), name \`process-lifecycle-inventory-d0404d5e33d4239e19c92db9b9ab4fb7529cf26c\`, retained through 2026-12-14 09:33 UTC.
- Inventory source of truth: \`docs/audits/PROCESS_LIFECYCLE_INVENTORY.md\`; full machine-readable hit list is in the Actions artifact. The run's 6,193-file text scan is an L1 lexical census, not an interprocedural call graph.
- Root \`dev_log.md\` had 75 records before this milestone; this entry adds record 76. It remains below the 384 KiB target; no archive rotation or INDEX change is due.

## Active task

- Priority P1 remains OPEN / NOT FIXED: make the Padavan shared process inventory and service-stop path fail closed with RUNNING / ABSENT / UNKNOWN semantics.
- Inventory tool added: \`scripts/audit_process_lifecycle.py\`. Reusable workflow: \`.github/workflows/process-lifecycle-inventory.yml\` (push on relevant main paths and manual workflow_dispatch). It uses a blob-filtered shallow fetch and exact-path sparse checkout, not a full repository checkout.
- Scan totals: 9,439 tracked blobs under \`trunk/user\`; 6,581 candidate source/script/build/test/config/executable paths; 6,193 readable text files scanned; 388 NUL-containing binary candidates skipped; zero missing candidates and zero read errors; 193 raw API symbol occurrences and 5,664 broad lifecycle lexical candidates.
- Key API census: \`pids()\` 77 call candidates plus one definition and one public declaration; \`pids_main()\` one caller in \`rc.c:2005\`; \`kill_services()\` 46 call candidates plus definition/declaration; \`kill_pidfile()\` five call candidates; \`kill_pidfile_s()\` 15 call candidates. Exact locations and excerpts are in the artifact.
- Important boundary: BusyBox 1.24.x contains its own \`procps_scan()\` / \`find_pid_by_name()\` implementation and internal consumers. Do not confuse this with \`trunk/user/shared/src/pids.c\` or blindly migrate BusyBox as part of the shared API change.
- Important additional finding: \`kill_pidfile_s()\` sends a signal using the numeric PID from a file without checking executable identity or process start time; stale PID reuse can signal an unrelated process. Its parser also does not validate \`strtoul()\` errno/end pointer.
- Main source blobs remain: \`shared/src/pids.c\` \`f5092806d6f1a76021643b0af54eb24705cbd8fe\`; \`shared/src/shutils.h\` \`6ccb679cf049374839a0c5de6a543955614dc275\`; \`shared/src/shutils.c\` \`e23960f774228e55316b122a793eb7c3dc12e32a\`; \`rc/src/common_ex.c\` \`de5cc703ce1b9e2eb0897dc52ed7e1c5ec15eb97\`; \`rc/src/services_ex.c\` \`8134ee60a84d626785956a073c1c1bde3422d84d\`; \`rc/src/service_lifecycle_policy.h\` \`abe9f77b5354d8a33bce66c2575d1b1d09a2d4e2\`; current policy test \`rc/src/tests/service_lifecycle_policy.c\` \`40cb5d834b2f90f599667fda8d2461c452b5406b\`.
- Do not change the public shared API yet. The lexical census is complete for its candidate policy, but semantic caller/callee classification, direct signal/PID-file routes, conditional compilation, package linkage and lifecycle consequences are not complete.

## Confirmed defects and design constraints

1. \`alloc_procps_scan()\` conflates allocation failure with \`opendir("/proc")\` failure by returning NULL.
2. \`procps_scan()\` treats NULL from \`readdir()\` as EOF without checking errno; per-PID stat read/parse failure can break the entire scan, truncating inventory.
3. argv0 allocation and PID-list growth failures can become false negatives or be collapsed into NULL; \`pids()\` / \`pids_main()\` turn that into zero/ABSENT.
4. \`service_lifecycle_stop()\` sends SIGTERM to all configured services before obtaining authoritative inventory. Its Boolean wait/escalation policy treats any nonzero status as running; a future negative UNKNOWN would be truthy in the SIGKILL pass unless explicit checks are added.
5. \`kill_pidfile_s()\` parses and signals a numeric PID without process identity protection; stale PID reuse is a separate risk.
6. Required rule: UNKNOWN must block TERM/KILL, stop-success, restart and destructive cleanup. Preflight all configured targets before sending the first signal; recheck before each escalation.

## Verification boundary

- L1: GitHub Actions exact-head sparse source census PASS; scanner self-test PASS; candidate coverage had zero missing/read errors. Manual semantic inventory is PARTIAL, not complete.
- L2: production P1 scanner/lifecycle GNU89/\`-Werror\` regressions NOT RUN. Existing \`service_lifecycle_policy.c\` does not yet exercise UNKNOWN/no-signal/preflight semantics.
- L3: historical MIPS/uClibc source build evidence remains limited to the previously recorded source SHA \`691e4d68a27580d0f053a8b4c9ae7a71bff03e6f\`; no new target build was run.
- L4: real router flash/boot/runtime NOT VERIFIED.
- No production source or firmware configuration was changed by this audit.

## Exact first action for the next GPT

1. Read the complete \`AI_DEVELOPMENT_PROTOCOL.md\`, fetch live main SHA, then read this handoff, the roadmap, newest 20 root log entries and archive index. Reconcile the latest docs Actions by exact \`head_sha\`.
2. Read \`docs/audits/PROCESS_LIFECYCLE_INVENTORY.md\`. Download the exact run artifact [43581078509](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43581078509/artifacts/14108820679) while retained; if expired, rerun \`.github/workflows/process-lifecycle-inventory.yml\` from current main.
3. Complete a semantic file-by-file inventory of all 77 \`pids()\` call candidates and 46 \`kill_services()\` call candidates. Trace enclosing lifecycle transactions and callers/callees; classify UI status, readiness/start gates, stop/restart, WAN/PPP/network teardown, storage release, USB and VPN behavior.
4. Independently inventory all direct \`kill()\`, \`killall\`, shell signal, \`pidof\`, \`pgrep\`, \`pkill\`, \`waitpid\`, \`/proc\` traversal and PID-file users, including dynamic command construction. Verify BusyBox package linkage and keep its internal scanner distinct unless evidence requires a separate fix.
5. Only after all consumers are classified, design one coherent tri-state API and service lifecycle contract. Add injectable GNU89/\`-Werror\` tests for allocation, \`/proc\` open, EOF/error, per-PID disappearance/access error, argv allocation, list growth, empty/matched lists, all-target preflight, UNKNOWN blocking TERM/KILL, PID-file parsing/identity and caller failure propagation.
6. Commit one bounded change directly to main, re-fetch before writes, then inspect exact-head push Actions. Do not claim L3/L4 without actually running them.
