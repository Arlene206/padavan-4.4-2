# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved verbatim and indexed. Always fetch live `main` and exact-head Actions before relying on a SHA.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2`, the maintained fork.
- **Latest bounded source milestone:** `cb70977f05b3d7d64dcabf5c02247bda05969f35` — operation-wide storage hotplug/safe-removal serialization plus regression-contract corrections. Its implementation commit is `5d5b61cfc9d5dd3bca237ff4be31eb0118da1dfb`.
- **Branch/checkpoints:** `main`; no separate remote checkpoint branch exists.
- **Exact-head Actions:** USB storage hotplug stability [42564939868](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939868) **SUCCESS**, HTTPD Authentication and Security Regression [42564939905](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939905) **SUCCESS**, and AI handoff documentation contract [42564939843](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939843) **SUCCESS**, all on exact source head `cb70977`. The storage job compiled and ran the forked-process lock regression; HTTPD/security completed all 39 validation steps.
- **Correction history:** first-pass Actions [42564591289](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564591289) and [42564591242](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564591242) failed. The include typo in `services_stor.c` was corrected and the hotplug unlock-order contract now checks the last cleanup unlock (the earlier `file_unlock(isLock)` is the correct early error path). Both workflows passed on `cb70977`.

## Active task

**Storage hotplug / safe-removal serialization — bounded cross-process fix implemented; L1/L2 evidence PASS. Continue the whole caller and shell-entrypoint audit.**

### Concrete race and implementation

- `mdev_sd_main()` and `mdev_mmc_main()` originally took only per-node locks. A whole disk and its partition have different lock identities (for example `sda` and `sda1`, or `mmcblk0` and `mmcblk0p1`), so their hotplug flows did not serialize.
- `safe_remove_stor_device()`, `safe_remove_all_stor_devices()` and PID 1's `umount_ejected()` also mutated storage topology without one operation-wide lock.
- New `storage_operation_lock.h` centralizes the existing `file_lock()` call on `/var/lock/storage.lock`. SD/MMC mdev keeps its per-device lock and then takes the shared operation lock; cleanup releases them in reverse order. Safe removal and eject-all hold the shared lock across inventory discovery and mount/swap/service mutations. PID 1's public `umount_ejected()` wrapper takes the same lock.
- The ejected-unmount body is split into `umount_ejected_locked()` and the public locking wrapper. This is required because `file_lock()` has a same-process re-entry guard; safe removal must call the locked body directly while it already owns `storage.lock`.
- The regression test first demonstrates that disk and partition device locks have distinct identities, then forks a child that holds the shared lock while the parent attempts to acquire it. The exact-head Actions run observed a 2,000 ms block.
- `automount.sh` already has a separate `automount_mountpoint.lock` flock protecting filesystem-label/mountpoint ownership. That is not the operation-wide `storage.lock`; audit all direct shell callers before deciding whether shell-side serialization also needs extending.

### Validation evidence and boundary

- GNU89 `-Wall -Wextra -Werror` lock regression compiled and ran in exact-head storage Actions [42564939868](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939868); existing partition-inventory regression and source/control-flow contracts passed in the same job.
- Full HTTPD/security contract passed on the same source head [42564939905](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939905), including storage mount/swap/safe-eject contracts. The AI documentation validator on the unchanged docs also passed [42564939843](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939843).
- No full production RC object, MIPS/uClibc compile, firmware/TRX build, filesystem integration test or physical router validation has been performed.

## Other open repository work

- **USB / storage / hotplug — PARTIAL / AUDITING:** operation-wide mdev/safe-eject serialization, complete partition-inventory preflight and PID 1 cleanup error propagation have bounded evidence. Direct shell callsites, add/remove event ordering, filesystem runtime and hardware remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT notifier ownership is per-network-namespace, but mapping hashes and dying-tuple queue remain module-global and not namespace-keyed. Mapping isolation, writer parity, rollback and runtime remain open.
- **DNS / DHCP / resolver — PARTIAL / AUDITING:** WAN/IPTV lease validation, route reconciliation, resolver refresh and DHCPv6 DUID handling have bounded evidence; LAN DHCP and remaining lifecycle/error propagation remain open.
- **WAN / PPP / routing — PARTIAL / AUDITING:** selected route-result propagation and WAN stop/teardown failure gates have bounded evidence; broader PPP and runtime routing remain open.
- **Wireless — PARTIAL / AUDITING:** radio-state, bridge-up and RT3352 iNIC lifecycle paths have bounded evidence; other board paths and concurrent restart/stop remain open.
- **HTTPD / CGI / authentication, NVRAM persistence, init/service lifecycle, switch/PHY/MDIO, FTP/Samba/NFS/accounts and integrated third-party userland:** remain separate open areas; no subsystem is declared complete.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/caller review | PASS for the distinct per-device lock identities, shared lock acquisition/release order, mdev SD/MMC, PID 1 eject wrapper, safe-remove and eject-all lock coverage. Shell-level direct callers are still being inventoried. |
| L2 host regression / Actions | Storage GNU89 forked-process lock regression and source contracts **PASS** [42564939868](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939868); full HTTPD/security **PASS** [42564939905](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939905); AI handoff docs **PASS** [42564939843](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939843), all on source SHA `cb70977`. |
| L3 MIPS/uClibc RC compile and complete firmware/TRX | **NOT VERIFIED**. |
| L4 USB unplug/eject, filesystem runtime, flash/boot and physical router behavior | **NOT VERIFIED**. |

## Exact first action for the next GPT

1. Fetch live `main`, confirm the current docs snapshot commit and verify the `AI handoff documentation contract` on that exact SHA. Record its run ID and conclusion; the run above is on the parent source SHA, not this new documentation snapshot.
2. Continue storage caller audit by enumerating every direct call to `/sbin/automount.sh`, `mount`, `umount`, `swapon` and `swapoff`; prove which paths execute under `storage.lock` and identify any direct bypass. Inspect `ejusb/ejata/ejmmc/ejall`, script invocation, PID 1 notification timing and same-process lock re-entry. Avoid introducing nested acquisition of `storage.lock`.
3. Keep FULLCONENAT mapping-table namespace isolation as a separate P2 task. Its notifier per-net ownership is already fixed; mapping hashes and dying-tuple queue remain module-global. Kernel/MTK/DTS work remains outside the ordinary userspace batch.

## Recovery and archive rule

- The superseded active handoff is preserved byte-for-byte as `docs/handoffs/archive/2026/11/part-013.md`; the superseded roadmap as `docs/roadmap-history/2026/11/part-014.md`.
- The 2026-11-09 09:13 CST guest AP scheduler/bridge-state record evicted from the root 20-record window is appended verbatim to `docs/dev-log/2026/11/part-005.md`; its original root source blob is `3bf6391ddb348e17f4c4955ba7658ec5444c8a10`.
- Root `dev_log.md` retains exactly the newest 20 complete entries. Archive byte counts, heading counts and Git blob SHAs remain authoritative.
