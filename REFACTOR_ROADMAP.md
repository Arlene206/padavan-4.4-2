## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only.
- Base `main` SHA for THIS vsftpd regression include-path correction: `a36c2aa14804e9a8eb45f367672d52cf1f306912`; only `main` exists and there is no separate checkpoint branch.
- Parent exact-head AI handoff docs [42660622526] passed. vsftpd [42660622353] failed at the added test compile because `-I trunk/user/vsftpd/src` shadowed libc's `<features.h>` with the project-local header; the test compile now uses `-iquote`, and the helper passed a local shadow-header regression.
- The path-scoped RNFR/RNTO ACL change remains the active security work; source/destination parent ACL mapping, absolute RNFR pinning and source/destination permission checks are already committed on this parent.
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
| FTP service / vsftpd | PARTIAL / AUDITING | Generated config uses checked same-directory staging/atomic replacement; startup/stop/restart status and bounded daemon-presence checks propagate into storage safety gates. RNFR pins an absolute source path and checks canonical source-parent delete ACL; RNTO revalidates source delete and destination write ACL against canonical parents. GNU89 host regression covers traversal and symlinked parents. | Path-aware ACL coverage for STOR/APPE/STOU, RETR, MKD/RMD/DELE and directory listings; direct metadata-file access, authentication/account semantics, anonymous policy and target runtime. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates; exact pseudo-filesystem tuple identity/conflict-safe teardown; strict LAN/VPN IPv4/IPv6 ACL validation and fail-closed publication; generated exports require `secure` source ports; custom override path accepts only direct regular files or symlinks to regular files and permits generated fallback only on `ENOENT`. | Root clients still map to server root; AUTH_SYS remains a trusted-network identity model. `async` durability versus throughput, compatibility-backed root-squash migration, trusted custom override ACL/path bypass and malformed-content/exportfs recovery, and explicit external-mount support remain open. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P1 — Verify exact-head vsftpd and AI handoff Actions for THIS path-scoped rename-ACL fix; inspect full `head_sha`, GNU89 compile/run and source-contract logs.
2. P1 — Complete FTP path-argument ACL audit for STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT and share metadata access. Preserve operation-specific symlink semantics.
3. P2 — Reassess same-boot recovery after abrupt termination during Samba passdb/config publication. `/etc` is tmpfs; persistent power-loss journaling there is not applicable.
4. P2 — Keep NFS `async` durability, root-squash compatibility, trusted custom override content/ACL bypass, and external mounts as explicit risks.
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
| L1 source review | PASS for FTP lifecycle status propagation and the RNFR/RNTO ACL correction, including canonical parent resolution and source/destination permission revalidation. | No live FTP session or hardware behavior claimed. |
| L2 host / Actions | The helper passed a GNU89 warnings-as-errors host run with a source-local `features.h` shadow fixture and quote-only include search. Parent AI docs [42660622526] passed; vsftpd [42660622353] exposed the initial `-I`/libc header collision and is corrected on this head. New exact-head Actions are pending. | No production vsftpd object, MIPS/uClibc build, firmware/TRX or router test claimed. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 storage eject, live Samba/WINS behavior / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.
