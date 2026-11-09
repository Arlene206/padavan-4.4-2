# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed; routine revisions of this live file are preserved by Git history. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Latest source/test checkpoint: THIS COMMIT — generate Samba smb.conf into a private same-directory staging file; check stream/flush/close; atomically publish only complete output; treat share-inventory errors as fatal; gate WINS startup and nmbd/smbd reload on publication success. Base main SHA `c51c0f73fa2dccdd191515347acf80fd2a0132d2`; reconcile this commit's full exact head SHA and Actions after push.
- Remote branch inventory at this recovery contained only main; no remote checkpoint branch exists. Recheck at next takeover.
- The preceding commit `c51c0f73fa2dccdd191515347acf80fd2a0132d2` passed vsftpd [42586171489](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171489), storage [42586171713](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171713), HTTPD [42586171537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171537) and AI handoff docs [42586171552](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171552), all SUCCESS on exact full SHA `c51c0f73fa2dccdd191515347acf80fd2a0132d2`. New Samba regression and this commit's exact-head workflow results are pending.
- Validation ceiling: the preceding vsftpd GNU89/source regression and all four exact-head workflows passed on `c51c0f73fa2dccdd191515347acf80fd2a0132d2`. The Samba atomic-publisher helper/test compiled with GNU89 `-Wall -Wextra -Werror` and passed in a scratch host directory. No complete production RC/Samba object build, MIPS/uClibc build, firmware/TRX, live share mount/authentication or router test is claimed.

## Active task

Samba share-configuration publication reliability — the existing smb.conf writer deleted/truncated the active configuration before generation had succeeded, and multiple service paths did not gate startup/reload on successful generation. This checkpoint stages the complete replacement, validates write/close status, atomically replaces the live file, and keeps the previous configuration intact on failure.

### Samba generated configuration and caller path reviewed

- `services_ex.c:write_smb_conf_header()` previously unlinked `SAMBA_CONF` and then opened it with `"w"`; any open or later generation failure could therefore remove the last-known-good file. `services_stor.c:write_smb_conf()` subsequently wrote share definitions directly to the live file, closed and reopened it in append mode while scanning for duplicate share names, and only discovered some write errors late.
- The user-folder inventory path continued when `get_folder_list_in_mount_path()` failed. That allowed the service to silently publish an incomplete set of SMB shares rather than treating an unreadable inventory as a failed generation.
- `start_wins()` previously ignored header-write/close failure and started nmbd regardless. `restart_nmbd()` ignored `write_smb_conf()` failure and proceeded with nmbd startup and smbd reload; it also stopped nmbd before proving that the replacement config could be generated.
- New `samba_config_file.h` stages beside `SAMBA_CONF` using `mkstemp()`, applies the existing 0644 config-file mode, checks stream/flush/close status, and uses same-directory `rename()` only after successful generation. Error paths close/unlink the candidate and leave the active config untouched.
- `write_smb_conf_header(FILE *fp)` is now a stream-only emitter; it no longer unlinks or opens the live path. `write_smb_conf()` writes the header and all share sections to the staged path, scans that same staged file for exact section-name collisions, fails closed on folder-inventory/read/append errors, frees the relevant lists on error, and publishes only at the end.
- `start_wins()` stages and publishes its WINS-only configuration before launching nmbd. When smbd is already running, `restart_nmbd()` now generates the replacement first; a failure preserves the running nmbd/smbd state, while success stops/restarts nmbd and only then signals smbd to reload.
- New `tests/samba_config_file.c` covers successful replacement and old-file visibility before rename, mode 0644, buffered `/dev/full`/ENOSPC failure, rename failure cleanup, explicit discard, missing-parent open failure and short-path-buffer handling. `.github/workflows/samba-share-stability.yml` compiles the host test with GNU89 `-Wall -Wextra -Werror` and checks the writer/caller failure gates and exact path filters.
- The helper/test were compiled and run in a scratch host directory: **PASS**. Source-contract assertions were also checked against the staged source blobs. The full Python workflow contract and docs validator have not run locally because no repository checkout is available; new exact-head Samba Actions are pending.
- Keep Samba PARTIAL / AUDITING. This fixes one config-publication and lifecycle failure mode; account/passdb mutation consistency, ACL/permission mapping, credential security and actual target service behavior remain open.

### Previous adjacent milestone — vsftpd

- The preceding exact-head milestone fixed generated `/etc/vsftpd.conf` truncation using same-directory staging, checked flush/close and atomic rename, with daemon startup gated on publication success. A follow-up fixed the int-valued writer's publication-error return to `-1`.
- vsftpd, storage, HTTPD and AI handoff workflows all passed on exact SHA `c51c0f73fa2dccdd191515347acf80fd2a0132d2`. Keep broader FTP authentication, account/ACL, path-confinement and stop/lifecycle audit PARTIAL / AUDITING.

### Next development rotation

After the Samba, FTP, storage, HTTPD and documentation Actions pass on the new exact head SHA, rotate into NFS mount/permission lifecycle and third-party add-on service supervision rather than staying in Samba. Preserve DHCP leasefail/accepted-NAK/release state semantics and the FULLCONENAT mapping/queue namespace audit as separate tasks.

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
| L1 source review | PASS for SMB generated-config staging/atomic publish, staged duplicate-share inspection, fail-closed folder inventory, WINS startup gate and nmbd/smbd reload ordering; PASS for the preceding vsftpd config publication/error-return fix and earlier bounded DHCP/storage work. |
| L2 host / GitHub Actions | vsftpd [42586171489](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171489), storage [42586171713](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171713), HTTPD [42586171537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171537) and docs [42586171552](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171552) all PASS on exact SHA `c51c0f73fa2dccdd191515347acf80fd2a0132d2`. New Samba helper/test passed a local scratch GNU89 `-Wall -Wextra -Werror` host run; new Samba workflow and this commit's exact-head Actions are pending. No full production RC/Samba compile or target/hardware test. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 live FTP login/transfer / storage churn / physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live main, all branches and Actions. Match the new Samba share stability, vsftpd, storage hotplug, HTTPD and AI handoff-doc runs against the current main's complete full `head_sha`; inspect any failed job/step logs.
2. If all exact-head jobs pass, rotate to NFS mount/permission lifecycle and add-on service supervision. Trace the mount/unmount callers, generated config and external init ordering before choosing one bounded P2 issue.
3. Keep Samba account/passdb mutation consistency, ACL/permission mapping and credential security open. Keep the broader vsftpd authentication/ACL/path-confinement/stop audit open; the bounded atomic config fix does not complete either service.
4. Preserve DHCP leasefail/accepted-NAK/release state semantics and BusyBox's discarded callback status as a separate userland task; do not rely on callback return-code-only handling.
5. Keep FULLCONENAT mapping-hash and dying-tuple queue namespace isolation as a separate kernel audit. No full MIPS/uClibc, firmware/TRX or hardware validation has been performed.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their respective state materially changes; use Git history to retrieve older versions. Do not create new routine handoff/roadmap archives.
- `dev_log.md` accumulates newest-first entries from 20–119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact; the validator continues checking their inventory and hashes.
- Update only records whose information changed: log for a coherent milestone; handoff for a changed checkpoint/next action or before stopping; roadmap for a changed module status, audit result, or global priority.
- Run `python3 scripts/check_ai_handoff_docs.py` when a local checkout is available. Inspect the exact-head Actions result after pushing. Do not make a follow-up docs-only commit solely to record a completed CI result; GitHub Actions is authoritative.
