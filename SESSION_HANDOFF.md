# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Current main SHA: 'a816159737241f398baa7f827d753f7b99e8131b'; remote branch inventory contains only 'main', with no separate checkpoint.
- Exact-head storage [42597871309](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871309), HTTPD [42597871334](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871334), Samba [42597871360](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871360), vsftpd [42597871344](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871344) and AI docs [42597871472](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871472) all SUCCESS on a816159. NFS [42597871349](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42597871349) passed shell syntax and all runtime mocks; its Python source contract failed only because it expected numeric tests instead of the case statement that implements the mount-status tri-state. This follow-up fixes the assertion and adds rollback checks.
- NFS inventory/restart-preflight and stop/eject gates are complete at source/CI level. Active task is checked NFS start/reload and partial-start cleanup. No full local checkout, docs validator, production object/firmware build or physical router test is claimed; L3/L4 remain NOT VERIFIED.

## Active task
NFS start/reload status propagation and partial-start safety (P2 stability/data-integrity). Previously, 'func_start()' ignored rpcbind, state creation, module/mount and export publication failures; a failed rpc.nfsd could be masked by a later successful rpc.mountd, and a mountd failure could leave the kernel server running. Reload and already-running paths also ignored missing prerequisites/exportfs errors, while the C wrappers discarded eval status.

The current implementation checks rpcbind, prevents recovery-state reset when only one server daemon is alive, checks module and pseudo-filesystem mounts, checks export publication, verifies both RPC daemons after start, and rolls back partial daemon startup through checked func_stop(). Reload requires rpcbind, both daemons and /etc/exports. C callers log command errors. The first exact-head mock suite passed all runtime cases; its source assertion incorrectly expected numeric comparisons although the helper uses a tri-state case. This follow-up aligns the assertion and invokes cleanup after NFS control mount, rpc_pipefs mount or export-publication failures. If export withdrawal also fails, etab and pseudo-mounts remain as recovery evidence.
### Completed adjacent work carried forward

- NFS export generation uses a complete six-field mount inventory and tri-state policy; the restart preflight checks inventory before stopping a working service. This is baseline and is not repeated in this commit.
- The NFS shutdown/eject commit `9dd2f4a6eed24103617023fc93ea058f51466a0a` passed the USB storage hotplug regression [42596118456](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596118456); the storage workflow validated shell syntax and NFS stop/eject source gates. Its HTTPD inventory assertion was stale and was corrected in `d143581bf5f351cb2cf0d388f2542c3710802c7b`; the complete HTTPD/security workflow then passed on d143581 [42596369094](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369094).
- NFS export generation uses the complete six-field mount inventory and tri-state policy; restart preflight validates inventory before stopping the daemon. NFS stop preserves state on process/export/pseudo-mount cleanup failure and safe-eject callers refuse to detach storage after an incomplete stop.
- Samba atomic config publication and dependent HTTPD contracts are green in the preceding history. Keep Samba passdb/account mutation, ACL/permissions and credential handling as a separate audit.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for NFS mount-inventory/restart-preflight and shutdown/eject propagation. THIS COMMIT adds checked start/reload prerequisites, daemon process verification and partial-start rollback; reviewed against exact base source blobs. |
| L2 host / GitHub Actions | Base SHA `d143581bf5f351cb2cf0d388f2542c3710802c7b`: HTTPD/security [42596369094](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369094) SUCCESS; AI docs [42596369073](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369073) queued at takeover. Prior exact source SHA `9dd2f4a6eed24103617023fc93ea058f51466a0a`: storage [42596118456](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596118456), Samba [42596117948](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117948), vsftpd [42596117585](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117585), WAN route [42596117615](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117615) and docs [42596117462](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117462) SUCCESS. THIS COMMIT's NFS start/reload shell regression, HTTPD/storage and docs checks await exact-head Actions. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 live NFS export/stop, USB detach, filesystem integration / physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live main, all branches and Actions and match every result to the exact full SHA. Reconcile the NFS mock rollback/source contracts and dependent HTTPD, storage and AI-doc workflows on the new main head.
2. If green, rotate to Optware opt-start.sh startup supervision: hook enumeration, status aggregation, partial-start rollback, lock/fd ownership and retry behavior; external hooks are user-supplied and untrusted.
3. Keep NFS export ACL/permission mapping and external mount lifecycle open; keep Samba passdb/account/ACL/credential and vsftpd authentication/path-confinement/stop audits open.
4. Preserve DHCP leasefail/accepted-NAK/release and FULLCONENAT mapping-hash/queue namespace audits as separate tasks. L3/L4 remain unverified.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their respective state materially changes; use Git history to retrieve older versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20–119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact; the validator continues checking their inventory and hashes.
- Update only records whose information changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for a changed module status, audit result, or global priority.
- Run `python3 scripts/check_ai_handoff_docs.py` when a local checkout is available. Inspect the exact-head Actions result after pushing. Do not make a follow-up docs-only commit solely to record a completed CI result; GitHub Actions is authoritative.
