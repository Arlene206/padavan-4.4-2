# Current Session Handoff — Padavan 4.4

> One actionable checkpoint only. Historical handoffs and roadmap snapshots are preserved verbatim in their archive indexes. Always fetch live `main` before relying on any SHA below.

## Current repository state

- **Repository:** `Arlene206/padavan-4.4-2` — this fork, not upstream.
- **Main before this workflow-contract reconciliation:** `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **Resulting main SHA:** THIS COMMIT; fetch live `main` on the next recovery.
- **Remote branches:** only `main` was observed during recovery; no separate checkpoint branch.
- **Previous code commit:** static/DHCP route error propagation, `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.

## Last completed bounded task

- **Module:** WAN/PPP route management; bounded fix only, not WAN closure.
- **Finding:** `control_static_routes()` discarded all route add/delete return statuses; the DHCP route helper was void and discarded legacy/classless route errors and `strdup()` allocation failures.
- **Fix:** static and DHCP route helpers now return the first nonzero status while continuing every remaining valid route. Both DHCP lists are processed independently; one allocation failure does not suppress the other. DHCP public wrappers return status; route and allocation failures are logged. Disabling dynamic-route addition still does not block cleanup deletions.
- **Changed code paths:** `net.c`, `net_wan.c`, `rc.h`, new `route_status_policy.h`, new `tests/route_status_policy.c`; new WAN route status workflow.
- **Exact implementation-head Actions:** [WAN route operation status regression 42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) SUCCESS on exact code SHA `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; both sanitizer regression and source-contract steps passed. [WAN interface teardown stability 42552840447](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840447) also SUCCESS on that same SHA. [AI handoff documentation contract 42552840438](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840438) SUCCESS on that same SHA.
- **HTTPD-wide regression:** run [42552840406](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840406) FAILED only in the existing DHCP lease source-contract step. The C DHCP lease policy regression passed; the stale Python contract expected the old `void` DHCP wrapper signature and an `if (is_add)` branch. The production helper now returns `int` and selects add/delete with a checked ternary. This workflow contract is corrected in the current follow-up commit; its exact-head rerun is PENDING.

## Active task — resume after exact-head contract CI

**Next module: Firewall/NAT lifecycle ownership (P1/P2).** After verifying the corrected HTTPD workflow and documentation-contract workflow on the resulting exact main SHA, rotate away from WAN. Audit FULLCONENAT's global notifier/reference-count ownership against the per-network-namespace APIs integrated in Linux 4.4. Trace registration, event delivery, all failure/unwind paths and module unload. Prove a concrete namespace, cleanup or resource-lifecycle defect before making one bounded change. Keep independent IPv4/IPv6 ruleset writer and rollback parity in the same ledger; do not claim Firewall/NAT closure.

## Other open whole-repository work

- **Wireless — AUDITING:** `check_inic_mii_rebooted()` guest-interface-up result and `stop_wifi_all_wl/rt()` interface-down/LED publication remain open.
- **WAN / PPP — PARTIAL / AUDITING:** route result propagation has bounded source/host/Actions evidence; PPP callback, WAN lifecycle and runtime routing coverage remain broader open scope.
- **DNS / DHCP — PARTIAL / AUDITING:** resolver snapshots, parser/rebinding and live recovery remain open.
- **Firewall / NAT — AUDITING:** FULLCONENAT notifier/reference-count versus per-net ownership and independent IPv4/IPv6 writer/rollback parity remain unresolved.
- **Switch / PHY, USB/storage/hotplug, NVRAM/configuration, init/service lifecycle, HTTPD/session/CSRF/ACL, FTP/Samba/NFS and shared/third-party userland:** remain open in the roadmap; bounded fixes do not close modules.

## Verification boundary

| Level | Status |
|---|---|
| L1 source/static review | PASS for the selected static/DHCP route helpers, wrappers and WAN/MAN cleanup ordering |
| L2 host regression / Actions | Route sanitizer regression and route source contracts PASS on exact code SHA `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; documentation contract PASS on that SHA; corrected HTTPD source-contract rerun PENDING |
| L3 MIPS/uClibc target build and complete firmware artifact | **NOT VERIFIED** |
| L4 router flash/boot/hardware behavior | **NOT VERIFIED** |

No production RC object, MIPS build, firmware/TRX, live route failure injection or physical-router test is claimed. The broad HTTPD workflow's failed step is a stale source assertion, not a production compile result; its correction must still pass.

## Exact first action for the next GPT

Fetch live `main`. Inspect the corrected `HTTPD Authentication and Security Regression` and `AI handoff documentation contract` runs; require successful conclusions and exact `head_sha` equality with main. Then read this handoff, roadmap, newest 20 dev-log records and archive indexes, and begin the bounded FULLCONENAT namespace/notifier ownership audit. Do not repeat WAN route result propagation.

## Recovery rule

After the next milestone, replace this active checkpoint and archive this exact version verbatim under `docs/handoffs/archive/YYYY/MM/` in the same commit.