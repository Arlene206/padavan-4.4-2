# Cross-session execution mandate

Continue repository work across GPT conversations. Recover once, then implement rather than repeatedly audit completed tasks.

- One logical slice: source modification, regression coverage, one atomic main commit, then exact-SHA Actions inspection.
- Use Git data APIs to create all blobs, one tree and one commit; update main with a non-forced compare-and-swap against the expected parent SHA. Never force-push or expose multi-file partial commits.
- Before context exhaustion, inspect live main and backup refs, exact-head relevant Actions, and local/host test facts; record the first next task and honest verification boundary.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending, including main SHA, backup SHA, exact-head relevant Actions and the first next task.

The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; current product/source commit: THIS COMMIT (parent a6908c5279bee5071759f8b45da3d1a75f27fd6d).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it.
- libdisk now has checked enumeration and folder-list snapshots, bounded file reads, atomic permission metadata writes, bounded account paths, fail-closed add-folder and set-permission handling, and folder delete/rename cleanup that continues across independent account files while retaining the first error.
- add_account() now checks every mounted volume's permission-file initializer, continues after independent volume failures, performs cleanup and Samba reload, and reports the first metadata failure. The existing NVRAM commit order is retained; there is no rollback guarantee.

## Active task

- Next: fix del_account(). It shortens acc_num before `free_2_dimension_list()`, leaking the final account-list entry; it constructs `var_file` with unchecked snprintf and ignores all unlink results. Preserve the original count for freeing, use `disk_share_format_account_var_path()`, treat only ENOENT as a no-op, continue across all mounted volumes, and return the first cleanup error after releasing disk/account inventories and reloading ftpsamba.
- Do not redesign NVRAM transaction/rollback semantics without checking the current NVRAM contract and test harness.

## Verification boundary

- Parent exact-head AI handoff check #45019713382 and libdisk check #45019713462 were queued when this source commit started. The preceding handoff check #45019602967 passed; libdisk #45019603097 still failed in its Python checker because both actual and expected source-call forms needed whitespace normalization. No C sanitizer failure was observed in that run.
- Exact-head Actions for this commit are not yet observed; fetch them live. Do not count an ancestor's PASS as this SHA's PASS.
- XY-C1 L3 is NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all pass on the identical product-source SHA. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current main and inspect exact-head libdisk, handoff, Process inventory, HTTPD, NVRAM and XY-C1 Actions for this commit; diagnose failures from logs.
2. Continue with the isolated del_account() lifecycle fix and source-contract regression described above.
3. Keep the backup branch immutable; claim L3 only with same-SHA Build + TRX + artifact upload PASS.

## Documentation anti-churn guard

Recovery is read-only by default. Never commit merely because a GPT has taken over. This is not a ban on useful documentation-only work. If the current handoff remains accurate, leave it untouched.

