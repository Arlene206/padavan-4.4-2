## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; never substitute the upstream repository.
- Base `main` SHA for THIS vsftpd path-scoped rename-ACL follow-up: `a36c2aa14804e9a8eb45f367672d52cf1f306912`; active branch is `main`. Only `main` exists; there is no separate checkpoint branch.
- Root `dev_log.md` contains 62 complete newest-first records after the CI compile-flag correction entry; it remains below the 384 KiB early-rotation target.

## Latest verified parent checkpoint

The immediate parent `a36c2aa14804e9a8eb45f367672d52cf1f306912` has two exact-head Actions, both carrying the matching full `head_sha`:

- AI handoff documentation contract [42660622526] — SUCCESS; handoff/roadmap and archive integrity checks passed.
- vsftpd configuration publication stability [42660622353] — FAILURE at the new test compile step. The existing configuration and daemon-policy GNU89 regressions passed first. Root cause: `-I trunk/user/vsftpd/src` caused the local vsftpd `features.h` to shadow glibc's system `<features.h>`; this follow-up changes it to GCC's quote-only `-iquote` include path. Reproduced locally with an intentional `features.h` shadow fixture; helper regression then passed. New exact-head Actions for this correction are pending.

## Work completed in THIS patch

- Bundled server is vsftpd 3.0.3; PAM is disabled in `builddefs.h`, and `vsf_sysutil_getpwnam()` routes through the Padavan NVRAM account adapter.
- Padavan NVRAM FTP accounts all map to UID/GID 99, so per-share metadata ACLs are the meaningful account-separation boundary.
- `RNFR` previously checked `PERM_DELETE` against the current working directory rather than the source path, then retained a relative source name that could resolve differently after a later CWD.
- `RNTO` applied the generic deny-file filter but did not check the Padavan destination share's `PERM_WRITE` ACL before `str_rename()`. A path-based cross-share rename could therefore cross into a share where the user lacked write permission.
- Added a bounded parent-path helper that resolves the source/destination parent with `realpath()`, so relative traversal and symlinked parent directories map to the canonical share before applying the existing ACL policy.
- `RNFR` now stores an absolute source path and checks source-parent delete permission. `RNTO` rechecks source delete permission and checks destination-parent write permission before rename.
- Added GNU89 host regression coverage for relative/absolute parent extraction, `..` traversal, symlink canonicalization, root/trailing slash, empty/undersized buffers and missing parents. Extended workflow compile/run, source contracts and push/PR path filters.
- CI follow-up: the first run exposed the source-directory `-I` versus libc `features.h` name collision; the test now uses `-iquote`, preventing project headers from shadowing system headers.

## Active task

1. Verify exact-head Actions for THIS path-scoped FTP rename-ACL correction. Confirm full `head_sha` values and inspect each triggered job/log.
2. Continue the FTP path-boundary audit: `STOR/APPE/STOU`, `RETR`, `MKD/RMD/DELE`, `LIST/NLST/STAT` and direct access to per-account share metadata. Trace parent-versus-target and symlink behavior before applying fixes.
3. Keep NFS custom override-content/ACL and `async` durability, DHCP leasefail/accepted-NAK/release semantics, and FULLCONENAT mapping-hash/dying-tuple namespace ownership on the whole-repository queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the RNFR/RNTO source/destination ACL gap, absolute RNFR path pinning, canonical parent mapping and fail-closed path-resolution errors. |
| L2 host / Actions | The standalone path-parent helper/test compiled with `cc -std=gnu89 -Wall -Wextra -Werror -iquote ...` and passed locally, including a fixture containing a competing source-local `features.h`. Parent AI docs [42660622526] passed; parent vsftpd [42660622353] failed only because the initial workflow used `-I` and is corrected in THIS follow-up. New exact-head Actions are pending. |
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
