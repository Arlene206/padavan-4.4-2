# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Current main SHA: `656e9819f1de186767738a6d49aedbd171f26ff0` (main is authoritative). Remote branch inventory: only `main`; no separate checkpoint branch exists.
- Exact-head Actions on `656e9819f1de186767738a6d49aedbd171f26ff0`: HTTPD Authentication and Security [42594767916](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767916) **SUCCESS**; AI handoff documentation [42594767915](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767915) **SUCCESS**. No storage-hotplug workflow run appeared for that head; THIS COMMIT adds explicit path triggers for the NFS script and RC header.
- The earlier NFS mount-inventory/restart-preflight change is already committed on this head and must not be repeated. The storage mount-inventory tri-state policy and HTTPD's NFS export validation are part of the completed baseline.
- Current active scope: NFS stop failure propagation and safe-eject gating. No local checkout is available; no local shell/compiler/Python/docs-validator run is claimed. Exact-head Actions for THIS COMMIT are pending. L3 production RC/MIPS-uClibc/firmware and L4 live NFS/storage/router validation remain NOT VERIFIED.

## Active task

NFS shutdown failure propagation before USB storage removal (P2 stability/data-integrity). The current script discarded process termination, `exportfs -ua`, and pseudo-filesystem unmount outcomes. `rm -f /var/lib/nfs/etab` could then erase recovery evidence after an incomplete stop. C `stop_nfsd()` discarded the shell status, `stop_stor_apps()` was `void`, and both safe-removal callers proceeded to unmount storage without proving NFS had stopped.

THIS COMMIT waits for both `rpc.mountd` and `nfsd`, escalates to SIGKILL only after a bounded graceful window, verifies process exit, checks export withdrawal, and uses a tri-state `/proc/mounts` helper (absent / present / unreadable) for `rpc_pipefs` and `nfsd`. It retains `etab` after any failed stage and gates the shell `restart` command on a successful stop. C propagates status through `stop_nfsd()`, the NFS restart notification and `stop_stor_apps()`; per-device and eject-all paths abort before storage unmount and release their owned resources/lock. The standalone `stop_nfsd` RC applet also returns failure. NFS start/reload partial-failure handling remains open.

### Completed adjacent work carried forward

- NFS export generation uses a complete six-field mount inventory and tri-state policy; the restart preflight checks inventory before stopping a working service. This is baseline and is not repeated in this commit.
- The exact main SHA `656e9819f1de186767738a6d49aedbd171f26ff0` passed HTTPD/security [42594767916](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767916) and AI handoff docs [42594767915](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767915).
- Samba atomic config publication and dependent HTTPD contracts are green in earlier history. Keep Samba passdb/account mutation, ACL/permissions and credential handling as a separate audit.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for NFS mount-inventory/restart-preflight baseline. THIS COMMIT adds daemon termination, export/pseudo-mount cleanup checks, status propagation and fail-closed storage eject; reviewed against exact base source blobs. |
| L2 host / GitHub Actions | Baseline exact SHA `656e9819f1de186767738a6d49aedbd171f26ff0`: HTTPD/security [42594767916](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767916) and AI docs [42594767915](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42594767915) SUCCESS. THIS COMMIT's storage-hotplug stop/lock regression, shell syntax and docs checks await exact-head Actions. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 live NFS export/stop, USB detach, filesystem integration / physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branches and Actions. Require the storage-hotplug and AI handoff-doc workflows to finish on the same exact full `head_sha` for THIS COMMIT; inspect the `sh -n` step and NFS stop/eject assertions and investigate any failed job.
2. After exact-head storage/docs checks pass, continue the NFS start/reload partial-failure audit: trace `func_start`, `func_reload`, `run_nfsd`, `reload_nfsd`, `write_nfsd_exports`, daemon/module setup and every caller. Propagate failures without stopping a healthy service before config and inventory preflight.
3. Then rotate to Optware/third-party service supervision and init ordering: arbitrary external S??* hooks, child/fd inheritance, status aggregation and storage-detach sequencing. Keep NFS export ACL/permission mapping and mount lifecycle open.
4. Keep Samba passdb/account mutation, ACL/permission and credential review open; keep vsftpd authentication/ACL/path-confinement/stop lifecycle open.
5. Preserve DHCP leasefail/accepted-NAK/release state semantics and BusyBox callback-status handling as a separate userland task. Keep FULLCONENAT mapping-hash and dying-tuple queue namespace ownership as a separate kernel audit. L3/L4 remain unverified.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their respective state materially changes; use Git history to retrieve older versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20–119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact; the validator continues checking their inventory and hashes.
- Update only records whose information changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for a changed module status, audit result, or global priority.
- Run `python3 scripts/check_ai_handoff_docs.py` when a local checkout is available. Inspect the exact-head Actions result after pushing. Do not make a follow-up docs-only commit solely to record a completed CI result; GitHub Actions is authoritative.
