# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed. Always fetch live main, all branches and exact-head Actions on takeover.

## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Base `main` SHA for THIS generated NFS source-port policy change: `010d5ebec0926cb52686b5c24f145d33a06514d7`; active branch: `main`. The remote branch list contains only `main`; no separate checkpoint branch exists.
- The preceding Samba write-only ACL correction is complete. All six Actions on exact SHA `010d5ebec0926cb52686b5c24f145d33a06514d7` passed, and every full `head_sha` matched:
  - Samba [42628532465](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532465): **SUCCESS**.
  - HTTPD [42628532589](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532589): **SUCCESS**.
  - NFS [42628532555](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532555): **SUCCESS**.
  - Storage [42628532417](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532417): **SUCCESS**.
  - vsftpd [42628532559](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532559): **SUCCESS**.
  - AI docs [42628532588](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42628532588): **SUCCESS**.
- The prior NFS IPv4 LAN/VPN CIDR and policy-header correction was also verified on exact base SHA `6f9226283de4a2b2dce2b14a21efd7efb4a0fc2b`: NFS [42625376591](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376591), HTTPD [42625376585](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376585), and AI docs [42625376685](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376685) all SUCCESS.
- NFS compatibility facts recovered from this fork:
  - `trunk/user/nfs-utils/Makefile` builds nfs-utils 2.6.1 with NFSv4, NFSv4.1, GSS and Kerberos disabled; `nfsd.sh` also starts nfsd with NFSv4 disabled. The configured service is NFSv2/v3 using AUTH_SYS.
  - Generated exports currently use `async,insecure,no_root_squash,no_subtree_check`.
  - Linux 4.4 `fs/nfsd/auth.c::nfsd_setuser()` assigns the request UID/GID directly to filesystem credentials and only maps UID/GID 0 when `NFSEXP_ROOTSQUASH` is set. `fs/nfsd/nfsfh.c::nfsd_setuser_and_check_port()` rejects a non-reserved client source port unless `NFSEXP_INSECURE_PORT` is set.
  - The unsafe `insecure,no_root_squash` pair therefore let an unprivileged process on an ACL-authorized LAN/VPN client network submit AUTH_SYS UID 0 from a high source port and operate as server root on writable exported removable-media data. This is a data-access trust-boundary exposure, not evidence of direct router-root compromise.
  - `/etc/storage/exports`, when present, is the operator's complete authoritative export file and is linked into `/etc/exports`; it intentionally bypasses generated LAN/VPN ACL and media-only path restrictions. Generated exports otherwise include supported local storage devices mounted under `/media/`; external/network mounts do not qualify through the device check.
- No production firmware or physical-router validation is claimed.

## Active task

### Require reserved source ports for generated NFS AUTH_SYS exports without changing root ownership semantics

P2 security/compatibility. The old generated rule combined `insecure` (which disables the kernel's reserved-source-port check) with `no_root_squash` (which maps client-supplied UID 0 to server root). Because this build uses AUTH_SYS without GSS/Kerberos, that allowed a normal unprivileged process on an admitted LAN/VPN client to claim UID 0 using a non-reserved source port and write as root to writable removable-media exports.

### Implementation in THIS commit

- Define generated export options as `async,secure,no_root_squash,no_subtree_check`; make `write_nfsd_exports()` consume this shared constant instead of embedding the old option tuple.
- Replace only `insecure` with `secure`. This restores the reserved-source-port barrier while preserving the historic `no_root_squash` UID/GID mapping for legitimate root-based clients; root-squash migration remains a separate compatibility decision because it changes ownership/access on existing files.
- Keep `async` unchanged pending a separate durability-versus-throughput evaluation, and keep `no_subtree_check`.
- Preserve the existing `/etc/storage/exports` override and its staged symlink publication. Administrators who intentionally require non-reserved source ports can still use a custom export file, but that override is authoritative and can also reintroduce the old security risk; its contents must be treated as trusted root configuration.
- Extend the GNU89 NFS policy test and workflow source contract to assert the exact default, require `secure`, forbid `insecure` in generated options, retain `no_root_squash` intentionally, and preserve custom override precedence.

### Files changed

- `trunk/user/rc/src/storage_nfs_export_policy.h`
- `trunk/user/rc/src/services_stor.c`
- `trunk/user/rc/src/tests/storage_nfs_export_policy.c`
- `.github/workflows/nfsd-start-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the Linux 4.4 AUTH_SYS UID/GID assignment and reserved-source-port check; the generated export writer now selects explicit `secure` while preserving the legacy root UID mapping. |
| L2 host / Actions | Base SHA `010d5ebec0926cb52686b5c24f145d33a06514d7`: Samba [42628532465], HTTPD [42628532589], NFS [42628532555], storage [42628532417], vsftpd [42628532559], and AI docs [42628532588] all PASS on matching full `head_sha`. THIS change's GNU89 test, NFS/HTTPD source contracts and documentation workflow are pending exact-head Actions. No local complete checkout or local compiler run is claimed. |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 actual AUTH_SYS client behavior, root-client ownership compatibility, external mounts and physical router | NOT VERIFIED. |

This is a bounded mitigation, not a claim that AUTH_SYS is authenticated or that client root is squashed: `no_root_squash` intentionally remains for compatibility. The operator export override can bypass all generated ACL and path restrictions and remains a separate trust-boundary audit item.

## Exact first action for the next GPT

1. Fetch live `main`, every branch and every Actions run for the exact full SHA created by THIS commit. Confirm the NFS GNU89 option regression, NFS lifecycle/source contract, HTTPD's dependent NFS policy compile/contract, and AI documentation validation; inspect full job conclusions and exact `head_sha`.
2. If exact-head Actions are green, make the separate compatibility decision on `root_squash`: identify root-client use cases and whether a visible opt-in or migration path is needed before changing root UID/GID mapping. Do not represent the current `secure,no_root_squash` policy as full root-squash protection.
3. Continue NFS `async` durability versus throughput analysis; audit custom `/etc/storage/exports` behavior for absent, dangling-symlink, unreadable and malformed files, `exportfs` failure recovery, and external mounts. Preserve the documented distinction that the override bypasses the generated LAN/VPN ACL and media path filters.
4. Then resume Samba passdb/account mutation consistency, credential failure/recovery and live reload; rotate to vsftpd authentication/ACL/path confinement/stop lifecycle. Keep DHCP leasefail/NAK/release callback state, FULLCONENAT queue namespace ownership and cross-module shutdown races in the whole-repository queue.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their checkpoint, evidence, priorities or next action materially changes; use Git history for prior versions.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
