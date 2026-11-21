# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on main only. Each logical task is one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real source defects with focused regressions; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, inspect the full diff, publish atomically, and preserve the exact next action here.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live main, verify the frozen backup SHA and check exact-SHA Actions at session end. Update this file only when the baseline, evidence, blocker or first next action materially changes. Do not make no-op commits.

## Current repository state

- Parent main SHA before the SCUT stale-PID serialization slice: `9ff6c64b9731a848ed07c0a08500d2c8608b09e7`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not move or modify it.
- Parent core RC rollback Action runs queued/in progress at the last snapshot: WAN route #44884091464; Process fail-closed #44884091402; XY-C1 #44884091334; AI handoff #44884091323; Storage #44884091287; NVRAM #44884091423. Many independent product regressions were also queued/in progress on the same exact SHA.
- This source slice closes a race between SCUT stale PID equality validation and `rm -f`: overlapping cron invocations could both validate the same stale PID, then one could unlink a PID file just claimed by the other. A persistent adjacent lock inode and `flock -n 9` serialize the complete check/reclaim/publish transaction.
- FD 9 is explicitly closed immediately after the PID claim completes, before NVRAM, ping, authentication or daemon-restart commands; this prevents child processes from inheriting and accidentally extending the lock. The lock file itself is never unlinked; symlink, non-regular and unavailable lock paths fail closed.
- BusyBox 1.24.x `CONFIG_FLOCK` is enabled by default. The SCUT workflow checks that Kconfig dependency and host flock; XY-C1 trigger/test coverage includes the BusyBox Kconfig source because firmware availability is a production dependency.
- Regression pauses the first process inside `rm` after stale-PID comparison, then launches a competing invocation. Exactly one health/NVRAM run must occur. Lock-file symlink refusal is also tested.
- Independent evidence: SCUT PID-identity #44882660640 PASS on `9df0573782afb3a3e19864ac17250d7358ef26ca`; NFS rpcbind readiness #44881990807 PASS on `5869188f5f174746cc2dea43c2938be55378c87c`. Historical Storage/XY pass on `646615093f93ca01ea5cbef7dac6b4937a9a5cfa` does not validate this source tip.
- XY-C1 L3 requires Build, TRX verification and artifact upload to pass on one identical source SHA. Physical-router L4 remains NOT VERIFIED.

## Active task

Checkpoint timestamp: 2026-11-21 (America/New_York).

- Current `main`: `f530b3a75684feae8490bb3e33044df8034e48b1` — `fix(crond): roll back detached startup timeout`.
- Frozen backup branch:
  ```
refs/heads/backup/pre-plugin-overhaul-2026-11-14 @ 72f4f157a084f8184b9597ca5bf334b8080e5807
  ```
- Latest verified source correction in this work period: `fix(crond): roll back detached startup timeout`, commit `f530b3a75684feae8490bb3e33044df8034e48b1`. During post-commit review, found that the crond retry loop could leave `status == 0` after a successful sleep followed by only ABSENT process samples. Passing zero to the rollback policy takes its successful-start no-op path; timeout now explicitly passes `-1` and `ETIMEDOUT`.
- Previous completed fixes remain in the ancestry: bounded rpcbind readiness; SCUT/MentoHUST late-child rollback; SCUT watchdog PID ownership validation and serialized stale-PID reclamation; detached IPTV rollback for UDPXY/MSD Lite, XUPnPD and IGMPproxy; and targeted rollback after failed detached starts for syslogd, klogd, infosvr, crond and networkmap.
- Source changes are atomic commits; no force push or backup-branch mutation.

### First task for the next session

Continue the cross-repository PID-file safety audit around `trunk/user/shared/src/shutils.c::kill_pidfile_s()` and its `kill_pidfile()` wrapper. Do not change the shared helper until all consumers and their PID-file producers are mapped. In particular, trace BusyBox `udhcpc` and PPPD numeric PID-file creation/cleanup, determine which daemons can provide stable process identity (PID plus proc start-time or command identity), and design a versioned/compatible format so external numeric-only PID files are not misparsed or inadvertently signalled. Add host regressions for malformed/trailing-garbage/overflow PID values, stale/reused PID ownership, symlink/non-regular paths, and races between validation and signal. If producer-side identity cannot be established, keep that consumer fail-closed rather than guessing.

### Recently completed development slices

