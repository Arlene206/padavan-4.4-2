## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; operate only on this fork. The last code-bearing `main` checkpoint is `5e215b5e694488793d86ec984af6ea109f977fce`, parent `d9bf7a52cbf684ddb6b196016e429bbb5c01a19c`; THIS COMMIT records the exact-head validation closeout. Remote branch inventory remains only `main`; no checkpoint branch exists.
- **DDNS RC applet status correction is green:** `start_ddns` now propagates the int result of `start_ddns(1)`. On exact SHA `d9bf7a52cbf684ddb6b196016e429bbb5c01a19c`, all 13 push-triggered Actions workflows passed. DNSMasq lifecycle [42895344137] and AI docs [42895344037] passed; full job logs were inspected and validated the two GNU89 policy tests, DDNS/`restart_dhcpd` source contracts, and root log 68/119.
- **WAN bridge teardown correction is green:** commit `5e215b5e694488793d86ec984af6ea109f977fce` returns aggregate status from `disassembly_bridges_wan()`, fails closed for positive errno and negative ioctl failures, skips bridge deletion when port detach is incomplete, and aborts `full_restart_wan()` before LAN reconfiguration/WAN startup. Exact-head WAN teardown [42903823098] and AI docs [42903823036] passed. The WAN job's GNU89 `-Wall -Wextra -Werror` compile and regression executable succeeded; its static teardown/restart contract printed PASS. AI docs validated root log 69/119, batch 100, retain 20, archive inventory `dev-log=6, handoff=22, roadmap=18`.
- **All 9 workflows triggered on the WAN code commit passed** on the same exact SHA: WAN DHCP callback [42903822810], VPN lifecycle [42903822831], AI handoff docs [42903823036], WAN teardown [42903823098], detect_link [42903822824], HTTPD security [42903823335], detect_internet [42903823353], WAN ARP [42903822903], and WAN route status [42903822863]. Every job and step was checked; the WAN teardown and AI docs full logs were inspected. The WAN DHCP callback regression also compiled and ran all seven policy regressions and passed its callback-publication source contract.
- **Validation boundary:** these are bounded source/host tests and GitHub Actions only. No production MIPS/uClibc RC build, BusyBox ash runtime, complete firmware/TRX or physical L4 device test is claimed.
- The documentation closeout commit `f1c92f623a6192aeb391e30b23468c8c48dec5ef` passed AI handoff docs [42904140039]; its job log reported root log 70/119 and archive integrity PASS. This census correction adds one root log record (70 before, 71 after); no archive or `docs/dev-log/INDEX.md` update is due.

## Active task

- Continue the whole-repository P2 stability/security audit without repeating the completed DDNS/WAN bridge fixes. Priority next is to audit FTP failed-upload cleanup across STOR/APPE/REST/STOU and determine whether a single staged-publication design can prove safe inode/parent handling and preserve filesystem semantics. The FTP upload-cleanup TOCTOU remains OPEN/BLOCKED until that proof exists; do not apply a partial check-then-unlink workaround. NFS durability, AUTH_SYS/root-squash and external-mount behavior remain evidence-gated on real clients.
- If FTP cannot be safely resolved from the source's filesystem/call-path semantics, select the next source/host-testable P2 reliability defect from the whole-repository ledger; maintain broad module coverage.

## Exact first action for the next GPT

1. Fetch live `main`, record exact SHA/parent and exhaust the remote branch/checkpoint inventory before editing.
2. Confirm this documentation-closeout commit is the only descendant of code checkpoint `5e215b5e694488793d86ec984af6ea109f977fce`; don't repeat the completed WAN/DDNS fixes.
3. Trace FTP upload creation, append/restart offsets, failed-upload cleanup and rename/publication for STOR/APPE/REST/STOU. Prove the complete design against symlink/inode/parent replacement and filesystem semantics before changing cleanup behavior; otherwise keep it OPEN/BLOCKED.
4. Keep claims bounded to source/host/Actions. Production MIPS/uClibc, BusyBox ash, firmware/TRX and L4 router behavior remain unverified.

## PID 1 notification queue batch fairness

