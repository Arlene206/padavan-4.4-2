## Current repository state

- Active branch: `main`. Latest source parent for this handoff checkpoint: `21ca0c368ea6968823a6c1eca6ecfad8766f7212` (`fix(frp): reject unsafe crontab directories and files`). This handoff-only commit is placed on top of that source commit; always re-read live main before editing.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- VSFTPD strict EPSV numeric parser: source/test/workflow commit `974315bb0113d5fbc294a716579edceeb7b5d68f`; exact VSFTPD #45372034160, AI handoff #45372034407, Process inventory #45372034153 PASS.
- VSFTPD EPSV ALL EPRT restriction: source/workflow commit `1b504eeda669d6c71df32ae3692adb99ffcaf13a`; exact VSFTPD #45372453417 PASS. The missing EPRT `!epsv_all` dispatcher gate is now present; PASV/PORT already had the guard. Its handoff contract run #45372453431 failed only because the previous handoff heading had `## Completed task` rather than the validator-required `## Active task`; this handoff checkpoint restores that heading.
- Current FRP crontab hardening: source/test/checker commit `21ca0c368ea6968823a6c1eca6ecfad8766f7212`. It rejects unsafe or symlinked cron ancestors, wrong ownership, group/other-writable modes, unsafe existing crontab files and hardlinks before publishing; regression coverage includes the unsafe directory/file cases. Exact FRP lifecycle #45372520302 PASS; Process inventory #45372520381 was IN_PROGRESS at last check.
- RAETH forced-mode indirect MMD page/data transaction: source `e721cb0e2f42f22a8189937ac2311d246ea9912c`, test include-path correction `542b8f7e38a30dca616db877a1757d22b1b19060`; exact Switch CLI regression #45363906601 and AI handoff #45363906553 PASS.
- FRP procfs read and newline-spoofing corrections: `0104186915ee6132d99871bede241ecf1835c279` and `5ca9761969591766c583d3146cf5f32155ab7891`; exact FRP lifecycle #45361194679, AI handoff #45361194624 and Process inventory #45361194576 PASS.
- VSFTPD SITE CHMOD/UMASK strict-octal validation: `ea7d7e28be9988175fbcc423f9378e5d765de764`, checker correction `0c23ad7c82fc2e033f73d4a94f6fb02a77324ca5`; exact VSFTPD #45359061300 PASS.
- Samba per-share ACL snapshot `d8f6092c6af34f1c8ec45c746ce9411e40522577`; HTTPD source-contract correction `3026ffe460fe9d01ad5b72312978ff957b400479`, exact HTTPD #45363047352 PASS.
- QMI client-ID workflow heredoc correction `4985f33bda8360f4dfb92581b7b7d47d7850a03c`; exact USB modem #45363094446 PASS.
- XY-C1 L3 firmware build/TRX/artifact verification is valid only for production-source baseline `ea7d7e28be9988175fbcc423f9378e5d765de764`: Build/TRX/artifact #45358712652, Process #45358712716, NVRAM #45358712663 PASS. No current later source SHA has a completed firmware-build claim. No physical L4 validation is claimed.

## Active task — Close the handoff validation and verify current FRP checks

- Production code and regression for the EPSV numeric-selector fix and the EPSV ALL/EPRT dispatcher guard are complete; exact VSFTPD Actions passed on `974315bb` and `1b504eed`.
- The documentation-only checkpoint `2593bd3bf54d3ddf670ac2bc17e3ac0d491a1e72` failed AI handoff #45372274258 because the validator requires the literal heading `## Active task`. The next fix preserves that mandatory heading; no firmware/runtime source change is involved.
- The latest FRP crontab source/test/checker slice is complete on `21ca0c368ea6968823a6c1eca6ecfad8766f7212`; FRP lifecycle #45372520302 PASS. Its Process inventory exact run #45372520381 was still in progress.
- Continue the normal loop after closing those exact checks; PENDING is not a reason to stop independent work.

## Verification boundary

- Exact `21ca0c368ea6968823a6c1eca6ecfad8766f7212`: FRP asynchronous startup lifecycle #45372520302 PASS; Process lifecycle inventory #45372520381 IN_PROGRESS at last poll.
- Exact `1b504eeda669d6c71df32ae3692adb99ffcaf13a`: VSFTPD stability #45372453417 PASS; AI handoff #45372453431 FAIL solely due the required section heading; NVRAM #45372453247 and XY-C1 firmware #45372453296 were IN_PROGRESS.
- Exact `974315bb0113d5fbc294a716579edceeb7b5d68f`: VSFTPD stability #45372034160 PASS; AI handoff #45372034407 PASS; Process inventory #45372034153 PASS. NVRAM #45372034333 and XY-C1 #45372034143 were IN_PROGRESS at last poll.
- The VSFTPD regression workflow compiled and ran its C policy suite, including the EPSV parser regression, and its inline contract passed on both relevant source commits. This runtime could not clone GitHub due DNS resolution failure; no separate local compile is claimed.
- Current FRP work has no firmware or physical-router result claimed. Verify Build, TRX and artifact on the same exact SHA before asserting firmware validation.

## Exact first action for the next GPT

1. Inspect the AI handoff Action triggered by this corrected handoff checkpoint. The prior failure was the missing required `## Active task` title; fix any further exact assertion failure without changing source behavior.
2. Refresh Process inventory #45372520381 on source SHA `21ca0c368ea6968823a6c1eca6ecfad8766f7212`; resolve any actual failure, but do not wait on a pending run.
3. Refresh NVRAM and XY-C1 exact checks for the latest source SHA. Do not claim current firmware success unless Build/TRX/artifact all pass on that identical SHA.
4. Then select a fresh concrete P1 stability/security bug outside the already-closed EPSV slices. Keep the frozen backup immutable and advance main only via complete-tree atomic commits with non-forced expected-SHA compare-and-swap.


# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change must land as one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task.
