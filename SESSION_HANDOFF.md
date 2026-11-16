# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST perform an end-of-session handoff checkpoint before ending. The checkpoint is mandatory; a document edit is conditional. Continue the recorded engineering task across conversations: recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and archive shards/index. Before context exhaustion, record precise live refs, exact Action results, and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent `main` for THIS COMMIT: `7630958f12938ae6014ee859d9eccfd21cdf7503`. Fetch the live ref again on every subsequent takeover and immediately before any write.
- Latest target firmware-source baseline: `7630958f12938ae6014ee859d9eccfd21cdf7503`; exact-head XY-C1 Build, TRX verification and artifact upload all PASS in run `43827946528` (artifact `14207562837`, 12,633,711 bytes, SHA-256 `bf5a475c7d2b599cc07a6ce5c3b6a0ed89410c1857f45fb4bc8d462ded0ac9c8`).
- Frozen archival ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Bootstrap, protocol, root handoff/roadmap/log governance, archive index, backup marker and legacy PR #1 have been recovered. PR #1 is closed; do not repeat that audit.
- Root `dev_log.md` currently contains 76 complete newest-first records (before THIS COMMIT); this entry brings it to 77, so no rotation is required.

## Active task

All ten exact-head push Actions on parent-main SHA `7630958f12938ae6014ee859d9eccfd21cdf7503` completed SUCCESS: Process inventory `43827946437`, FULLCONENAT `43827946431`, SkipDBv2 `43827946584`, NVRAM `43827946578`, infosvr `43827946518`, AliDDNS `43827946574`, AI handoff `43827946538`, AdGuardHome `43827946510`, AliyunDrive `43827946444`, and XY-C1 Firmware Validation `43827946528`.

The P1 USB modem build-profile mismatch is closed at L3 for source SHA `7630958f12938ae6014ee859d9eccfd21cdf7503`: exact-head run `43827946528` passed the new module-configuration contract, Build XY-C1, TRX verification and artifact upload. The TRX is 12,680,451 bytes; artifact `14207562837` is 12,633,711 bytes (SHA-256 `bf5a475c7d2b599cc07a6ce5c3b6a0ed89410c1857f45fb4bc8d462ded0ac9c8`). This is L3 build evidence only, not L4 device validation.

The current source task is boot-status propagation in `init_router()`. Source review found both mutually exclusive `start_logger()` paths, `start_detect_link()`, `start_detect_internet()`, `start_wins()`, and `start_dns_dhcpd()` either discarded or not merged into the existing first-error `boot_services_status`. THIS COMMIT captures and merges these results, adds detector/WINS failure logs, and extends `scripts/check_rc_boot_module_loads.py` to guard the source wiring. The existing GNU89/Werror/ASan/UBSan `service_start_status.c` test already checks first-error and signed/positive status preservation and is run by the XY-C1 workflow. Exact-head validation for THIS COMMIT is pending after publication.

Earlier USB modem source fix (commit `7630958f12938ae6014ee859d9eccfd21cdf7503`) enabled `CONFIG_USB_SERIAL_OPTION=m`, because `reload_modem_modules()` requests `option` for RAS modems while XY-C1 previously disabled the driver. The kernel Kconfig describes it as the GSM/CDMA USB serial driver for Option, Huawei, Audiovox, Novatel Wireless and Anydata devices; the serial Makefile maps the symbol to `option.o`. The exact-head run `43827946528` passed the module configuration contract, Build XY-C1, TRX verification and artifact upload (artifact `14207562837`, 12,633,711 bytes). The current source task does not change modem code. Keep the legacy void return API unchanged until a device-aware contract is proven: `module_smart_load()` returns zero both for an already-loaded module and for failure; `module_smart_unload()` returns one when refcount is positive (busy, not unloaded), as well as when the unload command succeeds. Missing modules/refcount-read errors and unload failures all return zero. The driver lists are intentionally broad alternatives, so any propagated failure must be based on actual device/module selection rather than treating each individual load failure as fatal.

## Files and caller chain reviewed

- `trunk/user/rc/src/usb_modem.c`: `reload_modem_modules()`, `unload_modem_modules()`, the RAS/NDIS readiness checks, interface selection and USB modem WAN launch/removal paths.
- `trunk/user/rc/src/rc.c`: boot-time call from `load_usb_modules()`; `RCN_RESTART_MODEM` first stops active WAN before changing modem modules, then updates cached modem type/rule and restarts WAN. THIS COMMIT also extends `init_router()` first-error aggregation to logger, detector, DNSMasq, WINS and `start_services_once()` outcomes.
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

- Parent source SHA `7630958f12938ae6014ee859d9eccfd21cdf7503`: all ten exact-head push Actions listed above completed SUCCESS. XY-C1 run `43827946528` passed the option driver config contract, all staged host regressions, Build XY-C1, TRX verification and artifact upload (artifact `14207562837`, 12,633,711 bytes; digest `bf5a475c7d2b599cc07a6ce5c3b6a0ed89410c1857f45fb4bc8d462ded0ac9c8`).
- THIS COMMIT contains new `rc.c` source behavior plus an extended semantic checker. No local checkout/host execution was available. Exact-head XY-C1 and Process runs are pending after publication; the successful 763 firmware result does not verify this new `rc.c` change.
- L3 firmware and artifact evidence is not L4 physical-router evidence. No USB modem or general router hardware runtime test is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect its exact-head XY-C1 Firmware Validation and Process regression push runs. Confirm the service first-error policy test, extended `init_router()` source contract, firmware C build, TRX verification and artifact upload pass; diagnose any failure at its first causal step.
2. After this commit is verified, resume the deferred device-aware `reload_modem_modules()` analysis: identify supported device-to-driver mappings/readiness postconditions and add a regression before changing the legacy void API.
3. Preserve the frozen backup; distinguish host/source checks, exact-SHA CI, target build and physical hardware validation. Do not repeat the Process checker, modem profile discovery or this `init_router()` status aggregation audit.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
