# Padavan 4.4 — Whole-Repository Modernization Roadmap

> **Current dashboard only.** Historical snapshots are preserved verbatim in [the roadmap history index](docs/roadmap-history/INDEX.md). A bounded fix does not prove module completion. Re-fetch live `main` before trusting any SHA.

- **Repository:** `Arlene206/padavan-4.4-2`.
- **Base main SHA before this dashboard commit:** `93277fe13c6c53e49fd94d3badfd4a1a4def0241`.
- **Prior exact-head handoff check:** [42553418388](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553418388) SUCCESS on `93277fe13c6c53e49fd94d3badfd4a1a4def0241`.
- **Current round:** per-network-namespace FULLCONENAT notifier lifecycle correction; new exact-head Actions results are PENDING until reconciled.

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
| Firewall / NAT / Netfilter / IPv6 | AUDITING | IPv4/IPv6 restore transaction and FULLCONENAT GC/notifier-registration error paths have scoped evidence. This commit replaces the single global FULLCONENAT notifier/refcount with one notifier/refcount per `struct net`, unregisters from the owning namespace, waits for RCU readers, and checks that iptables cleanup passes the same namespace before module_put. Exact-head workflow is pending. | Keep module open. FULLCONENAT mapping hashes and dying-tuple list remain module-global and are not netns-keyed; independently verify isolation, IPv4/IPv6 writer/hook parity, rollback completeness, unload safety and runtime behavior. |
| DNS / DHCP / resolver | PARTIAL / AUDITING | DNSMasq staging/restart, lease input normalization, DHCPv6 DUID file and WAN resolver refresh have bounded evidence. | Resolver snapshot transactionality, remaining parser/rebinding controls, DHCP renew/decline/release error handling and runtime integration. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Mount/swap inventories are staged before selected removal; safe-eject and NFS export parsers have bounded tests. | Operation-wide caller coverage, hot-unplug races, filesystem runtime and NFS integration; target/hardware validation. |
| HTTPD / CGI / authentication / sessions | PARTIAL / AUDITING | Auth/route access, same-NAT session ownership and selected CSRF/Origin checks have source-contract coverage. | Whole CGI inventory, request-to-shell/filesystem sinks, logout/session lifecycle, TLS/archive and remaining ACL surfaces. |
| Switch / PHY / MDIO / utilities | PARTIAL / AUDITING | Clause 22/45 CLI parsing and selected ioctl error propagation have host/Actions evidence. | Cross-tree caller inventory, PHY sequencing, target builds and live register failure paths. |
| NVRAM / configuration persistence | PARTIAL / AUDITING | Default restore, erase durability, group compaction and file staging have selected fail-closed contracts. | Whole call graph and power-loss/flash wear/atomicity coverage. |
| Init / service lifecycle / PID 1 | PARTIAL / AUDITING | Bounded service termination, signal deferral/reaping and WAN stop fences have regression contracts. | Process ownership, respawn races, shutdown ordering and all service dependency paths. |
| FTP / Samba / NFS / accounts | PARTIAL / AUDITING | Passdb/share staging, account identity boundaries and export inventory have selected tests. | Whole input surfaces, privilege boundary and runtime filesystem semantics. |
| Shared / third-party userland / kernel integration | NOT INVENTORIED / AUDITING | Selected parser/ABI/lifecycle fixes across integrated components. | Complete package/source inventory; no subsystem closure claim. |

## Current priority queue

1. **P2 — Verify the FULLCONENAT per-net notifier correction:** require the lifecycle regression and AI handoff contract workflows to pass on the same live `main` SHA; fix any failure before new work.
2. **P2 — Wireless interface-state residuals:** `check_inic_mii_rebooted()` guest-interface-up handling, then `stop_wifi_all_wl/rt()` interface-down and LED publication.
3. **P2 — FULLCONENAT mapping namespace isolation:** prove whether the module-global IPv4/IPv6 mapping hashes and dying-tuple list can cross-match identical tuples in separate namespaces; scope mappings by netns only if the call graph confirms a concrete defect.
4. **P2 — DNS/DHCP:** resolver snapshot transactionality, DHCP client lifecycle and parser/rebinding controls.
5. **P2 — USB/storage:** hotplug concurrency, mount inventory call-graph completeness and filesystem runtime behavior.

## Verification matrix

| Level | Evidence status | Limitation |
|---|---|---|
| L1 static/source review | PASS for the selected FULLCONENAT notifier registration/unwind, per-net teardown and RCU lifetime paths; earlier bounded changes remain scoped to their reviewed paths. | A bounded source review does not close the module or prove namespace isolation of mapping state. |
| L2 host regression / Actions | New FULLCONENAT lifecycle source-contract workflow and handoff documentation contract are PENDING on THIS COMMIT. | Workflow source contracts are not kernel compilation or a live conntrack/NAT test. |
| L3 MIPS/uClibc and complete firmware | NOT VERIFIED | Schedule the integrated build after core source governance is substantially complete. |
| L4 hardware/flash/boot/runtime | NOT VERIFIED | Requires router testing; do not infer from host or source-contract checks. |

## Governance

- Root handoff/roadmap/log remain compact; historical versions are byte-preserved and indexed under `docs/`.
- Root `dev_log.md` holds exactly the newest 20 complete entries, newest first; archive shards are split only between complete dated records and stay below 512 KiB.
- `scripts/check_ai_handoff_docs.py` and the `AI handoff documentation contract` workflow are authoritative for byte counts, Git blob hashes and record counts.
- Re-fetch main and exact-head Actions at every GPT handoff; never infer success from queued or stale results.
