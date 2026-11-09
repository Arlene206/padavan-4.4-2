# Padavan 4.4 — Whole-Repository Modernization Roadmap

> **Current dashboard only.** Historical roadmap snapshots are preserved verbatim in [the roadmap history index](docs/roadmap-history/INDEX.md). A bounded fix does not prove module completion. Re-fetch live `main` before trusting any SHA.

- **Repository:** `Arlene206/padavan-4.4-2`
- **Baseline for this round:** `347ab8b0392a3c74d85a230e199c2faae27f646c`.
- **Latest bounded code change:** THIS COMMIT — static/DHCP route result aggregation and propagation.
- **Last verified prior code SHA:** `8a02953fb60cced248254df86feca85e8bc42d10`; WAN run 42548284186 and HTTPD/service run 42548284184 succeeded on that exact code SHA.
- **Pre-change governance HEAD:** `347ab8b0392a3c74d85a230e199c2faae27f646c`; documentation-contract run 42550725545 SUCCESS. New route workflow result is PENDING until exact-head Actions is fetched.

## Status semantics

- `NOT INVENTORIED` — module/source ownership not mapped.
- `NOT AUDITED` — inventoried but no meaningful audit completed.
- `AUDITING` — ongoing; bounded findings/fixes do not close the module.
- `PARTIAL / AUDITING` — substantial scoped work exists; important paths remain open.
- `AUDITED — FINDINGS` / `AUDITED — NO FINDINGS` — defined scope reviewed; scope must be named.
- `FIXED — NOT VERIFIED`, `HOST VERIFIED`, `TARGET BUILD VERIFIED`, `HARDWARE VERIFIED` — evidence for the named change/scope only.

## Whole-repository module status

| Module / scope | Current status | Evidence / completed bounded work | Next audit action |
|---|---|---|---|
| HTTPD / HTTPS / authentication / sessions | AUDITING | Basic-auth parsing, selected privileged CGI/ASP gates, upload/archive handling and duplicate MTK/RTL switch status backend have scoped source/host/Actions evidence. | CSRF, Origin/Referer, logout, ACL, same-NAT session ownership, TLS/archive behavior and request-to-shell/filesystem sinks. |
| CGI / ASP / templates / command dispatch | AUDITING | apply.cgi/update.cgi and selected direct storage mutation routes/callbacks have POST/session gates and source contracts. | Remaining callback arguments, output encoding and eval/_eval/doSystem/file sinks; confirm every mutation path. |
| WAN / PPP / routing / IPv6 | PARTIAL / AUDITING | PPP callback identity, demand startup failure, service-stop fence, management/physical/USB NDIS teardown and static/DHCP route first-error aggregation have bounded source/host-contract evidence. | Keep WAN open; review lifecycle callers further only if a concrete downstream state inconsistency is demonstrated. Continue the broader PPP/DHCP/WAN runtime audit after rotating to Firewall/NAT. |
| DNS / DHCP / resolver | PARTIAL / AUDITING | Resolver caller inventory and DNSMasq reload status have bounded exact-head evidence; DNS config publication has staged/atomic work. | Upstream snapshot transactionality, parser/rebinding, reload/recovery and live service behavior. |
| Firewall / NAT / QoS / VPN | AUDITING | Core restore rollback, VPN lease locking, Adbyby staging, FULLCONENAT GC readiness, xt_webstr parsing and schedule guards have scoped regressions. | Independent IPv4/IPv6 writers, rollback/hook ordering, IPv6 parity and FULLCONENAT namespace/notifier ownership. |
| Wireless management / MTK / AP/APCLI/WDS | AUDITING | Ralink config/restart, ioctl, scan bounds, radio state, AP/APCLI/WDS bridge publication and RT3352 iNIC startup have bounded host/Actions evidence. | `check_inic_mii_rebooted()` guest-up and `stop_wifi_all_wl/rt()` down-result/LED gates; supplicant/driver callers. |
| Switch / PHY / MDIO / register utilities | AUDITING | Switch CLI argument/failure reporting and Clause 22/45 parsing/MDIO timeout paths have scoped host tests and exact-head Actions. | Cross-tree status API consumers and remaining utility/register partial-sequence/timeout propagation. |
| NVRAM / configuration persistence / restore | AUDITING | Profile I/O, dirty-state commit durability, startup/reset gates and all-column group deletion have scoped regression evidence. | Low-level close errors, legacy read ambiguity, restore transactionality and atomic backup/temp migration. |
| USB / storage / mount / hotplug / NFS exports | AUDITING | Modem-node metadata, partition probes, mount/swap inventory, safe-eject staging, NFS parser and export publication have scoped host/Actions evidence. | Operation-wide unmount ordering, all-device preflight, swap interaction and independent hotplug lifecycle. |
| FTP / Samba / file sharing | AUDITING | FTP worker/mode controls, Samba identity/token validation, credential handoff and staged passdb handling have scoped evidence. | ACL inheritance, non-APP_SMBD36 compatibility, NFS semantics, FTP identity/chroot/anonymous-write/umask and external callers. |
| Init / services / PID 1 / process lifecycle | AUDITING | Signal-context deferral, service-stop escalation and survivor checks have scoped host/Actions evidence. | All-caller start/stop results, watchdog/PID ownership, dependency ordering, shutdown and target BusyBox semantics. |
| IPC / shared helpers / common libraries | PARTIAL INVENTORY — NOT AUDITED | Some shared helper contracts are used by bounded RC/HTTPD fixes; no whole shared-layer audit is claimed. | Inventory public helpers/FIFOs/notify paths and callers; inspect framing, ownership, races and error propagation. |
| Third-party integrated userland / local patches | NOT INVENTORIED | Historical fixes exist in selected vendored services and packages. | Full inventory from build manifests, patch queues and init/config consumers; prioritize exposed/high-failure lifecycle paths. |
| Build / CI / firmware delivery | AUDITING | Focused GNU89 host regressions and exact-head Actions are used for bounded fixes. | Maintain CI contracts; schedule MIPS/uClibc build, complete image/TRX generation and artifact inspection as consolidated milestones. |

