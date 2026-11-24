## Current repository state

- Live parent main before this atomic change: `7e69ffb75bd26c854e2c36662185ef9d0815c194`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

FRP exact argv identity on procfs:
- `frp_process_matches()` converted `/proc/<pid>/cmdline` NUL delimiters to newlines inside command substitution. POSIX command substitution strips trailing newlines, so an extra empty final argv was erased before the `NR == 3` check. A process with `/usr/bin/frpc -c /tmp/frp/myfrpc.ini ""` could pass the supposedly exact three-argument identity test.
- The matcher now counts NUL terminators before text conversion and requires exactly three argv entries. Added a stop regression for a genuine FRPC executable with a trailing empty argument, plus static workflow coverage for the count-before-conversion guard.
- `tr -cd '\000' | wc -c` is compatible with BusyBox's `tr`/`wc` applets; exact-head FRP CI is still required to verify the target-like command environment. No router hardware validation is claimed.
## Verification boundary

- Exact e006c68f711c0fc47e6d46d7f34b3c337a06f7da: NVRAM audit #45347148992, Process inventory #45347149044, AI handoff #45347148865, XY-C1 firmware #45347148972 and USB modem identity #45347148897 were IN_PROGRESS/QUEUED at last observation. Check final statuses before relying on them.
- Exact 42257289849aa454630dfda18b8e1efc1a213411: Process inventory #45346865350, HTTPD #45346865450 and AliyunDrive WebDAV #45346865525 all completed SUCCESS after removing the stray malformed checker line from `scripts/check_dev_init_boot.py`.
- Exact 1dc62d8e8f99a40b2447cce57e77781e3e155827: USB modem identity #45346595809 PASS, including QMI status/PDH validation; AI handoff #45346595807 PASS. Process lifecycle #45346595881 failed only on the malformed MTD checker line later removed by 4225728. XY-C1 #45346595827 remained IN_PROGRESS at last observation.
- Exact 372400813c7d0888c508d6e6595353c46ea5d960: ZeroTier lifecycle #45344943818 PASS; Process inventory #45344943892 PASS; WAN route #45344943905 PASS; AI handoff #45344943861 PASS.

## Exact first action for the next GPT

1. Verify exact-head FRP lifecycle workflow and trailing-empty-argv process identity regression.
2. Continue the next independent stability/security defect in Padavan userland; do not wait on unrelated firmware Actions.
3. Poll the current exact-SHA XY-C1 Build, TRX and artifact states before making any firmware L3 claim.
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
