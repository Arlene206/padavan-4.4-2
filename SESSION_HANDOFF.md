## Current repository state

- Live `main` parent for this task: `0fa6f300b6bfa8353dd5a756a41c099b6afa9d73` (`fix(vsftpd): reconcile firewall after standalone FTP stop`). Re-fetch the branch before every write; concurrent ChatGPT sessions can advance it.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify. Legacy PR #1 remains closed/unmerged.
- Main-only, one atomic main commit per logical task: create every blob and one complete Git tree, then one commit, then non-forced expected-SHA ref update. Never force-push or create side branches/PRs.
- Roadmap remains below its 128 KiB handoff limit; this task adds one bounded P1 item without exceeding the validator ceiling.
- Clause 22 indirect register compatibility is preserved in `18d90355fb56b33100c05439df30ecc3e61ef85f`; Switch CLI #45441278142 PASS. HTTPD firewall status-call contract correction `377f5a6670caefe0da2e46b8fd8b97977ca8543c` passed exact-head HTTPD #45442773609 and AI #45442773576.

## Active source tasks

### FTP stop/restart firewall reconciliation

- Source `0fa6f300b6bfa8353dd5a756a41c099b6afa9d73` closes both `restart_ftpd()` and standalone `stop_ftp()`: after every confirmed daemon stop, reconcile FTP firewall/UPnP state once under the storage-operation lock; stop failures do not assume daemon absence; first operational errors retain precedence.
- Exact-head host checks on `0fa6f3`: VSFTPD #45443260653 PASS; USB Storage Hotplug #45443260461 PASS; NVRAM #45443260389 PASS; Process Inventory #45443259419 PASS; HTTPD #45443259557 PASS; AI handoff #45443260460 PASS; Firewall Startup #45443260423 PASS; Process lifecycle #45443260352 PASS; storage-enable #45443260652 PASS; IPv6 #45443260668 PASS; NFS #45443260561 PASS; Samba #45443259573 PASS; MiniDLNA #45443260586 PASS; libdisk #45443260558 PASS; Firefly #45443259386 PASS. XY-C1 #45443260501 remains IN_PROGRESS; do not claim L3 until Build, TRX and artifact upload all PASS on the same product SHA.
- Earlier restart-only source `7ff02de99fc676e0262b7c5770356b1e31bda758` had exact tests pass. Do not transfer target evidence from an earlier source SHA to this later combined stop/restart change.

### FTP ACL metadata path boundary

- THIS task adds `ftp_acl_path_build_account_permission_path()` to validate an absolute mount path, reject `/` inside the authenticated FTP username, and fail on `snprintf` truncation. `get_permission_ftp()` now denies access on invalid path construction instead of opening a potentially different ACL file.
- Regression test covers valid username, slash traversal, relative mount path and insufficient output size. The VSFTPD workflow checks source integration and builds the existing GNU89 ACL policy test. Exact-head VSFTPD/XY checks for this new SHA must be polled.

## Verification boundary

- Descriptor-pinned CWD ACL source `5e955b3bb2015e8db61a7e4c23eb44d29f873f8e`: VSFTPD #45442617633 PASS; Process #45442617629 PASS; NVRAM #45442617336 PASS. XY-C1 #45442617613 remained IN_PROGRESS at the last poll; no L3 claim.
- FTP firewall stop/restart combined source `0fa6f300b6bfa8353dd5a756a41c099b6afa9d73`: above exact-head host tests pass; XY-C1 #45443260501 pending. L3 NOT YET VERIFIED. Physical-device L4 remains NOT VERIFIED.
- Hardware-related MDIO/RAETH work still requires exact target validation; host policy tests do not establish hardware L4.

## Exact first action for the next GPT

1. Check the exact-head VSFTPD and AI handoff Actions triggered by THIS FTP ACL path commit; inspect logs if either fails, without weakening existing ACL/CWD coverage.
2. Inspect its XY-C1 Build, TRX and artifact steps separately. Continue another independent P1 source fix while the firmware job is pending.
3. Re-poll XY-C1 #45443260501 for the preceding combined FTP stop/restart source; keep evidence tied to exact SHA.
4. Continue the next high-priority unclosed reliability/security defect outside completed FTP CWD, stop/restart and firewall work.

## Mandatory handoff checkpoint

At session end inspect live `main`, frozen backup, product-source SHA, exact-head Actions, L2/L3/L4 evidence and first next action. The checkpoint is mandatory; a document edit is conditional. If accurate, leave it untouched.

# Cross-session execution mandate

Before context exhaustion, record the exact active source SHA, regression results, exact-SHA Actions and next first action. Recover once, implement, test and hand off. Keep one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware or hardware validation. Complete this checkpoint Before context exhaustion.
