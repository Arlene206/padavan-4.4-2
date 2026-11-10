## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; work only on this repository.
- Branch inventory: `main` only; no remote checkpoint branch. No force-pushes.
- **Live main baseline before THIS COMMIT:** `b8613b08fef80e9a1eb6b684ac4abdc4d2a95158` (tree `4ce05256452b16924bdddee792a0ab86bf460a34`); active branch `main`, and remote branch inventory contains only `main` (no checkpoint). Exact-head AI handoff docs [42776419895] passed; WAN route [42776419776] and WAN teardown [42776419751] passed. WAN callback [42776419753] failed only in the Python semantic contract at line 205 after all six GNU89 host regression binaries passed; its assertion looked for `current == ULONG_MAX` in the lock-function body instead of the generation-increment helper. HTTPD security [42776419785] failed only at the stale `stop_wan()` return-count assertion (expected 5, current implementation has 6 after the DHCPv6 fencing guard). These are source-contract mismatches, not failures from the host regression binaries or observed runtime behavior. This commit corrects both workflow contracts; exact-head reruns are pending.
- **Prior exact-head evidence:** on `febac337d73e7d197ed950e94f308e4c066907b6`, the four WAN GNU89 tests passed; HTTPD security [42760357682], AI docs [42760357696], WAN route [42760357470], NFS [42760357384], storage [42760357751], Samba [42760357403], vsftpd [42760357409] and wireless [42760357782] succeeded. WAN source contract [42760357637] exposed the first stale assertion and was then corrected.
- **Current code slice:** common `fput_string()` exposes buffered write/close errors; DHCPv6 private config is atomically published; PPP callback start/stop status propagates through IPv6 callbacks; `wan6_down()` aggregates teardown failures; native-static/native-DHCP and SIT IPv6 startup check critical proc/sys, link, address, route and client operations; PPP IPv4 ip-up publishes only after interface-address success. Main `b8613b08...` also includes generation-locked fencing across separately spawned DHCPv6 `rc` script children, with stop/start invalidation and WAN teardown gates. The new six GNU89 regression binaries passed on the live baseline; the remaining failed Actions are source-contract assertions being corrected in THIS COMMIT. Rollback remains best-effort; target runtime behavior remains unverified.
- **Log rotation:** root `dev_log.md` now keeps 25 complete newest-first entries; no rotation is due. The 100 older records remain oldest-first in `docs/dev-log/2026/11/part-006.md` (239599 UTF-8 bytes), with the index synchronized.
- **Known open issue:** FTP upload cleanup still has a check-then-`unlinkat` window after inode identity validation. Linux 4.4/MIPS O32 `renameat2` exchange/no-replace cannot atomically condition on expected inode; do not claim closure.
- L3 production RC/MIPS/uClibc objects, firmware/TRX and L4 physical-router validation remain NOT VERIFIED.

## DHCPv6 script-child callback fencing (bounded slice)

- **Finding:** this fork's BusyBox WIDE-derived `dhcp6c_script.c` invokes `spawn_and_wait()`; `spawn_and_wait()` forks/execs the configured `/tmp/dhcp6c.script` (`/sbin/rc`) and waits synchronously. The DHCP service stop path sends TERM/KILL to the process name `dhcp6c`, not to that separate `rc` child. If the parent reaches forced termination while the script is pending/running, the child can outlive it and later publish LAN IPv6/DNS state into a newer or torn-down WAN session.
- **Fix:** add a no-follow, regular-file/owner/link-count-checked mode-0600 generation file at `/tmp/dhcp6c-callback.state`, protected by a bounded cross-process fcntl lock. DHCPv6 start advances the generation and exports that generation plus the configured interface into the client's inherited environment; stop advances the generation while holding the lock as it stops the daemon; the script callback validates both token and interface and holds the same lock across its NVRAM/resolver updates. A waiting old child is rejected after stop/restart releases the lock. Callback validation does not create a missing state file. DHCPv6 stop/fence failures now abort IPv6 teardown; high-level WAN stop and SIT WAN-down paths also preserve the interface/state rather than proceeding with an unfenced client.
- **Regression:** GNU89 host test covers strict canonical generation parsing/overflow, interface identity, callback refusing to create a missing state file, symlink rejection and a forked old callback blocked behind stop-generation advancement. The exact BusyBox script and spawn implementations are included in the WAN workflow semantic contract.
- **Validation:** local `cc -std=gnu89 -Wall -Wextra -Werror` compile/run of the generation-policy regression PASS; on exact SHA `b8613b08...`, all six WAN GNU89 host binaries passed, while the WAN semantic contract stopped at its misplaced `ULONG_MAX` assertion (run [42776419753]). AI docs passed on that SHA (run [42776419895]); HTTPD security had the independent stale return-count contract failure (run [42776419785]). L1 code/control-flow review complete. No RC/MIPS/uClibc object, firmware/TRX, live DHCPv6 runtime or L4 router test is claimed.
- **Next:** verify WAN callback, HTTPD security and AI handoff docs on the exact resulting main SHA after these workflow-only contract corrections. If green, continue `wan_up()` route-status propagation and PPP ip-down route restoration/per-unit route ownership; keep FTP cleanup TOCTOU, NFS compatibility, FULLCONENAT target build, wireless board variants and L3/L4 verification open.

