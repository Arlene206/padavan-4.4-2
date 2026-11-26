# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development continues on `main` only.
- Parent `main` for the active SIT gateway slice: `293cd7559b28375a90e3f6c290825bb6e51d66fa`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify/delete. PR #1 remains closed and unmerged.
- Bootstrap/protocol recovery was already performed once. Do not repeat it or re-audit completed slices.

## Active task

- P1 WAN IPv6 SIT route safety. Two adjacent ownership invariants are enforced:
  1. `wan6_sit_route4_record_format()` rejects interface names of 16+ bytes because its parser uses a 16-byte destination buffer; 15-byte names remain round-trippable.
  2. `wan6_up()` previously accepted ambiguous `inet_aton()` gateway aliases through `is_valid_ipv4()`, then could add a route before strict `inet_pton()` ownership-record publication rejected the gateway. `wan6_sit_route4_gateway_normalize()` now performs strict preflight before service/route mutation. Empty and canonical `0.0.0.0` keep the legacy no-gateway behavior; other nonempty gateways must be strict dotted-quad literals.
- Both policy regressions are in `trunk/user/rc/src/tests/wan6_identity_policy.c`; the WAN DHCP callback workflow compiles/runs this test as GNU89/`-Werror`/ASan/UBSan.

## Verification boundary

- Source commit `032e71f1eee03ed2f3d4d6683c2485872bea16e1`: WAN DHCP callback #45784673035 PASS, WAN route status #45784673105 PASS, AI handoff #45784866508 PASS. Its XY-C1 #45784672998 was `Build XY-C1` in progress at last check; inspect final Build/TRX/artifact step results.
- Parent `5bec4f752ac0961c823a2750da03d34a3da0789a`: WAN DHCP callback #45783956641 PASS including IPTV regressions; AI first failed because mandatory handoff contract sections were missing, then exact AI #45784866508 passed after correction on `032e71f`. XY-C1 was host-only and skipped Build/TRX/artifact.
- Product L3 baseline remains `dd2885fc9941d3a57fd9c81cf01ed86488b0183d` with same-SHA Build XY-C1, TRX verification and artifact upload PASS (artifact #15098382009, 12,823,167 bytes). The later production source slice needs its own same-SHA Build + TRX + artifact PASS to advance L3. Physical router L4 NOT VERIFIED.
- No local repository checkout is available; do not claim local execution of the original C suite. The latest exact-SHA WAN callback/route/AI/XY checks are pending after this commit.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head WAN DHCP callback, WAN route, AI handoff and XY-C1 Actions for the latest gateway slice. Confirm both strict-gateway and IFNAMSIZ boundary regressions execute/pass; diagnose any causal failure.
2. In the exact-head XY run, require `Build XY-C1`, `Verify TRX output` and artifact upload all to conclude PASS. A skipped/pending step is not firmware verification.
3. Continue the P1 WAN/IPv6 route lifecycle investigation in `trunk/user/rc/src/net_wan6.c`, especially teardown ownership/first-error paths and BusyBox callback live-transition semantics. Change behavior only for a confirmed defect and add a focused regression.

## Mandatory handoff checkpoint

**Mandatory handoff checkpoint:** Every session MUST compare the live repository and relevant exact-head Actions with this file before ending. Verify the engineering/source baseline, active task/progress, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all facts are accurate, leave this file untouched and create no no-op commit.

# Cross-session execution mandate

Continue across conversations as the same repository maintainer. Each independently reviewable source/test milestone must be published as one atomic main commit: create all changed blobs, one complete Git tree and one commit, then update `main` with `force:false` and the expected parent SHA. Never publish partial-file commits or force-push.
Before context exhaustion, finish a safe bounded slice, inspect exact-SHA Actions, and leave this handoff with the precise next task and validation boundary.

## Execution invariants

- Main only; one complete tree/commit per logical task; expected-SHA non-forced ref update; never force-push.
- Preserve the frozen backup; do not reopen PR #1.
- PENDING is not a gate on independent safe work.
- Product L3 requires same-source-SHA Build XY-C1, TRX verification and artifact upload all PASS; physical L4 requires real target-device testing.
- Update dev_log for completed engineering slices, handoff for material resumable changes, and roadmap only for material module/status changes.
