# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base main SHA for THIS MTD retry/shutdown lifecycle fix: `652581beb0b71787096a1a74a90a3f6c32ce7cd6`; resulting commit: THIS COMMIT.
- Remote branch inventory at recovery: only `main`; no separate checkpoint branch.
- The preceding caller-contract checkpoint is green on its exact full SHA `652581beb0b71787096a1a74a90a3f6c32ce7cd6`: USB storage [42615347517](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347517) SUCCESS; AI handoff docs [42615347722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347722) SUCCESS.
- Parent production MTD result/retry implementation was `93406e8618deae765a7c9cf6033ce978e58e230a`; its storage run [42615049407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049407) failed only on the stale old-caller assertion, while the new MTD worker/retry assertions and GNU89 tests before it passed. Samba [42615049566](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049566), WAN [42615049422](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049422), HTTPD [42615049699](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049699), and docs [42615049390](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615049390) were SUCCESS on that exact parent SHA.
- THIS commit adds bounded retry/backoff and shutdown cancellation for the internal MTD/UBI Optware worker. Its storage and documentation Actions are pending at commit creation.

## Active task

### Verify the MTD/UBI Optware retry and shutdown-lifecycle fix

P2 storage/PID1 reliability. The previous worker reported actual `opt-mount.sh` status but retried every five seconds indefinitely, and its detached process tree could still run startup hooks while shutdown was stopping ordinary services.

### Implementation in THIS commit

- Keep PID1's five-second nonblocking result polling, but back off actual failed relaunch attempts exponentially: 5, 10, 20, 40, 80, 160, then 300 seconds maximum. Use monotonic `uptime()`; reset the delay after successful startup.
- Run the worker in a private process group. In the forked worker, reset inherited signal dispositions to defaults and clear PID1's blocked signal mask before `system()`, preventing PID1's SIGCHLD reaper or shutdown-only signal handlers from interfering with the shell child.
- Track the worker PID and stop its process group with TERM then KILL during `shutdown_router()`, before stopping ordinary services. The worker publishes no further retry once shutdown begins.
- Preserve asynchronous boot behavior, keep external hooks outside `storage.lock`, and retain the existing RWFS/UBI fail-closed teardown gate if the lifecycle lock or unmount cannot be completed.

### Files changed

- `trunk/user/rc/src/common_ex.c`, `trunk/user/rc/src/rc.c`, `trunk/user/rc/src/rc.h`
- `trunk/user/rc/src/storage_optware_start_policy.h`, `trunk/user/rc/src/tests/storage_optware_start_policy.c`
- `.github/workflows/storage-hotplug-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for retry state/backoff transitions, child signal reset, private process-group ownership, PID1 shutdown ordering, and unchanged fail-closed RWFS teardown. |
| L2 host / Actions | PENDING for THIS COMMIT's storage regression (GNU89 policy compile/run plus full caller-graph source contract) and AI handoff-doc/archive validator. The exact base SHA `652581beb0b71787096a1a74a90a3f6c32ce7cd6` passed storage [42615347517](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347517) and docs [42615347722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347722). |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 Real NAND/UBI hook cancellation, shutdown race, router boot/flash/hardware behavior | NOT VERIFIED. |

No complete local checkout, local production object build, MIPS/uClibc build, firmware/TRX, real NAND/UBI failure injection or physical-router test is claimed. Process-group cancellation cannot catch a third-party daemon that deliberately escapes its worker process group; RWFS teardown remains fail-closed if the lifecycle lock is still owned.

## Exact first action for the next GPT

1. Fetch live `main`, all remote branches and latest Actions. Match the USB storage and AI handoff-doc runs to THIS commit's exact full SHA; inspect every job step and the docs/archive validator.
2. If either current-head workflow fails, diagnose and fix that exact failure before selecting other work. Do not weaken the MTD caller contract or GNU89 regression.
3. Once exact-head storage/docs pass, rotate to the next P2 userland stability task: NFS export ACL/permission mapping and external mount lifecycle. Start with `trunk/user/nfs-utils/nfsd.sh`, NFS export generation/callers in `trunk/user/rc/src/services_stor.c`, and its RC start/stop/restart and safe-removal callers in `trunk/user/rc/src/rc.c`; trace permission ownership, export publication, mount state and rollback.
4. Keep Samba passdb/account/ACL/credential work, vsftpd authentication/ACL/path confinement, DHCP leasefail/NAK/release callback state, and FULLCONENAT queue namespace ownership open. L3/L4 remain unverified until target build and device tests actually run.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their state materially changes; use Git history for prior versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Update only records whose facts changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for module status/evidence/priorities. Do not mechanically update all documents or create docs-only commits merely to record completed CI.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
