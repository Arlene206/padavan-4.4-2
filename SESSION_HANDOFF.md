# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed. Always fetch live main, all branches and exact-head Actions on takeover.

## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Base main SHA for this NFS IPv6 export ACL hardening: `5faded68510e91fb526dea6ab3ebd34b5d9dc252`; resulting commit: THIS COMMIT.
- At recovery, only remote branch was `main`.
- The preceding MTD Optware retry/shutdown fix `5faded68510e91fb526dea6ab3ebd34b5d9dc252` passed on its exact full SHA:
  - USB storage/hotplug [42618640557](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640557) SUCCESS.
  - AI handoff docs [42618640385](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640385) SUCCESS.
  - Samba [42618640482](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640482) SUCCESS.
  - WAN route [42618640381](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640381) SUCCESS.
  - HTTPD [42618640379](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640379) SUCCESS.
- All five runs' full `head_sha` values matched the preceding main SHA. The MTD work was completed and is no longer the active task.

## Active task

### Validate IPv6 client ACL data before publishing NFS exports

P2 security/reliability. `write_nfsd_exports()` previously interpolated raw NVRAM `ip6_lan_addr` and `ip6_lan_size` values directly into `/etc/exports`. IPv6 parsing elsewhere in `net6.c` validates the address, but the NFS export writer bypassed it. Malformed values could produce invalid exports or inject NFS export syntax into the staged file.

### Implementation in THIS commit

- Add a bounded policy helper, compiled only with `USE_IPV6`, that parses the address with `inet_pton(AF_INET6)`, emits canonical text using `inet_ntop(AF_INET6)`, and accepts only decimal LAN prefix lengths 48–64, matching `reset_lan6_vars()`'s established range.
- Preserve the optional CIDR suffix accepted by the existing IPv6 parser, but use the separate `ip6_lan_size` value as the authoritative NFS export prefix.
- Emit only the canonical validated `address/prefix` string. An empty address remains “no IPv6 ACL”; malformed non-empty IPv6 configuration aborts publication, removes the staged temporary file and leaves the previous `/etc/exports` intact.
- Add GNU89 host regression cases for canonicalization, optional CIDR input, absent ACL, prefix bounds, malformed addresses and export-syntax injection payloads. Extend the focused NFS Action to compile/run the policy and assert the production writer never interpolates raw NVRAM fields.

### Files changed

- `trunk/user/rc/src/services_stor.c`
- `trunk/user/rc/src/storage_nfs_export_policy.h`
- `trunk/user/rc/src/tests/storage_nfs_export_policy.c`
- `.github/workflows/nfsd-start-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for NVRAM-to-export validation, canonical output, allowed prefix range and fail-closed staged-file cleanup. The custom operator exports symlink path remains a separate intentional override. |
| L2 host / Actions | PENDING for THIS COMMIT's NFS GNU89 IPv6 policy compile/run, export-writer/source contracts, existing NFS start/reload shell regression, storage/HTTPD caller contracts and AI handoff-doc validator. The exact base SHA `5faded68510e91fb526dea6ab3ebd34b5d9dc252` passed the five Actions listed above. |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 Actual NFSv3/v4 IPv6 client access, UID/GID/root mapping, exports reload and removable-disk lifecycle on router hardware | NOT VERIFIED. |

No production C object, MIPS/uClibc, full firmware/TRX, live NFS exportfs, NFS client interoperability or physical-router test is claimed. NFS still uses the existing hardcoded `async,insecure,no_root_squash,no_subtree_check` options; do not change these defaults without a separate compatibility/permission analysis.

## Exact first action for the next GPT

1. Fetch live `main`, all branches and latest Actions. Match the NFS start/export, USB storage, HTTPD and AI docs runs to THIS commit's exact full SHA; inspect every job and failing step.
2. Fix any exact-head regression before selecting a different task. Do not weaken address parsing or the fail-closed publication contract.
3. Once green, continue the NFS audit with root UID/GID and export permission semantics (`no_root_squash`, AUTH_SYS, `async`, `insecure`), and trace intended external-mount export support. Review `nfsd.sh`'s `grep`-based mountpoint matching for exact-path/regex false matches. Preserve existing user compatibility unless behavior is explicitly configured and tested.
4. Keep Samba passdb/account/ACL/credential security, vsftpd authentication/ACL/path confinement, DHCP leasefail/NAK/release callback state, FULLCONENAT queue namespace ownership and cross-module shutdown races open. L3/L4 remain unverified until target build and device tests actually run.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their checkpoint, evidence, priorities or next action materially changes; use Git history for prior versions.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