- Finding: handle_notifications() intentionally processes at most ten directory entries per invocation. notify_rc() creates queue marker files and sends SIGUSR1, but standard signals coalesce. If a burst queues more than ten distinct safe notifications before PID 1 consumes SIGUSR1, one signal wakes the handler, ten entries are consumed, and the remaining entries have no guaranteed new signal; the requests can stay in /tmp/rc_notification indefinitely.
- Fix: preserve the ten-entry bound. When a full batch completes without a shutdown/flash stop, scan only the remaining directory stream for a safe regular queue marker; if one remains, send PID 1 another SIGUSR1. The PID 1 loop already blocks SIGUSR1 while checking state and atomically waits with sigsuspend(), so the requeued signal is delivered on the next loop pass. Unsafe markers are skipped; directory read errors are logged rather than causing an unbounded self-signal loop.
- Regression: GNU89 batch policy covers below-limit, full-batch with/without pending work, and shutdown suppression. The dedicated workflow also compiles/runs the existing notification-marker trust-boundary test and checks the complete handler, signal wakeup, and push/PR path filters.
- Validation: the exact-head PID 1 workflow [42811049932] compiled and ran both the new GNU89 batch-policy test and existing RC notification marker trust-boundary test successfully. Its final source contract initially failed on a test assertion that matched an early error-path closedir(); this commit switches the ordering check to the final handler cleanup. Exact-head rerun is pending. No full RC host compile, MIPS/uClibc object, firmware/TRX or L4 router test is claimed.
- Next: inspect every job step in the PID 1 queue and AI handoff workflows on the exact resulting SHA, then select the next stable host-verifiable item from the repository-wide ledger.


## NFS storage lifecycle serialization (bounded stability fix)

- **Finding:** safe removable-storage paths hold the cross-process `storage` operation lock while stopping NFS and unmounting backing devices. The public RC `run_nfsd`/`stop_nfsd` applets and the `restart_nfsd` notification path previously bypassed that lock. A concurrent start/restart could therefore run after safe-removal stopped NFS but before the disk unmount, or race the mount inventory used by restart.
- **Call-path evidence:** `rc.c` dispatches direct `run_nfsd` and `stop_nfsd` commands to the public functions and `RCN_RESTART_NFSD` to `restart_nfsd()`. `safe_remove_stor_device()`, `safe_remove_all_stor_devices()`, and `try_start_stor_apps()` acquire `storage_operation_lock_acquire(file_lock)` before their integrated NFS start/stop work.
- **Fix:** public run/stop/reload wrappers now acquire the same storage lock. `restart_nfsd()` holds it continuously across mount inventory, stop, and optional restart/module unload. Internal `*_locked` helpers are used only by hotplug/safe-removal callers that already own the lock, avoiding recursive acquisition. `reload_nfsd()` now returns status as well as logging it.
- **Regression:** the NFS workflow source contract checks lock-before-side-effect ordering, the complete restart critical section, and that already-locked startup/removal paths use internal helpers. The initial exact-head contract still asserted the former public-function bodies and failed; this follow-up corrects it. The storage-hotplug and dependent HTTPD contracts are aligned as well. The NFS shell lifecycle fixture and GNU89 export-policy regression passed on the base SHA.
- **Compatibility boundary:** export defaults remain `async,secure,no_root_squash,no_subtree_check`. No durability, AUTH_SYS/root-squash or external-mount semantic change is justified without real-client compatibility evidence. The generated-export path still intentionally relies on the existing storage-device predicate and `/media/` mount policy.
- **Validation:** L1 caller/lock-boundary review PASS. On exact SHA `59dffdff5fc9ddd6b123a1759c7cac0853887797`, NFS shell syntax, the NFS start/reload lifecycle fixture and the GNU89 IPv4/IPv6 export-policy binary all passed, but the final source-contract step failed on an obsolete assertion expecting the command body inside public `run_nfsd()` rather than `run_nfsd_locked()`. USB storage [42804634773] and dependent HTTPD [42804633552] likewise exposed stale contracts expecting pre-lock helper call sites; no production C or runtime test failure was reported. This follow-up updates the NFS, USB storage and HTTPD contracts to check the new lock-owning wrappers and internal locked call graph; exact-head reruns are pending. AI docs [42804633693] passed on the base SHA. Local clone failed DNS resolution, so no separate local test or docs-validator run is claimed. No target MIPS/uClibc build, firmware/TRX or L4 router test is claimed.
- **Follow-up:** on exact base SHA `2d1ea332349ded6ff7c2c0801405f570095800e1`, the corrected NFS [42806093616], USB storage [42806093654], HTTPD [42806093873] and AI handoff [42806093551] Actions all completed SUCCESS. The DNSMasq audit and lifecycle-lock implementation are now recorded at `a6e6fbcd9e49dc25f0a3a104db301bbc46f502c2`. That commit inadvertently replaced the proven NFS/storage lock-wrapper source contracts with pre-lock assertions; the current continuation restores the previous green contract versions. Re-run both exact-head Actions before selecting the next repository-wide stability item. Keep NFS export compatibility and the blocked FTP leaf-cleanup TOCTOU explicit.

