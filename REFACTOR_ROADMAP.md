# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base main SHA for THIS MTD retry/shutdown lifecycle fix: `652581beb0b71787096a1a74a90a3f6c32ce7cd6`; resulting commit: THIS COMMIT. Only remote branch at recovery was `main`.
- The preceding exact-head checkpoint `652581beb0b71787096a1a74a90a3f6c32ce7cd6` passed USB storage [42615347517](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347517) and AI handoff docs [42615347722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347722). Both runs' full `head_sha` matched the base SHA.
- THIS commit adds exponential 5–300 second MTD Optware retry backoff, resets inherited signal handling in the forked worker, tracks its private process group, and cancels that group before shutdown begins stopping ordinary services. Exact-head storage/docs Actions are pending at commit creation.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. The internal MTD/UBI path now reports actual `opt-mount.sh` exit status, retries failed starts with monotonic exponential backoff (5–300 seconds), and tracks/cancels its worker process group before normal service shutdown. The removable USB path retains its separate nonblocking child-result channel outside `storage.lock`. RWFS unmount/UBI detach remains blocked on teardown or filesystem-unmount failure. | Exact-head storage/docs Actions for THIS commit pending. No production RC object/MIPS build; target filesystem, third-party daemon escape from the worker process group and real hardware behavior remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged and atomically published; folder inventory/read errors fail closed; WINS/nmbd/smbd lifecycle is gated on publication. Samba GNU89 helper regression passes; HTTPD's dependent trust-boundary contract now bounds its folder slice to the full writer instead of an early error cleanup; exact-head rerun pending. | Passdb/account mutation consistency, ACL/permission mapping, credential security, live Samba reload behavior and target runtime. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates have exact-head source/CI evidence. Optware startup helpers provide pathname-safe enumeration, fd-9 isolation and rollback for removable and internal MTD/UBI startup; RWFS shutdown honors teardown failure. | NFS export ACL/permission mapping and external mount lifecycle remain the next P2 audit. Arbitrary third-party hooks and target filesystem behavior remain unverified. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Verify THIS commit's exact-head USB storage and AI handoff-doc Actions for MTD retry backoff, worker signal isolation and shutdown cancellation. Resolve any failure before new implementation.
2. P2 — NFS export ACL/permission mapping and external mount lifecycle; trace `nfsd.sh`, export generation, RC callers, storage-eject and rollback paths.
3. P2 — Samba passdb/account mutation, ACL/permission and credential security; vsftpd authentication/ACL/path confinement/stop lifecycle.
4. P2/P3 — DHCP leasefail/accepted-NAK/release callback state semantics and BusyBox callback status handling; keep FULLCONENAT mapping-hash / dying-tuple queue namespace ownership as a separate kernel audit.


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
| L1 source review | PASS for removable/internal MTD/UBI lifecycle lock/supervisor integration, RWFS shutdown gates, removable-storage child result delivery, MTD bounded retry state, signal isolation, process-group shutdown ordering and fail-closed teardown. | NFS export ACL/permissions, external mounts, process-group escape behavior and target-specific filesystem behavior remain open. |
| L2 host / Actions | The immediately preceding caller-contract commit passed USB storage [42615347517](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347517) and docs [42615347722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42615347722) on exact SHA `652581beb0b71787096a1a74a90a3f6c32ce7cd6`. THIS commit's GNU89 retry-policy test, full storage caller-graph contract and AI handoff-doc/archive validation are PENDING exact-head Actions at commit creation; no local full checkout or production build is claimed. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

