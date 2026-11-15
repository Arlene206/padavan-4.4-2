# Cross-session execution mandate

Resume from live Git state, then continue implementation. Do not repeat completed DNSMasq lock-order reviews or the RC C-source process-signal sweep below. Do not stop at a plan or wait for pending CI while independent work is available.

For every multi-file slice, publish source, regression/source contract and required documentation as one atomic main commit: create blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main with the expected parent SHA. Never create development branches or PRs, never force-push, and never touch the preserved archive branch. Inspect Actions only for the exact current main SHA.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent main SHA for the current slice: 6832b792e4304114555ba054cd91f34ff4868fa2 (fix(rc): report VPN server boot launch failures). Current slice result: THIS COMMIT; resolve the resulting SHA from live main during takeover.
- Preserved archive branch: backup/pre-plugin-overhaul-2026-11-14, expected HEAD 72f4f157a084f8184b9597ca5bf334b8080e5807. Read-only forever; never switch to, update, delete, or develop on it.
- PR #1 is closed and unmerged; it is not a development or validation target.
- No local checkout or local cross-toolchain is available in this execution. Do not claim local tests or builds.
- Prior exact-head Actions on parent 6832b792e4304114555ba054cd91f34ff4868fa2: 26 of 27 push runs completed SUCCESS. Networkmap #43729759374 and AI handoff #43729759472 passed. XY-C1 Firmware Validation #43729759471 is IN_PROGRESS; its only job is #132859395581, Cross-build XY-C1 firmware. No L3 result applies to the current slice until its exact SHA has a successful Build, TRX verification and artifact upload. Physical-router L4 remains NOT VERIFIED.

## Completed in this slice

### P1 process-lifecycle direct-call audit — RC C-source scope

- Read the current bootstrap and development protocol, handoff, roadmap, newest root dev-log records and archive index. Reconfirmed main and preserved backup state.
- Reviewed all 36 trunk/user/rc/src/*.c files for active pids(), kill_pidfile_s(), killall and direct kill() patterns. The only active pids() use is the compatibility wrapper is_dns_dhcpd_run() in services_ex.c; no RC C caller was found. The only direct killall invocation in the inspected RC C sources is inside #if 0 in auth.c. kill_process_pidfile() is an unused helper within the RC C-source scope. Identity-bound service signaling and deliberate PID 1 / child-process signaling remain unchanged.
- This is not a whole-repository or all-package audit. The Linux 4.4 PID/start-time validation-to-signal TOCTOU remains unresolved; another pre-signal recheck does not close it.

### Networkmap boot launch diagnostics

- start_services_once() previously discarded the status returned by start_networkmap(1); UNKNOWN process inventory and launcher failures could therefore pass without a boot-sequence diagnostic.
- Capture the return status and log nonzero results without changing best-effort boot continuation. Added a semantic assertion to scripts/check_networkmap_signals.py. Existing Networkmap push path filters already include services.c and the checker; XY-C1 already watches services.c.
- Parent: 6832b792e4304114555ba054cd91f34ff4868fa2; resulting commit: THIS COMMIT. Exact-head Networkmap and AI docs checks for the new SHA must be confirmed. No local test, target build or hardware result is claimed.

## Active next task

### P1 — Complete boot startup status accounting

start_services_once() still ignores several int-returning start functions. Continue one call at a time, after checking each return contract and whether nonzero means disabled rather than failure. First inspect start_infosvr(), start_watchdog(), and start_gpio_btn(); start_infosvr() explicitly returns 1 when adsc_enable is disabled, so logging any nonzero status without a configuration guard would create false alarms. Check other start functions that log internally before adding duplicate messages. Preserve best-effort boot continuation.

For the first concrete unlogged failure, add status capture plus a useful diagnostic and extend the most relevant existing workflow/source contract. Keep XY-C1 triggers correct for changed product source. Do not bundle unrelated startup services into a broad rewrite.

## Other known boundaries

- DNSMasq/VPN process-associated lock-order review is complete; do not repeat it. Linux 4.4 PID/start-time check-to-kill TOCTOU remains a known limitation.
- FTP failed-upload unlink TOCTOU remains blocked because inode-conditional unlink-by-fd is unavailable; preserve STOR/APPE/REST/STOU semantics.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.
- Do not present host checks as MIPS L3 or physical-router L4. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and resolve the current slice commit SHA. Check exact-head push Actions for Networkmap signal stability, AI handoff docs and XY-C1. If a relevant run fails, inspect the first causal failure and fix it. If XY-C1 is pending, continue independently rather than waiting.
2. Resume the boot-status audit at start_infosvr(), then start_watchdog() / start_gpio_btn(). Confirm disabled/no-op return values, internal diagnostics and compile guards before implementing any logging change.
3. Keep all development on main, use one Git tree/commit for each logical multi-file slice, preserve the frozen backup, and update this handoff only when the exact task or validation checkpoint materially changes.

Preserve docs/dev-log/ archive shards and docs/dev-log/INDEX.md byte-for-byte.
