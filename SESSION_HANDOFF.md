## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; work only on this repository.
- Branch inventory at takeover: `main` only; no remote checkpoint branch. No force-pushes were used.
- Current main checkpoint before this test-contract correction: `6f15ad9a2580f9d0bba16447d61d315d5e8eb4e3` (RETR/SIZE descriptor-relative implementation); remote branches: `main` only; no checkpoint branch.
- Samba production implementation commit: `1a7097ace2e6c689cb7e92cd10055967d99fe143` (parent `6b44e023c2f669865fc8b5cb5425d824b8335d8b`). Follow-up commits updated the Samba and HTTPD semantic workflow extractors after initial assertions still referenced the previous writer layout.
- Exact Actions evidence:
  - Samba share configuration regression [42708213921] completed SUCCESS on exact SHA `e9a93eb84ac9f95447150869fab7e0ae1c2da9ff`. GNU89 host compilation/execution passed for config-file, passdb policy, the new paired transaction, and daemon policy tests; the full Samba source-order contract also passed.
  - HTTPD Authentication/Security regression [42708503436] completed SUCCESS on exact SHA `258532a78922549698bc6d9b869c7777f8f4ac05`, including Samba share/ACL trust-boundary checks and the updated unlocked-writer source extractor.
  - AI handoff documentation validation [42708503300] completed SUCCESS on exact SHA `258532a78922549698bc6d9b869c7777f8f4ac05`.
  - VSFTPD regression [42720752045] on exact SHA `6f15ad9a2580f9d0bba16447d61d315d5e8eb4e3` compiled successfully and passed config/daemon tests, then failed the ACL-path test because its symlink rejection assertion expected `ELOOP`; the policy correctly rejected earlier with `EINVAL` from `fstatat(AT_SYMLINK_NOFOLLOW)`. This test-contract mismatch is corrected in THIS COMMIT. AI handoff docs [42720752091] completed SUCCESS on that exact SHA.
  - Initial attempts on `1a7097a...` failed only in stale semantic workflow extractors; the new transaction host test itself passed. The assertions were corrected and later exact-SHA runs above passed. Do not report the initial failures as green.
- The exact-head DHCP callback and handoff checks on `6b44e023c2f669865fc8b5cb5425d824b8335d8b` were already SUCCESS: WAN DHCP callback [42703609156] and AI handoff docs [42703609087].
- Root `dev_log.md` has 82 complete records before this follow-up; this milestone makes 83/119. No archive rotation is due.
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

## Active task

1. The current bounded fix is RETR/SIZE descriptor-relative open/stat. It resolves existing final symlinks to their canonical target, pins the canonical parent with a verified dirfd, rechecks parent identity across the share ACL decision, and opens only the same regular-file inode with `openat(O_NOFOLLOW)`. SIZE uses an `O_PATH` descriptor so it does not newly require read permission on the file itself.
2. After this slice is exact-head green, continue FTP pathname TOCTOU work with descriptor-relative DELE/RMD/MKD/RNTO operations while preserving parent-versus-target ACL semantics. Then cover LIST/NLST recursion; MDTM and SITE CHMOD remain path-based metadata operations.
3. The earlier STOR/APPE/STOU cleanup `fstatat`/`unlinkat` pair remains non-atomic against a concurrent same-directory writer on Linux 4.4; preserve this limitation. FTP target config pins uClibc-ng 1.0.51/Linux 4.4, but no L3 target compile has been performed.
4. Keep IPTV DHCP callback status-loss, FULLCONENAT target kernel/MIPS build, and NFS `async`/AUTH_SYS/root-squash/external-mount compatibility questions in the global priority queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the bounded BusyBox DHCP event-state review, Samba paired publication/recovery, FTP upload dirfd anchoring, and RETR/SIZE descriptor-open control flow. |
| L2 host / GitHub Actions | VSFTPD run [42720752045] on exact SHA `6f15ad9a2580f9d0bba16447d61d315d5e8eb4e3` compiled the GNU89 host tests and passed config/daemon tests, but the ACL-path test failed only because its symlink-rejection assertion expected `ELOOP` instead of the helper's deliberate early `EINVAL`. AI-doc validation [42720752091] passed on that same SHA. The corrected regression and source contract in THIS COMMIT are pending. |
| L3 production RC/MIPS/uClibc/firmware builds and artifact validation | NOT VERIFIED. |
| L4 physical router behavior and hardware | NOT VERIFIED. |


## Exact first action for the next GPT

1. Fetch live `main`, branch list and Actions for the exact head; reconcile this errno-expectation correction's VSFTPD and AI-doc runs by exact `head_sha`.
2. If green, implement the next bounded FTP descriptor-relative mutation slice for DELE/RMD/MKD/RNTO while preserving existing ACL semantics; then assess LIST/NLST recursion.
3. Keep `handle_mdtm()` and `handle_site_chmod()` explicitly open as path-based operations, and preserve the non-atomic upload cleanup race plus IPTV DHCP callback, FULLCONENAT target-build and NFS compatibility follow-ups.


## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete records newest first; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
