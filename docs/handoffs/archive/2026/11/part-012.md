# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved byte-for-byte and indexed. Always fetch live `main` and exact-head Actions before relying on a SHA.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2`, the maintained fork.
- **Current source milestone:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a` — stage one complete `/proc/partitions` inventory before ejected-device unmount and propagate cleanup errors into PID 1 notification completion.
- **Current follow-up:** THIS COMMIT fixes the remaining legacy HTTPD source-contract block and archive-index byte counts.
- **Branch/checkpoints:** `main`; no separate remote checkpoint branch exists.
- **Exact-head history:** storage hotplug regression [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) passed on `7fee5c1`. HTTPD run [42561802353](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802353) at `fdd00a4` failed on the second source-contract block, which still expected the removed `procpt2` per-mount scan. AI docs run [42561802333](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802333) failed because two archive-index byte counts were incorrectly written as 15 instead of 6,377/8,180; the archive blobs themselves were preserved. Both issues are corrected in THIS COMMIT and need fresh exact-head Actions.

## Active task

**Storage eject inventory consistency and failure propagation — implementation and focused host test PASS; legacy whole-repository CI contracts being synchronized.**

Caller/lifecycle chain:
- `mdev_sd_main()` / `mdev_mmc_main()` signal `on_unplug_mass_storage` for whole-disk removal.
- PID 1 `handle_notifications()` invokes `umount_ejected()`.
- Before the source fix, `umount_ejected()` staged all `/proc/mounts` rows but reopened `/proc/partitions` for each mount. A later inventory read/parse/close failure could occur after earlier mountpoints had already been unmounted. PID 1 also discarded the cleanup result and could mark a failed cleanup complete.

Implementation at `7fee5c1fc589dc98c3333dd4e052d28881b8756a`:
1. `storage_ejected_parse_partition_line()` validates complete newline-terminated `/proc/partitions` header/data records, numeric major/minor/block fields, a bounded device token and exact field count.
2. `storage_stage_ejected_partitions()` stages and validates one complete partition snapshot, including read/parse/close/allocation errors, before any ejected unmount. The cleanup loop uses the staged names and does not rescan procfs per mount.
3. PID 1 now assigns `action_result = umount_ejected()`; a failed cleanup retains the incomplete notification marker. The common notification failure log is generic.
4. Seven parser assertions and the new `.github/workflows/storage-hotplug-stability.yml` host/source regression passed on exact code head `7fee5c1` (run [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425)).

CI contract synchronization:
- The first legacy `umount_ejected()` assertion block and the new focused workflow were updated and passed up to the later step.
- The HTTPD security workflow had a second independent `ejected_body` block that still asserted `storage_ejected_partition_matches()`, `procpt2` read/close state and per-mount checks. It is now migrated to assert one staged inventory before the unmount and no per-mount partition rescan.
- The docs validator discovered archive-index row sizes 15 for handoff part-010 and roadmap part-011. The file bodies and original blob SHAs were intact; the numeric size columns are corrected to 6,377 and 8,180.

## Other open repository work

- **USB / storage / hotplug — PARTIAL / AUDITING:** per-mount partition-rescan inconsistency and discarded PID 1 cleanup result are fixed. Caller-wide serialization, disk add/remove interleaving, per-device lock identity, filesystem runtime and hardware remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT notifier ownership is per-network-namespace, but mapping hashes and dying-tuple queue remain module-global and not namespace-keyed. IPv4/IPv6 mapping isolation, rollback and runtime remain open.
- **DNS / DHCP / resolver — PARTIAL / AUDITING:** WAN/IPTV lease validation, route reconciliation, resolver refresh and DHCPv6 DUID handling have bounded evidence; LAN DHCP and remaining lifecycle/error propagation remain open.
- **WAN / PPP / routing — PARTIAL / AUDITING:** selected route-result propagation and WAN stop/teardown failure gates have bounded evidence; broader PPP and runtime routing remain open.
- **Wireless — PARTIAL / AUDITING:** radio-state, bridge-up and RT3352 iNIC lifecycle paths have bounded evidence; other board paths and concurrent restart/stop remain open.
- **HTTPD / CGI / auth, NVRAM persistence, init/service lifecycle, switch/PHY/MDIO, FTP/Samba/NFS/accounts and integrated third-party userland:** remain separate open areas; no subsystem is declared complete.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/caller review | PASS for whole-disk remove → PID 1 notification → staged eject cleanup and failure propagation. |
| L2 host regression / Actions | Focused storage workflow PASS [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) on `7fee5c1`. At `fdd00a4`, HTTPD regression failed on the remaining old `ejected_body` assertions [42561802353](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802353); docs validator failed on the two incorrect index byte counts [42561802333](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561802333). This commit updates both and is pending exact-head verification. |
| L3 MIPS/uClibc RC compile and complete firmware/TRX | **NOT VERIFIED**. |
| L4 physical USB eject/disconnect, filesystem runtime and router hardware | **NOT VERIFIED**. |

## Exact first action for the next GPT

1. Fetch live `main` and verify the HTTPD security workflow and AI handoff documentation contract on that exact SHA. Record run IDs and conclusions.
2. If both pass, continue storage hotplug concurrency analysis: disk add/remove event ordering, `safe_remove_usb_device()`, `safe_remove_stor_device()`, `ejusb/ejata/ejmmc`, and `file_lock()` identity/scope. Prove a specific race before selecting the next code fix.
3. Keep FULLCONENAT mapping-table namespace isolation as a separate P2 stability/security task; do not conflate it with notifier per-net ownership. Kernel/MTK/DTS work is outside the normal userspace batch.

## Recovery and archive rule

- Superseded active handoff is archived verbatim as `docs/handoffs/archive/2026/11/part-011.md`; superseded roadmap as `docs/roadmap-history/2026/11/part-012.md`.
- The 2026-11-09 08:56 CST root log record is appended verbatim to `docs/dev-log/2026/11/part-005.md`.
- Root `dev_log.md` retains exactly the newest 20 complete entries; the archive indexes and byte/Git blob SHA checks are authoritative.
