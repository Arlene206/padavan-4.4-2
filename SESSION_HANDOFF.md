## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update `main` with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Branch and checkpoint

- Parent main SHA for this handoff update: `6d5d5376e425587e6f116d09f88cbcc7ac6a05c1`; refresh live refs before future writes.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- `main` is the only development ref. PR #1 is closed/unmerged and must not be ported.
- Bootstrap, protocol and handoff were recovered once; do not repeat startup audits without a new reason.

## Active task

### Firewall virtual-server optional translation-port reads

Source commit `6d5d5376e425587e6f116d09f88cbcc7ac6a05c1` fixes `include_vts_filter()` and `include_vts_nat()` treating failed reads of optional `vts_lport_x` as missing/empty. That fallback could change a mapping (for example, WAN 8080 -> LAN:8080 rather than LAN:80). Both builders now read the indexed string with presence information; absent/empty retains legacy external-port fallback, while a backend or malformed read result aborts incomplete rule staging so the previously published firewall ruleset remains active.

The new GNU89 regression covers absent, empty, configured values, backend errno, malformed presence/value results and output preservation. Its standalone GNU89 `-Wall -Wextra -Werror` ASan/UBSan compile/run passed locally.

Firewall Startup Stability #45503440459 on SHA 6d failed before the test compile because the Python checker searched for the checked NVRAM call on one line, while C wraps the invocation across lines. The production source/test was not exercised by that run. This checker-only follow-up normalizes whitespace before asserting the source order. Exact-head firewall rerun is pending.

## Completed source slices

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

### Firewall source SHA 6d5d5376e425587e6f116d09f88cbcc7ac6a05c1

- Firewall Startup Stability #45503440459: FAIL at the first Python source-check step due only to a multiline-sensitive substring search; compile/run test was skipped. Checker corrected in the current commit; rerun pending.
- WAN route #45503440493 and IPv6 service checked-read #45503440453: PASS.
- AI handoff docs #45503440345: PASS.
- WAN DHCP #45503440341, process inventory #45503440565, HTTPD #45503440466, NVRAM audit #45503440350, XY-C1 #45503440356 and FULLCONENAT #45503440566 were in progress at last poll.
- No target MIPS/uClibc runtime or physical-router L4 validation is claimed.

## Exact first action for the next GPT

1. Inspect the Firewall Startup Stability rerun triggered by this checker correction. Confirm the exact-source checker passes and the GNU89 ASan/UBSan VTS policy test actually runs; diagnose any new failure rather than assuming the previous failure was fixed.
2. Poll XY-C1 #45503440356 for product source SHA 6d and the preceding d096 run #45502092732 separately. Claim L3 only for an exact SHA whose run concludes SUCCESS and whose Build, TRX verification and artifact upload all pass.
3. Check AI docs on the checker-fix commit. Continue other independent stability/security source work while firmware build remains pending.
4. Keep the backup read-only. Before any write, refresh live main and expected SHA; before ending, update this handoff only when the first action or validation boundary materially changes.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Compare live refs, exact-head Actions, source baseline and first action against this file. Record resolved material FAIL/PENDING gates. If facts and first action still match, conclude the checkpoint with no documentation change or commit. Never create docs churn solely to synchronize a stored SHA.
