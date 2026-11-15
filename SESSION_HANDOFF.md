# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main for THIS COMMIT: `0ed88bde4fe182f057291429e7a94af5149da772`; immutable backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact-head Actions on `0ed88bde4fe182f057291429e7a94af5149da772`: USB storage #43687076965 passed the new restore staging/copy-failure/install-rename rollback regression and all preceding shell/caller tests, then the source checker raised NameError because its expanded path-filter check used undefined `pr_filters` rather than `pull_filters`. Process lifecycle #43687076988 SUCCESS; Process lifecycle inventory #43687076969 SUCCESS; NVRAM transaction #43687076931 SUCCESS; AI handoff docs #43687076907 SUCCESS; infosvr #43687076961 SUCCESS; AdGuardHome #43687076974 SUCCESS; AliDDNS #43687077041 SUCCESS; AliyunDrive #43687076945 SUCCESS. HTTPD security #43687076956 failed only because its inline restore contract still expected the old destructive copy sequence. Both validation contracts are corrected in THIS COMMIT.
- The transactional restore source and its fault-injection test passed on the parent. XY-C1 #43687076955 remained IN_PROGRESS at last poll; do not infer target build/TRX/artifact outcome.
- No local full checkout or target toolchain is available. Current L3 and physical L4 remain NOT VERIFIED.
## Active task

### Current P1 slice — reconcile restore source contracts after transactional replacement (THIS COMMIT)

- On exact parent `0ed88bde4fe182f057291429e7a94af5149da772`, USB storage #43687076965 passed the actual restore copy-failure and rename-failure rollback regression plus MTD save retry, persistence gate, eject inventory and storage caller-graph tests. Its final checker failed because it used undefined `pr_filters` rather than `pull_filters` when checking push and PR path filters.
- HTTPD security #43687076956 passed 33 earlier policy and storage checks, then its inline source contract failed on the old `cp -rf "$tmp_storage"/. "$dir_storage"/` restore sequence. Update that contract to check the prepared same-filesystem stage, copy, move of the old tree, install rename and rollback rename.
- Process lifecycle #43687076988, NVRAM transaction #43687076931, Process lifecycle inventory #43687076969 and AI handoff docs #43687076907 all passed exact-head.
- The P1 restore implementation is retained unchanged; this commit only repairs these two stale test contracts and records the exact outcomes.

### Next P1 task — audit MTD load, backup and reset postconditions

- `func_load()` ignores both `bzcat` and `tar xf` statuses and publishes the MD5 cache before validating extraction. Trace its early-boot caller and overlay semantics; prove whether partial extraction or false metadata publication can harm live configuration before changing behavior.
- `func_backup()` reports compression failure but the HTTPD download path ignores the script status; trace `STORAGE_FIFO_FILENAME` and `do_file()` and ensure failed/partial archives cannot be downloaded as valid backups.
- Audit `func_reset()` directory creation and `func_erase()` postconditions. Do not repeat the transactional restore, MTD save retry, RC timestamp/writer propagation, or completed eject/Optware/RWFS gates.

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

1. Inspect exact-head USB storage, HTTPD security, AI handoff docs and XY-C1 Actions for this commit. Confirm the restore regression and both updated source contracts; inspect logs on any failure.
2. Continue the P1 caller audit: trace early-boot `mtd_storage.sh load`, inspect HTTPD storage backup/download artifact handling, then audit reset/erase behavior. Do not assume load's overlay semantics; confirm the boot sequence first.
3. Commit complete source/test/workflow/docs trees on main using fresh expected SHA and `force=false`; do not update the immutable backup. Current-source L3 requires successful XY-C1 MIPS/uClibc build, TRX and uploaded artifact; L4 requires physical router validation.

Source/validation notes:
- Parent main for THIS COMMIT: 31ae7b6f253734cc0abf5f67b5c34e861229155b; immutable backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373, artifact #14147062315. Current product source needs a fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.

