# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Latest source/test checkpoint: THIS COMMIT — vsftpd now stages generated /etc/vsftpd.conf beside the destination, checks formatting/flush/close, atomically renames only a complete file, and prevents daemon startup when publication fails. Base source/test SHA: 85591fa3ea404728bc47a24c4d018148fa974295; resolve this commit's full SHA from live main on the next recovery.
- Remote branch inventory at this recovery contained only main; no remote checkpoint branch exists. Recheck at next takeover.
- The preceding storage caller-graph checkpoint 85591fa3ea404728bc47a24c4d018148fa974295: USB storage regression [42582582537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42582582537) SUCCESS and AI handoff docs [42582582507](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42582582507), both on exact full head_sha 85591fa3ea404728bc47a24c4d018148fa974295.
- The new vsftpd host regression and AI handoff documentation workflow are triggered by THIS COMMIT; their exact resulting head_sha, run IDs and conclusions must be fetched live. Do not infer their status from the preceding green checkpoint.
- Validation ceiling: this round adds a focused GNU89 host regression and source contract. No complete production RC/vsftpd object build, MIPS/uClibc build, firmware/TRX, live FTP login/transfer or hardware validation is claimed.

## Active task

The previous USB storage caller-graph source-contract milestone is complete. Its exact-head storage and documentation Actions passed on 85591fa3ea404728bc47a24c4d018148fa974295. This round's bounded vsftpd generated-configuration publication fix is included in THIS COMMIT; the new exact-head vsftpd and documentation Actions must be reconciled during the next recovery.

### Completed vsftpd configuration reliability fix

- Reviewed the APP_FTPD launch/stop path in trunk/user/rc/src/services_stor.c, RC event dispatch in trunk/user/rc/src/rc.c, exported declarations in rc.h, build inclusion in trunk/user/Makefile and the package Makefiles, and the FTP UI in trunk/user/www/n56u_ribbon_fixed/Advanced_AiDisk_ftp.asp.
- Confirmed defect: write_vsftpd_conf() opened /etc/vsftpd.conf with "w", immediately truncating the last usable config, ignored all fprintf()/flush/close failures, returned no status, and run_ftp() launched vsftpd regardless. A full/read-only filesystem or late write/close error could therefore leave and launch against a partial configuration.
- Fix in THIS COMMIT: use mkstemp() in the destination directory, set the generated file mode, stream the complete configuration to the temporary file, check stream error/flush/close, then atomically rename() it over the destination. Any staging/write/close/rename failure discards the temporary file and preserves the prior config. write_vsftpd_conf() now returns status and run_ftp() skips daemon startup on failure.
- New ftp_config_file.c GNU89 regression covers successful replacement and mode, /dev/full buffered-write failure, failed staging open, truncated staging path and preservation of the last-known-good content. The new workflow also asserts the config publication ordering and daemon-start gate. This regression has not yet been run locally; exact-head Actions are pending at commit creation.
- The focused audit did not establish a further credential/path-confinement bypass. Keep FTP authentication, ACL, path-confinement and stop/lifecycle behavior PARTIAL / AUDITING; this one fix does not close the service audit.
- Validation boundary: no production RC/vsftpd objects, MIPS/uClibc, firmware/TRX, real FTP login/transfer, storage-unplug concurrency or hardware check.

### Next development rotation

After exact-head Actions for THIS COMMIT pass, rotate away from storage/HTTPD to Samba/account/NFS/add-on service lifecycle and permission/error paths. Preserve the DHCP leasefail/accepted-NAK/release callback semantics as a separate userland task, and FULLCONENAT mapping namespace/queue ownership as a separate kernel audit.

### DHCP changes already completed

- b96fa1ee123d45bd15d2e90ff6e4a183ecda0fd0 hardened AP-mode LAN DHCP bound/renew required IP/netmask validation, optional router/DNS/WINS/domain normalization, withdrawn option clearing and lease-duration validation.
- 23321588f38c3ea9a28355ce7ccc50feb06ef1d8 deletes the old default gateway before publishing new temporary lease values. ESRCH means the route is already absent; other route-delete errors abort publication and preserve the previous lease temp fields. The udhcpc applet rejects an empty interface name.
- DHCP policy tests cover route-delete outcomes 0, ESRCH and EIO; deconfig failure precedence; and the bound transaction's route/address/resolver success, each primary failure, `EEXIST` ownership, withdrawn gateway and best-effort rollback. The HTTPD workflow runs the GNU89 test and checks both bound transaction ordering and deconfig gates.
- The prior HTTPD failure on b96fa1e was a stale workflow assertion after a DNSMasq validator moved to dnsmasq_domain_policy.h; the contract was corrected. Exact-head HTTPD/security and docs Actions passed on 2332158.

