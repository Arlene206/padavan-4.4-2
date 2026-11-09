# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base main SHA for THIS deferred USB Optware startup result/retry fix: `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`; resulting commit: THIS COMMIT.
- Remote branch inventory at recovery: only `main`; no separate checkpoint branch.
- The preceding internal MTD/UBI startup and RWFS shutdown task is reconciled: USB storage regression [42609876845](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876845) and AI handoff documentation [42609876722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876722) both completed SUCCESS on the exact full SHA `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`. All 9 storage workflow steps passed, including Optware MTD syntax, lifecycle-lock/teardown/startup host regressions, GNU89 storage inventory tests, and the whole storage caller-graph/source contract. The documentation validator also passed on that exact SHA.
- The production MTD/UBI lifecycle correction remains in `b13ca6b46e0bef9474ee6744cf4f7f1734aadc97`; the final RWFS source-contract correction is in `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`. The earlier failed runs on `d225e9e...` are historical and superseded by the two exact-head SUCCESS runs above.
- THIS commit changes the deferred removable-USB Optware result/retry path in `services_stor.c`, adds a small GNU89 result-channel policy regression, and extends the storage workflow. It does not change the MTD boot caller.

## Active task

### Deferred USB Optware startup result and retry

P2 storage stability / service lifecycle. The previous MTD/UBI/RWFS task is verified green on exact HEAD; this bounded follow-up addresses the separate removable-storage deferred-start path.

### Finding

`storage_has_mounted_devices()` previously converted an incomplete `/proc/mounts` read to false, indistinguishable from a complete empty inventory. `try_start_stor_apps()` launched `opt-start.sh` in the background and returned success without observing the script's exit status. `on_deferred_hotplug_dev()` then cleared both `usb_hotplug_ms` and `usb_opt_start`, losing retry state after hook/startup failure. A retry caused by lock contention also re-ran all integrated storage services.

### Fix in THIS commit

- Preserve a three-state mount result: mounted, complete-empty, or incomplete. An incomplete inventory keeps both pending NVRAM flags and is retried; a complete empty inventory clears stale work only when no Optware worker is still running.
- Treat `usb_hotplug_ms` as the one-shot request for integrated RC storage-service startup. Clear it after the complete inventory/start call, so Optware retries do not repeat Samba/NFS/FTP and other integrated startup.
- Keep `usb_opt_start` pending until the actual `opt-start.sh` child reports exit status 0. A child failure or premature EOF leaves the flag set and schedules retry.
- Fork the Optware worker only after releasing `storage.lock`. A nonblocking pipe carries its one-byte success/failure result back to PID 1; the child sends SIGALRM after publishing the result. PID 1's existing SIGCHLD reaper can continue reaping children without losing this separate pipe result.
- The single open result channel prevents duplicate worker launches while startup is running or its result has not yet been consumed. The child closes the read end, and its write end is close-on-exec so grandchildren from `system()` cannot keep the result pipe artificially open.
- The worker continues to use `opt-start.sh` and the existing shared Optware lifecycle lock/supervisor. No external Optware hook runs while the C caller owns `storage.lock`.

### Files changed

- `trunk/user/rc/src/services_stor.c`
- `trunk/user/rc/src/storage_optware_start_policy.h`
- `trunk/user/rc/src/tests/storage_optware_start_policy.c`
- `.github/workflows/storage-hotplug-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for tri-state `/proc/mounts` handling, one-shot integrated-service flag, async child result channel, no external hook under `storage.lock`, duplicate-worker suppression, and retained retry state. |
| L2 host / Actions | The new GNU89 policy test compiled and passed locally in a scratch directory. The base storage and AI-doc workflows both passed on exact SHA `6ae43a6a71e05bf5aeba47516b1c4b193c75075e` (storage [42609876845](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876845), docs [42609876722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876722)). THIS commit's storage and documentation Actions are pending on its resulting full SHA; no local full-source build or full checkout is claimed. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 removable-media hotplug/eject races, real Optware hook failure/hang, target filesystem and physical router | NOT VERIFIED. |

No production RC object, MIPS/uClibc build, firmware/TRX or target-device result is claimed. The policy unit test verifies the pipe-result decoder; Actions must validate the integrated source contract and full storage regression.

## Exact first action for the next GPT

1. Fetch live `main`, the complete branch inventory and Actions. Match the USB storage and AI handoff-documentation run `head_sha` values to THIS commit's exact full SHA; inspect every step, especially the deferred Optware result/retry source contract and archive/documentation validator.
2. If both exact-head workflows pass, audit the separate asynchronous boot-time `start_rwfs_optware()` → `opt-mount.sh ... &` result path in `common_ex.c`. Determine whether the MTD startup should report to PID 1/retry safely without holding `storage.lock`; do not conflate it with the removable USB result pipe.
3. Then resume NFS export ACL/permission mapping and external mount lifecycle; Samba passdb/account mutation, ACL/permission and credential review; vsftpd authentication/ACL/path-confinement/stop lifecycle; LAN DHCP leasefail/accepted-NAK/release callback state handling; and FULLCONENAT mapping-hash / dying-tuple queue namespace ownership.
4. Keep L3/L4 explicitly unverified until production MIPS/uClibc/firmware and live-router validation actually run.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their state materially changes; use Git history for prior versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Update only records whose facts changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for module status/evidence/priorities. Do not mechanically update all documents or create docs-only commits merely to record completed CI.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.

