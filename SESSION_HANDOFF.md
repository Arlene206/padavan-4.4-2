## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and handoff recovery were completed once in this session; do not repeat the initial audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. Only `backup/...` and `main` exist.
- Current main before this checker follow-up: `a2f9fcb43a27ecec971ceea2b61f6ec8e8b0fb02`; b7 fixed the earlier writer-count, C regression and missing-heading defects, while this follow-up normalizes whitespace-insensitive source assertions. Latest production-source commit is `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a`, parent `dce5f27b1787abbf83a712159358215839c3cadc`; `5fc78cc` and `b7f6cc` add regression/test, workflow and handoff changes only.
- PR #1 remains closed and unmerged; the CA trust-store feature is already integrated into main.

## Active task

- Finish the Samba V2 four-file transaction source-contract suite. The production journal and four-file host C regression pass. Exact a54 Samba run #46146444805 failed later in the durability checker because `compact()` removes whitespace, but two restore-order assertions still expected spaces in `passdb_path, passdb_snapshot` and `group_path, group_snapshot`. This commit normalizes both.
- Exact parent `a54d21dffbaa188471b09098189112d9e8a010e7`: AI handoff #46146444809 PASS. Samba #46146444805 FAIL at V2 rollback order assertion after the compacted tracking-flag assertion was corrected. No production source defect was reported by that run.

## Verification boundary

- Production source SHA `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a`: XY-C1 #46145600745 SUCCESS including Build XY-C1, TRX verification and artifact upload. This is L3 evidence for the production source tree through 670.
- Checker-only SHA `a2f9fcb43a27ecec971ceea2b61f6ec8e8b0fb02`: HTTPD #46146271738 PASS; AI handoff #46146271744 PASS; Samba #46146271742 failed at whitespace mismatch on `tracks_accounts` in compacted V2 read-state body.
- Checker-only SHA `a54d21dffbaa188471b09098189112d9e8a010e7`: AI handoff #46146444809 PASS; Samba #46146444805 failed at restore argument-order assertion because two expected fragments still contained spaces. The prior tracking-flag assertion now passes. Exact-head rerun required.
- NVRAM transaction audit b7 #46145970994 PASS; Process inventory b7 #46145970859 PASS. Do not repeat broad inventories unless touched inputs warrant it.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, untouched. Hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head Samba run caused by this matcher repair. Require the four-file host regression, durability source contract, snapshot and lifecycle contracts all to PASS. If a later assertion fails, inspect the exact line and normalize only whitespace in snippets taken from `compact()` bodies.
2. Do not rerun HTTPD #46146271738 or AI #46146444809 unless this commit affects their source/workflow inputs; both last exact runs PASS.
3. No production source changed after 670; its XY-C1 Build/TRX/artifact trio #46145600745 is SUCCESS. No additional firmware build is needed for checker-only descendants.
4. After Samba V2 checks are green, continue an independent source-backed stability/security fix; do not repeat completed module audits and keep the frozen backup immutable.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