### Confirmed BusyBox udhcpc callback contract

- Source: `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`. `udhcp_run_script()` is `void`: it calls `spawn_and_wait(argv)` synchronously but discards the child exit status. A nonzero callback result from `udhcpc_lan_main()` does **not** roll back or pause BusyBox's DHCP state machine. Do not make a return-code-only fix.
- `leasefail` is invoked after initial discovery retries are exhausted and after request retries expire (the REQUESTING branch falls back to INIT_SELECTING and jumps to the same label); BusyBox then follows its retry/backoff or `-n` exit policy.
- An accepted NAK invokes `nak`, then invokes `deconfig` only if the previous state was not REQUESTING. When a server ID is known, a NAK from another server is ignored.
- `deconfig` also follows lease expiry/rebind timeout, an ARP-conflicting offered address when not REQUESTING, a repeated SIGUSR1 while already RENEW_REQUESTED, and SIGUSR2/release or SIGTERM exit with release-on-quit. The initial startup `deconfig` call is commented out in this BusyBox fork.
- `route_del()` in `trunk/user/rc/src/net.c` returns 0 or positive errno. `ifconfig()` in `trunk/user/shared/src/netutils.c` returns 0, positive errno for ioctl failures, but -1 for socket/name-copy failures.
- `udhcpc_lan_deconfig()` delegates to an error-returning `lan_down_auto()`. Deconfig deletes the recorded DHCP gateway first (ESRCH is benign; other errors stop before changing the interface), checks static-address `ifconfig()`, then stages an empty resolver file and atomically publishes it under `file_lock("resolv")` using `dnsmasq_file_open()` / `dnsmasq_file_publish()`. Failed staging or publication leaves the previous resolver file intact; temporary lease/gateway identity is reset only after every cleanup stage succeeds. The pure policy regression covers route, interface and resolver status precedence.
- The new bound/renew path snapshots the last-known-good IP/netmask/gateway, checks old-route deletion, interface clear/set, new-route addition and resolver publication before committing temporary lease fields. Resolver output is staged under the shared `resolv` lock and atomically renamed. Any failed operation attempts compensating route/address restoration; route-add `EEXIST` is treated as already present and is not deleted during rollback unless this transaction created it. The host policy test now models each failure stage and rollback errors. BusyBox still discards the callback status, so retry correctness and real DHCP behavior remain open.

## Storage / Optware lifecycle follow-through

- 2c8b5ff9797cbb6a0abf99c5fa18210d09df6671 adds /sbin/optware_lifecycle_lock.sh and a process-shared lock between opt-start.sh and opt-umount.sh. Storage unmount/ejected cleanup invokes the Optware stop hook before detaching relevant mounts, and eject-all avoids a generic unmount fallback after a failed per-device pass. Deferred storage startup uses storage.lock and preserves pending hotplug state / retries on lock acquisition failure.
- 2a5911b24548ac2a1ef06442e0b61771a2714733 fixes the shell regression to source its helper before use.
- c2706ac972970bd325b3b9ebb9bd5311f49125f8 resolves a lock-order inversion: safe eject invokes opt-umount while holding storage.lock, so opt-umount now uses nonblocking `flock -n` via `optware_lifecycle_lock_try_acquire()`. If opt-start currently owns the lifecycle lock, teardown returns failure promptly and storage remains attached for retry instead of blocking while locks are held in reverse order. opt-start retains blocking acquisition. Service child commands close fd 9 so they cannot inherit the lock.
- The lock regression verifies that try-acquire fails promptly while another process owns the lock, and a normal blocking acquire waits until release. Exact-head storage run 42570160275 passed on c2706.
- The mdev/RC/HTTPD/libdisk caller review found no direct bypass in the inspected in-tree SD/MMC hotplug and safe-eject routes. mdev handlers take per-device then shared storage.lock; synchronous _eval waits for automount.sh and its synchronous hooks. Swap, mount and unmount occur within the parent-held lock. automount_mountpoint.lock is a separate mountpoint-ownership lock.
- Fix in THIS COMMIT: `opt-umount.sh` now uses `optware_lifecycle_stop_services()` to attempt every executable S??* hook in reverse order, aggregate failures, and fail closed if the script inventory cannot be enumerated. Any stop failure releases the lifecycle lock and exits nonzero before swapoff or /opt unmount. `optware_lifecycle_umount_opt()` retries once and returns nonzero if the second attempt also fails; the caller explicitly releases the lock and exits with failure instead of masking it. The nonblocking `flock -n` teardown lock and `opt-start.sh` blocking startup lock are unchanged.
- The C `eval()` macro in `trunk/user/shared/src/shutils.h` returns `_eval()`'s command status/errno. Both in-tree `services_stor.c` hook call sites already check `hook_status` and skip the associated mount unmount after failure, so no redundant C production change was necessary.
- New mock-based `optware_lifecycle_teardown.sh` covers reverse-order all-hook attempts, success/failure aggregation, missing versus empty directories, fail-closed inventory enumeration, first-attempt unmount success, successful retry, and both attempts failing. Local shell regression and `sh -n` checks passed; exact-head storage and documentation Actions for THIS COMMIT remain pending.
- External Optware init scripts are user-supplied and not present in the repo. The try-lock removes the direct blocking lock-order cycle when a startup script itself requests eject; their arbitrary side effects still need bounded integration assumptions.
- The storage workflow now includes the broader Python source-contract caller graph. It follows mdev SD/MMC device and shared locks through synchronous automount; checks the separate automount mountpoint lock and child fd closure; verifies swap activation/disable and the fail-closed /proc/swaps + /proc/mounts global cleanup gates; asserts unmount errors gate spindown; and covers safe-remove/eject-all, deferred-start and RC applet/notification/firmware/shutdown wiring. The regression is source-structure/ordering coverage, not a production object or runtime filesystem test.

