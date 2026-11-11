# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; latest branch inventory contains main only; no remote checkpoint branch.
- Base main SHA for THIS COMMIT: a3ee4d0e628434b1e16171f457afa0791c84f7a3 (parent 65119b857a7684974caf2ae0e13d8112a0e035bd). Resolve the resulting SHA from live main after publication.
- Bootstrap recovery completed in required order: AI_BOOTSTRAP_PROMPT.md, AI_DEVELOPMENT_PROTOCOL.md, live main ref, current handoff, roadmap, newest dev-log records, archive index, branch inventory and exact-head Actions. All 25 Actions runs on the recovered HEAD completed successfully.
- Root dev_log.md had 105 complete newest-first records before THIS COMMIT and has 106 afterward. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no rotation/index update is due.

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

## Closed task — BusyBox crond detached-daemon startup and restart safety (P2 reliability)

- Fix commit a3ee4d0e628434b1e16171f457afa0791c84f7a3; exact-head Crond lifecycle [42999257797] and AI handoff docs [42999257844] both SUCCESS. All 25 Actions runs for this SHA completed successfully; every job step and the Crond/docs full logs were inspected.
- GNU89 -Wall -Wextra -Werror ASan/UBSan regression printed `crond daemon startup and restart policy: PASS`; source checker printed `crond daemon startup, stop-before-start and RC call-path contracts: PASS`. Docs validator reported root log=105/119 and archive inventory 6/22/18.
- RC now propagates bounded stop status, refuses restart until the old scheduler is confirmed gone, verifies a live non-zombie detached daemon after the BusyBox launcher parent exits, and retains failed notification completion markers for retry. Live cron execution and target runtime remain unverified.

## Active task — BusyBox udhcpc event-script failure gating (P2 network reliability)

### Source and call-path evidence

- Production client: `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`; status contract: `trunk/user/busybox/busybox-1.24.x/libbb/vfork_daemon_rexec.c` and `include/libbb.h`; RC callbacks: `trunk/user/rc/src/net_wan.c`.
- `spawn_and_wait()` returns `wait4pid()` status, including normal nonzero exits and signal termination. `udhcp_run_script()` previously discarded that result. On DHCPACK, the client then moved to BOUND, refreshed its lease timer and published the ACK address regardless of whether the RC `bound`/`renew` callback had configured the interface, routes and resolver successfully.
- THIS COMMIT returns and logs event-script status, gates BOUND/address/renewed-timer publication on successful `bound`/`renew` execution, retries an initial selected DHCP request after the configured retry interval, and preserves an existing lease's original expiry deadline while charging failed renewal script runtime against it. If callback execution reaches/passes that deadline, the normal lease-expiry path runs instead of extending the lease.
- Added a GNU89 policy header and five-case regression for initial acquisition retry, renewal deadline preservation, callback-overrun expiry, exact-deadline expiry and already-expired leases; a source/caller contract checker verifies the actual BusyBox return semantics and gate ordering; a path-filtered dedicated Actions workflow runs both.

## Verification boundary

- L1 source review: PASS for BusyBox `spawn_and_wait()` return semantics, event-script result retention, ACK publication ordering and retry/lease-deadline handling; RC DHCP callbacks return their setup status to the BusyBox event script.
- Local policy reproduction: GNU89 -Wall -Wextra -Werror ASan/UBSan run across the five retry/deadline cases passed. The repository's committed dedicated test and source checker are pending exact-head Actions.
- Exact-head udhcpc script-status workflow and AI handoff docs validation are pending for THIS COMMIT. Inspect every step and full log before closing the task.
- Production MIPS/uClibc BusyBox object, full firmware/TRX, live IPv4 DHCP transitions, DHCPv6 behavior and L4 router operation remain NOT VERIFIED.

## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 fstatat() then unlinkat() is not inode-conditional; do not patch without a proved staged design preserving STOR/APPE/REST/STOU, parent/inode safety and filesystem behavior.
2. NFS async durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; require real client evidence.
3. Wireless bulk detach / bridge ownership across board variants — target validation open.
4. WAN PPP reconnect, callback recovery and IPv6 rollback — target behavior NOT VERIFIED.
5. Optware operation-lock / stop / unmount under BusyBox ash — target behavior NOT VERIFIED.
6. FULLCONENAT board/kernel/ABI coverage — prior object evidence is not a full firmware build.
7. All complete MIPS/uClibc builds, firmware/TRX inspection and physical-router flash/boot remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main SHA, parent and branch inventory; confirm THIS COMMIT is still the live head before relying on this handoff.
2. Query Actions for that exact SHA. Inspect every step and complete logs for the dedicated udhcpc event-script status workflow and AI handoff documentation contract, plus any other triggered WAN/HTTPD umbrella. Require the GNU89 ASan/UBSan policy binary, BusyBox `spawn_and_wait()` result contract, ACK state-publication gate, five retry/deadline cases and docs archive validator to pass on the same SHA.
3. If any exact-head job fails, inspect the full log and repair the implementation or contract; do not mark the task closed.
4. If green, close this DHCP callback slice and select the next under-covered module from the whole-repository ledger. Keep FTP upload TOCTOU, NFS client evidence, wireless/WAN target behavior and the separate infosvr SIGTERM issue visible; continue rotating across userland rather than returning to SkipDBv2 or HTTPD by default.

## Documentation and verification rules

- dev_log.md is newest-first. Rotate the oldest 100 complete records only when a new record would reach 120 or the file passes 384 KiB; 512 KiB is a hard cap. Preserve historical archives byte-for-byte.
- SESSION_HANDOFF.md and REFACTOR_ROADMAP.md are live single-copy files; update in place on material changes. Do not create routine snapshots.
- Do not create docs-only follow-ups solely to copy completed Actions results; exact-head Actions are the verification source of truth. Fix actual validator defects when found.
- L1 = source review; L2 = host tests/Actions; L3 = production MIPS/uClibc objects and complete firmware; L4 = real-router behavior. Never elevate evidence between levels.
