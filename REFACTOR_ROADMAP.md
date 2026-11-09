## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Current `main` before this CI-contract follow-up: `30432e4636c5b00c96cb8918caaebe37b133a28c`; only `main` exists and there is no separate checkpoint branch.
- Parent Samba passdb/config consistency commit is green on exact SHA `cf75016ebf1f1150cd4db39308ead1e09bb0e66e`: Samba [42638624538], NFS [42638624367], HTTPD/security [42638624464], USB storage [42638624595], vsftpd [42638624417], and AI docs [42638624607]. All six completed SUCCESS with matching full `head_sha`.
- Current daemon-lifecycle commit `30432e4636c5b00c96cb8918caaebe37b133a28c` has five exact-head passes (Samba [42647280352], NFS [42647280434], USB [42647280185], vsftpd [42647280155], WAN route [42647280379]); HTTPD [42647280187] and AI docs [42647282663] failed because their regression expectations were stale after the int signature / handoff heading updates. THIS follow-up fixes those test contracts; recheck the new exact SHA.
- THIS change checks return values for Samba `smbd`/`nmbd` HUP/start commands, requires a stable PID postcondition, propagates failures through `run_samba()`/`restart_smbd()` and RC notification/command callers, and reports optional WSDD2 failures separately.
- The parent passdb/config update still lacks power-failure atomicity: a reset between publishing the passdb and `smb.conf` may leave a mismatched pair until recovery. No journal recovery or cross-file crash atomicity is claimed.
- NFS generated defaults retain `secure,no_root_squash,async`; the custom override path accepts only direct regular files or symlinks to regular files and permits generated fallback only on `ENOENT`. Root-squash, AUTH_SYS trust, custom override content/ACL bypass, durability and external-mount policy remain open.


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
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged/atomically published; folder inventory/read errors fail closed; mode-4 write-only ACL generation is covered; credential provisioning completes in a private staged passdb before smb.conf publication, with hard-link snapshot/rollback on ordinary config errors. Current patch checks smbd/nmbd HUP/start status, stable PID postconditions and propagates failures through RC callers; optional WSDD2 failures are reported separately. | Power-loss journal/recovery for the passdb/config pair; WINS-only nmbd startup status; stop/restart lifecycle status; broader account mutation and live-auth compatibility. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement and daemon startup is gated on publication success. | Authentication, credential/ACL semantics, path confinement, stop/lifecycle error propagation and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates; exact pseudo-filesystem tuple identity/conflict-safe teardown; strict LAN/VPN IPv4/IPv6 ACL validation and fail-closed publication; generated exports require `secure` source ports; custom override path now accepts only direct regular files or symlinks to regular files and permits generated fallback only on `ENOENT`. | Root clients still map to server root; AUTH_SYS remains a trusted-network identity model. `async` durability versus throughput, compatibility-backed root-squash migration, trusted custom override ACL/path bypass and malformed-content/exportfs recovery, and explicit external-mount support remain open. THIS custom-path fix awaits exact-head CI. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Complete durable Samba passdb/config transaction recovery: journal the old pair before publishing either file, recover PREPARED transactions on every service entry, retain COMMITTED pairs, and fail closed on malformed or unsafe journal state. Then close WINS-only nmbd start/stop result handling.
2. P2 — Verify THIS Samba daemon-lifecycle change on exact-head Samba and HTTPD Actions; confirm each full `head_sha` and inspect logs.
3. P2 — Continue vsftpd authentication/ACL/path confinement/stop lifecycle.
4. P2 — Keep NFS `async` durability, root-squash compatibility, trusted custom override content/ACL bypass, and external mounts as explicit open risks.
5. P2/P3 — DHCP leasefail/accepted-NAK/release callback state semantics and BusyBox callback status; keep FULLCONENAT mapping-hash / dying-tuple queue namespace ownership as a separate kernel audit.

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
| L1 source review | PASS for strict NFS LAN/VPN IPv4/IPv6 ACL validation, Linux 4.4 AUTH_SYS/reserved-port semantics, Samba 0/1/2/3/error permission mapping, NFS override fail-closed classification, parent Samba passdb/config ordinary-error rollback, and current Samba daemon command status / stable-PID / RC propagation paths. |
| L2 host / Actions | Parent SHA `cf75016ebf1f1150cd4db39308ead1e09bb0e66e` exact-head Samba [42638624538], NFS [42638624367], HTTPD [42638624464], storage [42638624595], vsftpd [42638624417] and AI docs [42638624607] all SUCCESS with matching full `head_sha`. The daemon-lifecycle SHA `30432e4636c5b00c96cb8918caaebe37b133a28c` had five passes and two workflow-contract failures (HTTPD signature check and required handoff headings); this follow-up fixes both. New exact-head Actions remain pending. No full local checkout, production RC object, firmware build or device test is claimed.
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 live NFS export/stop, USB eject, filesystem integration / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.