## DHCPv6 script-child callback fencing (bounded slice)

- **Finding:** this fork's BusyBox WIDE-derived `dhcp6c_script.c` invokes `spawn_and_wait()`; `spawn_and_wait()` forks/execs the configured `/tmp/dhcp6c.script` (`/sbin/rc`) and waits synchronously. The DHCP service stop path sends TERM/KILL to the process name `dhcp6c`, not to that separate `rc` child. If the parent reaches forced termination while the script is pending/running, the child can outlive it and later publish LAN IPv6/DNS state into a newer or torn-down WAN session.
- **Fix:** add a no-follow, regular-file/owner/link-count-checked mode-0600 generation file at `/tmp/dhcp6c-callback.state`, protected by a bounded cross-process fcntl lock. DHCPv6 start advances the generation and exports that generation plus the configured interface into the client's inherited environment; stop advances the generation while holding the lock as it stops the daemon; the script callback validates both token and interface and holds the same lock across its NVRAM/resolver updates. A waiting old child is rejected after stop/restart releases the lock. Callback validation does not create a missing state file. DHCPv6 stop/fence failures now abort IPv6 teardown; high-level WAN stop and SIT WAN-down paths also preserve the interface/state rather than proceeding with an unfenced client.
- **Regression:** GNU89 host test covers strict canonical generation parsing/overflow, interface identity, callback refusing to create a missing state file, symlink rejection and a forked old callback blocked behind stop-generation advancement. The exact BusyBox script and spawn implementations are included in the WAN workflow semantic contract.
- **Validation:** on exact baseline SHA `0dbba043...`, all six WAN GNU89 binaries passed; the previous helper-scope and stop-unlock assertions also passed. The source contract then failed at line 308 on the stale direct-return check and contained subsequent obsolete expectations of continued teardown after callback-fence failure and SIT error accumulation (run [42778786415]). AI docs [42778786370] passed. Source review confirms the implementation advances the generation, stops the client under the fence, unlocks and returns combined status; `wan6_down()` aborts before interface changes when fencing fails; and SIT WAN-down aborts before IPv4 route cleanup on IPv6 failure. No target MIPS/uClibc object, firmware/TRX or L4 test is claimed.
- **Next:** verify WAN callback and AI handoff docs on the exact resulting main SHA; if green, implement the `wan_up()` route-status propagation gap and then continue per-unit PPP ip-down route ownership. Keep FTP cleanup TOCTOU, NFS compatibility, FULLCONENAT target build, wireless board variants and L3/L4 verification open.

## PPP IPv4 ip-up address publication gate (bounded slice)

