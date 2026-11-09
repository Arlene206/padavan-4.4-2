# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved byte-for-byte and indexed. Always fetch live `main` and exact-head Actions before relying on a SHA.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2`, the maintained fork.
- **Current source milestone:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a` — stage one complete `/proc/partitions` inventory before ejected-device unmount and propagate cleanup errors into PID 1 notification completion.
- **Current follow-up:** THIS COMMIT — update the pre-existing HTTPD security workflow's safe-eject assertions to match the staged inventory implementation.
- **Branch/checkpoints:** `main`; no separate remote checkpoint branch exists.
- **Exact-head results on source milestone `7fee5c1fc589dc98c3333dd4e052d28881b8756a`:** USB storage hotplug regression [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) **SUCCESS**; AI handoff documentation contract [42561240402](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240402) **SUCCESS**. HTTPD security regression [42561240397](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240397) **FAILURE** solely because the older source contract still demanded the removed per-mount `procpt2` reads. This follow-up migrates that assertion; exact-head rerun is pending.

## Active task

**USB storage eject inventory consistency and failure propagation — implementation and focused regression PASS; one legacy CI contract is being migrated.**

Caller/lifecycle chain:
- `mdev_sd_main()` / `mdev_mmc_main()` signal `on_unplug_mass_storage` for whole-disk removal.
- PID 1 `handle_notifications()` invokes `umount_ejected()`.
- Before the fix, `umount_ejected()` staged all `/proc/mounts` rows but reopened `/proc/partitions` for each mount. A later inventory failure could be observed after an earlier mount had been unmounted. PID 1 also discarded the cleanup return code and could mark the notification complete despite cleanup failure.

Implementation at `7fee5c1fc589dc98c3333dd4e052d28881b8756a`:
1. `storage_ejected_parse_partition_line()` validates complete newline-terminated `/proc/partitions` header/data records, numeric major/minor/block fields, a bounded device token and exact field count.
2. `storage_stage_ejected_partitions()` takes one complete inventory snapshot, checking read, parse, close and allocation failures before any ejected unmount begins. The cleanup loop consults this staged snapshot and no longer reopens procfs per mount.
3. PID 1 now sets `action_result = umount_ejected()`; a failed cleanup retains the incomplete notification marker. The common failure log is no longer incorrectly Wireless-specific.
4. Added seven parser cases, `.github/workflows/storage-hotplug-stability.yml`, and exact-head GNU89/source-order checks.

CI contract correction:
- The new storage workflow passed both compile/run and source wiring on the exact source milestone. The docs validator also passed.
- The existing `.github/workflows/httpd-security.yml` still asserted that `umount_ejected()` had a per-mount `procpt2` file read. That assertion became obsolete when inventory staging was moved before the loop. Its run failed only at this stale contract; this follow-up migrates it to assert the staged snapshot, absence of a rescan and staging-before-unmount order.

## Other open repository work

- **USB / storage / hotplug — PARTIAL / AUDITING:** this round fixes the per-mount partition-rescan failure window and ignored PID 1 cleanup result. Caller-wide serialization, hotplug add/remove interleaving, per-device lock identity, filesystem runtime and hardware remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT notifier ownership is per-network-namespace, but mapping hashes and the dying-tuple queue remain module-global and are not namespace-keyed. IPv4/IPv6 writer parity, rollback and runtime remain open.
- **DNS / DHCP / resolver — PARTIAL / AUDITING:** WAN/IPTV lease validation, route reconciliation, resolver refresh and DHCPv6 DUID handling have bounded evidence; LAN DHCP and remaining lease lifecycle/error propagation remain open.
- **WAN / PPP / routing — PARTIAL / AUDITING:** selected route-result propagation and WAN stop/teardown failure gates have bounded exact-head evidence; broader PPP and runtime routing remain open.
- **Wireless — PARTIAL / AUDITING:** radio-state, bridge-up and RT3352 iNIC lifecycle paths have bounded evidence; other board paths and concurrent restart/stop remain open.
- **HTTPD / CGI / auth, NVRAM persistence, init/service lifecycle, switch/PHY/MDIO, FTP/Samba/NFS/accounts and integrated third-party userland:** remain separate open areas. No subsystem is declared complete.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/caller review | PASS for whole-disk removal → PID 1 notification → staged eject cleanup and error propagation. |
| L2 host regression / Actions | Storage workflow **PASS** [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425), documentation contract **PASS** [42561240402](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240402), both exact `7fee5c1...`. HTTPD workflow legacy contract failed [42561240397](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240397); assertion migration is in THIS COMMIT and awaits rerun. |
| L3 MIPS/uClibc RC compile and complete firmware/TRX | **NOT VERIFIED**. |
| L4 physical USB eject/disconnect, filesystem runtime and router hardware | **NOT VERIFIED**. |

## Exact first action for the next GPT

1. Fetch live `main` and its exact-head Actions. Verify the HTTPD security workflow and AI handoff documentation contract after this assertion migration; record their run IDs and conclusions.
2. If both pass, continue whole-repository storage caller/concurrency audit: trace `mdev_sd_main()`, `mdev_mmc_main()`, `safe_remove_usb_device()`, `safe_remove_stor_device()`, `ejusb/ejata/ejmmc`, disk add/remove interleaving and `file_lock()` identity/scope. Choose the next fix only after confirming a concrete race or stale-state defect.
3. Keep FULLCONENAT mapping-table namespace isolation as a separate P2 stability/security task. Do not conflate it with the prior notifier per-net ownership fix. Kernel/MTK/DTS are outside the normal userspace batch.

## Recovery and archive rule

- Superseded current handoff is preserved verbatim as `docs/handoffs/archive/2026/11/part-010.md`; superseded roadmap as `docs/roadmap-history/2026/11/part-011.md`.
- The 2026-11-09 08:24 CST root log record is appended verbatim to `docs/dev-log/2026/11/part-005.md`.
- Root log contains exactly newest 20 complete records. Archive indexes and byte/Git blob SHA checks remain authoritative.
