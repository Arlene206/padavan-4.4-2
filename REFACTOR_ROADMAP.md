## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only; branch inventory contains `main` only, with no remote checkpoint branch.
- Current exact main checkpoint at takeover: `e1f27e5a5fb607a6ca8177c6cf1f220dc3515e7e`; branch inventory remains `main` only. DNSMasq lease-file applicability gating, HTTPD/DNSMasq generated-file trust, Samba config regression and AI handoff Actions [42735569321] [42735569085] [42735569423] all completed SUCCESS on this exact SHA. THIS round adds wireless shutdown-result propagation.
- Samba same-boot paired passdb/config transaction implementation: `1a7097ace2e6c689cb7e92cd10055967d99fe143`. The fix adds PREPARED/COMMITTED journal recovery and a shared process-scoped writer lock for `run_samba()`, public `write_smb_conf()`, and WINS `start_wins()`.
- Samba host transaction tests and source-order contract [42708213921] completed SUCCESS on exact SHA `e9a93eb84ac9f95447150869fab7e0ae1c2da9ff`. The new GNU89 test covers interrupted publication, committed cleanup, missing-original restore, rollback retry and symlink rejection.
- Dependent HTTPD/Samba trust-boundary suite [42708503436] and AI handoff documentation validation [42708503300] completed SUCCESS on exact SHA `258532a78922549698bc6d9b869c7777f8f4ac05`. The HTTPD workflow extractor now targets the unlocked config generator rather than the public lock wrapper.
- The earlier BusyBox DHCP callback state audit is complete; no applet behavior change was justified because the inspected status-discard contract did not demonstrate a client state-transition defect. The WAN callback source-contract correction is green on exact SHA `6b44e023c2f669865fc8b5cb5425d824b8335d8b` (DHCP [42703609156], docs [42703609087]).
- Initial Samba/HTTPD workflow attempts on the implementation head failed in stale semantic source extractors; later exact-SHA runs above passed after the test assertions were updated. Do not count the initial failed runs as green.
- Root `dev_log.md` contains 100 complete newest-first records after the wireless shutdown propagation entry and remains below the 119-record / 384 KiB thresholds. No archive rotation is due.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease validation and interface apply/rollback; MAN off-subnet gateway deletion retains old lease identity; MAN/WAN route teardown and gateway/DNS additions propagate failures; IPTV DHCP bound/deconfig check address and route operations and retain prior route inventory on teardown failure. Exact-head WAN DHCP [42733942876], route status [42733942923] and interface teardown [42733942837] all PASS on `186d3e6a2a392c21045aced3a3754073853c4c2e`. | BusyBox discards DHCP script exit status; separate static-IP IPTV and ZeroConf status propagation, leasefail/NAK/expiry/renew/SIGUSR2 semantics, PPP/multi-WAN and IPv6 callback lifecycle remain open; target route reconciliation not verified. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. Lease-file initialization is gated by finalized DHCP feature flags: DNS-only and RA-only configurations do not depend on an unused `/tmp/dnsmasq.leases` path, while lease-using DHCP configurations still fail closed. GNU89 regression and semantic source contract are pending on this round's exact SHA. | Remaining daemon lifecycle, generated-option ownership, DNS update and restart/stop error propagation; MIPS/uClibc production RC build remains unverified. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. The internal MTD/UBI path reports actual `opt-mount.sh` status, retries with monotonic exponential backoff, and tracks/cancels its worker process group. Samba and vsftpd stop failures now participate in `stop_stor_apps()`; the safe-removal callers abort before unmount when any required storage service does not stop. | No production RC object/MIPS build; target filesystem and real hardware behavior remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. Namespace/zone identity is stored and enforced for IPv4/IPv6 mappings and dying-tuple GC; notifier lifecycle, host policy and exact-head semantic contracts pass on `bc99b6801c44f55e13dc0214336408f712e0abf8`. | Full kernel/MIPS object compile and broader firewall/NAT audit remain open. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, atomic configuration publication, config preflight and radio-state publication gates. Shutdown now aggregates interface-down failures and aborts single-radio/DBDC restart before 802.1X teardown or replacement startup if any old interface remains. GNU89 shutdown-result regression and exact-head source contract pending. | Startup-helper failures remain hidden by void AP/WDS/APCLI helpers; bridge attachment command results, iNIC/board-specific concurrency and live radio behavior remain open. L3 target build/device behavior unverified. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf staging/atomic publication, folder inventory/read errors fail closed, mode-4 write-only ACL generation, credential staging and ordinary-error rollback; required smbd/nmbd command and process-presence postconditions; stop-status propagation into storage removal; WINS lifecycle checks. Same-boot paired passdb/config transaction now uses PREPARED/COMMITTED snapshots, idempotent rollback/recovery and a shared writer lock; GNU89 host crash-recovery tests and exact-head Samba/HTTPD Actions PASS. | Live authentication/reload compatibility and target build/device behavior remain unverified; volatile /etc journal is not a power-loss guarantee. |
| FTP service / vsftpd | PARTIAL / AUDITING | Upload/RETR/SIZE, DELE/RMD, MKD, RNFR/RNTO, recursive LIST/NLST/STAT ACL and metadata operations are descriptor-relative; recursion bounded to depth 32 / 4096 entries per directory / 32768 total. ACL config reader rejects blocking/symlink/non-regular/oversized inputs. STAT distinguishes complete versus failed/unavailable listings while retaining the 213 multiline terminator; exact-head VSFTPD and docs Actions PASS on `a86e2ceab5f6a448acfe82b8f388529a9941c3d3`. | MDTM/SITE CHMOD descriptor-relative metadata and upload cleanup race remain open; auth/account/anonymous compatibility and MIPS/uClibc runtime remain unverified. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Reconcile wireless shutdown-result propagation on this exact SHA: policy test must return negative status for incomplete shutdown; source contract must prove current and DBDC peer stop gates precede 802.1X teardown/replacement start. If green, continue AP/WDS/APCLI startup-helper status and bridge-operation audit.
2. P2 — Complete FULLCONENAT target kernel/MIPS object validation; host namespace/zone policy tests do not substitute for target compilation.
3. P2 — Keep static-IP IPTV/ZeroConf callbacks, FTP MDTM/SITE CHMOD and upload-cleanup race visible.
4. P2 — Keep NFS `async` durability/throughput, AUTH_SYS/root-squash migration and external-mount compatibility decisions visible.

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
| L1 source review | PASS for FTP descriptor-relative operations and recursive listing; PASS for IPTV DHCP callback interface/route status; PASS for DNSMasq lease-file feature gating; PASS for wireless stop-before-restart ordering and DBDC peer-stop gates. |
| L2 host / Actions | DNSMasq parent SHA `e1f27e5a5fb607a6ca8177c6cf1f220dc3515e7e`: HTTPD/DNSMasq [42735569321], AI handoff [42735569423] and Samba [42735569085] all SUCCESS. THIS wireless GNU89 policy test, source contract and AI handoff validation pending on resulting SHA; workflows do not compile production `net_wifi.c` for MIPS/uClibc. |
| L3 MIPS/uClibc production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 router/SMB/DHCP behavior and physical hardware | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when resumable work/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.
