# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Confirm the main/source baseline, exact-head Actions, validation boundary, blockers, frozen backup and precise next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Continue one engineering effort across GPT conversations: avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next. Finish safe slices, add regression coverage, inspect exact-SHA Actions, commit atomically and preserve the next task.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, create a development branch/PR, or modify the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; current main/source parent for this NVRAM slice: `a7e3eda16eca5d20701a0240287ebecffed443d7`.
- PPP/L2TP argv-based launch and xl2tpd result normalization are complete; VPN server restore accepts status 1 (already running) and fails only for negative status.
- IPv6 native rollback and WAN teardown flush neighbour entries between route and address cleanup; regression coverage is present on source SHA `f943ccbd8efe3e125afcec8165bf9e235b390e06`.
- Exact-head a7e3 AI handoff `43995311437`, Process fail-closed `43995311251`, infosvr `43995311336`, AdGuardHome `43995311368`, AliDDNS `43995311339`, and AliyunDrive `43995311301` PASS. NVRAM `43995311260` was auditing kernel API callers. XY-C1 `43994632505` was still in Build on f943, so that run is not same-head verification for docs-only a7e3.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Audit NVRAM adapter result handling. This slice fixes the previously ignored `close()` results after SET, COMMIT and CLEAR ioctls. Verify the new pure policy regression, source contract and exact-head NVRAM workflow; then verify same-SHA XY-C1 Build/TRX/artifact on a source commit.

## Verification boundary

- Predecessor `f87343b7e7a2da599c16baf1192ec1561dc44a9b` has product-source MIPS/uClibc L3 VERIFIED.
- The xl2tpd result normalization and VPN restore-sentinel correction are present on parent `79ba2d6f818d0cb541032b3440834b883b1e531d`; VPN and related source checks passed, with the Process fail-closed contract and AI handoff wording failures now fixed here. The IPv6 neighbour-cleanup source/tests and target build require new exact-SHA verification.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head NVRAM workflow and inspect its kernel API caller inventory; triage any concrete raw ioctl or unsafe caller.
2. Confirm the new close-status host test and `check_nvram_transaction_boundary.py` pass on the commit SHA.
3. Confirm XY-C1 Build, TRX verification and artifact upload on one source SHA; if pending, continue an independent source issue.
4. Preserve the backup and log history; make one complete Git tree/commit and update main with a non-forced expected-SHA compare-and-swap.