## Other repository-wide open areas

- USB / storage / hotplug — PARTIAL / AUDITING: complete ejected partition inventory, swapoff-before-unmount, operation-wide storage lock, deferred-start retry, Optware start/stop serialization, nonblocking teardown lock ordering, fail-closed stop-hook aggregation, /opt unmount retry status propagation, and expanded in-tree mdev/automount/swap/eject/spindown/RC caller-graph CI coverage. Still open: arbitrary external custom scripts, target filesystem behavior and hardware.
- WAN / DHCP / resolver — PARTIAL / AUDITING: route validation/error handling, DNSMasq file staging and resolver publication have bounded coverage; LAN DHCP now has fail-closed deconfig cleanup and a checked bound/renew address-route-resolver transaction with rollback. Leasefail/NAK/release policy and live/target validation remain open.
- Firewall / NAT — AUDITING: bounded FULLCONENAT worker/notifier failure fixes exist; mapping-hash and dying-tuple queue namespace isolation remains separate kernel work.
- Wireless — PARTIAL / AUDITING: selected driver result and AP/APCLI/WDS bridge-state publication gates exist; remaining iNIC, board-specific and concurrent lifecycle callers remain open.
- HTTPD / CGI / authentication — PARTIAL / AUDITING: recent security regression passes; continue route/input/error-path inventory.
- NVRAM / init / process supervision / switch-MDIO / FTP / Samba / NFS / accounts / third-party packages — remain open; no broad completion claim.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for the vsftpd generated-config staging/flush/close/atomic-rename control flow and the daemon-start failure gate; previous PASS evidence for AP DHCP transactions and the reviewed in-tree storage caller/lock-order graph remains bounded to those scopes. |
| L2 host / GitHub Actions | Previous storage regression [42582582537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42582582537) and AI handoff docs [42582582507](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42582582507) PASS on exact SHA 85591fa3ea404728bc47a24c4d018148fa974295. The vsftpd GNU89 host regression/source-contract workflow and AI handoff documentation check for THIS COMMIT are PENDING exact-head reconciliation. No local checkout was available for running the C regression or documentation validator independently. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 live FTP login/transfer / storage churn / physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live main, all remote branches and Actions; inspect the vsftpd configuration publication regression and AI handoff-documentation runs for THIS COMMIT, matching each run's full head_sha to the current main SHA.
2. If either exact-head run failed, inspect its job/step logs and correct only the demonstrated test/source-contract or implementation defect. Do not claim target build or hardware results.
3. Once the exact-head checks pass, rotate to Samba/accounts/NFS/add-on services and select one concrete P2 reliability/security defect from launch/stop, configuration, permission or error-propagation paths. The broader FTP credential/ACL/path-confinement/service-lifecycle audit remains open.
4. Preserve open DHCP leasefail/accepted-NAK/release behavior and BusyBox's discarded callback status as a separate task; do not rely on callback return-code-only handling.
5. Keep FULLCONENAT mapping namespace/dying-tuple queue isolation as a separate kernel audit. No full MIPS/uClibc build, firmware/TRX or hardware validation has been performed.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their respective state materially changes; use Git history to retrieve older versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20–119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact; the validator continues checking their inventory and hashes.
- Update only records whose information changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for a changed module status, audit result, or global priority.
- Run `python3 scripts/check_ai_handoff_docs.py` when a local checkout is available. Inspect the exact-head Actions result after pushing. Do not make a follow-up docs-only commit solely to record a completed CI result; GitHub Actions is authoritative.
