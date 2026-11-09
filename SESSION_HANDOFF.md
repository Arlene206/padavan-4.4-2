# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed. Always fetch live main, all branches and exact-head Actions on takeover.

## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Live `main` base for the NFS IPv4 ACL validation fix: `3c879e06efa256ffbf376848ad69359a9098330e`; active branch: `main`; no separate checkpoint branch exists.
- The preceding storage contract correction is green: USB storage [42623404967](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42623404967) and AI handoff docs [42623405148](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42623405148) both completed SUCCESS with exact `head_sha=3c879e06efa256ffbf376848ad69359a9098330e`. It corrected only the stale caller-graph assertion; no production behavior changed.
- The NFS exact mount-tuple implementation previously passed NFS daemon start/reload [42622521213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521213) on exact SHA `eb21523750bc3218121f5adf3457c587d8e9a6e0`; NFS production source has not changed since that run. New NFS/storage/HTTPD/docs Actions are required for THIS commit.
- The dedicated NFS and USB storage caller-graph checks now both cover the exact pseudo-filesystem tuple and checked service stop/reload paths. Keep storage, Samba, FTP and other module coverage rotating.

## Active task

### Reject malformed or non-contiguous IPv4 ACL networks before NFS export publication

P2 security/reliability. During the NFS root/ACL audit, `write_nfsd_exports()` was found to build both LAN and VPN IPv4 client prefixes with `ip2class()`. That helper uses unchecked `inet_addr()` values and counts all set bits in the mask rather than validating that the mask is a contiguous prefix. The LAN path only called `is_valid_ipv4()`, which checks parseability, not contiguous netmask semantics; the VPN path passed `vpns_vnet` directly without validating it. A non-contiguous LAN mask could therefore be misrepresented as a different CIDR prefix, while malformed VPN network state could produce an unintended ACL. A new NFS-specific formatter rejects invalid input and creates a canonical network prefix.

### Implementation in THIS commit

- Add a strict dotted-quad IPv4 ACL formatter with canonical network calculation, contiguous-netmask validation, explicit rejection of `/0`, unspecified and all-ones addresses, and truncation checks.
- Use the same formatter for the configured LAN ACL and every enabled VPN ACL path. Invalid configured networks fail closed before the staged `/etc/exports` file is published; preserve the last known-good exports file.
- Leave the historical `async,insecure,no_root_squash,no_subtree_check` export options unchanged pending compatibility review. The security fix is limited to reliable, correctly scoped client network ACLs.
- Extend GNU89 host regression for canonical /24, /32 and /1 networks, non-contiguous masks, /0, malformed/injection-like input, unspecified/broadcast addresses and short output buffers. Extend NFS workflow source contracts to ensure the writer no longer calls the unchecked legacy helper.

### Files changed

- `trunk/user/rc/src/storage_nfs_export_policy.h`
- `trunk/user/rc/src/tests/storage_nfs_export_policy.c`
- `trunk/user/rc/src/services_stor.c`
- `.github/workflows/nfsd-start-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for validating both LAN and enabled VPN IPv4 ACLs, rejecting non-contiguous masks and unrestricted /0, canonicalizing the network prefix, and preserving prior exports on validation failure. |
| L2 host / Actions | Base `3c879e06efa256ffbf376848ad69359a9098330e`: USB storage run 42623404967 and AI docs 42623405148 SUCCESS on exact SHA. Dedicated NFS run 42622521213 SUCCESS on unchanged NFS source at exact SHA `eb21523750bc3218121f5adf3457c587d8e9a6e0`. This commit's GNU89 IPv4/IPv6 policy test, NFS/storage/HTTPD and AI-doc exact-head workflows are pending. |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 actual NFS AUTH_SYS/root mapping, NFS client access control and physical-router export lifecycle | NOT VERIFIED. |

No local complete repository checkout or local build of the production RC object is available. The actual helper/contract regression runs under the repository's GNU89 GitHub Actions; do not claim target compiler or firmware validation. No change is made yet to the legacy root-squash, transport-port or sync/async options; their compatibility and risk remain a separate review item.

## Exact first action for the next GPT

1. Fetch live `main`, all branches and every Actions run for the resulting full SHA. Verify the dedicated NFS IPv4/IPv6 GNU89 policy test, NFS lifecycle, USB storage caller graph, HTTPD security and AI handoff docs; inspect logs for failures.
2. A malformed LAN or enabled VPN ACL must prevent `/etc/exports` publication and NFS start/reload. Do not change this to silently omit an invalid ACL or to widen its network.
3. If green, continue the NFS options/identity compatibility review: `no_root_squash` and AUTH_SYS UID/GID mapping, `async` durability, `insecure` source ports, operator-provided `/etc/storage/exports` override semantics and external-mount behavior. Preserve the existing option tuple until an explicit compatibility decision is supported.
4. Then rotate to Samba passdb/account mutation, ACL/permission and credential security; keep vsftpd authentication/path confinement, DHCP leasefail/NAK/release callback state, FULLCONENAT queue namespace ownership and cross-module shutdown races open. L3/L4 remain unverified until actual target build and device tests.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their checkpoint, evidence, priorities or next action materially changes; use Git history for prior versions.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
