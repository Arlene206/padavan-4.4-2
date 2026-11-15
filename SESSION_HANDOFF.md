# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main SHA for THIS COMMIT: `1eb57714cd9cbaaa71a4074615eaf472f053f359`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). Legacy PR #1 remains CLOSED / UNMERGED.
- Previous product-source baseline `2feac289de28bca92469c304a0af13d90c1fb5e9` contains transactional MTD load/restore, backup integrity, HTTPD backup gating, failed-restore upload retention and blank-MTD lock release. THIS COMMIT adds destructive reset/erase postcondition handling in `mtd_storage.sh`; current-source L3 must be revalidated.
- Exact-head USB storage #43691843963 on `1eb57714cd9cbaaa71a4074615eaf472f053f359` passed all steps 1–20, including host + BusyBox ash MTD load/backup fixtures, restore transaction, GNU89 policy tests and the complete storage caller graph. It failed only at the final checker: expected two checked MTD load save-lock removals, while source correctly contains three (failed zero-byte decompression, successful empty stream, successful load). THIS COMMIT fixes the count and adds a reset/erase failure-injection regression.
- Exact-head AI handoff docs #43691844015 on `1eb57714cd9cbaaa71a4074615eaf472f053f359` SUCCESS. Infosvr #43691843957, AdGuardHome #43691843925, AliDDNS #43691843931, AliyunDrive #43691843980, process inventory/lifecycle #43691843972 and NVRAM #43691843918 also SUCCESS.
- USB storage, AI docs and XY-C1 current-source validation for THIS COMMIT require fresh exact-head push runs. The MTD script is a product-source input to XY-C1; do not treat prior firmware artifacts as current evidence.
- Frozen backup remains unchanged. Current L3 and physical L4 are NOT VERIFIED; no full local checkout/toolchain or hardware result is available.

## Active task

### P1 MTD erase/reset postconditions — source + host/ash regression integrated (THIS COMMIT)

- `func_erase()` now creates the persistent save lock before invoking irreversible `mtd_write erase`. Any uncertain flash result or later hash/directory operation failure returns nonzero and retains the lock. Hash deletion, directory removal and recreation are all checked; a successful erase intentionally keeps the lock until next boot validates blank MTD.
- `func_reset()` now locks persistence before removing the stale hash and old storage tree, checks each removal/recreation operation, and returns failure with the lock retained. The dispatcher calls default fill only after preparation succeeds, releases the marker only after fill succeeds, and reports app-start failure.
- Added `mtd_storage_erase_reset.sh` fault injection for MTD failure, lock creation, hash deletion, storage removal/recreation, default-fill and lock-release failures; wired host-shell and BusyBox 1.24.2 ash runs plus push/PR path filters. The load checker now requires all three real checked unlock sites rather than an incorrect count of two.
- No rollback is claimed after a successful physical MTD erase. If post-erase cleanup fails, the lock remains to prevent an unsafe persistence attempt.

### Next P1 task — review authenticated restore-upload replacement and retry retention

- Inspect `do_restore_st_cgi()` and its HTTPD callers end-to-end. Ensure a failed previous upload remains available until a new authenticated upload has safely started/replaced it; test upload-open/write/close/rename failures and preserve the old retry payload on pre-publication failures.
- Existing restore application remains transactional and retains a failed archive; do not redo that implementation. The focus is the boundary between incoming authenticated upload replacement and the retained retry file.

## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; retain legacy Boolean until direct callers explicitly handle UNKNOWN. Continue remaining kill_pidfile_s() and name-wide signal audits; Linux 4.4 retains PID/start_time check-to-signal TOCTOU.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 lacks inode-conditional unlink-by-fd. Do not claim it is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.

## Verification boundary

- L1: Source review confirms erase locks before flash mutation, checks MTD/hash/directory operations and retains the lock on uncertain/partial outcomes. Reset retains the lock through hash deletion, directory recreation and default fill, releasing it only after successful fill. No physical rollback is claimed after a successful erase.
- L2: Parent USB storage #43691843963 on `1eb57714cd9cbaaa71a4074615eaf472f053f359` passed syntax, save retry, restore, host + BusyBox ash load/backup, GNU89 policy tests and the complete caller graph; the final checker failed solely because its expected lock-removal count was two rather than the three actual checked branches. THIS COMMIT fixes the contract and adds host + BusyBox ash reset/erase fault injection. New exact-head USB storage run required. Parent AI docs #43691844015 succeeded; new exact-head docs run required for this commit.
- L3: Previous full XY-C1 MIPS/uClibc build/TRX artifact at `cc61cb5f8ae594b0cf1aee3ddebdd414e0bf5286` is historical only. This commit changes product source `mtd_storage.sh`; a fresh exact-head XY-C1 build, TRX inspection and artifact upload are required.
- L4: Physical MTD erase, blank-MTD next boot, default recreation, reset behavior and later persistence remain NOT VERIFIED.
- No local checkout, local shell regression, full toolchain build or hardware result is claimed.

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

1. Inspect the exact-head USB storage, AI handoff docs and XY-C1 Actions for THIS COMMIT. Require the storage source contract and both host/BusyBox ash erase-reset fixtures to PASS; require the docs checker to pass with at least 20 root log records and unchanged archive/index hashes/counts; require a complete current-source XY-C1 build, TRX verification and artifact upload before marking L3 verified.
2. When those checks pass, continue P1 at `do_restore_st_cgi()`: trace authentication, incoming upload staging, replacement/rename and retained-failure retry behavior. Keep the prior retry payload until the new authenticated upload safely publishes; inject open/write/close/rename failures.
3. Before every write, fetch fresh `main`; publish each multi-file slice as one complete Git tree/commit and non-forced expected-SHA ref update. Keep the frozen backup untouched. L4 remains unverified.


Source/validation notes:
- Parent main for THIS COMMIT: 361bf44c724e64f3826b876d43a876bb65fc6039; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373, artifact #14147062315. Current source requires fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
