# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Current base main SHA for THIS storage/NFS regression-contract correction: `eb21523750bc3218121f5adf3457c587d8e9a6e0`; resulting commit: THIS COMMIT.
- NFS daemon start/reload [42622521213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521213) and AI handoff docs [42622521280](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521280) both **SUCCESS** on exact `head_sha=eb21523750bc3218121f5adf3457c587d8e9a6e0`.
- USB storage hotplug [42622521225](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521225) **FAILED** on the same exact SHA in the final caller-graph source contract: it still expected numeric NFS mount-state branches after `func_unmount_if_mounted()` changed to a fail-closed `case` statement. This commit aligns that contract without changing production code; exact-head rerun is pending.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. The internal MTD/UBI path now reports actual `opt-mount.sh` exit status, retries failed starts with monotonic exponential backoff (5–300 seconds), and tracks/cancels its worker process group before normal service shutdown. The removable USB path retains its separate nonblocking child-result channel outside `storage.lock`. RWFS unmount/UBI detach remains blocked on teardown or filesystem-unmount failure. | Base `eb21523750bc3218121f5adf3457c587d8e9a6e0` storage run failed only at the final cross-workflow NFS mount-state source assertion; all preceding shell/GNU89 steps passed. This commit updates that stale assertion; exact-head storage/docs reruns pending. No production RC object/MIPS build; target filesystem and real hardware behavior remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged and atomically published; folder inventory/read errors fail closed; WINS/nmbd/smbd lifecycle is gated on publication. Samba GNU89 helper regression passes; HTTPD's dependent trust-boundary contract now bounds its folder slice to the full writer instead of an early error cleanup; exact-head rerun pending. | Passdb/account mutation consistency, ACL/permission mapping, credential security, live Samba reload behavior and target runtime. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates have exact-head source/CI evidence. Optware startup helpers provide pathname-safe enumeration, fd-9 isolation and rollback for removable and internal MTD/UBI startup; RWFS shutdown honors teardown failure. IPv6 ACL input is validated/canonicalized and the two NFS pseudo-filesystems now require exact source/mountpoint/type identity with conflict-safe teardown. NFS start/reload workflow passed on exact SHA `eb21523750bc3218121f5adf3457c587d8e9a6e0`. | Root UID/GID mapping and historic `async,insecure,no_root_squash` need compatibility/security review; custom exports semantics and external-mount support remain open. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Verify the corrected USB storage caller-graph/NFS unmount source contract and AI handoff-doc/archive validation on the exact SHA; the base storage workflow failed at a stale numeric-state assertion while NFS and docs Actions passed.
2. P2 — Complete NFS permission/security audit: `no_root_squash`, AUTH_SYS UID/GID mapping, `async` durability, `insecure` client source ports, custom exports override semantics and external-mount support. Exact pseudo-filesystem mount tuple matching is implemented and its dedicated NFS workflow is green.
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
| L1 source review | PASS for removable/internal MTD/UBI lifecycle lock/supervisor integration, RWFS shutdown gates, MTD bounded retry/backoff, worker signal isolation, and exact NFS pseudo-filesystem source/mountpoint/type matching with conflict-safe teardown. NFS IPv6 ACL input is also validated and canonicalized before export publication. | NFS root UID/GID semantics, custom exports overrides, external mounts, check-to-mount races and target-specific behavior remain open. |
| L2 host / Actions | NFS start/reload run [42622521213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521213) and docs run [42622521280](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521280) PASS on exact SHA `eb21523750bc3218121f5adf3457c587d8e9a6e0`. Storage run [42622521225](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521225) FAILED only at the final stale caller-graph assertion; shell syntax and preceding GNU89/shell tests passed. This commit updates the cross-workflow contract; rerun pending. No production object or full local firmware build is claimed. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

