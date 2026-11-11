# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; latest branch inventory contains main only; no remote checkpoint branch.
- Base main SHA for THIS COMMIT: 17b4583ce0511a3cbbd02cef18024fe7b30110e7 (parent a3ee4d0e628434b1e16171f457afa0791c84f7a3). Resolve the resulting SHA from live main after publication.
- Bootstrap recovery completed in required order: AI_BOOTSTRAP_PROMPT.md, AI_DEVELOPMENT_PROTOCOL.md, live main ref, current handoff, roadmap, newest dev-log records, archive index, branch inventory and exact-head Actions. The udhcpc, infosvr packet and AI docs workflows on 17b4583ce0511a3cbbd02cef18024fe7b30110e7 all completed successfully; every step and full log was inspected.
- Root dev_log.md had 106 complete newest-first records before THIS COMMIT and has 107 afterward. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no rotation/index update is due.

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

## Active task — Optware/storage lifecycle under BusyBox ash (P2 reliability)

### Scope and known repository facts

- The storage governance ledger records full partition/ejected inventory staging, swapoff-before-unmount gates, an operation-wide storage lock, Optware startup supervision/rollback and checked removable-storage teardown. Recent changes to K95 Optware hooks preserve full hook pathnames and reverse startup order.
- Existing source review records that K95 Optware hook files are not ROMFS-installed or invoked by the in-tree S??* lifecycle dispatcher; firmware Aria2/Transmission integration uses separate RC scripts. Do not assume K95 is an active production caller without proving its install path.
- Target-specific behavior remains unverified: BusyBox ash lock acquisition/release, concurrent start/stop, partial Optware startup rollback, service-stop failure propagation, swapoff and /opt unmount ordering.

### Next investigation

- Trace the actual firmware installation and invocation paths for K95 hooks, Aria2/Transmission RC scripts, storage hotplug handlers and the shared operation lock. Identify which callers actually own the lock and which teardown path can reach umount /opt.
- Review complete startup/stop/rollback call graphs, pathname handling, lock descriptor lifetime and all failure returns before editing. Reconcile the prior "not ROMFS-installed" note against any changed packaging or script installation rules.
- Add focused host-side shell/source regressions for the actual active path; use BusyBox ash execution only when a matching host/target shell is available. Do not report host POSIX-shell checks as target BusyBox ash evidence.
- Fix a concrete lifecycle defect if one is demonstrated; otherwise document the exact missing target evidence without manufacturing a source change.

## Verification boundary

- L1 source review: PASS for the BusyBox spawn_and_wait() event result, DHCPACK lease-state publication gate, initial retry and renewal deadline accounting.
- L2 exact-head: udhcpc [43002980464], infosvr packet regression [43002980467] and AI docs [43002980526] all SUCCESS on exact SHA 17b4583ce0511a3cbbd02cef18024fe7b30110e7. The udhcpc GNU89 ASan/UBSan policy test and source checker passed; docs archive validator passed with root log=106/119 and archive inventory 6/22/18. Every step and complete log was inspected.
- Production MIPS/uClibc BusyBox object, complete firmware/TRX, live DHCP v4/v6 behavior and L4 router behavior remain NOT VERIFIED.
- Optware/ash next task is at investigation stage. No target BusyBox ash or router filesystem behavior is claimed.

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