## PPP IPv4 ip-up address publication gate (bounded slice)

- **Finding:** ipup_main() previously ignored the return value from ifconfig() and immediately replaced the saved WAN ipaddr/netmask, then published interface identity, gateway and DNS and proceeded to route setup. An address-configuration error could therefore leave NVRAM claiming a lease that this callback had failed to apply.
- **Fix:** ppp_wan_ipup_apply_address() now calls the interface operation first and publishes the new address/netmask only after success. ipup_main() propagates the original interface error, releases the callback-session lock and skips interface identity, gateway/DNS and route publication on failure. The lock still spans the validation, interface operation and successful publication path.
- **Regression:** a new GNU89 host policy test covers configure-before-publish ordering, a failed interface operation preserving an existing published address, and invalid-input no-side-effects. The test compiled with cc -std=gnu89 -Wall -Wextra -Werror and ran locally with PASS. The WAN workflow now compiles/runs this test and checks the source ordering; exact-head Actions for THIS COMMIT are pending.
- **Scope boundary:** this fixes the IPv4 address-publication gate only. It does not yet propagate wan_up() route-setup status through the legacy void wrapper, prove DHCPv6 script callbacks are fenced against late events, or close the remaining PPP/multi-WAN route ownership and restart-recovery audit. No target MIPS/uClibc build or router test is claimed.

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

## Active task

1. **Close current exact-head Actions failures first.** Live baseline `b8613b08fef80e9a1eb6b684ac4abdc4d2a95158`: WAN callback run [42776419753] has all six GNU89 binaries PASS but fails the source contract at the `ULONG_MAX` helper-scope assertion; HTTPD security run [42776419785] fails the old `stop_wan()` return count (5 expected, 6 actual). AI docs [42776419895], WAN route [42776419776] and WAN teardown [42776419751] pass on the same SHA. THIS COMMIT corrects the two contracts without changing firmware C behavior; verify the reruns on its exact resulting SHA.
2. **Next implementation task — PPP/multi-WAN route-state reliability.** After current Actions are green, resume the documented route-status propagation audit. Trace `wan_up()` / `wan_up_internal()` callers and return-value boundaries; then inspect PPP `ip-down` teardown/recovery, route deletion and restoration, per-unit route prefixes/ownership, callback-session lock lifetime, and stop/reconnect/restart ordering. Select one concrete source-demonstrable defect and add a GNU89 regression plus semantic source contract before editing broader paths. Do not revisit the already completed PPP ip-up apply-before-publish or DHCPv6 child-fencing slices absent new evidence.
3. **Project-wide open issues:** FTP upload cleanup TOCTOU; NFS `async` durability and AUTH_SYS/root-squash/external-mount compatibility; FULLCONENAT target kernel/MIPS object build; wireless bridge detach/membership behavior across board variants; SIT/native IPv6 rollback on target; and L3/L4 validation. Preserve each as NOT VERIFIED until target/client/hardware evidence exists.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the checked DHCPv6 generation/token/interface fence, callback lock lifetime, start/stop invalidation and WAN teardown gate; the two failing Actions are stale source-contract assertions, not observed C failures. |
| L2 host / Actions | On exact baseline SHA `b8613b08fef80e9a1eb6b684ac4abdc4d2a95158`, six WAN GNU89 binaries PASS; WAN source contract run [42776419753] FAIL at line 205 (helper assertion scope); HTTPD security run [42776419785] FAIL at the stale `stop_wan()` return count; AI handoff docs [42776419895], WAN route [42776419776] and WAN teardown [42776419751] SUCCESS. THIS COMMIT only corrects those two workflow contracts; exact-head reruns are pending. No full local checkout test run. |
| L3 production RC/MIPS/uClibc/firmware builds | NOT VERIFIED. |
| L4 physical router behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main` HEAD and branch inventory; do not trust this note's candidate SHA. Inspect WAN callback, HTTPD security and AI handoff-doc Actions for the exact resulting commit from THIS COMMIT. Confirm the WAN generation host binary and complete source contract pass, and confirm the HTTPD callback-fence ordering assertion as well as the updated stop failure count.
2. If those Actions pass, start the bounded PPP/multi-WAN route-state audit: read `wan_up()` and all callers/wrappers, identify where `wan_up_internal()` route errors are discarded or translated, then trace PPP `ip-down` route cleanup/restoration and per-unit route ownership. Choose one concrete defect and implement only that slice with a GNU89 regression and source contract.
3. Keep FTP upload-cleanup TOCTOU, NFS compatibility, FULLCONENAT target build, wireless board variants, target SIT/native rollback, and production MIPS/uClibc plus L4 hardware validation explicitly open/NOT VERIFIED.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete records newest first; at 120, archive the oldest 100 verbatim and retain newest 20. Archive shards and INDEX must be updated in the same commit. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
