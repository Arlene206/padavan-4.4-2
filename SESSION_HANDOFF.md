## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and handoff recovery were completed once in this session; do not repeat the initial audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. Only `backup/...` and `main` exist.
- Current main before this checker follow-up: `a2f9fcb43a27ecec971ceea2b61f6ec8e8b0fb02`; b7 fixed the earlier writer-count, C regression and missing-heading defects, while this follow-up normalizes whitespace-insensitive source assertions. Latest production-source commit is `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a`, parent `dce5f27b1787abbf83a712159358215839c3cadc`; `5fc78cc` and `b7f6cc` add regression/test, workflow and handoff changes only.
- PR #1 remains closed and unmerged; the CA trust-store feature is already integrated into main.

## Active task

- Finish aligning the V2 Samba durability checker with its own compact() normalization. Exact a2 Samba run #46146271742 passed the C journal regression, service failure-gate and account-snapshot parity checks, and the first two durability-checker sections; it then failed because the checker searched for '*tracks_accounts = 0' although compact() removes spaces from the extracted function body.
- The same compact-body issue also exists in four V2 recovery argument-order assertions and the V1 recovery-call contract. Normalize those expected snippets rather than weakening the production checks.
- b7 validation defects were partially repaired by a2: the HTTPD writer-count contract is now one transactional call, and the handoff's required Current repository state heading is present. The a2 handoff validator and HTTPD workflow both pass.

## Verification boundary

- Exact main SHA a2f9fcb43a27ecec971ceea2b61f6ec8e8b0fb02: Samba share run #46146271742 FAIL at scripts/check_samba_transaction_durability.py line 162, '*tracks_accounts = 0' in v2_read and '*tracks_accounts = 1' in v2_read. Earlier C transaction, service failure-gate and ACL/account snapshot steps PASS; the first two checker messages (directory durability and committed/rolled-back recovery reporting) also PASS. Remaining V2 format/recovery/commit-order checks did not execute past this assertion.
- Same a2 SHA: validate-handoff-records run #46146271744 PASS, including CI trigger governance and archive retention (root dev_log 111/119 at that run). HTTPD authentication/disclosure run #46146271738 PASS.
- Parent b7f6cc478d495488a4ad3d9398ef5f5ac2e5fc57: Samba checker previously failed at the V2 PASSWD/GROUP format assertion; a2 corrected that marker check and then exposed the whitespace-normalization mismatch at tracks_accounts.
- b7 XY-C1 run #46145970921 SUCCESS for host tests/source checks only; Build/TRX/artifact skipped by the firmware-input detector because there were no production input changes. No L3/L4 evidence from that run.
- Production source remains based on 670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a (V2 journal implementation and EINTR/fsync hardening); no exact-SHA L3 build/artifact or physical router test is established for that source here.

## Exact first action for the next GPT

1. Fetch the live main SHA and exact-head Actions; resolve THIS COMMIT from the newest dev_log entry.
2. Require the V2 durability checker to progress through all remaining V2 format, V1/V2 read, rollback order, sync order and cleanup assertions; inspect Samba and HTTPD workflow final results on this same SHA.
3. If the checks pass, resume one source-backed stability/security defect outside the now-repeated Samba static-checker repairs. Do not run another broad inventory.
4. Any L3 claim requires Build XY-C1, TRX validation and artifact upload all to succeed on one production-input SHA; no router L4 is verified.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
