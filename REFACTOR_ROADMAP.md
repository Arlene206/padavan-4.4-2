## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base `main` SHA for THIS vsftpd lifecycle correction: `78508d0de29e64952dafe8d5ae2ca30ab6f20f60`; only `main` exists and there is no separate checkpoint branch.
Parent `78508d0de29e64952dafe8d5ae2ca30ab6f20f60` has six exact-head Actions, all completed SUCCESS with matching full `head_sha`: Samba [42652191209], USB storage [42652191230], NFS [42652191208], HTTPD/security [42652191190], vsftpd [42652191151] and AI docs [42652191260]. The Samba workflow compiled/ran config, passdb and daemon-presence tests and passed the full semantic contract; the docs workflow validated the live handoff/roadmap/log integrity.
- The preceding daemon-lifecycle SHA `30432e4636c5b00c96cb8918caaebe37b133a28c` passed Samba, NFS, USB storage, vsftpd and WAN route; its stale HTTPD signature and handoff-heading contracts were repaired on the parent above.
- The preceding Samba/storage patch carries Samba stop failures into safe-removal gates, rejects unknown mount inventory during Samba restart and checks WINS-only nmbd status; all six affected exact-head Actions on the parent above are green.
- THIS change carries vsftpd stop failures into the storage safe-removal gate, checks FTP start command status and bounded presence, and propagates restart/stop results through RC and storage callers. Exact-head Actions for THIS change are pending.
- `dev_init.sh` mounts `/etc` as tmpfs. Samba config/passdb files under `/etc` are volatile and recreated at boot; a persistent power-loss journal in that mount cannot be durable. Same-boot abrupt process termination remains a separate, lower-priority recovery question.
- NFS generated defaults retain `secure,no_root_squash,async`; the custom override path accepts only direct regular files or symlinks to regular files and permits generated fallback only on `ENOENT`. Root-squash, AUTH_SYS trust, custom override content/ACL bypass, durability and external-mount policy remain open.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. The internal MTD/UBI path reports actual `opt-mount.sh` status, retries with monotonic exponential backoff, and tracks/cancels its worker process group. Samba and vsftpd stop failures now participate in `stop_stor_apps()`; the safe-removal callers abort before unmount when any required storage service does not stop. | No production RC object/MIPS build; target filesystem and real hardware behavior remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. | Mapping hashes and dying-tuple queue namespace ownership; separate kernel task. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged/atomically published; folder inventory/read errors fail closed; mode-4 write-only ACL generation is covered; credential provisioning is staged before config publication with ordinary-error passdb rollback. Required smbd/nmbd command status and process-presence postconditions are checked. Samba stop status now propagates into storage-removal gates; WINS-only lifecycle paths check nmbd stop/start/reload and smbd reload results. | Same-boot abrupt process termination between the two runtime file publications; live authentication/reload compatibility. Do not add a purported power-loss journal under /etc tmpfs. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated vsftpd config uses checked same-directory staging/atomic replacement; startup now checks the eval result and bounded daemon-presence postcondition. Stop status propagates into storage safe-removal gates and RC restart/stop callers; restart fails closed on unknown mount inventory. | Authentication/account provisioning, credential/ACL semantics, anonymous write/delete exposure, chroot/path confinement and symlink traversal, plus target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates; exact pseudo-filesystem tuple identity/conflict-safe teardown; strict LAN/VPN IPv4/IPv6 ACL validation and fail-closed publication; generated exports require `secure` source ports; custom override path accepts only direct regular files or symlinks to regular files and permits generated fallback only on `ENOENT`. | Root clients still map to server root; AUTH_SYS remains a trusted-network identity model. `async` durability versus throughput, compatibility-backed root-squash migration, trusted custom override ACL/path bypass and malformed-content/exportfs recovery, and explicit external-mount support remain open. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Verify THIS vsftpd lifecycle correction on exact-head vsftpd, Samba, HTTPD, storage, NFS and AI handoff Actions; inspect each job and full `head_sha`.
2. P2 — Audit bundled vsftpd authentication/account provisioning, share-mode ACL semantics, anonymous write/delete exposure, chroot/path confinement and symlink traversal.
3. P2 — Reassess whether a small same-boot recovery mechanism is justified for abrupt process termination during passdb/config publication. `/etc` is tmpfs, so persistent power-loss journaling is not applicable.
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
| L1 source review | PASS for Samba and FTP stop-result propagation into storage removal, fail-closed tri-state storage gating, WINS-only and vsftpd command results, bounded daemon-presence postconditions and RC status propagation. | No live process/hardware behavior claimed. |
| L2 host / Actions | The parent `78508d0de29e64952dafe8d5ae2ca30ab6f20f60` has six exact-head Actions all SUCCESS with matching full `head_sha`. The FTP GNU89 daemon-presence regression was compiled and passed locally from the exact helper/test content; exact-head Actions for THIS FTP change are pending. | No production RC object, MIPS/uClibc build, firmware/TRX or device test claimed. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 storage eject, live Samba/WINS behavior / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.
