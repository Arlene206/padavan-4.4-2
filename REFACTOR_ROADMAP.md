# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Current base main SHA for THIS NFS policy-header preprocessor correction: `ef1aec7b10389eaf240c20b5e999d80ef48996d1`; resulting commit: THIS COMMIT.
- The prior NFS IPv4 LAN/VPN ACL change is committed in `ef1aec7b10389eaf240c20b5e999d80ef48996d1`. Exact-head storage [42624781463](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781463), Samba [42624781449](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781449), vsftpd [42624781407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781407) and AI docs [42624781292](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781292) passed.
- NFS [42624781415](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781415) and HTTPD [42624781560](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781560) failed while compiling the shared policy header: an extra nested `#if defined (USE_IPV6)` left the include guard unterminated. NFS shell/lifecycle and all preceding HTTPD steps passed. THIS commit removes only that redundant guard; exact-head reruns are pending.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. The internal MTD/UBI path now reports actual `opt-mount.sh` exit status, retries failed starts with monotonic exponential backoff (5–300 seconds), and tracks/cancels its worker process group before normal service shutdown. The removable USB path retains its separate nonblocking child-result channel outside `storage.lock`. RWFS unmount/UBI detach remains blocked on teardown or filesystem-unmount failure. | The stale cross-workflow NFS unmount assertion was corrected and USB storage/AI docs Actions passed on exact SHA `3c879e06efa256ffbf376848ad69359a9098330e`. No production RC object/MIPS build; target filesystem and real hardware behavior remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged and atomically published; folder inventory/read errors fail closed; WINS/nmbd/smbd lifecycle is gated on publication. Samba GNU89 helper regression passes; HTTPD's dependent trust-boundary contract now bounds its folder slice to the full writer instead of an early error cleanup; exact-head rerun pending. | Passdb/account mutation consistency, ACL/permission mapping, credential security, live Samba reload behavior and target runtime. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates; exact pseudo-filesystem tuple identity and conflict-safe teardown. NFS LAN/VPN IPv4 ACL generation now has a strict contiguous-mask formatter, /0 rejection, canonicalization and fail-closed publication on invalid state; the first exact-head host compile exposed a duplicate preprocessor guard, corrected in THIS commit. | AUTH_SYS UID/GID mapping and historic `async,insecure,no_root_squash` need compatibility review; custom exports semantics and external-mount support remain open. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Verify the corrected NFS policy header compiles in both NFS and HTTPD shared workflows, then reconcile exact-head storage/Samba/vsftpd/docs Actions; preserve the strict LAN/VPN IPv4 CIDR validation and fail-closed behavior.
2. P2 — Continue NFS option/identity audit: `no_root_squash`, AUTH_SYS UID/GID mapping, `async` durability, `insecure` client source ports, custom exports override semantics and external-mount support. Existing export options remain unchanged pending compatibility review.
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
| L1 source review | PASS for exact NFS pseudo-filesystem tuple identity/conflict-safe teardown and strict IPv4 LAN/VPN ACL canonicalization, contiguous netmask validation, /0 rejection and fail-closed exports publication; duplicate conditional guard identified and removed in THIS commit. |
| L2 host / Actions | On exact SHA `ef1aec7b10389eaf240c20b5e999d80ef48996d1`: storage [42624781463](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781463), Samba [42624781449](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781449), vsftpd [42624781407](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781407) and docs [42624781292](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781292) PASS. NFS [42624781415](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781415) and HTTPD [42624781560](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42624781560) FAILED compiling the shared policy header due to an unmatched duplicate `USE_IPV6` guard. This commit corrects that preprocessing issue; affected reruns pending. No production object or full local firmware build is claimed. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

