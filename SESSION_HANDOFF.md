## Current repository state

- Live parent main before this atomic change: `08f39868ce700f008ba6501b4388eb06681cac64`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

QMI WAN startup ambiguous-result retry policy:
- QMI startup rejects stale/unparseable PDH files, checks `uqmi --start-network`, strictly parses the single PDH token, and requires both command success and valid handle. `cmd_wds_start_network_cb()` uses private, atomic `/tmp/qmi-handle` publication.
- A successful `uqmi --start-network` process result with no valid persisted PDH is ambiguous: the modem may have created a live session while the callback failed to publish its handle. Abort instead of retrying to avoid duplicate sessions. A failed command with no PDH is retryable; a failed command with a valid PDH is already ambiguous.
- Added a GNU89 policy helper and matrix regression for all command-status/PDH combinations, plus an exact source-contract assertion that the startup path uses the helper. QMI teardown also preserves CID/PDH state when `--stop-network` or `--release-client-id` fails.
- At parent d549f80, Process inventory #45347342258, Process fail-closed #45347342171, HTTPD #45347342170, WAN teardown #45347342223, DHCP callback #45347342218, detect_link #45347342139 and AI handoff #45347342145 PASS; USB modem identity #45347342135 failed only because the workflow tried to slice the stop function by an escaped newline delimiter (fixed in 08f3986). NVRAM #45347342159 and XY-C1 #45347342256 were still running at last poll. No target hardware validation is claimed.

## Verification boundary

- Exact e006c68f711c0fc47e6d46d7f34b3c337a06f7da: NVRAM audit #45347148992, Process inventory #45347149044, AI handoff #45347148865, XY-C1 firmware #45347148972 and USB modem identity #45347148897 were IN_PROGRESS/QUEUED at last observation. Check final statuses before relying on them.
- Exact 42257289849aa454630dfda18b8e1efc1a213411: Process inventory #45346865350, HTTPD #45346865450 and AliyunDrive WebDAV #45346865525 all completed SUCCESS after removing the stray malformed checker line from `scripts/check_dev_init_boot.py`.
- Exact 1dc62d8e8f99a40b2447cce57e77781e3e155827: USB modem identity #45346595809 PASS, including QMI status/PDH validation; AI handoff #45346595807 PASS. Process lifecycle #45346595881 failed only on the malformed MTD checker line later removed by 4225728. XY-C1 #45346595827 remained IN_PROGRESS at last observation.
- Exact 372400813c7d0888c508d6e6595353c46ea5d960: ZeroTier lifecycle #45344943818 PASS; Process inventory #45344943892 PASS; WAN route #45344943905 PASS; AI handoff #45344943861 PASS.

## Exact first action for the next GPT

1. Verify exact-head USB modem identity for the ambiguous QMI start result policy; fix any regression in the GNU89 test or embedded workflow contract.
2. Check exact-head AI handoff and USB modem actions; poll d549f80 NVRAM/XY-C1 runs for final outcomes and verify same-SHA Build/TRX/artifact steps.
3. Continue another independent userland stability/security slice once the QMI startup and teardown policies pass.

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
