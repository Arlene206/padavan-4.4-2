# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Base main SHA for THIS correction: `7005e6c48835b928d596f4a52f79ca9168a52994`; resulting commit: THIS COMMIT. Remote branch inventory at takeover contained only `main`, with no separate checkpoint.
- Prior exact-head NFS [42598774070](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598774070), storage [42598773937](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598773937) and AI docs [42598774002](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598774002) passed on `8e818c475d3aa26fb4d92e255e25405c11f2a08f`. First Optware validation on `7005e6c`: storage [42600941876](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42600941876) failed at the new startup fixture; docs [42600941917](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42600941917) failed because the handoff omitted the validator's exact `## Active task` heading. This correction fixes the regression's printf newline escapes and restores the required heading.
- Optware startup-hook failure handling and reverse-order rollback are implemented. Corrected exact-head storage/docs workflows are pending; do not treat the failed 7005e6 runs as green.

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
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged and atomically published; folder inventory/read errors fail closed; WINS/nmbd/smbd lifecycle is gated on publication. Samba GNU89 helper regression passes; HTTPD's dependent trust-boundary contract now bounds its folder slice to the full writer instead of an early error cleanup; exact-head rerun pending. | Passdb/account mutation consistency, ACL/permission mapping, credential security, live Samba reload behavior and target runtime. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates have exact-head source/CI evidence. THIS change adds Optware `opt-start.sh` hook enumeration, fd-9 isolation, failure propagation and reverse-order partial-start rollback with a host shell regression; exact-head storage/docs Actions pending. | NFS export ACL/permissions and external mounts; the separate `opt-mount.sh` MTD/UBI startup path still has a duplicate unguarded hook loop; target filesystem and arbitrary third-party hooks remain unverified. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. | Cross-module ordering, shutdown races and package supervision. |

## Priority queue

1. P2 — Reconcile THIS Optware startup-helper regression and AI handoff-doc Actions against the new exact full main SHA; inspect all runtime/source-contract steps.
2. P2 — Audit the duplicate `opt-mount.sh` internal MTD/UBI service-start path; align it with the shared supervisor only after tracing its caller/lock context and retry contract.
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
| L1 source review | PASS for NFS start/reload/shutdown lifecycle and Optware startup-hook order, failure propagation, fd-9 closure and reverse rollback. | NFS export ACL/permissions, external mounts, Optware MTD/UBI duplicate start path and target-specific behavior remain open. |
| L2 host / Actions | Corrected Optware startup shell regression and `sh -n` PASS in host scratch. First exact-head attempt `7005e6c` had storage [42600941876](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42600941876) fail at the new fixture (newline escapes corrected here) and docs [42600941917](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42600941917) fail the required heading contract (corrected here). This correction's exact-head Actions pending. Prior exact base SHA `8e818c475d3aa26fb4d92e255e25405c11f2a08f`: NFS [42598774070](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598774070), storage [42598773937](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598773937), docs [42598774002](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42598774002) SUCCESS. | No full local checkout or production build. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

