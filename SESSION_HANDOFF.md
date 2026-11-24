## Current repository state

- Live `main` at the parent of this handoff checkpoint: `6d4b0537316390ca00a388a015edb828456cc213` (`fix(vsftpd): bound ACL metadata account paths`). Re-fetch the branch before every write; concurrent ChatGPT sessions can advance it.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify. Legacy PR #1 remains closed/unmerged.
- Main-only, one atomic main commit per logical task: create every blob and one complete Git tree, then one commit, then non-forced expected-SHA ref update. Never force-push or create side branches/PRs.
- Roadmap remains below its 128 KiB handoff limit; this task adds one bounded P1 item without exceeding the validator ceiling.
- Clause 22 indirect register compatibility is preserved in `18d90355fb56b33100c05439df30ecc3e61ef85f`; Switch CLI #45441278142 PASS. HTTPD firewall status-call contract correction `377f5a6670caefe0da2e46b8fd8b97977ca8543c` passed exact-head HTTPD #45442773609 and AI #45442773576.

## Active task

### FTP stop/restart firewall reconciliation

- Source `0fa6f300b6bfa8353dd5a756a41c099b6afa9d73` closes both `restart_ftpd()` and standalone `stop_ftp()`: after every confirmed daemon stop, reconcile FTP firewall/UPnP state once under the storage-operation lock; stop failures do not assume daemon absence; first operational errors retain precedence.
- Exact-head host checks on `0fa6f3`: VSFTPD #45443260653 PASS; USB Storage Hotplug #45443260461 PASS; NVRAM #45443260389 PASS; Process Inventory #45443259419 PASS; HTTPD #45443259557 PASS; AI handoff #45443260460 PASS; Firewall Startup #45443260423 PASS; Process lifecycle #45443260352 PASS; storage-enable #45443260652 PASS; IPv6 #45443260668 PASS; NFS #45443260561 PASS; Samba #45443259573 PASS; MiniDLNA #45443260586 PASS; libdisk #45443260558 PASS; Firefly #45443259386 PASS. XY-C1 #45443260501 remains IN_PROGRESS; do not claim L3 until Build, TRX and artifact upload all PASS on the same product SHA.
- Earlier restart-only source `7ff02de99fc676e0262b7c5770356b1e31bda758` had exact tests pass. Do not transfer target evidence from an earlier source SHA to this later combined stop/restart change.

### FTP ACL metadata path boundary

- Source `6d4b0537316390ca00a388a015edb828456cc213` adds `ftp_acl_path_build_account_permission_path()` to validate an absolute mount path, reject `/` inside the authenticated FTP username, and fail on `snprintf` truncation. `get_permission_ftp()` now denies access on invalid path construction instead of opening a potentially different ACL file.
- Regression test covers valid username, slash traversal, relative mount path and insufficient output size. Exact-head VSFTPD #45444917834 and Process #45444917951 PASS; exact XY-C1 #45444916327 is still running with Build/TRX/artifact pending; NVRAM #45444916236 still running. AI #45444916200 failed only because this handoff used `## Active source tasks` instead of required `## Active task`; this checkpoint corrects the section heading.

## Verification boundary

- Exact current source SHA `6d4b0537316390ca00a388a015edb828456cc213`: VSFTPD configuration/publication #45444917834 PASS; Process lifecycle inventory #45444917951 PASS; AI handoff #45444916200 FAILS solely on the missing required heading `## Active task`, corrected in THIS checkpoint; XY-C1 #45444916327 is IN_PROGRESS with the firmware job active and Build/TRX/artifact not yet complete; NVRAM #45444916236 is IN_PROGRESS.
- FTP stop/restart combined source `0fa6f300b6bfa8353dd5a756a41c099b6afa9d73`: VSFTPD #45443260653, USB Storage Hotplug #45443260461, NVRAM #45443260389, Process Inventory #45443259419, HTTPD #45443259557, AI #45443260460, Firewall Startup #45443260423, Process lifecycle #45443260352, storage enable #45443260652, IPv6 #45443260668, NFS #45443260561, Samba #45443259573, MiniDLNA #45443260586, libdisk #45443260558 and Firefly #45443259386 PASS. XY-C1 #45443260501 remained IN_PROGRESS at last poll.
- Descriptor-pinned CWD ACL source `5e955b3bb2015e8db61a7e4c23eb44d29f873f8e`: VSFTPD #45442617633, Process #45442617629 and NVRAM #45442617336 PASS. Its XY #45442617613 remained IN_PROGRESS at the last observed poll; no L3 claim.
- Earlier restart-only source `7ff02de99fc676e0262b7c5770356b1e31bda758`: VSFTPD #45441167767, USB storage #45441189222, Process #45441167968, Firewall #45441167707 and NVRAM #45441167946 PASS. Its XY #45441167736 did not finish in the last observed poll.
- L3 is NOT VERIFIED for the current source SHA until exact same-SHA XY-C1 Build, TRX verification and artifact upload all PASS. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head AI handoff rerun caused by THIS checkpoint; it must pass the `## Active task` section contract. Check whether NVRAM #45444916236 and XY-C1 #45444916327 have completed; for XY inspect Build, TRX and artifact steps individually.
2. Audit the new FTP ACL account-path helper for remaining file-object identity/symlink risks in `read_file_to_buffer()` and its writers before claiming the path fix fully closes hostile metadata substitution.
3. If that audit finds a concrete issue, implement one narrowly scoped fail-closed source/test slice and update the path/source contract. Otherwise continue the next open P1 reliability item from the roadmap.
4. Recheck the pending XY-C1 runs #45443260501 (source `0fa6f3`) and #45442617613 (source `5e955b3`); keep evidence scoped to each SHA.

## Mandatory handoff checkpoint

At session end inspect live `main`, frozen backup, product-source SHA, exact-head Actions, L2/L3/L4 evidence and first next action. The checkpoint is mandatory; a document edit is conditional. If accurate, leave it untouched.

# Cross-session execution mandate

Before context exhaustion, record the exact active source SHA, regression results, exact-SHA Actions and next first action. Recover once, implement, test and hand off. Keep one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware or hardware validation. Complete this checkpoint Before context exhaustion.
