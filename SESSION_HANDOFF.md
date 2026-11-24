## Current repository state

- Live parent `main`: `79c5389fe33326d85cc23e9856f39e5ca0ac242f`; the next commit is a source-contract correction for the executable-bound Inadyn inventory. Re-fetch main and compare before any write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update source is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`: generic comm/argv/start_time identity plus exact `/proc/PID/exe == /bin/inadyn` (or exact ` (deleted)` suffix), filtered snapshots, bounded shared stop lifecycle; unknown procfs access blocks signals.
- This follow-up fixes the dependent Inadyn source-checker's C function-body extractor so forward declarations cannot redirect it into an unrelated function, and aligns the DNSMasq and WAN DDNS checker assertions with `inadyn_process_inventory_scan()`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; test-fixture exit semantics are corrected in `5687c609137bbba09c707896dd95216c3df9e961` (full fake PID dir removed on simulated process exit; metadata-loss tests preserve PID dir and exe). FRP crontab still has a pending root-cause fix described below.

## Verification boundary

- Exact `79c5389` PASS: Process inventory #45341769713; Process fail-closed #45341769769; HTTPD #45341769686; AI docs #45341769695; WAN DHCP #45341769830; miniupnpd #45341769855; Samba #45341769858; NVRAM audit #45341769831.
- Exact `79c5389` FAIL: Inadyn #45341769924 was a source-checker parser bug (C policy test compile/run steps passed; Python checker selected a forward declaration). DNSMasq #45341769848 and WAN route #45341769689 contained stale raw-scan assertions; corrected in this commit. No production regression was reported by those three failures.
- XY-C1 #45341769654 had all preceding host/policy checks pass and was in `Build XY-C1`; TRX and artifact upload still pending at last poll. No same-SHA L3 result is claimed.
- Exact FRP `5687c609` actions: Process inventory #45341607727 PASS; AI docs #45341607707 PASS; FRP #45341607742 FAIL at `frp_crontab.sh` with `start rejected a valid crontab`. The FRP asynchronous startup and proc-lifecycle subtests in `frp_lifecycle.sh` printed PASS. `frp_crontab.sh` currently leaves `FRP_PROC_ROOT` pointed at live `/proc`; inspect whether host procfs visibility makes its preliminary `frp_stop_daemons()` check UNKNOWN, then isolate with a fixture root if that is confirmed. Do not assume a root cause.
- No MIPS firmware/TRX/artifact result or physical-router L4 result is claimed. Linux 4.4 lacks pidfd; the final procfs-identity-validation-to-kill race remains a platform limitation.

## Exact first action for the next GPT

1. Inspect exact-SHA Inadyn #, DNSMasq, WAN route, Process, AI docs and XY-C1 actions for this source-contract correction; fix any new failure. For firmware, verify Build + TRX + artifact all pass on one exact SHA.
2. Fix the FRP `frp_crontab.sh` test failure at `start rejected a valid crontab`: trace `frp_start()` → `frp_stop_daemons()` and its `/proc` scan, reproduce with an isolated proc root, then update the host fixture or production code according to evidence.
3. Continue the next independent P1 stability/security issue once the FRP and Inadyn workflows are green; preserve the backup and use one full Git tree + one atomic main commit with expected-SHA non-forced update.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