- **Finding:** ipup_main() previously ignored the return value from ifconfig() and immediately replaced the saved WAN ipaddr/netmask, then published interface identity, gateway and DNS and proceeded to route setup. An address-configuration error could therefore leave NVRAM claiming a lease that this callback had failed to apply.
- **Fix:** ppp_wan_ipup_apply_address() now calls the interface operation first and publishes the new address/netmask only after success. ipup_main() propagates the original interface error, releases the callback-session lock and skips interface identity, gateway/DNS and route publication on failure. The lock still spans the validation, interface operation and successful publication path.
- **Regression:** a new GNU89 host policy test covers configure-before-publish ordering, a failed interface operation preserving an existing published address, and invalid-input no-side-effects. The test compiled with cc -std=gnu89 -Wall -Wextra -Werror and ran locally with PASS. The WAN workflow now compiles/runs this test and checks the source ordering; exact-head Actions for THIS COMMIT are pending.
- **Scope boundary:** this original bounded slice fixed address-before-publication only. A later follow-up now upgrades `wan_up()` to return the aggregated route status and propagates it through PPP ip-up after callback-lock release. Per-unit ip-down route ownership/restart recovery remains open. No target MIPS/uClibc build or router test is claimed.

## Completed native-static/native-DHCP and SIT IPv6 startup status propagation

- Exact base SHA `cd3cf8438e28694ffb0f00b1ad9cdd871b1439e2` passed WAN callback [42760927729] and AI docs [42760927859]; the new startup patch is the current `THIS COMMIT` checkpoint.
- `start_sit_tunnel()` now returns status, validates tunnel inputs, checks tunnel/6rd/route/link/IPv6 address/RA operations, best-effort cleans a partially created tunnel and endpoint route, and only publishes derived LAN address state after the setup sequence succeeds.
- Native-static address/privacy/RA and route operations and native-DHCP fallback-route/privacy/autoconf/RA/client-start operations now propagate failure. Touched state is cleaned best-effort; WAN `ifname6` is published only on complete success.
- `control_if_ipv6()`, `control_if_ipv6_dad()` and `control_if_ipv6_privacy()` now return proc/sys write status. `rc.h` declarations match all updated definitions, including the SIT helpers.
- The WAN workflow adds source-semantic checks for checked operation call sites and error/state ordering; four existing GNU89 regression binaries passed on `21854daf`, but the source-semantic stage failed at two legacy assertions tied to the former DHCPv6 status variable. THIS COMMIT aligns those assertions with the new common status/macro flow. Its exact-head WAN/AI Actions are pending. No target MIPS/uClibc object, firmware/TRX or L4 router test is claimed.
- Remaining: verify partial-setup cleanup and IPv6 behavior on target; audit PPP/multi-WAN state transitions next. FTP upload cleanup TOCTOU and NFS compatibility remain open.

## Completed DHCP callback and BusyBox event-state review

- The previously unfinished DHCP callback source-contract correction is reconciled on exact SHA `6b44e023c2f669865fc8b5cb5425d824b8335d8b`: the GNU89 interface transaction regression and callback source-order contract passed [42703609156]; the handoff contract passed [42703609087].
- Completed a bounded review of BusyBox `udhcp_run_script()` dispatch and callers for initial `leasefail`, accepted NAK, lease expiry, renew/repeated renew, SIGUSR2 and release-on-quit.
- The applet deliberately ignores `spawn_and_wait()` callback status. Inspected transitions invoke deconfig for lease expiry and release paths; a matching NAK deconfigures only a previously bound state, not the initial REQUESTING state. Retry exhaustion routes to `leasefail`; SIGUSR2/release-on-quit use the release path. No reproducible BusyBox protocol-state defect was established, so no behavior-changing BusyBox patch was justified.
- Follow-up finding and fix in THIS commit: `udhcpc_viptv_bound()` discarded address ioctl and old/new route operation results, then replaced the prior `manv_*` route inventory even when old-route cleanup failed. The IPTV `deconfig` dispatch also cleared gateway and route source fields regardless of teardown results. The callback now applies/rolls back the address through the existing WAN interface transaction policy, deletes old routes before publishing new lease state, restores the previous address and retains route inventory when teardown fails, reports new route setup failures, and preserves deconfig route inventory unless cleanup succeeds. BusyBox's deliberate disregard of DHCP script exit status remains unchanged; callback logs and retained NVRAM route inventory are the observable recovery evidence. Static-IP IPTV startup and ZeroConf callback operations remain outside this bounded change.

## Completed FULLCONENAT namespace / zone work

