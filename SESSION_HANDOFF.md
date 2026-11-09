## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; never substitute the upstream repository.
- Base `main` SHA for THIS vsftpd lifecycle and regression-contract correction: `af76d3b551a9161d1b10ecb1d33dc3cb27d49839`; active branch is `main`. Only `main` exists; there is no separate checkpoint branch.
- Root `dev_log.md` had 59 complete newest-first records before this entry; archive rotation is not required.

## Latest verified parent checkpoint

The immediate parent `af76d3b551a9161d1b10ecb1d33dc3cb27d49839` triggered seven exact-head Actions, all with matching full `head_sha`. Samba [42653764405], USB storage [42653764334], NFS [42653764330], WAN route [42653764278] and AI docs [42653764298] completed SUCCESS. The vsftpd workflow [42653764423] compiled and ran both GNU89 tests successfully, then failed only because its old config-publish contract still searched for a bare `return;` after `write_vsftpd_conf()`, although `run_ftp()` now correctly returns `-1`. HTTPD/security [42653764345] failed only because its FTP config source slice expected the old adjacent `void run_ftp` signature; the new daemon wait adapter now sits between the config writer and the int-returning startup function.

## Work completed in THIS patch

- Corrected the vsftpd source contract to require the error return `return -1;`, matching the new integer-returning startup contract.
- Corrected the HTTPD FTP-config contract to end its slice at the inserted `ftp_daemon_is_running()` adapter, keeping that test focused on `write_vsftpd_conf()` without depending on function adjacency.
- Extended the vsftpd workflow's own path-filter regression to include every newly watched RC file, helper and test.
- These changes correct CI source-contract false negatives; they do not alter production service behavior.

## Functional FTP lifecycle fix on the parent

- `stop_ftp()` returns `kill_services()` status and logs a daemon that did not stop cleanly.
- `stop_stor_apps()` includes FTP stop failure in its result, so existing safe-removal/eject callers refuse to unmount volumes while vsftpd remains active.
- `run_ftp()` checks the `eval("/sbin/vsftpd")` result and requires a bounded process-presence postcondition before reporting success.
- `restart_ftpd()` checks stop status and tri-state mount inventory: unknown aborts, known-empty leaves FTP stopped, mounted storage is required before startup. Status propagates through RC notification, direct run/stop, combined FTP/Samba commands and storage startup logging.
- Added a GNU89-tested daemon-presence policy (delayed start, exit during settle, timeout, invalid arguments).

## Earlier Samba audit correction

`dev_init.sh` mounts `/etc` as a 6 MiB tmpfs. Samba config/passdb and a journal placed there are volatile and disappear on reboot. Persistent power-loss journaling under `/etc` is not possible; same-boot abrupt process death between paired runtime file publication remains a distinct lower-priority question.

## Active task

1. Verify every exact-head Action for THIS regression-contract correction. Inspect vsftpd, HTTPD/security, Samba, USB storage, NFS, WAN route and AI docs. Confirm full `head_sha` values and inspect any failure logs.
2. Once green, audit the actual bundled vsftpd version and PAM/passwd integration, then trace NVRAM share modes to real user IDs, writable directories, anonymous upload/delete policy, chroot/path confinement and symlink traversal. Implement a bounded fix only after following the shipped daemon's implementation.
3. Keep NFS custom override-content/ACL and `async` durability, DHCP leasefail/accepted-NAK/release semantics, and FULLCONENAT mapping-hash/dying-tuple namespace ownership on the whole-repository queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for FTP stop-result propagation, storage safe-removal gating, fail-closed tri-state restart, checked start result and bounded daemon-presence postcondition. |
| L2 host / Actions | New FTP daemon-presence GNU89 test compiled and passed locally and in parent Action [42653764423]; FTP config publication GNU89 test also passed in that run. Parent static source contracts had two stale assumptions and are corrected here; exact-head Actions for THIS correction are pending. |
| L3 MIPS/uClibc / production RC object / firmware/TRX | NOT VERIFIED. |
| L4 live FTP authentication/ACL/chroot behavior, hot-unplug and router validation | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch list and Actions; verify all runs for the new exact SHA and inspect job steps.
2. If green, audit the shipped vsftpd code/config/authentication and account mapping, then implement one focused security or reliability fix with GNU89/source regression coverage.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update it only when the resumable task/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; at 120, archive the oldest 100 verbatim and retain the newest 20. Use 384 KiB as the early batch-rotation target and 512 KiB as the hard limit.
- Preserve existing archive snapshots/indexes byte-for-byte; do not add routine archive snapshots or docs-only CI follow-up commits.
- Run `scripts/check_ai_handoff_docs.py` only with a full local checkout; exact-head Actions are authoritative for the pushed SHA.
- Recheck live `main`, branches and exact-head Actions on every takeover. Never force-push. Do not claim target builds or device tests without evidence.
