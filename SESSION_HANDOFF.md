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

The XY-C1 modem build-profile fix is closed at L3 for `7630958f12938ae6014ee859d9eccfd21cdf7503`: run `43827946528`, job `133166106689`, passed the modem driver configuration contract, Build XY-C1, TRX verification and artifact upload. All ten exact-main Actions on that SHA were green.

Current source change `760ecfa9d1ff115e62b2df45afb9fda398229ba6` extends boot health reporting in `init_router()`: both mutually exclusive `start_logger()` paths, `start_detect_link()`, `start_detect_internet()`, `start_dns_dhcpd()`, `start_wins()`, and `start_services_once()` are captured and merged into a first-nonzero `boot_services_status`. Detection/WINS failures are diagnosed while initialization continues. Exact-main Process run `43831778050`, job `133178573988`, failed only at a stale source contract that still expected the old inline declaration `int boot_services_status = boot_modules_status;`; current production code declares `int boot_modules_status, boot_services_status;` and then assigns `boot_services_status = boot_modules_status;`. All preceding Process host tests and source checks passed. THIS COMMIT corrects those stale assertions to match the real declaration/assignment, with no production code changes.

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

- Source/config SHA `7630958f12938ae6014ee859d9eccfd21cdf7503`: XY-C1 run `43827946528`, job `133166106689`, SUCCESS through modem driver config check, Build XY-C1, TRX verification and artifact upload; TRX 12,680,451 bytes, artifact `14207562837` (12,633,711 bytes, SHA-256 `bf5a475c7d2b599cc07a6ce5c3b6a0ed89410c1857f45fb4bc8d462ded0ac9c8`).
- Exact-main `760ecfa9d1ff115e62b2df45afb9fda398229ba6`: Process `43831778050` FAILED at stale checker line 1307 only; all preceding C host regressions/source checks passed. Other relevant exact-main Actions: AI handoff `43831777933`, NVRAM `43831777948`, AliyunDrive `43831778007`, AdGuardHome `43831778084`, HTTPD auth `43831778004`, firewall `43831778056`, Samba `43831778037`, USB storage `43831777985`, NFS `43831777981`, FRP `43831777980`, ZeroTier `43831778248`, detect_link `43831778211`, VPN `43831778190`, DNS forwarder `43831778159`, Adbyby `43831778146`, crond `43831778106`, networkmap `43831778099`, detect_internet `43831777929`, Inadyn `43831777927`, rstats `43831777926`, iNIC `43831777920`, and related subsystem regressions completed SUCCESS. XY-C1 run `43831778029` was IN_PROGRESS when inspected.
- THIS COMMIT is checker/handoff-only. The Process exact-head rerun is required; the already-running XY-C1 build remains evidence for source SHA `760ecfa...` only. No L4 physical-router validation is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and verify exact-head Process Actions for THIS COMMIT; confirm the source integration checker passes and C host regressions remain green. Also recheck the XY-C1 run `43831778029` for the source commit `760ecfa...` and record Build/TRX/artifact steps.
2. After the stale checker is green, continue the WAN startup status gap: `launch_wan_usbnet()` ignores miniupnpd interface-migration and DHCP-client launch results, and `start_wan()`/restart callers discard modem startup status. Trace all callers, then implement a small source/test change that propagates first failure without aborting unrelated best-effort WAN setup.
3. Keep `reload_modem_modules()` void until a device-aware modem driver selection contract exists; do not treat alternative driver load failures as fatal.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
