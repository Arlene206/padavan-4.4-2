## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update `main` with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Branch and checkpoint

- Parent main SHA for this handoff update: `d096f168bfe4188f5223bf76806b4ea012742580`; fetch live `refs/heads/main` before every future write.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged.
- `main` is the only development ref. PR #1 is closed/unmerged and deeply behind main; do not port it.
- Bootstrap, protocol and handoff were recovered once. Do not repeat startup audits without a new reason.

## Active task

### Firewall virtual-server optional translation-port reads

The VTS filter and NAT builders read `vts_lport_x` with `portrange_conv()`, which collapsed both missing/empty settings and backend read failures into NULL. They then fell back to the external port. A transient NVRAM failure could therefore publish an unintended mapping: a configured WAN port 8080 -> LAN port 80 rule could become DNAT to LAN:8080.

The new source slice distinguishes absent/empty optional values from read failures with `firewall_read_indexed_string_setting_with_presence()` and `firewall_vts_lport_read_status()`. Historical missing/empty behavior remains unchanged; a backend or malformed read status now returns failure from the VTS builder, causing the caller to abort incomplete IPv4 filter/NAT staging and keep the previously published firewall ruleset. The regression tests missing, empty, present values, backend errno, malformed presence/value combinations, and output preservation. The exact-source checker verifies both builders abort before rule publication. The firewall Actions workflow compiles the policy test with GNU89, ASan and UBSan.

This slice is committed with its source, test, checker, workflow, development log and handoff. Exact-head firewall, AI docs and XY-C1 product checks must be reviewed after push. No target runtime or physical-router validation is claimed.

## Completed source slices

### OpenVPN client certificate lifetime argument

Commit `d096f168bfe4188f5223bf76806b4ea012742580` replaces privileged `ovpn_export_client`'s permissive `atoi(argv[3])` parsing with a strict GNU89 parser. Optional/empty input retains the 365-day default and whitespace/leading-plus compatibility; explicit values accept 1–36,500 days and reject negatives, zero, suffix junk, fractions, out-of-policy values and overflow before configuration/key-generation side effects. VPN lifecycle #45502092649 and AI docs #45502092664 PASS on d096.

### USB modem reload retry state

Commit `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` changes `RCN_RESTART_MODEM` so rule/type cache values publish only after `reload_modem_modules()` confirms the new driver group ready. Failure retains the previous baseline and prevents WAN restart, so the queued retry can retry the driver transition. Follow-up USB caller contract #45499733421 PASS on checker/docs SHA d2fa; PID 1 queue #45500981047 and AI docs #45500980887 PASS on SHA 8ebf.

### WAN traffic persisted-baseline parsing

Commit `786c504c8247fe42d5d1d5f2dc3ad28757fafc80` fixes `wanlink_hook()` accepting numeric-prefix junk such as `40junk` through `strtoull(..., NULL, 10)`. Strict decimal parsing rejects malformed/overflowing RX or TX baselines and marks the paired UI sample unavailable while preserving empty legacy baseline and zero-on-counter-reset behavior. WAN regression #45498624341 PASS.

## Verification boundary

### Product-source SHA 786c504c8247fe42d5d1d5f2dc3ad28757fafc80

- WAN sysfs byte-counter regression #45498624341: PASS.
- XY-C1 #45498624157: Build, TRX verification and artifact upload all PASS on this exact SHA.

### Product-source SHA 8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a

- XY-C1 #45499228265: COMPLETED / SUCCESS. Build XY-C1, TRX verification and artifact upload each PASS on this exact SHA.
- USB modem initial run #45499228398 and PID 1 initial run #45499228171 failed only on stale `stop_failed` workflow assertions; follow-up USB #45499733421 PASS on d2fa and PID 1 #45500981047 PASS on 8ebf.
- Process inventory #45499228303/#45499228337, WAN DHCP callback #45499228283, WAN route #45499228321, HTTPD security #45499228151, USB storage #45499228328, DNSMasq #45499228269 and other triggered service regressions passed on this SHA.

### Product/checker SHA d096f168bfe4188f5223bf76806b4ea012742580

- VPN server lifecycle #45502092649: PASS, including OpenVPN certificate lifetime argument GNU89 ASan/UBSan regression and exact-source contract.
- AI handoff docs #45502092664: PASS.
- NVRAM transaction boundary audit #45502092721: PASS. Its userland scan reported 4,585 source files, 23 raw ioctl references confined to the locked adapter and zero raw userland bypasses; kernel snapshot/caller contract also passed.
- Process lifecycle inventory #45502093101: PASS.
- IPv6 service checked-read #45502092750: PASS.
- HTTPD authentication/security #45502092764: PASS.
- OpenVPN client credential staging #45502092820: PASS.
- XY-C1 Firmware Validation #45502092732: PENDING at last poll; `Build XY-C1` was in progress and TRX/artifact steps had not started. This run is on d096 and does not validate the new firewall source that follows it.
- No target MIPS/uClibc runtime or physical-router L4 validation is claimed.

### Current firewall VTS translation-port source slice

- Parent main SHA: `d096f168bfe4188f5223bf76806b4ea012742580`.
- Dedicated firewall regression, exact-source contract, AI docs and XY-C1 Build/TRX/artifact validation are pending on this slice's resulting source SHA.

## Exact first action for the next GPT

1. Inspect the exact-head Firewall Startup Stability workflow triggered by the VTS translation-port header/test/source/checker. Confirm the GNU89 ASan/UBSan regression and both VTS exact-source fail-closed contracts pass; fix the first real failing assertion or test.
2. Check exact-head XY-C1 Build, TRX verification and artifact upload on the same resulting product-source SHA. Separately recheck prior XY-C1 run #45502092732 on d096; claim L3 for d096 only after that run concludes SUCCESS with all three steps passing, and do not transfer it to the newer firewall SHA.
3. Check AI handoff documentation validation on the same resulting commit. Update the handoff only in a later meaningful source/test commit when facts or the first action materially change.
4. Continue another concrete stability/security defect across Padavan userland. Do not reopen completed WAN baseline, USB modem retry, OpenVPN certificate parsing, QMI identity, Wireless, HTTPD, DNSMasq, WAN route/DHCP, process inventory or USB storage work absent a new failing case.
5. Before any write, refresh live main and the frozen backup ref; before ending, refresh exact-SHA Actions and leave the first concrete next action.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare live repository state, exact-head Actions, source baseline, validation boundary, blockers and first next action against this file. Record newly resolved relevant PENDING/FAIL gates. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
