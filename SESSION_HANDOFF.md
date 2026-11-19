# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup branch `backup/pre-plugin-overhaul-2026-11-14` is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Product-source baseline: OpenVPN client export hardening at `e01f70a766c4e628593e272192723f6add59ffca`. The live documentation/checker tip at recovery was `f4fbd093e934fda5304aa796cba30d59eb29b3a4`; always re-fetch `main` before writing.
- Exact-head OpenVPN client credential staging run `44613650285` on `f4fbd093e934fda5304aa796cba30d59eb29b3a4` PASS: four GNU89/ASan/UBSan regressions and the production/workflow contracts all succeeded.
- VPN server lifecycle run `44613173654` on source SHA `e01f70a766c4e628593e272192723f6add59ffca` PASS. XY-C1 run `44613173721` on that same source SHA completed Build XY-C1, TRX verification and artifact upload: L3 VERIFIED for this OpenVPN source slice.
- AI handoff documentation run `44613650462` on `f4fbd093e934fda5304aa796cba30d59eb29b3a4` FAILED because the required literal heading `## Active task` was missing. THIS COMMIT restores that required section and also repairs the root-log rotation invariant described below; the new exact-head documentation run must still be checked.
- Root `dev_log.md` had 121 dated records (above the 119-record ceiling). THIS COMMIT moves the 101 oldest complete records verbatim, oldest-first, to `docs/dev-log/2026/11/part-018.md`, retains the newest 20, and prepends this milestone as record 21. The archive index is updated atomically.
- Frozen backup branch remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not alter it. PR #1 is closed and unmerged; it remains legacy inventory only, not a development target.

## Active task

### Close OpenVPN credential export verification

- Preserve the private staged/atomic client-config publication, no-follow key reads, owner/mode checks, key-generation status handling and checked temporary cleanup introduced at source SHA `e01f70a766c4e628593e272192723f6add59ffca`. Do not roll back these security controls.
- OpenVPN regression, VPN server lifecycle and XY-C1 firmware validation are PASS at the exact source/checkpoint SHAs above. Hardware import/connection validation has not been performed.
- The current bounded documentation correction is to make the AI handoff contract pass: restore the validator-required active-task heading and bring the root log back inside its 20–119 entry window without rewriting archived history.

### Next engineering direction after the documentation check

Resume the whole-userland P1 stability queue in `REFACTOR_ROADMAP.md`, starting with the next independently evidenced dropped startup/teardown result in shared RC/service call paths. Do not repeat the already completed OpenVPN, DNSMasq resolver-file, `create_file()` close-status, USB modem notifier, WAN monitor-pause, or other explicitly closed audits. Preserve shared RC test fan-out and do not change workflow filters without tracing every invoked consumer.

## Exact first action for the next GPT

1. Check the AI handoff documentation workflow triggered by THIS COMMIT and match its `head_sha` to the new live `main`. If it fails, inspect the exact log and repair the actual contract issue; do not weaken archive integrity or the 20–119 record rule.
2. Once documentation validation passes, resume the next concrete P1 startup/teardown status-propagation defect from the roadmap; inspect only the relevant implementation, caller graph and regression path needed to finish that slice.
3. Keep the OpenVPN source result scoped to L1/L2/L3 above. Physical-router and real client import behavior remain NOT VERIFIED.

## Verification boundary

- L1: OpenVPN private-key export hardening is committed at source SHA `e01f70a766c4e628593e272192723f6add59ffca`.
- L2: OpenVPN credential staging run `44613650285` PASS on exact docs/checker descendant `f4fbd093e934fda5304aa796cba30d59eb29b3a4`; all four compiled regressions and source/workflow checks passed. VPN server lifecycle run `44613173654` PASS on exact source SHA `e01f70a766c4e628593e272192723f6add59ffca`.
- L3: XY-C1 run `44613173721` PASS on the exact OpenVPN source SHA `e01f70a766c4e628593e272192723f6add59ffca`; Build, TRX verification and artifact upload all succeeded.
- L4: physical XY-C1 router, OpenVPN export permissions on target, client import and actual tunnel connection remain NOT VERIFIED.
- AI documentation workflow `44613650462` FAIL at `f4fbd093` solely on the missing `## Active task` heading. THIS COMMIT fixes the heading and log rotation. The new commit's AI documentation run is PENDING until its exact-head result is inspected.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint. Compare live main, source baseline, exact-head Actions, module status and this first action. Update this file only when those resumable facts materially change; do not create a SHA-refresh or no-op commit.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; use force=false with the exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.
