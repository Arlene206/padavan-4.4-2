# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; live branch inventory contains main only; no remote checkpoint branch.
- Base main SHA for THIS COMMIT: `42800e0caaf90e0230f4e30d268e8301d0da2d70` (parent `02907c582c716a49c6255d2c06cbad429f5a3260`). The live ref and exact commit ancestry were rechecked before editing; verify the new SHA after publication.
- Exact-head Actions on `42800e0caaf90e0230f4e30d268e8301d0da2d70`: FULLCONENAT [43052866763] and infosvr packet/signal lifecycle [43052866751] completed SUCCESS. HTTPD umbrella [43052866810] completed FAILURE at the Python source checker because its new path-filter assertion counted the assertion's own quoted path literal (expected 2, actual 3); the new descriptor-inheritance GNU89 ASan/UBSan binary itself printed PASS, as did the earlier policy binaries in that step. AI docs [43052866839] was still in progress when this correction was prepared. The full HTTPD log was inspected.
- THIS COMMIT corrects the workflow contract to inspect push and pull_request path blocks independently, avoiding self-counting. The descriptor-bound production writer and host regression are unchanged from `42800e0caaf90e0230f4e30d268e8301d0da2d70`.
- Root dev_log.md has 117 complete newest-first records before THIS COMMIT and will have 118 afterward, below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no rotation/index update is due.

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

## Closed task — Optware/storage teardown under BusyBox ash (P2 reliability; L1/L2 complete, target runtime blocked)

### Completed slice — fail-closed /opt swap deactivation and caller-graph closure

- The firmware path and caller graph were traced across `hotplug_stor.c`, `automount.sh`, `opt-mount.sh`, `opt-start.sh`, `opt-umount.sh`, the shared lifecycle helper, RC deferred startup and safe-removal callers. Startup hooks run outside `storage.lock`; ejection acquires `storage.lock` before the nonblocking Optware lifecycle lock, avoiding lock inversion. Removable Optware startup is deferred until after `storage.lock` is released; MTD startup is serialized by the lifecycle lock and has rollback. Stop-hook failure blocks cleanup/unmount and is propagated by storage/RWFS callers.
- The P2 finding was that `opt-umount.sh` ignored `swapoff /opt/.swap` failure and proceeded with home/localtime cleanup and /opt unmount. The shared helper now validates the exact active entry in a complete `/proc/swaps` inventory, checks swapoff status, re-reads inventory and requires the entry to disappear. Unknown/malformed inventory, command failure or a still-active entry blocks all later cleanup. Child awk, swapoff and logger processes close lifecycle fd 9.
- Host shell teardown regression covers inactive and prefix-only entries, successful deactivation, command failure, a still-active postcondition, missing/malformed inventory and lock-fd inheritance. The storage workflow includes shell syntax, teardown regression and complete caller-graph/lock-order contracts.
- Exact-head L2 closure: storage run [43017662766], infosvr packet run [43017662767] and AI docs run [43017662804] all SUCCESS on base SHA `4ec9d637ca7daaff93296c494467b6e88c528311`; every job step and complete logs were inspected. The earlier stale literal assertion is corrected and the final storage caller-graph contract now passes.

### Target-runtime verification boundary

- The firmware's BusyBox source is 1.24.2 (`trunk/user/busybox/busybox-1.24.x/Makefile`, version 1.24.2). The target configuration `trunk/configs/boards/busybox.config` enables `CONFIG_ASH`, `CONFIG_ASH_BASH_COMPAT`, `CONFIG_FEATURE_SH_IS_ASH`, `CONFIG_AWK`, `CONFIG_FLOCK`, `CONFIG_SWAPONOFF`, `CONFIG_MOUNT` and `CONFIG_UMOUNT`; HUSH is disabled. The matching 1.24.2 executable is not present in this development environment. Its installed BusyBox 1.37.0 is not target evidence and was not substituted.
- Therefore Optware lock/fd/concurrency semantics under the actual BusyBox ash 1.24.2, production MIPS/uClibc objects, complete firmware/TRX and L4 router behavior remain NOT VERIFIED. Do not claim target ash completion from host `sh` regressions.
- No additional L1 storage defect was proven after the end-to-end caller/lock/failure audit. This slice is closed at source review and exact-head host/Actions level, with target ash explicitly evidence-blocked.

## Closed task — ASUS infosvr SIGTERM signal lifecycle (P2 reliability)

### Finding and fix

