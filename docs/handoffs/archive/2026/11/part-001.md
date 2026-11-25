# Current Session Handoff — Padavan 4.4

> **One active checkpoint only.** Historical checkpoints remain verbatim in [the handoff archive index](docs/handoffs/INDEX.md). Always fetch live `main` before relying on any SHA below.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2` — this fork, not upstream or another similarly named fork.
- **Branch:** `main`.
- **Main SHA at the start of this governance migration:** `71de75dd39b2ebc87fa56b1c74ff798cc0e57fdf`. This documentation update is based directly on it; fetch resulting live `main` on every new session.
- **Remote branches:** latest inventory before this migration contained only `main`; no separate checkpoint branch existed.
- **Latest completed firmware-source change:** `8a02953fb60cced248254df86feca85e8bc42d10` — verify the active USB NDIS interface during WAN teardown.
- **Exact-head code validation:** WAN workflow [42548284186](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42548284186) and broad HTTPD/service workflow [42548284184](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42548284184) both **SUCCESS** on exact code SHA `8a02953fb60cced248254df86feca85e8bc42d10`. These do not validate a later documentation-only HEAD.

## Last completed bounded task

- **Module:** RC WAN/PPP and USB modem teardown; status remains **PARTIAL / AUDITING**.
- **Fix:** `stop_wan()` verifies each distinct management, physical-WAN and active USB NDIS interface. It attempts remaining interfaces after failures, treats `ENODEV` as already unavailable, and skips final WAN state clear/LED-down publication after another interface-down error.
- **Code SHA:** `8a02953fb60cced248254df86feca85e8bc42d10`.
- **Regression:** GNU89 policy regression with `-Wall -Wextra -Werror`, ASan/UBSan and leak detection PASS; the two exact-head Actions runs above PASS.
- **Scope limit:** this does not close WAN/PPP or prove the production RC object, MIPS firmware or hardware behavior.

## Active task — resume before selecting another module

**P2 stability: propagate static and DHCP route add/delete failures without skipping remaining cleanup.**

1. Re-fetch live `main`; inspect `trunk/user/rc/src/net.c::control_static_routes()` and `trunk/user/rc/src/net_wan.c::control_dhcp_routes()`.
2. Enumerate callers and semantics across WAN/MAN up/down, DHCP bound/renew/deconfig and static-route configuration.
3. `route_add()` / `route_del()` already return `route_manip()` errors, but the static helper discards per-route statuses and returns success; the DHCP helper is void and discards dynamic route failures.
4. Define aggregate status. Failed operations must not report false success, but one failure must not short-circuit the remaining cleanup deletions.
5. Add focused GNU89 host regression and meaningful source contracts before changing caller behavior. Verify exact-head Actions and update all three records.
6. Preserve the inspected `udhcpc` SIGUSR2 RELEASE as best-effort unless new evidence shows a defect; the following `kill_services()` survivor check is the authoritative stop fence.

## Other open whole-repository work

- **Wireless — AUDITING:** `check_inic_mii_rebooted()` guest-interface-up result and `stop_wifi_all_wl/rt()` interface-down/LED publication remain open.
- **Firewall/NAT — AUDITING:** independent IPv4/IPv6 writers and restore semantics remain open; FULLCONENAT notifier/reference-count versus per-net namespace ownership is unresolved.
- **Switch/PHY — AUDITING:** cross-tree consumers of `phy_status_port_link()`, `phy_status_port_link_changed()`, `phy_status_port_speed()` and utility/register paths remain open.
- **DNS — PARTIAL / AUDITING:** resolver callers and DNSMasq reload status have bounded exact-head evidence; upstream snapshot transactionality, parser/rebinding and live recovery remain open.
- **USB/storage/hotplug, NVRAM/configuration, init/service lifecycle, HTTPD/session/CSRF/ACL, FTP/Samba/NFS and shared infrastructure:** retain open roadmap status; a bounded fix is not module closure.

## Verification boundary

| Level | Current status |
|---|---|
| L1 source/static review | PASS for last bounded WAN teardown change; docs contract has a dedicated workflow |
| L2 host regression / relevant Actions | PASS for last bounded WAN teardown change on exact code SHA above |
| L3 MIPS/uClibc target build and complete firmware artifact | **NOT VERIFIED** |
| L4 XY-C1 or other router hardware flash/boot/runtime | **NOT VERIFIED** |

No complete firmware image/TRX, production MIPS build, live NDIS failure injection or physical-router test is claimed.

## Exact first action for the next GPT

Fetch live `main`, latest Actions runs and branch inventory; read this handoff, `REFACTOR_ROADMAP.md`, the 20 entries in `dev_log.md` and `docs/dev-log/INDEX.md`. Confirm the documentation-contract workflow on the exact live HEAD. Then resume the route add/delete status-propagation task above; do not select a fresh module first.

## Recovery rule

This is the active checkpoint, not an append-only diary. After the next bounded task, replace it with the new current state and move the old version verbatim to `docs/handoffs/archive/YYYY/MM/` in the same commit.