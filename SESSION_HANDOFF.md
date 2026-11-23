# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: `f0ee5131e495a7b9f4a903293d53d304a3ec830b` (active RC PID1 SIGCHLD errno fix). Latest main before this handoff-only correction: `fff2b43aa36e71dbe2b3bbda4c74079799419e59`.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The active source area is the actual Padavan RC PID 1, `trunk/user/rc/src/init.c`, not the BusyBox init applet. The board config `trunk/configs/boards/busybox.config` explicitly has `# CONFIG_INIT is not set`, so generic BusyBox init code is not built as the router's PID 1.

Source commit `f0ee5131e495a7b9f4a903293d53d304a3ec830b` fixes SIGCHLD errno clobber:
- Added `pid1_signal_reap_available_children()` in `trunk/user/rc/src/pid1_signal_policy.h`. It saves errno, retries EINTR, records exact PIDs returned by `waitpid(-1, NULL, WNOHANG)`, then restores the interrupted caller's errno.
- `catch_sig_reap()` delegates to this helper.
- Added an injected host regression for an unrelated child, EINTR, tracked console-shell reap, terminal ECHILD and original errno restoration.
- Exact-head PID1 host test passed in CI with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan.

The current follow-up fixes the HTTPD workflow's stale contract to assert the helper call and helper implementation rather than requiring the waitpid loop inline in `catch_sig_reap()`. HTTPD run `45086487150` on source head `fff2b43aa36e71dbe2b3bbda4c74079799419e59` passed.

## Verification boundary

Exact-head runs on source SHA `f0ee5131e495a7b9f4a903293d53d304a3ec830b`:
- PID1 mdev/security `45086111693`: PASS.
- PID1 notification queue `45086111684`: PASS.
- USB storage hotplug `45086111681`: PASS.
- DNS forwarder `45086111683`: PASS.
- DNSMasq lifecycle `45086111735`: PASS.
- Process lifecycle inventory `45086111753`: PASS.
- Process inventory/lifecycle umbrella `45086111764`: PASS.
- AI handoff documentation `45086111726`: PASS.
- NVRAM transaction audit `45086111754`: PASS.
- HTTPD `45086111706` failed on a stale source assertion, then its assertion was corrected. The HTTPD run `45086487150` on descendant head `fff2b43aa36e71dbe2b3bbda4c74079799419e59` passed after the corrected source contract and its indentation repair.
- XY-C1 firmware validation `45086111757` was still in progress at the last poll; do not assume its result.

AI handoff documentation run `45086487184` on `fff2b43aa36e71dbe2b3bbda4c74079799419e59` failed because this handoff lacked required cross-session/mandatory-checkpoint contract sections. The current edit restores those required headings and exact protocol wording. Verify the new docs Action on its own SHA.

No local checkout or compile was run in this connector-only session. MIPS firmware build, TRX validation, artifact upload and physical XY-C1 boot are not verified for the current main head.

## Previously completed fixes

- DNSMasq split-DNS checked snapshot and fail-closed publication: `d73ba6d2b9df959b73f0c0cf283d9694362b85c3`; full snapshot zero-on-copy-error: `b642c460539c93e71c6b05a7b913a964e4cdd44a`; diagnostic compatibility restoration: `4fad5ae2ac97a35d79031c3a18e828702d010109`; DNSMasq exact-head run `45085247073` on `9b1d0a045de4355f670ce0d771e22c543a92f33e`: PASS.
- Storage restore symlink guard: `3adf4cc9d94bd930b6504b0dd11cc1fa109852fd`.
- SCUTClient credential-file changes were completed earlier; see `dev_log.md` and commits `ea5264dfbdb9470bc29971dee053503e8177b3aa`, `b30aeaf66d022f53061fd3433e98277131cbeda8`. Do not restart without a newly demonstrated defect.

## Exact first action for the next GPT

1. Fetch live `main` and branch inventory once; confirm frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Verify AI handoff documentation on the exact new head; verify HTTPD already passed only on its own exact head `fff2b43...`. Recheck XY-C1 run `45086111757` using its recorded exact SHA.
3. Continue with a concrete RC PID1 signal/lifecycle defect. Inspect signal-handler state transitions, child lifecycle, shutdown sequencing and console-shell behavior; add focused injected regression coverage before modifying behavior. Work on independent stability/security issues while Actions are pending.
4. For every logical source fix, create all blobs and one complete tree, make one atomic commit, then update `main` with `force:false` and a freshly read `expected_sha`. Inspect only Actions whose `head_sha` exactly matches the commit.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation SHA: `f0ee5131e495a7b9f4a903293d53d304a3ec830b`.
- Current docs correction parent: `fff2b43aa36e71dbe2b3bbda4c74079799419e59`; this commit restores required AI handoff contract headings/wording and logs the prior docs failure.
- Verified: RC PID1 injected waitpid/errno host regression passed on exact source SHA f0ee5131...; HTTPD workflow passed on exact SHA fff2b43....
- Not verified: local checkout/build, MIPS firmware build on current head, TRX, uploaded artifact, XY-C1 hardware boot or physical router behavior.
- Parent-SHA PASS results never validate descendant commits.
