# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2 (maintained fork).
- Latest source/test checkpoint: THIS COMMIT — transactional AP-mode LAN DHCP bound/renew address-route-resolver publication with compensating rollback. Resolve the exact SHA from live `main` at next recovery; later documentation-only commits may advance the branch.
- Exact-head HTTPD/DHCP and AI handoff docs for THIS COMMIT are PENDING; the previous code checkpoint `c42ce06` had both runs 42572938754/42572938846 PASS. The pre-change docs-only main `2f4942b` docs run 42574909848 PASS. USB storage [42570160275](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42570160275) passed on the earlier storage source checkpoint `c2706ac`.
- Validation limit: source review and host/Actions only. No production MIPS/uClibc objects, firmware/TRX or live hardware tests.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig route/interface checks plus atomic staged resolver cleanup; bound/renew checked address and route operations, resolver lock/staging/publication and compensating rollback with last-known-good lease fields preserved. | BusyBox discards event-script exit status; exact-head host/source contract and retry behavior still need verification. Leasefail/NAK/release policy and live DHCP transitions remain open. |
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

1. P1/P2 — Verify the new LAN DHCP bound transaction on the exact implementation SHA. The host regression must cover address clear/set, gateway delete/add, resolver publication, rollback, pre-existing routes (`EEXIST`) and failed rollback; then continue leasefail/NAK/release semantics. MIPS/firmware and live DHCP remain unverified.
2. P2 — Optware safe-eject error propagation. Check every S??* stop return and both /opt umount attempts; propagate failure to the C caller so it never treats incomplete Optware teardown as complete. Keep the try-lock nonblocking below storage.lock.
3. P2 — Storage caller-graph regression. Revalidate the drafted whole-callgraph Python contract and add it with mountpoint and lifecycle lock tests to storage-hotplug-stability.yml.
4. P2 — Cross-module rotation. After DHCP/storage bounded work, rotate to another userland area (FTP/Samba/account or add-on services). Do not get stuck in SkipDBv2, HTTPD, storage or DHCP.
5. Separate kernel task — FULLCONENAT mapping namespace state. Keep separate from userland work.

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
| L1 source review | PASS for AP DHCP input checks, changed-gateway delete-before-publish and deconfig fail-closed route/interface/atomic-resolver checks, plus bounded storage/Optware caller and lock-order review. | Does not establish complete bound-side-effect recovery or external custom-script behavior. |
| L2 host / Actions | HTTPD/DHCP [42572938754](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938754) and docs [42572938846](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42572938846) PASS on `c42ce06`; storage [42570160275](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42570160275) PASS on `c2706ac`. | The Actions run compiles/runs the policy test and checks source ordering, but does not compile the full `net_lan.c`; caller-graph check and Optware error-return fix remain open. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full build phase required. |
| L4 live router / DHCP churn / storage hotplug / physical hardware | NOT VERIFIED. | Requires device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

