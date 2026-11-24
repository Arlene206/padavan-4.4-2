## Current repository state

- Live `main` parent for this correction: `5e955b3bb2015e8db61a7e4c23eb44d29f873f8e`; re-fetch the ref before every write because concurrent sessions are advancing it.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never mutate it. Legacy PR #1 remains closed and unmerged.
- Main-only development: one atomic commit per logical task, full tree/commit created together, non-forced expected-SHA update. No side branches, PRs or force pushes.
- Roadmap was reduced to 130,288 UTF-8 bytes; AI handoff #45441892663 PASS on `5248d9145cb955a08b741bfa9d6f40827cc9d628`.
- Clause 22 direct/indirect source-contract check: Switch CLI #45441278142 PASS on `a8284ea16840ffbd10f3c0444e68a829bfc7d9d5`. C22 source SHA `18d90355fb56b33100c05439df30ecc3e61ef85f` XY-C1 #45439391086 completed with firmware build stages skipped; no L3. L4 hardware remains NOT VERIFIED.

## Active task

### Close FTP firewall reconciliation and shared HTTPD firewall contract

- Product-source commit `7ff02de99fc676e0262b7c5770356b1e31bda758` fixes `services_stor.c::restart_ftpd()`: after a confirmed daemon stop, reconcile FTP firewall/UPnP state exactly once even if storage is absent, mount inventory is unknown, or replacement startup fails. Unknown inventory fails closed; the original start error retains precedence over firewall refresh failure; stop failure aborts because daemon absence is not proven.
- Added `ftp_restart_should_reconcile_firewall()` policy regression and source integration contract; USB Storage Hotplug workflow also tracks the changed paths.

## Verification boundary

- Exact push SHA `7ff02de99fc676e0262b7c5770356b1e31bda758`: VSFTPD #45441167767 PASS; USB storage hotplug #45441189222 PASS; storage enable #45441167587 PASS; Process inventory #45441167968 PASS; NVRAM #45441167946 PASS; Samba #45441167644 PASS; Firewall Startup #45441167707 PASS. XY-C1 #45441167736 is `in_progress`; inspect Build, TRX and artifact individually. L3 requires all three successful on this same product-source SHA.
- HTTPD Security #45441167757 failed on `7ff02de` after firewall host tests passed. Follow-up `d8ad60504a9465b5a75d5067aa44f80cf3945ace` repaired the retired boolean-helper and WAN gate assertions but left a stale argument-suffix assertion at line 204; exact-head HTTPD #45442258062 still failed there. THIS correction replaces it with an exact eleven-argument check. AI docs #45442257900 on `d8ad605` PASS.
- New concurrent product-source commit `5e955b3bb2015e8db61a7e4c23eb44d29f873f8e` adds descriptor-pinned CWD ACL enforcement for vsftpd; its NVRAM audit #45442617336 and XY-C1 #45442617613 were IN_PROGRESS at last observation. Review its exact-head VSFTPD/host regressions before relying on it; do not conflate it with the earlier FTP firewall slice.
- Prior RC firewall readiness source `7ca46fc7049f3380755f6b2fe5073d86727c6382` has same-SHA XY-C1 Build/TRX/artifact PASS #45434352284. This evidence applies only to that older source. Physical-device L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head HTTPD Security and AI handoff workflows triggered by THIS correction. If HTTPD fails, read the first traceback; preserve all source and behavior assertions.
2. Inspect XY-C1 #45441167736 on FTP source `7ff02de`, checking Build, TRX and artifact separately. Diagnose Build failures from the first compiler error.
3. Inspect new vsftpd descriptor-pinned CWD ACL commit `5e955b3`: verify its policy tests/source contract and exact-SHA VSFTPD/XY/NVRAM status.
4. After closing these independent checks, continue the next highest-priority open stability/security defect from the whole-repository roadmap. Pending CI does not block independent engineering.

## Mandatory handoff checkpoint

At session end inspect live `main`, the frozen backup, product-source SHA, exact-head Actions, L2/L3/L4 evidence and the first next action. The checkpoint is mandatory; a document edit is conditional. If accurate, leave the handoff untouched.

# Cross-session execution mandate

Before context exhaustion, record the exact active source SHA, regression results, exact-SHA Actions and next first action. Recover once, implement, test and hand off. Use one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware or hardware validation.
