# 2026-11-18 continuation checkpoint — propagate Shadowsocks shutdown service errors

## Current repository state
- Repository Arlene206/padavan-4.4-2, branch main. This source/test slice is based on exact parent b118b841d5b292285c296288fd01229fcf63f28a; refresh live main before any further write.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains immutable at 72f4f157a084f8184b9597ca5bf334b8080e5807; PR #1 remains closed/unmerged.
- Exact b118 checks: Shadowsocks lifecycle 44292658234 PASS; AI handoff docs 44292658587 PASS. XY-C1 run 44291015307 for parent source cb6d completed successfully: Build XY-C1, TRX verification, artifact upload and the cross-build job all SUCCESS. This new shutdown source slice has not yet run exact-head CI.
- Prior test iterations on cb6d/3ec/8e fixed the DNSMasq stop-failure fixture; on 8e all runtime tests passed and only one stale source assertion remained, corrected in b118.

## Active task
- DNSMasq transaction / ChinaDNS-ng readiness work remains in place: staged syntax-checked configuration, verified stop-before-publish, bounded standalone dnsmasq PID launch outside Shadowsocks cgroups, rollback, and ChinaDNS active-state gating.
- This slice fixes a concrete close-path defect: ssp_close previously discarded errors from firewall cleanup, DNSMasq config edits, /sbin/restart_dhcpd and PPPoE-MWAN reconciliation, then returned success unconditionally. It now preserves the first encountered cleanup/service error, logs failures, continues independent cleanup, and returns the status to stop/restart/reserver callers. Incomplete proxy process stop still exits before tearing down shared state.
- Host tests cover healthy close, DNSMasq restart error propagation, first-error retention, and aborting cleanup after proxy stop failure.

## Verification boundary
- Exact b118 Shadowsocks lifecycle and AI handoff Actions PASS. Parent cb6d XY-C1 Build/TRX/artifact SUCCESS. All new ssp_close source/test changes require fresh exact-head regression and XY-C1 Build/TRX/artifact checks.
- No target BusyBox runtime or physical-router L4 is claimed.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head Shadowsocks, Process inventory/umbrella, AI handoff and XY-C1 Actions for this shutdown-error slice; fix any actual failure rather than merely changing expected text.
2. Confirm the same product SHA completes Build XY-C1, TRX verification and artifact upload.
3. Continue the DNSMasq lifecycle boundary review: Shadowsocks dnsmasq helper directly stops/starts the process and therefore bypasses the existing C RC /tmp/.dnsmasq.lifecycle.lock; decide whether to use official /sbin/restart_dhcpd inside the atomic publish/rollback transaction while keeping dnsmasq outside Shadowsocks cgroups.
4. Then replace broad substring-based dnsmasq include cleanup in oversea / ssp_close / ss-rules with exact directive matching and injected regression tests. Keep backup immutable and physical-router L4 unverified.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and compare it with SESSION_HANDOFF.md, verifying active engineering baseline, exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all facts are accurate, make no file change and do not create a no-op commit.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent work during PENDING checks. Before context exhaustion, leave an accurate checkpoint with the exact source SHA, tests/Actions status, validation boundary and next first action. Never force-push, mutate the frozen backup, reopen PR #1 or claim unperformed validation.
