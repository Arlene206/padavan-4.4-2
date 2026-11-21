# Padavan 4.4 — Whole-Repository Modernization Roadmap

> **Current dashboard only.** Historical snapshots are preserved verbatim in [the roadmap history index](docs/roadmap-history/INDEX.md). A bounded fix does not prove module completion. Re-fetch live `main` before trusting any SHA.

- **Repository:** `Arlene206/padavan-4.4-2`.
- **Latest source milestone:** `cb70977f05b3d7d64dcabf5c02247bda05969f35` — process-shared storage hotplug/safe-removal lock and corrected exact-head regression contracts. The implementation was introduced by `5d5b61cfc9d5dd3bca237ff4be31eb0118da1dfb`; `cb70977` corrected the malformed include and a test contract that confused the intentional early error unlock with normal reverse-order cleanup.
- **Earlier bounded milestones:** `7fee5c1fc589dc98c3333dd4e052d28881b8756a` — complete partition inventory staged before ejected-device unmounts and PID 1 cleanup failure propagation; `0810b15526a7c6c7b7ba5980679c62fd522f481a` — wireless iNIC guest recovery and radio shutdown/LED publication; `91cae3d19bb72a9e0cebcad509db256aded52467` — FULLCONENAT notifier state moved to per-network-namespace ownership.
- **Exact-head results:** storage hotplug [42564939868](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939868), HTTPD Authentication and Security Regression [42564939905](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939905), and AI handoff documentation contract [42564939843](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939843) all **PASS** on exact source SHA `cb70977`. The storage job observed the shared lock blocking a competing process for 2,000 ms; HTTPD/security completed all 39 validation steps.

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
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | One complete `/proc/partitions` snapshot is staged before ejected unmounts and PID 1 propagates cleanup errors. New `/var/lock/storage.lock` serializes SD/MMC hotplug add/remove, manual safe removal, eject-all and PID 1 ejected cleanup across processes; the GNU89 fork regression and exact-head USB/HTTPD contracts pass. | Inventory all direct shell/script mount, unmount, swapon and swapoff paths to prove they cannot bypass the operation lock; continue disk/partition notification ordering, filesystem/runtime and hardware validation. No module closure. |
| HTTPD / CGI / authentication / sessions | PARTIAL / AUDITING | Auth/route access, same-NAT session ownership and selected CSRF/Origin, upload, output-encoding and storage CGI boundaries have source-contract coverage. | Whole CGI inventory, request-to-shell/filesystem sinks, logout/session lifecycle, TLS/archive and remaining ACL surfaces. |
| Switch / PHY / MDIO / utilities | PARTIAL / AUDITING | Clause 22/45 CLI parsing and selected ioctl, close and MDIO timeout error propagation have host/Actions evidence. | Cross-tree caller inventory, PHY sequencing, board-specific register paths, target builds and live register failure handling. |
| NVRAM / configuration persistence | PARTIAL / AUDITING | Default restore/erase durability, group compaction, checked reads and selected file staging have failure-contract tests. | Whole call graph, power-loss behavior, flash wear and transaction/atomicity coverage. |
| Init / service lifecycle / PID 1 | PARTIAL / AUDITING | Bounded service termination, signal deferral/reaping and WAN stop failure gates have regression contracts. | Process ownership, respawn races, shutdown ordering and all service dependency paths. |
| FTP / Samba / NFS / accounts | PARTIAL / AUDITING | Passdb/share staging, account identity boundaries and complete mount-inventory gate for NFS export have selected tests. | Whole input surfaces, privilege boundaries and runtime filesystem semantics. |
| Shared / third-party userland / kernel integration | NOT INVENTORIED / AUDITING | Selected parser, ABI and lifecycle fixes exist across integrated components. | Complete package/source inventory; no subsystem closure claim. |

## Current priority queue

1. **P2 — USB/storage shell caller and operation-lock coverage audit:** mdev add/remove, safe-remove, eject-all and PID 1 ejected cleanup now share `storage.lock`. Next enumerate direct `automount.sh`, `mount`, `umount`, `swapon` and `swapoff` callsites, identify callers that can mutate storage outside the shared lock, and extend coverage only where the caller graph proves a bypass.
2. **P2 — FULLCONENAT mapping namespace isolation:** inspect module-global IPv4/IPv6 mapping hashes and dying-tuple queue for cross-namespace tuple matching. Keep separate from the already-fixed notifier ownership; no kernel change is part of the current user-space batch.
3. **P2 — Remaining DHCP lifecycle:** audit LAN client lease validation and WAN/IPTV deconfig, leasefail, NAK/decline/release plus callback/service failure handling.
4. **P2 — Wireless driver/switch coverage:** other board-conditional interface sets, start/stop ordering and concurrent restart/stop behavior.
5. **P2 — Resolver and service recovery:** test failure paths and recovery after generated-file, DNSMasq restart and service-control errors.

## Verification matrix

| Level | Evidence status | Limitation |
|---|---|---|
| L1 static/source review | PASS for device-vs-operation lock identities, SD/MMC mdev ordering, safe-remove/eject-all and PID 1 ejected cleanup serialization. Existing `automount.sh` mountpoint flock is distinct from the operation lock; direct shell bypasses remain under audit. | Does not establish that every external/script caller takes the shared lock or close the storage module. |
| L2 host regression / Actions | Exact-head storage GNU89 lock/inventory regression PASS [42564939868](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939868); full HTTPD/security PASS [42564939905](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939905); AI docs contract PASS [42564939843](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42564939843), all on `cb70977`. | Host/source contracts do not compile the target RC or verify router behavior. |
| L3 MIPS/uClibc and complete firmware | **NOT VERIFIED** | Run the integrated build after core source governance is substantially complete. |
| L4 hardware/flash/boot/runtime | **NOT VERIFIED** | Requires router testing; do not infer from host or source-contract checks. |

## Governance

- Root handoff/roadmap/log remain compact; historical versions are byte-preserved and indexed under `docs/`.
- Root `dev_log.md` holds exactly the newest 20 complete entries, newest first; archive shards split only between complete dated records and remain below 512 KiB.
- `scripts/check_ai_handoff_docs.py` and the `AI handoff documentation contract` workflow are authoritative for byte counts, Git blob hashes, section counts and record counts.
- Re-fetch live `main` and exact-head Actions at every GPT handoff; never infer success from queued or stale results.
