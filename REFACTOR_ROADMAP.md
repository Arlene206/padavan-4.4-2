## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Base main SHA for THIS downstream source-contract correction: `5718821e4b566fea1856945c322899b5145c995c`; active branch `main`; remote branch inventory contains only `main`, with no checkpoint branch.
- Exact-head evidence on `5718821e4b566fea1856945c322899b5145c995c`: WAN DHCP callback [42694649038], WAN interface teardown [42694649527] and AI handoff docs [42694649648] SUCCESS. WAN route status [42694649350] and HTTPD security [42694649091] failed only after reaching Python semantic source checks; both still extracted the public `man_down()` wrapper and therefore missed the moved `man_down_internal()` implementation. Route-status sanitizer host compilation/execution succeeded; HTTPD's earlier host policy and preceding source assertions succeeded. Failure logs were inspected.
- THIS batch repairs both downstream workflow contracts to inspect the internal teardown body and verify off-subnet host-route deletion before address invalidation, while also checking the public wrapper delegation. No production code changes are included in this correction.
- Production fix under test: MAN deconfig now retains `wanx_ipaddr` until the old off-subnet gateway host route and DHCP/static MAN routes have been removed; then it clears the stale address before resolver/firewall refresh. Failed interface clear is logged and returned while cleanup continues.
- BusyBox audit confirmed script-exit status is discarded; initial discovery exhaustion invokes `leasefail`; NAK invokes its callback then deconfig for non-REQUESTING states; SIGUSR2 / release-on-quit also invoke deconfig. No BusyBox behavior change is made.
- Remaining WAN work is route-delete status propagation through MAN/WAN teardown; current callers can discard helper errors before lease-state publication.
- NFS custom-export provenance review found no untrusted write path: `/etc` is tmpfs, `/etc/storage` is root-managed persistent MTD content, and backup restore requires authenticated privileged access. Invalid override paths fail closed; no compatibility-safe reason to restrict trusted operator content was demonstrated.
- Root `dev_log.md` will contain 74 /119 records after THIS entry; no archive rotation is due.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation and route-text safety; route operation error propagation in the underlying helpers and selected teardown gates. Candidate address application/rollback and first-lease double-clear-failure edge case passed exact SHA `bf80204bf59e26f74636e9c75ce52c6d5d652bee` [42690696672]. MAN DHCP deconfig now retains old address through off-subnet gateway host-route removal and reports interface-clear failure; THIS source-contract correction updates downstream workflows after the `man_down_internal()` split. | MAN/WAN teardown still discards gateway, DHCP and static route-delete results before state publication; exact-head route/HTTPD workflow reruns for THIS correction pending; BusyBox discards callback exit status; leasefail/NAK/release, PPP/multi-WAN and IPv6 callback lifecycle remain open. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. The internal MTD/UBI path reports actual `opt-mount.sh` status, retries with monotonic exponential backoff, and tracks/cancels its worker process group. Samba and vsftpd stop failures now participate in `stop_stor_apps()`; the safe-removal callers abort before unmount when any required storage service does not stop. | No production RC object/MIPS build; target filesystem and real hardware behavior remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. Namespace/zone identity is stored and enforced for IPv4/IPv6 mappings and dying-tuple GC; notifier lifecycle, host policy and exact-head semantic contracts pass on `bc99b6801c44f55e13dc0214336408f712e0abf8`. | Full kernel/MIPS object compile and broader firewall/NAT audit remain open. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged/atomically published; folder inventory/read errors fail closed; mode-4 write-only ACL generation is covered; credential provisioning is staged before config publication with ordinary-error passdb rollback. Required smbd/nmbd command status and process-presence postconditions are checked. Samba stop status now propagates into storage-removal gates; WINS-only lifecycle paths check nmbd stop/start/reload and smbd reload results. | Same-boot abrupt process termination between the two runtime file publications; live authentication/reload compatibility. Do not add a purported power-loss journal under /etc tmpfs. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated config staging/start/stop/restart lifecycle and source-port/root-mapping policy retained. RNFR/RNTO use canonical source/destination ACLs. Target-path checks cover STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT, SIZE, MDTM and SITE CHMOD; exact sidecar/canonical-target filtering blocks `.__folder_list.txt` and `.__<account>_var.txt`. vsftpd and AI docs Actions [42667988550]/[42667988522] passed on exact SHA `895f2a9ebcb58e810694f76cbf13706ffea9ef75`. | realpath-to-path-syscall TOCTOU, FTP auth/account/anonymous compatibility and MIPS/uClibc / target runtime remain open. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates; exact pseudo-filesystem tuple identity/conflict-safe teardown; strict LAN/VPN IPv4/IPv6 ACL validation and fail-closed publication; generated exports require `secure` source ports. Custom override provenance is now traced to root-managed `/etc/storage` MTD content and authenticated privileged backup restore; invalid symlink/non-regular/inaccessible paths fail closed, and `exportfs -r` failures propagate through startup/reload. No untrusted writer or demonstrated override vulnerability was found. | Root clients still map to server root; AUTH_SYS remains a trusted-network identity model. `async` durability versus throughput, compatibility-backed root-squash migration and explicit external-mount support remain separate compatibility decisions. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Reconcile THIS downstream WAN route-status, HTTPD and AI handoff docs workflow correction against the exact resulting main SHA (along with the green WAN DHCP/teardown jobs).
2. P2 — Continue WAN/MAN teardown route result propagation in `man_down()` and `wan_down()`: attempt all required deletes, preserve their source lease data through cleanup, and surface accumulated failures. Complete the bounded BusyBox leasefail/NAK/release callback audit without changing ignored script-status semantics absent a demonstrated need.
3. P2 — Reassess same-boot Samba passdb/config recovery without proposing persistent journaling under /etc tmpfs.
4. P2 — Keep FULLCONENAT kernel/MIPS build and residual lifecycle work, plus FTP pathname realpath-to-syscall TOCTOU, visible.
5. P2 — Keep NFS `async` durability, AUTH_SYS/root-squash compatibility and external mounts as separate evidence-backed decisions; no default change from this audit.

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
| L1 source review | PASS for the MAN off-subnet gateway host-route dependency and deconfig ordering fix, downstream semantic contracts updated for the internal helper split, WAN DHCP bound callback input checks/address-before-state/rollback and NFS custom-export trust boundary. FULLCONENAT namespace/zone identity and cleanup ordering reviewed. | Wider WAN teardown route status, firewall/NAT, NFS compatibility and other module audits remain open. |
| L2 host / Actions | Exact SHA `5718821e4b566fea1856945c322899b5145c995c`: WAN DHCP [42694649038], WAN teardown [42694649527], AI docs [42694649648] SUCCESS; WAN route [42694649350] and HTTPD [42694649091] failed at stale source-body contracts after refactor. THIS workflow correction and exact-head reruns PENDING. The prior `bf802...` and `b6c1...` evidence remains limited to those exact base SHAs. | No production RC object build, MIPS/uClibc, firmware/TRX, live ioctl failure injection or router test. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 storage eject, live Samba/NFS/NAT/DHCP behavior / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.
