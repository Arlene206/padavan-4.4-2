## Current repository state

# Cross-session execution mandate

Each source/test slice is one atomic main commit: construct one complete Git tree, create one commit, then update main by a non-forced expected-SHA compare-and-swap. Continue this repository as one cross-session engineering stream. Never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow does not block independent safe work.

## Mandatory handoff checkpoint

Every session MUST inspect live repository state before ending: verify main and frozen backup, identify exact SHA, inspect relevant exact-head Actions, record material state transitions, and name the next concrete implementation task. The checkpoint is mandatory; a document edit is conditional. Before context exhaustion, finish a safe slice and preserve exact verification boundaries and first action. If these facts remain accurate, do not create a no-op commit. Never claim a test, build, artifact or hardware check that was not actually performed.

- Parent main for this atomic source/test slice: `a35ae98adefb4d90e6deba8ade947c90e348c5d9`; verify the actual new main tip before writing again.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- PR #1 remains closed/unmerged and deeply behind main; do not port it wholesale.
- Main is the only development ref. Use one complete Git tree and one atomic main commit per logical slice, with expected-SHA non-force updates.

## Active task

### VSFTPD failed-upload cleanup and path safety

Relevant files:
- `trunk/user/vsftpd/src/ftp_acl_path_policy.h`
- `trunk/user/vsftpd/src/tests/ftp_acl_path_policy.c`
- `.github/workflows/vsftpd-stability.yml`

The old cleanup compared the open upload FD with the public leaf using `fstatat()`, then unlinked that public name. A concurrent replacement in the gap could cause another writer's file to be deleted.

THIS commit moves a candidate leaf atomically with `renameat2(RENAME_NOREPLACE)` into an effective-UID-owned 0700 quarantine directory, verifies the moved inode through the pinned quarantine FD, and unlinks only the matching moved entry. A mismatch or unlink failure attempts a no-replace restore. If a concurrent writer has occupied the public leaf, the quarantined object is retained rather than deleting either file. The reserved directory component is excluded from all client paths. There is no unsafe fallback when renameat2 is unavailable.

## Verification boundary

- Parent `a35ae98adefb4d90e6deba8ade947c90e348c5d9`: AI handoff #45484687791 PASS. Process lifecycle inventory #45484687868, XY-C1 #45484687835, NVRAM #45484687920, DNSMasq #45484687914, button timing #45484687913, crond #45484687872, combined Process inventory/lifecycle #45484688144 and link detection #45484687806 were IN_PROGRESS at last poll.
- Parent f5b01d9: storage-hotplug #45484032665 PASS and XY-C1 #45484032783 PASS; this exact evidence closes the checker-only safe-remove follow-up and product build for f5b01d9 only.
- THIS commit changes VSFTPD production policy header, GNU89 C regression, embedded workflow source contract, dev log and handoff in one complete Git tree. Exact-head VSFTPD regression and embedded Python contracts are required. Same-SHA XY-C1 Build, TRX and artifact upload are separately required to establish L3; previous product-source build results do not transfer.
- No local host compile, MIPS build or physical router test was performed. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head VSFTPD workflow; require the GNU89 `-Wall -Wextra -Werror` ACL policy regression and all embedded Python source contracts to PASS. If it fails, fix the causal issue without reintroducing a public-leaf unlink race.
2. Inspect exact-head XY-C1 Build, TRX verification and artifact upload separately; only same-SHA PASS establishes L3.
3. Inspect the parent a35ae98 process-inventory and related exact-SHA runs if they complete. Then rotate to the next high-risk stability/security issue across Padavan userland; keep the frozen backup read-only and use expected-SHA non-force updates only.
