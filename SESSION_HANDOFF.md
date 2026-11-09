## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Base `main` SHA for THIS Samba passdb/configuration consistency change: `90e6c14d835216f06163f1347c0d5d8a3a465484`; active branch `main`. Only `main` exists; no separate checkpoint branch.
- The preceding custom NFS exports-path fail-closed change is complete on exact SHA `90e6c14d835216f06163f1347c0d5d8a3a465484`. All six latest Actions completed SUCCESS and every full `head_sha` exactly matched:
- Samba config [42635610400](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610400): **SUCCESS**.
- vsftpd [42635610401](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610401): **SUCCESS**.
- USB storage [42635610419](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610419): **SUCCESS**.
- AI handoff docs [42635610461](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610461): **SUCCESS**.
- NFS [42635610314](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610314): **SUCCESS**.
- HTTPD/security [42635610731](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42635610731): **SUCCESS**.
- The NFS fix permits generated exports only when `lstat("/etc/storage/exports")` reports `ENOENT`; direct regular files and symlinks to regular files remain authoritative; dangling links, inaccessible entries and non-regular paths fail closed before generated exports are staged. The NFS workflow rechecked RC status propagation and GNU89 policy.
- Generated NFS exports require `secure` source ports while retaining `no_root_squash` and `async`; root-squash compatibility, custom override authority/content validation, durability and external-mount policy remain separate open items. No firmware build or hardware validation is claimed.

## Active task

### Fail closed on an invalid custom NFS exports path

P2 reliability/security. In `services_stor.c::write_nfsd_exports()`, the old `check_if_file_exist()` helper followed symlinks with `stat()` and returned false for both an absent path and an existing non-regular/unresolvable path. A dangling `/etc/storage/exports` symlink, directory, FIFO, device, or inaccessible path could therefore be mistaken for “no custom configuration”; the ## Active task

### Keep Samba passdb and share configuration consistent on credential or config failure

P2 stability/security. In `run_samba()`, `write_smb_conf()` atomically published the new `/etc/smb.conf` **before** account provisioning. For Samba 3.6.25, `smbpasswd` writes into a staged `/etc/samba/.smbpasswd.*` and the old `smbpasswd` is intentionally preserved if any configured account fails. One invalid password, duplicate username or failed `smbpasswd` could therefore leave the new share ACL/config on disk while the old passdb remains active; a running daemon is not reloaded on that error, and the persisted pair is inconsistent for the next service start.

### Implementation in THIS commit

- Move Samba share-config publication until after all configured credentials have been provisioned successfully to the private staged passdb.
- Snapshot the previous private passdb with a same-directory hard link before replacing it. This preserves the original inode, mode and contents without copying credential hashes into a second file.
- Publish the fully provisioned passdb first, then publish the generated smb.conf using the existing same-directory staged/atomic writer.
- If smb.conf generation/publication fails, restore the previous passdb from the snapshot (or remove a new first-run database) before returning without daemon reload. If restoration itself fails, log a CRITICAL error and retain the snapshot pathname for manual recovery.
- Add GNU89 filesystem tests for snapshot/restore of an existing passdb, rollback of a first-run database, and rejection of non-regular/symlink database paths. Extend the Samba workflow to compile/run the tests and enforce production caller ordering.
- This is rollback for normal error returns, not a power-failure-atomic transaction across two files. A reset between passdb rename and smb.conf publication can leave the rollback snapshot; startup journal recovery remains a follow-up.

### Files changed

- `trunk/user/rc/src/services_stor.c`
- `trunk/user/rc/src/samba_passdb_policy.h`
- `trunk/user/rc/src/tests/samba_passdb_policy.c`
- `.github/workflows/samba-share-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for deferred config publication, private passdb snapshot and rollback ordering. Credential failures leave both active files untouched; ordinary config I/O failures attempt to restore the old passdb before service reload. |
| L2 host / Actions | Base SHA `90e6c14d835216f06163f1347c0d5d8a3a465484` exact-head Samba [42635610400], NFS [42635610314], HTTPD [42635610731], storage [42635610419], vsftpd [42635610401] and AI docs [42635610461] all SUCCESS with matching full `head_sha`. THIS commit's GNU89 passdb snapshot/restore regression and Samba source contract are pending exact-head Actions. |
| L3 MIPS/uClibc / production RC object / firmware/TRX | NOT VERIFIED. |
| L4 live Samba authentication/reload, interruption between the two file updates and physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, all branches and Actions for THIS commit's exact full SHA. Inspect the Samba passdb/config transaction job and HTTPD-dependent Samba account contracts; confirm every full `head_sha`.
2. If green, audit crash/power-loss recovery for the short interval after `smbpasswd` rename and before smb.conf publication. The snapshot is retained only when ordinary rollback fails; consider a durable journal/recovery gate before making any stronger atomicity claim.
3. Continue the Samba reload boundary: `doSystem(killall -SIGHUP)` and `eval(/sbin/smbd|nmbd)` statuses are ignored; trace their callers/stop semantics and add postcondition checks without duplicating completed ACL/config staging fixes.
4. Then rotate to vsftpd authentication/ACL/path confinement/stop lifecycle; preserve NFS custom-export content/ACL override and durability, DHCP leasefail/NAK/release, and FULLCONENAT namespace ownership in the broader userland queue.

ed earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
