## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update `main` with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Branch and checkpoint

- Parent main SHA for this handoff update: `8ebf9dfc1aadd4dc45aeaa10514086aa97ed74ef`; fetch live `refs/heads/main` before every future write.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged.
- `main` is the only development ref. PR #1 is closed/unmerged and deeply behind main; do not port it.
- Bootstrap, protocol and handoff were recovered once. Do not repeat startup audits without a new reason.

## Active task

### OpenVPN client certificate validity argument hardening

The privileged `ovpn_export_client` RC applet accepted `days_valid` using `atoi()` twice. Inputs such as `30junk` were silently accepted as 30 days; oversized values could trigger undefined `atoi()` conversion behavior before reaching `openvpn-cert.sh`.

This source slice replaces that parsing with `openvpn_cert_days_valid_parse()`: an optional/empty argument retains the 365-day default; explicit values must be strict positive decimal days from 1 through 36,500, with legacy whitespace and leading-plus acceptance. Invalid syntax, negative/zero values, suffixes, fractions, over-policy values and numeric overflow fail before configuration reads, temporary export staging or key generation. GNU89 host tests cover boundaries and invalid inputs; the VPN workflow compiles/runs them under ASan/UBSan and its source contract verifies validation occurs before side effects.

The change is committed atomically with its policy test, exact-source contract, workflow trigger/step, development log and this handoff. Exact-head VPN regression and XY-C1 validation are pending on the new source SHA. The prior XY-C1 run #45499228265 completed SUCCESS on product source SHA `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a`, with Build, TRX and artifact upload all successful; it does not validate this new OpenVPN source change.

## Completed source slices

### WAN traffic persisted-baseline parsing

Commit `786c504c8247fe42d5d1d5f2dc3ad28757fafc80` fixed `wanlink_hook()` accepting numeric-prefix junk such as `40junk` via `strtoull(..., NULL, 10)`. Strict decimal parsing now rejects malformed/overflowing RX or TX baselines and marks the paired UI sample unavailable; empty legacy baseline and zero-on-counter-reset behavior remain. Regression covers junk, signs, overflow, empty/missing, NULL and reset. WAN sysfs byte-counter #45498624341 PASS on that SHA.

### USB modem reload retry state

The `RCN_RESTART_MODEM` handler had advanced in-memory modem configuration caches before `reload_modem_modules()` proved the new driver group ready. A retry could therefore skip a failed reload. Commit `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` only publishes rule/type cache values after successful readiness and blocks WAN restart after failure. GNU89 policy tests cover failed/successf## Verification boundary

### Product-source SHA 786c504c8247fe42d5d1d5f2dc3ad28757fafc80

- WAN sysfs byte-counter regression #45498624341: PASS.
- XY-C1 #45498624157: Build XY-C1, TRX verification and artifact upload all PASS on this exact product-source SHA.

### Product-source SHA 8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a

- XY-C1 Firmware Validation #45499228265: COMPLETED / SUCCESS. Build XY-C1, TRX verification and artifact upload each PASS on this exact SHA. This closes L3 for the modem product-source fix at this SHA; it does not validate later source changes.
- USB modem initial run #45499228398 and PID 1 initial run #45499228171 exposed stale workflow assertions for the old `stop_failed` identifier, not production test failures. Follow-up USB #45499733421 PASS on checker/docs SHA d2fa; follow-up PID 1 #45500981047 PASS on SHA 8ebf.
- Process inventory #45499228303 and #45499228337, WAN DHCP callback #45499228283, WAN route #45499228321, HTTPD security #45499228151, USB storage #45499228328, DNSMasq #45499228269, Adbyby/DNSMasq #45499228820 and other triggered service regressions passed on this SHA.

### Checker/docs SHA d2fa77e69aa8017db457d5e8cfd4f9744cebe5a4

- USB modem WDM/NDIS #45499733421: PASS.
- AI handoff documentation #45499733544: PASS.

### Main/checkpoint SHA 8ebf9dfc1aadd4dc45aeaa10514086aa97ed74ef

- PID 1 notification queue stability #45500981047: PASS; updated modem retry caller-graph contract passed.
- AI handoff documentation #45500980887: PASS.
- This SHA contains the workflow contract and handoff/log corrections, not the OpenVPN lifetime parser.

### Current OpenVPN certificate lifetime source slice

- Parent main SHA `8ebf9dfc1aadd4dc45aeaa10514086aa97ed74ef` contained the `atoi(argv[3])` parser. The new strict parser, GNU89 test and exact-source lifecycle contract are included in this commit; exact-head VPN, AI docs and XY-C1 validation must be checked after push.
- No physical-router L4 validation is claimed.

S.
- This SHA changes only the USB workflow contract and dev log; XY-C1 product validation remains tied to product source SHA 8a2f.

## Exact first action for the next GPT

1. Inspect the exact-head VPN server lifecycle workflow triggered by the OpenVPN parser source/test changes. Confirm the ASan/UBSan GNU89 parser regression and `check_vpn_server_lifecycle.py` contract pass; diagnose the first real failing step if not.
2. Check exact-head XY-C1 Build, TRX verification and artifact upload on the same new product-source SHA; claim L3 only when the run concludes SUCCESS and all three steps pass.
3. Check AI handoff documentation validation on the same commit. If the handoff contract flags an issue, fix it in the next real code/test slice rather than a standalone docs-only synchronization commit.
4. Once these gates close, continue with a concrete uncovered stability/security defect across Padavan userland. Do not reopen completed WAN-baseline, USB modem retry, QMI identity, Wireless, HTTPD, DNSMasq, WAN route/DHCP, process inventory or USB storage work absent a new failing case.
5. Before ending, refresh live refs and exact-SHA Actions; update this handoff only when its facts or first action materially change.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare live repository state, exact-head Actions, source baseline, validation boundary, blockers and first next action against this file. Record newly resolved relevant PENDING/FAIL gates. If all facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
