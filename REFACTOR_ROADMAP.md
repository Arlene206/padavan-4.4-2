## Current source / CI checkpoint

- Repository: `Arlene206/padavan-4.4-2`; maintained fork only; branch inventory contains `main` only, with no remote checkpoint branch.
- Current main baseline: `cd3d66c4687d8e207ce293c088f2920b1ccc7a6d`. Latest VSFTPD regression [42728646505] FAILED at final FIFO fixture cleanup (`remove fixture: Directory not empty`); AI handoff docs [42728646431] SUCCESS on the same exact SHA. The ACL config reader implementation is committed; the host test forgot to unlink its FIFO fixture. Remote branches: `main` only; no checkpoint branch.
- Samba same-boot paired passdb/config transaction implementation: `1a7097ace2e6c689cb7e92cd10055967d99fe143`. The fix adds PREPARED/COMMITTED journal recovery and a shared process-scoped writer lock for `run_samba()`, public `write_smb_conf()`, and WINS `start_wins()`.
- Samba host transaction tests and source-order contract [42708213921] completed SUCCESS on exact SHA `e9a93eb84ac9f95447150869fab7e0ae1c2da9ff`. The new GNU89 test covers interrupted publication, committed cleanup, missing-original restore, rollback retry and symlink rejection.
- Dependent HTTPD/Samba trust-boundary suite [42708503436] and AI handoff documentation validation [42708503300] completed SUCCESS on exact SHA `258532a78922549698bc6d9b869c7777f8f4ac05`. The HTTPD workflow extractor now targets the unlocked config generator rather than the public lock wrapper.
- The earlier BusyBox DHCP callback state audit is complete; no applet behavior change was justified because the inspected status-discard contract did not demonstrate a client state-transition defect. The WAN callback source-contract correction is green on exact SHA `6b44e023c2f669865fc8b5cb5425d824b8335d8b` (DHCP [42703609156], docs [42703609087]).
- Initial Samba/HTTPD workflow attempts on the implementation head failed in stale semantic source extractors; later exact-SHA runs above passed after the test assertions were updated. Do not count the initial failed runs as green.
- Root `dev_log.md` contains 87 complete records after the RNFR/RNTO implementation; this CI-contract reconciliation will make 88. No archive rotation is due.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IP/netmask validation; optional router/DNS/WINS/domain normalization; withdrawn option clearing; changed-gateway delete-before-publish; fail-closed deconfig cleanup; bound/renew checked address/route/resolver transaction with rollback and last-known-good lease fields preserved. Exact-head GNU89 host/source regression PASS on `70415484275e9c542be3d45b8bad807fb6a09591`. | BusyBox discards event-script exit status; leasefail/NAK/release state handling, retry semantics and live DHCP transitions remain open. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease validation and interface apply/rollback; MAN off-subnet gateway deletion retains old lease identity; MAN/WAN route teardown status is accumulated; direct MAN/WAN gateway/DNS additions and static/DHCP helper results now propagate through private up helpers, log failures and preserve public void wrappers; DHCP bound/renew reports route setup status. | THIS add-route exact-head workflow pending; BusyBox discards callback status; leasefail/NAK/expiry/renew/SIGUSR2 semantics, PPP/multi-WAN and IPv6 callback lifecycle remain open; target route reconciliation not verified. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain validation, resolver refresh failure contracts and generated-file trust checks. | Daemon lifecycle, option ownership, DNS update and restart/stop error propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Full partition/ejected inventory staging, swapoff-before-unmount gates, operation-wide storage lock, Optware startup supervision/rollback, and checked removable-storage teardown. The internal MTD/UBI path reports actual `opt-mount.sh` status, retries with monotonic exponential backoff, and tracks/cancels its worker process group. Samba and vsftpd stop failures now participate in `stop_stor_apps()`; the safe-removal callers abort before unmount when any required storage service does not stop. | No production RC object/MIPS build; target filesystem and real hardware behavior remain unverified. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling. Namespace/zone identity is stored and enforced for IPv4/IPv6 mappings and dying-tuple GC; notifier lifecycle, host policy and exact-head semantic contracts pass on `bc99b6801c44f55e13dc0214336408f712e0abf8`. | Full kernel/MIPS object compile and broader firewall/NAT audit remain open. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Selected ioctl checks, scan bounds, config preflight, radio-state publication and AP/APCLI/WDS bridge gates. | Remaining iNIC/board-specific/concurrent lifecycle and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Authentication, disclosure, safe publication, NVRAM restore and CGI policy checks have recent Actions. | Whole-route input, authorization and error-path inventory. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication and restore failure gates. | Remaining direct callers, flash recovery and board variants. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected ioctl propagation, PVID/link retry, CLI ordering and MDIO timeout checks. | Other board drivers and live hardware behavior. |
| Samba share service / accounts | PARTIAL / AUDITING | Generated smb.conf staging/atomic publication, folder inventory/read errors fail closed, mode-4 write-only ACL generation, credential staging and ordinary-error rollback; required smbd/nmbd command and process-presence postconditions; stop-status propagation into storage removal; WINS lifecycle checks. Same-boot paired passdb/config transaction now uses PREPARED/COMMITTED snapshots, idempotent rollback/recovery and a shared writer lock; GNU89 host crash-recovery tests and exact-head Samba/HTTPD Actions PASS. | Live authentication/reload compatibility and target build/device behavior remain unverified; volatile /etc journal is not a power-loss guarantee. |
| FTP service / vsftpd | PARTIAL / AUDITING | Upload writes, RETR/SIZE, LIST metadata and traversal ACL are descriptor-relative; DELE/RMD `unlinkat()`, MKD `mkdirat()`, RNFR/RNTO `renameat()`; recursive listing uses no-follow child opens, per-child READ ACL, `fstatat()`/`readlinkat()`, opened-base visibility and 32/4096/32768 resource caps. ACL config reader now rejects FIFO/device/symlink and oversized files using a nonblocking no-follow fd with bounded reads; implementation committed at `cd3d66c4687d8e207ce293c088f2920b1ccc7a6d`. | First exact-head VSFTPD regression failed only at FIFO fixture teardown because the test omitted `unlink(fifo_file)`; cleanup correction and exact-head rerun pending. Review LIST/NLST/STAT output compatibility, then MDTM/SITE CHMOD descriptor-relative metadata and upload-cleanup race. Auth/account/anonymous compatibility and MIPS/uClibc runtime remain open. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping and selected start/stop failure gates. MTD Optware worker resets inherited PID1 signal dispositions/mask and is cancelled before normal service shutdown. | Cross-module ordering, other package supervision and remaining shutdown races. |

