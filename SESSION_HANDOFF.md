## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; never substitute the upstream repository.
- Base `main` SHA for THIS vsftpd lifecycle correction: `78508d0de29e64952dafe8d5ae2ca30ab6f20f60`; active branch is `main`. Only `main` exists; there is no separate checkpoint branch.
- Before this entry, root `dev_log.md` had 58 complete newest-first records (about 147 KiB), below the 384 KiB early rotation target and 512 KiB hard limit.

## Latest verified parent checkpoint

The immediate parent `78508d0de29e64952dafe8d5ae2ca30ab6f20f60` has six exact-head Actions, all completed SUCCESS with matching full `head_sha`:

- Samba share stability [42652191209] — host compile/run and complete source-contract step SUCCESS.
- USB storage hotplug [42652191230] — all storage teardown/startup and safe-removal regression steps SUCCESS.
- NFS daemon lifecycle [42652191208] — shell syntax, failure injection and IPv4/IPv6 ACL policy steps SUCCESS.
- HTTPD authentication/security [42652191190] — all policy, disclosure, Samba identity, storage and route checks SUCCESS.
- vsftpd config publication [42652191151] — GNU89 config regression and publication/startup source contracts SUCCESS.
- AI handoff docs [42652191260] — live handoff/roadmap and log integrity validator SUCCESS.

## Work completed in THIS patch

- `stop_ftp()` now returns `kill_services()` status and logs a daemon that did not stop cleanly.
- `stop_stor_apps()` now includes vsftpd stop failure in its status. Existing safe-removal/eject paths already refuse to unmount storage when `stop_stor_apps()` fails, preventing an upload/session daemon that remains active from being hidden while data volumes are detached.
- `run_ftp()` now checks the `eval("/sbin/vsftpd")` result and requires a bounded process-presence postcondition before reporting success.
- `restart_ftpd()` checks stop status and the tri-state storage inventory: unknown inventory aborts, known-empty storage leaves FTP stopped, and mounted storage is required before starting. FTP status now propagates through the RC notification queue, direct start/stop commands, combined `run_ftpsamba` / `stop_ftpsamba` commands and storage startup logging.
- Added a GNU89-testable bounded vsftpd-presence policy and tests for delayed startup, a process that exits during settling, timeout and invalid arguments. Extended vsftpd workflow path filters and semantic source contracts to cover the new lifecycle and storage gates.

## Earlier Samba audit correction

`trunk/user/scripts/files/sbin/dev_init.sh` mounts `/etc` as a 6 MiB tmpfs before creating `/etc/samba`. Therefore `/etc/smb.conf`, `/etc/samba/smbpasswd`, and a journal there are runtime-only and disappear on reboot/power loss. A persistent power-loss journal in `/etc` cannot be durable; same-boot abrupt process termination between paired passdb/config publications remains a separate, lower-priority issue. Do not implement a purported durable journal under `/etc` tmpfs.

## Active task

1. Verify every exact-head Action for THIS vsftpd lifecycle commit, including Samba, USB storage, NFS, HTTPD/security, vsftpd and AI docs. Inspect the failing job logs if any and confirm every full `head_sha`.
2. Continue the FTP security audit: trace local/anonymous authentication and account provisioning, share-mode ACL semantics, anonymous write/delete exposure, chroot/path confinement and symlink traversal against the exact shipped vsftpd version/config. Do not assume the config directives are supported or safe without checking the bundled server source.
3. Keep NFS custom override-content/ACL and `async` durability, DHCP leasefail/accepted-NAK/release state, and FULLCONENAT mapping-hash/dying-tuple namespace ownership on the whole-repository queue.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for FTP stop-result propagation into storage removal, fail-closed tri-state restart gating, command-result checking and bounded daemon-presence postconditions. |
| L2 host / Actions | New FTP daemon-presence GNU89 test was compiled and passed locally from the exact helper/test content. Exact-head Actions for THIS commit are pending. Immediate parent `78508d0de29e64952dafe8d5ae2ca30ab6f20f60` has six exact-head Actions listed above, all SUCCESS with matching full `head_sha`. |
| L3 MIPS/uClibc / production RC object / firmware/TRX | NOT VERIFIED. |
| L4 live FTP authentication/ACL/chroot behavior, storage hot-unplug and physical-router validation | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch list and Actions. Verify all runs for the new full SHA; inspect vsftpd, Samba, storage, NFS, HTTPD and docs job steps.
2. If green, inspect the actual bundled vsftpd version and PAM/passwd integration, then trace every NVRAM share mode to real user IDs, writable directories, anonymous upload/delete policy and chroot escape boundaries. Implement one bounded security or reliability fix with GNU89/source regressions.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update it only when the resumable task/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; when the next entry would reach 120, archive the oldest 100 verbatim and keep the newest 20. Use 384 KiB as the earlier batch-rotation target and 512 KiB as the hard limit.
- Keep legacy `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots/indexes byte-for-byte unchanged. Do not add routine snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only when a full local checkout is available; exact-head GitHub Actions is authoritative for the pushed SHA.
- Recheck live `main`, branches/checkpoints and exact-head Actions at every takeover. Never force-push. Do not claim MIPS build, full firmware/TRX or device validation without evidence.
