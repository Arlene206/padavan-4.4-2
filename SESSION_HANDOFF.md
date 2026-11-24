## Current repository state

- Active branch: `main` at `a8284ea16840ffbd10f3c0444e68a829bfc7d9d5`. This commit fixed the Clause 22 indirect-write source contract; exact-head Switch CLI #45441278142 PASS. AI handoff #45441278100 failed because `REFACTOR_ROADMAP.md` exceeded the 128 KiB validator limit by 380 bytes; the P1 USB checked-read historical entry was compressed without changing its technical outcome. Re-read live main before any further write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; its marker explicitly prohibits mutation. Legacy PR #1 remains closed and unmerged.
- Main-only development: each logical task is one atomic main commit, create all blobs + one complete tree + one commit, then non-forced expected-SHA compare-and-swap. No side branches, PRs or force pushes.
- RC firewall startup readiness source SHA `7ca46fc7049f3380755f6b2fe5073d86727c6382` is L3 VERIFIED by same-SHA XY-C1 Build/TRX/artifact #45434352284. Hardware L4 remains NOT VERIFIED.

## Active task

### Fix FTP firewall reconciliation on restart failure/no-storage paths

- In `services_stor.c::restart_ftpd()`, after `stop_ftp_locked()` returned success, the old code called `control_ftp_fw(is_run_before)` only inside the mounted-storage + successful-relaunch branch. A previously running vsftpd that was stopped and not relaunched (storage absent, inventory unknown or `run_ftp()` failure) skipped the presence-transition firewall rebuild, potentially leaving stale FTP port/UPnP state.
- THIS COMMIT normalizes unknown mount inventory failure to errno/EIO, skips unsafe FTP start, but calls the firewall reconciler once after every successful stop. `rc_first_service_start_status()` keeps mount/start failure ahead of firewall refresh failure. Stop failure still aborts without assuming absence.
- Added `ftp_restart_should_reconcile_firewall()` policy and GNU89 tests for success/failure status; VSFTPD workflow contract pins the guard and single firewall call after all start decisions, with no post-stop early exit. Storage Hotplug workflow also filters these changed source/test paths.

## Verification boundary

- Parent `79c278b3f12cd95db5880052c95e912d482e96c5`: its two direct corrections are not yet validated by observed exact-head Actions. Parent `a0643ffbe0b7e715c265de12d5b6a3de48b5303f` Switch CLI #45440335408 passed the Clause 22 host binary then failed its Python contract on a combined register/data guard substring; AI #45440335554 failed the required `Before context exhaustion` token. Both are fixed in 79c, not re-certified yet.
- Parent production SHA `18d90355fb56b33100c05439df30ecc3e61ef85f`: AI #45439390998 PASS, Switch CLI #45439391302 PASS, NVRAM #45439391348 PASS; XY-C1 #45439391086 was IN_PROGRESS with Build running and TRX/artifact pending.
- XY-C1 #45437415030 on `a650ed8c8fdd8b0100bc26bb91458cdf4bde9277` and #45438125350 on `bc17758dcb25dc349c9304a6d3f671600fc7f5c7` passed same-SHA Build, TRX and artifact upload. Clause 45 write-payload SHA `7ae410c1ff14d974e659b8c29bbcd83d18269f0c` XY #45438348129 remained IN_PROGRESS (Build running; TRX/artifact pending) at last poll.
- This commit's GNU89 FTP policy binary and source integration contract are pending exact-head VSFTPD/Storage Actions; no local checkout/build was available. Current-head L3 is NOT VERIFIED until its exact XY-C1 Build, TRX and artifact upload all PASS. Physical-device L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Query AI handoff after this roadmap size correction; confirm byte-size and live-handoff validation PASS. Recheck XY-C1 #45441167736 for source SHA `7ff02de99fc676e0262b7c5770356b1e31bda758` (FTP firewall reconciliation) and inspect Build/TRX/artifact individually.
2. Inspect HTTPD Authentication/Security run #45441167757 on `7ff02de`; read its job log and distinguish source regression from test-contract failure before editing. The exact run was FAILED at last poll.
3. Close the Clause 22 indirect-range exact-head validation on the latest main, including Switch CLI #45441278142 PASS and this new docs result. Then proceed with the highest-risk open fix.


1. Inspect exact-head VSFTPD configuration/lifecycle, Storage Hotplug, AI handoff, NVRAM and XY-C1 Actions triggered by THIS COMMIT. Confirm the new stop-gate tests and the post-stop reconciler source contract pass.
2. If a checker fails, inspect the first exact-SHA causal log and fix the stale assertion without weakening coverage. If XY-C1 Build fails, diagnose the first compiler error; L3 requires Build, TRX and artifact on the same SHA.
3. Re-poll pending parent XY-C1 #45439391086 (`18d9035`) and #45438348129 (`7ae410c`); keep evidence scoped to those exact SHAs.
4. Continue the next ranked P1 service/userland stability issue from `REFACTOR_ROADMAP.md`; do not repeat completed SmartDNS, PPP callback/session-lock, or RC firewall-readiness work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At each session ending inspect live `main`, frozen backup, source SHA, exact-head Actions, L2/L3/L4 evidence and first next action. If accurate, do not create a no-op handoff commit.

# Cross-session execution mandate

Before context exhaustion, record the exact active source SHA, regression results, exact-SHA Actions and next first action. Recover once, implement, test and hand off. Use one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware or hardware validation.
