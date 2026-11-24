## Current repository state

# Cross-session execution mandate

Continue one autonomous engineering stream on `main`. For each logical slice, create all changed blobs, one complete Git tree based on the live main root tree, and one atomic main commit; update `main` with a non-forced expected-SHA compare-and-swap. Never force-push, create/use feature branches, make PRs, split one logical change into per-file commits, or modify the frozen backup. A PENDING workflow never blocks independent work. Before context exhaustion, finish the current safe slice and checkpoint the first concrete next action.

## Branch and checkpoint

- Parent main SHA for this handoff update: `d2fa77e69aa8017db457d5e8cfd4f9744cebe5a4`; fetch live `refs/heads/main` before every future write.
- Frozen `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and unchanged.
- `main` is the only development ref. PR #1 is closed/unmerged and deeply behind main; do not port it.
- Bootstrap, protocol and handoff were recovered once. Do not repeat startup audits without a new reason.

## Active task

### PID 1 notification queue workflow — retry-safe modem contract synchronization

Exact-head PID 1 run #45499228171 on source SHA `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` passed bounded-batch, notification-marker trust-boundary, enqueue-status, optional-daemon stop and GNU89 SSH restart policy tests. Its final inline Python source contract failed because modem checks still expected the old `stop_failed` flag.

The USB restart fix replaced that flag with `transition_failed`: failed NDIS/RAS readiness retains the previous modem rule/type cache, returns a failed queue action and blocks WAN restart so the notification can retry. Commit `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` added cache publication only after successful readiness and GNU89 regression cases. Commit `d2fa77e69aa8017db457d5e8cfd4f9744cebe5a4` corrected the dedicated USB workflow's stale caller assertions; USB #45499733421 and AI docs #45499733544 PASS on d2fa.

This handoff checkpoint updates the PID 1 workflow contract to validate both guarded modem reload paths, failed-transition ordering and the final WAN restart gate. Exact-head PID 1 rerun and AI docs validation on the resulting commit are pending. Production source is unchanged in this checker repair.

## Completed source slices

### WAN traffic persisted-baseline parsing

Commit `786c504c8247fe42d5d1d5f2dc3ad28757fafc80` fixed `wanlink_hook()` accepting numeric-prefix junk such as `40junk` via `strtoull(..., NULL, 10)`. Strict decimal parsing now rejects malformed/overflowing RX or TX baselines and marks the paired UI sample unavailable; empty legacy baseline and zero-on-counter-reset behavior remain. Regression covers junk, signs, overflow, empty/missing, NULL and reset. WAN sysfs byte-counter #45498624341 PASS on that SHA.

### USB modem reload retry state

The `RCN_RESTART_MODEM` handler had advanced in-memory modem configuration caches before `reload_modem_modules()` proved the new driver group ready. A retry could therefore skip a failed reload. Commit `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` only publishes rule/type cache values after successful readiness and blocks WAN restart after failure. GNU89 policy tests cover failed/successful publication. Dedicated USB caller contract correction `d2fa77e69aa8017db457d5e8cfd4f9744cebe5a4` passes USB and AI docs Actions.

## Verification boundary

### Product-source SHA 786c504c8247fe42d5d1d5f2dc3ad28757fafc80

- WAN sysfs byte-counter regression #45498624341: PASS.
- XY-C1 #45498624157: Build XY-C1, TRX verification and artifact upload all PASS on this exact product-source SHA. L3 applies to this SHA only.

### Product-source SHA 8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a

- PID 1 notification queue #45499228171: FAIL only in final inline Python source assertions; preceding C policy, secure-marker and daemon tests passed. Root cause was stale `stop_failed` expectation; this commit updates that workflow contract. Exact-head rerun pending.
- USB modem #45499228398 initially failed on the same stale modem caller assertion. Follow-up USB contract #45499733421 PASS on child checker/docs SHA d2fa.
- XY-C1 #45499228265: last poll IN_PROGRESS at `Build XY-C1`; TRX and artifact upload had not started. Re-poll this exact run and do not claim L3 until all three succeed on 8a2f.
- Process inventory #45499228303 and #45499228337, WAN DHCP callback #45499228283, WAN route #45499228321, HTTPD security #45499228151, USB storage #45499228328, DNSMasq #45499228269, Adbyby/DNSMasq #45499228820 and other triggered service regressions passed on this SHA.
- No target MIPS/uClibc runtime or physical-router L4 validation is claimed.

### Checker/docs SHA d2fa77e69aa8017db457d5e8cfd4f9744cebe5a4

- USB modem WDM/NDIS #45499733421: PASS.
- AI handoff documentation #45499733544: PASS.
- This SHA changes only the USB workflow contract and dev log; XY-C1 product validation remains tied to product source SHA 8a2f.

## Exact first action for the next GPT

1. Inspect the exact-head PID 1 notification queue run triggered by this commit. Confirm the updated modem cache-publication/order assertions pass; otherwise inspect the job log and repair the first mismatch.
2. Re-poll XY-C1 run #45499228265 for product source SHA `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a`. Claim L3 only if Build XY-C1, TRX verification and artifact upload all PASS on that same SHA.
3. Once these gates close, continue with a concrete uncovered stability/security defect across Padavan userland. Do not reopen completed WAN-baseline, USB modem retry, QMI identity, Wireless, HTTPD, DNSMasq, WAN route/DHCP, process inventory or USB storage work absent a new failing case.
4. Before ending, refresh live refs and exact-SHA Actions; update this handoff only when its facts or first action materially change.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must compare live repository state, exact-head Actions, source baseline, validation boundary, blockers and first next action against this file. Record newly resolved relevant PENDING/FAIL gates. If all facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Never create documentation churn solely to synchronize a stored SHA with live main.
