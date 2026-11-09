# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed. Always fetch live main, all branches and exact-head Actions on takeover.

## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Base main SHA for this NFS mountpoint lifecycle fix: `363f2e4101bd8087e25710e875a4719023af6637`; resulting commit: THIS COMMIT.
- Live branch inventory before this change: only `main`; current base main SHA was fetched directly from GitHub.
- The preceding IPv6 export ACL hardening commit `363f2e4101bd8087e25710e875a4719023af6637` passed all six exact-head Actions: NFS [42620760496](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760496), USB storage [42620760576](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760576), HTTPD [42620760658](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760658), vsftpd [42620760715](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760715), AI handoff docs [42620760726](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760726), and Samba [42620760733](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42620760733). Every full `head_sha` exactly matched the base SHA.
- The IPv6 ACL validation task is complete; this round rotates within NFS to its pseudo-filesystem mount lifecycle.

## Active task

### Refuse conflicting or ambiguous NFS pseudo-filesystem mountpoints

P2 stability. `nfsd.sh` previously used `grep -q " $mountpoint " /proc/mounts` for both startup and teardown. It checked only the path, not the mounted source or filesystem type. A foreign filesystem at `/proc/fs/nfsd` or `/var/lib/nfs/rpc_pipefs` could be accepted as the NFS pseudo-filesystem; shutdown/rollback could then unmount that unrelated filesystem. Duplicate/stacked entries were also treated as an ordinary single mount.

### Implementation in THIS commit

- Add a shared exact-tuple inventory helper that matches `/proc/mounts` fields by literal mountpoint, expected source and filesystem type using AWK field equality, not a regular-expression grep.
- Treat an unused mountpoint as absent, exactly one matching row as mounted, and mismatched/duplicate rows or inventory-command errors as conflicts. Refuse to mount over a conflict.
- After a successful mount command, re-read `/proc/mounts` and require the expected tuple to have appeared. During stop/rollback, unmount only one exact expected tuple, then verify the mountpoint is empty; preserve any foreign or ambiguous entry and return failure.
- Extend the NFS lifecycle fixture so mocked mounts record the actual source/type. Add wrong-source, wrong-filesystem and stacked-mount cases; extend the workflow source contract to verify exact tuple matching and guarded teardown.

### Files changed

- `trunk/user/nfs-utils/nfsd.sh`
- `trunk/user/nfs-utils/tests/nfsd_start_lifecycle.sh`
- `.github/workflows/nfsd-start-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for literal mountpoint/source/filesystem matching, ambiguous-entry rejection, post-mount verification, and fail-closed teardown that preserves foreign/stacked mounts. |
| L2 host / Actions | Local focused shell regressions for absent/correct/wrong-source/wrong-type/duplicate tuples and guarded unmount passed. The NFS start/reload fixture and source-contract workflow, plus AI handoff documentation validation, are PENDING for THIS COMMIT's exact head. The base SHA `363f2e4101bd8087e25710e875a4719023af6637` passed all six Actions listed above. |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 NFS mount-stack behavior on the router, NFS client interoperability, storage removal and physical router lifecycle | NOT VERIFIED. |

No complete local checkout, production RC object, MIPS/uClibc build, complete firmware/TRX or physical-router test is claimed. The mount inventory check and subsequent mount/unmount are not an atomic kernel transaction; the reserved NFS mountpoints and source/type tuple are fixed, and conflicts fail closed. Existing export options `async,insecure,no_root_squash,no_subtree_check` remain unchanged pending the separate compatibility/permission audit.

## Exact first action for the next GPT

1. Fetch live `main`, all branches and the latest NFS/AI Actions. Match each run's `head_sha` to THIS commit's exact full SHA and inspect all test steps; fix any failure before moving on.
2. Do not weaken the exact mount tuple, duplicate-entry rejection or fail-closed unmount contract if a workflow catches a regression.
3. Once green, finish the NFS root UID/GID/export permission and security review (`no_root_squash`, AUTH_SYS, `async`, `insecure`), custom exports override semantics and external-mount support. Keep the existing options unchanged until compatibility impact is demonstrated.
4. Then rotate to Samba passdb/account mutation, ACL/permission and credential security. Keep vsftpd authentication/path confinement, DHCP leasefail/NAK/release callback state, FULLCONENAT namespace ownership and cross-module shutdown races open. L3/L4 remain unverified until target build and device tests actually run.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their checkpoint, evidence, priorities or next action materially changes; use Git history for prior versions.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
