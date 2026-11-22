# Cross-session execution mandate

Continue repository development across GPT conversations. Recover once, then implement. Do not repeat completed audits or ask the user what to do next.

- One logical slice means source change, regression coverage, one atomic main commit, and exact-SHA Actions inspection.
- Use Git data APIs to create all blobs, one tree and one commit. Update `main` with non-forced compare-and-swap against the expected parent SHA. Never force-push or expose a multi-file half-state.
- Before context exhaustion, inspect live main and backup refs, exact-head Actions and tests, then keep this handoff truthful about the first next task.

## Mandatory handoff checkpoint

Every session MUST inspect live repository state before ending, including `main` SHA, backup SHA, relevant exact-head Actions, test facts and the first next task.

The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; current source commit: THIS COMMIT (parent 0a7581f2ef8ad2a04d00fa827626ae28fd85a339).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable.
- libdisk now uses checked directory enumeration and folder snapshots, bounded metadata reads and account paths, atomic permission-file publication, fail-closed folder/account initialization, and first-error propagation for folder delete/rename and account add/delete operations.
- add_account() propagates every mounted-volume permission initializer failure after attempting all volumes and reloading Samba; the NVRAM ordering is retained.
- del_account() now frees the complete original account list, uses bounded metadata paths, ignores only missing-file unlink errors, continues through all mounted volumes, and reports the first cleanup failure after ftpsamba reload.

## Active task

- Next: inspect `mod_account()`. Its per-volume path construction still uses unchecked snprintf() for old/new metadata paths and ignores `rename()` results. Use bounded account path helpers for both paths, distinguish ENOENT from real errors, continue through independent volumes, retain the first error, and preserve existing NVRAM ordering. Add source-contract regression coverage.
- Do not redesign NVRAM transaction/rollback semantics without inspecting the NVRAM transaction API and current CI contract.

## Verification boundary

- Parent exact-head libdisk run #45019873074 PASS on 0a7581f2ef8ad2a04d00fa827626ae28fd85a339. Its two GNU89 ASan/UBSan executables and Python source-contract step all passed.
- Parent AI handoff validation #45019873116 PASS on 0a7581f2ef8ad2a04d00fa827626ae28fd85a339. Earlier exact-head checker failures #45019713462 and #45019778723 were Python contract assumptions only; no C sanitizer failure was observed. The current source-contract normalization is covered by the passing parent run.
- Exact-head Actions for THIS COMMIT must be fetched live; do not count an ancestor PASS as the current SHA PASS.
- XY-C1 firmware L3 is NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all PASS on the identical product-source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head libdisk, handoff, Process, HTTPD, NVRAM and XY-C1 Actions for this commit; fix causal failures from logs.
2. Continue the isolated `mod_account()` rename-path and error-propagation slice with regression contracts.
3. Keep the backup branch immutable. Claim L3 only after same-SHA Build, TRX and artifact upload PASS.

## Documentation anti-churn guard

Recovery is read-only by default. Never commit merely because a GPT has taken over. This is not a ban on useful documentation-only work. If the current handoff remains accurate, leave it untouched.

