# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. The next GPT must recover live state once, then immediately resume the exact P1 task below and implement/test/commit—not repeat this session's documentation work, re-audit already completed evidence, or ask the user to choose a task. Do not stop after a plan or status report when safe source work is available. Keep working through bounded engineering slices while context/time permits.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task/progress, relevant exact-head Actions, validation boundary, blockers and exact first action. If any next-session-critical fact is missing, stale or changed, update and commit one consolidated correction (prefer batching with source work). If everything is accurate, leave this file untouched and make no no-op documentation commit. The checkpoint is mandatory; a document edit is conditional.

For every multi-file slice, publish source + regression tests + required workflow/build integration as **one atomic main commit**: create blobs, one tree, one commit parented to the freshly fetched main SHA, then non-forced update of main with that same SHA as expected_sha. Never create partial per-file commits for one logical change. If main moved, rebase/reapply and retest; never force-push. Verify exact-head push Actions. Before context exhaustion, finish a safe slice and leave this handoff materially updated with committed progress, validation and the next exact action. Avoid progress narration; repository progress and a truthful continuation point are the priority.

## Current repository state

- Repository: Arlene206/padavan-4.4-2. Parent main for THIS COMMIT: 31ae7b6f253734cc0abf5f67b5c34e861229155b. Production source includes checked buffered storage writes, radio/custom-hook shutdown gates, checked Optware/RWFS/UBI teardown, and identity-validated Transmission/Aria stops. Recent docs/checker-only commits repaired the duplicated RWFS workflow contract (bda473715cf8bb2e1bbb8c704370639854f2a07c) and a literal-brace ValueError in the standalone checker (31ae7b6f253734cc0abf5f67b5c34e861229155b).
- Immutable backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only. Do not modify, merge or reset it. Legacy PR #1 remains CLOSED / UNMERGED.
- Parent bda473715cf8bb2e1bbb8c704370639854f2a07c USB storage run #43683399235 reached the standalone checker after all previous stages and failed on its Python format-string literal-brace bug; commit 31ae7b6 fixes that checker. Exact-head revalidation is required and no status transfers to THIS COMMIT.
- The workflow-run connector available here filters to pull-request-triggered runs; combined main commit statuses may be empty. Empty results are NOT PASS. No exact-head push CI success is claimed until checked directly.
- Historical L3 only: 335f137a831725f1ed241151d849e160578cca69, XY-C1 #43677484373 (MIPS/uClibc build, TRX verification, artifact #14147062315, 12,595,529 bytes). Current L3 and physical L4 are NOT VERIFIED.
- No full local checkout or target toolchain is available. Do not claim local build/test or hardware validation; newly added regressions are integrated into Actions.

## Active task

### P1 slice — Preserve retryability and propagate MTD storage save failures (THIS COMMIT)

- mtd_storage.sh previously wrote the new archive MD5 directly to storage_md5 before invoking mtd_write. A failed flash therefore caused the next identical save to validate the new hash and skip retrying, leaving the old MTD snapshot active.
- Save now stages the hash in a temporary file and publishes it only after mtd_write succeeds. Hash staging, compression, flash and checkpoint publication failures return non-zero; a failed flash leaves the old hash authoritative.
- write_storage_to_mtd() now propagates doSystem() status. Startup reports failed persistence while continuing crond startup; shutdown logs failure but continues power-down; firmware update refuses image writing unless timestamp and MTD persistence both succeed, and attempts watchdog recovery on abort.
- storage_save_time() now checks clock/localtime conversion and overflow, uses a same-directory temporary file, checks write/flush/fsync/close and atomically renames it. Failure cleans the temp file while preserving the previous timestamp.
- Added a shell regression for baseline save → injected MTD failure → identical retry → unchanged-save skip. Added a GNU89 -Wall -Wextra -Werror persistence-policy regression and expanded Python source/order contracts.
- Changed paths: rc.c, rc.h, mtd_storage.sh, new MTD retry shell test, new persistence policy/header test, storage-hotplug and XY-C1 workflows, check_storage_safe_remove.py, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. Parent main SHA: 31ae7b6f253734cc0abf5f67b5c34e861229155b. Exact-head push Actions and local test/build results are not available here; no CI PASS is claimed.
- Do not repeat already-landed checked buffered writer work (180e90075a11ad1135ee6801915211548337b52e), shutdown radio/custom-hook gates (335f137a831725f1ed241151d849e160578cca69 and d16c7ed591fdec961992f6236a441f12c286fe61), checked Optware/RWFS/UBI teardown (b41fa1fae03904078f0d765d157d2dfdf97daa51 and e89f8ca3da85860e71acfd4f6a50eae4ce096311), or Transmission/Aria identity-bound stop (2aedad115793e7a6de4f2114272c845559105783). Recent bda/31ae commits fixed workflow/checker contracts only.

## Explicitly uncompleted P1 work

- Migrate remaining direct pids() callers by semantic group; keep the legacy Boolean API until all callers explicitly handle UNKNOWN. Continue auditing remaining kill_pidfile_s() and direct name-wide signal paths; Linux 4.4 retains a PID/start_time check-to-signal TOCTOU window.
- Next bounded storage task: audit mtd_storage.sh load/restore/backup/reset for unchecked decompression/extraction, directory replacement, cp/mkdir/metadata publication and rollback failures. Trace caller and lock semantics before changing any destructive path; reproduce one concrete failure.
- Continue DNSMasq public/locked caller graph and process identity/stop/HUP/wait-reap/FD/temp-file/rollback audit with source-backed findings.
- FTP upload cleanup TOCTOU remains blocked; Linux 4.4 has no inode-conditional unlink-by-fd. Do not claim the race is solved.
- NFS durability, AUTH_SYS/root-squash and external-mount compatibility require real client/storage evidence.
- Rotate work across userland modules after the storage restore audit; avoid speculative lifecycle changes.

## Verification boundary

- L1: The MD5 bug is directly traced in func_save(): the old implementation published its skip hash before mtd_write. The new shell fixture exercises failed flash followed by an identical retry and later idempotent skip; source contracts cover script hash order and every RC persistence caller. Not executed locally in this context.
- L2: Storage-hotplug regression and standalone source checker are wired to run on the new main push; exact-head result is PENDING/UNKNOWN until the relevant push run is inspected. Prior bda run #43683399235 failed before full completion on a checker exception fixed by 31ae7b6; that result is not carried forward.
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

