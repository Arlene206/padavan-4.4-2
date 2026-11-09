# Padavan 4.4 — Whole-Repository Modernization Roadmap

> **Current dashboard only.** Historical snapshots are preserved verbatim in [the roadmap history index](docs/roadmap-history/INDEX.md). A bounded fix does not prove module completion. Re-fetch live `main` before trusting any SHA.

- **Repository:** `Arlene206/padavan-4.4-2`.
- **Base main SHA before this documentation update:** `91cae3d19bb72a9e0cebcad509db256aded52467`.
- **Completed code change:** `91cae3d19bb72a9e0cebcad509db256aded52467` — FULLCONENAT notifier state is now per network namespace.
- **Exact-head evidence:** FULLCONENAT lifecycle regression [42555493213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555493213) and AI handoff contract [42555491302](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555491302) both SUCCESS on `91cae3d19bb72a9e0cebcad509db256aded52467`. A documentation-only update now triggers a fresh handoff check.

## Status semantics

- `NOT INVENTORIED` — module/source ownership not mapped.
- `NOT AUDITED` — inventoried but no meaningful audit completed.
- `AUDITING` — ongoing; bounded findings/fixes do not close the module.
- `PARTIAL / AUDITING` — substantial scoped work exists; important paths remain open.

## Whole-repository module status

| Module / area | Status | Evidence already established | Open coverage / next boundary |
|---|---|---|---|
| WAN / PPP / routing / IPv6 | PARTIAL / AUDITING | PPP callback identity, demand startup failure, service-stop fence, management/physical/USB NDIS teardown and static/DHCP route first-error aggregation have bounded source/host/Actions evidence. | Keep WAN open; broader PPP/DHCP/WAN runtime, route recovery semantics and target behavior remain open. |
| Wireless management / driver / iNIC / WDS | PARTIAL / AUDITING | Radio-state publication, AP/APCLI/WDS bridge-up gates and RT3352 iNIC startup recovery have bounded host/Actions evidence. | Next: guest-interface-up result in `check_inic_mii_rebooted()`; interface-down and LED publication in `stop_wifi_all_wl/rt()`; driver/switch consumer coverage. |
| Firewall / NAT / Netfilter / IPv6 | AUDITING | FULLCONENAT's former module-global notifier/reference count is now per-`struct net`; exact source-contract run 42555493213 passed on code SHA `91cae3d19bb72a9e0cebcad509db256aded52467`. The contract checks correct-owner unregister, RCU release and IPv4/IPv6 rule cleanup before module_put. | Keep module open. FULLCONENAT mapping hashes and dying-tuple list remain module-global and are not netns-keyed; verify mapping isolation, IPv4/IPv6 writer/hook parity, rollback, unload safety and runtime behavior. |
| DNS / DHCP / resolver | PARTIAL / AUDITING | DNSMasq staging/restart, lease input normalization, DHCPv6 DUID file and WAN resolver refresh have bounded evidence. | Resolver snapshot transactionality, remaining parser/rebinding controls, DHCP renew/decline/release error handling and runtime integration. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Mount/swap inventories are staged before selected removal; safe-eject and NFS export parsers have bounded tests. | Operation-wide caller coverage, hot-unplug races, filesystem runtime and NFS integration; target/hardware validation. |
| HTTPD / CGI / authentication / sessions | PARTIAL / AUDITING | Auth/route access, same-NAT session ownership and selected CSRF/Origin checks have source-contract coverage. | Whole CGI inventory, request-to-shell/filesystem sinks, logout/session lifecycle, TLS/archive and remaining ACL surfaces. |
| Switch / PHY / MDIO / utilities | PARTIAL / AUDITING | Clause 22/45 CLI parsing and selected ioctl error propagation have host/Actions evidence. | Cross-tree caller inventory, PHY sequencing, target builds and live register failure paths. |
| NVRAM / configuration persistence | PARTIAL / AUDITING | Default restore, erase durability, group compaction and file staging have selected fail-closed contracts. | Whole call graph and power-loss/flash wear/atomicity coverage. |
| Init / service lifecycle / PID 1 | PARTIAL / AUDITING | Bounded service termination, signal deferral/reaping and WAN stop fences have regression contracts. | Process ownership, respawn races, shutdown ordering and all service dependency paths. |
| FTP / Samba / NFS / accounts | PARTIAL / AUDITING | Passdb/share staging, account identity boundaries and export inventory have selected tests. | Whole input surfaces, privilege boundary and runtime filesystem semantics. |
| Shared / third-party userland / kernel integration | NOT INVENTORIED / AUDITING | Selected parser/ABI/lifecycle fixes across integrated components. | Complete package/source inventory; no subsystem closure claim. |

## Current priority queue

1. **P2 — Verify this documentation-only handoff update:** require the new AI handoff documentation contract run to pass on the resulting live `main` SHA; repair any archive/index mismatch before new coding.
2. **P2 — Wireless interface-state residuals:** `check_inic_mii_rebooted()` guest-interface-up handling, then `stop_wifi_all_wl/rt()` interface-down and LED publication.
3. **P2 — FULLCONENAT mapping namespace isolation:** prove whether the module-global IPv4/IPv6 mapping hashes and dying-tuple list can cross-match identical tuples in separate namespaces; scope mappings by netns only if the call graph confirms a concrete defect.
4. **P2 — DNS/DHCP:** resolver snapshot transactionality, DHCP client lifecycle and parser/rebinding controls.
5. **P2 — USB/storage:** hotplug concurrency, mount inventory call-graph completeness and filesystem runtime behavior.

## Verification matrix

| Level | Evidence status | Limitation |
|---|---|---|
| L1 static/source review | PASS for the selected FULLCONENAT notifier ownership, registration/unwind, teardown and RCU lifetime paths. | Does not prove FULLCONENAT mapping-table namespace isolation or close Firewall/NAT. |
| L2 host regression / Actions | FULLCONENAT lifecycle contract and handoff docs/archive checker both PASS on exact code SHA `91cae3d19bb72a9e0cebcad509db256aded52467`; a fresh docs-only handoff check is pending on the resulting main SHA. | The source contract is not kernel compilation or a live conntrack/NAT test. |
| L3 MIPS/uClibc and complete firmware | NOT VERIFIED | Schedule the integrated build after core source governance is substantially complete. |
| L4 hardware/flash/boot/runtime | NOT VERIFIED | Requires router testing; do not infer from host or source-contract checks. |

## Governance

- Root handoff/roadmap/log remain compact; historical versions are byte-preserved and indexed under `docs/`.
- Root `dev_log.md` holds exactly the newest 20 complete entries, newest first; archive shards are split only between complete dated records and stay below 512 KiB.
- `scripts/check_ai_handoff_docs.py` and the `AI handoff documentation contract` workflow are authoritative for byte counts, Git blob hashes and record counts.
- Re-fetch main and exact-head Actions at every GPT handoff; never infer success from queued or stale results.
