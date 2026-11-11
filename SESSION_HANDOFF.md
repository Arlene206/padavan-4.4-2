# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`; only remote branch is `main`; no checkpoint branch.
- Base main SHA for THIS COMMIT: `d5b5ff7ed841ea12af2eda604580722e49bf7436` (parent `d65a890e11712db880c09b48e05215a691118ab6`).
- DNS Forwarder caller-CWD-independent launcher is L1/L2 CLOSED on exact SHA `c2e1c85517ed2dcbe094ed43e3cae18d7af4d972`: DNS [43066687882], AI docs [43066687814] and infosvr [43066687818] SUCCESS.
- DNSMasq DHCP pool/router-address exclusion is L1/L2 CLOSED on exact SHA `c22542d36e9de26780b338267f41a91d99aaf34a`: DNSMasq [43069047780], AI docs [43069047752], HTTPD [43069047723], infosvr [43069047578], Samba [43069047831] and miniupnpd [43069047633] SUCCESS.
- WAN bridge VLAN deletion failure propagation is L1/L2 CLOSED on exact SHA `55740d3491fba908416b930664143943b5acca1a`: wireless [43069624285], AI docs [43069624357], infosvr [43069624265] and WAN DHCP [43069624334] SUCCESS.
- Wi-Fi bridge-port detach during radio shutdown is L1/L2 CLOSED on exact SHA `8c715b2b156b6c40627d026a84007eb4059a76c6`: wireless [43070956995], AI docs [43070956985] and infosvr [43070957210] all SUCCESS. Full job steps/logs were inspected; both GNU89 policy regressions and whitespace-normalized radio/membership source contracts passed; docs validator root log=30/119 and archives=7/22/18.
- THIS COMMIT stages OpenVPN server/client primary config generation beside the live destination, keeps the stage mode 0600 while writing, checks flush/ferror/fsync/mode/close/rename and only then atomically publishes mode 0644. Failure leaves the previous config intact and existing startup callers fail closed.
- Exact-head OpenVPN run [43071668469] passed the credential regression but failed config-test compile at step 4: four YAML compiler lines ended in two backslashes, so ld received a literal backslash input and reported `cannot find \\`. The config binary and source-contract step did not run. Docs [43071668470], HTTPD [43071668464] and infosvr [43071668500] all passed on that SHA. THIS COMMIT fixes the four shell continuations; corrected exact-head OpenVPN/docs runs are pending.
- On exact SHA `d5b5ff7ed841ea12af2eda604580722e49bf7436`, corrected shell continuations allowed both GNU89 sanitizer binaries to PASS, but the OpenVPN source-contract step failed at line 51 because it still searched for the obsolete `fp = fopen(conf_file, "w+")` boundary. `openvpn_create_client_conf()` now opens via `openvpn_config_file_open()`. Docs [43072228055] and infosvr [43072227944] passed on d5b5ff7. THIS COMMIT updates the fail-closed ordering assertion to the staged-config open call; corrected exact-head OpenVPN/docs runs are pending.
- Root `dev_log.md` grows 32→33 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- FTP upload-cleanup TOCTOU remains P2 OPEN/BLOCKED pending a Linux 4.4-safe inode/parent proof; NFS requires real-client evidence. Production MIPS/uClibc, full firmware/TRX and physical-router behavior remain NOT VERIFIED.

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

## Closed task — GPIO short-press timing and workflow contract (P2 input reliability; L1/L2 complete)

- Production timing correction: commit `54504272e2c4ca0f7783ba78aa52a2bfe212de1c`; workflow symmetry correction: commit `31578abaefe5b2838b7c825460e21e730176b563`.
- The hold counter begins at count 1 and advances each 100 ms. Counts 1–8 include the full 700–799 ms short-press interval; count 9 excludes 800 ms. Existing >3 s long and >5 s reset thresholds remain unchanged.
- GPIO workflow [43065042288], AI docs [43065042295] and infosvr [43065042298] all SUCCESS on exact SHA `31578abaefe5b2838b7c825460e21e730176b563`. Startup/timing GNU89 ASan/UBSan binaries, source/caller contracts and exact-once path-filter checks passed. Docs validator reported root log=24/119 and archive counts 7/22/18.
- The first CI run correctly exposed duplicated timing paths in push and missing pull_request entries; commit `31578aba...` corrected the filters. This bounded GPIO timing L2 slice is CLOSED.
- Production MIPS/uClibc, physical button/LED/uevent behavior, full firmware/TRX and L4 remain NOT VERIFIED.

## Closed task — DNS Forwarder caller-CWD-independent launcher (P2 reliability; L1/L2 complete)