## Priority queue

1. P2 — Reconcile the ACL config reader's exact-head host regression after the FIFO fixture cleanup correction. If green, compatibility-test FTP LIST/NLST/STAT output, error, sorting and filtering at the recursive listing bounds; then finish MDTM/SITE CHMOD descriptor-relative metadata and the upload cleanup race. Target MIPS/uClibc validation remains required.
2. P2 — Review the IPTV DHCP callback route/interface status-loss observation in `trunk/user/rc/src/net_wan.c` as a bounded follow-up; the broader BusyBox client event-state audit is complete.
3. P2 — Complete FULLCONENAT target kernel/MIPS object validation; host namespace/zone policy tests do not substitute for target compilation.
4. P2 — Keep NFS `async` durability/throughput, AUTH_SYS/root-squash migration and explicit external-mount compatibility decisions visible.

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
| L1 source review | PASS for the bounded DHCP/Samba/FTP upload, RETR/SIZE, DELE/RMD, MKD and RNFR/RNTO hardening, plus this round's top-level listing identity binding and no-follow, ACL-checked recursive child opens. | `ls.c` entry metadata path syscalls and wider listing visibility/resource audit remain open. |
| L2 host / Actions | On exact SHA `cd3d66c4687d8e207ce293c088f2920b1ccc7a6d`, VSFTPD [42728646505] FAILED at final FIFO fixture cleanup after the earlier GNU89 host binaries passed; AI docs [42728646431] SUCCESS. The test cleanup correction and exact-head rerun are pending. Workflow does not compile production `asus_ext.c` or `ls.c`. |
| L3 MIPS/uClibc production objects / firmware/TRX | NOT VERIFIED. | Full target build phase required. |
| L4 router/SMB/DHCP behavior and physical hardware | NOT VERIFIED. | Requires target-device validation. |

## Handoff requirements

- SESSION_HANDOFF.md is the only actionable continuation point.
- Update the handoff only when resumable work/checkpoint/next action changes or before stopping; update the roadmap only when module status/evidence/priorities change; record coherent milestones in dev_log.md.
- Keep 20–119 complete records in root dev_log.md. At 120 records, batch-archive the oldest 100 and retain the newest 20; also honor the 384 KiB target and 512 KiB hard limit.
- Recheck main, checkpoint branches and exact-head Actions on every takeover.
