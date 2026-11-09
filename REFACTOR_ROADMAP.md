## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Base main SHA for THIS FULLCONENAT call-site and workflow-contract correction: e0dcc7dbc3a85d5f6b1115e63e8f7b75a0e8e08d; active branch main; only main exists.
- FULLCONENAT namespace/zone implementation at e0dcc7dbc3a85d5f6b1115e63e8f7b75a0e8e08d adds struct net * plus conntrack zone ID/flags/direction to IPv4/IPv6 mappings and pending destroy tuples; lookup and per-net teardown are namespace-scoped. Exact-head notifier lifecycle contract and GNU89 host policy regression passed in [42674430844], but its final source-contract step failed because the new struct regex was over-escaped. L1 review also found three missed IPv4 check_mapping() call sites with obsolete argument forms; THIS correction fixes those calls and strengthens the workflow assertions. Exact-head rerun pending.
- AI handoff docs run [42674431213] reports failure at exact SHA e0dcc7dbc3a85d5f6b1115e63e8f7b75a0e8e08d, but the Actions API returned no jobs/check runs for that run. Cause unclassified; verify the next exact-head docs run rather than assuming.
- Prior FTP/sidecar ACL batch is closed at exact SHA 895f2a9ebcb58e810694f76cbf13706ffea9ef75: vsftpd [42667988550] and AI handoff docs [42667988522] both SUCCESS with exact matching head_sha.
- dev_init.sh mounts /etc as tmpfs. Samba config/passdb files under /etc are volatile and recreated at boot; persistent power-loss journaling there is not durable.
- NFS generated defaults remain async,secure,no_root_squash,no_subtree_check. Custom override provenance/permission/content trust, AUTH_SYS/root-squash compatibility, durability and external-mount policy remain open.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, route operation error propagation and selected WAN teardown gates. | Broader callback, multi-WAN and route reconciliation audit. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. The internal MTD/UBI path reports actual `opt-mount.sh` status, retries with monotonic exponential backoff, and tracks/cancels its worker process group. Samba and vsftpd stop failures now participate in `stop_stor_apps()`; the safe-removal callers abort before unmount when any required storage service does not stop. | No production RC object/MIPS build; target filesystem and real hardware behavior remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. Namespace/zone identity is stored and enforced for IPv4/IPv6 mappings and dying-tuple GC; GNU89 host policy test, notifier lifecycle and IPv4 target call-site contracts passed. The semantic contract then exposed its scanner's inability to handle duplicate #if/#else signatures; THIS batch removes five now-identical wrappers without changing runtime behavior. | Exact-head semantic/docs Actions and actual kernel/MIPS compilation remain pending; broader firewall/NAT lifecycle review stays open. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf is staged/atomically published; folder inventory/read errors fail closed; mode-4 write-only ACL generation is covered; credential provisioning is staged before config publication with ordinary-error passdb rollback. Required smbd/nmbd command status and process-presence postconditions are checked. Samba stop status now propagates into storage-removal gates; WINS-only lifecycle paths check nmbd stop/start/reload and smbd reload results. | Same-boot abrupt process termination between the two runtime file publications; live authentication/reload compatibility. Do not add a purported power-loss journal under /etc tmpfs. |
| FTP service / vsftpd | PARTIAL / AUDITING | Generated config staging/start/stop/restart lifecycle and source-port/root-mapping policy retained. RNFR/RNTO use canonical source/destination ACLs. Target-path checks cover STOR/APPE/STOU, RETR, MKD/RMD/DELE, LIST/NLST/STAT, SIZE, MDTM and SITE CHMOD; exact sidecar/canonical-target filtering blocks `.__folder_list.txt` and `.__<account>_var.txt`. vsftpd and AI docs Actions [42667988550]/[42667988522] passed on exact SHA `895f2a9ebcb58e810694f76cbf13706ffea9ef75`. | realpath-to-path-syscall TOCTOU, FTP auth/account/anonymous compatibility and MIPS/uClibc / target runtime remain open. |
| NFS / third-party add-ons | PARTIAL / AUDITING | NFS inventory/restart preflight, checked start/reload/shutdown and storage-eject gates; exact pseudo-filesystem tuple identity/conflict-safe teardown; strict LAN/VPN IPv4/IPv6 ACL validation and fail-closed publication; generated exports require `secure` source ports; custom override path accepts only direct regular files or symlinks to regular files and permits generated fallback only on `ENOENT`. | Root clients still map to server root; AUTH_SYS remains a trusted-network identity model. `async` durability versus throughput, compatibility-backed root-squash migration, trusted custom override ACL/path bypass and malformed-content/exportfs recovery, and explicit external-mount support remain open. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P1/P2 — Complete the FULLCONENAT namespace/zone correction: verify exact-head Actions and AI handoff docs after removing the redundant conditional wrappers that blocked semantic function extraction; actual kernel-object/MIPS compilation remains unverified.
2. P1/P2 — Trace NFS custom exports writer/trust boundary, /etc/storage ownership/modes and exportfs recovery before deciding whether custom override validation is a real exposure. Keep async durability, AUTH_SYS/root-squash compatibility and external mounts separate.
3. P2 — Compare DHCP leasefail/accepted-NAK/release callback state semantics and BusyBox callback-status handling; prioritize reproducible stale-state or retry failures.
4. P2 — Reassess same-boot recovery after abrupt termination during Samba passdb/config publication. /etc is tmpfs; persistent power-loss journaling there is not applicable.
5. P2 — Keep FULLCONENAT tuple namespace/zone lifecycle residuals and FTP pathname realpath-to-syscall TOCTOU visible; avoid reopening completed FTP ACL work without new evidence.

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
| L1 source review | PASS for FULLCONENAT namespace/zone identity propagation, namespace-aware lookup/GC and teardown ordering; THIS batch corrects all three stale IPv4 helper call sites. | Exact-head source contracts pending; broader firewall/NAT audit is not closed. |
| L2 host / Actions | Base exact-head FULLCONENAT [42674430844]: notifier lifecycle and GNU89 namespace/zone policy compile/run PASS; semantic contract failed on over-escaped struct regex. Follow-up [42677259891]: notifier and host policy PASS; semantic contract passed struct/IPv4 call-site checks, then failed because the scanner counted duplicate conditional signature braces. THIS cleanup removes the five identical wrappers; exact-head rerun pending. AI docs [42677259833] PASS on exact base SHA. FTP and docs runs [42667988550]/[42667988522] PASS on exact 895f2a9ebcb58e810694f76cbf13706ffea9ef75. | AI docs run [42674431213] failed at base SHA but exposed no job details; recheck exact-head docs run. No kernel object or target build claimed. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 storage eject, live Samba/WINS behavior / physical router | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when the resumable task/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.
