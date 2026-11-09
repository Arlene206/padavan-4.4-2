# Current Session Handoff — Padavan 4.4

> One actionable checkpoint only. Historical handoffs and roadmap snapshots are preserved verbatim in their archive indexes. Always fetch live `main` before relying on any SHA below.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2` — this fork, not upstream.
- **Main before this final CI reconciliation:** `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`.
- **Resulting main SHA:** THIS COMMIT; fetch live `main` on the next recovery.
- **Remote branches:** only `main` was observed; no separate checkpoint branch exists.
- **Route implementation commit:** `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.

## Last completed bounded task

- **Module:** WAN/PPP route management; bounded fix only, not WAN closure.
- **Finding:** `control_static_routes()` discarded all route add/delete return statuses; the DHCP route helper was void and discarded legacy/classless route errors and `strdup()` allocation failures.
- **Fix:** both helpers now return the first nonzero status while continuing every remaining valid route. Both DHCP option lists are processed independently; one allocation failure does not suppress the other. DHCP public wrappers return status; route and allocation failures are logged. Disabling dynamic-route addition still does not block cleanup deletions.
- **Changed code paths:** `net.c`, `net_wan.c`, `rc.h`, new `route_status_policy.h`, new `tests/route_status_policy.c`; WAN route and HTTPD workflow contracts updated.
- **Route workflow:** [42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) SUCCESS on exact code SHA `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; sanitizer regression and route source/caller contracts passed.
- **WAN teardown workflow:** [42552840447](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840447) SUCCESS on that exact code SHA.
- **Documentation workflow on route commit:** [42552840438](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840438) SUCCESS on that exact code SHA.
- **Cross-workflow correction:** first HTTPD run [42552840406](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840406) exposed stale assertions for the previous void DHCP wrapper and old conditional shape. The C DHCP lease test passed. The contract was corrected in `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`; corrected HTTPD regression [42553153266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153266) and documentation contract [42553153260](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153260) both SUCCESS on exact SHA `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`.

## Active task — resume after final documentation CI reconciliation

**Next module: Firewall/NAT lifecycle ownership (P1/P2).** First fetch live main and verify the `AI handoff documentation contract` run for the resulting current SHA. Then rotate away from WAN. Audit FULLCONENAT's global notifier/reference-count ownership against the per-network-namespace APIs integrated in Linux 4.4. Trace registration, event delivery, all failure/unwind paths and module unload. Prove a concrete namespace, cleanup or resource-lifecycle defect before making one bounded change. Keep independent IPv4/IPv6 ruleset writer and rollback parity in the same ledger; do not claim Firewall/NAT closure.

## Other open whole-repository work

- **Wireless — AUDITING:** `check_inic_mii_rebooted()` guest-interface-up result and `stop_wifi_all_wl/rt()` interface-down/LED publication remain open.
- **WAN / PPP — PARTIAL / AUDITING:** route result propagation has bounded source/host/Actions evidence; PPP callbacks, WAN lifecycle and runtime routing remain broader open scope.
- **DNS / DHCP — PARTIAL / AUDITING:** resolver snapshots, parser/rebinding and live recovery remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT notifier/reference-count versus per-net ownership and independent IPv4/IPv6 writer/rollback parity remain unresolved.
- **Switch / PHY, USB/storage/hotplug, NVRAM/configuration, init/service lifecycle, HTTPD/session/CSRF/ACL, FTP/Samba/NFS and shared/third-party userland:** remain open in the roadmap; bounded fixes do not close modules.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/static review | PASS for the selected static/DHCP route helpers, wrappers and WAN/MAN cleanup ordering |
| L2 host regression / Actions | Route sanitizer and source contracts PASS on `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; WAN teardown PASS on the same SHA; corrected HTTPD and docs workflows PASS on `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`; final documentation-only commit's docs run must be reconciled by exact HEAD |
| L3 MIPS/uClibc target build and complete firmware artifact | **NOT VERIFIED** |
| L4 router flash/boot/hardware behavior | **NOT VERIFIED** |

No production RC object, MIPS build, firmware/TRX, live route failure injection or physical-router test is claimed.

## Exact first action for the next GPT

Fetch live `main`. Locate the `AI handoff documentation contract` Actions run for that exact SHA and require SUCCESS with matching `head_sha`. Re-read this handoff, roadmap, newest 20 dev-log records and archive indexes. Then begin the bounded FULLCONENAT namespace/notifier ownership audit; do not repeat WAN route status propagation.

## Recovery rule

After the next milestone, replace this active checkpoint and archive this exact version verbatim under `docs/handoffs/archive/YYYY/MM/` in the same commit.