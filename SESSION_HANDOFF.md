# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; latest branch inventory contains main only; no remote checkpoint branch.
- Base main SHA for THIS COMMIT: f986ed961bbe14a8db4bc8d99219e5a1a1330826 (parent 17b4583ce0511a3cbbd02cef18024fe7b30110e7). Resolve the resulting SHA from live main after publication.
- Bootstrap recovery completed in required order: AI_BOOTSTRAP_PROMPT.md, AI_DEVELOPMENT_PROTOCOL.md, live main ref, current handoff, roadmap, newest dev-log records, archive index, branch inventory and exact-head Actions. Current main f986ed961bbe14a8db4bc8d99219e5a1a1330826 has only the main branch and no remote checkpoint; AI docs [43004194407] and infosvr packet stability [43004194313] both completed SUCCESS on that exact SHA. Every job step and complete log was inspected.
- Root dev_log.md had 107 complete newest-first records before THIS COMMIT and has 108 afterward. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no rotation/index update is due.

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

## Closed task — BusyBox udhcpc event-script failure gating (P2 network reliability)

- Fix commit 17b4583ce0511a3cbbd02cef18024fe7b30110e7; dedicated udhcpc workflow [43002980464], the path-triggered infosvr packet regression [43002980467] and AI handoff documentation validation [43002980526] all SUCCESS on that exact SHA. All jobs and steps completed successfully; complete logs were inspected.
- Dedicated GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer policy regression printed "udhcpc script failure retry policy: PASS"; source checker printed "udhcpc event-script status and lease-state source contracts: PASS".
- AI docs validator printed "AI handoff documentation check: PASS (root log=106/119 entries, batch=100, retain=20, legacy/current archives: dev-log=6, handoff=22, roadmap=18)".
- BusyBox now retains/logs event-script exit status and gates DHCPACK BOUND/address/lease-timer publication on successful bound/renew; initial acquisition retries after callback failure, while failed renewals retain the old expiry deadline and charge script runtime against it.
- L2 host/Actions verification is CLOSED for this slice. Production MIPS/uClibc BusyBox object, complete firmware/TRX, live DHCP transitions, DHCPv6 and L4 router operation remain NOT VERIFIED. Other leasefail/NAK/deconfig/release event-state semantics remain open.

## Closed task slice — storage-backed daemon stop-status propagation (P2 reliability)

- Actual firmware path was traced: trunk/user/optware/Makefile installs opt-start/opt-mount/opt-umount but not K95aria2/K95transmission; the in-tree firmware Aria/Transmission integration is separate, through RC service commands and /usr/bin/aria.sh plus /usr/bin/transmission.sh. scripts/Makefile copies scripts/files into ROMFS, including /sbin/optware_lifecycle_lock.sh.
- Found a fail-open eject gap: stop_stor_apps() propagated NFS, Samba and FTP failures but ignored MiniDLNA, Firefly, Transmission and Aria stop outcomes. Their wrappers discarded kill_services()/eval() status and did not verify daemon absence.
- THIS COMMIT changes those four wrappers to return failure unless the stop command succeeded and the daemon is no longer observable. stop_stor_apps() now aborts eject on any failure, and manual restart callers refuse to relaunch over a daemon that did not stop. storage_service_stop_policy.h and a five-case GNU89 regression cover this postcondition.
- Local GNU89 -Wall -Wextra -Werror ASan/UBSan policy regression passed. Exact-head storage Actions are pending publication; no matching target BusyBox ash, MIPS/uClibc object, full firmware/TRX or L4 behavior is claimed.

## Active task — Optware/storage teardown under BusyBox ash (P2 reliability)

### Remaining scope and verified caller facts

- Removable-storage safe removal owns storage.lock before stop_stor_apps() and the checked opt-umount hook; opt-umount acquires the separate Optware lifecycle lock nonblocking, preserving the lock order. MTD/RWFS teardown checks opt-umount status before filesystem unmount and UBI detach.
- K95 scripts are source-side optional hooks, not the in-tree firmware dispatcher path. Keep them distinct from built-in Aria/Transmission RC integration.
- opt-umount.sh attempts swapoff /opt/.swap but does not propagate failure before home/localtime cleanup and /opt unmount. The later checked umount prevents falsely reporting success for a still-busy /opt mount, but active swap verification and early failure gating remain open.
- BusyBox ash lock acquire/release, concurrent start/stop, descriptor inheritance and target filesystem behavior have not been exercised using a matching target shell. Host sh results are not target ash evidence.

