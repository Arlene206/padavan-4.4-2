## Current repository state

- Parent main SHA: be4e55398b9e4038f97f9cbc937686e7c74da96e. The descriptor-pinned del_folder() implementation is THIS COMMIT; fetch live main for its exact SHA before every write.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged; core AliDDNS CA-bootstrap/HMAC changes are integrated on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### Removable-share ACL mount identity

- set_permission() and mod_folder() are pinned to one mount-root descriptor. THIS COMMIT extends the same guarantee to del_folder(): no-follow folder validation, ACL snapshot/publication/rollback, directory staging/removal, and snapshot refresh run against pinned root/staging descriptors. Incomplete directory compensation intentionally preserves the staged payload.
- Host regressions cover mountpoint replacement, ACL write failure rollback, stage-name collision/error/truncation and retained recovery data. Exact-SHA Actions for THIS COMMIT are the required next verification gate.
## Verification boundary

- Parent main be4e55398b9e4038f97f9cbc937686e7c74da96e fixes the XY-C1 C89 prototype error found on 53ef9fe; at last check its exact-head Actions run list had not appeared. Inspect exact results, especially XY-C1.
- THIS COMMIT changes del_folder() production behavior and adds FD deletion rollback/mount-swap tests plus checker assertions. No Actions result for THIS SHA is available at commit time; exact-head libdisk host tests and XY-C1 Build/TRX/artifact are mandatory.
- A failure in directory compensation leaves the folder under the hidden .share-delete-* staging parent and reports incomplete rollback. Do not clean or delete that recovery payload automatically.
- Backup backup/pre-plugin-overhaul-2026-11-14 remains frozen at 72f4f157a084f8184b9597ca5bf334b8080e5807. L3 NOT VERIFIED until Build, TRX and artifact upload all pass on one exact SHA; physical-router L4 NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect this commit's exact-SHA Actions: GNU89 ASan/UBSan libdisk tests, Python source contracts, AI handoff, Process, HTTPD, NVRAM and XY-C1 Build/TRX/artifact. Fix any regression on a new atomic commit.
2. Confirm the be4e55398b9e4038f97f9cbc937686e7c74da96e XY-C1 rerun separately; its prototype fix is present in this commit's parent, but do not infer its build result.
3. Continue the whole-userland stability/security ledger beyond libdisk while PENDING Actions run; preserve the frozen backup.
