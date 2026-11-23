# SESSION HANDOFF — Padavan 4.4

## Current repository state

- Development branch: `main` only. No development feature branches/PRs and no force-push.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. Preserve permanently.
- Current source checkpoint: `f0ee5131e495a7b9f4a903293d53d304a3ec830b` (RC PID1 SIGCHLD errno preservation). Current tip before this formatting fix: `00cb20522874510b126cc647efc7afcb1d2952f4`.
- The active source task is the real RC PID1, `trunk/user/rc/src/init.c`. The board BusyBox config has `# CONFIG_INIT is not set`; generic BusyBox init applet improvements are not current firmware PID1 behavior.

## Current PID1 source change

`f0ee5131e495a7b9f4a903293d53d304a3ec830b` adds `pid1_signal_reap_available_children()` in `trunk/user/rc/src/pid1_signal_policy.h`. The helper saves errno, retries EINTR, records each exact PID returned by `waitpid(-1, NULL, WNOHANG)`, then restores errno before returning to interrupted PID1 code. `catch_sig_reap()` delegates to it. An injected host test covers unrelated and tracked-child reaps, EINTR, terminal ECHILD and original errno restoration.

The PID1 host regression passed in GitHub Actions with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. The test was run by PID1 mdev/security Action `45086111693` on exact source SHA f0ee5131....

## Exact SHA results for f0ee5131e495a7b9f4a903293d53d304a3ec830b

PASS:
- PID1 mdev/security `45086111693`
- PID1 notification queue `45086111684`
- USB storage hotplug `45086111681`
- DNS forwarder `45086111683`
- DNSMasq lifecycle `45086111735`
- Process lifecycle inventory `45086111753`
- Process inventory/lifecycle umbrella `45086111764`
- AI handoff documentation `45086111726`
- NVRAM transaction audit `45086111754`

FAIL:
- HTTPD security `45086111706`: its embedded source checker expected waitpid/EINTR inline in `catch_sig_reap()`, although the loop was moved into the errno-preserving helper. A first correction in commit `00cb20522874510b126cc647efc7afcb1d2952f4` updated the contract but missed YAML's embedded Python indentation; run `45086402010` failed before job creation (`total_count=0`). The present change fixes indentation. Confirm subsequent HTTPD run by matching exact head SHA.
- XY-C1 firmware validation `45086111757`: still IN_PROGRESS at last poll; recheck actual run metadata.

These results belong only to f0ee5131... and do not validate later descendants.

## Other completed slices

- DNSMasq split-DNS checked snapshot and fail-closed publication: `d73ba6d2b9df959b73f0c0cf283d9694362b85c3`; full snapshot zero-on-copy-error `b642c460539c93e71c6b05a7b913a964e4cdd44a`; HTTPD diagnostic compatibility restoration `4fad5ae2ac97a35d79031c3a18e828702d010109`; DNSMasq lifecycle exact-head run `45085247073` on `9b1d0a045de4355f670ce0d771e22c543a92f33e` PASS.
- Storage restore symlink guard: `3adf4cc9d94bd930b6504b0dd11cc1fa109852fd`.
- SCUTClient credential-file work completed in previous commits; don't repeat absent a newly demonstrated defect.

## First next task

1. Fetch live main and branch inventory once; verify the immutable backup still matches.
2. Check the HTTPD and AI handoff workflows triggered by the indentation correction and match each run's exact head SHA. Check XY-C1 run `45086111757` only against its recorded source SHA.
3. When HTTPD passes, continue a targeted RC PID1 signal/lifecycle review and add focused injected regression coverage for the next demonstrated defect. While workflows are pending, continue independent stability work.
4. Keep commits atomic: create one complete tree, commit against the exact parent, and update `main` with `force:false` and a freshly fetched `expected_sha`.

## End-of-session accuracy checkpoint

- Source change under verification: `f0ee5131e495a7b9f4a903293d53d304a3ec830b`.
- Current pending change fixes indentation in `.github/workflows/httpd-security.yml` and refreshes this handoff and log.
- Verified host regression: PID1 injected waitpid/errno test PASS on source SHA f0ee5131....
- Not verified: local checkout/build; MIPS firmware for current SHA; TRX; artifact upload; XY-C1 boot/physical hardware.
- Never attribute parent-SHA Actions to a descendant commit.
