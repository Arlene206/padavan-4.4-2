# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base main SHA for THIS deferred Optware source-contract correction: `a154f30da5d0eac2d3923d221c39eeb8295965f2`; resulting commit: THIS COMMIT.
- Remote branch inventory: only `main`; no separate checkpoint branch.
- The previous MTD/UBI startup and RWFS shutdown task is green on exact SHA `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`: USB storage run [42609876845](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876845) and AI documentation run [42609876722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876722) both SUCCESS.
- The deferred USB startup implementation is in parent `a154f30da5d0eac2d3923d221c39eeb8295965f2`. Its exact-head USB storage run [42612494953](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494953) compiled and passed the new GNU89 child-result test and all preceding shell/storage tests, but failed the final source-contract check at stdin line 129 (the workflow's string-order assertion for inventory and startup call). AI documentation [42612494818](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494818), Samba [42612494997](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494997), vsftpd [42612494935](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494935), NFS [42612494823](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494823), and HTTPD [42612494815](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494815) all SUCCESS on the same exact SHA.
- THIS commit clarifies the source contract by anchoring to the exact inventory assignment, checking flag clearing only after the service-start call, and searching for the Optware launch call without assuming a trailing semicolon. No production behavior is changed.

## Active task

### Deferred USB Optware startup result and retry

P2 storage stability / service lifecycle. Production changes are in parent commit `a154f30da5d0eac2d3923d221c39eeb8295965f2`; THIS commit fixes only the new storage workflow's source-contract anchors.

### Implementation already present in parent

- Preserve three states for `/proc/mounts`: mounted, complete-empty, or incomplete. Incomplete inventories retain pending flags and schedule retry.
- Clear the one-shot `usb_hotplug_ms` after the integrated RC storage services run against a complete mounted inventory, so retries for Optware do not repeatedly start Samba/NFS/FTP.
- Keep `usb_opt_start` pending until the actual `opt-start.sh` child reports exit status zero.
- Fork the worker only after releasing `storage.lock`; use a nonblocking result pipe with close-on-exec writer and SIGALRM wakeup. The open result fd suppresses duplicate launches while a worker is active or the result is unconsumed.
- Preserve NVRAM retry state after child failure/EOF. The child uses existing `opt-start.sh` and its lifecycle lock/supervisor.

### CI contract correction in THIS commit

The exact-head storage run passed all shell regressions, the GNU89 result decoder test, and preceding inventory/lock tests, then stopped at a fragile whole-caller-graph assertion on line 129. The revised assertion:
- anchors the inventory read to the exact `mounted_devices = storage_has_mounted_devices(...)` assignment;
- searches for the `usb_hotplug_ms` clear only after `start_stor_apps();` (there is a valid earlier clear in the separate complete-empty branch);
- searches for `storage_optware_start_async()` without requiring `();`, since it is called in a conditional expression.
No production source is changed by THIS correction.

### Files changed

- `.github/workflows/storage-hotplug-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for tri-state inventory, one-shot integrated service startup, async result channel, lock release before external hooks, and child failure retry. This commit changes only test-contract anchors. |
| L2 host / Actions | Parent exact-head storage run [42612494953](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494953): all shell and GNU89 tests passed; final source-contract test failed at stdin line 129. AI docs [42612494818](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494818) SUCCESS; Samba [42612494997](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494997), vsftpd [42612494935](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494935), NFS [42612494823](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494823), and HTTPD [42612494815](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494815) SUCCESS on the same SHA. THIS contract correction's exact-head storage/docs Actions are pending. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 removable-media hotplug/eject races, real Optware hook failure/hang, target filesystem and physical router | NOT VERIFIED. |

No full local checkout, production RC object, MIPS/uClibc build, firmware/TRX, external hook failure/hang injection, removable-media stress or physical-router result is claimed.

## Exact first action for the next GPT

1. Fetch live `main`, complete branch inventory and Actions. Match USB storage and AI handoff-documentation run `head_sha` values to THIS commit's exact full SHA; inspect the source-contract step and archive validator.
2. If green, audit the separate asynchronous boot-time `start_rwfs_optware()` → `opt-mount.sh ... &` result path in `common_ex.c`; determine whether it needs safe reporting/retry without holding `storage.lock`, separately from the removable USB pipe.
3. Continue NFS export ACL/permission mapping and external mount lifecycle; Samba passdb/account mutation, ACL/permission and credential review; vsftpd authentication/ACL/path-confinement/stop lifecycle; LAN DHCP leasefail/accepted-NAK/release callback state handling; and FULLCONENAT mapping-hash / dying-tuple queue namespace ownership.
4. Keep L3/L4 unverified until production MIPS/uClibc/firmware and live-router validation actually run.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their state materially changes; use Git history for prior versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Update only records whose facts changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for module status/evidence/priorities. Do not mechanically update all documents or create docs-only commits merely to record completed CI.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.