- Production fix SHA: `e6801775fc3af56dd084321eca9376899b2e7d76`; fixture line-ending correction SHA: `c2e1c85517ed2dcbe094ed43e3cae18d7af4d972`.
- RC startup now calls the ROMFS-installed `/usr/bin/dns-forwarder.sh` instead of the CWD-relative slash-containing `usr/bin/dns-forwarder.sh`. The relative name failed from an unrelated working directory because `eval()` delegates to `execvp()` and a slash-containing name is not PATH-searched.
- Exact-head DNS Forwarder [43066687882], AI docs [43066687814] and infosvr [43066687818] all SUCCESS on `c2e1c85517ed2dcbe094ed43e3cae18d7af4d972`. The CWD behavior regression, service/installer/execvp source contracts, ten exact-once push/pull_request filters and docs validator passed; docs reported root log=26/119 and archive inventory 7/22/18.
- L1/L2 slice CLOSED. Production MIPS/uClibc, daemon readiness/live forwarding and L3/L4 router validation remain NOT VERIFIED.

## Closed task — Prevent DNSMasq DHCP pools from including the router LAN address (P2 LAN reliability; L1/L2 complete)

- Exact main SHA `c22542d36e9de26780b338267f41a91d99aaf34a`: DNSMasq lifecycle [43069047780] and AI handoff docs [43069047752] both completed SUCCESS. HTTPD authentication/security [43069047723], infosvr packet/signal [43069047578], Samba config [43069047831] and miniupnpd config [43069047633] also completed SUCCESS.
- DNSMasq lifecycle job: all ten steps succeeded. GNU89 restart policy, DHCP pool conflict/range policy, cross-process lifecycle lock, and private atomic Inadyn configuration regressions passed; DNSMasq parser/source, generated DHCP file ordering, daemon postcondition and RC status propagation contracts passed.
- AI docs job: all seven steps succeeded. Validator reported root log=27/119 and archive inventory dev-log=7, handoff=22, roadmap=18. Full logs were inspected; no test failure was found.
- This closes the DHCP pool/router-address exclusion slice at L1/L2 only. Production MIPS/uClibc build, complete firmware/TRX, live lease allocation and L4 router behavior remain NOT VERIFIED.

## Closed task — Propagate VLAN deletion failures during WAN bridge teardown (P2 reliability; L1/L2 complete)

- Exact main SHA 55740d3491fba908416b930664143943b5acca1a: wireless bridge/VLAN regression [43069624285], AI handoff docs [43069624357], infosvr packet/signal [43069624265] and WAN callback umbrella [43069624334] all SUCCESS. Every job checked out the exact SHA; all steps passed and full logs were inspected.
- GNU89 bridge membership/teardown regression printed "Bridge membership and teardown policy regression: PASS". The source contract printed "Wireless driver/restart failure state-publication contracts: PASS", including VLAN selection and propagation of vconfig_del_if() failure into br_del_all_ifs(). AI docs validator reported root log=28/119, archives=dev-log 7 / handoff 22 / roadmap 18.
- Caller path from br_del_all_ifs() through disassembly_bridges_wan() to the full-WAN-restart fail-closed gate is source-reviewed. This closes the bounded host/Actions slice at L1/L2 only.
- Production MIPS/uClibc bridge/VLAN ioctl behavior, full firmware/TRX and physical router WAN teardown remain NOT VERIFIED.

## Closed task — Detach Wi-Fi bridge ports during radio shutdown (P2 network stability; L1/L2 complete)

- Parent source commit `dfd4e4711cf53620b7be75c9f65d430154249fc8`; checker whitespace correction `8c715b2b156b6c40627d026a84007eb4059a76c6`.
- Exact-head wireless [43070956995], AI docs [43070956985] and infosvr [43070957210] all SUCCESS; every job step and full logs inspected.
- Wireless log confirms GNU89 interface status policy, bridge membership/teardown policy and complete radio helper/membership/source contracts all PASS. The prior failure was only an exact-string checker false negative for valid multiline RT3352 MII calls; checker now normalizes whitespace while retaining argument/order assertions.
- Radio shutdown brings each interface down before detaching its bridge port; aggregates failures and retains LED/restart state on incomplete shutdown. RT3352 MII guest maps to the guest VLAN and iNIC main also detaches. Add/delete membership is idempotent only after complete bridge inventory and verified duplicate-operation races.
- AI docs validator reports root log=30/119, batch=100, retain=20, archives dev-log=7, handoff=22, roadmap=18.
- L1/L2 closed. Target MIPS/uClibc, RT3352 MII/iNIC board variants, DBDC and L4 radio behavior remain NOT VERIFIED.

## Active task — Publish OpenVPN server/client configuration atomically and fail closed (P2 reliability)

### Finding and implementation in THIS COMMIT

