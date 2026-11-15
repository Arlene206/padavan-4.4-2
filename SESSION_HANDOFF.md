# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main for THIS COMMIT: `a7fe3e4648563f6941dcb0816b59c9a903b5f8bc`; immutable backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Parent `a7fe3e4648563f6941dcb0816b59c9a903b5f8bc` contains the MTD save retry fix and RC timestamp/writer status propagation. Its exact-head USB storage #43686251138 passed MTD syntax, retry, persistence-gate, disk inventory, daemon stop and caller-graph tests, then failed at step 18 in `scripts/check_storage_safe_remove.py` because its function separator searched for a literal backslash-n instead of a newline. Process inventory/lifecycle #43686251333 failed only at checker line 286 on a stale `write_storage_to_mtd();` expectation for the status-returning expression. Both contracts are corrected in THIS COMMIT.
- On the same parent SHA, HTTPD security #43686251157 SUCCESS; Process lifecycle inventory #43686251172 SUCCESS; AI handoff docs #43686251185 SUCCESS; AdGuardHome #43686251162 SUCCESS; infosvr #43686251181 SUCCESS; AliDDNS #43686251183 SUCCESS; AliyunDrive #43686251202 SUCCESS. XY-C1 #43686251137 and NVRAM audit #43686251209 were IN_PROGRESS at last poll; do not infer their outcome.
- A concrete independent restore defect was found in `func_restore()`: it removed the live `/etc/storage` tree before checking the new `mkdir` and `cp`, so an I/O or space failure could destroy the last usable configuration and still return success. The current patch copies into a same-filesystem staging directory, preserves the old tree under a rollback name, atomically renames the prepared tree into place and restores the old tree if installation fails.
- A new shell regression injects staging-copy and install-rename failures and checks the prior files and uploaded archive survive. It is integrated into USB storage Actions alongside a source contract. No local full checkout or target toolchain is available; current L3 and physical L4 remain NOT VERIFIED.
## Active task

### Current P1 slice — make storage restore replacement transactional and repair exact-head contracts (THIS COMMIT)

- Exact-head parent Actions: USB storage #43686251138 reached its final source checker after the MTD retry and storage caller-graph steps passed, then failed because `mtd_save_end` searched for the literal characters `\\nfunc_backup()` instead of a real newline. Process lifecycle #43686251333 failed because its shutdown ordering check still searched for `write_storage_to_mtd();`. The checker now uses the actual function separator and status-returning call expression.
- `func_restore()` had a destructive fail-open sequence: it removed the live storage directory and then ignored `mkdir`/`cp` failures. It now prepares a complete staged tree on the destination filesystem, parks the old tree, atomically installs the staged directory and rolls the old tree back if installation fails. Failed preparation leaves the live tree and upload archive untouched.
- Added `trunk/user/scripts/tests/mtd_storage_restore_transaction.sh`; it injects copy failure and install-rename failure, checks rollback preserves the prior config and archive, then verifies successful replacement. USB storage Actions run this test and source assertions.
- Exact-head USB storage, Process lifecycle and AI documentation checks are required after this commit. Current-source XY-C1 result on parent `a7fe3e4648563f6941dcb0816b59c9a903b5f8bc` was pending; this commit still requires a fresh XY-C1 build/TRX/artifact result for current-source L3.
- No full local firmware checkout/toolchain or hardware validation is claimed.

### Next P1 task — audit MTD load, backup and reset postconditions

- `func_load()` ignores both `bzcat` and `tar xf` statuses and publishes the MD5 cache before validating extraction. Determine whether this can leave partial live config or falsely publish a valid boot snapshot; design the smallest safe stage/commit and failure rollback after tracing startup semantics.
- `func_backup()` reports compression failure but the web download path ignores the script status; trace the upload/download file aliases and ensure a failed archive is never offered as a valid download.
- Audit `func_reset()` directory creation and `func_erase()` postconditions. Add focused fault-injection tests only after defining compatibility behavior. Do not repeat the restore transaction, MTD save retry, RC timestamp/writer propagation, or completed eject/Optware/RWFS gates.

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

1. Fetch live main and inspect exact-head USB storage, Process inventory/lifecycle, Process lifecycle inventory and AI handoff docs for this commit. Verify the restore fault-injection test, checker separator and shutdown ordering checks. Inspect logs for any failure before editing.
2. Finish the P1 audit of `func_load()`, `func_backup()`, `func_reset()`, and the HTTPD storage download path. Preserve boot-time overlay semantics and prove a concrete failure before changing it.
3. Keep source, tests, workflow, checker and handoff updates in one complete tree/commit parented to fresh main; update with `force=false` and `expected_sha`. L3 remains unverified until current exact-head MIPS/uClibc XY-C1 build, TRX and artifact upload pass; L4 remains unverified.

Source/validation notes:
- Parent main for THIS COMMIT: 31ae7b6f253734cc0abf5f67b5c34e861229155b; immutable backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373, artifact #14147062315. Current product source needs a fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.

