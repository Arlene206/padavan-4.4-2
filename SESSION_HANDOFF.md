## Current repository state

- Parent main SHA: c7b44c5facfd78736909a869b79f07d6d7f8c5f3. Re-fetch live main before every write because other sessions advance it concurrently.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify or delete it.
- PR #1 remains closed/unmerged. Main contains its core AliDDNS CA-bootstrap/HMAC work with later evolution; do not port the stale PR branch wholesale.
- Main-only development: one complete tree and one commit per logical task, followed by a non-forced expected-SHA update. No force-push, development branches or PRs.

## Active task

### libdisk removable-share descriptor and C89 build repair

- Production fix 552699f8003dbd1e48c2c109e526a77b4b15da8d restored add_folder() and pins the mounted directory through creation, metadata publication and rollback.
- Production fix f22e42974a86bb499cdc30f0b4537bcfaaa0626d restored free_2_dimension_list() and anchored full-volume ACL rebuild/pruning to one pinned directory descriptor.
- Production fix 572ec294eeae01befe36684ed7ab3102b9430e21 placed the descriptor-relative ACL rollback writer before its caller.
- Checker-only commit 3629400054732dc685d102cb2aee0e78b9604e94 removed a duplicate stale full-metadata assertion. Commit c7b44c5facfd78736909a869b79f07d6d7f8c5f3 aligned the add_folder test with its descriptor-relative implementation. The c7 libdisk run passed both host binaries but failed the checker because the failure-path close/transaction order assertion was reversed. This commit fixes that assertion and addresses the separate MIPS C89 compile failure by declaring two helper functions before their first callers.

## Verification boundary

- XY-C1 run 45461770986 on product parent 572ec294eeae01befe36684ed7ab3102b9430e21 FAILED during MIPS musl build: disk_share_account_metadata_status_at() and disk_share_remove_folder_path() were called before their static definitions, creating implicit non-static declarations that conflicted with the later static definitions. TRX/artifact steps skipped. This commit adds prototypes before both callers.
- Exact c7 libdisk run 45462879523: both GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined host binaries PASS; source checker failed at the reversed add_folder ordering assertion. AI handoff run 45462879233 PASS.
- Exact 362940 AI handoff run 45462263766 PASS. Earlier product SHA 572ec: HTTPD security #45461770997, Process inventory #45461771186 and NVRAM audit #45461771441 PASS. L3 NOT VERIFIED; XY-C1 failed before Build completion and TRX/artifact were skipped. Physical-router L4 NOT VERIFIED.
- This source/prototype correction and checker update require fresh exact-head libdisk, AI handoff and XY-C1 runs. No local cross-build is claimed.

## Exact first action for the next GPT

1. Inspect exact-head libdisk, AI handoff and XY-C1 Actions. Confirm both GNU89 ASan/UBSan binaries, source contract, MIPS build, TRX verification and artifact upload.
2. If any MIPS compile error remains, fix the actual declaration/order defect; do not downgrade it to host-only validation.
3. After this build blocker closes, continue the P1 removable-storage audit across metadata writers/renames for mount-path replacement, symlink and stale-inode races, with targeted host regressions.
4. Then continue independent userland stability/security work outside libdisk. PENDING checks do not block independent work.

## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, regression outcomes, exact-SHA Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Implement, test and hand off. Keep one atomic main commit per logical task with non-forced expected-SHA ref updates. Never force-push or overstate firmware or hardware validation. Complete this checkpoint Before context exhaustion.
