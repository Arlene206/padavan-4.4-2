# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved byte-for-byte and indexed. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Latest source/test checkpoint: `c42ce06cfc784e64bb71e44088423abc144e2cd6` — LAN DHCP deconfig fail-closed handling plus atomic resolver staging. Documentation-only commits may advance main; always fetch live HEAD at takeover.
- Remote branch inventory immediately before this refresh contained only main; recheck at next takeover.
- Exact source/test-head HTTPD Authentication and Security Regression [42572938754](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938754) SUCCESS and AI handoff documentation contract [42572938846](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938846), both on `c42ce06cfc784e64bb71e44088423abc144e2cd6`. The storage hotplug regression [42570160275](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42570160275) passed on the earlier storage source checkpoint `c2706ac`.
- Earlier exact-source HTTPD/security [42568701873](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701873) SUCCESS on 2c8b5ff; DHCP lease / renewal and HTTPD checks [42568560276](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568560276) SUCCESS on 2332158. Do not claim these are new exact-head runs on c2706; the exact-head storage and docs runs above are.
- Validation ceiling: source audit and host/Actions tests only. No complete production RC/BusyBox object build, MIPS/uClibc build, firmware/TRX, live DHCP transition, storage hotplug churn or hardware validation. The exact-head HTTPD workflow compiles/runs the DHCP policy test and evaluates RC source contracts; it does not compile the complete `net_lan.c` production translation unit.

## Active task

LAN DHCP lifecycle reliability — AP-mode bound/renew input validation, changed-gateway reconciliation and fail-closed deconfig cleanup now have bounded implementation and host/source-contract tests. Continue the remaining bound-path transaction; keep DHCP PARTIAL / AUDITING.

### DHCP changes already completed

- b96fa1ee123d45bd15d2e90ff6e4a183ecda0fd0 hardened AP-mode LAN DHCP bound/renew required IP/netmask validation, optional router/DNS/WINS/domain normalization, withdrawn option clearing and lease-duration validation.
- 23321588f38c3ea9a28355ce7ccc50feb06ef1d8 deletes the old default gateway before publishing new temporary lease values. ESRCH means the route is already absent; other route-delete errors abort publication and preserve the previous lease temp fields. The udhcpc applet rejects an empty interface name.
- DHCP policy tests cover route-delete outcomes 0, ESRCH and EIO, plus deconfig failure precedence for route, interface and resolver errors. HTTPD workflow contracts verify the bound-path publish gate, deconfig failure gates, and staged resolver publication order.
- The prior HTTPD failure on b96fa1e was a stale workflow assertion after a DNSMasq validator moved to dnsmasq_domain_policy.h; the contract was corrected. Exact-head HTTPD/security and docs Actions passed on 2332158.

### Confirmed BusyBox udhcpc callback contract

- Source: `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`. `udhcp_run_script()` is `void`: it calls `spawn_and_wait(argv)` synchronously but discards the child exit status. A nonzero callback result from `udhcpc_lan_main()` does **not** roll back or pause BusyBox's DHCP state machine. Do not make a return-code-only fix.
- `leasefail` is invoked after initial discovery retries are exhausted and after request retries expire (the REQUESTING branch falls back to INIT_SELECTING and jumps to the same label); BusyBox then follows its retry/backoff or `-n` exit policy.
- An accepted NAK invokes `nak`, then invokes `deconfig` only if the previous state was not REQUESTING. When a server ID is known, a NAK from another server is ignored.
- `deconfig` also follows lease expiry/rebind timeout, an ARP-conflicting offered address when not REQUESTING, a repeated SIGUSR1 while already RENEW_REQUESTED, and SIGUSR2/release or SIGTERM exit with release-on-quit. The initial startup `deconfig` call is commented out in this BusyBox fork.
- `route_del()` in `trunk/user/rc/src/net.c` returns 0 or positive errno. `ifconfig()` in `trunk/user/shared/src/netutils.c` returns 0, positive errno for ioctl failures, but -1 for socket/name-copy failures.
- `udhcpc_lan_deconfig()` delegates to an error-returning `lan_down_auto()`. Deconfig deletes the recorded DHCP gateway first (ESRCH is benign; other errors stop before changing the interface), checks static-address `ifconfig()`, then stages an empty resolver file and atomically publishes it under `file_lock("resolv")` using `dnsmasq_file_open()` / `dnsmasq_file_publish()`. Failed staging or publication leaves the previous resolver file intact; temporary lease/gateway identity is reset only after every cleanup stage succeeds. The pure policy regression covers route, interface and resolver status precedence.
- The bound path also publishes new temporary lease fields before interface configuration; `ifconfig()` results and `lan_up_auto()` route-add / resolver write-close failures are ignored. Review bound/deconfig as one state transition before editing.

## Storage / Optware lifecycle follow-through

