## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; work only on this repository.
- Branch inventory: `main` only; no remote checkpoint branch. No force-pushes.
- **Last fully reconciled main SHA:** `118db7691f9dfb0389151236f5ad4e8e10bbdb18`. All ten exact-head Actions completed SUCCESS: WAN DHCP callback [42758479890], AI handoff docs [42758480028], WAN route [42758479837], storage hotplug [42758479898], WAN teardown [42758480112], HTTPD security [42758480227], NFS lifecycle [42758480113], vsftpd [42758480121], Samba [42758479866], and wireless [42758479880].
- **Current branch head at handoff:** `118db7691f9dfb0389151236f5ad4e8e10bbdb18`. DHCPv6 config publication and PPP IPv6-up result propagation are verified by the WAN job and AI docs on this exact SHA. All ten Actions listed above passed.
- **Completed previous slices:** DNSMasq start/stop status propagation and LAN relaunch gating passed exact-head HTTPD/DNSMasq and AI docs Actions on `822edf9d46a2f369a63eae0bcc1bb49f326c29e1`; FULLCONENAT's `NF_CONNTRACK_EVENTS` Kconfig dependency passed lifecycle and AI docs Actions on `15f350ab9397a60ab23ce854be7ee54191861a71`; bridge membership preflight/recheck passed on `5c78797b19e93beaa0bc313a69bbf37cb724b274`; bulk bridge teardown passed on `af3e41e72bb10d073bd7be1486b05266f2d53355`; NFS startup status now propagates through restart and RC notification callers and passed NFS lifecycle/policy/contracts plus AI docs on exact SHA `7fe64406495562776d7e9b43dbc7ff1d008744ca`.
- **Current development slice:** propagate DHCPv6 client-stop failures through IPv6 teardown. `stop_dhcp6c()` now returns `kill_services()` status; `wan6_up()` refuses to start a new client if the prior instance did not stop cleanly; `wan6_down()` returns that status while continuing interface/route/address cleanup; `ipv6down_main()` returns it after releasing the callback lock. Non-PPP teardown logs/aggregates the error and `stop_wan()` skips final WAN state clearing when IPv6 teardown remains incomplete. Exact-head workflow contracts are pending.
- **Known open issue:** FTP upload cleanup still has a check-then-`unlinkat` window after inode identity validation. Linux 4.4 and MIPS O32 provide `renameat2` (`__NR_renameat2 = 4351`), but exchange/no-replace operations cannot condition a rename or unlink on the expected device/inode. No safe atomic quarantine was established; retain the documented best-effort behavior and do not claim the race is closed.
- **Next after exact-head reconciliation:** verify the DHCPv6 stop-status contracts, all three WAN GNU89 regressions and AI docs on the exact resulting SHA. If green, continue PPP callback session fencing/reentrancy review and the remaining IPv6 RA/route teardown error surfaces; keep NFS export compatibility evidence separate. Keep MIPS/uClibc L3 and physical L4 validation visible.
- Root `dev_log.md` contains 117 complete newest-first records after this entry and remains below the 119-record / 384 KiB early-rotation thresholds; no archive rotation is due.
- L3 production RC/MIPS/uClibc objects, firmware/TRX and L4 physical-router validation remain NOT VERIFIED.

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

## Active task

1. **IPv6 teardown status propagation.** `stop_dhcp6c()` now returns `kill_services()` status. `wan6_down()` reports a failed client stop while continuing the remaining interface/route/address cleanup; the PPP IPv6-down callback returns this status only after releasing the callback lock.
2. **WAN teardown gates.** `wan6_up()` aborts reconfiguration/start if a previous DHCPv6 client could not be stopped. Non-PPP `wan_down_internal()` aggregates status; `stop_wan()` logs and skips final WAN state clearing after an IPv6 teardown failure. Public declarations are updated. WAN workflow source contracts are pending exact-head reconciliation.
3. **Completed PPP IPv6-up status.** `wan6_up()` returns DHCPv6 startup status; `ipv6up_main()` reports it after releasing the lock. The no-client-features path returns success; non-PPP callers log or aggregate status. WAN run [42758479890] and AI docs [42758480028] passed on exact SHA `118db7691f9dfb0389151236f5ad4e8e10bbdb18`.
4. **Completed DHCPv6 config publication.** Private 0600 same-directory staging, flush/close checks and atomic rename passed all three WAN GNU89 host regressions plus source contracts on `118db7691f9dfb0389151236f5ad4e8e10bbdb18`.
5. **Completed NFS start result.** `run_nfsd()` reports export-generation and daemon-start errors; restart and direct RC notification callers propagate them. NFS run [42758480113] and AI docs [42758480028] passed on `118db7691f9dfb0389151236f5ad4e8e10bbdb18`.
6. **L3/L4 boundary and open work.** No MIPS/uClibc production build, firmware/TRX or physical router tests. FTP upload cleanup retains a check/unlink race; NFS durability/AUTH_SYS/root-squash compatibility, PPP callback concurrency, remaining IPv6 RA/route teardown status and board-specific behavior remain open.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for DHCPv6 private publication, PPP IPv6-up result propagation and the new stop-status propagation path. FTP upload-cleanup inode-check/unlink race remains OPEN; other prior DNSMasq, FULLCONENAT, wireless and NFS source reviews remain recorded. |
| L2 host / Actions | Exact SHA `118db7691f9dfb0389151236f5ad4e8e10bbdb18`: all ten Actions SUCCESS. WAN DHCP job [42758479890] passed all three GNU89 WAN tests and the start-status contracts; AI docs [42758480028] passed. The new IPv6-down/stop-status contracts in THIS COMMIT are pending. |
| L3 production RC/MIPS/uClibc/firmware builds | NOT VERIFIED. |
| L4 physical router behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, confirm exact SHA, and inspect WAN DHCP callback plus AI handoff Actions. Verify `stop_dhcp6c()` returns process-stop failure, `wan6_down()` continues interface cleanup but reports failure, and `ipv6down_main()` forwards it after unlock.
2. If green, continue PPP callback-session fencing/reentrancy review and inspect IPv6 RA/route/address cleanup helpers for silent failures. Preserve NFS `async` / AUTH_SYS / root-squash compatibility review separately.
3. Keep MIPS/uClibc L3 build, PPP live session, DHCPv6 daemon and physical-router L4 behavior explicitly NOT VERIFIED.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete records newest first; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
