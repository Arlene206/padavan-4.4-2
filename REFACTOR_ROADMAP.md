# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base main SHA for THIS NFS IPv6 export ACL hardening: `5faded68510e91fb526dea6ab3ebd34b5d9dc252`; resulting commit: THIS COMMIT.
- The preceding MTD retry/shutdown commit `5faded68510e91fb526dea6ab3ebd34b5d9dc252` passed USB storage [42618640557](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640557), AI handoff docs [42618640385](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640385), Samba [42618640482](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640482), WAN route [42618640381](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640381), and HTTPD [42618640379](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640379). All full run `head_sha` values matched that exact SHA.
- THIS commit validates and canonicalizes IPv6 LAN client ACL data before NFS export publication, rejects malformed non-empty values without replacing the previous exports file, and adds a GNU89 IPv6 policy regression plus production source contracts. Exact-head Actions are pending at commit creation.

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
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates have exact-head source/CI evidence. Optware startup helpers provide pathname-safe enumeration, fd-9 isolation and rollback for removable and internal MTD/UBI startup; RWFS shutdown honors teardown failure. This commit validates/canonicalizes IPv6 LAN ACL input before writing NFS exports and fails closed without replacing the old exports file on malformed non-empty values. | Exact-head NFS GNU89/source contracts pending. Root UID/GID mapping and the hardcoded `async,insecure,no_root_squash` policy still need compatibility/security review; external-mount export behavior and exact mountpoint matching remain open. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Verify THIS commit's exact-head NFS export/start, USB storage, HTTPD and AI handoff-doc Actions, including the GNU89 IPv6 ACL policy test and whole-writer source contracts.
2. P2 — Complete NFS permission/security audit: `no_root_squash`, AUTH_SYS UID/GID mapping, `async` durability, `insecure` client source ports, custom exports override semantics and external-mount support. Review exact mountpoint parsing/matching in `nfsd.sh`.
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
| L1 source review | PASS for removable/internal MTD/UBI lifecycle lock/supervisor integration, RWFS shutdown gates, removable-storage child result delivery, MTD bounded retry/backoff, worker signal isolation and shutdown process-group ordering. NFS IPv6 ACL input is validated with inet_pton, canonicalized with inet_ntop, restricted to the configured /48–/64 range and rejected before publishing a malformed exports file. | NFS root UID/GID semantics, external mounts, custom exports overrides, exact mountpoint matching and target-specific behavior remain open. |
| L2 host / Actions | The preceding MTD checkpoint passed USB storage [42618640557](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640557), docs [42618640385](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640385), Samba [42618640482](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640482), WAN [42618640381](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640381) and HTTPD [42618640379](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42618640379) on exact SHA `5faded68510e91fb526dea6ab3ebd34b5d9dc252`. THIS commit's GNU89 IPv6 policy compile/run, NFS writer/source contracts, start/reload shell regression and docs validator are PENDING exact-head Actions; no production object or full local firmware build is claimed. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