## Highest-priority queue

1. **P1/P2 — Firewall/NAT lifecycle ownership:** prove or clear FULLCONENAT global notifier/reference-count versus per-net namespace ownership, including registration failure, event delivery and unload cleanup. Independently review IPv4/IPv6 writer and rollback ordering.
2. **P2 — Wireless interface-state residuals:** guest-interface-up result in `check_inic_mii_rebooted()`; interface-down and LED publication in `stop_wifi_all_wl/rt()`.
3. **P2 — WAN route lifecycle follow-up:** route helpers now return/log first errors while continuing all operations; keep WAN PARTIAL / AUDITING and revisit callers only for a demonstrated downstream inconsistency.
4. **P1/P2 — Switch status contract:** cross-tree return conventions and consumers of PHY status APIs; retain the kernel/user ABI boundary.
5. **Whole-userland rotation:** storage/hotplug, init/process lifecycle, NVRAM restore, HTTPD/session/CSRF, FTP/Samba/NFS, DNS and shared infrastructure. Re-rank using current evidence after each bounded task.

## Verification status

| Level | State | Rule |
|---|---|---|
| L1 static/source review | PASS for recorded bounded changes, including current route result aggregation | Do not extend one finding to an unaudited caller/module. |
| L2 host regression / Actions | Prior WAN/HTTPD runs PASS on `8a02953...`; route helper host regression PASS; new route workflow PENDING at commit creation | Reconcile exact head SHA and run ID before marking the current round Actions-verified. |
| L3 MIPS/uClibc + complete firmware artifact | NOT VERIFIED | No target build/image claim until produced and inspected. |
| L4 XY-C1 or other target hardware | NOT VERIFIED | No flash/boot/runtime claim until physically tested. |

## Whole-tree audit discipline

- Source inventory must come from actual tree/build manifests/init scripts/call graphs, not only this list.
- Mark an area closed only when defined scope and callers are reviewed; a host test verifies only the covered policy/source contract.
- Keep security review active in parallel with stability work. P0 critical security, corruption, boot or severe outage findings override ordinary work.
- Do not repeatedly expand a recently polished area while higher-risk modules remain unaudited.
- For earlier evidence, use [roadmap history](docs/roadmap-history/INDEX.md), [handoff history](docs/handoffs/INDEX.md) and [dev-log archive](docs/dev-log/INDEX.md).
