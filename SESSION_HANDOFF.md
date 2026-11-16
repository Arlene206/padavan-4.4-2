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

Production-source commit `8a8dfed5441936dc9fcf8e9414f45a21684aff33` adds post-load-verified status reporting for USB/XHCI/EHCI/OHCI, AHCI, MMC/SDHCI, MTK AES and IPSet module groups, retains XHCI's `usb3_disable` parameter, and aggregates first errors into `init_router()`. Exact-source XY-C1 workflow `43823036488`, job `133150286335`, completed SUCCESS including module policy regression, semantic module-group check, Build XY-C1, TRX verification and artifact upload.

The process-lifecycle exact-head run `43825080891`, job `133156882058`, passed all compile/runtime regressions but failed its final RC integration checker at line 1298: it searched for the obsolete bare call `load_mmc_modules();`, while the production caller correctly wraps the call as an argument to `rc_first_service_start_status()`. THIS COMMIT makes the ordering contract compare the actual aggregate-call expressions for MMC/USB/ATA/AES/IPSet against the earlier wireless load. The preceding stale generic-vs-wireless loader contracts were corrected in parent `4bfc47d4aec1548c600039c89a98dc7464e022dc`; this follow-up remains checker-only.

WAN-route is green on parent exact main `f9de514198a1b6cb90b1e12528b87e7d16f955c2` (`43823916390`). The next production audit is legacy `reload_modem_modules()`: inspect module helper semantics and all callers before changing its void interface. Its NDIS and serial module sets include alternative drivers, so individual missing drivers cannot be treated as fatal without a device-aware selection policy.

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

- L3 firmware verification for boot-module source `8a8dfed5441936dc9fcf8e9414f45a21684aff33`: XY-C1 run `43823036488`, job `133150286335`, SUCCESS including module policy regression, source contract, Build XY-C1, TRX verification and artifact upload.
- Exact main `4bfc47d4aec1548c600039c89a98dc7464e022dc`: Process lifecycle `43825080891` FAILED only in final RC integration checker at line 1298 (stale bare call `load_mmc_modules();`); every prior compile/runtime host regression passed. The other exact-head checks AI handoff `43825080774`, AliDDNS `43825080850`, infosvr `43825080775`, AdGuardHome `43825080872`, AliyunDrive `43825080731`, NVRAM `43825080908` completed SUCCESS.
- Earlier exact main `f9de514198a1b6cb90b1e12528b87e7d16f955c2`: WAN-route `43823916390` SUCCESS.
- THIS COMMIT is checker/docs-only; exact-head Process rerun required. No new firmware build is implied; L3 remains attached to `8a8dfed...`. No physical-router L4 validation claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect its exact-head Process inventory/lifecycle Action. Confirm the final shared-library/RC integration checker accepts each aggregate module-group call in the correct order and all prior tests remain green.
2. Once Process is green, continue the P1 `reload_modem_modules()` audit: establish actual supported modem/module combinations, trace all callers and configuration changes, distinguish optional alternative drivers from required module dependencies, and inspect unload/refcount behavior. Do not change the void API until an accurate end-to-end error contract and regression are available.
3. Preserve the immutable backup; keep source, CI, XY-C1 and hardware validation levels separate. Do not repeat the boot-module source investigation or full bootstrap.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
