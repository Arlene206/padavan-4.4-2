## Current repository state

# Cross-session execution mandate

Each source/test slice is one atomic main commit: construct one complete Git tree, create one commit, then update main by a non-forced expected-SHA compare-and-swap. Continue this repository as one cross-session engineering stream. Never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow does not block independent safe work.

## Mandatory handoff checkpoint

Every session MUST inspect live repository state before ending: verify main and frozen backup, identify exact SHA, inspect relevant exact-head Actions, record material changes, and name the next concrete implementation task. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, finish a safe slice and preserve exact verification boundaries and first action. If these facts remain accurate, do not create a no-op commit. Never claim a test, build, artifact or hardware check that was not actually performed.

- Parent main before the quarantine declaration repair: `d01e302d2d22365bf1f828b4708dd6fb22ac5234`; follow-up checker parent: `4ee6d7bc1b55c81bab7022e160397c7f7da297d6`.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- PR #1 remains closed/unmerged and deeply behind main; do not port it wholesale.
- Main is the only development ref. Use one complete Git tree and one atomic main commit per logical slice, with expected-SHA non-force updates.

## Active task

### VSFTPD failed-upload cleanup race — private quarantine

The old cleanup compared the open upload FD with the public leaf using `fstatat()`, then unlinked that public name. A concurrent replacement in the gap could cause another writer's file to be deleted.

The current code uses `renameat2(RENAME_NOREPLACE)` into an effective-UID-owned 0700 quarantine directory, verifies the moved inode through the pinned quarantine FD, and unlinks only a matching regular file there. Mismatches/unlink failures attempt a no-replace restore; if a concurrent writer occupies the public leaf, the quarantined item is retained. The reserved directory component is excluded from client paths. No unsafe fallback exists when renameat2 is unavailable.

THIS follow-up removes duplicate `static int` declarations accidentally left by the function-replacement edit. The first exact-head run failed at C compile time before runtime tests; no product behavior has yet been verified.

## Verification boundary

- Exact parent `d01e302d2d22365bf1f828b4708dd6fb22ac5234`, VSFTPD run #45485007449 FAILED at GNU89 compilation with duplicate declaration specifiers in `ftp_acl_path_policy.h`; the C binary and embedded Python contracts did not run.
- Follow-up source SHA `4ee6d7bc1b55c81bab7022e160397c7f7da297d6` VSFTPD #45485311376 compiled all GNU89 binaries, and the C regression reported PASS; its embedded Python contract then failed on a formatting-sensitive exact-string assertion for `fstatat(cleanup_fd, quarantine_leaf, ...)`, whose production call is intentionally split over two lines. This change normalizes whitespace before checking that same call. Exact-head VSFTPD rerun required.
- THIS compile repair changes only the policy header plus log/handoff. A fresh exact-head VSFTPD workflow must compile and run the GNU89 ACL policy test and pass the embedded Python source checks.
- Parent a35ae98: AI handoff #45484687791 PASS. Process lifecycle inventory #45484687868, XY-C1 #45484687835, NVRAM #45484687920, DNSMasq #45484687914, button timing #45484687913, crond #45484687872, combined Process inventory/lifecycle #45484688144 and link detection #45484687806 were pending at last poll.
- Earlier f5b01d9 storage-hotplug #45484032665 and XY-C1 #45484032783 PASS for f5b01d9 only. They do not verify current VSFTPD source.
- No local host compile, MIPS build or physical-router test was performed. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head VSFTPD Actions after the whitespace-normalized source contract. Require GNU89 `-Wall -Wextra -Werror` ACL regression and every embedded Python contract PASS; fix the first causal failure without weakening the quarantine guarantee.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload separately. All required same-SHA steps must pass before claiming L3.
3. Recheck parent a35ae98 process-inventory, NVRAM, DNSMasq, button, crond and link-detection runs by exact SHA; then continue with the next high-risk userland item. Keep backup immutable and main updates expected-SHA/non-force only.
