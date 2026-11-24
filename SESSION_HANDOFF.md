## Current repository state

- Live parent main before this atomic change: `bc34d1db08a86f762ffad75bec8c8276629f29c3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

QMI WAN session startup fail-closed:
- The ZeroTier process-ownership changes through 3724008 passed their exact ZeroTier lifecycle, Process inventory, WAN route and AI handoff checks; do not re-audit them.
- PID 1 MTD emergency-reboot fallback source change is in 6130ebd. The old checker assertion was overbroad; 33680078 introduced an incomplete Python raw string and is superseded by 69045235245d1b9bbdf8e830e9a5570906329929, which uses a complete line-anchored regex. Exact Process/AliyunDrive/HTTPD/AI handoff results for 6904523 must be checked.
- Current QMI source defect: qmi_control_network_start() previously removed QMI_HANDLE_PDH without checking unlink, ignored uqmi --start-network's _eval status, and reported success from file existence alone. get_qmi_handle() also ignored fscanf/fclose results and accepted partial numeric strings. A stale/malformed file could falsely report WAN session creation.
- Require checked stale-handle removal (ENOENT benign), parse one complete nonnegative integer token within INT_MAX, and accept an attempt only when both _eval succeeds and the PDH handle is valid. Clean invalid files before retry; if uqmi fails after publishing a valid handle, stop retries to avoid blindly creating a duplicate possible live session.
- Add GNU89 regressions and USB modem workflow source contract. No physical-modem hardware validation is claimed.

## Verification boundary

- Exact 69045235245d1b9bbdf8e830e9a5570906329929: AliyunDrive WebDAV #45346026318, Process inventory #45346026329, HTTPD #45346026346 and AI handoff #45346026565 were queued at first observation; verify their final outcomes.
- Exact 6130ebd28962da5bf55439349632175dfc0d858e: DNSMasq #45345435699 PASS; PID1 notification queue #45345435667 PASS; Process inventory #45345435999 PASS; HTTPD #45345435733 PASS; PID1 mdev security #45345435825 PASS; DNS forwarder #45345435708 PASS; RC timezone #45345435849 PASS; USB storage #45345435532 PASS; NVRAM transaction audit #45345435848 PASS; AI handoff #45345435583 PASS. Process fail-closed #45345435889 and AliyunDrive #45345435736 failed only on the obsolete negative checker assertion, corrected in follow-up.
- Exact ZeroTier commit 372400813c7d0888c508d6e6595353c46ea5d960: ZeroTier lifecycle #45344943818 PASS; Process inventory #45344943892 PASS; WAN route #45344943905 PASS; AI handoff #45344943861 PASS. XY-C1 #45344943825 was still in progress at Build; no firmware L3 claim.
## Exact first action for the next GPT

1. Verify exact-SHA USB modem identity Actions for the QMI PDH parser/start-network status change, including the GNU89 modem WAN policy regression.
2. Check exact Process inventory, AliyunDrive, HTTPD and AI handoff reruns for 6904523; resolve any remaining failure.
3. Poll XY-C1 Build/TRX/artifact for the current production source SHA. Do not claim firmware L3 without all same-SHA steps passing.

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
