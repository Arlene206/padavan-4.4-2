# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main for THIS COMMIT: `5338c5adfe54a7eedcfc460b284f4783c03a09cf`; immutable backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Parent source commit `5338c5adfe54a7eedcfc460b284f4783c03a09cf` implemented MTD digest staging/retryability, status-returning `write_storage_to_mtd()`, checked atomic `storage_save_time()`, and firmware/shutdown/boot persistence gating. Do not reimplement those changes.
- Exact-head Actions on parent `5338c5adfe54a7eedcfc460b284f4783c03a09cf`: USB storage #43685359346 FAILED at step 11 because GNU coreutils rejects the BusyBox-specific `md5sum -s`; process lifecycle #43685359507 FAILED at its integration checker because it expected `write_storage_to_mtd();` instead of the now status-returning expression; HTTPD security #43685359427 FAILED at the corresponding duplicated inline source contract. XY-C1 #43685359467 was IN_PROGRESS at last poll. All three failures are being corrected in THIS COMMIT; the other completed parent workflows were successful.
- The isolated shell reproduction showed GNU `md5sum -c` succeeds after a durable retry and correctly skips the following unchanged snapshot. The full repository shell regression is wired to exact-head USB storage Actions, not claimed locally.
- Current-source L3 is NOT VERIFIED until exact-head XY-C1 MIPS/uClibc build, TRX verification and artifact upload pass. Physical L4 remains NOT VERIFIED. No full local checkout/toolchain is available.
## Active task

### Current P1 slice — repair exact-head validation fallout from the MTD persistence commit (THIS COMMIT)

- In `mtd_storage.sh`, changed the unchanged-snapshot check from BusyBox-only `md5sum -s` to portable `md5sum -c` with stdout/stderr redirected. GNU coreutils in GitHub Actions does not accept `-s`; the old command therefore returned failure on every run and never skipped an unchanged archive.
- Updated the Process lifecycle integration checker and the HTTPD security workflow's duplicated source-order contract to recognize `write_storage_to_mtd()` as a status-returning expression, rather than the previous void-call literal ending in a semicolon. Existing ordering constraints remain.
- A local isolated shell reproduction using GNU md5sum confirmed that after a successful retry the next identical snapshot validates and skips MTD write. Full repository regressions and target build await exact-head Actions.
- Fresh exact-head USB storage, Process lifecycle, HTTPD security and XY-C1 workflows are the required validation.

### Next P1 task — audit MTD load/restore/backup/reset failure handling

- Trace `mtd_storage.sh` load/restore/backup/reset callsites, lock semantics, scratch paths, decompression/extraction, target replacement, metadata publication and rollback. Identify one reproducible failure before modifying destructive paths.
- Add a focused shell regression for the actual failure, integrate the correct workflow path filters and verify exact-head Actions. Do not repeat MTD save retry, RC writer/time status propagation, firmware watchdog recovery, checker contract repairs, RWFS/UBI teardown or safe-eject.

## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; keep the legacy Boolean API until all callers explicitly handle UNKNOWN. Continue auditing remaining kill_pidfile_s() and direct name-wide signal paths; Linux 4.4 retains a PID/start_time check-to-signal TOCTOU window.
- Next bounded storage task: audit mtd_storage.sh load/restore/backup/reset for unchecked decompression/extraction, directory replacement, cp/mkdir/metadata publication and rollback failures. Trace caller and lock semantics before changing any destructive path; reproduce one concrete failure.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit with source-backed findings.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 has no inode-conditional unlink-by-fd. Do not claim the race is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.
- Rotate work across userland modules after the storage restore audit; avoid speculative lifecycle changes.

## Verification boundary

- L1: The MD5 bug is directly traced in func_save(): the old implementation published its skip hash before mtd_write. The new shell fixture exercises failed flash followed by an identical retry and later idempotent skip; source contracts cover script hash order and every RC persistence caller. Not executed locally in this context.
- L2: Parent #43685359346 storage failed only at the unchanged-save idempotence assertion because GNU md5sum rejected `-s`; #43685359507 and #43685359427 failed on stale `write_storage_to_mtd();` source-contract text. This commit corrects those causes; fresh exact-head runs are required. The previous bda checker exception was repaired and 31ae7b6 USB storage #43684904469 passed.
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

1. Fetch live main and inspect the exact-head USB storage push workflow for this commit. Verify the shell retry test, GNU89 persistence gate, embedded RWFS contract and repaired standalone checker all pass; inspect the first failure log if not.
2. Continue P1 with a bounded mtd_storage.sh load/restore/backup/reset audit. Trace callsites, locks, scratch paths, extraction and destructive replacement; add a focused regression for one concrete failure.
3. Do not repeat the MTD save hash-retry fix, RC writer/timestamp status propagation, firmware watchdog recovery, checker literal-brace repair, RWFS/UBI teardown, Transmission/Aria identity stop or checked safe-eject.
4. Use one complete Git tree/commit per logical slice, parented to fresh main; update with force=false and expected_sha; preserve the frozen backup. Keep L3/L4 claims evidence-scoped.

Source/validation notes:
- Parent main for THIS COMMIT: 31ae7b6f253734cc0abf5f67b5c34e861229155b; immutable backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373, artifact #14147062315. Current product source needs a fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.

