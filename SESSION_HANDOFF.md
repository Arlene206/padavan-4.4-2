# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Do not create feature branches or PRs; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve it permanently; do not modify, rename, delete or check it out.
- Current source checkpoint: `f0ee5131e495a7b9f4a903293d53d304a3ec830b` (active RC PID1 SIGCHLD errno fix). Latest main before this handoff-only correction: `fff2b43aa36e71dbe2b3bbda4c74079799419e59`.
- Continue implementation rather than repeating completed audits or asking what to do next.

## Active task

The RC PID1 console-shell recovery path previously respawned an exact tracked child immediately after a quick shell exit. A failed/missing shell executable could spin PID 1 through repeated fork/exec. Commit `36bd2b1e01c0472b7d8fdc0a31e4a56e3b6762de` adds an exact one-shot `console_shell_reaped` event and a one-second throttle only for shells running <2 seconds, with GNU89/ASan/UBSan host coverage.

Exact push run `45087584585` on that SHA failed only at the newly added test's compiler command: the inserted workflow continuation characters were doubled. Existing mdev config/retry/TUN/boot-device and PID1 signal tests before that step passed. This checker/workflow correction removes the extra backslashes; no production source change is needed for this failure. Verify its exact-head rerun.

## Exact-head results for `36bd2b1e01c0472b7d8fdc0a31e4a56e3b6762de`

PASS:
- AI handoff docs `45087584565`
- HTTPD Authentication/Security `45087584572`
- PID1 notification queue `45087584511`
- DNS Forwarder `45087584533`
- DNSMasq lifecycle `45087584507`
- Process lifecycle inventory `45087584547`

FAIL:
- PID1 mdev hotplug configuration security regression `45087584585`: all earlier GNU89/ASan/UBSan binaries including `pid1_signal_policy` passed, but the new console test compile failed because its YAML block contained doubled shell continuation backslashes, producing linker input `\\`. Correcting the new step only; rerun exact SHA required.

PENDING at last poll:
- XY-C1 Firmware Validation `45087584681` (building through LAN IPv6 regression; Build/TRX/artifact not yet observed).
- Process inventory/fail-closed `45087584469` (LAN startup caller regression).
- NVRAM transaction audit `45087584527` (raw userland ioctl audit).
- USB storage hotplug `45087584548` (BusyBox build/Optware lifecycle ash run).

All listed runs are push events and have `head_sha` exactly `36bd2b1e01c0472b7d8fdc0a31e4a56e3b6762de`. Do not transfer these results to a later commit.

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

1. Fetch live `main` and branch inventory once; keep frozen backup `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807` immutable.
2. Inspect the exact-head PID1 mdev run triggered by the corrected console-test compiler command; confirm the full suite reaches the source-contract step and passes. Its previous failure on `36bd2b1` was solely the doubled backslash; do not mark the source fix PASS until the new exact SHA run is green.
3. Continue a focused actual RC PID1 review only after the console test passes; preserve one-shot signal-context event semantics and add regression coverage for any concrete child/shutdown defect. Prioritize NVRAM raw-ioctl audit and storage worker lifecycle results for independent follow-up while Actions are pending.
4. Inspect XY-C1 run `45087584681` for Build XY-C1, TRX and artifact upload on its own exact SHA `36bd2b1`; never infer L3 from host tests.
5. For each logical slice, create one complete tree and one atomic commit, then update `main` with `force:false` and freshly fetched exact `expected_sha`. Inspect only push Actions whose `head_sha` equals that commit.
## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every continuation must leave the next GPT an accurate current SHA, exact verification boundary, first actionable task, and test/CI state. Do not churn documentation when the existing handoff remains accurate.

# Cross-session execution mandate

Continue the same development effort across conversations. Use one atomic main commit per logical source task, with complete Git trees and a non-forced compare-and-swap ref update. Before context exhaustion, close the current safe slice, record the actual commit SHA and exact Actions state, and leave the next GPT a concrete first task. Do not ask the user what to do next or wait on pending workflows when independent work can proceed.

## End-of-session accuracy checkpoint

- Current implementation SHA: `36bd2b1e01c0472b7d8fdc0a31e4a56e3b6762de` (rapid recovery console-shell respawn throttle).
- Exact-head AI docs, HTTPD, PID1 notification queue, DNS Forwarder, DNSMasq and Process inventory Actions passed on that SHA. PID1 mdev failed only at the added console regression compile command (extra backslashes); exact-head correction pending. XY-C1, Process umbrella, NVRAM and USB storage were in progress at last poll.
- The new PID1 signal-state and console retry regression code is present, but the console policy test did not compile in that run. Existing PID1 signal policy regression did compile/pass.
- Not verified: local checkout/build; current-head MIPS firmware build, TRX, artifact, XY-C1 hardware boot or physical router behavior.
- Parent-SHA PASS results never validate descendant commits.
