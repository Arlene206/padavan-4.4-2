## Current repository state

- Repository: Arlene206/padavan-4.4-2; development branch: main only. This handoff is based on the live main SHA 736259cde2507d0997a240f01814c4ab4ff13527; always re-fetch refs/heads/main before any write.
- Source inventory baseline: d0404d5e33d4239e19c92db9b9ab4fb7529cf26c. Compare to pre-review main 736259cde2507d0997a240f01814c4ab4ff13527: only two documentation commits changed REFACTOR_ROADMAP.md, SESSION_HANDOFF.md, dev_log.md and docs/audits/PROCESS_LIFECYCLE_INVENTORY.md. No production source/build/workflow/test file changed since the inventory baseline.
- Remote refs remain main plus immutable archive backup/pre-plugin-overhaul-2026-11-14 (72f4f157a084f8184b9597ca5bf334b8080e5807). Never use, modify, rename, prune, rebase, reset, merge or delete that archive.
- Legacy PR #1 remains CLOSED / UNMERGED. Its valid CA/HMAC changes were ported to main earlier. Do not reopen or use the PR head.
- Latest process inventory run: 43581078509, push on main, exact source head d0404d5e33d4239e19c92db9b9ab4fb7529cf26c. Artifact 14108820679 is the source evidence.
- Semantic caller review: docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md. It classifies all 77 pids() call candidates, the one pids_main() caller, all 46 kill_services() calls, all five kill_pidfile() callers and 15 kill_pidfile_s() call expressions. It records targeted direct signals, wait/reap, PID producers/consumers, feature guards and package linkage.
- Root dev_log.md advances 77 to 78 records with this milestone and remains below the 384 KiB rotation threshold. Do not rotate or update docs/dev-log/INDEX.md for this entry.
- Six exact-head push Actions on pre-review SHA 736259cde2507d0997a240f01814c4ab4ff13527 all passed, including NVRAM transaction audit run 43584332802. Re-fetch Actions after this documentation commit and report only runs whose head_sha exactly equals resulting main SHA.

## Active task

- P1 remains OPEN / NOT FIXED: make the Padavan shared process inventory and service lifecycle fail closed with RUNNING / ABSENT / UNKNOWN.
- Phase 1 semantic caller classification for the requested API surface is complete. The L1 lexical census and artifact remain source evidence; the manual review is a direct-call-site review, not a claim that all 5,664 broad lifecycle lexical candidates have a complete interprocedural call graph.
- Production source has not been changed. Do not change pids() return convention before migrating callers explicitly so negative UNKNOWN cannot be consumed as a C Boolean.
- Build ownership: pids.c/shutils.c/shutils.h are in shared libshared.so, consumed by RC and HTTPD. BusyBox libbb/procps.c is an independent scanner and must not be conflated or batch-edited.
- Detailed caller classification and phase-2 constraints are in docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md. Highest-risk chains: VPN Server stop/restart; VPN Client stop/restart; USB modem removal; AP LAN teardown; flash_firmware() through void stop_services(); Samba config/restart; HTTPD/crond/GPIO readiness; storage service stop/eject; pids()-guarded udhcpc/inadyn/networkmap/watchdog notifications.

## Confirmed defects and non-negotiable constraints

1. alloc_procps_scan() conflates allocation and /proc open errors; readdir() errors are treated as EOF; per-PID inaccessible/malformed records are skipped without marking the overall inventory UNKNOWN; argv0/vector allocation failures can become false negatives; pids()/pids_main() collapse these failures to zero.
2. pids() currently returns a process count. Keep count/snapshot information separate from the tri-state presence contract.
3. service_lifecycle_stop() has no whole-list authoritative preflight before TERM. Its wait/SIGKILL checks are Boolean and would treat negative UNKNOWN as true. UNKNOWN blocks TERM, KILL and stop-success.
4. Multiple callers signal after pids() checks, including inadyn, udhcpc, networkmap, watchdog and xl2tpd. UNKNOWN must not authorize a signal.
5. kill_pidfile_s() parses numeric PID without strict syntax, expected executable identity or start-time checks. PID files have mixed owners: in-tree monitors, BusyBox udhcpc, PPPD and an unresolved lpdparent.pid writer. Coordinate each writer or legacy data remains UNKNOWN/no-signal.
6. Before a multi-service stop sends its first signal, preflight every target, including additional PID-file targets in the same lifecycle transaction. Revalidate each snapshot identity before TERM and every escalation.
7. Linux 4.4 has no pidfd_send_signal(); PID+start-time validation has residual check/kill TOCTOU. Do not claim race-free PID identity.
8. UNKNOWN cannot be interpreted as RUNNING, ABSENT, already stopped, restart-safe or cleanup-safe. No destructive cleanup, interface teardown, storage detach, callback deletion, stopped-state publication or restart may continue after UNKNOWN.
9. Never touch backup/pre-plugin-overhaul-2026-11-14. Do not create branches or PRs. Do not modify GPT Session Rotator.

## Verification boundary

- L1: sparse whole-tree lexical inventory PASS at d0404d5e33d4239e19c92db9b9ab4fb7529cf26c; requested direct API semantic caller classification PASS in the new review document. Broad non-API lifecycle-candidate closure remains outside this milestone.
- L2: P1 production scanner/lifecycle regressions NOT RUN. Existing service_lifecycle_policy.c has six Boolean cases and no UNKNOWN/no-signal/preflight tests.
- L3: historical MIPS/uClibc source build evidence only at SHA 691e4d68a27580d0f053a8b4c9ae7a71bff03e6f; no new target build.
- L4: XY-C1 hardware flash/boot/runtime NOT VERIFIED.
- Six exact-head push Actions on pre-review SHA passed; resulting documentation SHA must be checked independently. Green Actions are not firmware or hardware validation.

## Exact first action for the next GPT

1. Read complete AI_DEVELOPMENT_PROTOCOL.md first, then fetch live main and read this handoff, REFACTOR_ROADMAP.md, newest 20 dev_log.md records and docs/dev-log/INDEX.md. Check exact-head Actions.
2. Read docs/audits/PROCESS_LIFECYCLE_SEMANTIC_REVIEW.md and cited sources. Do not restart lexical-scan design or repeat completed call-site census.
3. Design a coherent snapshot+tri-state API for pids.c/shutils: explicit scan errors, per-PID disappearance versus permission/I/O/parse failure, allocation failure, argv0 failure and vector-growth failure.
4. Design lifecycle policy around all-target preflight and identity-bound signal operations. Document residual Linux 4.4 PID check/kill race.
5. Add GNU89/-Wall/-Wextra/-Werror injectable scanner/lifecycle tests with ASan/UBSan; add dedicated path-filtered Actions or extend the most relevant existing host-regression workflow.
6. Migrate all 77 pids() callers and error-ignoring kill_services() stop/restart/cleanup chains. Prioritize VPN, USB modem, LAN/WAN teardown, flash shutdown, Samba and storage; keep BusyBox process inventory independent while coordinating its udhcpc PID-file writer.
7. Migrate in-tree PID-file writers and consumers as coordinated transactions. Identify lpdparent.pid writer before allowing any signal through it.
8. Commit one bounded atomic update directly to main using the verified expected head. Re-fetch before writing, preserve the archive untouched, then inspect exact-head push Actions and report L1-L4 separately.
