# Padavan 4.4 — Whole-Repository Modernization Roadmap

> **Current dashboard only.** Historical snapshots are preserved verbatim in [the roadmap history index](docs/roadmap-history/INDEX.md). A bounded fix does not prove module completion. Re-fetch live `main` before trusting any SHA.

- **Repository:** `Arlene206/padavan-4.4-2`.
- **Latest source milestone:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a` — stage one complete `/proc/partitions` snapshot before ejected-device unmounts and propagate cleanup failure into PID 1 notification completion. The contract synchronization commit `82096fbab795598bc6eb4384003e8f66998d1997` updates both legacy HTTPD eject assertions and corrects archive byte counts.
- **Earlier bounded milestones:** `0810b15526a7c6c7b7ba5980679c62fd522f481a` — wireless iNIC guest recovery and radio shutdown/LED publication; `91cae3d19bb72a9e0cebcad509db256aded52467` — FULLCONENAT notifier state moved to per-network-namespace ownership.
- **Exact-head results:** storage hotplug [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) **PASS** on source SHA `7fee5c1fc589dc98c3333dd4e052d28881b8756a`; HTTPD Authentication and Security Regression [42562165008](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165008) and AI handoff documentation contract [42562165077](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165077) both **PASS** on exact integration SHA `82096fbab795598bc6eb4384003e8f66998d1997`. Earlier failed attempts 42561802353 / 42561802333 exposed stale source assertions and index byte counts; both were corrected before these successful runs.

## Status semantics

- `NOT INVENTORIED` — module/source ownership not mapped.
- `NOT AUDITED` — inventoried but no meaningful audit completed.
- `AUDITING` — ongoing; bounded findings/fixes do not close the module.
- `PARTIAL / AUDITING` — substantial scoped work exists; important paths remain open.

## Whole-repository module status

| Module / area | Status | Evidence already established | Open coverage / next boundary |
|---|---|---|---|
| WAN / PPP / routing / IPv6 | PARTIAL / AUDITING | PPP callback identity/generation, startup failure gates, service-stop fences, WAN management/physical/USB NDIS teardown, static/DHCP route first-error aggregation and interface-down failure handling have bounded source/host evidence. | Broader PPP failure propagation, DHCP client lifecycle beyond router withdrawal, route recovery semantics and target runtime remain open. |
| Wireless management / driver / iNIC / WDS | PARTIAL / AUDITING | Radio-state publication, AP/APCLI/WDS bridge-up gates, RT3352 iNIC startup recovery and guest-interface shutdown/LED ordering have bounded source/host/Actions evidence. | Other board-conditional lifecycle paths, ioctl/driver consumers and concurrent restart/stop behavior; target radio-cycle unverified. |
| Firewall / NAT / Netfilter / IPv6 | AUDITING | FULLCONENAT notifier/reference-count ownership is per `struct net`; bounded lifecycle contracts verify correct-owner unregister, RCU release and IPv4/IPv6 rule cleanup before module reference release. | Mapping hashes and dying-tuple queue remain module-global and not namespace-keyed; investigate cross-netns tuple matching, IPv4/IPv6 writer parity, rollback, unload safety and runtime. |
| DNS / DHCP / resolver | PARTIAL / AUDITING | DNSMasq file staging/restart, DHCP lease input validation, route reconciliation, resolver refresh, DHCPv6 DUID file handling and WAN/IPTV handling of an omitted DHCP Router option have bounded tests/contracts. | LAN DHCP input validation, deconfig/leasefail/NAK/decline/release semantics, ignored callback/service errors, resolver recovery and runtime integration. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | One `/proc/partitions` snapshot is staged before ejected-mount cleanup; the PID 1 caller propagates cleanup failure. Focused storage GNU89/source regression and integrated HTTPD storage contracts pass on their recorded exact heads. | Continue caller/concurrency audit: disk add/remove ordering, safe-remove caller coverage, per-device lock identity, overlapping ejects and filesystem/runtime behavior; no module closure. |
| HTTPD / CGI / authentication / sessions | PARTIAL / AUDITING | Auth/route access, same-NAT session ownership and selected CSRF/Origin, upload, output-encoding and storage CGI boundaries have source-contract coverage. | Whole CGI inventory, request-to-shell/filesystem sinks, logout/session lifecycle, TLS/archive and remaining ACL surfaces. |
| Switch / PHY / MDIO / utilities | PARTIAL / AUDITING | Clause 22/45 CLI parsing and selected ioctl, close and MDIO timeout error propagation have host/Actions evidence. | Cross-tree caller inventory, PHY sequencing, board-specific register paths, target builds and live register failure handling. |
| NVRAM / configuration persistence | PARTIAL / AUDITING | Default restore/erase durability, group compaction, checked reads and selected file staging have failure-contract tests. | Whole call graph, power-loss behavior, flash wear and transaction/atomicity coverage. |
| Init / service lifecycle / PID 1 | PARTIAL / AUDITING | Bounded service termination, signal deferral/reaping and WAN stop failure gates have regression contracts. | Process ownership, respawn races, shutdown ordering and all service dependency paths. |
| FTP / Samba / NFS / accounts | PARTIAL / AUDITING | Passdb/share staging, account identity boundaries and complete mount-inventory gate for NFS export have selected tests. | Whole input surfaces, privilege boundaries and runtime filesystem semantics. |
| Shared / third-party userland / kernel integration | NOT INVENTORIED / AUDITING | Selected parser, ABI and lifecycle fixes exist across integrated components. | Complete package/source inventory; no subsystem closure claim. |

## Current priority queue

1. **P2 — USB/storage hotplug and safe-removal caller audit:** this round fixes operation-wide partition-inventory consistency and discarded eject errors. Continue tracing `mdev_sd_main()`, `mdev_mmc_main()`, `safe_remove_usb_device()`, eject commands and per-device lock identity for overlapping add/remove/safe-eject races; keep filesystem and hardware validation open.
2. **P2 — FULLCONENAT mapping namespace isolation:** inspect module-global IPv4/IPv6 mapping hashes and dying-tuple queue for cross-namespace tuple matching. Keep separate from the already-fixed notifier ownership; no kernel change is part of the current user-space batch.
3. **P2 — Remaining DHCP lifecycle:** audit LAN client lease validation and WAN/IPTV deconfig, leasefail, NAK/decline/release plus callback/service failure handling.
4. **P2 — Wireless driver/switch coverage:** other board-conditional interface sets, start/stop ordering and concurrent restart/stop behavior.
5. **P2 — Resolver and service recovery:** test failure paths and recovery after generated-file, DNSMasq restart and service-control errors.

## Verification matrix

| Level | Evidence status | Limitation |
|---|---|---|
| L1 static/source review | PASS for the storage eject caller chain, complete partition-inventory preflight and notification failure propagation; earlier DHCP, FULLCONENAT notifier and wireless lifecycle reviews remain valid. | Does not establish namespace isolation of mapping state or close any module. |
| L2 host regression / Actions | Storage hotplug GNU89 regression + source contract PASS [42561240425](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42561240425) on `7fee5c1`; full HTTPD/security regression PASS [42562165008](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165008) and AI docs contract PASS [42562165077](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42562165077) on `82096fb`. The final docs-only snapshot will receive its own docs-contract run. | Host/source contracts do not compile the target RC or verify router behavior. |
| L3 MIPS/uClibc and complete firmware | **NOT VERIFIED** | Run the integrated build after core source governance is substantially complete. |
| L4 hardware/flash/boot/runtime | **NOT VERIFIED** | Requires router testing; do not infer from host or source-contract checks. |

## Governance

- Root handoff/roadmap/log remain compact; historical versions are byte-preserved and indexed under `docs/`.
- Root `dev_log.md` holds exactly the newest 20 complete entries, newest first; archive shards split only between complete dated records and remain below 512 KiB.
- `scripts/check_ai_handoff_docs.py` and the `AI handoff documentation contract` workflow are authoritative for byte counts, Git blob hashes, section counts and record counts.
- Re-fetch live `main` and exact-head Actions at every GPT handoff; never infer success from queued or stale results.
