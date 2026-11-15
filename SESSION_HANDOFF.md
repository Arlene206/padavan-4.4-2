# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main SHA for THIS COMMIT: `bb2b87054ef9631fc97c0473dd47d7e8b4fccd87`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). Legacy PR #1 remains CLOSED / UNMERGED.
- Product-source baseline through `99eeb6fa237b529ec9b9615976961c7a60a02575` includes transactional MTD load/restore, backup integrity, HTTPD backup gating, failed-restore upload retention, blank-MTD lock release and fail-closed erase/reset. THIS COMMIT additionally enforces checked stale-hash deletion before save-lock release on both zero-byte MTD paths.
- Exact-head USB storage #43696174488 on `bb2b87054ef9631fc97c0473dd47d7e8b4fccd87` SUCCESS after earlier checker-only failures were corrected: syntax/save retry, restore, host + BusyBox ash load/backup/erase-reset, GNU89 policy, caller graph and source contracts all passed. Exact-head AI docs #43696174518 SUCCESS. Infosvr #43696174495, Process inventory/lifecycle #43696174597, AdGuardHome #43696174519, AliyunDrive #43696174530 and AliDDNS #43696174537 SUCCESS. NVRAM #43696174497 was IN_PROGRESS at last poll.
- XY-C1 #43694958186 SUCCESS on product-source SHA `99eeb6fa237b529ec9b9615976961c7a60a02575`, including Build XY-C1, TRX verification and artifact upload. This is historical evidence for that exact source only; THIS COMMIT changes `mtd_storage.sh` and requires a fresh current-source MIPS/uClibc build/TRX inspection/artifact upload.
- Frozen backup remains unchanged. Current-source L3 and physical L4 are NOT VERIFIED; no local checkout, local regression or current-source firmware-build result is claimed.
## Active task

### P1 Keep the blank-MTD save lock until stale-hash deletion succeeds (THIS COMMIT)

- Both zero-byte paths in `func_load()` (failed decompression with zero-byte output, and a successful decompressor that emits an empty stream) now check stale MD5 removal before attempting to release the one-shot save lock.
- If hash deletion fails, the old settings tree, stale checkpoint and save lock remain intact; a diagnostic is logged and load returns nonzero. This prevents a stale matching hash from making `func_save()` skip writing a blank partition.
- Added injected hash-unlink failures for both branches in `mtd_storage_load_transaction.sh`; assertions require the existing configuration, checkpoint and lock to survive. The existing storage workflow runs the fixture under host shell and BusyBox 1.24.2 ash. The source checker requires checked hash-before-lock ordering and both injected failures.
- Parent exact-head USB storage #43696174488 on `bb2b87054ef9631fc97c0473dd47d7e8b4fccd87` passed before this source change. Exact-head USB, docs and XY-C1 Actions are required for THIS COMMIT.

### Next P1 task — review authenticated restore-upload replacement and retry retention

- Inspect `do_restore_st_cgi()` and all HTTPD upload/authentication callers end-to-end. A previously failed restore archive must remain available until a new authenticated upload has completed in private staging and atomically replaced it.
- Inject upload-open, write, close and rename failures; verify pre-publication failures preserve the previous retry payload and never leave a truncated file at `STORAGE_FIFO_FILENAME`. Existing restore application is transactional and retains a failed archive; do not redo that implementation.
## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; retain legacy Boolean until direct callers explicitly handle UNKNOWN. Continue remaining kill_pidfile_s() and name-wide signal audits; Linux 4.4 retains PID/start_time check-to-signal TOCTOU.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 lacks inode-conditional unlink-by-fd. Do not claim it is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.

## Verification boundary

