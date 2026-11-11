# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; latest branch inventory contains main only; no remote checkpoint branch.
- Base main SHA for THIS COMMIT: `abff1ed57217c1fe482bf80c2c419dde66437bf0` (parent `c4afed88518d2c4a9dc8c4e4dc2d0f2f129996f4`). Resolve the resulting SHA from live main after publication.
- Bootstrap recovery completed in required order: AI_BOOTSTRAP_PROMPT.md, AI_DEVELOPMENT_PROTOCOL.md, live main ref, current handoff, roadmap, newest 20 complete dev-log records, archive index, branch inventory and exact-head Actions. On exact SHA `abff1ed57217c1fe482bf80c2c419dde66437bf0`, udhcpc [43040638862], infosvr [43040638851] and AI docs [43040638793] passed; storage [43040639192] passed steps 3–8 and the BusyBox build/ash/awk/flock smoke plus script syntax checks, but failed in the BusyBox ash lock regression. Every step and complete available log was inspected.
- Root dev_log.md has 114 complete newest-first records before THIS COMMIT and will have 115 afterward, below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no rotation/index update is due.

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

## Active task — Complete Optware lifecycle tests under the matching firmware BusyBox ash 1.24.2 (P2)

### Exact-head Actions reviewed

- Live main at recovery: `abff1ed57217c1fe482bf80c2c419dde66437bf0`; remote branch inventory contains only `main`, with no checkpoint.
- Storage workflow [43040639192] completed FAILURE on that exact SHA. Steps 3–8 passed. Step 9 built and linked BusyBox 1.24.2; direct ash and awk executions, descriptor-backed `flock -n 9` / `flock -u 9` smoke, and production-script `ash -n` checks passed. The first lifecycle test then exited with `flock is required for this regression`.
- The precise cause is the test's preliminary `command -v flock` check. The firmware-derived BusyBox config has `CONFIG_ASH_CMDCMD` disabled (the oldconfig log confirms the option was left unset), so this ash build does not provide the optional `command` builtin used by that guard. The later real `flock` operation is the meaningful requirement; the guard is incompatible with the exact shell configuration and must be removed. Lock/teardown/startup behavior under ash has not yet been completed.
- UdHCPC [43040638862] SUCCESS: GNU89 ASan/UBSan retry-policy binary and corrected event-script/lease-state source contract passed.
- Infosvr [43040638851] SUCCESS: packet initialization/bounds and SIGTERM deferral ASan/UBSan policy tests plus semantic source contract passed.
- AI handoff docs [43040638793] SUCCESS. Validator reported root log 114/119 and archives dev-log=6, handoff=22, roadmap=18.

### Fix in THIS COMMIT

- Remove only the non-portable `command -v flock` preflight from `optware_lifecycle_lock.sh`. Its first lock acquisition already executes `flock 9`; the dedicated CI smoke also proves the actual BusyBox applet can acquire/release a descriptor lock.
- Do not enable `CONFIG_ASH_CMDCMD` merely to accommodate the test. Keep the host ash feature set matched to the firmware board config. No production lifecycle code changes are intended.

### Exact next action

- Fetch the resulting live main SHA. Inspect storage and AI handoff docs Actions on that exact SHA, every job step and complete logs; confirm the udhcpc and infosvr workflows remain green for the same commit.
- Require all three Optware lock/teardown/startup regressions to complete under BusyBox ash, followed by storage policy binaries and the whole caller-graph/lock/failure-propagation source contract (steps 10–13).
- Keep production MIPS/uClibc objects, complete firmware/TRX, matching router BusyBox runtime and L4 hotplug/lock timing NOT VERIFIED. FTP upload-cleanup TOCTOU remains OPEN/BLOCKED absent a Linux 4.4-safe semantics-preserving design.

## Verification boundary

- L1 source review: PASS for BusyBox udhcpc event-status/lease-publication gating, storage-backed daemon stop postconditions, Optware exact-/proc/swaps fail-closed teardown and caller/lock-order graph, and infosvr SIGTERM deferral/normal-context cleanup. The remaining storage failure is isolated to the lock regression's optional `command -v` preflight; the firmware BusyBox ash configuration disables that builtin. THIS COMMIT removes the test-only assumption while leaving the actual lock checks intact.
- L2: on exact SHA `abff1ed57217c1fe482bf80c2c419dde66437bf0`, udhcpc [43040638862], infosvr [43040638851] and AI docs [43040638793] passed. Storage [43040639192] passed POSIX-shell syntax/lock/teardown/startup steps 3–8, BusyBox 1.24.2 build/link, direct ash/awk/flock smoke and `ash -n` script checks; the first under-ash lifecycle regression stopped at `command -v flock` because `CONFIG_ASH_CMDCMD` is disabled. Teardown/startup under ash and storage steps 10–13 therefore remain pending. New exact-head storage/docs verification is pending.
- Production MIPS/uClibc objects, complete firmware/TRX, live DHCP v4/v6 behavior, matching target BusyBox ash 1.24.2 and L4 router behavior remain NOT VERIFIED.

## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 has no inode-conditional unlink-by-fd: fstatat() followed by unlinkat() still deletes whichever inode owns the name at unlink time. rename/exchange is not a conditional inode compare, and staged publication would change existing STOR/APPE/REST visibility and overwrite semantics. Do not patch without a proved design preserving STOR/APPE/REST/STOU, parent/inode safety and supported filesystem behavior.
2. NFS async durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; require real client evidence.
3. Wireless bulk detach / bridge ownership across board variants — target validation open.
4. WAN PPP reconnect, callback recovery and IPv6 rollback — target behavior NOT VERIFIED.
5. Optware lock/fd/concurrent start-stop behavior — BusyBox 1.24.2 build/link, descriptor-lock smoke and ash syntax checks now pass; the lock script's `command -v` preflight failed because firmware ash disables CONFIG_ASH_CMDCMD. This commit removes that test-only optional-builtin dependency; exact-head ash regressions remain pending. Production MIPS/uClibc and router runtime remain NOT VERIFIED.
6. FULLCONENAT board/kernel/ABI coverage — prior object evidence is not a full firmware build.
7. All complete MIPS/uClibc builds, firmware/TRX inspection and physical-router flash/boot remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` SHA and confirm it descends from this commit; inspect storage [USB storage hotplug stability regression] and AI handoff docs Actions on that exact SHA, every job step and complete logs; confirm udhcpc [BusyBox udhcpc event script status stability regression] and infosvr [infosvr packet and signal lifecycle stability regression] remain green on the same SHA.
2. Require the BusyBox 1.24.2 ash run to pass lock contention/release, teardown-status and startup-supervision/rollback regressions; then require storage steps 10–13 (GNU89 policy tests and full caller graph/locks/failure propagation) to pass.
3. If an ash failure remains, preserve the firmware-derived shell configuration and diagnose the exact missing feature/behavior. Do not enable optional shell builtins solely to appease the host test.
4. Once exact-head L2 is green, close this bounded Optware host-shell slice and rotate to the next evidence-actionable repository-wide stability item. FTP upload-cleanup TOCTOU remains OPEN/BLOCKED without a proved Linux 4.4-compatible design. Production MIPS/uClibc, full firmware/TRX and L4 remain NOT VERIFIED.
