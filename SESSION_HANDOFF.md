## Current repository state

- Live parent main before this atomic change: `42257289849aa454630dfda18b8e1efc1a213411`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

Atomic QMI packet-data-handle publication:
- The prior QMI WAN start path now rejects stale/unparseable `/tmp/qmi-handle`, checks the `uqmi --start-network` exit status, and requires both a successful command and a valid nonnegative PDH handle before reporting success. Exact USB modem identity Action #45346595809 passed that code/test slice.
- Remaining QMI reliability gap: `cmd_wds_start_network_cb()` still published the PDH handle using `fopen("w+")/fprintf/fclose` without checking any result. It could truncate the old file and leave an incomplete or stale-looking status after an interrupted/failed write.
- Reuse the existing `uqmi_status_file_write()` private staging + fsync + atomic rename helper for `/tmp/qmi-handle`. On publication failure, emit an error and do not add a handle to the command status; the rc-side strict parser then refuses to treat the attempt as a live session.
- The existing UQMI status-file test covers 0600 mode, atomic replacement, symlink destination replacement, failed rename preserving the prior file, and staging-file cleanup. Extend the USB modem workflow source contract to pin callback ordering and forbid direct `fopen/fprintf/fclose` writes.
- The 4225728 MTD checker cleanup removed a stray dangling source line. Its exact Process #45346865350, HTTPD #45346865450 and AliyunDrive #45346865525 runs were IN_PROGRESS at last poll; verify their final results. No physical modem or target MTD hardware validation is claimed.

## Verification boundary

- Exact 42257289849aa454630dfda18b8e1efc1a213411: cleanup commit removed the stray dangling `, fatal_shutdown) is None` line from `scripts/check_dev_init_boot.py`. Process inventory #45346865350, HTTPD #45346865450 and AliyunDrive WebDAV #45346865525 were IN_PROGRESS at last poll; check their final exact-SHA outcomes.
- Exact 1dc62d8e8f99a40b2447cce57e77781e3e155827: USB modem identity #45346595809 PASS, including QMI client ID and packet-data-handle source contract; AI handoff #45346595807 PASS. Process lifecycle #45346595881 failed at the malformed MTD checker line, removed by 4225728. XY-C1 #45346595827 was IN_PROGRESS at Build at last observation; no TRX/artifact/L3 claim.
- Exact 372400813c7d0888c508d6e6595353c46ea5d960: ZeroTier lifecycle #45344943818 PASS; Process inventory #45344943892 PASS; WAN route #45344943905 PASS; AI handoff #45344943861 PASS.

## Exact first action for the next GPT

1. Verify exact-head USB modem identity Action for the atomic PDH writer change; fix the first failing source/test assertion.
2. Confirm final exact-SHA Process inventory, HTTPD and AliyunDrive outcomes after the 4225728 MTD checker cleanup. Continue the next independent userland stability/security source defect.
3. Poll XY-C1 Build/TRX/artifact on its exact source SHA; do not claim L3 without same-SHA Build, TRX and artifact PASS.

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