- IPv4/IPv6 mapping entries and pending dying-tuple records retain the owning struct net pointer plus conntrack zone identity (id, flags and direction).
- Mapping lookup and tuple garbage collection are scoped by the full identity. Conntrack lookups reconstruct the saved zone; final per-net target teardown unregisters the notifier, waits for legacy RCU callback readers, purges only that namespace's mapping/queue entries, then drops the conntrack namespace reference.
- The GNU89 host policy regression covers same/different namespace, zone ID, flags, direction and null inputs. It passed exact-head runs [42674430844], [42677259891], [42677876443] and [42678293858]. The notifier/cleanup lifecycle contract also passed all four runs.
- Three IPv4 helper call sites now pass &context; five identical version-conditional declaration wrappers were removed so the semantic function-body extractor can parse the refactored functions.
- The exact-head semantic contract progressed through mapping structures, namespace/zone lookup filters, IPv4/IPv6 lookup paths and conditional notifier callback signatures. FULLCONENAT lifecycle regression [42678875089] and AI handoff docs [42678875391] passed on exact SHA `bc99b6801c44f55e13dc0214336408f712e0abf8`; subsequent WAN DHCP work did not modify the kernel code.
- Kernel translation-unit compilation remains unverified; the host policy test and semantic source contract do not replace a target kernel object build.

## Completed FTP upload path anchoring (bounded first slice)

- Finding: upload ACL resolution canonicalized the target with `realpath()`, but `str_create()` / `str_create_exclusive()` reopened the original pathname. A final or parent symlink could be substituted between authorization and open, redirecting the write after the ACL decision.
- Fix: `ftp_acl_path_resolve_write_entry()` resolves the effective target and returns canonical parent plus leaf; `ftp_acl_path_open_directory()` walks absolute canonical components using `openat(O_PATH|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC)`; `asus_open_upload_path()` checks ACLs, compares pre-open `stat()` identity with the pinned descriptor's `fstat()`, and opens the leaf with `openat(O_NOFOLLOW|O_CLOEXEC)`. Existing final-symlink writes still target their canonical destination; dangling links and metadata targets remain rejected.
- Failed-upload cleanup uses the retained directory descriptor and compares the open file's device/inode against `fstatat(AT_SYMLINK_NOFOLLOW)` before `unlinkat()`. This detects a replacement already present at check time, but the final check/unlink pair is not atomic against a concurrent writer in that same directory on Linux 4.4; cleanup remains best-effort. Cleanup through a final symlink alias is deliberately disabled rather than unlinking the resolved target.
- Regression tests exercise parent-path replacement by a cross-share symlink, final-leaf `O_NOFOLLOW`, canonical target resolution, dangling-link rejection and replacement-inode cleanup. The host compile/run step passed in run [42713615415] on `ac75a8ff43fca3672b44605a2974a0497ecd1c0d`; its static-contract step exposed a stale hard-coded count and was corrected in THIS COMMIT.
- L3 MIPS/uClibc production-object validation and L4 device behavior remain NOT VERIFIED.

## Completed FTP work

- Existing RNFR/RNTO fix: absolute source-path pinning plus source-delete/destination-write ACL revalidation against canonical parents.
- Path-aware checks now cover STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT, SIZE, MDTM and SITE CHMOD. Upload/RETR/metadata operations authorize the effective existing symlink target; MKD/RMD/DELE authorize the entry parent; listings authorize the actual opened directory before transfer setup.
- Added fail-closed handling for dangling final upload symlinks and canonical resolution of symlinked parent directories.
- Sidecar disclosure finding: `trunk/user/libdisk/src/disk_share.c` creates `.__folder_list.txt` and `.__<account>_var.txt` under the USB mount root. Per-share READ ACLs do not govern MOUNT_LAYER, so direct FTP file commands could expose folder/account ACL metadata. Exact-name filtering now hides and rejects these reserved files before the chroot ACL bypass.
- Both existing-file and upload path resolvers also classify the canonical `realpath()` target. Thus a harmless-looking final symlink alias to an ACL sidecar is denied as well as the direct filename. Ordinary `.__notes.txt` user files do not match the classifier.
- GNU89 regression now covers positive/negative metadata filenames, direct sidecar denial, final symlink alias denial, cross-share symlinks, dangling links, new leaves and symlinked directories.
- The only failure on `30d64c73546db236c12679d24eda74c1ea9f2916` is the CI Python variable-order defect described above; host binaries themselves all passed. The workflow correction moves all helper-body extraction before assertions.

