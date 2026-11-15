# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main for THIS COMMIT: `546ca8e66a00aee08c53922a55368e61ceb507e2`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). Legacy PR #1 remains CLOSED / UNMERGED.
- Parent `546ca8e66a00aee08c53922a55368e61ceb507e2` fixed HTTPD backup/restore source boundaries and retained uploaded restore data on failed restore. HTTPD security #43690054071 SUCCESS, confirming these contracts. USB storage #43690054009 FAILED only because the storage workflow pull_request filter omitted `mtd_storage_load_transaction.sh`; the source checker also requires the new backup test path. This commit adds both PR triggers.
- Parent XY-C1 #43690054061 remained IN_PROGRESS at last poll. NVRAM #43690054024 SUCCESS; FRP #43690054108, AdGuardHome #43690054047, AI docs #43690054062, Process inventory #43690054006, Process lifecycle #43690054021, infosvr #43690054113, AliDDNS #43690054011 and AliyunDrive #43690054109 SUCCESS.
- A source trace found a real boot lifecycle regression: successful `func_erase()` creates `$slk` to suppress saves until the next boot. If the next boot sees a blank MTD, zero-byte decompression must clear the marker; otherwise saves remain suppressed indefinitely. This commit adds that behavior and a host/BusyBox ash regression.
- Current L3 and physical L4 remain NOT VERIFIED. No full local checkout/toolchain or target hardware is available.

## Active task

### Current P1 correction — release the one-shot save lock after erased MTD and complete PR test triggers (THIS COMMIT)

- `func_erase()` sets `$slk` after a successful flash erase; runtime `save)` exits while it exists. Early boot executes `mtd_storage.sh load` before `fill`. The empty/erased MTD is identified by a zero-byte decompression result, which is the point at which the one-shot lock and stale MD5 can safely be cleared.
- Nonempty decompressor failures remain corruption and retain the previous hash and lock. The load regression now covers empty-output with decompressor failure, empty-output with nominal success, and nonempty failed output, as well as the existing tar extraction failure, promotion rollback and successful load.
- Storage-hotplug now watches both load and backup fault-injection scripts in pull_request filters; the source checker enforces the pair of push/PR triggers.

### Next P1 task — audit destructive reset/erase postconditions

- Trace `func_erase()` and `func_reset()` end-to-end. Check MTD erase status, stale hash deletion, storage directory removal/recreation, lock creation/removal and every partial failure. Add fault injection for erase failure and directory creation/removal failure. Do not claim physical rollback after a successful flash erase.
- Restore upload retry retention is now implemented in `do_restore_st_cgi()`: the upload is deleted only on successful restore and retained on failure. The HTTPD security exact-head run for parent `546ca8e66a00aee08c53922a55368e61ceb507e2` passed.

## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; retain legacy Boolean API until direct callers explicitly handle UNKNOWN. Continue remaining kill_pidfile_s() and name-wide signal audits; Linux 4.4 retains PID/start_time check-to-signal TOCTOU.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 lacks inode-conditional unlink-by-fd. Do not claim it is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility need real client/storage evidence.

## Verification boundary

- L1: Blank MTD output after erase must clear stale MD5+one-shot lock; a nonempty failed stream must preserve both. The load path still rejects incomplete archive/decompression and stages before publication.
- L2: Parent `546ca8e66a00aee08c53922a55368e61ceb507e2` USB storage #43690054009 failed at the final trigger-path assertion; HTTPD security #43690054071 succeeded. This commit fixes PR path triggers and adds blank-MTD/empty-success/partial-corruption regression assertions. Exact-head storage and XY-C1 validation required.
- L3: Current source is NOT VERIFIED. The MTD change requires a fresh XY-C1 MIPS/uClibc build, TRX verification and artifact upload.
- L4: Physical erase, blank-MTD next boot, default recreation and later save remain NOT VERIFIED.
- No local test/build or physical hardware result is claimed.

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

1. Inspect exact-head HTTPD security and USB storage Actions. Confirm backup source-boundary checks, failed-restore upload-retention assertions, MTD backup host/BusyBox tests and the full caller graph.
2. Continue P1 with `func_reset()` and `func_erase()`: they ignore directory/hash/lock operation statuses; erase success followed by failed settings-directory recreation can leave startup without its expected directory. Add error propagation and failure-injection coverage; do not claim MTD rollback after physical erase.
3. Review restore retry UX and expiry: a failed upload should remain available for retry, but the next authenticated upload must replace stale retry data only when the new upload safely begins.
4. Use a fresh main SHA, one complete Git tree/commit, force=false and expected_sha. Frozen backup untouched. Current-source L3 and physical L4 remain NOT VERIFIED.

Source/validation notes:
- Parent main for THIS COMMIT: 546ca8e66a00aee08c53922a55368e61ceb507e2; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373, artifact #14147062315. Current source requires a fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
