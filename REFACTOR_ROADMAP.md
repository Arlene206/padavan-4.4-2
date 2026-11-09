# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2 (maintained fork).
- Latest source/test checkpoint: THIS COMMIT — Optware teardown aggregates all executable service-stop results, blocks /opt detachment after a stop failure, and propagates a failed second unmount attempt. Resolve this commit's exact SHA from live `main` at next recovery.
- HTTPD/DHCP [42577172281](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42577172281) and AI handoff docs [42577172280](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42577172280) PASS on exact prior source SHA `70415484275e9c542be3d45b8bad807fb6a09591`. Storage-hotplug and docs Actions for THIS COMMIT are PENDING until their exact run heads are reconciled.
- Validation limit: source review and host/Actions only. No production MIPS/uClibc objects, firmware/TRX or live hardware tests.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition inventory staging, swapoff-before-unmount, operation-wide storage lock, deferred-start retry, Optware lifecycle locking/nonblocking teardown try-lock, reverse-order stop-hook aggregation, and fail-closed /opt unmount retry result propagation with mock shell coverage. | Broader in-tree caller-graph contract still needs CI; external init scripts, filesystem runtime and hardware remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| NFS / Samba / FTP / accounts / add-ons | AUDITING | NFS mount inventory and selected Samba/account boundary checks. | Permissions, command injection, service failure and external script lifecycle. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. | Cross-module ordering, shutdown races and package supervision. |

## Priority queue

1. P2 — Complete the drafted in-tree storage caller-graph Python regression and add it to `storage-hotplug-stability.yml` alongside mountpoint, operation-lock and Optware lifecycle tests; reconcile exact-head Actions.
2. P2 — Rotate to another userland service area after the caller-graph contract (FTP/Samba/accounts/NFS/add-ons). Keep broad subsystem coverage moving.
3. P2/P3 — Continue LAN DHCP leasefail, accepted-NAK and release state semantics; account for BusyBox discarding callback exit status. Target/live DHCP validation remains outstanding.
4. Separate kernel task — FULLCONENAT mapping-hash and dying-tuple queue namespace ownership. Keep it separate from userland work.

## Working rules

- Trace complete caller/callee paths, state changes, error returns and cleanup before editing.
- Prefer focused GNU89 host policy tests plus semantic source contracts; avoid brittle symbol-only checks.
- Every path-filtered workflow watches production files, helper, tests and workflow for push and pull_request.
- Actions evidence is valid only for the exact head SHA. Inspect logs for failures.
- Let root dev_log.md accumulate from 20 to 119 newest-first records. When the next entry would make 120, archive the oldest 100 complete records verbatim and retain the newest 20; use 384 KiB as an earlier batch-rotation target and 512 KiB as the hard limit.
- Treat SESSION_HANDOFF.md and REFACTOR_ROADMAP.md as live single-copy files: update in place only when their information materially changes; use Git history for previous versions. Do not create new routine handoff/roadmap archive snapshots.
- Preserve existing handoff/roadmap archive snapshots and indexes byte-for-byte as legacy history. Continue validating their inventory and hashes; do not add new snapshots for ordinary edits.
- Run scripts/check_ai_handoff_docs.py when a local checkout is available, then inspect the documentation workflow on the exact pushed SHA. Do not create a follow-up docs-only commit solely to write completed CI results into the files.

## Verification matrix

| Level | Evidence | Limits |
|---|---|---|
| L1 source review | PASS for DHCP input and transactional bound/deconfig handling; PASS for Optware stop-hook aggregation, fail-closed /opt unmount retry status, shared eval/_eval return semantics and both C caller gates; lock-order review retained. | Does not establish complete bound-side-effect recovery or external custom-script behavior. |
| L2 host / Actions | HTTPD/DHCP run [42577172281](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42577172281) and docs run [42577172280](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42577172280) PASS on exact SHA `70415484275e9c542be3d45b8bad807fb6a09591`. Local Optware mock shell regression and `sh -n` PASS; exact-head storage and docs Actions for THIS COMMIT are PENDING. | Storage C host policy tests/source contracts do not build the full target userland or verify external init scripts/filesystem/hardware behavior. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full build phase required. |
| L4 live router / DHCP churn / storage hotplug / physical hardware | NOT VERIFIED. | Requires device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