## Completed Samba paired passdb/config publication hardening

- Finding: `run_samba()` staged and renamed the private passdb, then atomically published `/etc/smb.conf`. Ordinary errors restored the passdb, but abrupt termination between the two renames could leave a new passdb paired with old configuration and no same-boot recovery path.
- Fix: added `samba_config_transaction.h` and a paired transaction journal under `/etc/samba/.config-transaction`. PREPARED records both original-file snapshots before either live file changes; recovery restores both idempotently. COMMITTED records the new pair as authoritative and makes snapshot cleanup retryable. This is explicitly a same-boot process-crash journal in volatile `/etc`, not a power-loss durability guarantee.
- A shared process-scoped lock at `/var/run/samba-config.lock` serializes `run_samba()`, public `write_smb_conf()`, and WINS-only `start_wins()`. Every writer recovers an interrupted pair before modifying configuration. `run_samba()` keeps the lock through publication and daemon postcondition checks; rollback is attempted before releasing it on an uncommitted failure.
- Added GNU89 host regression coverage for interruption after passdb rename, interruption after both renames but before COMMITTED, committed-pair cleanup, first-time passdb creation rollback, retry after partial rollback, and symlink-target rejection.
- Changed source/test paths: `trunk/user/rc/src/services_stor.c`, `trunk/user/rc/src/services_ex.c`, `trunk/user/rc/src/samba_config_transaction.h`, `trunk/user/rc/src/tests/samba_config_transaction.c`, `trunk/user/rc/src/samba_passdb_policy.h`; updated `.github/workflows/samba-share-stability.yml` and dependent `.github/workflows/httpd-security.yml` static contracts.
- Exact successful Samba host/source regression: [42708213921] on `e9a93eb84ac9f95447150869fab7e0ae1c2da9ff`. Exact successful dependent HTTPD suite: [42708503436] on `258532a78922549698bc6d9b869c7777f8f4ac05`. No full target build or router test is claimed.

## Completed FTP descriptor-relative DELE/RMD (bounded mutation slice)

- Finding: DELE and RMD authorized the canonical parent through `asus_check_permission_path()`, then called `str_unlink(path)` / `str_rmdir(path)`. Replacing a parent component after ACL approval could redirect the mutation to another share.
- Fix: `asus_remove_path()` resolves and pins the canonical parent directory, performs the existing per-share DELETE ACL check while holding the directory descriptor, revalidates the canonical path against the pinned device/inode, then calls `unlinkat()` relative to that descriptor.
- Semantics: DELE uses no `unlinkat` flags and removes the final symlink entry itself; RMD uses `AT_REMOVEDIR` and refuses a final symlink rather than following its target. Reserved metadata leaf names are rejected. DELE trailing-slash behavior remains rejected as `ENOTDIR`.
- Regression: host policy tests cover symlink-entry deletion without deleting its target, RMD refusing a symlink to a directory, successful empty-directory removal, reserved metadata denial, and parent path identity mismatch after a symlink substitution. GNU89 host policy/source-contract and AI docs completed SUCCESS on exact SHA `05c7a13f44e64b4ba81f4af34b1000bb8eb4792e` (VSFTPD [42721783706], docs [42721783725]).
- Remaining FTP work: descriptor-relative MKD, RNFR/RNTO and bounded recursive LIST/NLST/STAT are complete. MDTM/SITE CHMOD descriptor-relative metadata review and the upload cleanup check/unlink race remain open; the latter is not atomic on Linux 4.4.

## Completed FTP descriptor-relative MKD (bounded creation slice)

