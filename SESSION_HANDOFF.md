# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed. Always fetch live main, all branches and exact-head Actions on takeover.

## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Base `main` SHA for THIS custom NFS override fail-closed change: `a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`; active branch: `main`. The remote branch list contained only `main`; no separate checkpoint branch existed at recovery.
- The preceding generated NFS source-port correction is complete on exact SHA `a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`. All six runs completed SUCCESS and every full `head_sha` exactly matched:
  - Storage [42631443686](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443686).
  - NFS [42631443757](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443757).
  - vsftpd [42631443784](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443784).
  - Samba [42631443514](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443514).
  - AI handoff docs [42631443719](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443719).
  - HTTPD [42631443747](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443747).
- The preceding NFS correction changed only the generated defaults from `async,insecure,no_root_squash,no_subtree_check` to `async,secure,no_root_squash,no_subtree_check`. It retains `no_root_squash` for existing root-client compatibility but restores the kernel's reserved-source-port check. This is not AUTH_SYS authentication or root-squash protection.
- NFS is built/configured as NFSv2/v3 AUTH_SYS; NFSv4, GSS and Kerberos are disabled. Linux 4.4 `nfsd_setuser()` accepts client UID/GID and remaps UID/GID 0 only under `NFSEXP_ROOTSQUASH`; `nfsd_setuser_and_check_port()` rejects unprivileged source ports unless `NFSEXP_INSECURE_PORT` is set.
- Existing valid `/etc/storage/exports` is a complete authoritative export configuration linked to `/etc/exports`; it bypasses generated LAN/VPN ACL and local-media path restrictions and must be treated as trusted root configuration.
- No full firmware build or physical-router validation is claimed.

## Active task

### Fail closed on an invalid custom NFS exports path

P2 reliability/security. In `services_stor.c::write_nfsd_exports()`, the old `check_if_file_exist()` helper followed symlinks with `stat()` and returned false for both an absent path and an existing non-regular/unresolvable path. A dangling `/etc/storage/exports` symlink, directory, FIFO, device, or inaccessible path could therefore be mistaken for “no custom configuration”; the writer then generated broader default media exports instead of refusing to start/reload NFS.

### Implementation in THIS commit

- Classify the path with `lstat()`; follow it with `stat()` only when the entry is a symlink.
- Permit generated defaults only when `lstat()` fails specifically with `ENOENT`.
- Accept a direct regular file or a symlink whose target is regular. Fail closed on other `lstat` errors, dangling/looping symlinks, symlinks to non-regular targets, and direct non-regular entries.
- Return before reading `/proc/mounts` or staging `/etc/exports` on invalid path state, preserving the previously published NFS configuration. The existing valid-override symlink publication remains unchanged.
- Add GNU89 classification cases for absent, permission/parent errors, direct regular file, symlink to regular file, dangling/error symlink, symlink to non-regular target, and direct non-regular entry. Extend the NFS workflow's source contract.
- Root-squash remains a separate compatibility decision: the current automount policy gives FAT/exFAT/NTFS/HFS broad `umask=0` access, while ext2/3/4 retain POSIX ownership/mode semantics. Enabling root-squash can therefore change root-client writes and access to root-owned ext files; do not silently change it.
- Keep `async` unchanged in this bounded fix. It trades write durability under power loss/device failure against throughput; a separate evidence/benchmark-backed decision is required.
- Generated exports still target supported local block-device filesystems mounted under `/media/`; external/network mounts require an explicit trusted custom export file. `nfsd.sh` checks `exportfs -r` and reports start/reload failures; malformed regular custom-file contents are not auto-replaced by defaults.

### Files changed

- `trunk/user/rc/src/services_stor.c`
- `trunk/user/rc/src/storage_nfs_export_policy.h`
- `trunk/user/rc/src/tests/storage_nfs_export_policy.c`
- `.github/workflows/nfsd-start-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the old false/absent conflation and new fail-closed state matrix. The implementation preserves direct and symlink-to-regular custom files and does not stage generated exports for invalid path states. |
| L2 host / Actions | Focused standalone GNU89 classification regression compiled with `-Wall -Wextra -Werror` and passed. Full repository policy test, NFS workflow source contract, HTTPD dependent contract and AI documentation workflow are pending on THIS commit's exact SHA. No full local checkout or complete production source compilation is claimed. |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 NFS client behavior, filesystem permissions, storage failure recovery / physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, all branches and Actions for THIS commit's exact full SHA. Confirm NFS, HTTPD, storage and AI docs runs, inspect job conclusions and ensure each `head_sha` exactly matches.
2. If green, rotate to Samba passdb/account mutation consistency: trace account create/change/delete callers, plaintext credential handling, private passdb replacement/rollback, partial failure and service reload state. Avoid repeating the completed write-only ACL or config-publication fixes.
3. Keep the following NFS follow-ups distinct and open: custom regular-file content/error recovery, `async` durability versus throughput, `no_root_squash` compatibility/opt-in migration, authoritative override ACL/path bypass, and support expectations for external mounts.
4. Continue afterward with vsftpd authentication/ACL/path confinement/stop lifecycle; preserve DHCP leasefail/NAK/release callback semantics and FULLCONENAT namespace ownership in the global audit queue.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their checkpoint, evidence, priorities or next action materially changes; use Git history for prior versions.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete records verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
