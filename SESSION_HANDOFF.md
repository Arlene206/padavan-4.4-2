# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. Never use feature branches or PRs for development; never force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve permanently; never modify, rename, delete or check it out.
- Source parent before this workflow-contract correction: `f0ee5131e495a7b9f4a903293d53d304a3ec830b`. The current commit updates an HTTPD workflow assertion and the handoff/log only; obtain the new SHA from the resulting commit.
- Continue implementing rather than repeating completed audits or asking what to do next.

## Firmware PID 1 target

The actual Padavan firmware PID 1 is the RC init executable, `trunk/user/rc/src/init.c`. The board config `trunk/configs/boards/busybox.config` explicitly has `# CONFIG_INIT is not set`. BusyBox applet changes under `trunk/user/busybox/busybox-1.24.x/init/init.c` are generic and are not the running router PID 1. The deferred-reap BusyBox improvement in commits `bde45fc2ee6fb6e18669c944b4fe2a1d9dcd5189`, `629cf55f5f8d1476f585e0c4e30ad2670c2f9806`, and `93e3dd84aa09a2f6d0e4666530bcc3e1a932f5f2` must not be claimed as firmware-runtime behavior.

## Active RC PID 1 source slice

Commit `f0ee5131e495a7b9f4a903293d53d304a3ec830b` fixed the SIGCHLD handler's errno clobber:
- Before the fix, `catch_sig_reap()` looped on `waitpid(-1, NULL, WNOHANG)` and left errno set to terminal `ECHILD`/other error.
- SIGCHLD can interrupt an unrelated libc/system call in PID 1, so the handler must preserve the interrupted code's errno.
- Added `pid1_signal_reap_available_children()` to `pid1_signal_policy.h`; it saves errno, retries EINTR, records returned child PIDs and restores errno.
- `catch_sig_reap()` delegates to that helper.
- Added an injected waitpid regression for an unrelated child, EINTR, tracked console-shell reap, terminal ECHILD and caller errno restoration.

The host C regression passed in exact-head CI with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. No local checkout/build was run in this connector-only session.

## Exact-head Actions for `f0ee5131e495a7b9f4a903293d53d304a3ec830b`

- PID1 mdev hotplug/security run `45086111693`: PASS.
- PID1 notification queue `45086111684`: PASS.
- USB storage hotplug `45086111681`: PASS.
- DNS forwarder launch path `45086111683`: PASS.
- DNSMasq lifecycle `45086111735`: PASS.
- Process lifecycle inventory `45086111753`: PASS.
- Process inventory/lifecycle umbrella `45086111764`: PASS.
- AI handoff documentation `45086111726`: PASS.
- HTTPD Authentication/Security `45086111706`: FAIL only because it still expected the waitpid loop inside `catch_sig_reap()`; this atomic correction moves the contract to the tested policy helper.
- NVRAM transaction audit `45086111754` and XY-C1 Firmware Validation `45086111757`: IN_PROGRESS at last poll. Recheck exact `head_sha`; do not transfer those statuses to descendant commits.

## Prior completed fixes

- DNSMasq split-DNS checked NVRAM snapshot: source `d73ba6d2b9df959b73f0c0cf283d9694362b85c3`; full snapshot zeroing `b642c460539c93e71c6b05a7b913a964e4cdd44a`; legacy HTTPD diagnostics `4fad5ae2ac97a35d79031c3a18e828702d010109`; DNSMasq exact-head run `45085247073` on `9b1d0a045de4355f670ce0d771e22c543a92f33e`: PASS.
- Storage restore symlink guard: `3adf4cc9d94bd930b6504b0dd11cc1fa109852fd`.
- SCUTClient credential-file work is already implemented on main; see `dev_log.md` and commits `ea5264dfbdb9470bc29971dee053503e8177b3aa`, `b30aeaf66d022f53061fd3433e98277131cbeda8`. Do not restart absent a newly proven defect.

## First next task

1. Fetch live `main` and branch inventory once; confirm frozen backup remains unchanged. Match new workflow runs to the exact correction SHA.
2. Confirm HTTPD's PID1 contract passes on the new exact SHA. Recheck NVRAM audit and XY-C1 only against their exact SHA.
3. Continue stability/security work in actual RC PID 1. Inspect signal handling, child-state transitions, shutdown sequencing and console-shell lifecycle for a concrete failure mode; add focused injected regression coverage before behavior changes. Continue independent core-service work while runs are pending.
4. Use complete Git trees and one atomic commit per logical task. Update `main` with `force:false` and a fresh `expected_sha`; never weaken tests merely to obtain PASS.

## End-of-session accuracy checkpoint

- Last source SHA: `f0ee5131e495a7b9f4a903293d53d304a3ec830b`.
- Current correction: HTTPD's embedded PID1 source contract only; update SHA and Action status after commit.
- Verified: injected RC PID1 waitpid/errno host regression passed on exact source SHA f0ee5131... in the PID1 mdev/security workflow.
- Not verified: local build, current-SHA MIPS firmware, TRX validation, artifact upload, XY-C1 router boot, or physical hardware.
- Never attribute any ancestor SHA's Action result to a descendant.

