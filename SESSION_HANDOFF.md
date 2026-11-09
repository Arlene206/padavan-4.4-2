# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed. Always fetch live main, all branches and exact-head Actions on takeover.

## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Base `main` SHA for THIS Samba ACL correction: `6f9226283de4a2b2dce2b14a21efd7efb4a0fc2b`; active branch: `main`; only `main` exists, with no separate checkpoint branch. Resulting commit: THIS COMMIT.
- The preceding NFS IPv4 policy-header guard correction is now verified on this exact base SHA:
  - AI handoff documentation [42625376685](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376685): **SUCCESS**.
  - NFS daemon start/reload lifecycle [42625376591](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376591): **SUCCESS**.
  - HTTPD security [42625376585](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42625376585): **SUCCESS**.
  - Every listed run's full `head_sha` exactly equals `6f9226283de4a2b2dce2b14a21efd7efb4a0fc2b`.
- NFS option/identity source review is complete enough to confirm a material security exposure, but no compatibility-sensitive export option was changed:
  - `trunk/user/nfs-utils/Makefile` configures nfs-utils 2.6.1 with `--disable-nfsv4`, `--disable-nfsv41`, `--disable-gss`, `--without-gssglue`, and `--without-krb5`; its ROMFS install list contains neither Kerberos/GSS authentication daemons nor NFSv4 support.
  - `trunk/user/nfs-utils/nfsd.sh` starts the server with NFSv4 disabled. The normal generated export rule remains `async,insecure,no_root_squash,no_subtree_check`.
  - Kernel `trunk/linux-4.4.x/fs/nfsd/auth.c::nfsd_setuser()` initializes filesystem UID/GID directly from the RPC request credentials; root is remapped only when `NFSEXP_ROOTSQUASH` is set. With AUTH_SYS and `no_root_squash`, a client able to construct requests from an allowed LAN/VPN source can claim UID/GID 0 for exported removable-media files. This is a data-access trust-boundary exposure, not a claim of direct router-root compromise.
  - `async` weakens write durability across power loss; `insecure` accepts non-reserved client source ports. Both have compatibility/performance implications and remain unchanged pending an explicit compatibility decision.
  - Existing `/etc/storage/exports`, when present, is treated as the operator's complete authoritative export configuration and linked into `/etc/exports`; it intentionally bypasses generated LAN/VPN ACL and media-only path policy. The generated path currently exports only valid `/dev/sd*` (and configured MMC) storage mounted below `/media/`; external/network mounts do not qualify through this device check.
- This round addresses the handoff's next Samba ACL/permission item. No production firmware or physical-router test is claimed.

## Active task

### Preserve Samba write-only permissions in generated per-folder ACLs

P2 stability/permission correctness. `libdisk/src/disk_share.c::get_permission()` documents and returns the CIFS rights as 0=deny, 1=read, 2=write, 3=read/write; read errors or malformed values return -1. In `write_smb_conf()`, the `invalid users` list was populated whenever `samba_acl_right_can_read(right)` was false. That incorrectly placed right=2 accounts in the deny list even though the same account was subsequently written to `write list`. Samba's `invalid users` denies login; a write-list grant cannot restore access to a user explicitly denied at share login. The same read-only gate also failed to enable guest access / `valid users` membership when the guest's configured right was write-only.

### Implementation in THIS commit

- Add `samba_acl_right_is_denied()`: deny only when neither read nor write is granted; malformed/error rights remain fail-closed.
- For guest access and `valid users`, admit any supported grant (read, write, or read/write). Keep `read list` and `write list` strictly tied to their respective capabilities.
- Build `invalid users` only from rights that are truly denied; this preserves write-only accounts without weakening denial of 0, -1 or out-of-range values.
- Extend the GNU89 share-policy regression matrix and HTTPD security workflow's source contract to verify the helper is used in the production writer for guest and account ACLs.
- No NFS export options or passdb mutation behavior changed.

### Files changed

- `trunk/user/rc/src/services_stor.c`
- `trunk/user/rc/src/samba_share_policy.h`
- `trunk/user/rc/src/tests/samba_share_policy.c`
- `.github/workflows/httpd-security.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for rights mapping, guest/account writer call sites and fail-closed handling of unknown/error values; Samba documents that a user present in both valid and invalid lists is denied. |
| L2 host / Actions | The regression test and production-source contract are included in HTTPD security workflow; this commit's exact-head HTTPD, Samba, NFS/storage and AI-doc workflow results are PENDING until the new commit runs. No local full checkout was available, so no local compile/run or docs validator invocation is claimed. |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 live Samba guest/write-only behavior, NFS identity policy, storage/eject / physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, all branches, and every Actions run for the exact resulting commit SHA. Confirm the HTTPD security workflow compiles/runs `samba_share_policy.c` and checks the production `write_smb_conf()` integration; inspect all failures rather than relying on workflow names alone. Also confirm Samba config, NFS/storage and AI handoff docs runs if triggered.
2. If CI is green, continue NFS compatibility/security decision with the confirmed AUTH_SYS + `no_root_squash` trust boundary. Evaluate a compatibility-preserving migration toward root-squash, and assess `async`, `insecure`, operator override ACL ownership and external-mount support. Do not silently alter existing defaults.
3. Continue the still-open Samba passdb/account mutation consistency, credential failure/recovery and live reload audit; then vsftpd authentication/ACL/path confinement/stop lifecycle.
4. Keep DHCP leasefail/NAK/release callback state, FULLCONENAT mapping-hash/dying-tuple queue namespace ownership, and cross-module shutdown races in the repository-wide queue. L3/L4 remain outstanding.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their checkpoint, evidence, priorities or next action materially changes; use Git history for prior versions.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
