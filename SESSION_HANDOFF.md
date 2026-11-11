# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`; the branch inventory contains only `main`, with no remote checkpoint branch.
- Base main SHA for THIS COMMIT: `65f070aa3dfaa73154d3ab1ecb03d82a17abc388` (parent `d002eba06a914d75ac79b20867dbd3cdb6f416ca`).
- The HTTPD daemon lifecycle/duplicate-launch task is L1/L2 CLOSED on exact SHA `65f070aa3dfaa73154d3ab1ecb03d82a17abc388`: dedicated HTTPD lifecycle [43063659893], HTTPD security umbrella [43063082646] and AI handoff docs [43063659714] all SUCCESS. The dedicated lifecycle job passed all six steps including GNU89 ASan/UBSan policy binary and branch-aware source/path contract. The umbrella passed all 41 steps. The exact-head docs validator passed with root log=22/119 and archives dev-log=7, handoff=22, roadmap=18. infosvr regression [43063659718] also passed all six steps.
- THIS COMMIT addresses a separate GPIO front-panel timing defect: the hold counter begins at 1, making elapsed 700–799 ms map to count 8; the old strict `count < 8` condition dropped that whole short-action bucket despite the 800 ms comment. The shared 100 ms timing policy now accepts counts 1–8 and excludes count 9 at 800 ms, preserving the 3 s long-press and 5 s reset thresholds.
- Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` timing regression with leak detection passed. The exact-head GPIO lifecycle and AI docs Actions for THIS COMMIT are pending.
- Root `dev_log.md` had 22 complete entries at the base and will have 23; no rotation is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Production MIPS/uClibc RC object, complete firmware/TRX and physical router behavior remain NOT VERIFIED. FTP upload cleanup TOCTOU remains OPEN/BLOCKED; NFS external-client evidence remains required.
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

## Closed task — Descriptor-bound core firewall snapshot writing (P2 robustness/security; L1/L2 complete, L3/L4 open)

- `firewall_snapshot_core_table()` now keeps the mode-0600 `mkstemp()` descriptor open across `doSystem()`, clears `FD_CLOEXEC`, and directs `iptables-save` stdout to the inherited descriptor instead of reopening a shared-`/tmp` pathname.
- On exact SHA `9eaf81cc6eef88f87ca44ffe10dc574a2e988250`, HTTPD [43053436665], infosvr [43053436537] and AI handoff docs [43053436425] all completed SUCCESS. The descriptor writer regression printed PASS; transaction/status/startup policy binaries and the corrected push/pull path checker passed. Docs validator reported root log=118/119 and archives dev-log=6, handoff=22, roadmap=18.
- This closes only the L1/L2 host/Actions slice. Production MIPS/uClibc, full firmware/TRX and router L4 remain NOT VERIFIED.

## Closed task — Retry interrupted PID 1 mdev configuration fsync (P2 boot/hotplug reliability)

- Exact SHA `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`: mdev [43055641071], infosvr [43055640661], and AI handoff docs [43055640617] all completed SUCCESS. The injected two-EINTR plus real-fsync GNU89 ASan/UBSan regression and source contracts passed.
- `init_mdev_config_publish()` retries staged-file `fsync()` only for EINTR; all other errors remain fail-closed. L1/L2 is closed; production MIPS/uClibc, complete firmware/TRX, BusyBox target behavior and L4 hotplug remain NOT VERIFIED.

## Closed task — HTTPD daemon lifecycle and duplicate-launch prevention (P2 stability; L1/L2 complete)

- Exact main SHA `65f070aa3dfaa73154d3ab1ecb03d82a17abc388` (parent `d002eba06a914d75ac79b20867dbd3cdb6f416ca`).
- Dedicated HTTPD lifecycle [43063659893], HTTPD security umbrella [43063082646] and AI handoff docs [43063659714] all completed SUCCESS on the exact SHA. infosvr packet/signal regression [43063659718] also completed SUCCESS.
- Dedicated lifecycle job [130672703940]: all six steps SUCCESS. GNU89 ASan/UBSan policy regression printed `HTTPD daemon startup, duplicate-launch, stop-state and restart policy: PASS`; the branch-aware production integration and symmetric path-filter checker printed `HTTPD daemon startup, state publication, stop-before-start and path-filter source contracts: PASS`.
- HTTPD umbrella job [130670597005]: all 41 steps SUCCESS; full 4,618-line log inspected with no hard errors or traceback. AI docs job [130672703576]: all seven steps SUCCESS and printed `AI handoff documentation check: PASS (root log=22/119 entries, batch=100, retain=20, legacy/current archives: dev-log=7, handoff=22, roadmap=18)`.
- The previous source-check failure on `d002eba06a914d75ac79b20867dbd3cdb6f416ca` was correctly diagnosed as a false positive: the checker compared the normal launch publication with an earlier valid duplicate-daemon branch. `65f070aa...` scopes its check to the post-probe normal-launch path and passes.
- Bounded HTTPD service-state behavior is L1/L2 CLOSED. Process presence is not socket/readiness proof; simultaneous cross-process startup locking, target MIPS/uClibc, full firmware/TRX and L4 router behavior remain open/unverified.

## Active task — Correct GPIO short-press cutoff at the 800 ms boundary (P2 input reliability)

### Finding and implementation in THIS COMMIT

- `btn_update_press_count()` initializes a held button at count 1 and advances once per elapsed 100 ms: `floor(elapsed_ms / 100) + 1`. As a result, 700–799 ms maps to count 8 and 800 ms maps to count 9.
- `btn_released_handle()` previously dispatched a short WPS/FN action only for `count > 0 && count < CANCEL_COUNT`, while `CANCEL_COUNT=8` was documented as 800 ms. The complete 700–799 ms bucket was therefore suppressed.
- THIS COMMIT centralizes elapsed monotonic-time conversion and short/long/reset threshold predicates in `gpio_btn_timing_policy.h`. Short actions accept counts 1–8 and exclude count 9 at 800 ms. Existing >3 s long-press and >5 s reset boundaries are preserved. The daemon receive timeout now shares the same 100 ms quantum.
- Added a standalone GNU89 regression for 0, 699, 700, 799 and 800 ms, 2999/3000 ms, 4999/5000 ms, negative clock-delta normalization and `INT_MAX` saturation. Local `-Wall -Wextra -Werror -fsanitize=address,undefined` with leak detection: PASS (`GPIO button hold timing and threshold policy: PASS`).
- The existing GPIO lifecycle workflow now compiles/runs both startup and timing policy regressions and checks production integration, all threshold boundaries and symmetric push/pull_request path filters. Exact-head GPIO and AI docs Actions are pending.

### Exact next action

- Fetch the resulting live `main` SHA; inspect the GPIO startup/timing and AI handoff docs Actions on that exact SHA, including every job step and full logs.
- Require both GNU89 ASan/UBSan binaries, 700/799 ms short-action inclusion, 800 ms exclusion, unchanged 3 s/5 s thresholds, shared timer/source integration, source checker and symmetric push/pull_request path filters to pass. Docs validator should report root log=23/119 and archives dev-log=7, handoff=22, roadmap=18.
- On exact-head green, close only this L2 timing slice and rotate to another evidence-actionable userland reliability risk. FTP cleanup TOCTOU remains OPEN/BLOCKED; NFS remains external-evidence gated; target MIPS/uClibc and L4 remain NOT VERIFIED.

## Verification boundary

- L1 source review: PASS for HTTPD `_eval()` error propagation, duplicate-launch preflight, detached-child presence confirmation, normal start-state publication only after proof, stop-state clearing only after confirmed exit and restart gating. GPIO source review confirms count 8 is the complete 700–799 ms bucket and the old `< 8` predicate excluded it; THIS COMMIT uses a shared policy helper and keeps all thresholds consistent.
- L2 host / Actions: HTTPD lifecycle [43063659893], security umbrella [43063082646], AI docs [43063659714] and infosvr [43063659718] all SUCCESS on exact SHA `65f070aa3dfaa73154d3ab1ecb03d82a17abc388`; all job steps and complete logs inspected. THIS COMMIT's isolated GNU89 ASan/UBSan GPIO timing regression passed locally. Exact-head GPIO lifecycle and AI docs Actions are pending.
- L3 production MIPS/uClibc RC object / complete firmware/TRX: NOT VERIFIED for GPIO and current RC, storage, firewall and PID 1 changes. Prior FULLCONENAT object evidence is not a complete firmware build.
- L4 router / physical hardware: NOT VERIFIED. No live key/LED, uevent ordering/loss, router networking, HTTPD socket readiness or other device behavior is claimed.

## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 has no inode-conditional unlink-by-fd: fstatat() followed by unlinkat() still deletes whichever inode owns the name at unlink time. rename/exchange is not a conditional inode compare, and staged publication would change existing STOR/APPE/REST visibility and overwrite semantics. Do not patch without a proved design preserving STOR/APPE/REST/STOU, parent/inode safety and supported filesystem behavior.
2. NFS async durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; require real client evidence.
3. Wireless bulk detach / bridge ownership across board variants — target validation open.
4. WAN PPP reconnect, callback recovery and IPv6 rollback — target behavior NOT VERIFIED.
5. Optware lock/fd/concurrent start-stop behavior — BusyBox 1.24.2 build/link, ash lock/teardown/startup and storage caller-graph L2 tests now pass on exact SHA `02907c582c716a49c6255d2c06cbad429f5a3260`; production MIPS/uClibc and real router runtime remain NOT VERIFIED.
6. FULLCONENAT board/kernel/ABI coverage — prior object evidence is not a full firmware build.
7. All complete MIPS/uClibc builds, firmware/TRX inspection and physical-router flash/boot remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`, confirm it descends from `65f070aa3dfaa73154d3ab1ecb03d82a17abc388`, and inspect exact-head GPIO startup/timing and AI docs Actions, every job step and full logs.
2. Require startup/timing GNU89 ASan/UBSan tests, short-action inclusion through 799 ms, exclusion at 800 ms, unchanged 3 s/5 s thresholds, source integration, symmetric workflow filters and docs validation (root log=23/119; archive inventory 7/22/18) to pass.
3. If green, close the bounded GPIO L2 timing slice and rotate to another evidence-actionable issue from the whole-userland ledger. Do not revisit already closed PID 1 or HTTPD work.
4. FTP failed-upload cleanup TOCTOU remains OPEN/BLOCKED until a Linux 4.4-safe inode/parent design preserves STOR/APPE/REST/STOU; NFS requires real-client evidence. Production MIPS/uClibc, full firmware/TRX and L4 remain NOT VERIFIED.
