## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; never substitute the upstream repository.
- Base `main` SHA for THIS FTP metadata-boundary batch: `071739a00401b1b229bb79613d30bdd4c452faa4`. Only `main` exists; there is no separate checkpoint branch.
- Parent exact-head Actions are both green on `071739a00401b1b229bb79613d30bdd4c452faa4`: vsftpd [42665789660] SUCCESS and AI handoff docs [42665789545] SUCCESS. Both full `head_sha` values equal `071739a00401b1b229bb79613d30bdd4c452faa4`; the vsftpd GNU89 path helper regression and semantic source contracts passed, and the documentation/legacy archive integrity validator passed.
- Root `dev_log.md` has 64 complete newest-first records before THIS entry, remains well below the 384 KiB early-rotation target, and does not need archive rotation.

## Work completed in THIS patch

- The prior RNFR/RNTO correction pins the source path absolutely, checks canonical source-parent delete permission and rechecks source/destination rights before rename.
- The previous path-ACL batch fixed CWD-versus-argument permission checks for STOR/APPE/STOU, RETR, MKD/RMD/DELE and LIST/NLST/STAT. It preserves operation-specific symlink behavior: upload and RETR resolve existing final symlinks to their effective target; new upload leaves use their canonical parent; directory listings check the actual opened directory; MKD/RMD/DELE check entry parents.
- This batch extends path-aware authorization to SIZE (PERM_READ), MDTM (PERM_WRITE for an enabled timestamp mutation, otherwise PERM_READ) and SITE CHMOD (PERM_WRITE). Each follows the final target, so ACL evaluation uses the canonical existing target's parent.
- Found that `trunk/user/libdisk/src/disk_share.c` stores `.__folder_list.txt` and `.__<account>_var.txt` at the USB mount root. That is MOUNT_LAYER, where per-share READ ACLs do not apply. Direct FTP file commands could expose the account/folder ACL database even though mount-root file entries were not shown in normal directory listings.
- Added a shared exact-name classifier for only the folder inventory and per-account `_var.txt` sidecars. All path-scoped permission helpers reject those names before the `chroot_local_user` bypass, and directory visibility hides them at every layer. Existing-file and upload resolvers also classify the canonical `realpath()` target, blocking a non-reserved symlink alias to a reserved sidecar. Ordinary unrelated `.__notes.txt` files are not matched by this classifier.
- Extended the GNU89 host regression with positive/negative metadata-name cases, direct reserved-sidecar denial, and symlink-alias denial. The workflow source contract now covers SIZE/MDTM/CHMOD ordering and all metadata guards.
- The pathname model still has a symlink-swap TOCTOU window between `realpath()` authorization and the later pathname syscall. Dirfd/openat-style operation binding remains a separate hardening task.

## Active task

1. Verify all Actions triggered by THIS commit against the exact full new main SHA. Inspect both GNU89 host regression output and semantic source-contract/documentation logs.
2. Broaden governance beyond FTP. Priority candidates remain NFS custom override content/ACL bypass and `async` durability; DHCP leasefail/accepted-NAK/release semantics; and FULLCONENAT mapping-hash / dying-tuple queue namespace ownership. Compare impact and reproducibility, then trace the complete caller/callee/state/error/cleanup path for the next bounded fix.
3. Keep FTP pathname TOCTOU hardening, authentication/account/anonymous semantics and target runtime validation visible as remaining work; do not call the current pathname checks race-free.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for target-path ACL enforcement across STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT, SIZE, MDTM and SITE CHMOD; internal share ACL sidecars are blocked via direct names and canonical symlink targets, and hidden from listings. |
| L2 host / Actions | Parent `071739a00401b1b229bb79613d30bdd4c452faa4`: exact-head vsftpd [42665789660] and AI docs [42665789545] SUCCESS. THIS batch adds metadata-name and symlink-alias GNU89 regression cases plus SIZE/MDTM/CHMOD source contracts; exact-head validation is pending. |
| L3 MIPS/uClibc / production vsftpd object / firmware/TRX | NOT VERIFIED. |
| L4 live FTP cross-share ACL, direct metadata disclosure, authentication/anonymous policy, hot-unplug and router validation | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch list and Actions. Verify each workflow's full `head_sha` equals the new main SHA and inspect every triggered job/log.
2. If exact-head CI is green, move to the highest-priority non-FTP stability issue. Start with a complete NFS custom-override path/content/ACL and `async` durability trace; compare with DHCP callback-state semantics and FULLCONENAT mapping-hash/dying-tuple namespace ownership.
3. Preserve FTP pathname TOCTOU, authentication/anonymous and target validation residuals on the queue.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update it only when the resumable task/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain the newest 20. Use 384 KiB as the early batch-rotation target and 512 KiB as the hard limit.
- Preserve existing archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
