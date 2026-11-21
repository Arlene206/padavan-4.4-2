# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved byte-for-byte and indexed. Always fetch live `main` and exact-head Actions before relying on a SHA.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2`, the maintained fork.
- **Source milestone:** THIS COMMIT — storage eject cleanup now stages a complete partition inventory before unmounting and propagates cleanup errors into PID 1 notification completion.
- **Documentation baseline:** THIS COMMIT; superseded active handoff/roadmap snapshots and the evicted development-log record are archived and indexed in this change.
- **Branch/checkpoints:** `main`; no separate remote checkpoint branch was present at the recovery baseline.
- **Base main SHA:** `692b7b2372dd4ccabab879a53ddcd78aad69e751`.

## Active task

**USB storage eject inventory consistency and error propagation — implemented; exact-head regression pending.**

Caller and lifecycle trace:
- `mdev_sd_main()` / `mdev_mmc_main()` call `notify_rc("on_unplug_mass_storage")` for whole-disk removal. PID 1 consumes the notification in `handle_notifications()` and calls `umount_ejected()`.
- `umount_ejected()` collected a complete `/proc/mounts` inventory before side effects, but reopened `/proc/partitions` for every staged mount entry. A later partition-inventory failure could occur after earlier mountpoints had already been unmounted, creating partial cleanup within one eject event.
- The PID 1 notification branch ignored the integer result from `umount_ejected()`; the common notification-completion logic could therefore remove the incomplete marker even when cleanup failed.

Implementation in THIS COMMIT:
1. Added a strict `storage_ejected_parse_partition_line()` contract for complete `/proc/partitions` records. It accepts the validated heading and blank lines, requires numeric major/minor/block fields, a bounded device token, exactly four fields and a terminating newline.
2. Added one operation-wide `/proc/partitions` staging pass in `services_stor.c`. Read, parse, close or allocation failure aborts before any ejected mount is unmounted. The cleanup loop uses the validated snapshot and no longer reopens procfs per mount.
3. Changed the PID 1 `on_unplug_mass_storage` branch to assign `action_result = umount_ejected()`; failed cleanup retains the incomplete notification marker and is logged as a failed notification rather than falsely completing.
4. Extended `storage_ejected_inventory_policy.c` and added `.github/workflows/storage-hotplug-stability.yml` for GNU89 host regression and source-order contracts.

## Other open repository work

- **USB / storage / hotplug — PARTIAL / AUDITING:** this round closes the per-mount partition-rescan failure window and discarded-notification-result gap. Caller-wide serialization, hotplug add/remove interleaving, per-device lock identity, filesystem runtime and target hardware remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT notifier ownership is per-network-namespace, but mapping hashes and the dying-tuple queue remain module-global and are not namespace-keyed. IPv4/IPv6 writer parity, rollback and runtime remain open.
- **DNS / DHCP / resolver — PARTIAL / AUDITING:** WAN/IPTV lease validation, route reconciliation, resolver refresh and DHCPv6 DUID file handling have bounded evidence. LAN DHCP input handling, deconfig/leasefail/NAK/decline/release semantics, error propagation and runtime integration remain open.
- **WAN / PPP / routing — PARTIAL / AUDITING:** selected route-result propagation and WAN stop/teardown failure gates have exact-head host evidence; broader PPP and runtime routing remain open.
- **Wireless — PARTIAL / AUDITING:** radio-state, bridge-up and RT3352 iNIC lifecycle paths have bounded evidence; other board-conditional paths and concurrent restart/stop remain open.
- **HTTPD / CGI / authentication, NVRAM persistence, init/service lifecycle, switch/PHY/MDIO, FTP/Samba/NFS/accounts and integrated third-party userland:** remain separate open areas in `REFACTOR_ROADMAP.md`. No subsystem is declared complete by this storage fix.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/caller review | PASS for the whole-disk remove → PID 1 notification → ejected-mount cleanup path, the inventory failure window and error-result propagation. |
| L2 host regression / GitHub Actions | PENDING for THIS COMMIT: storage partition parser/test, source-order contract and exact-head AI documentation contract. |
| L3 MIPS/uClibc RC compile and complete firmware/TRX | **NOT VERIFIED**. |
| L4 flash/boot, real USB disconnect/eject, mount teardown and physical router behavior | **NOT VERIFIED**. |

No production MIPS/uClibc compile, full firmware image, live filesystem integration or physical-router test is claimed.

## Exact first action for the next GPT

1. Fetch live `main`, root governance files and all three archive indexes. Check the **USB storage hotplug stability regression** and **AI handoff documentation contract** against that exact SHA; do not rely on stale or queued runs.
2. Record exact commit/run IDs and PASS/FAIL in a documentation-only follow-up if needed. If a workflow fails, fix the failing source/test first and keep the handoff honest.
3. Continue the global storage caller/concurrency audit: trace disk/partition add/remove event ordering, `file_lock()` identity and scope, `safe_remove_usb_device()`, `safe_remove_stor_device()`, `ejusb/ejata/ejmmc` entry points and overlapping safe-eject requests. Pick one demonstrated defect for the next bounded fix.
4. Keep FULLCONENAT mapping-table namespace isolation as a separate P2 security/stability task. Do not conflate it with the completed notifier per-net ownership change; kernel/MTK/DTS changes remain outside the ordinary user-space repair batch.

## Recovery and archive rule

- The superseded handoff is preserved byte-for-byte as `docs/handoffs/archive/2026/11/part-009.md`; the superseded roadmap is preserved byte-for-byte as `docs/roadmap-history/2026/11/part-010.md`.
- The superseded 08:14 CST NFS record evicted from the root rolling window is appended verbatim to `docs/dev-log/2026/11/part-005.md`.
- Root `dev_log.md` retains exactly the newest 20 complete records, newest first. Historical snapshots/records stay indexed and byte-preserved.
