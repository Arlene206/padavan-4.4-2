# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; only remote branch observed before THIS COMMIT is main; no checkpoint branch.
- Base main SHA for THIS COMMIT: 65119b857a7684974caf2ae0e13d8112a0e035bd (parent 947ee69cfbda917ec1728cdde39a00b6a7726eeb). Resolve the resulting SHA from live main after publication.
- Bootstrap recovery completed in required order: AI_BOOTSTRAP_PROMPT.md, AI_DEVELOPMENT_PROTOCOL.md, live main ref, current handoff, roadmap, newest dev-log records, archive index, branch inventory and exact-head Actions.
- Root dev_log.md had 104 complete newest-first records before THIS COMMIT and has 105 afterward. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no rotation/index update is due.

## Closed task — NVRAM HDR2 profile-save atomic publication

- Commit ce7efad34dbd81097d923f9568f0ae9946694b58; exact-head HTTPD [42992756073] and AI docs [42992755926] both SUCCESS. All 39 HTTPD job steps passed.
- Same-directory mode-0600 staging, complete short-write/EINTR handling, file fsync, checked close and atomic rename preserve the last-good profile on pre-publication failure. Parent-directory fsync/directory-entry power-loss durability are not claimed.
- Production MIPS/uClibc object, complete firmware/TRX and L4 remain NOT VERIFIED.

## Closed task — networkmap signal-context safety

- Production fix a73ca2221712f16dd5aa4bab450eed18a6cd3f92; handoff/source-contract correction 8ec8198c4564e57cd3956ea675e343bc35098e72.
- Networkmap [42996232222] and AI docs [42996232115] both SUCCESS on exact SHA 8ec8198c4564e57cd3956ea675e343bc35098e72; all job steps/logs inspected. GNU89 ASan/UBSan policy test and signal/install/lifecycle/build contract both PASS. Docs reported root log=102/119 and archives=6/22/18.
- SIGUSR1/SIGTERM handlers only record sig_atomic_t events; scan state and resource cleanup run in normal context. Production MIPS/uClibc, firmware/TRX and L4 LAN discovery remain NOT VERIFIED.

## Closed task — ASUS infosvr UDP packet initialization and bounds

- Packet fix commit 947ee69cfbda917ec1728cdde39a00b6a7726eeb; validator/source-contract correction 65119b857a7684974caf2ae0e13d8112a0e035bd.
- Exact-head infosvr packet workflow [42997959662] and AI docs [42997959722] both SUCCESS on 65119b857a7684974caf2ae0e13d8112a0e035bd; every job step and full log inspected.
- GNU89 ASan/UBSan test printed infosvr packet initialization and length policy: PASS; source checker printed infosvr packet receive and bounds source contracts: PASS; docs validator printed root log=104/119, batch=100, retain=20 and archive counts 6/22/18.
- Receive-buffer initialization, actual UDP byte count, base/extended prefix checks and malformed local MAC fail-closed behavior are CLOSED at L2. Infosvr MIPS/uClibc object, firmware/TRX and live packet interoperability remain NOT VERIFIED.
- Separate P2 follow-up remains open: infosvr catch_sig_infosvr(SIGTERM) calls resource cleanup and exit() inside signal context. It has not been changed by the packet parsing fix.

## Active task — BusyBox crond detached-daemon startup and restart safety (P2 reliability)

### Source and call-path evidence

- RC lifecycle: trunk/user/rc/src/services.c, trunk/user/rc/src/rc.h and trunk/user/rc/src/rc.c; shared stop gate: trunk/user/rc/src/common_ex.c and service_lifecycle_policy.h.
- BusyBox implementation: trunk/user/busybox/busybox-1.24.x/miscutils/crond.c; daemonizer: trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c; process inventory: trunk/user/shared/src/pids.c.
- In RC, start_crond() called _eval() and treated its status as scheduler readiness. _eval() forks and waits for its direct child. BusyBox crond then daemonizes: its original parent exits successfully immediately after fork_or_rexec(); the detached child only afterward runs setsid(), changes to the crontab directory, rescans jobs and writes /var/run/crond.pid. A child-side startup failure after parent exit is therefore invisible to _eval().
- stop_crond() discarded the result from kill_services(); restart_crond() always started another scheduler. A process surviving the bounded TERM/KILL stop could overlap the replacement and run cron jobs twice.
- THIS COMMIT returns stop status, refuses to restart unless the old scheduler is confirmed gone, and adds an initial grace plus bounded checks for a live non-zombie pids("crond") process after launcher exit. It preserves the existing disabled-service return of 1. RC notification callers propagate negative restart failures so the completion marker is retained for retry; initial crontab regeneration logs a failed restart.
- A GNU89 policy header/test, source/call-path checker and dedicated path-filtered workflow cover the RC integration, bounded lifecycle helper, /proc matching and BusyBox parent/child initialization order.

## Verification boundary

- Local host regression: cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer; run with ASAN_OPTIONS=detect_leaks=1: PASS, output crond daemon startup and restart policy: PASS.
- L1 source review: PASS for _eval() wait semantics, BusyBox parent/child order, non-zombie process postcondition, stop-before-start gate and RC notification error propagation.
- Exact-head Crond workflow, RC-related umbrella workflows and AI docs validation are pending for the resulting commit. Inspect every job/step and full log; do not close L2 before exact-SHA checks are green.
- Production MIPS/uClibc RC object, full firmware/TRX, actual cron job scheduling and L4 router behavior remain NOT VERIFIED.

## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 fstatat() then unlinkat() is not inode-conditional; do not patch without a proved staged design preserving STOR/APPE/REST/STOU, parent/inode safety and filesystem behavior.
2. NFS async durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; require real client evidence.
3. Wireless bulk detach / bridge ownership across board variants — target validation open.
4. WAN PPP reconnect, callback recovery and IPv6 rollback — target behavior NOT VERIFIED.
5. Optware operation-lock / stop / unmount under BusyBox ash — target behavior NOT VERIFIED.
6. FULLCONENAT board/kernel/ABI coverage — prior object evidence is not a full firmware build.
7. All complete MIPS/uClibc builds, firmware/TRX inspection and physical-router flash/boot remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main SHA, parent and branch inventory. Confirm THIS COMMIT is still the live head before relying on this handoff.
2. Query Actions for that exact SHA. Inspect every step and complete logs for Crond daemon lifecycle stability regression, AI handoff documentation contract, and any other workflow triggered by the RC paths. Require the GNU89 ASan/UBSan policy binary, detached daemon/stop-before-start/caller contract and docs archive validator to pass on the same SHA.
3. If any exact-head job fails, inspect the full log and repair the implementation or contract; do not mark the task closed.
4. If green, close this crond slice and choose the next under-covered module from the whole-repository ledger. Keep FTP TOCTOU, NFS client evidence and the separate infosvr SIGTERM issue visible; avoid immediate returns to NVRAM, HTTPD, SkipDBv2, networkmap or infosvr packet parsing.

## Documentation and verification rules

- dev_log.md is newest-first. Rotate the oldest 100 complete records only when a new record would reach 120 or the file passes 384 KiB; 512 KiB is a hard cap. Preserve historical archives byte-for-byte.
- SESSION_HANDOFF.md and REFACTOR_ROADMAP.md are live single-copy files; update in place on material changes. Do not create routine snapshots.
- Do not create docs-only follow-ups solely to copy completed Actions results; exact-head Actions are the verification source of truth. Fix actual validator defects when found.
- L1 = source review; L2 = host tests/Actions; L3 = production MIPS/uClibc objects and complete firmware; L4 = real-router behavior. Never elevate evidence between levels.
