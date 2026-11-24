## Current repository state

- Live `main` parent for this VSFTPD contract correction: `0f812eef2ce02057c8d95227dcad7d1478f758ba` (`fix(libdisk): pin removable metadata parent paths`). Re-fetch the branch before every write; concurrent ChatGPT sessions can advance it.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify. Legacy PR #1 remains closed/unmerged.
- Main-only, one atomic main commit per logical task: create every blob and one complete Git tree, then one commit, then non-forced expected-SHA ref update. Never force-push or create side branches/PRs.
- Roadmap remains below its 128 KiB handoff limit; this task adds one bounded P1 item without exceeding the validator ceiling.
- Clause 22 indirect register compatibility is preserved in `18d90355fb56b33100c05439df30ecc3e61ef85f`; Switch CLI #45441278142 PASS. HTTPD firewall status-call contract correction `377f5a6670caefe0da2e46b8fd8b97977ca8543c` passed exact-head HTTPD #45442773609 and AI #45442773576.

## Active task

### FTP stop/restart firewall reconciliation

- Source `0fa6f300b6bfa8353dd5a756a41c099b6afa9d73` closes both `restart_ftpd()` and standalone `stop_ftp()`: after every confirmed daemon stop, reconcile FTP firewall/UPnP state once under the storage-operation lock; stop failures do not assume daemon absence; first operational errors retain precedence.
- Exact-head host checks on `0fa6f3`: VSFTPD #45443260653 PASS; USB Storage Hotplug #45443260461 PASS; NVRAM #45443260389 PASS; Process Inventory #45443259419 PASS; HTTPD #45443259557 PASS; AI handoff #45443260460 PASS; Firewall Startup #45443260423 PASS; Process lifecycle #45443260352 PASS; storage-enable #45443260652 PASS; IPv6 #45443260668 PASS; NFS #45443260561 PASS; Samba #45443259573 PASS; MiniDLNA #45443260586 PASS; libdisk #45443260558 PASS; Firefly #45443259386 PASS. XY-C1 #45443260501 remains IN_PROGRESS; do not claim L3 until Build, TRX and artifact upload all PASS on the same product SHA.
- Earlier restart-only source `7ff02de99fc676e0262b7c5770356b1e31bda758` had exact tests pass. Do not transfer target evidence from an earlier source SHA to this later combined stop/restart change.

### FTP ACL metadata path and parent identity

- Source `6d4b0537316390ca00a388a015edb828456cc213` rejects `/` inside authenticated FTP usernames, requires an absolute mount path and fails closed on filename truncation. Its exact-head VSFTPD #45444917834, Process #45444917951, NVRAM #45444916236 and XY-C1 #45444916327 all PASS; the XY job's Build, TRX and artifact-upload steps all succeeded (L3 VERIFIED for that SHA only).
- Parent-pinning commit `ecb381796f856495b41d0d76e87718c82944ea6d` changed the bounded ACL path helper to open its leaf with `openat()` relative to a verified directory descriptor and added symlinked-parent regression coverage. Exact-head VSFTPD #45447276290, Process #45447276310, AI #45447276525 and NVRAM #45447276423 PASS; XY-C1 #45447276297 was still IN_PROGRESS at last poll.
- Reader revalidation source `9fe57d025796fbc0d6574dc68b9f71c5dc80903b`: Process #45448598258 and AI #45448598098 PASS; VSFTPD #45448598152 failed only on the two obsolete wrapper assertions at line 450; XY-C1 #45448598232 IN_PROGRESS; NVRAM #45448598003 CANCELLED. THIS commit corrects the test contract.
- Source `9fe57d025796fbc0d6574dc68b9f71c5dc80903b` keeps the mount descriptor open through metadata reading, verifies directory identity before open, after open and after read, and compares metadata device/inode/size. Process #45448598258 and AI #45448598098 PASS; VSFTPD #45448598152 failed at Python line 450 because two duplicate type/size assertions still targeted the wrapper after those checks moved to the `_at` helper. THIS commit removes those stale duplicates without removing the equivalent helper checks. XY #45448598232 was IN_PROGRESS; NVRAM #45448598003 was CANCELLED. Exact VSFTPD/AI reruns pending.

## Verification boundary

- Source SHA `6d4b0537316390ca00a388a015edb828456cc213`: VSFTPD #45444917834, Process #45444917951, NVRAM #45444916236 and XY-C1 #45444916327 PASS; the exact-SHA XY job's Build, TRX and artifact upload all succeeded (L3 VERIFIED). AI handoff #45445344806 PASS on documentation-only descendant `f6ed1313a31c9a4a89cc9a44d3994869719e5c08`.
- FTP stop/restart combined source `0fa6f300b6bfa8353dd5a756a41c099b6afa9d73`: VSFTPD #45443260653, USB Storage Hotplug #45443260461, NVRAM #45443260389, Process Inventory #45443259419, HTTPD #45443259557, AI #45443260460, Firewall Startup #45443260423, Process lifecycle #45443260352, storage enable #45443260652, IPv6 #45443260668, NFS #45443260561, Samba #45443259573, MiniDLNA #45443260586, libdisk #45443260558 and Firefly #45443259386 PASS. XY-C1 #45443260501 remained IN_PROGRESS at last poll.
- Descriptor-pinned CWD ACL source `5e955b3bb2015e8db61a7e4c23eb44d29f873f8e`: VSFTPD #45442617633, Process #45442617629 and NVRAM #45442617336 PASS. Its XY #45442617613 remained IN_PROGRESS at the last observed poll; no L3 claim.
- Earlier restart-only source `7ff02de99fc676e0262b7c5770356b1e31bda758`: VSFTPD #45441167767, USB storage #45441189222, Process #45441167968, Firewall #45441167707 and NVRAM #45441167946 PASS. Its XY #45441167736 did not finish in the last observed poll.
- L3 is NOT VERIFIED for the current source SHA until exact same-SHA XY-C1 Build, TRX verification and artifact upload all PASS. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head VSFTPD and AI handoff Actions triggered by THIS contract correction; VSFTPD must pass the ACL helper assertions and compile/run host regressions.
2. Poll XY-C1 #45448598232 for source `9fe57d0`, and inspect Build, TRX and artifact upload separately. NVRAM #45448598003 was cancelled; rerun only if required by the CI matrix.
3. Continue the independent libdisk removable-metadata parent-path task from `0f812eef2ce02057c8d95227dcad7d1478f758ba` while its exact-head XY/NVRAM/Process Actions are pending, then take the next open P1 reliability/security item.
4. Do not transfer L3 evidence from the earlier `6d4b053` firmware build to later source SHAs.

## Mandatory handoff checkpoint

At session end inspect live `main`, frozen backup, product-source SHA, exact-head Actions, L2/L3/L4 evidence and first next action. The checkpoint is mandatory; a document edit is conditional. If accurate, leave it untouched.

# Cross-session execution mandate

Before context exhaustion, record the exact active source SHA, regression results, exact-SHA Actions and next first action. Recover once, implement, test and hand off. Keep one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware or hardware validation. Complete this checkpoint Before context exhaustion.
