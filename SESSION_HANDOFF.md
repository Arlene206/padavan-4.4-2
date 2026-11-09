# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base main SHA for THIS MTD boot-time Optware result/retry fix: `19c1627490ec550b74b2deb3e19e8965d4374406`; resulting commit: THIS COMMIT.
- Remote branches at recovery: only `main`; no separate checkpoint branch.
- The previous deferred removable-USB Optware result/retry implementation is green on exact SHA `19c1627490ec550b74b2deb3e19e8965d4374406`: storage [42613569550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569550) SUCCESS (all 11 steps, including the child-result GNU89 test and full caller-graph contract); AI handoff docs [42613569474](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569474) SUCCESS (handoff/roadmap/archive validator).
- Parent commit `a154f30da5d0eac2d3923d221c39eeb8295965f2` introduced the deferred USB result pipe; source-contract anchors were corrected in `19c1627490ec550b74b2deb3e19e8965d4374406`. Other module regressions on `a154f30...` also passed: Samba [42612494997](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494997), vsftpd [42612494935](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494935), NFS [42612494823](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494823), and HTTPD [42612494815](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494815).
- THIS commit addresses the separate boot-time `start_rwfs_optware()` fire-and-forget path in `common_ex.c`. It adds a nonblocking child-result channel and PID1 alarm-loop retry; target/runtime validation remains outstanding.

## Active task

### Internal MTD/UBI Optware boot startup result and retry

P2 storage lifecycle reliability. The removable USB path is complete and exact-head green; this task closes the separate boot-time result gap.

### Finding

`init_router()` calls `start_rwfs_optware()`, which previously executed `doSystem("/usr/bin/opt-mount.sh %s %s &", ...)`. The shell command's background launch status does not reveal whether `opt-mount.sh` later mounts `/opt`, acquires the lifecycle lock, or starts services successfully. The C boot caller discarded the actual script result and had no retry path. RWFS teardown already fails closed when Optware teardown cannot acquire the lifecycle lock, so startup must remain outside PID1's lock-sensitive main path.

### Fix in THIS commit

- Retain asynchronous startup but run the command through a tracked worker with a nonblocking result pipe.
- Use the shared `storage_optware_start_decode_result()` policy so a missing/invalid result or nonzero script exit is treated as failure.
- Keep the write descriptor close-on-exec so shell descendants cannot keep the pipe alive; the worker waits for the actual `opt-mount.sh` status, publishes one result byte, and signals PID1 with SIGALRM.
- Add `retry_rwfs_optware()` to PID1's existing deferred alarm event. It polls without blocking; if the worker is still running it rearms a five-second poll, if the script succeeded it clears pending state, and if the script failed or launch failed it starts another worker and retries.
- Preserve one-worker ownership through the result FD. No external Optware hook runs in PID1's main thread, and no storage lock is acquired by this boot caller.
- Add a no-NAND stub so non-NAND builds retain the same public call surface.

### Files changed

- `trunk/user/rc/src/common_ex.c`
- `trunk/user/rc/src/init.c`
- `trunk/user/rc/src/rc.h`
- `.github/workflows/storage-hotplug-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for asynchronous worker ownership, nonblocking parent read, close-on-exec writer, wait-status decode, retry lifecycle, PID1 event-loop integration, and no external hook in PID1's synchronous path. |
| L2 host / Actions | Prior deferred USB task passed exact-head storage [42613569550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569550) and docs [42613569474](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569474) on `19c1627490ec550b74b2deb3e19e8965d4374406`. THIS MTD startup integration is not yet compiled or run by exact-head Actions; those are pending at commit creation. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 NAND RWFS/UBI mount, real `opt-mount.sh` success/failure, long/hung hook, shutdown race and physical router | NOT VERIFIED. |

No full local checkout, full RC object, MIPS/uClibc build, firmware/TRX, target filesystem test, or router result is claimed.

## Exact first action for the next GPT

1. Fetch live `main`, branches and Actions. Match the storage and AI handoff-doc `head_sha` values to THIS commit's exact full SHA. Inspect the new MTD caller-graph assertions and archive validator.
2. If both pass, review retry behavior for persistent Optware hook failures and shutdown while a worker holds the lifecycle lock; ensure retry remains bounded in CPU/log rate and teardown continues to fail closed.
3. Then continue NFS export ACL/permission mapping and external mount lifecycle; Samba passdb/account mutation, ACL/permission and credential review; vsftpd authentication/ACL/path-confinement/stop lifecycle; LAN DHCP leasefail/accepted-NAK/release callback state handling; and FULLCONENAT mapping-hash / dying-tuple queue namespace ownership.
4. Keep L3/L4 unverified until production MIPS/uClibc/firmware and live-router validation actually run.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their state materially changes; use Git history for prior versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Update only records whose facts changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for module status/evidence/priorities. Do not mechanically update all documents or create docs-only commits merely to record completed CI.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.

