# Current Session Handoff — Padavan 4.4

> One actionable checkpoint only. Historical handoffs and roadmap snapshots are preserved verbatim and indexed. Always fetch live `main` before relying on any SHA below.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2` — this fork, not upstream.
- **Base main SHA before this commit:** `93277fe13c6c53e49fd94d3badfd4a1a4def0241`.
- **Resulting main SHA:** THIS COMMIT; fetch live `main` during the next recovery and match Actions to that exact SHA.
- **Active branch:** `main`.
- **Remote branches:** only `main` was observed; no separate checkpoint branch exists.
- **Prior handoff CI:** [AI handoff documentation contract run 42553418388](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553418388) SUCCESS on exact base SHA `93277fe13c6c53e49fd94d3badfd4a1a4def0241`.

## Last completed bounded task

- **Module:** WAN/PPP route management; bounded fix only, not WAN closure.
- **Finding/fix:** static and DHCP route helpers now preserve the first operation error while continuing remaining valid operations; DHCP route-list allocation errors are propagated and logged. The dependent HTTPD source contract was updated for the status-returning DHCP wrapper.
- **Code SHA:** `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; route regression [42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) SUCCESS, WAN teardown [42552840447](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840447) SUCCESS on that exact SHA.
- **Dependent contract correction:** `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`; corrected HTTPD regression [42553153266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153266) and documentation contract [42553153260](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153260) SUCCESS on exact SHA `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`.

## Active task — FULLCONENAT per-net notifier ownership

- **Module / priority:** Linux Netfilter `xt_FULLCONENAT`; P2 connection-lifecycle and namespace-cleanup reliability. Firewall/NAT remains **AUDITING**.
- **Confirmed finding:** the kernel stores conntrack event notifier state in `struct net` (`net->ct.nf_conntrack_event_cb` or `net->ct.nf_conntrack_chain`). FULLCONENAT previously kept a single module-global notifier and `tg_refer_count`: only the first namespace received destroy events, while the final target removed in another namespace attempted to unregister from that different namespace. The legacy API checks notifier identity with `BUG_ON(notify != new)`, making the cross-netns teardown capable of panicking the kernel; chain-event builds would unregister against the wrong chain and could leave event/GC state stale.
- **Implementation in this commit:** one registry node, notifier instance and reference count per `struct net`; first target in each namespace registers on that namespace, later targets there share its count, and the last target unregisters using the exact stored owning namespace and notifier object. Allocation/registration/overflow failures unwind the `nf_ct_netns_get()` reference. Unregister is followed by an RCU grace period before freeing callback storage.
- **Teardown evidence:** IPv4 and IPv6 iptables entry cleanup pass the owning `net` to the target destructor and release the target module reference only after `.destroy` returns. The focused workflow now locks this ownership ordering into source contracts.
- **Changed implementation/test paths:** `trunk/linux-4.4.x/net/netfilter/xt_FULLCONENAT.c`; `.github/workflows/fullconenat-lifecycle.yml`; related source contracts read `include/net/netns/conntrack.h`, IPv4 `ip_tables.c`, IPv6 `ip6_tables.c`, and the existing conntrack/restore sources.
- **Independent remaining risk:** FULLCONENAT's IPv4/IPv6 mapping hash tables and dying-tuple work list are still module-global and are not keyed by network namespace. This change fixes notifier ownership only; namespace isolation of mapping state, IPv4/IPv6 writer parity, rollback and runtime behavior remain open. Do not mark Firewall/NAT or FULLCONENAT complete.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for notifier API ownership, registration/unwind, target destructor, RCU release and iptables rule/module teardown paths |
| L2 host regression / Actions | New FULLCONENAT source-contract workflow and documentation contract are PENDING on THIS COMMIT; verify exact head SHA. No local kernel object or host executable build was run. |
| L3 MIPS/uClibc target build and complete firmware artifact | **NOT VERIFIED** |
| L4 router flash/boot/hardware behavior | **NOT VERIFIED** |

No kernel module build, MIPS target build, firmware/TRX, live multi-netns conntrack/NAT test or physical-router validation is claimed.

## Other open whole-repository work

- **Wireless — AUDITING:** guest-interface-up handling in `check_inic_mii_rebooted()`; interface-down and LED publication in `stop_wifi_all_wl/rt()`; remaining driver/switch consumers.
- **WAN / PPP — PARTIAL / AUDITING:** route status propagation has bounded source/host/Actions evidence; PPP callbacks, WAN lifecycle and runtime routing remain broader open scope.
- **DNS / DHCP — PARTIAL / AUDITING:** resolver snapshots, parser/rebinding and DHCP client recovery remain open.
- **Firewall / NAT — AUDITING:** per-net notifier ownership is fixed in this bounded change pending exact-head CI; mapping hash namespace scoping and IPv4/IPv6 writer/rollback parity remain open.
- **USB/storage/hotplug, Switch/PHY, NVRAM/configuration, init/service lifecycle, HTTPD/session/CSRF/ACL, FTP/Samba/NFS and shared/third-party userland:** remain open in the roadmap; bounded fixes do not close modules.

## Exact first action for the next GPT

Fetch live `main`. Locate both **FULLCONENAT lifecycle regression** and **AI handoff documentation contract** Actions runs whose `head_sha` exactly matches that live SHA. Require both to be SUCCESS. If either fails, inspect the failed job/log and fix this checkpoint before selecting new work. If both pass, rotate to Wireless: inspect `check_inic_mii_rebooted()` guest-interface-up result handling, then `stop_wifi_all_wl/rt()` interface-down and LED publication gates. Keep the global FULLCONENAT mapping-table namespace scope explicitly open for later risk-based review; do not claim the module is closed.

## Recovery rule

After the next milestone, replace this active checkpoint and archive this exact version verbatim under `docs/handoffs/archive/YYYY/MM/` in the same commit.
