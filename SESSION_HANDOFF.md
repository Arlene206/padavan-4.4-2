## Current repository state

- Live parent main before this atomic change: `5de929ef21c09630adffaaeff06f057afb2ba704`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Inadyn executable-bound stop/reload/manual-update implementation is in `79c5389fe33326d85cc23e9856f39e5ca0ac242f`. Its dependent source-contract correction is in `257ac6399c4c7e8ec3b518f8f8438532d0821c33`.
- FRP process inventory/stop hardening is in `fa3dd0012cea550b339d01df4ff6ec97d0fdc00a`; its test follow-ups are `5687c609137bbba09c707896dd95216c3df9e961` and `4be3f3898faf4665ebe19b1e87cf385bd162c21e`.

## Active task

QMI ambiguous-start retry and recovery-state preservation:
- QMI startup rejects stale/unparseable PDH files, checks uqmi --start-network, strictly parses one PDH token, requires command success plus valid handle, and atomically publishes the PDH.
- A successful uqmi process result without a valid persisted PDH is ambiguous because the void WDS callback may have failed to publish the handle while the modem did create the session. A failed start with a valid PDH is also ambiguous; both abort retries. A failed start without a PDH remains retryable.
- New recovery-state defect: the ambiguous failed-command/valid-PDH path unlinked /tmp/qmi-handle before exiting. The modem could remain live while its only recovery handle was erased. This slice preserves a valid PDH when the result is ambiguous and only unlinks malformed/missing PDH state. GNU89 regressions cover this preservation matrix; the workflow asserts the preservation decision precedes unlink.
- Parent 5de929 exact Actions: USB modem identity #45347628860, AI handoff #45347628915, HTTPD #45347628767, WAN teardown #45347629033, WAN DHCP callback #45347628857, detect_link #45347628882 and Process inventory #45347628829 PASS. Process fail-closed #45347628853, NVRAM #45347628791 and XY-C1 #45347628749 were still running. XY-C1 Build/TRX/artifact were pending; no firmware L3 or hardware claim.

## Verification boundary

- Exact e006c68f711c0fc47e6d46d7f34b3c337a06f7da: NVRAM audit #45347148992, Process inventory #45347149044, AI handoff #45347148865, XY-C1 firmware #45347148972 and USB modem identity #45347148897 were IN_PROGRESS/QUEUED at last observation. Check final statuses before relying on them.
- Exact 42257289849aa454630dfda18b8e1efc1a213411: Process inventory #45346865350, HTTPD #45346865450 and AliyunDrive WebDAV #45346865525 all completed SUCCESS after removing the stray malformed checker line from `scripts/check_dev_init_boot.py`.
- Exact 1dc62d8e8f99a40b2447cce57e77781e3e155827: USB modem identity #45346595809 PASS, including QMI status/PDH validation; AI handoff #45346595807 PASS. Process lifecycle #45346595881 failed only on the malformed MTD checker line later removed by 4225728. XY-C1 #45346595827 remained IN_PROGRESS at last observation.
- Exact 372400813c7d0888c508d6e6595353c46ea5d960: ZeroTier lifecycle #45344943818 PASS; Process inventory #45344943892 PASS; WAN route #45344943905 PASS; AI handoff #45344943861 PASS.

## Exact first action for the next GPT

1. Verify exact-head USB modem identity and GNU89 regression for preserving /tmp/qmi-handle on ambiguous failed starts.
2. Poll parent 5de929 Process fail-closed, NVRAM and XY-C1 outcomes, checking same-SHA Build, TRX and artifact separately.
3. Continue another independent userland stability/security defect after QMI recovery-state preservation passes.

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
