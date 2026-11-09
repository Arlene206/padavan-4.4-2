# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved verbatim and indexed. Always fetch live `main` and exact-head Actions before relying on a SHA.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2`, the maintained fork.
- **Latest source milestone:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a` — operation-wide partition inventory preflight for ejected-device cleanup and PID 1 error propagation.
- **Latest CI/contract synchronization:** `82096fbab795598bc6eb4384003e8f66998d1997` — both legacy HTTPD eject contracts migrated to the new snapshot design; handoff/roadmap archive byte-count metadata corrected.
- **Branch/checkpoints:** `main`; no separate remote checkpoint branch exists.
- **Exact-head evidence:** USB storage regression [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) **SUCCESS** on `7fee5c1`; HTTPD Authentication and Security Regression [42562165008](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165008) **SUCCESS** on `82096fb`; AI handoff documentation contract [42562165077](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165077) **SUCCESS** on `82096fb`. The HTTPD job completed all 39 validation steps. Earlier failed attempts (42561802353 / 42561802333) were caused by obsolete source assertions and incorrect index byte counts and are superseded by the passing runs.

## Active task

**Storage eject inventory consistency and notification failure propagation — implemented; focused and integrated host/source regressions PASS. Continue storage caller/concurrency audit.**

Caller/lifecycle chain:
- `mdev_sd_main()` / `mdev_mmc_main()` signal `on_unplug_mass_storage` for whole-disk removal.
- PID 1 `handle_notifications()` invokes `umount_ejected()`.
- Before the fix, `umount_ejected()` staged all `/proc/mounts` rows but reopened `/proc/partitions` for each mount. A later inventory read/parse/close failure could occur after earlier mountpoints had already been unmounted. PID 1 also discarded the cleanup result and could mark failed cleanup as complete.

Implementation at `7fee5c1fc589dc98c3333dd4e052d28881b8756a`:
1. `storage_ejected_parse_partition_line()` validates complete newline-terminated `/proc/partitions` heading/data rows, numeric major/minor/block fields, bounded device token and exact field count.
2. `storage_stage_ejected_partitions()` stages and validates one complete partition inventory, including read/parse/close/allocation failures, before any ejected unmount. The unmount loop consults staged names and does not rescan procfs per mount.
3. PID 1 now assigns `action_result = umount_ejected()`; a failed cleanup retains the incomplete notification marker. The shared failure log is no longer misleadingly Wireless-specific.
4. Seven parser cases and `.github/workflows/storage-hotplug-stability.yml` compile/run and source-order contract passed on exact source head `7fee5c1` (run [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425)).
5. The two existing storage assertions in `.github/workflows/httpd-security.yml` were migrated. The full HTTPD/security regression, including both storage contracts, passed on exact head `82096fb` (run [42562165008](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165008)). The AI documentation validator also passed on that head (run [42562165077](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165077)).

## Other open repository work

- **USB / storage / hotplug — PARTIAL / AUDITING:** per-mount partition-rescan inconsistency and discarded PID 1 cleanup result are fixed. Caller-wide serialization, add/remove interleaving, per-device lock identity, filesystem runtime and hardware remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT notifier ownership is per-network-namespace, but mapping hashes and the dying-tuple queue remain module-global and not namespace-keyed. Mapping isolation, writer parity, rollback and runtime remain open.
- **DNS / DHCP / resolver — PARTIAL / AUDITING:** WAN/IPTV lease validation, route reconciliation, resolver refresh and DHCPv6 DUID handling have bounded evidence; LAN DHCP and remaining lifecycle/error propagation remain open.
- **WAN / PPP / routing — PARTIAL / AUDITING:** selected route-result propagation and WAN stop/teardown failure gates have bounded evidence; broader PPP and runtime routing remain open.
- **Wireless — PARTIAL / AUDITING:** radio-state, bridge-up and RT3352 iNIC lifecycle paths have bounded evidence; other board paths and concurrent restart/stop remain open.
- **HTTPD / CGI / authentication, NVRAM persistence, init/service lifecycle, switch/PHY/MDIO, FTP/Samba/NFS/accounts and integrated third-party userland:** remain separate open areas; no subsystem is declared complete.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/caller review | PASS for whole-disk remove → PID 1 notification → staged eject cleanup and error propagation. |
| L2 host regression / Actions | Storage hotplug GNU89 compile/run and source contract **PASS** [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) on `7fee5c1`; complete HTTPD/security regression **PASS** [42562165008](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165008) and AI docs contract **PASS** [42562165077](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165077), both on `82096fb`. This final docs-only checkpoint will also trigger the docs validator on its own new SHA. |
| L3 MIPS/uClibc RC compile and complete firmware/TRX | **NOT VERIFIED**. |
| L4 USB unplug/eject, filesystem runtime, flash/boot and physical router behavior | **NOT VERIFIED**. |

## Exact first action for the next GPT

1. Fetch live `main` and verify the AI handoff documentation contract on that exact final documentation SHA. Record the run ID and conclusion; do not infer it from the passing parent run.
2. Continue the storage caller/concurrency audit: trace disk/partition add/remove ordering, `safe_remove_usb_device()`, `safe_remove_stor_device()`, `ejusb/ejata/ejmmc` and `file_lock()` identity/scope. Prove a concrete race or stale-state defect before choosing the next bounded fix.
3. Keep FULLCONENAT mapping-table namespace isolation as a separate P2 stability/security task; do not conflate it with the fixed notifier per-net ownership. Kernel/MTK/DTS work remains outside the ordinary userspace batch.

## Recovery and archive rule

- Superseded active handoff is preserved byte-for-byte as `docs/handoffs/archive/2026/11/part-012.md`; superseded roadmap as `docs/roadmap-history/2026/11/part-013.md`.
- The 2026-11-09 09:04 CST wireless radio-state record evicted from the root rolling log is appended verbatim to `docs/dev-log/2026/11/part-005.md`.
- Root `dev_log.md` retains exactly the newest 20 complete entries. All archive/index byte counts and Git blob SHAs remain authoritative.
