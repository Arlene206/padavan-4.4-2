## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; never substitute the upstream repository.
- Base `main` SHA for THIS path-scoped vsftpd rename-ACL correction: `f35d0a8b97db4b4ef283cbd367cd4198d63f6b73`; active branch is `main`. Only `main` exists; there is no separate checkpoint branch.
- Root `dev_log.md` had 60 complete newest-first records before this entry; adding one keeps it within 20–119 and below the 384 KiB early-rotation target.

## Latest verified parent checkpoint

The immediate parent `f35d0a8b97db4b4ef283cbd367cd4198d63f6b73` has three relevant exact-head Actions, all completed SUCCESS with matching full `head_sha=f35d0a8b97db4b4ef283cbd367cd4198d63f6b73`:

- vsftpd configuration publication stability [42654362095] — GNU89 regression tests and source-contract step SUCCESS.
- AI handoff documentation contract [42654362075] — live handoff/roadmap and archive inventory/hash validation SUCCESS.
- HTTPD authentication/security [42654362050] — policy compile/run and semantic source regression SUCCESS.

## Work completed in THIS patch

- Bundled server is vsftpd 3.0.3; PAM is disabled in `builddefs.h`, and `vsf_sysutil_getpwnam()` routes through the Padavan NVRAM account adapter.
- Padavan NVRAM FTP accounts all map to UID/GID 99, so per-share metadata ACLs are the meaningful account-separation boundary.
- `RNFR` previously checked `PERM_DELETE` against the current working directory rather than the source path, then retained a relative source name that could resolve differently after a later CWD.
- `RNTO` applied the generic deny-file filter but did not check the Padavan destination share's `PERM_WRITE` ACL before `str_rename()`. A path-based cross-share rename could therefore cross into a share where the user lacked write permission.
- Added a bounded parent-path helper that resolves the source/destination parent with `realpath()`, so relative traversal and symlinked parent directories map to the canonical share before applying the existing ACL policy.
- `RNFR` now stores an absolute source path and checks source-parent delete permission. `RNTO` rechecks source delete permission and checks destination-parent write permission before rename.
- Added GNU89 host regression coverage for relative/absolute parent extraction, `..` traversal, symlink canonicalization, root/trailing slash, empty/undersized buffers and missing parents. Extended workflow compile/run, source contracts and push/PR path filters.

## Active task

1. Verify exact-head Actions for THIS path-scoped FTP rename-ACL correction. Confirm full `head_sha` values and inspect each triggered job/log.
2. Continue the FTP path-boundary audit: `STOR/APPE/STOU`, `RETR`, `MKD/RMD/DELE`, `LIST/NLST/STAT` and direct access to per-account share metadata. Trace parent-versus-target and symlink behavior before applying fixes.
3. Keep NFS custom override-content/ACL and `async` durability, DHCP leasefail/accepted-NAK/release semantics, and FULLCONENAT mapping-hash/dying-tuple namespace ownership on the whole-repository queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the RNFR/RNTO source/destination ACL gap, absolute RNFR path pinning, canonical parent mapping and fail-closed path-resolution errors. |
| L2 host / Actions | The standalone path-parent helper/test compiled with `cc -std=gnu89 -Wall -Wextra -Werror` and passed locally. Exact-head Actions for THIS commit are pending; parent vsftpd [42654362095], AI docs [42654362075] and HTTPD/security [42654362050] all passed. |
| L3 MIPS/uClibc / production vsftpd object / firmware/TRX | NOT VERIFIED. |
| L4 live FTP cross-share rename, authentication/ACL/chroot, storage hot-unplug and router validation | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch list and Actions. Verify the new full SHA against every triggered workflow and inspect job steps/logs.
2. If exact-head CI is green, continue path-aware permission review of the remaining FTP filename-taking commands. Do not infer that this RNFR/RNTO fix closes other command paths.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update it only when the resumable task/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain the newest 20. Use 384 KiB as the early batch-rotation target and 512 KiB as the hard limit.
- Preserve existing archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only follow-up commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
