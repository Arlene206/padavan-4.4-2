# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved byte-for-byte and indexed. Always fetch live `main` and exact-head Actions before relying on a SHA.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2`, the maintained fork.
- **Source milestone:** `4e9614b5b40ff602423da3488d1aa6dfe859d4d4` — DHCP router-option withdrawal is reconciled on renewal for WAN and IPTV.
- **Documentation baseline:** source commit above; this commit rotates the compact handoff/roadmap/log and archives their superseded versions.
- **Branch/checkpoints:** `main`; no separate remote checkpoint branch was present.
- **Exact-head source verification on the source milestone:** AI handoff contract [42559613261](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613261), WAN interface teardown [42559613131](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613131), HTTPD Authentication and Security [42559613119](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613119), and WAN route operation status [42559612983](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559612983) all completed **SUCCESS** on exactly `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`.

## Active task

**DHCP WAN/IPTV renewal router withdrawal — implemented and L2 verified; DNS/DHCP subsystem remains PARTIAL / AUDITING.**

Root cause:
- BusyBox `udhcpc` builds its script environment from DHCP options actually present in the ACK. When DHCP option 3 (Router) is omitted during a renewal, the `router` environment variable is absent rather than present as an empty string.
- The previous `udhcpc_renew()` and `udhcpc_viptv_renew()` compared the gateway only inside `if (getenv("router"))`. A server-side router withdrawal therefore bypassed lease reconciliation and could leave the previous gateway/default route active.

Implementation in `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`:
1. Added `dhcp_lease_router_is_changed()` in `trunk/user/rc/src/dhcp_lease_policy.h`. It normalizes a missing/empty router option to an empty gateway, validates and selects the first router address, then compares it with the saved gateway.
2. Updated WAN and IPTV renew callbacks in `trunk/user/rc/src/net_wan.c` to run this comparison unconditionally, including when `getenv("router") == NULL`. A withdrawal now enters the existing bound/reconciliation path, which removes the previous lease-derived default/host routes and clears the stale gateway.
3. Extended `tests/dhcp_lease_policy.c` for absent, empty, unchanged, changed and invalid router-option cases.
4. Extended the HTTPD/DHCP workflow's source contracts to guard both WAN and IPTV renewal call paths and ensure neither remains conditional on the variable being present.

Exact L2 evidence:
- HTTPD job step **Verify DHCP lease input and renewal reconciliation** passed on the exact code SHA. Logs report `DHCP lease input policy regression: OK` and `DHCP lease normalization, route reconciliation, and resolver refresh contracts: OK`.
- The four exact-head Actions listed above all passed. These checks cover the host policy regression, source contracts, WAN teardown, route status and documentation contract; they do not replace target-router validation.

## Other open repository work

- **USB / storage / hotplug — PARTIAL / AUDITING:** mount/swap inventories, safe-eject and NFS export have bounded tests; caller-wide ordering, concurrent hot-unplug and filesystem runtime remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT notifier ownership is per-network-namespace, but mapping hashes and the dying-tuple queue remain module-global and are not namespace-keyed. IPv4/IPv6 writer parity, rollback and runtime are open.
- **DNS / DHCP / resolver — PARTIAL / AUDITING:** WAN/IPTV lease validation, route reconciliation, resolver refresh and DHCPv6 DUID file handling have bounded evidence. LAN DHCP input handling, deconfig/leasefail/NAK/decline/release semantics, error propagation and runtime integration remain open.
- **WAN / PPP / routing — PARTIAL / AUDITING:** selected route-result propagation and WAN teardown failure gates have exact-head host evidence; broader PPP and runtime routing remain open.
- **Wireless — PARTIAL / AUDITING:** radio-state, bridge-up and RT3352 iNIC lifecycle paths have bounded evidence; other board-conditional paths and concurrent restart/stop remain open.
- **HTTPD / CGI / authentication, NVRAM persistence, init/service lifecycle, switch/PHY/MDIO, USB/storage, FTP/Samba/NFS/accounts and integrated third-party userland:** remain governed as separate open areas in `REFACTOR_ROADMAP.md`. No subsystem is declared complete by this bounded DHCP fix.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/caller review | PASS for the BusyBox missing-option contract and WAN/IPTV router withdrawal call paths. |
| L2 host regression / GitHub Actions | PASS on exact source SHA `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`; see run IDs above. |
| L3 MIPS/uClibc RC compile and complete firmware/TRX | **NOT VERIFIED**. |
| L4 flash/boot, DHCP server withdrawal, route table and physical router behavior | **NOT VERIFIED**. |

No production MIPS/uClibc compile, full firmware image, live network route test or physical-router test is claimed. The documentation contract for this archival rotation must also be fetched and checked against the resulting live `main` SHA.

## Exact first action for the next GPT

1. Fetch live `main`, all three root governance files and their archive indexes. Check the newest **AI handoff documentation contract** run against that exact SHA; do not rely on the source-SHA result above if `main` has moved.
2. If the documentation contract passes, continue whole-repository stability work with **USB/storage hotplug caller and concurrency audit**, starting at `trunk/user/rc/src/hotplug_stor.c`, `hotplug_usb.c`, `services_stor.c` and the storage inventory/eject helpers. Trace actual callers and ordering before choosing one concrete stale-state, ignored-error or race defect; implement a bounded fix and host regression.
3. Keep FULLCONENAT mapping-table namespace isolation as a separate P2 security/stability audit. Do not claim it is fixed by the notifier per-net change, and respect the project rule that kernel/MTK/DTS work is outside the ordinary user-space repair batch.

## Recovery and archive rule

- The superseded handoff is preserved byte-for-byte as `docs/handoffs/archive/2026/11/part-008.md` and indexed in the same commit.
- `REFACTOR_ROADMAP.md` and `dev_log.md` remain compact dashboards/windows; superseded snapshots and evicted complete records are preserved in their indexed archive shards.
- At the next milestone, archive the active handoff and roadmap exactly before replacing them; keep exactly the newest 20 complete records in root `dev_log.md`.
