# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Base main SHA for THIS COMMIT: `b799ac5bb7588e015913a5bbc70e68efb5adec77`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). Legacy PR #1 remains CLOSED / UNMERGED.
- Product source already on main includes checked buffered storage writes (`180e90075a11ad1135ee6801915211548337b52e`), storage shutdown persistence gates, MTD save retry/GNU md5sum compatibility, RC timestamp persistence and transactional MTD restore.
- Exact-head parent Actions on `b799ac5bb7588e015913a5bbc70e68efb5adec77`: USB storage #43687393291 SUCCESS; HTTPD security #43687393326 SUCCESS; Process inventory/lifecycle #43687393295 SUCCESS; NVRAM audit #43687393286 SUCCESS; AI handoff docs #43687393272 SUCCESS; infosvr #43687393356 SUCCESS; AdGuardHome #43687393288 SUCCESS; AliDDNS #43687393306 SUCCESS; AliyunDrive #43687393297 SUCCESS. XY-C1 #43687393279 was IN_PROGRESS at last poll.
- Current P1 defect: `func_load()` ignores `bzcat`/tar failures, accepts partial non-empty decompression, publishes MD5 before extraction, and extracts directly into live `/etc/storage`. This commit validates/stages before same-filesystem directory replacement with rollback.
- Current source L3 and physical L4 remain NOT VERIFIED. No full local checkout/toolchain or target hardware is available; no local shell/build result is claimed.
## Active task

### P1 slice — Make boot-time MTD load transactional (THIS COMMIT)

- Early boot creates `/etc/storage` and invokes MTD load before defaults and the custom start script. Old load ignored decompressor/tar statuses, accepted partial output, advanced MD5 before extraction and unpacked over live configuration.
- New load checks decompressor status, numeric/nonzero size, staged tar extraction, directory permissions and hash staging; then parks the old directory and publishes the complete sibling stage. Failed publication restores the old tree. Cleanup/checkpoint failures are surfaced without replacing good settings with partial data.
- Added host and BusyBox ash regression for decompression failure after output, partial extraction, failed publish/rollback and complete replacement. Integrated storage workflow push/PR filters and production source checker.
- Changed paths: `mtd_storage.sh`, new `mtd_storage_load_transaction.sh`, storage-hotplug workflow, `check_storage_safe_remove.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent `b799ac5bb7588e015913a5bbc70e68efb5adec77`; exact-head Actions PENDING. No local run is claimed.
## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; keep the legacy Boolean API until all callers explicitly handle UNKNOWN. Continue auditing remaining kill_pidfile_s() and direct name-wide signal paths; Linux 4.4 retains a PID/start_time check-to-signal TOCTOU window.
- Next bounded storage task: audit mtd_storage.sh load/restore/backup/reset for unchecked decompression/extraction, directory replacement, cp/mkdir/metadata publication and rollback failures. Trace caller and lock semantics before changing any destructive path; reproduce one concrete failure.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit with source-backed findings.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 has no inode-conditional unlink-by-fd. Do not claim the race is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.
- Rotate work across userland modules after the storage restore audit; avoid speculative lifecycle changes.

## Verification boundary

- L1: The MD5 bug is directly traced in func_save(): the old implementation published its skip hash before mtd_write. The new shell fixture exercises failed flash followed by an identical retry and later idempotent skip; source contracts cover script hash order and every RC persistence caller. Not executed locally in this context.
- L2: On exact parent `a7fe3e4`, USB storage #43686251138 failed only at the standalone checker separator (`\\nfunc_backup()` was incorrectly searched as literal text); Process inventory/lifecycle #43686251333 failed at shutdown's stale `write_storage_to_mtd();` expectation. This commit fixes both and adds the storage restore failure regression. The actual MTD retry shell test passed on the parent; HTTPD security #43686251157 passed. Fresh exact-head runs are required.
- L3: Current source is NOT VERIFIED. A fresh exact-head XY-C1 MIPS/uClibc build, TRX verification and artifact upload are required after rc.c, header and persistence helper changes.
- L4: Physical XY-C1 flash, boot/runtime and storage persistence behavior are NOT VERIFIED.
- No local build/test or physical hardware result is claimed.

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

1. Inspect exact-head USB storage and XY-C1 Actions. Require host/BusyBox ash load fixtures, storage caller graph and source checker to pass.
2. Continue P1 with `func_backup()` and HTTPD `do_storage_file()`: prevent a failed or partial backup from being streamed as valid; test compressor failure and file existence.
3. Then update `do_restore_st_cgi()` to retain `STORAGE_FIFO_FILENAME` after failed restore for retry; audit `func_reset()`/`func_erase()` directory, MTD erase, lock and hash postconditions.
4. Use fresh parent, one complete tree/commit, force=false/expected_sha; keep frozen backup untouched. Current L3/L4 NOT VERIFIED.

Source/validation notes:
- Parent main for THIS COMMIT: 31ae7b6f253734cc0abf5f67b5c34e861229155b; immutable backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373, artifact #14147062315. Current product source needs a fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.

