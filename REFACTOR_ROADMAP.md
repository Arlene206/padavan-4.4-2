## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Base main SHA for THIS MAN DNS conditional correction: `ca94b95812ba8eace9162279b268a54184e9f7bc`; active branch `main`; only `main` exists, no remote checkpoint branch.
- The preceding WAN/MAN route teardown status implementation on `ca94b95812ba8eace9162279b268a54184e9f7bc` passed its exact-head WAN route [42697380249], WAN teardown [42697380232], HTTPD [42697380274], WAN DHCP [42697380012], and AI docs [42697379978] Actions. All completed SUCCESS.
- Source review found that the new MAN DNS status aggregation was not braced with the existing multi-condition predicate: in non-matching DNS iterations, it could aggregate/log stale `route_status` despite not performing a route delete. THIS correction encloses the delete, status aggregation and error log within the predicate block and updates the route source contract to verify that structure.
- The MAN off-subnet gateway route identity fix and WAN/MAN route-delete status aggregation remain; BusyBox still discards callback exit codes, so event logs are required.
- Next P2 focus after validation: audit route-add error propagation / rollback in `add_man_gateway_routes()`, `wan_up()` and `man_up()`; then move coverage beyond WAN into Samba and other userland reliability domains.
- Root `dev_log.md` will contain 76 /119 records after THIS entry; no archive rotation is due.
## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease validation and route-text safety; checked interface apply/rollback; MAN off-subnet gateway route deletion retains old lease identity. MAN/WAN route-delete/helper results are accumulated and returned/logged without short-circuiting; THIS correction braces the MAN DNS delete/status/log operations under the full input predicate and adds a source assertion. | Parent exact-head Actions all green; THIS corrective SHA pending inspection; route-add failures/rollback in MAN/WAN up paths; BusyBox ignores callback status; broader callback semantics remain open. |
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

1. P2 — Reconcile the parent WAN/MAN route teardown status Actions and THIS DNS-conditional correction on the exact new main SHA; require all applicable WAN/HTTPD/docs checks to pass.
2. P2 — Audit ignored route-add results and partial-install rollback in `add_man_gateway_routes()`, `wan_up()` and `man_up()`.
3. P2 — Finish the bounded BusyBox leasefail/NAK/renew/SIGUSR2 callback-state review without changing ignored callback-exit semantics absent a demonstrated defect.
4. P2 — Reassess Samba same-boot passdb/config publication and the next whole-userland reliability/security items; retain FULLCONENAT kernel/MIPS build and FTP realpath-to-syscall TOCTOU in the queue.
5. P2 — Keep NFS async durability, AUTH_SYS/root-squash and external mounts as separate evidence-backed compatibility decisions.
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