- Both `openvpn_create_server_conf()` and `openvpn_create_client_conf()` opened their live destination with `fopen(conf_file, "w+")`, truncating the last usable configuration before the new one was complete. They ignored `fprintf`/custom-config copy, `fclose()` and `chmod()` errors and returned success unconditionally; callers then proceeded to interface setup and daemon launch.
- New `openvpn_config_file.h` creates a same-directory `mkstemp` staging file, verifies regular-file ownership/link count, keeps it mode 0600 while generating, then checks `fflush`, `ferror`, EINTR-safe `fsync`, final mode 0644, a second `fsync`, `fclose` and atomic `rename`. Failed publication closes/unlinks the stage, restores errno, preserves the prior live config and returns failure so existing start functions stop.
- Both server and client config generators use the shared helper, log the publication failure and return nonzero. Existing private client credential publication remains unchanged.
- New GNU89 ASan/UBSan regression covers private stage and final mode under umask 0000, last-good preservation until rename, RLIMIT_FSIZE-induced flush failure, failed-stage cleanup, symlink replacement semantics, rename failure, and invalid/undersized paths. Standalone host test passed with `OpenVPN config atomic publication and failure preservation: PASS`; exact-head Actions are pending.
- Scope is the primary server/client configuration. Per-client CCD files and VPN lease snapshots are separate future audit items; no parent-directory fsync or power-loss durability claim is made.

### Exact next action

- Inspect OpenVPN client credential/config regression and AI handoff docs Actions on the resulting exact main SHA; verify every job step and full logs.
- Require existing private credential test plus the new config GNU89 ASan/UBSan regression, RLIMIT_FSIZE EFBIG failure injection, last-good preservation/stage cleanup, source contracts for both server/client call paths, exact-once symmetric push/PR path filters, and docs validator root log=31/119 with archives=7/22/18 to pass on the same SHA.
- If green, close this bounded OpenVPN config L2 slice. Then audit OpenVPN per-client CCD and VPN lease snapshot publication. FTP TOCTOU remains OPEN/BLOCKED until a Linux 4.4-safe inode/parent approach proves STOR/APPE/REST/STOU preservation; NFS requires client evidence; target MIPS/uClibc, full firmware/TRX and L4 remain NOT VERIFIED.

## Verification boundary

- L1 source review: PASS for both OpenVPN server/client writers now staging beside destination and returning failure on publication error; existing start callers gate interface creation/daemon launch on the writer result.
- L2 host: standalone GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` config publication test PASS. RLIMIT_FSIZE injected EFBIG during flush, and prior config remained intact with stage removed. Prior Wi-Fi interface-state and bridge membership/teardown regressions and source contracts are L2 CLOSED on exact SHA `8c715b2b156b6c40627d026a84007eb4059a76c6`.
- L2 Actions: OpenVPN credential/config and AI docs Actions for THIS COMMIT are pending; do not close OpenVPN config until all steps and full logs are inspected. Wi-Fi [43070956995], docs [43070956985], infosvr [43070957210] were all SUCCESS.
- L3 production MIPS/uClibc RC objects / complete firmware/TRX: NOT VERIFIED.
- L4 router / physical hardware: NOT VERIFIED. No live OpenVPN daemon start, authentication handshake or physical-router behavior is claimed.

## Other open governance-ledger risks

1. FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED. Linux 4.4 has no inode-conditional unlink-by-fd: fstatat() followed by unlinkat() still deletes whichever inode owns the name at unlink time. rename/exchange is not a conditional inode compare, and staged publication would change existing STOR/APPE/REST visibility and overwrite semantics. Do not patch without a proved design preserving STOR/APPE/REST/STOU, parent/inode safety and supported filesystem behavior.
2. NFS async durability, AUTH_SYS/root-squash and external mount compatibility — P2 OPEN; require real client evidence.
3. Wireless bulk detach / bridge ownership across board variants — target validation open.
4. WAN PPP reconnect, callback recovery and IPv6 rollback — target behavior NOT VERIFIED.
5. Optware lock/fd/concurrent start-stop behavior — BusyBox 1.24.2 build/link, ash lock/teardown/startup and storage caller-graph L2 tests now pass on exact SHA `02907c582c716a49c6255d2c06cbad429f5a3260`; production MIPS/uClibc and real router runtime remain NOT VERIFIED.
6. FULLCONENAT board/kernel/ABI coverage — prior object evidence is not a full firmware build.
7. All complete MIPS/uClibc builds, firmware/TRX inspection and physical-router flash/boot remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and remote branches; verify exact SHA and parent after THIS COMMIT.
2. Inspect OpenVPN credential/config and AI docs Actions on that exact SHA, every job step and full log. Require both GNU89 ASan/UBSan/leak binaries, RLIMIT_FSIZE failure injection, last-good preservation/stage cleanup, and the source contract with credential failure ordered before openvpn_config_file_open(); docs validator must report root log=33/119 and archives=7/22/18.
3. If green, close the bounded OpenVPN server/client config L2 slice. Then audit per-client CCD and VPN lease snapshot publication; primary config validation does not cover those separate write paths.
4. FTP upload-cleanup TOCTOU remains OPEN/BLOCKED until a proven Linux 4.4-safe inode/parent design preserves STOR/APPE/REST/STOU. NFS needs external-client evidence; production MIPS/uClibc, full firmware/TRX and L4 remain NOT VERIFIED.
