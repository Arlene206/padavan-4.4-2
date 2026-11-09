# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2 (maintained fork).
- Latest source/test checkpoint: `6175e15b0ce153f5829ea2324e7fb9acb909906e`. Later documentation-only commits may advance `main`; refetch live HEAD before writes.
- Exact source/test-head HTTPD Authentication and Security Regression [42571973359](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42571973359) PASS and AI handoff docs [42571973171](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42571973171) PASS on `6175e15`. USB storage [42570160275](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42570160275) passed on the earlier storage source checkpoint `c2706ac`.
- Validation limit: source review and host/Actions only. No production MIPS/uClibc objects, firmware/TRX or live hardware tests.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete before lease publication; fail-closed deconfig route/interface/resolver checks gate temporary-state reset; ESRCH benign. | BusyBox waits for DHCP event scripts but ignores child exit status. Remaining bound transaction: interface `ifconfig`, gateway `route_add`, resolver publication/close failures and last-known-good rollback; leasefail/NAK policy. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition inventory staging, swapoff-before-unmount, operation-wide storage lock, deferred-start retry, Optware lifecycle locking and nonblocking teardown try-lock to avoid lock inversion. | opt-umount stop-hook / second umount status propagation; broader in-tree caller-graph test draft not yet in CI; external scripts, filesystem runtime and hardware. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| NFS / Samba / FTP / accounts / add-ons | AUDITING | NFS mount inventory and selected Samba/account boundary checks. | Permissions, command injection, service failure and external script lifecycle. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. | Cross-module ordering, shutdown races and package supervision. |

## Priority queue

1. P1/P2 — Complete LAN DHCP bound transaction. Deconfig now aborts on a failed recorded-gateway delete (except ESRCH), static `ifconfig`, or resolver lock/open/close error and does not reset temporary lease identity until success. Next make bound `ifconfig` / `route_add` / resolver publication checked and coherent with the last-known-good lease despite BusyBox ignoring callback exit status.
2. P2 — Optware safe-eject error propagation. Check every S??* stop return and both /opt umount attempts; propagate failure to the C caller so it never treats incomplete Optware teardown as complete. Keep the try-lock nonblocking below storage.lock.
3. P2 — Storage caller-graph regression. Revalidate the drafted whole-callgraph Python contract and add it with mountpoint and lifecycle lock tests to storage-hotplug-stability.yml.
4. P2 — Cross-module rotation. After DHCP/storage bounded work, rotate to another userland area (FTP/Samba/account or add-on services). Do not get stuck in SkipDBv2, HTTPD, storage or DHCP.
5. Separate kernel task — FULLCONENAT mapping namespace state. Keep separate from userland work.

## Working rules

- Trace complete caller/callee paths, state changes, error returns and cleanup before editing.
- Prefer focused GNU89 host policy tests plus semantic source contracts; avoid brittle symbol-only checks.
- Every path-filtered workflow watches production files, helper, tests and workflow for push and pull_request.
- Actions evidence is valid only for the exact head SHA. Inspect logs for failures.
- Keep root dev_log.md at exactly the newest 20 complete records; append evicted records verbatim to indexed shards under 512 KiB.
- Preserve superseded root handoff/roadmap snapshots byte-for-byte in docs archives and update indexes.
- Run scripts/check_ai_handoff_docs.py and exact-head documentation Actions at handoff.

## Verification matrix

| Level | Evidence | Limits |
|---|---|---|
| L1 source review | PASS for AP DHCP input checks, changed-gateway delete-before-publish and deconfig fail-closed route/interface/resolver checks, plus bounded storage/Optware caller and lock-order review. | Does not establish complete bound-side-effect recovery or external custom-script behavior. |
| L2 host / Actions | HTTPD/DHCP [42571973359](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42571973359) and docs [42571973171](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42571973171) PASS on `6175e15`; storage [42570160275](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42570160275) PASS on `c2706ac`. | The Actions run compiles the policy test and checks source ordering, but does not compile the full `net_lan.c`; caller-graph check and Optware error-return fix remain open. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full build phase required. |
| L4 live router / DHCP churn / storage hotplug / physical hardware | NOT VERIFIED. | Requires device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update handoff, roadmap and log when the milestone changes.
- Keep exactly 20 whole records in root dev_log.md; archive complete evicted records and maintain counts/bytes/blob SHAs.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

