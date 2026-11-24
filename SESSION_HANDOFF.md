## Current repository state

- Active branch: `main` parent `377f5a6670caefe0da2e46b8fd8b97977ca8543c` (`test(httpd): verify complete firewall status inputs`). Re-fetch live main before any write because other sessions may advance it.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never mutate it. Legacy PR #1 remains closed and unmerged.
- Main-only: one atomic main commit per logical task, create all blobs + one complete tree + one commit, then non-forced expected-SHA update. No side branches, PRs or force pushes.
- Roadmap limit corrected at `5248d91`; AI handoff #45441892663 PASS. Clause 22 indirect source contract corrected at `a8284ea`; Switch CLI #45441278142 PASS.
- RC firewall startup readiness source `7ca46fc7049f3380755f6b2fe5073d86727c6382` is L3 VERIFIED by same-SHA XY-C1 Build/TRX/artifact #45434352284. Hardware L4 remains NOT VERIFIED.

## Active task

### Reconcile firewall state after standalone FTP stop

- Previous `7ff02de` fixed `restart_ftpd()` so every confirmed daemon stop triggers `control_ftp_fw()`, including no-storage, unknown inventory and failed restart outcomes.
- A second exposed path remained: public `stop_ftp()` (called by `/sbin/rc stop_ftp`) called `stop_ftp_locked()` and immediately unlocked/returned, never refreshing firewall/UPnP state. A direct FTP stop could therefore leave stale mappings after RUNNING-to-ABSENT.
- THIS COMMIT captures presence while holding the storage-operation lock, performs the stop, refreshes firewall only when `ftp_stop_should_reconcile_firewall(status)` confirms stop success, aggregates firewall status with `rc_first_service_start_status()`, and unlocks only after reconciliation. Stop failure preserves its own error and never assumes absence.
- The common helper is renamed from restart-specific to `ftp_stop_should_reconcile_firewall()` because both public stop and restart use the same confirmed-stop predicate. Existing policy tests cover success and failure outcomes; the VSFTPD workflow now asserts lock/presence/stop/firewall/error/unlock order plus the `rc.c` action mapping.

## Verification boundary

- Parent `377f5a6670caefe0da2e46b8fd8b97977ca8543c`: AI handoff #45442773576 PASS; HTTPD #45442773609 was IN_PROGRESS at last poll after the full firewall-argument contract correction. Earlier d8 HTTPD #45442258062 failed at the obsolete nested-call argument suffix; this was corrected at 377. Exact-head HTTPD rerun must confirm.
- Parent ACL source `5e955b3bb2015e8db61a7e4c23eb44d29f873f8e`: XY-C1 #45442617613 Build IN_PROGRESS, TRX/artifact pending; VSFTPD #45442617633, Process #45442617629, NVRAM #45442617336 and AI #45442617388 were in progress when last observed. Do not transfer their outcomes to this new source SHA.
- Restart firewall production source `7ff02de99fc676e0262b7c5770356b1e31bda758`: VSFTPD #45441167767 PASS; USB storage hotplug #45441189222 PASS; Process #45441167968 PASS; NVRAM #45441167946 PASS; Firewall Startup #45441167707 PASS. XY-C1 #45441167736 still Build IN_PROGRESS with TRX/artifact pending at last poll; L3 not established.
- This standalone stop source slice and workflow contract have not yet been validated. Current source L3 is NOT VERIFIED until same-SHA XY-C1 Build, TRX validation and artifact upload all PASS. Physical-device L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head VSFTPD, USB Storage Hotplug, HTTPD Security, AI handoff, NVRAM and XY-C1 Actions triggered by THIS COMMIT. Confirm the new public-stop contract and policy binary pass; diagnose any first causal failure.
2. Re-poll XY-C1 #45442617613 on source `5e955b3` and #45441167736 on `7ff02de`; keep evidence scoped to those exact SHAs.
3. Continue the next P1 stability/security issue from `REFACTOR_ROADMAP.md` after this FTP stop/restart lifecycle is green. Avoid repeating completed MDIO, PPP callback/session-lock and RC firewall readiness work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At every session ending inspect live `main`, frozen backup, exact source SHA, exact-head Actions, L2/L3/L4 evidence and first next action. If accurate, do not create a no-op handoff commit.

# Cross-session execution mandate

Before context exhaustion, record the exact active source SHA, regression results, exact-SHA Actions and next first action. Recover once, implement, test and hand off. Use one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware or hardware validation.
