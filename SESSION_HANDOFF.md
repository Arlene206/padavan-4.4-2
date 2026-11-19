# 2026-11-19 continuation checkpoint — preserve first error across WAN fail-fast preflights

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development remains main-only.
- Last committed source parent: `8e0763554388cf4dfcbc985c0712a991b3bb3c14`; the follow-up source and regression changes are committed in the next checkpoint commit. Always fetch live `main` rather than infer its SHA from this document.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it. PR #1 remains closed/unmerged and is not a development path.
- On parent `8e0763554388cf4dfcbc985c0712a991b3bb3c14`, USB modem `44442187961`, Process lifecycle `44442187890`, WAN route `44442187838`, and AI docs `44442187852` passed. XY-C1 `44442187996` was still IN_PROGRESS at the last parent checkpoint (job `135285479772`); recheck it only as evidence for that exact parent SHA.

## Active task

The previous WAN first-error work unified USB NDIS/RAS and ordinary WAN startup results. A subsequent review found three fail-fast branches that still returned their local error and could mask an earlier WAN sysctl/setup failure:

1. `create_cb_links()` — capture errno (EIO fallback) before logging, merge into `wan_start_status`, and return the accumulator.
2. `config_vinet_wan()` — merge its failure before callback-link cleanup and return the first chronological status.
3. `config_apcli_wisp()` — use the same aggregate-before-cleanup/return rule.

Regression `test_early_wan_abort_preserves_first_error()` covers prior ENOMEM followed by callback, VLAN and AP-client failures, plus the no-prior-error behavior. `scripts/check_modem_wan_start_status.py` statically verifies the three production branches. Existing USB modem, WAN route, Process lifecycle and XY-C1 workflows already cover the changed sources; no workflow or path filter was expanded.

## Verification boundary

- L1: code path reviewed; all three direct-return paths now preserve the first chronological failure.
- L2: host GNU89/ASan/UBSan policy binary passed on the first exact-head run, but the source checker contained a whitespace-normalization mismatch and must be rerun after correction. WAN route status passed; USB modem and Process failed only at that same checker assertion. AI docs failed because this handoff omitted required governance wording; correction is part of the current follow-up. Exact-head reruns are pending.
- L3: XY-C1 run `44443755078` failed at the same WAN source-checker assertion late in the job; inspect whether firmware Build/TRX/artifact completed before that checker failed. This does not count as L3 PASS until all three build stages are confirmed on the exact SHA.
- L4: physical WAN/modem operation and router behavior are NOT VERIFIED. No local C tests or local MIPS build were run in this connector-only session.

## Exact first action for the next GPT

1. Fetch live `main` and the immutable backup ref; inspect exact-head USB modem `44443755296`, Process `44443755214`, AI docs `44443755298`, WAN route `44443755126`, and XY-C1 `44443755078` for the previous SHA `18a73adeea90de2935a07cad347e1f214a76966c`.
2. Inspect and fix only demonstrated failures. The checker whitespace bug and handoff required-text omission are corrected in this checkpoint; rerun each on the new SHA.
3. For XY-C1, claim L3 only if Build, TRX verification and artifact upload all pass on an exact source SHA. Do not wait idle on PENDING; continue separate stability work.
4. Once the WAN checker slice passes, select an independent concrete P1 reliability/security defect elsewhere in Padavan userland.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

Use one atomic main commit per logical slice. Create all blobs and a complete Git tree, then one commit with the exact live main SHA as its sole parent; update main only through a non-forced expected-SHA compare-and-swap. Keep working across GPT conversations; do not wait on unrelated PENDING checks and do not alter the frozen backup.

## Before context exhaustion

Finish any safe active slice, record its exact source/test/CI evidence and unverified boundaries, and leave one concrete next action. Never claim tests, builds, artifacts or hardware checks that were not actually completed.
