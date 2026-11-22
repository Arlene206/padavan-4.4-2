# Cross-session execution mandate

Continue repository development across GPT conversations. Recover once, then implement. Do not repeat completed audits or ask the user what to do next.

- One logical slice means source change, regression coverage, one atomic main commit, and exact-SHA Actions inspection.
- Use Git data APIs to create all blobs, one tree and one commit. Update `main` with non-forced compare-and-swap against the expected parent SHA. Never force-push or expose a multi-file half-state.
- Before context exhaustion, inspect live main and backup refs, exact-head Actions and tests, then keep this handoff truthful about the first next task.

## Mandatory handoff checkpoint

Every session MUST inspect live repository state before ending, including `main` SHA, backup SHA, relevant exact-head Actions, test facts and the first next task.

The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; current source commit: THIS COMMIT (parent 5f00e014e746562927ffe73b75db82333c170300).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable.
- libdisk now uses checked directory enumeration and folder snapshots, bounded metadata reads and account paths, atomic permission-file publication, fail-closed folder/account initialization, and first-error propagation for folder delete/rename and account add/delete operations.
- add_account() propagates every mounted-volume permission initializer failure after attempting all volumes and reloading Samba; the existing NVRAM ordering is retained.
- del_account() frees the complete original account list, uses bounded metadata paths, ignores only missing-file unlink errors, continues through all mounted volumes, and reports the first cleanup failure after ftpsamba reload.
- mod_account() now formats both old/new per-volume metadata paths with the bounded formatter; ignores only missing old files, aggregates other rename/path errors across volumes, frees disk inventory and reloads ftpsamba before returning the first failure. The existing NVRAM commit ordering is unchanged.
- mod_account() also uses the checked storage inventory API, rejects non-regular source metadata and checks for existing target files before rename rather than overwriting stale per-volume permissions.

## Active task

- `mod_account()` now uses `read_disk_data_checked()` so an incomplete inventory is not treated as an empty disk set after the account name has been committed to NVRAM.
- Both metadata paths use the bounded formatter. The old source must be a regular file; an existing new-account metadata destination is rejected with EEXIST rather than overwritten. Missing source metadata is idempotent. Other per-volume path/stat/rename errors are aggregated while continuing independent volumes, followed by inventory cleanup and ftpsamba reload.
- GNU89 policy regressions cover account rename success, ENOENT and real errors; source contracts pin checked inventory, both bounded paths, source/target validation, no-clobber and full error ordering. Parent libdisk #45020631266 passed both sanitizer executables but failed the Python checker on a redundant stale mod_account block. That block was removed in #9ab6be48e197346138804d954433ffb956e507cf; fetch that exact run before using it as baseline.
- NVRAM account name/password updates still precede filesystem migration. A migration failure may leave NVRAM updated while some per-volume metadata remains under the old name; no rollback is claimed.

## Verification boundary

- Parent `074bc11b3338917d1472e5146b31f9c0c59cca25`: AI handoff #45020129047 PASS; libdisk #45020129064 PASS; Process #45020129138 PASS; HTTPD #45020129049 PASS. NVRAM audit #45020129046 was still scanning ioctl/API callers. XY-C1 #45020129134 was building, with TRX and artifact upload pending.
- These are ancestor results only. Inspect exact-head Actions for THIS COMMIT; do not transfer ancestor PASS results.
- XY-C1 L3 remains NOT VERIFIED until Build, TRX verification and artifact upload all PASS on the same exact product/source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head libdisk, AI handoff, Process, HTTPD, NVRAM and XY-C1 push Actions for this commit. Resolve failures from causal logs; do not count ancestor PASS results.
2. Continue outside libdisk: inspect the WAN/service lifecycle inventory for a concrete unchecked failure path (especially start/stop/cleanup paths that report success after partial state change), then add a focused regression and atomic fix.
3. Preserve the frozen backup. Claim XY-C1 L3 only after Build, TRX and artifact upload pass on one exact product/source SHA; keep hardware L4 NOT VERIFIED.

## Documentation anti-churn guard

Recovery is read-only by default. Never commit merely because a GPT has taken over. This is not a ban on useful documentation-only work. If the current handoff remains accurate, leave it untouched.

