# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed. Always fetch live main, all branches and exact-head Actions on takeover.

## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Live `main` base for this preprocessor-guard correction: `ef1aec7b10389eaf240c20b5e999d80ef48996d1`; active branch: `main`; no separate checkpoint branch exists.
- The IPv4 NFS LAN/VPN ACL formatter and its tests were implemented in `ef1aec7b10389eaf240c20b5e999d80ef48996d1`. Exact-head Actions:
  - USB storage [42624781463](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781463): **SUCCESS**.
  - Samba [42624781449](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781449): **SUCCESS**.
  - vsftpd [42624781407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781407): **SUCCESS**.
  - AI handoff docs [42624781292](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781292): **SUCCESS**.
  - NFS daemon lifecycle [42624781415](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781415): **FAILURE** at the GNU89 policy compile; NFS shell syntax and start/reload lifecycle fixture passed before it.
  - HTTPD security [42624781560](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781560): **FAILURE** at the shared NFS export policy compile; all preceding HTTPD/storage/etc. checks in that job passed.
- All six full `head_sha` values exactly matched `ef1aec7b10389eaf240c20b5e999d80ef48996d1`. Failure logs identify a duplicate nested `#if defined (USE_IPV6)` in `storage_nfs_export_policy.h`, leaving the outer header guard unterminated. This is a preprocessing/build-contract defect, not a failing ACL assertion or NFS runtime fixture.

## Active task

### Finish strict NFS IPv4 LAN/VPN ACL validation and restore the policy header guard

P2 security/reliability. `write_nfsd_exports()` previously built LAN and VPN IPv4 ACLs using legacy `ip2class()`, which consumes unchecked `inet_addr()` values and counts all set netmask bits instead of checking for a contiguous prefix. The LAN's existing `is_valid_ipv4()` check validated parseability but not mask contiguity; the VPN path passed the configured network without equivalent validation. This could represent a non-contiguous mask as a different CIDR range.

The strict formatter and fail-closed callers are already in `ef1aec7b10389eaf240c20b5e999d80ef48996d1`, but the first exact-head compile exposed an extra nested IPv6 preprocessor guard introduced alongside the new unconditional IPv4 includes. The corrective commit removes only that redundant opening guard, leaving the existing IPv6 helper block and its closing guard intact.

### Implementation in THIS commit

- Remove the duplicate `#if defined (USE_IPV6)` directive in `storage_nfs_export_policy.h`; preserve exactly one guard around the existing IPv6-only helpers and keep the outer include guard balanced.
- Do not change the IPv4 formatter, LAN/VPN fail-closed export publication, or the historical `async,insecure,no_root_squash,no_subtree_check` option tuple.

### Files changed

- `trunk/user/rc/src/storage_nfs_export_policy.h`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for locating the duplicate nested guard and confirming the corrected preprocessor nesting is balanced; production IPv4 ACL semantics unchanged. |
| L2 host / Actions | Base `ef1aec7b10389eaf240c20b5e999d80ef48996d1`: storage, Samba, vsftpd and docs passed exact-head Actions. NFS and HTTPD failed at compilation of the same shared header with `unterminated #ifndef`; NFS shell syntax/lifecycle and preceding HTTPD checks passed. This correction's NFS/HTTPD/storage/docs exact-head reruns are pending. |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 NFS AUTH_SYS/root mapping, export permission behavior and physical-router lifecycle | NOT VERIFIED. |

No local complete checkout or target compiler run is claimed. The GitHub Actions GNU89 host compile is the current source-policy validation boundary; production RC object, MIPS/uClibc, firmware/TRX and router testing remain unverified.

## Exact first action for the next GPT

1. Fetch live `main`, all branches and every Actions run for its exact full SHA. Confirm NFS IPv4/IPv6 policy compile, NFS lifecycle, HTTPD's shared NFS policy compile, USB storage caller graph, Samba/vsftpd and AI docs; inspect all failures.
2. Do not weaken the strict contiguous-mask check, rejection of `/0`, or the fail-closed behavior for malformed LAN/VPN state.
3. Once all affected Actions are green, resume the NFS export option/identity audit: `no_root_squash` and AUTH_SYS UID/GID mapping, `async` durability, `insecure` client source ports, operator-provided `/etc/storage/exports` override semantics and external-mount support. Keep historical options unchanged until compatibility implications are understood.
4. Then rotate to Samba passdb/account mutation, ACL/permission and credential security. Keep vsftpd authentication/path confinement, DHCP leasefail/NAK/release callback state, FULLCONENAT queue namespace ownership and cross-module shutdown races open. L3/L4 remain unverified until actual target build and device tests.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their checkpoint, evidence, priorities or next action materially changes; use Git history for prior versions.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
