## Current repository state

- Live parent main before this atomic change: `bc34d1db08a86f762ffad75bec8c8276629f29c3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

QMI WAN session startup fail-closed:
- The current QMI change on 2b7281b requires successful uqmi --start-network plus a strictly parsed PDH handle, checked stale-file removal, and cleanup between retries.
- Exact USB modem identity Action #45346405084 initially failed in the GNU89 modem WAN policy test because the test fixture accidentally encoded backslash-plus-n instead of a newline. The source parser is behaving correctly. Replaced the test fixture with explicit byte arrays containing ASCII newline and fixed the workflow's function slicing to avoid over-escaped newline delimiters.
- Re-run the exact USB modem identity Action and WAN teardown / NVRAM / Process lifecycle Actions for the follow-up commit. Check the static source contract after the C regression passes.
- Separately, the 6904523 MTD checker correction exposed further unrelated assertions in Process and AliyunDrive Actions. The next handoff must inspect the exact logs and correct the common contract without reverting the PID 1 fallback.
- No physical modem or target MTD hardware validation is claimed.

## Verification boundary

- Exact 2b7281bca3ed4be30f65b5ec803413e6cd78f86f: USB modem identity #45346405084 failed in the new parser unit test because its string fixture encoded literal backslash-n, not newline. WAN teardown #45346404889, HTTPD #45346404898, DHCP callback #45346405088, Process inventory #45346405010 and NVRAM audit #45346404844 were in progress at last poll; rerun exact head after fixture/workflow correction.
- Exact 69045235245d1b9bbdf8e830e9a5570906329929: AI handoff #45346026565 PASS; HTTPD #45346026346 PASS. AliyunDrive #45346026318 and Process inventory #45346026329 failed on further shared MTD/PID1 source-contract assertions (exact logs pending triage).
- Exact 6130ebd28962da5bf55439349632175dfc0d858e: the bulk of Process, DNSMasq, PID1 notification, HTTPD, PID1 mdev, DNS forwarder, RC timezone, USB storage, NVRAM and AI handoff checks PASS; initial Process fail-closed and AliyunDrive failed only on the stale negative MTD checker assertion.
## Exact first action for the next GPT

1. Verify exact-head USB modem identity, WAN teardown, DHCP callback, Process inventory, NVRAM audit, and AI handoff Actions after the QMI test fixture/workflow correction.
2. Inspect exact failures on 6904523 Process inventory and AliyunDrive, then fix their shared PID 1 source-contract expectations without changing production behavior unless a real defect is proven.
3. Poll XY-C1 Build/TRX/artifact for the latest production SHA. Do not claim L3 without same-SHA success.

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
