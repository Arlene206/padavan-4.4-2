# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base main SHA for THIS MTD caller-contract correction: `93406e8618deae765a7c9cf6033ce978e58e230a`; resulting commit: THIS COMMIT.
- Remote branch inventory: only `main`; no separate checkpoint branch.
- Previous deferred removable-USB Optware startup result/retry fix is verified green on exact SHA `19c1627490ec550b74b2deb3e19e8965d4374406`: storage [42613569550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569550) SUCCESS (all 11 steps); AI handoff docs [42613569474](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569474) SUCCESS.
- MTD boot result/retry implementation is in parent `93406e8618deae765a7c9cf6033ce978e58e230a`. Exact-head Samba [42615049566](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049566), WAN route [42615049422](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049422), HTTPD security [42615049699](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049699), and AI handoff docs [42615049390](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049390) all SUCCESS. USB storage [42615049407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049407) passed the shell syntax, storage lock/teardown/startup regressions, and both GNU89 tests, then failed only because the final caller-graph workflow still asserted the removed `doSystem("... &")` implementation. The newly added MTD worker/retry source assertions before that stale assertion had passed.
- THIS commit removes that obsolete fire-and-forget source contract. Production C behavior is unchanged.

## Active task

### Internal MTD/UBI Optware boot startup result and retry

P2 storage lifecycle reliability. Production implementation is in parent commit `93406e8618deae765a7c9cf6033ce978e58e230a`; THIS commit removes the now-obsolete workflow assertion for the previous fire-and-forget caller.

### Production change in parent

- `start_rwfs_optware()` now starts a tracked worker rather than `doSystem("... &")`.
- The worker executes `opt-mount.sh`, records its actual exit status in a nonblocking pipe, closes the writer on exec descendants, and signals PID1 after publishing the result.
- `retry_rwfs_optware()` polls the result from PID1's deferred SIGALRM event. Success clears pending state; failure/launch error retries; a running worker retains a single result channel and is not duplicated.
- The boot path stays asynchronous; external Optware hooks do not run synchronously in PID1's main event loop. No storage operation lock is acquired in the caller.

### CI contract correction in THIS commit

The exact-head storage run passed the NFS/Optware shell checks, lifecycle lock/teardown/startup tests, storage inventory test, deferred child-result GNU89 test, and the new MTD worker/result/retry/PID1 source assertions. It then hit the pre-existing assertion that still required `doSystem("/usr/bin/opt-mount.sh ... &")`. This is obsolete after the parent commit replaced that call with a tracked worker. THIS commit removes the stale assertion and retains explicit checks for the new worker and retry entrypoints. No production behavior changes.

### Files changed

- `.github/workflows/storage-hotplug-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for MTD async worker, real command result channel, PID1 polling/retry, and removal of synchronous hook work from PID1. |
| L2 host / Actions | On exact parent SHA `93406e8618deae765a7c9cf6033ce978e58e230a`, storage [42615049407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049407) passed all shell and GNU89 tests and the new MTD assertions, then failed the final obsolete fire-and-forget contract. Samba [42615049566](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049566), WAN route [42615049422](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049422), HTTPD [42615049699](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049699), and AI docs [42615049390](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049390) all SUCCESS on the same full SHA. THIS workflow correction's exact-head storage/docs reruns are pending. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 NAND RWFS/UBI mount, real `opt-mount.sh` success/failure, long/hung hook, shutdown race and physical router | NOT VERIFIED. |

No full local checkout, full RC object, MIPS/uClibc build, firmware/TRX, real NAND/UBI or physical-router test is claimed.

## Exact first action for the next GPT

1. Fetch live `main`, branch inventory and Actions. Match storage and AI handoff-doc `head_sha` values to THIS commit's exact full SHA and inspect every storage step plus the docs/archive validator.
2. If green, review the five-second persistent-failure retry cadence for MTD hooks and shutdown while the lifecycle lock is held; determine whether log/retry backoff is warranted without creating duplicate workers or weakening RWFS fail-closed teardown.
3. Continue NFS export ACL/permission mapping and external mount lifecycle; Samba passdb/account mutation, ACL/permission and credential review; vsftpd authentication/ACL/path-confinement/stop lifecycle; LAN DHCP leasefail/accepted-NAK/release callback state handling; and FULLCONENAT mapping-hash / dying-tuple queue namespace ownership.
4. Keep L3/L4 unverified until production MIPS/uClibc/firmware and live-router validation actually run.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their state materially changes; use Git history for prior versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Update only records whose facts changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for module status/evidence/priorities. Do not mechanically update all documents or create docs-only commits merely to record completed CI.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.