- `5869188f5f174746cc2dea43c2938be55378c87c`: rpcbind bounded readiness and delayed-child regression.
- `b5052e9f49edb751f1c68cfe7f848bbdb47bc8a6` + `4806c8fd5e387e93fcbe832679efcb0f1a97b095`: SCUT/MentoHUST late-child cleanup and one-shot PIDOF fixture.
- `068762548f4004200bbf42e49423eac1da09997f` plus subsequent SCUT-watchcat fixture corrections and `1b25077e6e5a9f8750d4b5c76637beec5ca354d1`: stale watchdog PID safety, regression and serialized reclamation.
- `734f811620f596b9ea72863261d8cc4a7103bee2`, `daf8248436c650eb10429d7098e52535c436ae4d`, `41023c0835fc399ea521e397a8fb5563dd86f02e`, `c8923187b3027da02b41dd6d76ae00b2dc0b12cd`: targeted rollback of failed IPTV daemon startups and validator alignment.
- `9ff6c64b9731a848ed07c0a08500d2c8608b09e7`: core RC detached-daemon rollback.
- `7085606f16b27e7ebc693cb0c3d3af29eb8e117b`: infosvr/crond/Process Lifecycle source-contract alignment.
- `f530b3a75684feae8490bb3e33044df8034e48b1`: crond timeout rollback status fix.
## Verification boundary

- Most recent exact-SHA run set queried during this checkpoint (SHA `f530b3a75684feae8490bb3e33044df8034e48b1`):
- HTTPD daemon lifecycle stability (run 44884633341): completed/success @ f530b3a75684
- DNS Forwarder launch path stability regression (run 44884633474): completed/success @ f530b3a75684
- WAN gateway ARP probe socket stability regression (run 44884633367): completed/success @ f530b3a75684
- detect_internet signal lifecycle stability regression (run 44884633396): completed/success @ f530b3a75684
- infosvr packet and signal lifecycle stability regression (run 44884633463): completed/success @ f530b3a75684
- detect_link signal lifecycle stability regression (run 44884633356): completed/success @ f530b3a75684
- watchdog signal lifecycle stability regression (run 44884633410): completed/success @ f530b3a75684
- ZeroTier lifecycle stability regression (run 44884633404): completed/success @ f530b3a75684
- DDNSTO lifecycle stability regression (run 44884633300): completed/success @ f530b3a75684
- XY-C1 Firmware Validation (run 44884633378): in_progress/PENDING @ f530b3a75684
- DNSMasq lifecycle stability regression (run 44884633298): completed/success @ f530b3a75684
- PID 1 notification queue stability regression (run 44884633344): completed/success @ f530b3a75684
- FRP asynchronous startup lifecycle regression (run 44884633310): completed/success @ f530b3a75684
- WAN route operation status regression (run 44884633362): completed/success @ f530b3a75684
- Adbyby download and dnsmasq configuration stability regression (run 44884633443): completed/success @ f530b3a75684
- AdGuardHome config security regression (run 44884633338): completed/success @ f530b3a75684
- networkmap signal lifecycle stability regression (run 44884633448): completed/success @ f530b3a75684
- AliyunDrive WebDAV lifecycle stability regression (run 44884633274): completed/success @ f530b3a75684
- AliDDNS signed API transport security regression (run 44884633316): completed/success @ f530b3a75684
- VPN server lifecycle stability regression (run 44884633446): completed/success @ f530b3a75684
- Crond daemon lifecycle stability regression (run 44884633405): completed/success @ f530b3a75684
- AI handoff documentation contract (run 44884633392): completed/success @ f530b3a75684
- GPIO button daemon startup and press timing stability regression (run 44884633387): completed/success @ f530b3a75684
- Process lifecycle inventory - f530b3a75684feae8490bb3e33044df8034e48b1 (run 44884633475): completed/success @ f530b3a75684
- HTTPD Authentication and Security Regression (run 44884633451): completed/success @ f530b3a75684
- USB storage hotplug stability regression (run 44884633441): completed/success @ f530b3a75684
- Process inventory and lifecycle fail-closed regression (run 44884633485): completed/success @ f530b3a75684
- SkipDBv2 Regression (run 44884633522): completed/success @ f530b3a75684
- NVRAM transaction boundary audit (run 44884633303): completed/success @ f530b3a75684
- Earlier exact-SHA validation on `7085606f16b27e7ebc693cb0c3d3af29eb8e117b` reported Process Lifecycle, Crond, Infosvr, Networkmap and AI handoff checks PASS. The current crond timeout source fix supersedes that checkpoint and requires its own fresh exact-SHA results.
- No local repository checkout was available; do not claim local compiler/shell/Python execution. The C rollback policy unit test is wired into Process Lifecycle with GNU89, ASan and UBSan. The host Actions are the execution evidence.
- Firmware/host CI does not establish physical-router runtime correctness. No hardware validation is claimed.
- When a workflow is PENDING, continue another independent source task and return to its exact SHA later. On FAIL, inspect failed job logs, fix the root cause in a new atomic commit, and re-query exact SHA.
## Exact first action for the next GPT

Fetch live main and verify the frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Inspect exact-SHA SCUT, AI handoff and XY-C1 Actions for the lock-scope commit. If SCUT passes, begin a focused shared PID-file producer/consumer review in `trunk/user/shared/src/shutils.c`; do not repeat completed NFS readiness or SCUT PID-identity work.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep the frozen backup permanently unchanged.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