- Finding: `handle_mkd()` checked WRITE permission against the canonical parent path, then invoked path-based `str_mkdir()`; a parent component could be swapped after ACL approval.
- Fix: `asus_make_directory_path()` splits the leaf, resolves and pins the canonical parent, checks WRITE ACL with the parent descriptor held, verifies the same parent device/inode after authorization, and creates via `mkdirat()`.
- Compatibility: keeps parent-directory WRITE ACL semantics, mode `0777` subject to the existing process umask, and FTP NOPERM versus FILEFAIL responses. Reserved metadata leaves remain blocked.
- Regression: host policy tests create and inspect a directory using the pinned parent fd and assert reserved metadata cannot be created. GNU89 host policy/source-contract and AI docs completed SUCCESS on exact SHA `9b6c65686ad95295fd456e074292638a6c9ccf92` (VSFTPD [42722114919], docs [42722114837]).
- RNFR/RNTO remain a separate two-path rename transaction and are not included in this slice.



## Completed DNSMasq lease-file applicability fix

- **Finding:** `prepare_dns_dhcpd()` previously validated/created `/tmp/dnsmasq.leases` before knowing whether the generated configuration would contain `dhcp-leasefile`. DNS-only and AP-mode RA-only configurations do not consume this path, but an unrelated unsafe stale path could block DNS service preparation.
- **Fix:** DHCP lease and RA feature bits are explicit; `dnsmasq_lease_file_ensure()` runs only when the finalized DHCP feature flags require the lease database, immediately before emitting `dhcp-leasefile`. On error, the staged config is closed/discarded and the original error returned. Lease-using DHCP remains fail-closed.
- **Regression:** GNU89 host tests cover no-feature, RA-only, lease-using and combined feature flags. HTTPD generated-file contract asserts the gate, source ordering and staged-config cleanup.
- **Exact-head validation:** commit `e1f27e5a5fb607a6ca8177c6cf1f220dc3515e7e`: HTTPD/DNSMasq generated trust-boundary regression [42735569321] SUCCESS (including the GNU89 test binary and full source semantic contract); AI handoff docs [42735569423] SUCCESS; Samba config regression [42735569085] SUCCESS. All three runs used this exact SHA.
- **Limit:** no local checkout was available for independent test execution; no production RC/MIPS/uClibc or L4 router validation is claimed.

## Other open risks carried forward

1. **FTP failed-upload cleanup TOCTOU — P2 OPEN/BLOCKED.** `fstatat()` followed by `unlinkat()` leaves a final pathname replacement race. Linux 4.4/MIPS O32 has no inode-conditional unlink. Do not add quarantine/advisory-lock workarounds without a proved staged-publication design covering STOR/APPE/REST/STOU, anonymous ownership/modes and filesystem compatibility.
2. **NFS client compatibility — P2 OPEN.** Preserve `async,secure,no_root_squash,no_subtree_check`; no policy migration without actual client evidence.
3. **FULLCONENAT target compile — P2 OPEN.** Host namespace/zone tests do not replace target kernel/MIPS object validation; `NF_CONNTRACK_EVENTS` gating is present.
4. **Wireless bridge/board variants — P2 OPEN.** Bulk detach and bridge ownership still require MIPS/board validation.
5. **WAN PPP/IPv6 — PARTIAL.** Logical unit 0 only; target reconnect, callback recovery and IPv6 rollback remain NOT VERIFIED.
6. **Global validation — OPEN.** Production RC/MIPS/uClibc objects, complete firmware/TRX and physical router flash/boot remain unverified.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the exact K95 installation/caller trace and the Optware stop-hook pathname/reverse-order source correction. |
| L2 host / Actions | Base SHA `1f7b3f2a568d8b6c8327206fe6111ce4e49dfbfa`: Shadowsocks [42879041009] and AI handoff docs [42879041085] SUCCESS, every job/step inspected. THIS COMMIT's storage hotplug regression/source contract and AI handoff docs are pending exact-head Actions. |
| L3 production BusyBox/MIPS/uClibc objects, full firmware/TRX and artifact inspection | NOT VERIFIED. |
| L4 physical router, removable-storage eject with live Optware services, Aria2/Transmission and WAN/VPN runtime | NOT VERIFIED. |

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete records newest first; at 120, archive the oldest 100 verbatim and retain newest 20. Archive shards and INDEX must be updated in the same commit. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
