# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are byte-preserved and indexed. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base `main` SHA for THIS custom NFS override fail-closed change: `a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`; resulting commit: THIS COMMIT. Only `main` existed at recovery; no separate checkpoint branch.
- The preceding NFS reserved-source-port correction is green on exact SHA `a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`: storage [42631443686](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443686), NFS [42631443757](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443757), vsftpd [42631443784](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443784), Samba [42631443514](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443514), AI docs [42631443719](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443719), and HTTPD [42631443747](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42631443747); all six completed SUCCESS with matching full `head_sha`.
- THIS commit makes the custom `/etc/storage/exports` override fail closed: only an actually absent path permits generated exports; a valid direct regular file or symlink to a regular file remains authoritative; dangling symlinks, non-regular entries/targets and uncertain access states refuse NFS export generation. Exact-head NFS/HTTPD/storage/docs CI is pending.
- NFS remains NFSv2/v3 AUTH_SYS. Generated exports require `secure` source ports but intentionally retain `no_root_squash` and `async` pending separate compatibility and durability decisions. A valid custom override still bypasses generated LAN/VPN ACL and local-media path restrictions; treat it as trusted root configuration.


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
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged and atomically published; folder inventory/read errors fail closed; service reload is gated on publication. Mode-4 account/guest ACL generation now distinguishes write-only grants from deny; GNU89 rights matrix and HTTPD writer contract passed all six exact-head Actions on `010d5ebec0926cb52686b5c24f145d33a06514d7`. | Passdb/account mutation consistency, credential failure/recovery, live Samba reload behavior and target runtime. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates; exact pseudo-filesystem tuple identity/conflict-safe teardown; strict LAN/VPN IPv4/IPv6 ACL validation and fail-closed publication; generated exports require `secure` source ports; custom override path now accepts only direct regular files or symlinks to regular files and permits generated fallback only on `ENOENT`. | Root clients still map to server root; AUTH_SYS remains a trusted-network identity model. `async` durability versus throughput, compatibility-backed root-squash migration, trusted custom override ACL/path bypass and malformed-content/exportfs recovery, and explicit external-mount support remain open. THIS custom-path fix awaits exact-head CI. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Verify THIS custom `/etc/storage/exports` path fail-closed fix on exact-head NFS/HTTPD/storage/docs Actions.
2. P2 — Rotate to Samba passdb/account mutation consistency, credential failure/recovery and live reload; preserve the already-fixed write-only ACL and atomic config publication behavior.
3. P2 — Continue vsftpd authentication/ACL/path confinement/stop lifecycle. Keep NFS `async` durability, root-squash compatibility, custom override authority/content handling and external mounts as explicit open risks.
4. P2/P3 — DHCP leasefail/accepted-NAK/release callback state semantics and BusyBox callback status; keep FULLCONENAT mapping-hash / dying-tuple queue namespace ownership as a separate kernel audit.

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
| L1 source review | PASS for strict NFS LAN/VPN IPv4/IPv6 ACL validation, Linux 4.4 AUTH_SYS/reserved-port semantics, Samba 0/1/2/3/error permission mapping, and this round's custom NFS override path classification/fail-closed state matrix. |
| L2 host / Actions | Preceding source-port change exact SHA `a753c86b1f6329e4f9ac8e73ccfcb97f0a14865d`: storage [42631443686], NFS [42631443757], vsftpd [42631443784], Samba [42631443514], AI docs [42631443719], and HTTPD [42631443747] all SUCCESS with matching full `head_sha`. THIS custom-path change's focused standalone GNU89 regression PASS; full production policy test and NFS/HTTPD/storage/docs Actions pending exact-head. No local full checkout, production RC object or firmware build is claimed.
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

