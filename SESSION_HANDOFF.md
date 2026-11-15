# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Base main SHA for THIS COMMIT: `dbfcdaba996473dd135c93c574c88f02f6f3bcf0`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). Legacy PR #1 remains CLOSED / UNMERGED; its remaining changes were already ported to main.
- Product source on main includes checked buffered storage writes, storage shutdown persistence gates, MTD save retry/GNU md5sum compatibility, transactional restore, and boot-time MTD load (`dbfcdaba996473dd135c93c574c88f02f6f3bcf0`).
- Exact-head Actions on `dbfcdaba996473dd135c93c574c88f02f6f3bcf0`: storage-hotplug #43688365428 FAILED only at the final standalone source checker (job 132733514178); the POSIX-shell and BusyBox 1.24.2 ash MTD-load regressions and all preceding storage checks passed. The checker incorrectly requires the literal `BUSYBOX_BIN=` inside the test fixture even though the fixture consumes `${BUSYBOX_BIN:-}` and the workflow injects the variable. XY-C1 #43688365628 and NVRAM #43688365369 were IN_PROGRESS. Other exact-head checks: infosvr #43688365536, inventory #43688365376, AliDDNS #43688365481, HTTPD security #43688365384, handoff docs #43688365472, AdGuardHome #43688365455, process lifecycle #43688365347, and AliyunDrive #43688365350 SUCCESS.
- Current source L3 and physical L4 remain NOT VERIFIED. No complete local checkout/toolchain or target hardware is available; no local shell/build result is claimed.

## Active task

### P1 slice — Correct the MTD-load regression source contract (THIS COMMIT)

- The load fixture reads ${BUSYBOX_BIN:-} and runs the production script through "$BUSYBOX_BIN" ash; the storage-hotplug workflow explicitly supplies BUSYBOX_BIN="$busybox_bin" and executes the fixture after its POSIX-shell run.
- Exact-head storage-hotplug #43688365428 on parent dbfcdaba996473dd135c93c574c88f02f6f3bcf0 successfully ran the host and BusyBox 1.24.2 ash fault-injection cases (decompressor emits bytes then fails, partial tar extraction, failed publication/rollback, and complete replacement), as well as all preceding storage checks. It failed only because the source checker looked for an assignment in the fixture file rather than verifying the environment lookup and workflow invocation.
- This correction asserts the fixture's environment lookup, ash invocation, and workflow injection together; it does not relax the regression. No local checkout or local test execution is available. Exact-head Actions for this commit are pending.

### Next P1 slice — Harden MTD backup and HTTPD storage download

- Inspect func_backup() in trunk/user/scripts/files/sbin/mtd_storage.sh and the complete HTTPD do_storage_file() caller path. Trace archive creation, compression status, temp-file lifetime, file existence/size, lock ownership, HTTP response headers/body and error propagation.
- Prevent a failed or partial backup from being streamed as a valid configuration download. Reproduce the failure with fault injection, add a shell regression and source contract, and wire push/PR + target-build path filters as applicable.
- Then retain STORAGE_FIFO_FILENAME after failed do_restore_st_cgi() restores for retry; audit func_reset()/func_erase() directory, erase, lock and hash postconditions.

## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; keep the legacy Boolean API until all callers explicitly handle UNKNOWN. Continue auditing remaining kill_pidfile_s() and direct name-wide signal paths; Linux 4.4 retains a PID/start_time check-to-signal TOCTOU window.
- Next bounded storage task: harden func_backup() and HTTPD do_storage_file() against failed/partial archive publication and invalid streaming; then audit restore FIFO retention, func_reset() and func_erase() postconditions. The transactional load and restore slices are already implemented; do not repeat them.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit with source-backed findings.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 has no inode-conditional unlink-by-fd. Do not claim the race is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.
- Rotate work across userland modules after the storage restore audit; avoid speculative lifecycle changes.

## Verification boundary

- L1: MTD load and restore now validate/stage before publishing and preserve the previous configuration on pre-publication failures. The current checker failure is a false negative in a brittle string assertion, not a failed storage fixture; the correction verifies both the fixture's BusyBox dispatch and workflow environment injection.
- L2: On exact parent dbfcdaba996473dd135c93c574c88f02f6f3bcf0, storage-hotplug #43688365428 / job 132733514178 passed script syntax, MTD save retry, restore transaction, both host/BusyBox-ash load transaction fixtures, policy tests and the full storage caller graph; the last scripts/check_storage_safe_remove.py step failed at line 510 solely on the BUSYBOX_BIN= literal mismatch. Fresh exact-head storage Actions are required after this correction. Other exact-head status is recorded above; XY-C1 #43688365628 and NVRAM #43688365369 were pending at last poll.
- L3: Current source is NOT VERIFIED. A fresh exact-head XY-C1 MIPS/uClibc build, TRX verification and artifact upload are required.
- L4: Physical XY-C1 flash, boot/runtime and storage persistence behavior are NOT VERIFIED.
- No local test, build or physical hardware result is claimed.

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

1. Inspect exact-head storage-hotplug, XY-C1, NVRAM and handoff-doc Actions for this correction commit. Require the storage checker to pass and retain both POSIX-shell and BusyBox 1.24.2 ash coverage; triage any new failure at its first causal log line.
2. Continue the P1 backup/download task: inspect func_backup(), HTTPD do_storage_file(), and the caller that serves STORAGE_FIFO_FILENAME. Add regression coverage for compressor failure after partial output, missing/empty output, backup status propagation and HTTP response suppression.
3. Then fix restore-FIFO retry retention after failed restore and audit func_reset()/func_erase() destructive postconditions.
4. Use a fresh main parent, one complete tree/commit, force=false/expected_sha; keep frozen backup untouched. Current L3/L4 remain NOT VERIFIED.

Source/validation notes:
- Parent main for THIS COMMIT: `dbfcdaba996473dd135c93c574c88f02f6f3bcf0`; immutable backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Historical L3 only: `335f137a831725f1ed241151d849e160578cca69`, XY-C1 #43677484373, artifact #14147062315. Current product source needs a fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.