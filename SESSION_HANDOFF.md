## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; never substitute the upstream repository.
- Base `main` SHA for THIS workflow-contract correction: `30d64c73546db236c12679d24eda74c1ea9f2916`. Only `main` exists; no separate checkpoint branch.
- The preceding target-path ACL commit `071739a00401b1b229bb79613d30bdd4c452faa4` passed both exact-head workflows: vsftpd [42665789660] and AI handoff docs [42665789545].
- Current second-batch commit `30d64c73546db236c12679d24eda74c1ea9f2916` produced exact-head docs SUCCESS [42667318504]. Its vsftpd run [42667318607] compiled all GNU89 regression binaries and ran them successfully, but the Python semantic-contract step failed with `NameError: acl_path is not defined`: the new ACL helper bodies were referenced before extraction. THIS commit only corrects the workflow declaration order and refreshes the continuation record; production C source and host tests are unchanged.
- Root `dev_log.md` has 65 complete newest-first records before THIS entry; it is under 384 KiB and does not need archive rotation.

## Completed FTP work

- Existing RNFR/RNTO fix: absolute source-path pinning plus source-delete/destination-write ACL revalidation against canonical parents.
- Path-aware checks now cover STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT, SIZE, MDTM and SITE CHMOD. Upload/RETR/metadata operations authorize the effective existing symlink target; MKD/RMD/DELE authorize the entry parent; listings authorize the actual opened directory before transfer setup.
- Added fail-closed handling for dangling final upload symlinks and canonical resolution of symlinked parent directories.
- Sidecar disclosure finding: `trunk/user/libdisk/src/disk_share.c` creates `.__folder_list.txt` and `.__<account>_var.txt` under the USB mount root. Per-share READ ACLs do not govern MOUNT_LAYER, so direct FTP file commands could expose folder/account ACL metadata. Exact-name filtering now hides and rejects these reserved files before the chroot ACL bypass.
- Both existing-file and upload path resolvers also classify the canonical `realpath()` target. Thus a harmless-looking final symlink alias to an ACL sidecar is denied as well as the direct filename. Ordinary `.__notes.txt` user files do not match the classifier.
- GNU89 regression now covers positive/negative metadata filenames, direct sidecar denial, final symlink alias denial, cross-share symlinks, dangling links, new leaves and symlinked directories.
- The only failure on `30d64c73546db236c12679d24eda74c1ea9f2916` is the CI Python variable-order defect described above; host binaries themselves all passed. The workflow correction moves all helper-body extraction before assertions.

## Active task

1. Verify the exact full SHA of THIS correction commit on every triggered workflow. Confirm vsftpd GNU89 test step PASS and semantic source-contract step PASS; docs validator must also pass at the same full head SHA.
2. Then continue the whole-repository audit outside FTP, beginning with NFS custom exports. Initial inspection found:
   - `write_nfsd_exports()` accepts `/etc/storage/exports` if it is a regular file or symlink to a regular file; only ENOENT enables generated exports.
   - Custom content is linked into `/etc/exports` without checking ownership, mode, content scope, or NFS options. Determine who can create/write `/etc/storage/exports` and whether it is an intentional trusted operator override before treating this as a security bug.
   - Generated options are `async,secure,no_root_squash,no_subtree_check`; source comments state `secure` is retained to stop unprivileged source ports claiming UID 0 under AUTH_SYS, while `no_root_squash` is preserved for compatibility.
   - Generated exports validate LAN/VPN IPv4/IPv6 ACLs, mount inventory and staging errors. `nfsd.sh` has explicit start/reload/stop checks and retains state when daemon termination fails.
   - Inspect web UI/config writer, `/etc/storage` provisioning and permission modes, NFS shell tests/workflows, exportfs semantics, and `/etc` tmpfs behavior before deciding the next bounded fix.
3. Keep DHCP leasefail/accepted-NAK/release state and FULLCONENAT mapping-hash/dying-tuple namespace ownership visible on the whole-repository queue. Keep FTP pathname realpath-to-syscall TOCTOU and target validation residuals too.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for path-target ACL guards across the above FTP command families and exact reserved-sidecar name/canonical-target denial. Pathname symlink-swap TOCTOU remains. |
| L2 host / Actions | Parent `30d64c73546db236c12679d24eda74c1ea9f2916`: GNU89 compilation and all host regression binaries PASS; docs action [42667318504] PASS; vsftpd semantic contract failed only due to `acl_path` being referenced before assignment. THIS correction moves extraction before use; exact-head rerun pending. |
| L3 MIPS/uClibc / production vsftpd object / firmware/TRX | NOT VERIFIED. |
| L4 live FTP ACL/account/anonymous tests, NFS clients and physical router validation | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch current `main`, branches and Actions. Verify the new full SHA matches every run; inspect steps/logs rather than trusting summary conclusions.
2. If green, resume NFS override trust-boundary investigation described above and make a focused host/source test if a reproducible flaw is confirmed.
3. Then rank DHCP callback-state semantics vs FULLCONENAT tuple namespace ownership by severity and reproducibility.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update only when resumable work/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain newest 20. Use 384 KiB as early-rotation target and 512 KiB as hard limit.
- Preserve archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
