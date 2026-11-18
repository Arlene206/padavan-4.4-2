# 2026-11-18 continuation checkpoint — CI trigger governance

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Product-source baseline: `648e7353b58c1f33fddc4d6ba8bb65e195f6695f`; its XY-C1 build, TRX verification and artifact upload completed in run `44329872151`. This evidence applies only to that exact product SHA.
- Current main parent for this governance milestone: `acd10ac4d35795344669b00065a7ea9520c98830`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged. GPT Session Rotator remains untouched.

## Active task
- CI trigger review found that common RC files fan out to several focused suites. Inspection of the AdGuardHome, Inadyn, NFS, WAN route and USB modem workflows shows those tests/checkers consume relevant shared RC declarations and production call paths. Their shared-file triggers are real dependencies and must remain; do not remove them just to reduce the Action count.
- The dedicated Process lifecycle inventory workflow intentionally scans all tracked `trunk/user/**` paths. The Process lifecycle regression workflow is separately explicit-allowlisted, with matching push and pull_request path sets.
- XY-C1 host regressions remain available for relevant source/workflow changes. Full firmware Build, TRX verification and artifact upload are gated by the production-input detector; unknown inputs and untrustworthy comparisons must fail safe to a full build.
- Exact parent SHA `acd10ac4d35795344669b00065a7ea9520c98830` had AI handoff run `44334501605`, Process lifecycle run `44334501702`, and Process inventory run `44334501962` all PASS. The Adbyby detached-child cleanup race was fixed in that parent by reaping the test double; production behavior was not changed.

## Verification boundary
- This governance commit changes root instructions, the AI handoff validator, and the AI docs workflow. It does not change firmware product source.
- The new validator enforces the sole whole-userland inventory exception, explicit Process regression allowlist and push/PR parity, and the XY-C1 production-input gates.
- No new full XY-C1 firmware build is warranted for these documentation/CI-governance-only changes. The XY-C1 host-only run should confirm its detector skips Build/TRX/artifact.
- No local checkout or physical-router L4 validation was available in this session.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-head AI handoff, Process lifecycle, Process inventory and XY-C1 Actions for this commit.
2. Confirm the governance validator passes, Process regression and inventory remain green, and XY-C1 reports host-only validation without running Build/TRX/artifact.
3. If a relevant run fails, inspect its first causal log and repair the defect without removing tests or weakening the production-input detector.
4. Continue the existing whole-userland stability/security roadmap; do not repeat the completed shared-RC trigger audit absent a new concrete workflow change.

## Mandatory handoff checkpoint
Every session must inspect live main and exact-head Actions before ending. Leave this file unchanged when the active task, validation boundary and first next action remain accurate.

## Cross-session execution mandate
Continue on `main` only. Use one atomic Git tree/commit for a multi-file milestone and a non-forced expected-SHA ref update. Keep the archival backup immutable. Do not create branches or PRs, and do not wait idly for pending Actions.
