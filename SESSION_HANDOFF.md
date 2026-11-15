# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main SHA for THIS COMMIT: `99eeb6fa237b529ec9b9615976961c7a60a02575`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). Legacy PR #1 remains CLOSED / UNMERGED.
- Product-source commit `99eeb6fa237b529ec9b9615976961c7a60a02575` adds checked, lock-protected MTD erase/reset and host + BusyBox ash fault injection. Do not repeat the implementation.
- Exact-head USB storage #43695808627 on `c2ee97b727e1fa16c86a93969129bf3c1c54f49f` passed all MTD source, shell and host + BusyBox ash fixture steps, including reset/erase, and the full caller graph. The final checker failed at `body(mtd_storage, "func_erase()")`: its C brace/string parser misread the apostrophe in a shell comment and reported an unterminated function. THIS COMMIT changes the shell contracts to bounded source slicing; no production source behavior changes.
- Exact-head AI handoff docs #43694958340 SUCCESS. Process inventory/lifecycle #43694958225, infosvr #43694958242, AdGuardHome #43694958168, AliDDNS #43694958275, AliyunDrive #43694958193, process lifecycle inventory #43694958234, NVRAM #43694958278 and HTTPD security #43694958209 also SUCCESS, all on the same SHA.
- XY-C1 Firmware Validation #43694958186 was still IN_PROGRESS at last poll. Its result is required for current-source L3. Physical L4 remains NOT VERIFIED.
- Frozen backup remains unchanged; no local full checkout/toolchain or target hardware result is available.

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

- L1: MTD erase establishes the save lock before flash mutation, checks erase/hash/directory outcomes and retains the marker on uncertain/partial results. Reset retains the save barrier until default fill completes. No rollback is claimed after successful physical flash erase.
- L2: USB storage #43695808627 on `c2ee97b727e1fa16c86a93969129bf3c1c54f49f` passed all shell, host + BusyBox ash save/restore/load/backup/erase-reset, GNU89 policy and caller-graph steps. It failed only in the final checker because the generic C function parser cannot safely parse shell comments (apostrophe in `func_erase()`) and reported an unterminated function. THIS COMMIT uses shell function boundary slicing for erase/reset source contracts; fresh exact-head storage check required.
- L3: XY-C1 #43694958186 on product-source SHA `99eeb6fa237b529ec9b9615976961c7a60a02575` must be checked for exact SHA, full MIPS/uClibc build, TRX verification and artifact upload. Do not treat an in-progress run as L3 evidence.
- L4: Physical erase/reset, blank-MTD next boot and subsequent persistence remain NOT VERIFIED.
- No current L3 or physical L4 result is claimed.


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

1. Inspect exact-head USB storage and AI docs push Actions for THIS COMMIT; require the shell source checker and reset/erase host + BusyBox ash fixtures to pass. Check XY-C1 #43694958186 against its exact product-source SHA and require full build/TRX/artifact evidence before marking L3 verified.
2. Continue P1 at `do_restore_st_cgi()`: trace authenticated restore-upload staging and the incoming writer using `STORAGE_FIFO_FILENAME`. Preserve an older retained retry archive until a new upload is fully parsed, flushed and atomically published. Gate restore callback on per-request upload success so a rejected upload cannot consume stale bytes; inject open/write/close/rename failures and check trusted-session gating.
3. Every multi-file slice must be one Git tree/commit; non-force update main with expected SHA. Frozen backup untouched. L4 remains unverified.

Source/validation notes:
- Parent main for THIS COMMIT: 361bf44c724e64f3826b876d43a876bb65fc6039; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373, artifact #14147062315. Current source requires fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
