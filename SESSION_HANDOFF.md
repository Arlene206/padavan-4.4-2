# 2026-11-18 continuation checkpoint — dnsmasq transaction/cgroup boundary correction

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. The transactional dnsmasq change is `e9a69e67c0864f34a297aa86b8d4e28d9ad2e1e6`; this correction is based on that exact parent.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.
- Exact e9 checks: shell syntax, required startup status, process lifecycle runtime and Adblock regression PASS; Process inventory `44289343898` and umbrella `44289344240` PASS; AI handoff `44289343965` PASS. Shadowsocks `44289343841` failed only in the static checker because it still expected the removed inline ChinaDNS `sed` edits. XY-C1 `44289343944` was IN_PROGRESS at last poll.
- ChinaDNS source `4b956fe` has exact-source XY-C1 `44286192538` SUCCESS, including Build, TRX and artifact upload.

## Active task
- e9a69e67 added transactional dnsmasq config staging, syntax validation, atomic publication and bounded stop/start rollback; all shell runtime regressions passed.
- This correction changes the stale ChinaDNS source contract to require the transactional publisher after daemon readiness.
- Review found that `run_bin()` enrolls children in the Shadowsocks CPU/memory cgroup, while dnsmasq was previously launched outside it. Add a dedicated dnsmasq launcher using validated PID-count baseline and bounded count+1 readiness without cgroup writes; a regression enables `ss_cgroups=1` during dnsmasq restart to pin this compatibility boundary.

## Verification boundary
- Exact e9 shell tests passed; its Python checker stopped at the stale ChinaDNS direct-edit contract. This correction's static checker and cgroup-boundary test need exact-SHA rerun.
- XY-C1 run `44289343944` for e9 was IN_PROGRESS at last poll. This correction changes ROMFS source and needs its own exact-SHA Build/TRX/artifact result.
- No target BusyBox runtime or physical-router L4 is claimed.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-head Shadowsocks, Process inventory, AI docs and XY-C1 for this cgroup-boundary/source-contract correction; fix the first causal failure.
2. Confirm Build XY-C1, Verify TRX output and artifact upload on this exact source SHA.
3. Continue checking dnsmasq restart/DHCP service-manager coupling and rollback races; then broaden to RC/userland P1 reliability.
4. Keep the frozen backup unchanged and physical-router L4 NOT VERIFIED.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending, verifying active baseline, exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, make no changes and do not create a no-op commit.

# Cross-session execution mandate
Complete each logical source/test slice in one atomic main commit. Continue independent work while CI is PENDING. Before context exhaustion, leave a precise next action and tested/unverified boundaries. Never force-push, mutate the frozen backup, reopen PR #1 or claim unperformed tests/builds.
