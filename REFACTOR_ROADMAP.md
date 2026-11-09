# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Base main SHA for THIS storage source-contract correction: `b13ca6b46e0bef9474ee6744cf4f7f1734aadc97`; resulting commit: THIS COMMIT. The remote branch inventory contained only `main`; no separate checkpoint.
- On the exact base SHA, USB storage hotplug regression [42608853044](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42608853044) passed shell syntax, Optware lock/teardown/startup regressions and GNU89 storage tests, but failed its final Python source contract with `ValueError: substring not found` at the branch marker assertion. AI docs [42608852967](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42608852967) SUCCESS.
- THIS correction anchors the contract to the actual MTD-branch comment in `opt-mount.sh`; the source behavior and remaining semantic checks are unchanged. Exact-head storage/docs results pending.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. Commit `b13ca6b` extends the lifecycle lock and shared supervisor to the internal MTD/UBI `opt-mount.sh` path and prevents RWFS unmount/UBI detach when its Optware stop hook or real unmount fails. | Deferred USB Optware startup still clears pending NVRAM flags before the background child result is known; MTD boot startup remains asynchronous to its C caller. No production object/runtime filesystem test; external init scripts, target filesystem behavior and hardware remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged and atomically published; folder inventory/read errors fail closed; WINS/nmbd/smbd lifecycle is gated on publication. Samba GNU89 helper regression passes; HTTPD's dependent trust-boundary contract now bounds its folder slice to the full writer instead of an early error cleanup; exact-head rerun pending. | Passdb/account mutation consistency, ACL/permission mapping, credential security, live Samba reload behavior and target runtime. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates have exact-head source/CI evidence. Optware startup helpers now provide pathname-safe enumeration, fd-9 isolation and rollback for both removable deferred startup and internal MTD/UBI startup; RWFS shutdown honors Optware teardown failure. | NFS export ACL/permissions and external mounts; asynchronous deferred Optware result/retry semantics and the boot caller's asynchronous MTD startup result remain open; target filesystem and arbitrary third-party hooks remain unverified. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. | Cross-module ordering, shutdown races and package supervision. |

## Priority queue

1. P2 — Reconcile the internal MTD/UBI Optware startup and RWFS shutdown regression against exact-head storage and AI handoff-doc Actions; inspect the new shell syntax and RWFS caller/source-contract steps.
2. P2 — Correct deferred USB Optware startup result/retry semantics in `services_stor.c`: preserve pending flags until actual child success, retry failed/busy startup without duplicate launches, distinguish incomplete mount inventories from empty inventory, and do not rerun integrated storage services on each retry. Audit the boot-time asynchronous MTD startup result separately.
3. P2 — Continue NFS export ACL/permission mapping and external mount lifecycle; continue Samba passdb/account mutation, ACL/permission and credential audit; continue vsftpd authentication/ACL/path-confinement/stop lifecycle review.
4. P2/P3 — Preserve DHCP leasefail/accepted-NAK/release state semantics and BusyBox callback status handling; keep FULLCONENAT mapping-hash and dying-tuple queue namespace ownership as a separate kernel audit.

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
| L1 source review | PASS for the removable and internal MTD/UBI startup caller contexts, shared lifecycle lock/supervisor integration, and RWFS shutdown fail-closed gates. | Deferred child result/retry semantics, NFS export ACL/permissions, external mounts and target-specific behavior remain open. |
| L2 host / Actions | On base SHA `d225e9e1e09e56b79c6b00b46e61d2e0a08b6166`, storage [42609330908](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609330908) passed all syntax/runtime/GNU89 checks and failed only at the RWFS source-contract search offset; docs [42609330808](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609330808) SUCCESS. THIS assertion-only correction awaits exact-head storage/docs Actions. No full local checkout or production build. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

