# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Current main SHA: `d143581bf5f351cb2cf0d388f2542c3710802c7b` (main is authoritative). Remote branch inventory: only `main`; no separate checkpoint branch exists.
- Exact-head HTTPD Authentication and Security [42596369094](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369094) **SUCCESS** on `d143581bf5f351cb2cf0d388f2542c3710802c7b`. Its predecessor run [42596117899](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117899) failed on `9dd2f4a6eed24103617023fc93ea058f51466a0a` only because the NFS restart source contract still expected the old `void restart_nfsd` signature; commit d143581 aligned it with the new int-returning stop gate. AI handoff docs [42596369073](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596369073) are queued on d143581.
- On exact prior source SHA `9dd2f4a6eed24103617023fc93ea058f51466a0a`, storage hotplug [42596118456](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596118456), Samba [42596117948](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117948), vsftpd [42596117585](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117585), WAN route [42596117615](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117615) and AI docs [42596117462](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42596117462) all **SUCCESS**. The NFS stop/eject change is therefore validated at the workflow/source-contract level on that exact source SHA.
- The NFS mount-inventory/restart-preflight and shutdown/eject-gating work is complete at source/CI level and must not be repeated. Current bounded task: NFS start/reload status propagation, process/mount prerequisites and rollback. There is no local full checkout; no local production build or docs-validator run is claimed. L3 production RC/MIPS-uClibc/firmware and L4 live NFS/storage/router validation remain NOT VERIFIED.

## Active task

NFS start/reload status propagation and partial-start safety (P2 stability/data-integrity). `func_start()` previously ignored failures from rpcbind, NFS state creation, module/mount setup and export publication. It also tested only the later `rpc.mountd` status, so a failed `rpc.nfsd` followed by a successful mountd command could be reported as a successful service start. A failed mountd start could leave the kernel server active. The already-running and reload branches could also return success when reload prerequisites were missing or `exportfs -r` failed. C `run_nfsd()` and `reload_nfsd()` discarded `eval()` results.

THIS COMMIT makes NFS start fail closed on rpcbind, state-file/directory, module, procfs mount, export publication and RPC daemon failures; validates that each daemon actually appears after its start command; refuses to reset export recovery state when only one NFS daemon is alive; and rolls back a partial kernel-server start through the existing checked `func_stop()` path. A new tri-state mount helper distinguishes an absent NFS pseudo-filesystem from an unreadable `/proc/mounts`; the optional max-block-size tuning remains best-effort. Reload now requires rpcbind, both NFS daemons and an exports file, and propagates `exportfs -r` errors. C callers log nonzero start/reload command results. NFS stop/eject gating and mount-inventory preflight remain completed adjacent work.

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

1. Fetch live `main`, all remote branches and Actions; match every run to the exact full `head_sha`. Inspect the NFS start/reload regression plus HTTPD, storage and AI-doc runs for THIS COMMIT; correct failures without weakening contracts.
2. If the exact-head checks pass, rotate to third-party Optware startup supervision: inspect `opt-start.sh` service-hook enumeration, start-result aggregation, partial-start rollback, lock/fd ownership and retry behavior. Do not assume user-supplied hooks are trustworthy or present.
3. Keep NFS export ACL/permission mapping, external mount lifecycle and remaining target-specific stop behavior open; keep Samba passdb/account mutation, ACL/permission and credential review open; keep vsftpd authentication/ACL/path-confinement/stop lifecycle open.
4. Preserve DHCP leasefail/accepted-NAK/release semantics and BusyBox callback-status handling as a separate userland task. Keep FULLCONENAT mapping-hash and dying-tuple queue namespace ownership as a separate kernel audit. L3/L4 remain unverified.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their respective state materially changes; use Git history to retrieve older versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20–119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact; the validator continues checking their inventory and hashes.
- Update only records whose information changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for a changed module status, audit result, or global priority.
- Run `python3 scripts/check_ai_handoff_docs.py` when a local checkout is available. Inspect the exact-head Actions result after pushing. Do not make a follow-up docs-only commit solely to record a completed CI result; GitHub Actions is authoritative.
