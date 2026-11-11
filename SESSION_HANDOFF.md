# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; latest branch inventory contains main only; no remote checkpoint branch.
- Base main SHA for THIS COMMIT: 14e22191f48c9ad6f3804729ab502ac81670a30b (parent 2968bb70c97e13f405389795628184f804abcf5d). Resolve the resulting SHA from live main after publication.
- Bootstrap recovery completed in required order: AI_BOOTSTRAP_PROMPT.md, AI_DEVELOPMENT_PROTOCOL.md, live main ref, current handoff, roadmap, newest 20 complete dev-log records, archive index, branch inventory and exact-head Actions. Live main 14e22191f48c9ad6f3804729ab502ac81670a30b has only the main branch and no remote checkpoint. On that SHA, storage [43017000972] failed only at the whole-caller source contract's stale literal swapoff lookup; its Optware syntax and teardown regression steps passed. Infosvr [43017000995] and AI docs [43017000981] both completed SUCCESS; complete job logs and every step were inspected.
- Root dev_log.md had 109 complete newest-first records before THIS COMMIT and has 110 afterward. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no rotation/index update is due.

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
- The GNU89 -Wall -Wextra -Werror ASan/UBSan policy regression passed before publication. The workflow command-continuation correction is on main SHA 2968bb70c97e13f405389795628184f804abcf5d; storage [43009772930] and AI docs [43009772871] both completed SUCCESS on that exact SHA. Every job step passed. No matching target BusyBox ash, MIPS/uClibc object, full firmware/TRX or L4 behavior is claimed.

## Active task — Optware/storage teardown under BusyBox ash (P2 reliability)

### Completed slice — fail-closed /opt swap deactivation

- Active firmware paths were confirmed: `trunk/user/optware/Makefile` installs `opt-umount.sh`; `trunk/user/scripts/Makefile` publishes the shared lifecycle helper consumed at `/sbin/optware_lifecycle_lock.sh`. The Optware teardown hook is called from storage and RWFS removal paths that check its exit status.
- Finding: `opt-umount.sh` called `swapoff /opt/.swap` but ignored failure and proceeded to rewrite `/home/admin`, remove `/etc/localtime`, and attempt `/opt` unmount. File existence was used as a proxy for activation, and no postcondition proved the swap entry had disappeared.
- THIS COMMIT adds an exact-entry `/proc/swaps` reader to the shared lifecycle helper. Return states distinguish active, inactive, and malformed/unreadable inventory; the header and complete records are validated, including read and close errors. For an exact active `/opt/.swap` entry, teardown requires a successful `swapoff` command and a second complete inventory proving that the entry disappeared. A still-active entry, command failure, read failure or close failure blocks all home/localtime cleanup and `/opt` unmount; the caller releases lifecycle fd 9 and returns failure. Child awk, swapoff and logger processes close fd 9.
- `optware_lifecycle_teardown.sh` adds host shell cases for inactive and prefix-only entries (no command), successful deactivation, command failure, a still-active postcondition, missing inventory, malformed inventory and lock-fd inheritance. The storage workflow adds shell syntax checks and a source-order/fail-closed contract. On exact SHA 14e22191f48c9ad6f3804729ab502ac81670a30b, host Optware syntax and teardown regression steps passed, but the storage job's final caller-graph check failed because an older assertion still searched for literal `swapoff /opt/.swap` after the implementation moved the operation behind `optware_lifecycle_swapoff_file()`. THIS COMMIT updates that assertion to the explicit fail-closed helper call; exact-head rerun is pending. Matching target BusyBox ash behavior remains NOT VERIFIED.

### Remaining active scope

- Prove BusyBox ash lock acquire/release, descriptor inheritance, concurrent start/stop and command semantics using the matching BusyBox implementation only if it is available. Host `sh` regressions are not target ash evidence.
- Re-run the complete Optware/storage caller graph and lock-order review after the swapoff gate. Keep MTD/RWFS teardown, removable-storage ejection and daemon stop status linked to the hook's exit result.
- Keep FTP upload-cleanup TOCTOU OPEN/BLOCKED, NFS client semantics evidence-gated, wireless/WAN target behavior NOT VERIFIED, and the separate infosvr SIGTERM cleanup-in-handler issue visible in the whole-repository ledger.

### Exact next investigation

1. Fetch the exact live main SHA and inspect storage, infosvr and AI docs Actions for THIS COMMIT; require the swap inventory regression, updated caller-graph contract and docs validation to pass on that SHA.
2. Locate the exact BusyBox version/configuration used by the firmware build and determine whether a corresponding `ash` executable can be run in the development environment. If not available, leave target ash behavior NOT VERIFIED rather than substituting host `sh`.
3. Continue the actual in-firmware teardown graph: inspect opt-start/opt-mount/opt-umount entrypoints, startup rollback, MTD worker shutdown, storage lock ordering, fd 9 inheritance and every caller's failure gate. Fix only a demonstrated remaining defect.
4. Once this active storage slice is closed or explicitly blocked by unavailable target evidence, rotate to the highest-priority unfinished module in the global roadmap; do not default back to SkipDBv2 or HTTPD.

## Verification boundary

- L1 source review: PASS for BusyBox udhcpc event-status/lease-publication gating, storage-backed daemon stop command/process-absence gating for MiniDLNA/Firefly/Transmission/Aria, and the Optware exact-/proc/swaps gate before home/localtime cleanup and /opt unmount.
- L2 exact-head: udhcpc [43002980464], infosvr packet regression [43002980467] and AI docs [43002980526] all SUCCESS on 17b4583ce0511a3cbbd02cef18024fe7b30110e7. Storage [43009772930] and AI docs [43009772871] passed on base 2968bb70c97e13f405389795628184f804abcf5d. On 14e22191f48c9ad6f3804729ab502ac81670a30b, Optware syntax and teardown regression steps passed; the final caller-graph static step failed only on a stale literal swapoff assertion. Infosvr [43017000995] and AI docs [43017000981] passed. THIS COMMIT corrects the assertion; exact-head rerun pending. Host `sh` is not matching target ash evidence.
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
1. Fetch live main SHA, parent and branch inventory; inspect storage-hotplug, infosvr and AI docs runs for the exact SHA, including every failed/passed step and complete log.
2. Confirm the corrected caller-graph assertion now checks the explicit swapoff helper gate and that the host regression remains green.
3. Locate the exact firmware BusyBox version/configuration and establish whether matching ash can be executed. Do not equate host `sh` with target ash.
4. Continue the Optware/storage fd 9, concurrent startup/teardown and caller graph audit; after closing or evidence-blocking it, choose the next highest unfinished governance item.


## Documentation and verification rules

- dev_log.md is newest-first. Rotate the oldest 100 complete records only when a new record would reach 120 or the file passes 384 KiB; 512 KiB is a hard cap. Preserve historical archives byte-for-byte.
- SESSION_HANDOFF.md and REFACTOR_ROADMAP.md are live single-copy files; update in place on material changes. Do not create routine snapshots.
- Do not create docs-only follow-ups solely to copy completed Actions results; exact-head Actions are the verification source of truth. Fix actual validator defects when found.
- L1 = source review; L2 = host tests/Actions; L3 = production MIPS/uClibc objects and complete firmware; L4 = real-router behavior. Never elevate evidence between levels.
