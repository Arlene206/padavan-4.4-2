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

Latest main source commit `8a8dfed5441936dc9fcf8e9414f45a21684aff33` contains verified status aggregation for USB/XHCI/EHCI/OHCI, AHCI, MMC/SDHCI, MTK AES and IPSet boot module groups, including preservation of XHCI parameters. Its XY-C1 build, TRX and artifact validation remain in progress at last inspection (run `43823036488`, Build XY-C1 step still running).

IGMP proxy lifecycle source commit `3e7acc5caeb8b8a9d0588edbcc8ab9292b750479` passed XY-C1 Build/TRX/artifact. Static IPTV regression passed. WAN-route exact-SHA checker chain still fails only in older exact operation-count assertions: after fixing MAN teardown count, the next failure was its stale MAN up exact accumulation count; WAN up has the same stale count (+1 from the correctly accumulated proxy result). This follow-up makes the historical operation counts lower bounds, while the independent semantic checker already requires explicit start/stop proxy status capture and aggregation.

Next: check exact-head WAN-route after this test-only patch, and check XY-C1 for boot module commit `8a8dfed...`. Then continue tracing legacy `reload_modem_modules()` implementation/callers before modifying any of its void API.

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

- Previous source L3: b2d3288b6282c16cf23859eb9e7f8fdce083cd62, XY-C1 run 43814395678 completed Build, TRX verification and artifact upload successfully. This evidence predates the current IGMP source change.
- Previous live main d552d85d79196f95d88d22ea2b29f45061cda4f2: AI handoff 43816938416, DNSMasq 43816938423, Process lifecycle 43816938428, NVRAM 43816938384, AdGuardHome 43816938365, infosvr 43816938338, AliyunDrive 43816938443, and AliDDNS 43816938506 all completed SUCCESS on that exact SHA. NVRAM run 43815228491 also completed SUCCESS.
- The earlier DNSMasq YAML indentation failure 43816311414 on 78af5220da96994440003d1e794ea948457dbd5f was fixed; d552 exact-head DNSMasq is PASS.
- THIS COMMIT: L1 source review is complete. Static IPTV host regression and semantic caller checks are wired into the WAN-route workflow but their exact-head Actions result is not yet known until publication. Exact-source XY-C1 build/TRX/artifact result is also pending after publication.
- No local full repository checkout or local host test execution was available during this slice. No L4 physical-router validation is claimed.

## Exact first action for the next GPT

1. Fetch live main, then inspect exact-head WAN route operation status regression and XY-C1 Firmware Validation push runs for THIS COMMIT. Confirm static IPTV callback test and MAN/WAN/DHCP/ZeroConf caller contracts actually ran. If either fails, inspect the first causal job/log and repair it with a new atomic main commit; do not count older-SHA runs.
2. Continue P1 RC service startup-result audit outside WAN/IPTV: trace remaining enabled daemon/module launch helpers and their notification/startup callers for discarded nonzero results. Start with trunk/user/rc/src/services_ex.c plus declarations/callers; add a production-linked regression for a confirmed defect rather than changing void APIs without an end-to-end caller.
3. Preserve the frozen backup and keep L3/L4 validation boundary explicit.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory testing. Use only `main`; do not touch the frozen backup. Update root `dev_log.md` for each code/documentation commit; keep it within 20–119 records and the size limits; preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, published history rewrite, or unrun-test/build/hardware claim.
