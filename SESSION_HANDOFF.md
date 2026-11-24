## Current repository state

- Live parent main before this atomic change: `bc34d1db08a86f762ffad75bec8c8276629f29c3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

ZeroTier daemon-stop ownership hardening:
- The existing stop path first used `pidof zerotier-one` as its process inventory and then used name-wide `killall zerotier-one` / `killall -9 zerotier-one`. A process can set its mutable comm name to `zerotier-one` without executing the ZeroTier binary, so this could signal unrelated same-name processes.
- Replace name matching with tri-state procfs inventory that requires `/proc/PID/exe` to resolve to the installed `/usr/bin/zerotier-one` and argv[0] to match the launcher path. Unknown/incomplete metadata blocks stop/restart. Re-enumerate and revalidate each PID immediately before TERM/KILL, retaining bounded waits and UNKNOWN fail-closed escalation.
- Expand `zerotier_lifecycle.sh` with synthetic procfs cases for a same-name impostor, mismatched argv[0], incomplete executable metadata, unavailable proc root, graceful TERM, KILL escalation and stuck-daemon failure. Exact ZeroTier lifecycle and AI handoff Actions must be checked for the resulting commit.
- Linux 4.4 lacks pidfd; the residual validation-to-signal race remains documented and is not claimed eliminated.

## Verification boundary

- Exact bc34d1db: FRP asynchronous startup lifecycle regression #45343199725 PASS; AI handoff documentation contract #45343199794 PASS. The earlier FRP source-contract mismatch is closed; do not repeat the FRP audit.
- Exact c8985b2b: FRP #45343036173 passed the executable FRP startup, crontab, boot/config, process-stop and lock regressions, then failed only on one test-message mismatch in check_frp_lifecycle.py. This commit aligns the expected message with the actual test while retaining its UNKNOWN and no-signal checks. AI docs #45343036253 was IN_PROGRESS at last poll.
- Exact ebbd4dd: AI handoff #45342815724 PASS; FRP #45342815755 passed all executable regressions and failed only on the obsolete "verified timeout" checker literal, fixed in c8985b2b.
- Exact 4be3f389: Process inventory #45342556827 PASS; FRP executable tests passed before the stale proc-path checker failure; AI docs failure was corrected in ebbd4dd.
- Exact 257ac639: Inadyn #45342407697 PASS; DNSMasq #45342407692 PASS; WAN route #45342407753 PASS.
- Exact 79c5389: Process inventory #45341769713 PASS; Process fail-closed #45341769769 PASS; HTTPD #45341769686 PASS; WAN DHCP #45341769830 PASS; miniupnpd #45341769855 PASS; Samba #45341769858 PASS; NVRAM audit #45341769831 PASS. Inadyn/DNSMasq/WAN checker failures were corrected in 257ac639.
- XY-C1 #45341769654 on production source 79c5389 remained IN_PROGRESS at Build; TRX/artifact are unconfirmed. No L3 claim.
- No MIPS firmware/TRX/artifact or physical-router L4 result is claimed for this current source. Linux 4.4 has no pidfd; final procfs identity-validation-to-signal race remains.

## Exact first action for the next GPT

1. Check the exact-head ZeroTier lifecycle and AI handoff Actions for the exact-executable stop change. Triage the first failing assertion and correct it before proceeding.
2. Once those Actions pass, continue the next independent P1 service stability/security defect; do not repeat FRP or completed process-identity audits.
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
