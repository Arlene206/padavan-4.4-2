## Current repository state

- Parent main SHA reviewed for this checker correction: 3629400054732dc685d102cb2aee0e78b9604e94 (test(libdisk): drop obsolete metadata rebuild assertions). Re-fetch live main before every write because other sessions advance the branch concurrently.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807. Never modify or delete it.
- PR #1 remains closed and unmerged. Main includes its core AliDDNS CA-bootstrap/HMAC work, with later runtime/workflow/checker evolution; do not port the stale PR branch wholesale.
- Main-only development. Create all blobs, one complete tree and one commit, then a non-forced expected-SHA update. Keep one atomic main commit per logical task; never force-push or create development branches.

## Active task

### libdisk removable-share mount identity and ACL consistency

- Production fix 552699f8003dbd1e48c2c109e526a77b4b15da8d restored the header-declared add_folder API and pins the mounted directory for scan/create, folder snapshot, ACL creation and rollback.
- Production fix f22e42974a86bb499cdc30f0b4537bcfaaa0626d restored the truncated free_2_dimension_list destructor and changed full ACL metadata rebuild to preflight, revalidate the account inventory under one NVRAM lease, publish ACL/folder metadata and prune stale entries through one pinned directory fd.
- Production fix 572ec294eeae01befe36684ed7ab3102b9430e21 moved the descriptor-relative rollback ACL writer ahead of add_folder so the definition and call contract are explicit.
- Checker-only commit 3629400054732dc685d102cb2aee0e78b9604e94 removed a duplicate obsolete full-metadata assertion. Its libdisk run then failed at line 1338 because a second legacy add_folder contract still expected pathname-based lookup/mkdir/ACL/rollback although production uses the pinned directory fd. This correction aligns that source contract with the descriptor-relative implementation and regression.

## Verification boundary

- Exact parent 3629400054732dc685d102cb2aee0e78b9604e94, libdisk run 45462263753: both GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined host binaries PASS; the Python source contract failed only at the stale add_folder assertion (line 1338).
- AI handoff run 45462263766 PASS on the same parent SHA.
- Prior product SHA 572ec294eeae01befe36684ed7ab3102b9430e21: HTTPD security #45461770997, Process inventory #45461771186 and NVRAM audit #45461771441 PASS. XY-C1 #45461770986 was still IN_PROGRESS at last poll; Build, TRX and artifact upload had not all been confirmed.
- Parent 362940 and this follow-up modify checker/docs only, not firmware product sources. A same-production-SHA XY-C1 Build/TRX/artifact PASS is still required for L3. Current correction requires fresh exact-head libdisk and AI handoff runs. No local cross-build is claimed; physical-router L4 is NOT VERIFIED.
- Frozen backup remains read-only.

## Exact first action for the next GPT

1. Inspect the exact-head libdisk share-list and AI handoff runs for this correction. Confirm both GNU89 ASan/UBSan binaries and the complete source contract pass.
2. Confirm XY-C1 Build, TRX verification and artifact upload on the identical production-source SHA; do not claim L3 when any step is skipped or pending.
3. Continue the P1 removable-storage audit across metadata writers/renames for mount-path replacement, symlink and stale-inode races. Add focused host regression tests for concrete defects and commit each logical source fix atomically.
4. Do not let PENDING workflows block independent implementation. If a run fails, inspect that exact job log and fix the real contract or production defect without weakening no-follow, descriptor-relative or atomic-publication guarantees.

## Mandatory handoff checkpoint

At session end verify live main, frozen backup, exact active SHA, regression outcomes, exact-SHA Actions and L2/L3/L4 evidence. The checkpoint is mandatory; a document edit is conditional.

# Cross-session execution mandate

Implement, test and hand off. Keep one atomic main commit per logical task with a non-forced expected-SHA update. Never force-push or overstate firmware or hardware validation. Complete this checkpoint Before context exhaustion.
