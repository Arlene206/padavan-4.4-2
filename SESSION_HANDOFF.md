# 2026-11-18 continuation checkpoint — transactional dnsmasq config and restart

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. ChinaDNS-ng readiness source is `4b956fe1d8fa3fe3c6dbad8bd9bcb8116379b7bc`; this dnsmasq transaction slice is based on docs-only parent `2528eb7e483cf9a31cc49776999a8c7a2962a68d`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.
- Exact source SHA `4b956fe`: Shadowsocks lifecycle `44286192332` PASS; Process inventory `44286192381` PASS; umbrella Process inventory `44286192617` PASS; XY-C1 `44286192538` SUCCESS including Build XY-C1, TRX verification and artifact upload. AI docs on 4b956 `44286193029` failed only for a missing literal end-of-session checkpoint sentence; docs-only `2528eb7` restores it and AI docs run `44288452305` PASS.
- Earlier ABSENT-under-errexit source SHA `cfbf00f9` has exact-SHA XY-C1 run `44283616981` SUCCESS (Build, TRX and artifact all PASS).

## Active task
- ChinaDNS-ng validates upstream DNS and PID inventory, prevents duplicate launch, waits for the named daemon and propagates failure to router/GFW.
- THIS slice replaces live dnsmasq in-place edits and unchecked detached restart with same-directory config staging, metadata-preserving backup, dnsmasq syntax validation, atomic publication, bounded stop/start, and rollback to the previous config/service when the new restart fails.
- Host regressions cover successful config update/restart, invalid staged config preserving live file and active daemon, and failed replacement startup restoring old config and successfully restarting the previous service.

## Verification boundary
- This transaction slice has not run exact-SHA Actions yet. Require shell syntax, process lifecycle regression, source checker, Process inventory, AI handoff and XY-C1 after push.
- Parent XY-C1 `44286192538` on source SHA `4b956fe` succeeded: Build, TRX verification and artifact upload are all PASS. This transaction changes ROMFS source and requires its own exact-SHA firmware run before L3 can be claimed.
- No target BusyBox runtime or physical-router L4 is claimed.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-head Shadowsocks, Process inventory, AI docs and XY-C1 for this transaction slice; fix any causal failures.
2. Confirm Build XY-C1, Verify TRX output and artifact upload on this exact product SHA.
3. Continue checking dnsmasq restart/DHCP service-manager coupling and rollback races, then broaden to RC/userland P1 reliability.
4. Keep frozen backup immutable and physical-router L4 NOT VERIFIED.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending, verifying active engineering baseline, exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all facts are accurate, make no file change and do not create a no-op commit.

# Cross-session execution mandate
Complete each logical source/test slice in one atomic main commit. Continue independent work while CI is PENDING. Before context exhaustion, leave a precise next action and tested/unverified boundaries. Never force-push, mutate the frozen backup, reopen PR #1 or claim unperformed tests/builds.
