# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved byte-for-byte and indexed. Always fetch live `main` before relying on a SHA.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2`, the maintained fork.
- **Baseline main SHA:** `91b3bb41efe0f6ad11d18f8a19d9df3592d7c7b6`.
- **Active branch/checkpoints:** `main`; no separate checkpoint branch was present.
- **This round:** bounded wireless iNIC recovery and radio shutdown-state fix. Source and regression changes are committed together with the active checkpoint; resulting SHA is THIS COMMIT.
- **Previous handoff documentation check:** run [42555712550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555712550) SUCCESS on exact baseline SHA `91b3bb41efe0f6ad11d18f8a19d9df3592d7c7b6`.

## Last completed milestone

- FULLCONENAT per-network-namespace notifier ownership was implemented in commit `91cae3d19bb72a9e0cebcad509db256aded52467`. Exact source lifecycle regression [42555493213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555493213) and handoff documentation contract [42555491302](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555491302) both passed on that code SHA.
- The subsequent documentation-only main update at `91b3bb41efe0f6ad11d18f8a19d9df3592d7c7b6` passed the AI handoff contract at [42555712550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555712550).

## Active task — Wireless interface lifecycle

**Scoped code change implemented; exact-head L2 Actions pending.**

Reviewed:
- `trunk/user/rc/src/net_wifi.c`: `wif_control()`, `wif_control_success()`, `check_inic_mii_rebooted()`, `start_inic_mii()`, `stop_wifi_all_wl()`, `stop_wifi_all_rt()`, `start_wifi_ap_rt()`, `restart_wifi_wl/rt()`.
- `trunk/user/rc/src/inicd.c`: the SIGALRM handler invokes `check_inic_mii_rebooted()`; the periodic timer is re-armed after the check.
- `trunk/user/shared/src/netutils.c`: `is_interface_up()` reports false when the interface is down or its flags query fails; the health-check retains the existing query-before-up behavior.
- `trunk/user/rc/src/switch_mtk_esw.c` and `switch.h`: checked iNIC isolate/RGMII-disable helper contracts.

Findings and fixes in this commit:
1. `check_inic_mii_rebooted()` previously discarded the guest `ifconfig up` status. It now uses `wif_control_success()` and logs that a failed recovery is deferred until the next periodic check.
2. The 5GHz and 2.4GHz stop paths previously discarded all `ifconfig down` results and always published LED_OFF. They now attempt each required stop, accumulate failures without short-circuiting subsequent cleanup, and turn the radio LED off only after all required interface-down operations succeed.
3. The RT3352 MII 2.4GHz path now tears down the actual `IFNAME_INIC_GUEST` and `IFNAME_INIC_MAIN` interfaces started by `start_inic_mii()`, removes guest VLAN bridge membership, and records isolate/RGMII-disable failures. Non-MII boards retain the generic APCLI/WDS/AP interface set.
4. Added a GNU89-compatible sticky shutdown-status helper and host regression, plus source-contract coverage for health-check status, all stop attempts, MII teardown and LED ordering.

## Exact verification required

- Wireless workflow: `.github/workflows/wireless-radio-state.yml` — must pass host compile/test and source contracts on the resulting live main SHA.
- AI handoff documentation contract — must pass on the same exact live main SHA.
- Baseline documentation contract was run [42555712550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555712550) SUCCESS on `91b3bb41efe0f6ad11d18f8a19d9df3592d7c7b6`; this is not evidence for the new commit.

## Other open repository work

- **Wireless — PARTIAL / AUDITING:** this bounded iNIC recovery/shutdown fix is not full driver/switch coverage. Other board-conditional start/stop paths and concurrent restart/stop behavior remain open.
- **Firewall/NAT — AUDITING:** FULLCONENAT notifier state is per-net; mapping hashes and the dying-tuple queue remain module-global and not namespace-keyed. IPv4/IPv6 writer parity, rollback and runtime are open.
- **DNS/DHCP — PARTIAL / AUDITING:** resolver staging/restart and DHCPv6 DUID handling have bounded evidence; DHCP renew/decline/release, client lifecycle and resolver recovery remain open.
- **WAN/PPP — PARTIAL / AUDITING:** route first-error propagation is bounded and tested; broader PPP and runtime routing remain open.
- **USB/storage/hotplug, HTTPD/CGI/authentication, NVRAM/config persistence, init/service lifecycle, switch/PHY/MDIO and shared/third-party userland:** remain open in the roadmap.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/caller review | PASS for the bounded iNIC guest recovery and wireless shutdown-state paths |
| L2 host regression / Actions | New wireless host test and exact-head wireless/docs workflows PENDING at commit creation |
| L3 MIPS/uClibc RC build and full firmware/TRX | **NOT VERIFIED** |
| L4 router flash/boot/radio hardware | **NOT VERIFIED** |

No target RC compile, firmware image, radio-cycle or physical-router result is claimed.

## Exact first action for the next GPT

Fetch live `main`. Match the newest Wireless radio state regression and AI handoff documentation contract runs to that exact SHA. If either fails, inspect the job/log and fix it before new work. Once both pass, rotate to **DNS/DHCP client lifecycle error handling**: trace the actual DHCP client event/renew/decline/release paths and their service callers, identify one concrete ignored-error or stale-state failure, then implement a bounded fix and host regression. Keep FULLCONENAT mapping namespace isolation tracked as a separate P2 item.

## Recovery and archive rule

The superseded handoff is preserved verbatim under `docs/handoffs/archive/2026/11/` and indexed in the same commit. At the next milestone, archive this active checkpoint exactly before replacing it.
