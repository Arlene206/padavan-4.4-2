# Current Session Handoff — Padavan 4.4

> One actionable checkpoint only. Historical handoffs and roadmap snapshots are preserved verbatim and indexed. Always fetch live `main` before relying on any SHA below.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2` — this fork, not upstream.
- **Base main SHA before this documentation update:** `91cae3d19bb72a9e0cebcad509db256aded52467`.
- **FULLCONENAT implementation commit:** `91cae3d19bb72a9e0cebcad509db256aded52467`; source regression passed on this exact SHA.
- **Resulting main SHA:** THIS DOCUMENTATION COMMIT; fetch live `main` during the next recovery and match the new documentation Actions run to that exact SHA.
- **Active branch:** `main`; only `main` was observed remotely, with no separate checkpoint branch.
- **FULLCONENAT lifecycle regression:** [42555493213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555493213) SUCCESS on exact code SHA `91cae3d19bb72a9e0cebcad509db256aded52467`.
- **AI handoff documentation contract:** [42555491302](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555491302) SUCCESS on exact code SHA `91cae3d19bb72a9e0cebcad509db256aded52467`; the docs-only follow-up triggers a fresh contract run that must be checked against live `main`.

## Last completed bounded task

- **Module:** WAN/PPP route management; bounded fix only, not WAN closure.
- **Finding/fix:** static and DHCP route helpers preserve the first operation error while continuing remaining valid operations; DHCP route-list allocation errors are propagated and logged. The dependent HTTPD source contract was updated for the status-returning DHCP wrapper.
- **Code SHA:** `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; route regression [42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) SUCCESS, WAN teardown [42552840447](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840447) SUCCESS on exact SHA.
- **Dependent contract correction:** `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`; HTTPD regression [42553153266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153266) and docs contract [42553153260](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153260) SUCCESS on exact SHA.

## Active task — FULLCONENAT per-net notifier ownership

- **Scoped implementation complete:** commit `91cae3d19bb72a9e0cebcad509db256aded52467`, `fix(netfilter): scope FULLCONENAT notifiers per netns`.
- **Confirmed defect:** Linux 4.4 stores conntrack event notifier state in each `struct net` (`net->ct.nf_conntrack_event_cb` or `net->ct.nf_conntrack_chain`). The old module-global `tg_refer_count` and notifier registered only in the first namespace; a last-rule teardown in another namespace then attempted unregister on a different `par->net`. The legacy notifier API checks identity using `BUG_ON(notify != new)`, so cross-netns teardown could panic the kernel; chain-event builds targeted the wrong chain and missed the correct cleanup events.
- **Implemented:** one notifier object and refcount per network namespace; registration occurs once per owning namespace, reference sharing is limited to that namespace, and the last destructor unregisters with its stored `net` and notifier. Allocation/registration/overflow errors unwind `nf_ct_netns_get()`; callback storage is freed only after an RCU grace period.
- **Teardown verified at source-contract level:** IPv4 and IPv6 iptables cleanup pass the owning `net` to target `.destroy`, then release the target module reference. The lifecycle workflow checks both notifier API variants, error unwind, namespace-local reference sequences and this teardown ordering.
- **Actions:** FULLCONENAT lifecycle run 42555493213 SUCCESS; AI handoff docs run 42555491302 SUCCESS on exact code SHA `91cae3d19bb72a9e0cebcad509db256aded52467`.
- **Independent remaining risk:** FULLCONENAT IPv4/IPv6 mapping hashes and the dying-tuple work list remain module-global and not keyed by network namespace. The notifier fix does not establish mapping isolation or full multi-netns support. Keep Firewall/NAT **AUDITING**; IPv4/IPv6 writer parity, rollback, runtime behavior and unload paths beyond the tested ownership contract remain open.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for per-net notifier API ownership, registration/unwind, correct-owner unregister, RCU lifetime and iptables rule/module teardown paths |
| L2 focused host regression / Actions | PASS on exact code SHA `91cae3d19bb72a9e0cebcad509db256aded52467` for FULLCONENAT lifecycle source contract and compact handoff/archive checker. A docs-only follow-up requires a fresh exact-head documentation contract run. |
| L3 MIPS/uClibc target build and complete firmware artifact | **NOT VERIFIED** |
| L4 router flash/boot/hardware behavior | **NOT VERIFIED** |

No kernel object/module compilation, live multi-netns conntrack/NAT execution, MIPS/uClibc build, firmware/TRX or physical-router test is claimed.

## Other open whole-repository work

- **Wireless — AUDITING:** guest-interface-up handling in `check_inic_mii_rebooted()`; interface-down and LED publication in `stop_wifi_all_wl/rt()`; remaining driver/switch consumers.
- **WAN / PPP — PARTIAL / AUDITING:** route status propagation has bounded source/host/Actions evidence; PPP callbacks, WAN lifecycle and runtime routing remain open.
- **DNS / DHCP — PARTIAL / AUDITING:** resolver snapshots, parser/rebinding and DHCP client recovery remain open.
- **Firewall / NAT — AUDITING:** per-net notifier ownership has source-contract pass; mapping namespace scoping and IPv4/IPv6 writer/rollback parity remain open.
- **USB/storage/hotplug, Switch/PHY, NVRAM/configuration, init/service lifecycle, HTTPD/session/CSRF/ACL, FTP/Samba/NFS and shared/third-party userland:** remain open in the roadmap.

## Exact first action for the next GPT

Fetch live `main`. Require the newest **AI handoff documentation contract** run to have `head_sha` exactly equal to that live SHA and conclusion SUCCESS. Also retain [FULLCONENAT lifecycle run 42555493213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555493213) as the source-code regression evidence for commit `91cae3d19bb72a9e0cebcad509db256aded52467`; it is not expected to rerun on a documentation-only commit. If the newest documentation run fails, inspect its job/log and repair the handoff records first. Once green, rotate to Wireless: inspect `check_inic_mii_rebooted()` guest-interface-up result handling, then `stop_wifi_all_wl/rt()` interface-down and LED publication. Keep the global FULLCONENAT mapping-table namespace scope tracked as an open risk.

## Recovery rule

After the next code milestone, replace this active checkpoint and archive this exact version verbatim under `docs/handoffs/archive/YYYY/MM/` in the same commit.
