# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main SHA for this checker correction: e01f70a766c4e628593e272192723f6add59ffca. The live main for future writes must be resolved again.
- Exact-head on the parent: VPN server lifecycle 44613173654 PASS; OpenVPN client credential staging 44613173756 FAIL only in the newly added source assertion because the tls-crypt-v2 key call spans two lines and the checker expected one line. Its four compiled policy regressions passed. AI handoff, HTTPD, process inventory, NVRAM and XY-C1 were still in progress at the first status query.
- Parent's source implementation introduced private atomic export publication, no-follow key reads, generated-key error checks and owner-only staging; do not revert those controls.

## Active verification correction — multiline OpenVPN key assertion

- The exact-head OpenVPN job log pinpoints the checker mismatch: all four host compiled tests passed, previous source/workflow contracts passed, then the checker failed to find a one-line spelling of the tls-crypt-v2 call. The production call is split over two C lines; only the textual assertion needs correction. No source defect was reported by this failure.
- The correction asserts the unique second-line argument after the preceding chmod and before temporary directory cleanup.

## Exact first action for the next GPT

1. Check exact-head OpenVPN client credential staging, VPN server lifecycle, AI handoff docs and XY-C1 Actions for the corrected checker commit.
2. If the contract passes, continue reviewing the next independently evidenced P1 stability/security defect; if not, inspect the exact failing line/log and repair the contract without weakening the secure export.
3. Keep the OpenVPN source changes until exact-head XY-C1 and VPN workflow compile gates pass. Hardware OpenVPN behavior remains NOT VERIFIED.

## Verification boundary

- L1: source hardening committed at e01f70a766c4e628593e272192723f6add59ffca.
- L2: host tests for client secret file, config atomic publication, VPN lease snapshot and lease lock all PASS in run 44613173756. The newly added source assertion failed because of multiline formatting; correction is the only changed checker behavior.
- L3: exact-head XY-C1 run status was IN_PROGRESS at initial inspection; no new firmware pass claim yet.
- L4: physical OpenVPN export permissions and imported client connection remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with the exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
