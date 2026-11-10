## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; work only on this repository.
- Branch inventory at takeover: `main` only; no remote checkpoint branch. No force-pushes were used.
- Current main baseline for the recursive FTP listing hardening slice: `5ca27e7c420284f1ac21362e28566cfb56a693f1`; RNFR/RNTO VSFTPD regression [42724420056] and AI handoff docs [42724420034] both completed SUCCESS on this exact SHA. Remote branches: `main` only; no checkpoint branch.
- Samba production implementation commit: `1a7097ace2e6c689cb7e92cd10055967d99fe143` (parent `6b44e023c2f669865fc8b5cb5425d824b8335d8b`). Follow-up commits updated the Samba and HTTPD semantic workflow extractors after initial assertions still referenced the previous writer layout.
- Exact Actions evidence:
  - Samba share configuration regression [42708213921] completed SUCCESS on exact SHA `e9a93eb84ac9f95447150869fab7e0ae1c2da9ff`. GNU89 host compilation/execution passed for config-file, passdb policy, the new paired transaction, and daemon policy tests; the full Samba source-order contract also passed.
  - HTTPD Authentication/Security regression [42708503436] completed SUCCESS on exact SHA `258532a78922549698bc6d9b869c7777f8f4ac05`, including Samba share/ACL trust-boundary checks and the updated unlocked-writer source extractor.
  - AI handoff documentation validation [42708503300] completed SUCCESS on exact SHA `258532a78922549698bc6d9b869c7777f8f4ac05`.
  - VSFTPD run [42720948938] on exact SHA `4c0d2d8acd9f16c175037a64e4f05494ec7e565f` completed GNU89 host compilation and all three host test executions successfully, including the new ACL-path regression, then failed in the Python semantic source contract at the stale SIZE ordering assertion (`size.index()` still expected the removed path-based check/stat calls). AI handoff docs [42720949030] completed SUCCESS on that exact SHA. THIS COMMIT also updates the stale source-order assertion and the metadata-target classifier count (now four after the added canonical-target check).
  - Final RETR/SIZE exact-head regression [42721150755] and AI handoff docs [42721150532] both completed SUCCESS on exact SHA `bfd883ad75d5579c54922432a597efcade5a7509`; GNU89 host tests and the descriptor-based source contract both passed.
  - DELE/RMD descriptor-relative mutation regression [42721783706] and AI handoff docs [42721783725] both completed SUCCESS on exact SHA `05c7a13f44e64b4ba81f4af34b1000bb8eb4792e`; GNU89 policy tests and source-contract checks passed.
  - MKD descriptor-relative creation regression [42722114919] and AI handoff docs [42722114837] both completed SUCCESS on exact SHA `9b6c65686ad95295fd456e074292638a6c9ccf92`; GNU89 host suites and semantic source contract passed.
  - RNFR/RNTO rename commit `2953898fb09e4d5535fd0628bb3c74bb8b21a831`: VSFTPD regression [42724086864] failed only in its Python source contract at the RNTO cleanup-order assertion; its preceding GNU89 compile/run step passed all host suites. The contract used the first `str_free()` as if it were the post-rename cleanup, overlooking the earlier intentional clear in the destination permission-denial branch. AI docs [42724086704] passed on the same exact SHA. THIS follow-up corrects the contract to assert both branches explicitly; exact-head reruns are pending.
  - Initial attempts on `1a7097a...` failed only in stale semantic workflow extractors; the new transaction host test itself passed. The assertions were corrected and later exact-SHA runs above passed. Do not report the initial failures as green.
- The exact-head DHCP callback and handoff checks on `6b44e023c2f669865fc8b5cb5425d824b8335d8b` were already SUCCESS: WAN DHCP callback [42703609156] and AI handoff docs [42703609087].
- Root `dev_log.md` had 86 complete records before RNFR/RNTO and 87 after that implementation record; this source-contract reconciliation makes 88/119. No archive rotation is due.
- L3 production RC/MIPS/uClibc/firmware builds and L4 physical router validation are NOT VERIFIED.

## Completed DHCP callback and BusyBox event-state review