- 2c8b5ff9797cbb6a0abf99c5fa18210d09df6671 adds /sbin/optware_lifecycle_lock.sh and a process-shared lock between opt-start.sh and opt-umount.sh. Storage unmount/ejected cleanup invokes the Optware stop hook before detaching relevant mounts, and eject-all avoids a generic unmount fallback after a failed per-device pass. Deferred storage startup uses storage.lock and preserves pending hotplug state / retries on lock acquisition failure.
- 2a5911b24548ac2a1ef06442e0b61771a2714733 fixes the shell regression to source its helper before use.
- c2706ac972970bd325b3b9ebb9bd5311f49125f8 resolves a lock-order inversion: safe eject invokes opt-umount while holding storage.lock, so opt-umount now uses nonblocking `flock -n` via `optware_lifecycle_lock_try_acquire()`. If opt-start currently owns the lifecycle lock, teardown returns failure promptly and storage remains attached for retry instead of blocking while locks are held in reverse order. opt-start retains blocking acquisition. Service child commands close fd 9 so they cannot inherit the lock.
- The lock regression verifies that try-acquire fails promptly while another process owns the lock, and a normal blocking acquire waits until release. Exact-head storage run 42570160275 passed on c2706.
- The mdev/RC/HTTPD/libdisk caller review found no direct bypass in the inspected in-tree SD/MMC hotplug and safe-eject routes. mdev handlers take per-device then shared storage.lock; synchronous _eval waits for automount.sh and its synchronous hooks. Swap, mount and unmount occur within the parent-held lock. automount_mountpoint.lock is a separate mountpoint-ownership lock.
- Residual Optware issue: opt-umount.sh does not check each S??* service stop command's return status and does not check the second umount /opt attempt. A final lock-release command may mask those failures. Verify and fix stop/unmount error propagation before calling storage eject complete.
- External Optware init scripts are user-supplied and not present in the repo. The try-lock removes the direct blocking lock-order cycle when a startup script itself requests eject; their arbitrary side effects still need bounded integration assumptions. The drafted broader Python caller-graph contract remains uncommitted and is not in current CI.

## Other repository-wide open areas

- USB / storage / hotplug — PARTIAL / AUDITING: complete ejected partition inventory, swapoff-before-unmount, operation-wide storage lock, Optware start/stop serialization and nonblocking teardown lock ordering. Still open: stop-hook/second-unmount status propagation, drafted whole-caller-graph CI, external custom scripts, filesystem runtime and hardware.
- WAN / DHCP / resolver — PARTIAL / AUDITING: route validation/error handling, DNSMasq file staging and resolver publication have bounded coverage; LAN DHCP deconfig now has fail-closed atomic resolver cleanup, while bound-side effects/rollback remain open.
- Firewall / NAT — AUDITING: bounded FULLCONENAT worker/notifier failure fixes exist; mapping-hash and dying-tuple queue namespace isolation remains separate kernel work.
- Wireless — PARTIAL / AUDITING: selected driver result and AP/APCLI/WDS bridge-state publication gates exist; remaining iNIC, board-specific and concurrent lifecycle callers remain open.
- HTTPD / CGI / authentication — PARTIAL / AUDITING: recent security regression passes; continue route/input/error-path inventory.
- NVRAM / init / process supervision / switch-MDIO / FTP / Samba / NFS / accounts / third-party packages — remain open; no broad completion claim.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for AP DHCP option validation, changed-gateway delete-before-publish, and fail-closed deconfig route/interface/atomic-resolver cleanup; PASS for the bounded SD/MMC mdev/RC/HTTPD/libdisk/Optware caller and lock-order review. |
| L2 host / GitHub Actions | Exact-head HTTPD/DHCP [42572938754](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938754) and AI docs [42572938846](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938846) SUCCESS on `c42ce06`. Storage [42570160275](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42570160275) SUCCESS on `c2706ac`. The HTTPD workflow compiles/runs the pure DHCP policy test and checks source contracts; it does not compile the full production `net_lan.c` translation unit. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. |
| L4 live DHCP churn / storage hotplug / physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live main, checkpoint branches and exact-head Actions. Latest source/test checkpoint before this documentation refresh is `c42ce06cfc784e64bb71e44088423abc144e2cd6`.
2. Continue the remaining DHCP bound transaction. Deconfig now fail-closes after old route delete (ESRCH benign), static interface setup and atomic staged resolver publication; temporary lease identity is reset only after complete success. Inspect `udhcpc_lan_bound()` / `lan_up_auto()` because they currently publish temp fields before checking address `ifconfig()`, `route_add()` and resolver write/publication outcomes. Define a retry-safe rollback/last-known-good policy that remains correct even though BusyBox discards callback exit status; add GNU89 policy tests and exact source contracts.
3. Audit opt-umount.sh stop-hook and second-umount exit propagation. Ensure the C storage caller sees a nonzero result when teardown did not complete. Preserve the nonblocking try-lock; do not replace it with a blocking lock while holding storage.lock.
4. Re-read current call sites and finish the drafted storage caller-graph Python regression; add it alongside the existing storage_mountpoint_lock.sh and Optware lock tests in the storage workflow. Verify exact-head Actions.
5. Rotate to a different userland module after these bounded tasks. Keep FULLCONENAT mapping namespace work separate. Do not repeat completed SkipDBv2/HTTPD work.

## Recovery and archive rule

- Superseded root handoff snapshots are preserved byte-for-byte as `docs/handoffs/archive/2026/11/part-016.md` through `part-020.md`; the preceding root handoff blob `c38ab64b10c69b4a302b87ddd2cef796bed4923a` is part-020. See `docs/handoffs/INDEX.md`.
- Superseded root roadmap snapshots are preserved byte-for-byte as `docs/roadmap-history/2026/11/part-016.md` through `part-018.md`; the preceding root roadmap blob `7fb99941514b3067268e8758ba74cc4da2d22dbf` is part-018. See `docs/roadmap-history/INDEX.md`.
- The oldest record evicted by this rotation is appended verbatim to `docs/dev-log/2026/11/part-005.md`; see `docs/dev-log/INDEX.md` for exact counts, byte lengths and blob SHA.
- Root `dev_log.md` retains exactly the newest 20 complete records. Run `python3 scripts/check_ai_handoff_docs.py` and the exact-head docs workflow after each handoff update.

