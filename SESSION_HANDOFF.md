# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main SHA for THIS COMMIT: `de14e2ec6c3365fd1e9be4cce5c13cb37ace60bc`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only). Legacy PR #1 remains CLOSED / UNMERGED.
- Product baseline includes transactional MTD restore/load/backup/erase/reset and checked blank-MTD hash-before-lock release. The checker-only function-boundary correction is commit `de14e2ec6c3365fd1e9be4cce5c13cb37ace60bc`: exact-head USB #43698297016 PASS and AI docs #43698297005 PASS.
- XY-C1 #43697297850 SUCCESS on product source SHA `8c95ef5ad8a2d4a2ac7755fbe02b7ac2211214b8`, including Build XY-C1, TRX verification and artifact upload. That baseline predates THIS COMMIT's HTTPD product-source change.
- Parent HTTPD security run #43697297903 SUCCESS on source SHA `8c95ef5ad8a2d4a2ac7755fbe02b7ac2211214b8`. THIS COMMIT changes HTTPD upload handling and requires fresh exact-head HTTPD regression plus current-source XY-C1 build/TRX/artifact validation.
- The frozen backup is unchanged. Current-source L3 for THIS COMMIT and physical router L4 are NOT VERIFIED; no physical-device result is claimed.

## Active task

### P1 — Preserve retained HTTPD restore archives across failed replacement uploads (THIS COMMIT)

- `do_restore_st_post()` no longer unlinks `STORAGE_FIFO_FILENAME` or writes directly to that stable path. It requires `get_login_safe()`, validates the multipart boundary, and opens a unique same-directory staging file with mode 0600.
- The multipart parser accepts an optional pre-opened stream and a publish target. Normal firmware/NVRAM uploads keep their one-shot cleanup behavior. Storage restore publishes only after its complete BZ2 multipart payload and boundary validate, followed by checked fflush, fsync and fclose, then same-directory atomic rename.
- Stage-open, parse/write, flush, fsync, close and rename failures remove only the staging file. The prior retry archive remains unchanged until rename succeeds. A per-request ready latch is reset at POST entry and consumed by the CGI output callback; rejected uploads cannot cause an old archive to be restored accidentally.
- A failed MTD restore continues to retain the newly uploaded archive; successful restore still removes it. The existing HTTPD MIME route remains authenticated and POST-only.
- Expanded host C regression covers staging-open, flush/write, fsync, close and rename failures, previous-archive integrity, successful replacement and symlink-entry replacement. Local scratch command `cc -std=gnu89 -Wall -Wextra -Werror -I <httpd-src> tests/httpd_upload_file.c` and execution PASS.
- Exact-head HTTPD, AI docs, USB storage and XY-C1 Actions for THIS COMMIT are pending. The source/test/workflow tree is committed atomically; no CI or current-target build result is claimed until inspected.
## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; retain legacy Boolean until direct callers explicitly handle UNKNOWN. Continue remaining kill_pidfile_s() and name-wide signal audits; Linux 4.4 retains PID/start_time check-to-signal TOCTOU.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 lacks inode-conditional unlink-by-fd. Do not claim it is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.

## Verification boundary

- L1: New restore uploads never unlink/truncate the stable retry archive before successful complete parsing. Same-directory mode-0600 staging plus checked flush/fsync/close and atomic rename means the stable path contains the old complete archive or the new complete archive, never an incomplete replacement.
- L2: The helper regression compiled and passed locally with GNU89 `-Wall -Wextra -Werror`. Fault injection covers staging-open, flush, fsync, fclose and rename; success verifies archive replacement and that rename replaces a symlink entry without following it. The HTTPD workflow already compiles/runs this fixture and now checks source/callback integration.
- L2 exact-head CI for THIS COMMIT: pending. Parent HTTPD #43697297903 on `8c95ef5ad8a2d4a2ac7755fbe02b7ac2211214b8` passed; it is not evidence for the new code.
- L3: XY-C1 #43697297850 succeeded on source `8c95ef5ad8a2d4a2ac7755fbe02b7ac2211214b8`; it predates this HTTPD change. Require a new full MIPS/uClibc build, TRX verification and artifact upload for THIS COMMIT.
- L4: Actual router restore upload, retry after failed restore, reboot and persistence remain NOT VERIFIED.
- No physical-device result is claimed.

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

1. Inspect exact-head HTTPD Authentication/Security, AI handoff docs, USB storage and XY-C1 push Actions for THIS COMMIT. Read and fix any failures. XY-C1 must complete Build XY-C1, Verify TRX output and artifact upload on this exact product source SHA.
2. After this slice is green, continue the P2 DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temporary-file/rollback audit. Avoid repeating already L2-verified storage and service lifecycle work.
3. Keep main-only atomic commits using a freshly fetched expected SHA and non-forced update. Frozen backup remains immutable. Physical L4 remains unverified.

Source/validation notes:
- Parent main for THIS COMMIT: bb2b87054ef9631fc97c0473dd47d7e8b4fccd87; frozen backup: 72f4f157a084f8184b9597ca5bf334b8080e5807.
- Historical current-source build before this MTD fix: product-source SHA 99eeb6fa237b529ec9b9615976961c7a60a02575, XY-C1 #43694958186 completed Build XY-C1, TRX verification and artifact upload. THIS COMMIT requires a fresh build.
- Preserve docs/dev-log archived shards and docs/dev-log/INDEX.md byte-for-byte.
