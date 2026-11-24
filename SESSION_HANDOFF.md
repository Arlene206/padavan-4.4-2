## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update `main` with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Branch and checkpoint

- Parent main SHA for this handoff update: `05356ed622d1762e5e251c6adeb5fcd7be7713b6`; refresh live refs before future writes.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- `main` is the only development ref. PR #1 is closed/unmerged and must not be ported.
- Bootstrap, protocol and handoff were recovered once; do not repeat startup audits without a new reason.

## Active task

### Reject unsupported SSH daemon mode values

`sshd_nvram_mode_status()` strictly parsed an integer but accepted every value in the C `int` range. `start_sshd_with_mode()` handles only 1 (Dropbear) and 2 (OpenSSH); other values returned success without starting a daemon, while negative values silently acted like disabled mode. A corrupt `sshd_enable=3` could therefore falsely report successful startup with no SSH listener.

The parser now rejects values outside 0..2 with `EINVAL` and leaves the output untouched. GNU89 regression cases cover -1, 3, INT_MIN and INT_MAX with sentinel preservation. The Process Lifecycle Regression workflow already filters the policy header, test and exact-source checker, and compiles the test with `-Wall -Wextra -Werror -fsanitize=address,undefined`. Parent main SHA: `05356ed622d1762e5e251c6adeb5fcd7be7713b6`; exact-head Process Lifecycle, AI docs and applicable XY-C1 validation must be checked after this atomic commit.

## Completed source slices

### Firewall virtual-server optional translation-port reads

Commit `6d5d5376e425587e6f116d09f88cbcc7ac6a05c1` fixed `include_vts_filter()` and `include_vts_nat()` conflating failed `vts_lport_x` reads with absent/empty optional values, which could publish the wrong internal-port mapping. Missing/empty settings keep the legacy external-port fallback; backend and malformed results abort incomplete staging to retain the previously published ruleset. Firewall Startup Stability #45503784466 PASS on checker-fix SHA `2be0e5125bb29f01fc3a9a15fe8938a554387452`, including the VTS policy test compile/run and exact-source contract.

### OpenVPN client certificate lifetime arguments

Commit `d096f168bfe4188f5223bf76806b4ea012742580` replaces privileged `atoi(argv[3])` parsing with strict positive decimal days (1–36,500); optional/empty input keeps the 365-day default and legacy whitespace/leading-plus support. Invalid syntax, negative/zero, suffix, fractions, range and overflow fail before side effects. VPN lifecycle #45502092649 and AI docs #45502092664 PASS on d096.

### USB modem reload retry state

Commit `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` publishes modem rule/type caches only after driver readiness; failure retains the previous baseline and blocks WAN restart so a queued retry can retry. USB caller contract #45499733421 PASS on d2fa; PID 1 queue #45500981047 and AI docs #45500980887 PASS on 8ebf.

### WAN traffic persisted-baseline parsing

Commit `786c504c8247fe42d5d1d5f2dc3ad28757fafc80` strictly parses persisted RX/TX byte baselines instead of accepting numeric-prefix junk. WAN regression #45498624341 PASS.

## Verification boundary

### XY-C1 exact-source results

- SHA `786c504c8247fe42d5d1d5f2dc3ad28757fafc80`: #45498624157 Build, TRX and artifact upload PASS.
- SHA `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a`: #45499228265 Build, TRX and artifact upload PASS.
- SHA `d096f168bfe4188f5223bf76806b4ea012742580`: #45502092732 PENDING at last poll; Build active, TRX/artifact steps pending. This does not cover the later firewall source change.
- SHA `6d5d5376e425587e6f116d09f88cbcc7ac6a05c1`: #45503440356 PENDING at last poll; Build active.

### d096f168bfe4188f5223bf76806b4ea012742580 exact-head Actions

- VPN lifecycle #45502092649: PASS.
- AI handoff docs #45502092664: PASS.
- NVRAM boundary audit #45502092721: PASS; zero raw userland bypasses, kernel snapshot/caller contract passed.
- Process lifecycle inventory #45502093101: PASS.
- IPv6 service checked-read #45502092750: PASS.
- HTTPD authentication/security #45502092764: PASS.
- OpenVPN client credential staging #45502092820: PASS.

### Firewall / checker SHA 2be0e5125bb29f01fc3a9a15fe8938a554387452

- Firewall Startup Stability #45503784466: PASS. The exact-source checker and GNU89 ASan/UBSan VTS policy test passed, together with the other firewall policy checks.
- AI handoff documentation #45503784498: FAIL because the handoff omitted the validator's exact mandatory-checkpoint sentence. Commit #05356ed622d1762e5e251c6adeb5fcd7be7713b6 restores it; rerun #45504014822 was in progress at the last poll.
- XY-C1 #45503440356 remains PENDING on older product-source SHA 6d5d5376e425587e6f116d09f88cbcc7ac6a05c1; it does not validate later SSH source.
- NVRAM audit #45503440350, WAN route #45503440493, IPv6 service #45503440453, AI docs #45503440345, WAN DHCP #45503440341, process inventory #45503440565, HTTPD #45503440466 and FULLCONENAT #45503440566: PASS on SHA 6d5d5376e425587e6f116d09f88cbcc7ac6a05c1.
- No target MIPS/uClibc runtime or physical-router L4 validation is claimed.

## Exact first action for the next GPT

1. Inspect exact-head Process Lifecycle Regression and AI handoff documentation runs for the SSH mode validation commit. Confirm the GNU89 ASan/UBSan test, exact-source contract and handoff checker pass; fix the first real failing step.
2. Recheck XY-C1 #45503440356 on source SHA 6d separately; claim L3 only if Build XY-C1, TRX verification and artifact upload all pass on the identical SHA. Do not transfer this result to newer SSH source.
3. Continue a concrete uncovered stability/security defect across Padavan userland. Do not reopen completed VTS port-read, WAN baseline, USB modem retry, OpenVPN certificate parsing, QMI identity, Wireless, HTTPD, DNSMasq, WAN route/DHCP, process inventory or USB storage work absent a new failing case.
4. Before any write, refresh live main and the frozen backup ref; before ending, refresh exact-SHA Actions and leave the first concrete next action.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Compare live refs, exact-head Actions, source baseline and first action against this file. Record resolved material FAIL/PENDING gates. If facts and first action still match, conclude the checkpoint with no documentation change or commit. Never create docs churn solely to synchronize a stored SHA.