- L1: Both blank-MTD paths now require successful stale MD5 deletion before attempting save-lock removal. A hash-unlink failure retains the marker and the previous live settings/checkpoint; this prevents an identical-save skip after blank flash.
- L2: Exact-head USB storage #43696174488 on `bb2b87054ef9631fc97c0473dd47d7e8b4fccd87` passed all workflow steps, including MTD syntax/save retry, transactional restore/load, host + BusyBox ash fixtures, GNU89 policy tests, the complete storage caller graph and source contracts. THIS COMMIT adds two hash-unlink fault cases; a fresh exact-head USB run is required. AI docs #43696174518 on the parent succeeded; new exact-head docs check required.
- L3: XY-C1 #43694958186 succeeded on product-source SHA `99eeb6fa237b529ec9b9615976961c7a60a02575` through Build XY-C1, TRX verification and artifact upload. That artifact predates THIS COMMIT's `mtd_storage.sh` change; current-source L3 requires another exact-head build/TRX/upload.
- L4: Physical MTD erase, blank-partition boot, default recreation, save after erase and long-term persistence remain NOT VERIFIED.
- No local checkout, local shell regression or current-source firmware build is claimed.

## HTTPD daemon lifecycle — completed slice

- `start_httpd()` now uses `process_inventory_scan("httpd", ...)` with tri-state `ABSENT/RUNNING/UNKNOWN` semantics. Only an authoritative ABSENT result permits NVRAM configuration normalization, stale-state clearing, launcher execution, or firewall publication.
- A confirmed RUNNING daemon returns `EBUSY` and may publish `httpd_started=1`; UNKNOWN returns `EIO` (or scan errno) without mutating configuration or claiming the daemon is running.
- Detached-child readiness now requires RUNNING. ABSENT retries then returns `ESRCH`; UNKNOWN terminates the probe with an error and never publishes started state. GNU89 regression covers RUNNING, ABSENT, UNKNOWN, invalid states and preserved launcher errors.
- The first exact-head lifecycle Action exposed a stale source-contract assertion expecting the old `status == 0` form. The checker was corrected to assert the new fail-before-publish branch. Current exact run state is listed in the repository checkpoint above.

## Storage eject propagation — completed slice

- `read_disk_data_checked()` now strictly validates `/proc/partitions`: canonical four-column header, blank separator, complete newline-terminated records, exactly four fields, unsigned decimal major/minor/block counts, and overflow-safe conversion. Malformed, signed, truncated, overlong, or overflowed records fail closed; the legacy `read_disk_data()` pointer wrapper is preserved.
- `safe_remove_all_stor_devices()` distinguishes unknown inventory from valid empty inventory, checks the per-device path and generic fallback, and requires a complete final mount snapshot to prove zero storage mounts. Firmware update does not persist storage MTD or execute `mtd_write` when eject is incomplete; normal reboot/halt proceeds but skips storage MTD persistence on failure.
- Parser regression is compiled as GNU89 with `-Wall -Wextra -Werror`. The exact c5 parser Action initially failed earlier in the job on BusyBox `writev` missing its declaration; `b022942` added `<sys/uio.h>`, after which the storage-hotplug exact-head run is represented in the latest status block above. Do not treat prior SHA results as evidence for later source.
- Backup branch remains immutable; physical-router L4 remains unverified.

## Exact first action for the next GPT

1. Inspect exact-head USB storage, AI handoff docs and XY-C1 Actions for THIS COMMIT. Require both new blank-MTD hash-unlink failure cases to pass under host shell and BusyBox 1.24.2 ash, the complete storage checker to pass, and the docs checker to pass with at least 20 root log records and unchanged archive/index hashes/counts. Require current-source XY-C1 build, TRX verification and artifact upload before marking L3 verified.
2. Resume P1 at `do_restore_st_cgi()`: trace authentication and upload staging through open/write/close/rename and all callers. Preserve the prior retry archive until a new authenticated upload has completed and is atomically published; inject each pre-publication failure.
3. Before each write, fetch fresh `main`; publish each multi-file slice as one complete Git tree/commit and non-forced expected-SHA ref update. Keep the frozen backup untouched. L4 remains unverified.

Source/validation notes:
- Parent main for THIS COMMIT: bb2b87054ef9631fc97c0473dd47d7e8b4fccd87; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical current-source build before this MTD fix: product-source SHA 99eeb6fa237b529ec9b9615976961c7a60a02575, XY-C1 #43694958186 completed Build XY-C1, TRX verification and artifact upload. THIS COMMIT requires a fresh build.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
