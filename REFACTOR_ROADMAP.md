# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2 (maintained fork).
- Latest source/test checkpoint: THIS COMMIT — stage the entire generated Samba configuration, check stream/flush/close and share-inventory reads, atomically publish, and gate WINS startup plus nmbd/smbd reload. Base main SHA `c51c0f73fa2dccdd191515347acf80fd2a0132d2`; new Samba exact-head Actions are pending.
- Previous exact-head runs: vsftpd [42586171489](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171489), storage [42586171713](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171713), HTTPD [42586171537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171537) and AI handoff docs [42586171552](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171552) all PASS on exact SHA `c51c0f73fa2dccdd191515347acf80fd2a0132d2`.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, deferred-start retry, Optware lifecycle serialization and fail-closed teardown error propagation. The storage Actions now include source contracts for SD/MMC mdev → automount, fd8 mountpoint-lock ownership, Optware bind/swap, procfs swap/mount staging, safe-eject/global cleanup, spindown and RC entry points. | No production object/runtime filesystem test; arbitrary external init scripts, target filesystem behavior and hardware remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged and atomically published; share-folder inventory/read errors fail closed; WINS startup and nmbd/smbd reload are gated on successful configuration publication. Focused GNU89 host/source regression added. | Passdb/account mutation consistency, ACL/permission mapping, credential security, live Samba reload behavior and target runtime. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | AUDITING | NFS mount inventory and selected external-script lifecycle checks. | Permissions, command injection, service failures, mount lifecycle and arbitrary external init ordering. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. | Cross-module ordering, shutdown races and package supervision. |

## Priority queue

1. P2 — After exact-head Samba checks pass, rotate to NFS mount/permission lifecycle and third-party add-on service supervision; trace full caller and init order before selecting one bounded P2 bug.
2. P2 — Continue the separate Samba account/passdb mutation and ACL/permission audit; do not equate atomic config publication with complete service security.
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
| L1 source review | PASS for SMB staged/atomic config generation, fail-closed folder inventory, WINS startup and nmbd/smbd reload gates; PASS for vsftpd staged config, daemon-start gate and corrected error return, plus bounded DHCP/storage reviews. |
| L2 host / Actions | vsftpd [42586171489](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171489), storage [42586171713](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171713), HTTPD [42586171537](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171537) and docs [42586171552](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42586171552) PASS on exact SHA `c51c0f73fa2dccdd191515347acf80fd2a0132d2`. New Samba helper/test compiled and passed locally under GNU89 `-Wall -Wextra -Werror`; its workflow and exact-head run are pending. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full build phase required. |
| L4 live router / DHCP churn / storage hotplug / physical hardware | NOT VERIFIED. | Requires device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

