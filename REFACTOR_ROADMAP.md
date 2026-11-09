# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base main SHA for THIS MTD boot-time Optware result/retry fix: `19c1627490ec550b74b2deb3e19e8965d4374406`; resulting commit: THIS COMMIT. Only remote branch is `main`.
- The deferred removable-USB Optware result/retry implementation is green on exact SHA `19c1627490ec550b74b2deb3e19e8965d4374406`: storage [42613569550](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569550) SUCCESS (all 11 steps); AI handoff docs [42613569474](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42613569474) SUCCESS.
- Samba [42612494997](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494997), vsftpd [42612494935](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494935), NFS [42612494823](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494823), and HTTPD [42612494815](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42612494815) were also SUCCESS on the immediately preceding production SHA `a154f30da5d0eac2d3923d221c39eeb8295965f2`.
- THIS commit closes the separate MTD boot result gap: child exit is reported through a nonblocking pipe; PID1's alarm loop polls and retries without running the Optware hook synchronously. Exact-head storage/docs Actions are pending at commit creation.


## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. Commit `b13ca6b` extends the lifecycle lock and shared supervisor to the internal MTD/UBI `opt-mount.sh` path and prevents RWFS unmount/UBI detach when its Optware stop hook or real unmount fails. THIS commit adds tri-state deferred inventory, one-shot integrated-service startup, and an asynchronous Optware child-result/retry channel outside `storage.lock`. | Boot-time MTD startup remains asynchronous to its C caller. No production object/runtime filesystem test; external init scripts, target filesystem behavior and hardware remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged and atomically published; folder inventory/read errors fail closed; WINS/nmbd/smbd lifecycle is gated on publication. Samba GNU89 helper regression passes; HTTPD's dependent trust-boundary contract now bounds its folder slice to the full writer instead of an early error cleanup; exact-head rerun pending. | Passdb/account mutation consistency, ACL/permission mapping, credential security, live Samba reload behavior and target runtime. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates have exact-head source/CI evidence. Optware startup helpers now provide pathname-safe enumeration, fd-9 isolation and rollback for both removable deferred startup and internal MTD/UBI startup; RWFS shutdown honors Optware teardown failure. THIS commit adds an asynchronous removable-USB child-result/retry path that keeps external hooks outside `storage.lock`. | NFS export ACL/permissions and external mounts; the boot caller's asynchronous MTD startup result remains open; target filesystem and arbitrary third-party hooks remain unverified. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. | Cross-module ordering, shutdown races and package supervision. |

## Priority queue

1. P2 — Verify THIS MTD boot-time Optware result/retry worker on exact-head storage and AI handoff-doc Actions; inspect PID1 alarm integration, pipe ownership, and source contracts.
2. P2 — Audit the asynchronous boot-time `start_rwfs_optware()` → `opt-mount.sh ... &` result path in `common_ex.c`; decide on safe status/retry handling without holding `storage.lock`.
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
| L1 source review | PASS for removable/internal MTD/UBI lifecycle lock/supervisor integration, RWFS shutdown gates, tri-state deferred mount inventory, one-shot integrated-service startup, and asynchronous Optware child result delivery outside `storage.lock`. | Boot-time MTD result/retry, NFS export ACL/permissions, external mounts and target-specific behavior remain open. |
| L2 host / Actions | The preceding MTD/RWFS work passed storage [42609876845](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876845) and docs [42609876722](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42609876722) on exact SHA `6ae43a6a71e05bf5aeba47516b1c4b193c75075e`. THIS deferred USB fix has a local GNU89 result-policy regression PASS; its exact-head storage/docs Actions are pending at commit creation. No full local checkout or production build. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

