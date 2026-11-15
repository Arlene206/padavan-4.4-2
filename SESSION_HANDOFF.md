# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main SHA for THIS COMMIT: 361bf44c724e64f3826b876d43a876bb65fc6039; frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 (read-only). Legacy PR #1 remains CLOSED / UNMERGED.
- Product source baseline remains 2feac289de28bca92469c304a0af13d90c1fb5e9: transactional MTD load/restore, backup integrity, HTTPD backup gating, failed restore upload retention and blank-MTD one-shot lock release. Commits after that are regression/documentation changes only.
- Exact-head USB storage #43691365991 on 361bf44c724e64f3826b876d43a876bb65fc6039 passed shell syntax, save retry, restore transaction, host + BusyBox ash load/backup fixtures, GNU89 policy tests and the complete storage caller graph. It failed only in scripts/check_storage_safe_remove.py:502 because the order assertion selected the first lock-marker removal in the zero-byte failure branch instead of the final successful-load cleanup. THIS COMMIT repairs the assertion while requiring both paths.
- Exact-head AI handoff docs #43691366036 failed because the preceding commit left only one record in dev_log.md and dropped the prior 110-record root history. THIS COMMIT restores those 110 records verbatim from parent 2feac289de28bca92469c304a0af13d90c1fb5e9 and retains the new entry; root count will be 112 after this entry, under the 120-record rotation threshold. No archive/index changed.
- On 361bf44c724e64f3826b876d43a876bb65fc6039, AliyunDrive #43691366014, Process lifecycle #43691366021, infosvr #43691366019, process inventory #43691366005, AliDDNS #43691366013, AdGuardHome #43691366009 and NVRAM #43691366022 SUCCESS. New exact-head storage and AI docs checks are required.
- Current L3 and physical L4 remain NOT VERIFIED. No full local checkout/toolchain or target hardware result is available.

## Active task

### P1 CI repair — validate final MTD lock cleanup and restore lossless dev-log history (THIS COMMIT)

- The storage checker now requires exactly two checked save-lock removal branches: blank-MTD failure and successful load. The ordering assertion uses the last occurrence for success publication/cleanup, not the earlier blank-MTD path.
- Restored the 110 previous root records from parent 2feac289de28bca92469c304a0af13d90c1fb5e9 verbatim; retained the single 361bf44 record. Root log count is 112 after adding this entry, so no archive rotation is due.

### Next P1 task — audit destructive reset/erase postconditions

- Trace func_erase() and func_reset() end-to-end. Check MTD erase status, stale hash deletion, storage directory removal/recreation, lock creation/removal and every partial failure. Add deterministic failure injection for erase failure and directory creation/removal failure. Do not claim rollback after irreversible successful flash erase.
- Restore upload retention, MTD backup integrity and blank-MTD lock release are already implemented; do not repeat these slices.

## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; retain legacy Boolean until direct callers explicitly handle UNKNOWN. Continue remaining kill_pidfile_s() and name-wide signal audits; Linux 4.4 retains PID/start_time check-to-signal TOCTOU.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 lacks inode-conditional unlink-by-fd. Do not claim it is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.

## Verification boundary

- L1: The MTD load path releases the erase lock only for zero-byte flash, preserves it on nonempty corruption, and stages verified data before promotion. The host/BusyBox load fixture now seeds hashes with real newlines and preserves failures across cleanup.
- L2: USB storage #43691365991 on 361bf44c724e64f3826b876d43a876bb65fc6039 passed all shell, MTD host/BusyBox fixture, GNU89 policy and caller-graph steps, then failed only on the stale checker ordering assertion at line 502. This commit corrects it. AI docs #43691366036 failed because dev_log.md had been truncated to one record; this commit restores the previous 110 records plus the 361bf44 and current entries. New exact-head storage/docs Actions required.
- L3: Current product source baseline 2feac289de28bca92469c304a0af13d90c1fb5e9 is NOT VERIFIED by a fresh current-main XY-C1 build/TRX upload. Test/docs commits do not change firmware source.
- L4: Physical erase, blank-MTD next boot, default recreation and later persistence remain NOT VERIFIED.
- No local shell/test/build or hardware result is claimed.

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

1. Inspect exact-head USB storage and AI handoff documentation Actions for THIS COMMIT. Require storage source contract PASS and docs checker pass with at least 20 root records and all indexed archive hashes/counts unchanged.
2. When both pass, resume P1 at func_reset() / func_erase() in trunk/user/scripts/files/sbin/mtd_storage.sh. Make hash, directory and lock operations status-checked; test MTD erase failure, storage removal/recreation failures and lock failure. A successful physical flash erase cannot be rolled back.
3. After reset/erase behavior is covered, review new authenticated restore-upload replacement so retained retry data is not discarded before safe replacement.
4. Before writing, fetch the live main SHA; create complete blob set/tree/commit for every slice and non-force update main using expected_sha. Frozen backup untouched; current L3/L4 unverified.

Source/validation notes:
- Parent main for THIS COMMIT: 361bf44c724e64f3826b876d43a876bb65fc6039; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373, artifact #14147062315. Current source requires fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
