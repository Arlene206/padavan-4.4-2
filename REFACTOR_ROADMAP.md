# Padavan 4.4 — Whole-Repository Modernization Roadmap

> **Current dashboard only.** Historical snapshots are preserved verbatim in [the roadmap history index](docs/roadmap-history/INDEX.md). A bounded fix does not prove module completion. Re-fetch live `main` before trusting any SHA.

- **Repository:** `Arlene206/padavan-4.4-2`
- **Route implementation commit:** `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **Route workflow:** [42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) SUCCESS on exact code SHA `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **WAN teardown workflow:** [42552840447](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840447) SUCCESS on that exact code SHA.
- **HTTPD contract correction commit:** `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`; corrected full regression [42553153266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153266) SUCCESS and documentation contract [42553153260](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153260) SUCCESS on exact `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`. The earlier HTTPD run 42552840406 failed only because the old contract expected the void DHCP wrapper/branch shape; its C DHCP lease policy test passed, and the updated contract now passes.

## Status semantics

- `NOT INVENTORIED` — module/source ownership not mapped.
- `NOT AUDITED` — inventoried but no meaningful audit completed.
- `AUDITING` — ongoing; bounded findings/fixes do not close the module.
- `PARTIAL / AUDITING` — substantial scoped work exists; important paths remain open.

## Whole-repository module status

| Module / area | Status | Evidence already established | Open coverage / next boundary |
|---|---|---|---|
| WAN / PPP / routing / IPv6 | PARTIAL / AUDITING | PPP callback identity, demand startup failure, service-stop fence, management/physical/USB NDIS teardown and static/DHCP route first-error aggregation have bounded source/host/Actions evidence. | Keep WAN open; review lifecycle callers further only if a concrete downstream state inconsistency is demonstrated. Broader PPP/DHCP/WAN runtime and target behavior remain open. |
| Wireless management / driver / iNIC / WDS | PARTIAL / AUDITING | Radio-state publication, AP/APCLI/WDS bridge-up gates and RT3352 iNIC startup recovery have bounded host/Actions evidence. | Guest-interface-up handling in `check_inic_mii_rebooted()`; interface-down and LED publication in `stop_wifi_all_wl/rt()`; driver/switch consumer coverage. |
| Firewall / NAT / Netfilter / IPv6 | AUDITING | IPv4/IPv6 restore transaction and FULLCONENAT GC/notifier-registration error paths have scoped evidence; parser hardening exists for `xt_webstr`. | FULLCONENAT global notifier/reference-count versus per-net namespace ownership; independently prove IPv4/IPv6 writer/hook parity, rollback completeness and module unload safety. |
| DNS / DHCP / resolver | PARTIAL / AUDITING | DNSMasq staging/restart, lease input normalization, DHCPv6 DUID file and WAN resolver refresh have bounded evidence. | Resolver snapshot transactionality, remaining parser/rebinding controls, DHCP renew/decline/release error handling and runtime integration. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Mount/swap inventories are staged before selected removal; safe-eject and NFS export parsers have bounded tests. | Operation-wide caller coverage, hot-unplug races, filesystem runtime and NFS integration; target/hardware validation. |
| HTTPD / CGI / authentication / sessions | PARTIAL / AUDITING | Auth/route access, same-NAT session ownership and selected CSRF/Origin checks have source-contract coverage. | Whole CGI inventory, request-to-shell/filesystem sinks, logout/session lifecycle, TLS/archive and remaining ACL surfaces. |
| Switch / PHY / MDIO / utilities | PARTIAL / AUDITING | Clause 22/45 CLI parsing and selected ioctl error propagation have host/Actions evidence. | Cross-tree caller inventory, PHY sequencing, target builds and live register failure paths. |
| NVRAM / configuration persistence | PARTIAL / AUDITING | Default restore, erase durability, group compaction and file staging have selected fail-closed contracts. | Whole call graph and power-loss/flash wear/atomicity coverage. |
| Init / service lifecycle / PID 1 | PARTIAL / AUDITING | Bounded service termination, signal deferral/reaping and WAN stop fences have regression contracts. | Process ownership, respawn races, shutdown ordering and all service dependency paths. |
| FTP / Samba / NFS / accounts | PARTIAL / AUDITING | Passdb/share staging, account identity boundaries and export inventory have selected tests. | Whole input surfaces, privilege boundary and runtime filesystem semantics. |
| Shared / third-party userland / kernel integration | NOT INVENTORIED / AUDITING | Selected parser/ABI/lifecycle fixes across integrated components. | Complete package/source inventory; no subsystem closure claim. |

## Current priority queue

1. **P1/P2 — Firewall/NAT lifecycle ownership:** prove or clear FULLCONENAT global notifier/reference-count versus per-net namespace ownership, including registration failure, event delivery and unload cleanup. Independently review IPv4/IPv6 writer and rollback ordering.
2. **P2 — Wireless interface-state residuals:** guest-interface-up result in `check_inic_mii_rebooted()`; interface-down and LED publication in `stop_wifi_all_wl/rt()`.
3. **P2 — WAN route lifecycle follow-up:** route helpers now return/log first errors while continuing all operations; keep WAN PARTIAL / AUDITING and revisit callers only for a demonstrated downstream inconsistency.
4. **P2 — DNS/DHCP:** resolver snapshots, route/option source completeness and DHCP client lifecycle.
5. **P2 — USB/storage:** hotplug concurrency, mount inventory call-graph completeness and filesystem runtime behavior.

## Verification matrix

| Level | Evidence status | Limitation |
|---|---|---|
| L1 static/source review | PASS for recorded bounded changes, including current route result aggregation | Do not extend a finding to an unaudited caller/module. |
| L2 host regression / Actions | Route sanitizer/source contracts PASS on `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; WAN teardown PASS on the same SHA; corrected HTTPD and docs workflows PASS on `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`. | Final documentation-only commit still requires exact-head handoff-contract reconciliation; none of these host checks are production target builds. |
| L3 MIPS/uClibc and complete firmware | NOT VERIFIED | Schedule the integrated build after core source governance is substantially complete. |
| L4 hardware/flash/boot/runtime | NOT VERIFIED | Requires router testing; do not infer from host tests. |

## Governance

- Root handoff/roadmap/log remain compact; historical versions are byte-preserved and indexed under `docs/`.
- Root `dev_log.md` holds exactly the newest 20 complete entries; archive shards are split only between complete dated records and stay below 512 KiB.
- `scripts/check_ai_handoff_docs.py` and the `AI handoff documentation contract` workflow are authoritative for byte counts, Git blob hashes and record counts.
- Re-fetch main and exact-head Actions at every GPT handoff; never infer success from queued or stale results.