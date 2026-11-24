## Current repository state

# Cross-session execution mandate

Continue this repository as one cross-session development stream. A logical source/test change is one atomic main commit with a complete Git tree, followed by an exact-SHA Actions check; never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow is not a reason to stop independent work.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending: verify main and the frozen backup, identify the exact current SHA, inspect relevant exact-head Actions, record any state transitions from PENDING to PASS/FAIL, and name the next concrete implementation task. Before context exhaustion, complete the current safe slice and update this handoff accurately. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no no-op commit. Do not claim tests, builds, artifacts or hardware checks that were not actually performed.


- Parent main SHA: 80f1bfccd255536a0a2682c7d046f0216038a922. This commit restores the mandatory handoff-checkpoint wording required by the documentation validator; fetch live main for its exact SHA before every write.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged; core AliDDNS CA-bootstrap/HMAC changes are integrated on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### Removable-share ACL mount identity

- set_permission() and mod_folder() are pinned to one mount-root descriptor. THIS COMMIT extends the same guarantee to del_folder(): no-follow folder validation, ACL snapshot/publication/rollback, directory staging/removal, and snapshot refresh run against pinned root/staging descriptors. Incomplete directory compensation intentionally preserves the staged payload.
- Exact SHA 80f1bfccd255536a0a2682c7d046f0216038a922: libdisk share-list regression #45471885325 PASS, including GNU89 ASan/UBSan and source-contract checks; AI handoff #45471885677 FAILED because this file omitted the mandatory accuracy-checkpoint section. That section is restored in THIS COMMIT. XY-C1 #45471885211 remains in progress at Build; no TRX/artifact result yet. Process, HTTPD and NVRAM are still in progress at last check.
- Host regressions cover mountpoint replacement, ACL write failure rollback, stage-name collision/error/truncation and retained recovery data. Exact-SHA Actions for THIS COMMIT are the required next verification gate.
## Verification boundary

- Exact main 80f1bfccd255536a0a2682c7d046f0216038a922: libdisk share-list #45471885325 PASS (GNU89 address/undefined sanitizer regression, read_whole_file regression and public API/source contract checks). AI handoff #45471885677 FAILED only because its required mandatory end-of-session checkpoint wording was absent from this file; restored in THIS COMMIT, rerun required.
- XY-C1 #45471885211 is in progress at Build; TRX verification and artifact upload are pending. Parent be4e55398b9e4038f97f9cbc937686e7c74da96e XY-C1 #45469960684 is still in progress at Build. Do not report either as passing until all same-SHA Build/TRX/artifact steps complete.
- Process lifecycle, HTTPD authentication and NVRAM transaction checks on 80f1bf are in progress at last observation; inspect exact results on THIS COMMIT.
- A failure in directory compensation leaves the folder under the hidden .share-delete-* staging parent and reports incomplete rollback. Do not clean or delete that recovery payload automatically.
- Backup backup/pre-plugin-overhaul-2026-11-14 remains frozen at 72f4f157a084f8184b9597ca5bf334b8080e5807. L3 NOT VERIFIED until Build, TRX and artifact upload all pass on one exact SHA; physical-router L4 NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect this commit's exact-SHA Actions: GNU89 ASan/UBSan libdisk tests, Python source contracts, AI handoff, Process, HTTPD, NVRAM and XY-C1 Build/TRX/artifact. Fix any regression on a new atomic commit.
2. Confirm the be4e55398b9e4038f97f9cbc937686e7c74da96e XY-C1 rerun separately; its prototype fix is present in this commit's parent, but do not infer its build result.
3. Continue the whole-userland stability/security ledger beyond libdisk while PENDING Actions run; preserve the frozen backup.
