# 2026-11-19 continuation checkpoint — preserve first error across WAN fail-fast preflights

## Repository state

- Repository: `Arlene206/padavan-4.4-2`; development is main-only.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. Never modify it.
- Legacy PR #1 remains closed/unmerged; it is not a development path.
- Product-source parent for this checkpoint: `8e0763554388cf4dfcbc985c0712a991b3bb3c14`. Fetch live `main` before any work; do not assume a handoff SHA is the live ref.
- The preceding chronological USB modem/WAN accumulator slice landed on that parent. Exact-head USB modem `44442187961`, Process lifecycle `44442187890`, WAN route `44442187838`, and AI docs `44442187852` all passed. XY-C1 run `44442187996` was still IN_PROGRESS at the last checkpoint (head SHA `8e0763554388cf4dfcbc985c0712a991b3bb3c14`; Build job `135285479772`). Recheck its final Build/TRX/artifact result; do not transfer it to later SHAs.
- No local C test or MIPS build was run in this connector-only session. Physical-router L4 is NOT VERIFIED.

## Completed source slice in this checkpoint

Reviewing the just-completed `start_wan()` chronological accumulator fix exposed three earlier fail-fast returns that bypassed it:

1. `create_cb_links()` failure returned a fresh `errno`/EIO value after earlier WAN sysctl status had already been accumulated. The error is now captured before logging (so logging cannot disturb errno), merged with `rc_first_service_start_status()`, and the aggregate is returned.
2. `config_vinet_wan()` failure returned `vlan_result` directly, potentially masking an earlier WAN startup failure. It now merges the configuration result before rollback/logging and returns the aggregate.
3. `config_apcli_wisp()` had the same direct-return defect and now follows the same first-error rule.

Regression coverage is in `trunk/user/rc/src/tests/modem_wan_start_policy.c` (`test_early_wan_abort_preserves_first_error()`). `scripts/check_modem_wan_start_status.py` checks all three production branches, errno capture ordering and no direct-return regressions. Existing workflow path filters already cover all touched C/test/checker paths; no workflow was added or broadened.

## Exact next action

1. Fetch live `main`, then inspect push-triggered Actions whose `head_sha` exactly matches that live commit: USB modem identity, WAN route status, Process lifecycle/inventory, AI handoff docs, and XY-C1.
2. Read logs for any failed run and fix only the demonstrated regression. For XY-C1, claim L3 only after Build, TRX verification and artifact upload all pass on the exact source SHA.
3. Continue the P1 stability queue outside this already-reviewed WAN startup branch; select a concrete source/caller defect, add a focused regression, and reuse an existing workflow where appropriate.
4. Do not repeat the completed WAN modem accumulator/preflight audit unless new source evidence demonstrates another failure path.

## Validation boundary

- L1: source review identifies the three direct-return paths; follow-up merges each failure in chronological order.
- L2: exact-head GNU89/ASan/UBSan modem-WAN policy regression, modem source checker, WAN route and Process integration checks are pending for this checkpoint.
- L3: pending exact-source XY-C1 Build + TRX + artifact verification.
- L4: physical modem/WAN operation and router behavior remain NOT VERIFIED.

## Handoff governance

This checkpoint records the last meaningful product-source parent, active task, exact prior CI boundary, validation gaps and one actionable first step. The next GPT must fetch live refs and exact-head Actions; no no-op SHA-refresh commit is needed.
