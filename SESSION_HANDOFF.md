# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST perform an end-of-session handoff checkpoint before ending. The checkpoint is mandatory; a document edit is conditional. Continue the recorded engineering task across conversations: recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and archive shards/index. Before context exhaustion, record precise live refs, exact Action results, and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Main immediately before this log-integrity and CI-contract repair: `7b82bc0826ebf93c66b6ed42811990222f2b98e5`.
- Latest product-source commit: `b2d3288b6282c16cf23859eb9e7f8fdce083cd62` (full LAN restart first-error aggregation). Commits `f66dc26`, `7ac9582`, and `7b82bc0` after it changed workflow/test contracts and handoff records, not firmware production source.
- Frozen archival ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, move, rewrite or delete.
- Bootstrap, development protocol, handoff, roadmap, archive index, branch inventory, backup marker and legacy PR #1 have already been recovered. PR #1 is closed; valid AliDDNS changes are on main. Do not repeat that audit.
- The current root log had been truncated to three records by the LAN restart checkpoint sequence. The parent `2c75de8133751e8063ded244faa225011f2891b6` retained 61 complete records; THIS COMMIT restores those records verbatim after the three newer records, then adds this repair entry. The resulting root window remains within 20–119 records, without touching archived shards or indexes.

## Active task

Production-source commit `8a8dfed5441936dc9fcf8e9414f45a21684aff33` adds post-load-verified status reporting for USB/XHCI/EHCI/OHCI, AHCI, MMC/SDHCI, MTK AES and IPSet module groups, retaining XHCI's `usb3_disable` parameter and aggregating first errors into `init_router()`. Its exact-source XY-C1 workflow completed SUCCESS on run `43823036488`, job `133150286335`: boot module policy regression, semantic module-group check, Build XY-C1, TRX verification and artifact upload all passed.

WAN-route regression is green on exact main `f9de514198a1b6cb90b1e12528b87e7d16f955c2`: run `43823916390` SUCCESS. The process-lifecycle exact-head run `43823916381`, job `133153102957`, failed only at the final shared-library/RC source contract because its checker still required the former wireless-only `rc_wireless_module_load_verified()` implementation and `wireless_modules_status` variable. The production code intentionally generalized policy to `rc_module_load_verified(module_name, module_param, &ops)` and aggregates all optional hardware groups as `boot_modules_status`; earlier host regressions and the generalized module policy test passed. THIS COMMIT updates those stale checker assumptions, explicitly verifies the retained wireless compatibility wrapper and XHCI parameter-forwarding test, and does not change production behavior. The Process workflow already watches this checker path in both push and pull_request filters.

The next engineering item is the legacy `reload_modem_modules()` API. Trace its full implementation, all callers, modem type/rule configuration paths, module availability semantics, and restart/rollback behavior before deciding whether to alter its void return contract. Do not change the prototype without an end-to-end caller contract and regression.

## Files and caller chain reviewed

- trunk/user/rc/src/net.c: start_igmpproxy() / stop_igmpproxy() return meaningful start/stop errors. Start first gates replacement on successful old-process stop, then aggregates udpxy/xupnpd/config publication/igmpproxy launch status. No helper API change was needed.
- trunk/user/rc/src/rc.h: both helper declarations already return int.
- trunk/user/rc/src/net_wan.c: static IPTV callback; man_up_internal, man_down_internal, wan_up_internal, wan_down_internal; DHCP IPTV bound; ZeroConf IPTV bound/deconfig. First-error aggregation leaves route and gateway outputs independent where they feed recovery/retry state.
- trunk/user/rc/src/wan_viptv_static_policy.h: multicast callback now returns status; callback still runs after route-add failure.
- trunk/user/rc/src/tests/wan_viptv_static_policy.c: multicast failure propagation, callback-always-attempted, and route-first-error preservation.
- scripts/check_wan_vlan_setup.py and .github/workflows/wan-route-status.yml: semantic caller contracts, exact push/PR path filters, and GNU89/Werror/ASan/UBSan regression execution.
- No unrelated source modules, network policy, DHCP wire behavior, NVRAM format, or persistent configuration format was changed.

## Compatibility and sequencing notes

- Existing helper int APIs and stop-before-start behavior are unchanged.
- MAN/WAN up/down continue independent cleanup and other route operations after a multicast error; first nonzero status is returned to the int-return caller.
- Static IPTV policy attempts multicast even if its gateway route failed, then returns the first nonzero status.
- DHCP IPTV writes only route setup failure to manv_route_err; proxy failure reaches the DHCP script caller without poisoning route-only renew retry semantics.
- ZeroConf IPTV teardown reports proxy stop failure, but the existing address identity decision remains based on address/route teardown outcomes.

## Verification boundary

- L3 firmware verification for boot-module source `8a8dfed5441936dc9fcf8e9414f45a21684aff33`: XY-C1 run `43823036488`, job `133150286335`, completed SUCCESS including module policy regression, semantic group checker, Build XY-C1, TRX verification and artifact upload.
- Exact main `f9de514198a1b6cb90b1e12528b87e7d16f955c2`: WAN-route `43823916390`, AI handoff `43823916418`, AliDDNS `43823916394`, AliyunDrive `43823916419`, infosvr `43823916437` and AdGuardHome `43823916443` completed SUCCESS. NVRAM `43823916423` was still IN_PROGRESS when checked. Process lifecycle `43823916381` FAILED only in the final RC source-integration checker (job `133153102957`, assertion line 1279); all preceding compile/runtime regressions passed.
- THIS COMMIT is test/source-contract and handoff-only. Exact-head Process rerun is required. No new firmware build is implied; L3 evidence remains attached to source SHA `8a8dfed...`.
- Previous IGMP source `3e7acc5caeb8b8a9d0588edbcc8ab9292b750479` also completed XY-C1 Build/TRX/artifact on run `43820348179`. No L4 physical-router validation is claimed; no local full checkout/host execution was available in this session.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the exact-head Process inventory/lifecycle Action for THIS COMMIT. Confirm the RC integration checker now validates the generic post-load module helper, wireless compatibility wrapper, boot-module aggregate and parameter-forwarding test. Diagnose any remaining assertion against current source semantics rather than weakening unrelated checks.
2. Once Process is green, continue the P1 `reload_modem_modules()` audit: trace implementation, declarations, all callers, module helper semantics, modem type/rule transitions, and failure cleanup. Add a regression and propagate a first error only where the end-to-end contract can be preserved. Make a small atomic source/test commit and verify its exact-SHA Actions.
3. Preserve the immutable backup and distinguish source, CI, XY-C1 and hardware validation. Do not repeat the completed bootstrap or boot-module source audit.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
