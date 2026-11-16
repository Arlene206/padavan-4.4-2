# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST perform an end-of-session handoff checkpoint before ending. The checkpoint is mandatory; a document edit is conditional. Continue the recorded engineering task across conversations: recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and archive shards/index. Before context exhaustion, record precise live refs, exact Action results, and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent `main` for THIS COMMIT: `3905bc882d54ee5a37f5dd0493742d9a752565ff`. Fetch the live ref again on every subsequent takeover and immediately before any write.
- Latest recent RC firmware-source baseline: `8a8dfed5441936dc9fcf8e9414f45a21684aff33` (verified post-load status propagation for boot hardware modules). This slice changes the XY-C1 kernel module configuration and build-time validation; it does not change C runtime behavior.
- Frozen archival ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Bootstrap, protocol, root handoff/roadmap/log governance, archive index, backup marker and legacy PR #1 have been recovered. PR #1 is closed; do not repeat that audit.
- Root `dev_log.md` currently contains 75 complete newest-first records (before THIS COMMIT); no rotation is required.

## Active task

The previous Process integration-checker blocker is closed. Exact-main Process run `43826215394`, job `133160569142`, completed SUCCESS on parent `3905bc882d54ee5a37f5dd0493742d9a752565ff`. The exact-main AI handoff, AdGuardHome, AliDDNS, infosvr, AliyunDrive and NVRAM Actions also completed SUCCESS on that SHA.

The continuing P1 USB modem audit found a concrete XY-C1 build-profile mismatch:

- `trunk/user/rc/src/usb_modem.c::reload_modem_modules()` attempts to load `option` in its serial/RAS mode, along with `cdc_acm`, `qcserial`, and `sierra`. The NDIS branch likewise requests seven WWAN network drivers.
- The XY-C1 kernel configuration explicitly had `# CONFIG_USB_SERIAL_OPTION is not set`, so `modprobe option` could never succeed on this target even though the RC loader requested it. The kernel Kconfig describes this as the GSM/CDMA USB serial driver for Option, Huawei, Audiovox, Novatel Wireless and Anydata devices; the serial Makefile maps the symbol to `option.o`.
- THIS COMMIT enables `CONFIG_USB_SERIAL_OPTION=m`, adds `scripts/check_usb_modem_driver_config.py` to require every module requested by `reload_modem_modules()` to be built as a module in the XY-C1 profile, and wires the check and relevant path filters into the XY-C1 firmware workflow. The exact-SHA firmware build is required to validate kernel compilation, packaging, TRX and artifact upload.
- Do not yet change the legacy void return API. `module_smart_load()` returns zero both for an already-loaded module and for failure; `module_smart_unload()` returns one when refcount is positive (busy, not unloaded), as well as when the unload command succeeds. Missing modules/refcount-read errors and unload failures all return zero. The driver lists are intentionally broad alternatives, so any propagated failure must be based on actual device/module selection rather than treating each individual load failure as fatal.

## Files and caller chain reviewed

- `trunk/user/rc/src/usb_modem.c`: `reload_modem_modules()`, `unload_modem_modules()`, the RAS/NDIS readiness checks, interface selection and USB modem WAN launch/removal paths.
- `trunk/user/rc/src/rc.c`: boot-time call from `load_usb_modules()`; `RCN_RESTART_MODEM` first stops active WAN before changing modem modules, then updates cached modem type/rule and restarts WAN.
- `trunk/user/rc/src/common_ex.c`: actual return semantics for `module_smart_load()`, `module_smart_unload()`, and `get_module_refcount()`.
- `trunk/user/rc/src/rc.h`: legacy public declarations are `void unload_modem_modules(void)` and `void reload_modem_modules(int modem_type, int reload)`.
- `trunk/configs/boards/XY-C1/kernel-4.4.x.config`, Linux USB serial Kconfig/Makefile and `option.c`: confirmed the missing XY-C1 driver and its build wiring.
- Other current `module_smart_unload()` callers in `hotplug_usb.c`, `net.c`, and `services_stor.c` ignore its result; no cross-repository return-contract change is included.

## Compatibility and sequencing notes

- Keep the existing NDIS/RAS mode split and broad driver coverage; do not interpret an absent/unmatched alternative driver as a global failure.
- `CONFIG_USB_SERIAL_OPTION=m` enables an existing Linux 4.4 driver already requested by the RC loader; it does not change driver source, USB IDs, modem protocol, NVRAM schema, or WAN sequencing.
- The `RCN_RESTART_MODEM` caller continues to gate configuration changes on successful WAN teardown. Any future reload failure propagation must preserve that stop-before-reload/restart order and avoid caching a new modem type when a required selected driver failed.
- No new device-specific attach/detach or live-router behavior has been verified.

## Verification boundary

- Process lifecycle regression: exact parent-main SHA `3905bc882d54ee5a37f5dd0493742d9a752565ff`, run `43826215394`, job `133160569142`, SUCCESS.
- On the same parent SHA, AI handoff `43826215471`, AdGuardHome `43826215583`, AliDDNS `43826215454`, infosvr `43826215459`, AliyunDrive `43826215453` and NVRAM `43826215414` all completed SUCCESS.
- Previous boot-module firmware source `8a8dfed...` completed XY-C1 Build, TRX verification and artifact upload in run `43823036488`; its 12,658,634-byte TRX predates this option-driver configuration change.
- THIS COMMIT has no local checkout/host execution. Exact-head XY-C1 validation is pending after publication; do not claim the new kernel module has built or reached the image until that run passes. No L4 physical-router validation is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the exact-head XY-C1 Firmware Validation push run for THIS COMMIT. Confirm the new static config/loader contract, MIPS kernel module build, final TRX verification and artifact upload all pass; diagnose any failure against the actual build output.
2. If the target build passes, continue the P1 `reload_modem_modules()` end-to-end audit: determine supported device-to-driver combinations and readiness postconditions, then design a device-aware error contract and regression before changing the void API.
3. Preserve the frozen backup; distinguish host/source checks, exact-SHA CI, target build and physical hardware validation. Do not repeat the Process checker or boot-module source audit.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