### Exact next investigation

1. Fetch live main SHA, parent and branch inventory, then inspect exact-head Actions before editing.
2. Add a fail-closed swapoff policy for /opt/.swap: distinguish an inactive swap file from an active entry in /proc/swaps, propagate unreadable inventory and swapoff failure, and prevent home/localtime cleanup plus /opt unmount whenever release is uncertain. Add active/inactive/failure host regressions.
3. Re-run the whole Optware/storage caller graph and lock-order review. Use BusyBox ash only if a matching BusyBox implementation is actually available; otherwise leave target ash evidence explicitly NOT VERIFIED.
4. Keep FTP upload-cleanup TOCTOU OPEN/BLOCKED, NFS client semantics, wireless/WAN target behavior and the separate infosvr SIGTERM cleanup-in-handler risk visible in the global ledger. Continue rotating across userland modules rather than defaulting back to SkipDBv2 or HTTPD.

## Verification boundary

- L1 source review: PASS for BusyBox udhcpc event-status/lease-publication gating and storage-backed daemon stop command/process-absence gating for MiniDLNA, Firefly, Transmission and Aria.
- L2 exact-head: udhcpc [43002980464], infosvr packet regression [43002980467] and AI docs [43002980526] all SUCCESS on 17b4583ce0511a3cbbd02cef18024fe7b30110e7. On current base f986ed961bbe14a8db4bc8d99219e5a1a1330826, infosvr [43004194313] and AI docs [43004194407] also SUCCESS. The new storage daemon stop policy test passed locally under GNU89 ASan/UBSan; exact-head storage Actions for THIS COMMIT are pending.
- Production MIPS/uClibc objects, complete firmware/TRX, live DHCP v4/v6 behavior, matching target BusyBox ash and L4 router behavior remain NOT VERIFIED.

## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 fstatat() then unlinkat() is not inode-conditional; do not patch without a proved staged design preserving STOR/APPE/REST/STOU, parent/inode safety and filesystem behavior.
2. NFS async durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; require real client evidence.
3. Wireless bulk detach / bridge ownership across board variants — target validation open.
4. WAN PPP reconnect, callback recovery and IPv6 rollback — target behavior NOT VERIFIED.
5. Optware operation-lock / stop / unmount under BusyBox ash — target behavior NOT VERIFIED.
6. FULLCONENAT board/kernel/ABI coverage — prior object evidence is not a full firmware build.
7. All complete MIPS/uClibc builds, firmware/TRX inspection and physical-router flash/boot remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main SHA, parent and branch inventory; confirm the documentation closeout commit for this session is still the exact main head.
2. Start the Optware/storage lifecycle investigation. Resolve the real ROMFS/install paths and in-firmware entrypoints for K95 Optware, Aria2/Transmission, hotplug and storage teardown; do not rely on unverified K95 caller assumptions.
3. Trace lock ownership, descriptor lifetime, start/stop races, partial-start rollback, swapoff and /opt unmount failure propagation across the complete active caller graph. Add focused host-side regression coverage and use BusyBox ash only when the tested shell is demonstrably the matching implementation.
4. Fix a proven defect in a focused code/test commit. Keep FTP upload TOCTOU OPEN/BLOCKED, NFS client compatibility evidence, wireless and WAN target behavior, and the separate infosvr SIGTERM cleanup-in-handler issue visible in the whole-repository ledger. Do not drift back to SkipDBv2 or HTTPD by default.

## Documentation and verification rules

- dev_log.md is newest-first. Rotate the oldest 100 complete records only when a new record would reach 120 or the file passes 384 KiB; 512 KiB is a hard cap. Preserve historical archives byte-for-byte.
- SESSION_HANDOFF.md and REFACTOR_ROADMAP.md are live single-copy files; update in place on material changes. Do not create routine snapshots.
- Do not create docs-only follow-ups solely to copy completed Actions results; exact-head Actions are the verification source of truth. Fix actual validator defects when found.
- L1 = source review; L2 = host tests/Actions; L3 = production MIPS/uClibc objects and complete firmware; L4 = real-router behavior. Never elevate evidence between levels.
