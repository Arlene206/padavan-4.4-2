## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; never substitute the upstream repository.
- Base `main` SHA for THIS Samba/storage lifecycle correction: `af3a388fd889238789f9eb0d19efe6b89c2b24e9`; active branch is `main`. The only remote branch is `main`; no separate checkpoint branch exists.
- Root `dev_log.md` had 56 complete newest-first records before this entry (about 142 KiB); no archive rotation is required.

## Latest verified parent checkpoint

The immediate parent `af3a388fd889238789f9eb0d19efe6b89c2b24e9` has two exact-head Actions, both completed SUCCESS with matching full `head_sha`:

- HTTPD authentication/security [42647880398] — all job steps SUCCESS, including Samba share trust and Samba/Unix identity-boundary regressions.
- AI handoff documentation [42647880152] — documentation contract job SUCCESS.

The preceding daemon-lifecycle implementation commit `30432e4636c5b00c96cb8918caaebe37b133a28c` passed Samba, NFS, USB storage, vsftpd and WAN route Actions; its HTTPD signature and AI-handoff heading contract failures were corrected by the immediate parent above. Earlier passdb/config ordinary-error rollback commit `cf75016ebf1f1150cd4db39308ead1e09bb0e66e` passed all six then-required Actions.

## Work completed in THIS patch

- `stop_samba()` now returns the `kill_services()` result and skips TDB/trash cleanup when a requested daemon remains active. `restart_smbd()` does not start a new instance after an incomplete stop.
- `stop_stor_apps()` now includes Samba stop failure in its return status. The existing safe-removal/eject callers already gate unmount on this status, so a still-running `smbd`/WSDD2 no longer gets hidden behind the earlier NFS-only check.
- `restart_smbd()` now treats `storage_has_mounted_devices()` as tri-state: negative means inventory failure and aborts startup, positive starts the storage-backed Samba service, and zero leaves only the WINS path eligible. This avoids treating an unknown mount inventory as “mounted”.
- WINS-only `start_wins()`, `stop_nmbd()`, `reload_nmbd()` and `restart_nmbd()` now report command/stop failures; starts and reloads require bounded process-presence postconditions. Status is propagated through `restart_smbd()` and direct RC stop commands.
- Added a GNU89-testable bounded daemon-presence policy and regression cases for delayed startup, a process that exits during the settling interval, timeout, and invalid arguments.

## Audit correction: no power-loss journal on /etc tmpfs

`trunk/user/scripts/files/sbin/dev_init.sh` mounts `/etc` as a 6 MiB tmpfs before creating `/etc/samba`. Therefore `/etc/smb.conf`, `/etc/samba/smbpasswd`, and any journal placed there are runtime-only and disappear on reboot/power loss; they are rebuilt during startup from persistent configuration/NVRAM. A purported fsync-backed journal in `/etc` cannot provide cross-power-loss durability and should not be implemented as the previous handoff proposed. The normal-return rollback remains useful. Abrupt process termination during the same boot could still leave a temporary passdb/config mismatch; keep that distinct from persistent recovery and assess its practical value before adding complexity.

## Active task

1. Verify the exact-head Actions for THIS commit, especially Samba share stability, HTTPD/security, USB/storage, NFS, vsftpd and the handoff documentation contract. Confirm every returned full `head_sha`.
2. Once green, rotate the main stability audit to vsftpd authentication/credential handling, ACL semantics, path confinement and stop/restart error propagation. Keep the same-boot Samba process-abort window as a lower-priority open item; do not add a non-durable journal under `/etc`.
3. Continue the broader userland queue: NFS custom-override content/ACL and `async` durability, DHCP leasefail/accepted-NAK/release state semantics, and the separate FULLCONENAT mapping-hash/dying-tuple namespace audit.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for Samba stop-result propagation into storage removal, fail-closed tri-state storage gating, WINS-only command results and bounded process-presence postconditions. |
| L2 host / Actions | GNU89 daemon-presence regression and all affected exact-head Actions are pending for THIS commit. The immediate parent `af3a388fd889238789f9eb0d19efe6b89c2b24e9` HTTPD [42647880398] and AI docs [42647880152] runs are SUCCESS with exact matching full `head_sha`. |
| L3 MIPS/uClibc / production RC object / firmware/TRX | NOT VERIFIED. |
| L4 live Samba/WINS behavior, storage hot-unplug and physical-router validation | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live `main`, branch list and Actions. Verify all runs for the exact new full SHA; inspect Samba, HTTPD, storage, NFS, vsftpd and docs job steps/logs.
2. If green, take the next bounded stability task in vsftpd authentication/ACL/path confinement/stop lifecycle. Do not repeat this Samba patch or introduce a “durable” journal in tmpfs.

## Continuation rules

- `SESSION_HANDOFF.md` is the sole actionable continuation point; update it only when the resumable task/checkpoint/next action materially changes.
- Update `REFACTOR_ROADMAP.md` only when module status/evidence/priorities materially change.
- Root `dev_log.md` keeps 20–119 complete newest-first records; when the next entry would reach 120, archive the oldest 100 verbatim and keep the newest 20. Use 384 KiB as the earlier batch-rotation target and 512 KiB as the hard limit.
- Keep legacy `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots/indexes byte-for-byte unchanged. Do not add routine snapshots or docs-only commits.
- Run `scripts/check_ai_handoff_docs.py` only when a full local checkout is available; exact-head GitHub Actions is authoritative for the pushed SHA.
- Recheck live `main`, branches/checkpoints and exact-head Actions at every takeover. Never force-push. Do not claim MIPS build, full firmware/TRX or device validation without evidence.
