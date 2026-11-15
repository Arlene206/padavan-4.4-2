# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Base main SHA for THIS COMMIT: `0baeb3d38480464bcfc7c3127847d234caca226a`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). Legacy PR #1 remains CLOSED / UNMERGED.
- Product work already on main includes checked storage writes/persistence shutdown gates, transactional MTD save/load/restore, and MTD backup validation + HTTPD download gating from parent `0baeb3d38480464bcfc7c3127847d234caca226a`.
- Exact-head parent USB storage #43689545333 and HTTPD security #43689545379 FAILED in newly added source-contract boundary lookups: Python string boundaries used literal escaped backslash-n, not an actual newline. Product source validation was not green at this parent. This commit replaces these lookups with `chr(10)` and fixes the newly confirmed restore upload-retention bug.
- Parent `0baeb3d` Actions: AdGuardHome #43689545261, Process lifecycle #43689545223, AliyunDrive #43689545256, Process inventory #43689545238, infosvr #43689545251, AliDDNS #43689545236, NVRAM #43689545257, FRP #43689545218 and AI handoff docs #43689545233 SUCCESS. XY-C1 #43689545219 was IN_PROGRESS at last poll. These do not mark firmware L3 complete.
- Current source L3 and physical L4 remain NOT VERIFIED. No full local checkout/toolchain or target hardware is available; no local test/build result is claimed.
## Active task

### P1 slice — Preserve failed restore uploads for retry; repair exact-head source contracts (THIS COMMIT)

- Parent exact-head USB storage #43689545333 and HTTPD security #43689545379 failed in source-contract parsing: backup/HTTPD Python boundary lookups searched for literal backslash-n strings rather than actual newlines. This commit uses `chr(10)` for these source boundaries.
- `do_restore_st_cgi()` unconditionally unlinked `STORAGE_FIFO_FILENAME` after invoking restore, including on failure. That discarded an upload even though the transactional restore routine intentionally preserves retryable input on extraction/publication failure.
- The handler now deletes the uploaded archive only inside the `ret == 0` success branch. On restore failure it shows the error response and leaves the archive available for retry.
- Added HTTPD source regression requiring exactly one unlink in the success branch and none in the error branch. Corrected storage checker and HTTPD workflow newline delimiters so their source contracts can execute.
- Changed paths: `web_ex.c`, `check_storage_safe_remove.py`, `httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Parent SHA `0baeb3d38480464bcfc7c3127847d234caca226a`; exact-head Actions pending. No local test/compile is claimed.
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

1. Inspect exact-head HTTPD security and USB storage Actions. Confirm backup source-boundary checks, failed-restore upload-retention assertions, MTD backup host/BusyBox tests and the full caller graph.
2. Continue P1 with `func_reset()` and `func_erase()`: they ignore directory/hash/lock operation statuses; erase success followed by failed settings-directory recreation can leave startup without its expected directory. Add error propagation and failure-injection coverage; do not claim MTD rollback after physical erase.
3. Review restore retry UX and expiry: a failed upload should remain available for retry, but the next authenticated upload must replace stale retry data only when the new upload safely begins.
4. Use a fresh main SHA, one complete Git tree/commit, force=false and expected_sha. Frozen backup untouched. Current-source L3 and physical L4 remain NOT VERIFIED.

Source/validation notes:
- Parent main for THIS COMMIT: `dbfcdaba996473dd135c93c574c88f02f6f3bcf0`; immutable backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Historical L3 only: `335f137a831725f1ed241151d849e160578cca69`, XY-C1 #43677484373, artifact #14147062315. Current product source needs a fresh build/TRX/upload.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.