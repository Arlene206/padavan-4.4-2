## Current repository state

- Live parent `main`: `4be3f3898faf4665ebe19b1e87cf385bd162c21e`; this commit corrects one exact source-contract expectation and the canonical handoff heading. Re-fetch main before further writes.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

Align FRP static source assertions to the actual procfs path construction and restore the exact `## Active task` heading required by the AI handoff validator.

FRP implementation composes paths from `frp_match_proc_dir="$FRP_PROC_ROOT/$frp_match_pid"`, then `frp_match_exe_path="$frp_match_proc_dir/exe"` and `frp_match_cmdline="$frp_match_proc_dir/cmdline"`. The source checker now pins those relationships rather than demanding duplicated direct path literals. No runtime logic or process-matching policy was weakened.

The FRP cron-only test fixture now sets `FRP_PROC_ROOT` to an empty temporary directory because it overrides the daemon readiness helper and is not intended to scan the hosted runner's real `/proc`. Independent synthetic procfs lifecycle/stop tests continue to cover fail-closed process behavior.

## Verification boundary

- Exact `4be3f389`: FRP run #45342556772 passed every executable test step (async startup/watchdog, managed crontab, boot/autostart, private config, exact process stop and lifecycle lock); only static source checker failed on the direct-vs-composed path expectation corrected here. Process inventory #45342556827 PASS. AI docs #45342556719 FAILED only because of the old `## Active slice` heading corrected here.
- Exact `257ac639`: Inadyn #45342407697 PASS; DNSMasq #45342407692 PASS; WAN route #45342407753 PASS; AI docs #45342407729 failed on the same handoff heading.
- Exact `79c5389`: Process inventory #45341769713 PASS; Process fail-closed #45341769769 PASS; HTTPD #45341769686 PASS; WAN DHCP #45341769830 PASS; miniupnpd #45341769855 PASS; Samba #45341769858 PASS; NVRAM audit #45341769831 PASS. Its dependent Inadyn/DNSMasq/WAN source contract failures are closed by `257ac639`.
- XY-C1 #45341769654 on production SHA `79c5389` was still building; TRX and artifact upload pending at last poll. No L3 result is claimed.
- No MIPS firmware/TRX/artifact result or physical-router L4 result is claimed for the current source. Linux 4.4 has no pidfd; final procfs validation-to-signal race remains.

## Exact first action for the next GPT

1. Verify exact-SHA AI docs and FRP lifecycle Actions for this commit; any remaining FRP failure must be triaged from the first failed step, not from a stale source expectation.
2. Continue an independent P1 source stability/security fix while those Actions run; do not wait on pending runs. Confirm XY-C1 Build, TRX and artifact result on one exact production-source SHA before claiming L3.
3. Keep the backup immutable and all main writes complete-tree atomic with expected-SHA non-forced ref updates.

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