- The previously unfinished DHCP callback source-contract correction is reconciled on exact SHA `6b44e023c2f669865fc8b5cb5425d824b8335d8b`: the GNU89 interface transaction regression and callback source-order contract passed [42703609156]; the handoff contract passed [42703609087].
- Completed a bounded review of BusyBox `udhcp_run_script()` dispatch and callers for initial `leasefail`, accepted NAK, lease expiry, renew/repeated renew, SIGUSR2 and release-on-quit.
- The applet deliberately ignores `spawn_and_wait()` callback status. Inspected transitions invoke deconfig for lease expiry and release paths; a matching NAK deconfigures only a previously bound state, not the initial REQUESTING state. Retry exhaustion routes to `leasefail`; SIGUSR2/release-on-quit use the release path. No reproducible BusyBox protocol-state defect was established, so no behavior-changing BusyBox patch was justified.
- Follow-up observation outside this bounded applet audit: the IPTV DHCP callback path in `trunk/user/rc/src/net_wan.c` appears to discard some route/interface operation statuses. It remains a separate queued review and was not changed in this round.

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
- Regression: host policy tests cover symlink-entry deletion without deleting its target, RMD refusing a symlink to a directory, successful empty-directory removal, reserved metadata denial, and parent path identity mismatch after a symlink substitution. GNU89 test and exact-head workflow remain pending for this commit.
- Remaining FTP work: MKD and RNFR/RNTO still use path-based mutation; LIST/NLST recursive traversal and MDTM/SITE CHMOD also remain open. The upload cleanup check/unlink pair is still non-atomic on Linux 4.4.

## Completed FTP descriptor-relative MKD (bounded creation slice)

- Finding: `handle_mkd()` checked WRITE permission against the canonical parent path, then invoked path-based `str_mkdir()`; a parent component could be swapped after ACL approval.
- Fix: `asus_make_directory_path()` splits the leaf, resolves and pins the canonical parent, checks WRITE ACL with the parent descriptor held, verifies the same parent device/inode after authorization, and creates via `mkdirat()`.
- Compatibility: keeps parent-directory WRITE ACL semantics, mode `0777` subject to the existing process umask, and FTP NOPERM versus FILEFAIL responses. Reserved metadata leaves remain blocked.
- Regression: host policy tests create and inspect a directory using the pinned parent fd and assert reserved metadata cannot be created. Exact-head GNU89 test/source-contract and AI docs are pending for this commit.
- RNFR/RNTO remain a separate two-path rename transaction and are not included in this slice.

## Active task

1. RNFR/RNTO descriptor-relative rename and its source-contract correction are complete. Both VSFTPD regression [42724420056] and AI handoff docs [42724420034] passed on exact SHA `5ca27e7c420284f1ac21362e28566cfb56a693f1`.
2. THIS commit addresses the next P2 FTP listing boundary: top-level LIST/NLST/STAT now binds the ACL decision to the already-open directory descriptor; recursive descent opens each direct child via `openat(O_NOFOLLOW|O_DIRECTORY)`, verifies the child entry still names that inode, applies the READ share ACL to the child path, revalidates parent and child identities, then wraps the descriptor with `fdopendir()`. The host policy suite adds direct-child and symlink-rejection cases.
3. After exact-head VSFTPD and AI-doc Actions pass, continue auditing `ls.c` entry metadata (`str_lstat()` / `str_readlink()` remain pathname-based), `asus_check_file_visible()` using session CWD rather than listing base, recursive depth/resource limits, and LIST/NLST/STAT option/filter behavior. Do not conflate this bounded recursive-open fix with a complete listing audit.
4. Keep MDTM/SITE CHMOD path-based metadata syscalls, Linux 4.4 non-atomic upload cleanup, IPTV DHCP callback status, FULLCONENAT target build, NFS compatibility and L3/L4 validation open.
## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for upload and RETR/SIZE descriptor anchoring, DELE/RMD pinned-parent unlink, MKD pinned-parent mkdir, two-parent RNFR/RNTO rename, top-level listing ACL-to-DIR identity binding and recursive child `openat(O_NOFOLLOW)` plus per-child READ ACL/identity checks. |
| L2 host / Actions | RNFR/RNTO VSFTPD run [42724420056] and AI docs [42724420034] both completed SUCCESS on exact SHA `5ca27e7c420284f1ac21362e28566cfb56a693f1`. THIS recursive listing GNU89 host regression/source contract and AI-doc validation are pending on the resulting SHA. |
| L3 production RC/MIPS/uClibc/firmware builds | NOT VERIFIED. |
| L4 physical router behavior | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branches and Actions; reconcile this recursive-listing commit's VSFTPD and AI-doc jobs against the exact resulting `head_sha` and inspect every job step.
2. If both are green, continue the residual `ls.c` audit: replace per-entry pathname `str_lstat()` / `str_readlink()` only after defining descriptor-relative formatting semantics; inspect `asus_check_file_visible()` base-path handling and bound recursive traversal resources.
3. Keep MDTM/SITE CHMOD, upload-cleanup race, IPTV DHCP callback, FULLCONENAT target build, NFS compatibility and L3/L4 validation open.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete records newest first; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
