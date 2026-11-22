# SESSION HANDOFF — Padavan 4.4

## Current checkpoint

- Development branch: main only. Do not create or use feature branches/PRs; do not force-push.
- Frozen archive: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807. Preserve permanently; do not modify, checkout, rename or delete.
- Latest source fix: 4fad5ae2ac97a35d79031c3a18e828702d010109 — split-DNS NVRAM snapshot error handling and preserved diagnostics. Main subsequently received 3adf4cc9d94bd930b6504b0dd11cc1fa109852fd, a separate storage restore symlink guard; fetch live main before future writes.
- Current slice: fix the DNSMasq CI contract that incorrectly rejects the required unlock on a post-acquisition snapshot failure. Then continue PID1 robustness work in BusyBox init.

## Work closed during this continuation

- Split-DNS read-collapse defect: fill_dnsmasq_servers() now takes one bounded NVRAM transaction snapshot of vpnc_pdns, vpnc_dom_t, vpnc_dns_t, wanx_domain and wanx_dns; snapshot errors abort before stage creation and preserve the prior server file. Source commit d73ba6d2b9df959b73f0c0cf283d9694362b85c3.
- Snapshot test uncovered partial remnants in the generic snapshot helper; a full memset on copy error was added in b642c460539c93e71c6b05a7b913a964e4cdd44a.
- A separate existing HTTPD source contract required the legacy common invalid-domain log message twice; restored it in 4fad5ae2ac97a35d79031c3a18e828702d010109.
- Storage restore security: main commit 3adf4cc9d94bd930b6504b0dd11cc1fa109852fd rejects a symlinked restore start_script.sh before staging, stopping rstats, or replacing the prior storage tree.
- Old handoff SCUTClient credential-file task is complete on main. Do not restart absent a newly demonstrated failure. Existing commits ea5264dfbdb9470bc29971dee053503e8177b3aa and b30aeaf66d022f53061fd3433e98277131cbeda8 plus subsequent hardening cover it.

## DNSMasq exact-SHA verification

For code SHA 4fad5ae2ac97a35d79031c3a18e828702d010109:
- DNSMasq lifecycle run 45084767538 FAILED in step "Verify daemon start postconditions and RC exit status propagation", due to a stale contract assertion: it required no file_unlock() between lock acquisition failure and stage opening, inadvertently including the new post-lock NVRAM snapshot error branch. The source behavior is correct: after acquiring the lock, a snapshot failure must unlock before returning. Update the contract to isolate the lock-acquisition branch and explicitly require unlock in the snapshot-failure branch.
- HTTPD security 45084767484 PASS.
- Process inventory/lifecycle 45084767501 PASS.
- Process lifecycle inventory 45084767638 PASS.
- Inadyn lifecycle 45084767520 PASS.
- miniupnpd config publication 45084767512 PASS.
- WAN route status 45084767491 PASS.
- WAN DHCP callback state 45084767478 PASS.
- Samba config stability 45084767463 PASS.
- NVRAM transaction boundary audit 45084767487 and XY-C1 Firmware Validation 45084767430 were IN_PROGRESS at last check; recheck exact SHA/head_sha before claiming any result.
All statuses above belong only to 4fad5ae2..., not the new workflow-contract commit or its descendants.

## Active atomic change

The current update narrows the DNSMasq workflow assertion:
- The branch from if (lock < 0) to the snapshot marker must preserve errno/return failure and must not unlock.
- The snapshot read must occur after acquiring the lock and before dnsmasq_file_open(servers_path).
- The if (snapshot_status != 0) branch must call file_unlock(lock), preserve the existing server file, restore errno and return failure.

Changed paths in this pending logical slice:
- .github/workflows/dnsmasq-lifecycle.yml
- dev_log.md
- SESSION_HANDOFF.md

No local compile/test has been run in this connector-only session. Exact-head Actions for this workflow follow-up are not yet known.

## PID1 investigation target

BusyBox PID1 implementation is trunk/user/busybox/busybox-1.24.x/init/init.c (at prior fetch: 80c5d0f74cb8fac6b1de810d9bf4897433a3e12f). Initial source inspection identified these areas to inspect before selecting a safe, testable fix:
- mark_terminated() calls update_utmp_DEAD_PROCESS() while reaping.
- stop_handler() runs as SIGTSTP handler and calls wait_any_nohang() then mark_terminated(), which may execute non-async-signal-safe utmp operations in signal context.
- reload_inittab() has an optional delayed SIGKILL child under ENABLE_FEATURE_KILL_REMOVED/CONFIG_FEATURE_KILL_DELAY; inspect config enablement and assess stale numeric PID reuse after parent reaping before changing it.
- waitfor() and the main waitpid(-1, ...) loop must remain compatible with the special stop handler and exact child reap semantics.
Do not edit this area until the precise invariant/bug is established. Add host regression coverage or an appropriate BusyBox test hook before claiming a fix.

## First actions

1. Check live main and exact Actions for this workflow-contract commit. Verify the backup still points to 72f4f157a084f8184b9597ca5bf334b8080e5807.
2. Confirm the DNSMasq workflow now passes on the new exact SHA; inspect exact-head job logs for failures. Also recheck NVRAM audit and XY-C1 result for 4fad5ae2... using the proper head_sha.
3. Continue PID1 implementation/testing on main while independent Actions are pending.
4. Update this handoff with the next source SHA, test commands/outcomes and exact CI run IDs/statuses.

Use complete Git trees and one atomic commit per logical change. Update main with force:false and expected_sha from a fresh branch read. Preserve process identity tri-state semantics and fail-closed UNKNOWN behavior. Do not report local tests, firmware/TRX/artifacts or physical-device validation unless actually observed.