- \`catch_sig_infosvr(SIGTERM)\` previously called \`res_cleanup()\` and \`exit(0)\` inside signal context. Socket close, PID-file removal and stdio/exit processing are not async-signal-safe and could re-enter interrupted libc state.
- A flag-only handler alone would leave a check-to-block race because the old event loop waited indefinitely in \`select()\`, while \`read_packet()\` retried every \`EINTR\` result.
- Commit \`2c769eac12cfdcb597644ebcf9c65b51423c8e4d\` makes the handler a single \`volatile sig_atomic_t\` store, installs checked \`sigaction()\` with \`SA_RESTART\` disabled, bounds \`select()\`, \`recvfrom()\` and \`sendto()\` waits to one second, stops receive/send \`EINTR\` retries after shutdown, and leaves descriptor/PID cleanup in the main loop. Descriptor cleanup correctly accepts fd 0.

### Exact-head verification and boundary

- Infosvr workflow [43024714802] and AI handoff docs [43024714674] both completed SUCCESS on exact main SHA \`2c769eac12cfdcb597644ebcf9c65b51423c8e4d\`; every job step and complete log was inspected.
- GNU89 \`-Wall -Wextra -Werror -fsanitize=address,undefined\` packet and signal policy regressions both printed PASS; \`scripts/check_infosvr_packets.py\` printed \`infosvr packet bounds and SIGTERM signal-lifecycle source contracts: PASS\`.
- AI docs validator printed \`AI handoff documentation check: PASS (root log=111/119 entries, batch=100, retain=20, legacy/current archives: dev-log=6, handoff=22, roadmap=18)\`.
- This closes the bounded source/L2 slice only. Production MIPS/uClibc object, complete firmware/TRX, target daemon signal timing and L4 router behavior remain NOT VERIFIED.

## Closed task — Optware lifecycle validation under firmware-matched BusyBox ash 1.24.2 (P2; L1/L2 complete, target L3/L4 open)

- The previously active task is now closed at L1/L2 on exact main SHA `02907c582c716a49c6255d2c06cbad429f5a3260`. Storage workflow [43042263807] completed SUCCESS; all 15 job steps passed and the complete log was inspected.
- The workflow built and linked the repository's BusyBox 1.24.2 source/config, exercised direct ash/awk/flock behavior and production-script ash syntax, then passed the lifecycle lock contention/release, teardown status propagation, startup supervision/rollback, ejected-device inventory, deferred Optware child-result/retry, storage daemon stop postcondition and whole caller-graph/lock-order checks (steps 9–13). The earlier unsupported fractional `sleep 0.1` was replaced by the BusyBox-compatible bounded integer poll in commit `02907c582c716a49c6255d2c06cbad429f5a3260`.
- Infosvr [43042263817] and AI handoff docs [43042263966] also completed SUCCESS on that exact SHA. The infosvr packet and SIGTERM deferral regressions/source contracts passed; documentation validation reported root log=116/119 and archives dev-log=6, handoff=22, roadmap=18.
- This closes the bounded host/Actions slice only. Production MIPS/uClibc BusyBox, matching target router ash timing, complete firmware/TRX and L4 hotplug/device behavior remain NOT VERIFIED. NFS client semantics remain evidence-blocked.

## Active task — Verify descriptor-bound core firewall snapshot writing (P2 robustness/security)

### Production finding and fix

- Commit `42800e0caaf90e0230f4e30d268e8301d0da2d70` keeps the mode-0600 `mkstemp()` descriptor open across `doSystem()`, clears `FD_CLOEXEC` explicitly, and directs `iptables-save` stdout to the inherited descriptor (`>&fd`) instead of reopening the shared-`/tmp` pathname. The descriptor is checked on close; the pathname remains for later `iptables-restore` and transaction cleanup.
- New `tests/firewall_snapshot_fd.c` creates a 0600 snapshot, unlinks its pathname, then uses a shell subprocess to write through the inherited descriptor and verifies bytes on the still-open inode. The exact GNU89 ASan/UBSan binary passed on run [43052866810].

### Exact-head CI finding and correction in THIS COMMIT

- HTTPD umbrella [43052866810] on exact SHA `42800e0caaf90e0230f4e30d268e8301d0da2d70` failed only in the source-contract Python checker. The new assertion expected the quoted test pathname twice, but the workflow text also contained the assertion's own quoted literal, making the actual count three. The test binary printed `firewall snapshot descriptor writer inheritance: PASS`; the transaction/status/startup policy binaries preceding the checker also ran without reported failure.
- THIS COMMIT replaces the self-referential count with independent assertions that the test is listed exactly once in the push path block and once in the pull_request block, and that the firewall CI step references the test source. Production code and the C regression are unchanged.
- FULLCONENAT [43052866763] and infosvr [43052866751] completed SUCCESS on `42800e0caaf90e0230f4e30d268e8301d0da2d70`; AI docs [43052866839] was still in progress when the correction was prepared.

### Exact next action

- Fetch the resulting live main SHA and verify ancestry from `42800e0caaf90e0230f4e30d268e8301d0da2d70`. Inspect the retriggered HTTPD Authentication and Security Regression and AI handoff documentation workflows on that exact SHA, every job step and complete logs.
- Require the descriptor-writer GNU89 ASan/UBSan regression, independent push/pull path assertions, all existing firewall transaction/command-status/startup/add-on contracts and AI docs validator to pass. If any fail, fix the actual failing contract and rerun on the exact new SHA.
- Once L2 is green, close this bounded writer slice and rotate to a different evidence-actionable userland subsystem. Do not remain focused on Firewall. FTP upload-cleanup TOCTOU remains OPEN/BLOCKED until a Linux 4.4-safe design preserves STOR/APPE/REST/STOU and parent/inode semantics. Production MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

## Verification boundary

- L1 source review: PASS for the BusyBox 1.24.2 Optware lock/teardown/startup path, storage daemon stop postconditions, infosvr packet bounds/SIGTERM deferral, and descriptor-bound firewall snapshot writing/close/error cleanup. The firewall writer's path is still retained for subsequent restore/cleanup; only the output write is descriptor-relative.
- L2 host / Actions: on exact SHA `02907c582c716a49c6255d2c06cbad429f5a3260`, storage [43042263807], infosvr [43042263817] and AI handoff docs [43042263966] all completed SUCCESS. Storage step 9 built/linked BusyBox 1.24.2; lock/teardown/startup and storage steps 10–13 passed. The firewall descriptor-writer regression and source contract added in THIS COMMIT require exact-resulting-SHA HTTPD and AI-docs runs.
- L3 MIPS/uClibc production objects / complete firmware/TRX: NOT VERIFIED for the current RC/firewall/storage changes. Existing historical FULLCONENAT object evidence is separately recorded; it does not establish a full firmware build.
- L4 router / physical hardware: NOT VERIFIED. No live firewall rollback, storage hotplug, wireless, WAN or other router behavior is claimed.
## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 has no inode-conditional unlink-by-fd: fstatat() followed by unlinkat() still deletes whichever inode owns the name at unlink time. rename/exchange is not a conditional inode compare, and staged publication would change existing STOR/APPE/REST visibility and overwrite semantics. Do not patch without a proved design preserving STOR/APPE/REST/STOU, parent/inode safety and supported filesystem behavior.
2. NFS async durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; require real client evidence.
3. Wireless bulk detach / bridge ownership across board variants — target validation open.
4. WAN PPP reconnect, callback recovery and IPv6 rollback — target behavior NOT VERIFIED.
5. Optware lock/fd/concurrent start-stop behavior — BusyBox 1.24.2 build/link, ash lock/teardown/startup and storage caller-graph L2 tests now pass on exact SHA `02907c582c716a49c6255d2c06cbad429f5a3260`; production MIPS/uClibc and real router runtime remain NOT VERIFIED.
6. FULLCONENAT board/kernel/ABI coverage — prior object evidence is not a full firmware build.
7. All complete MIPS/uClibc builds, firmware/TRX inspection and physical-router flash/boot remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` SHA and confirm it descends from `42800e0caaf90e0230f4e30d268e8301d0da2d70`. Inspect the new exact-head HTTPD Authentication and Security Regression and AI handoff documentation runs, including every step and complete logs.
2. Require `firewall_snapshot_fd.c` to pass under GNU89 ASan/UBSan, verify the independent push/pull path filters and the complete existing firewall transaction, status, startup and add-on source contracts.
3. If the exact-head HTTPD/docs checks are green, close descriptor-bound snapshot output at L2 and rotate to another evidence-actionable userland subsystem. Do not remain in Firewall.
4. Keep FTP upload-cleanup TOCTOU OPEN/BLOCKED until a Linux 4.4-safe design proves inode/parent safety and preserves STOR/APPE/REST/STOU. NFS external-client semantics, target MIPS/uClibc, full firmware/TRX and L4 remain NOT VERIFIED.
