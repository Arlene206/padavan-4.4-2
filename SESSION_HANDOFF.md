## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and handoff recovery were completed once in this session; do not repeat the initial audit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. Only `backup/...` and `main` exist.
- Current main before this assignment-limit fix: `2bbb25778ea99a2734e7916b4acc989ad38dc74a` (handoff-only descendant of `0827abadc76f456ba9d67003cbedbdf7854d9e89`). The 0827 exact-head NVRAM run had all policy binaries and the inventory scans in progress at its last poll; exact completion is not yet known. Source checker regex was corrected in 0827.
- PR #1 remains closed and unmerged; the CA trust-store feature is already integrated into main.

## Active task

- Kernel NVRAM's append-only value arena retained credential remnants after ordinary mutations. A shorter in-place update overwrote the new NUL but left the previous value suffix; a longer update abandoned the old slot without wiping it; `_nvram_unset()` ultimately called `_nvram_free()` which freed only tuple metadata. Full arena wipe on later compaction/reset/module exit did not protect the interval between those operations.
- This commit adds secure slot-tail/full-slot helpers, scrubs old value storage only after a successful growth copy, scrubs reused-slot suffixes after shorter replacements, and erases the full slot before tuple free. The existing NVRAM compaction host regression now covers those helpers; the semantic checker verifies production call ordering.

## Exact-SHA CI checkpoint

- Current parent main: `2b1f66627886472ecd16a3e223505fe10e5a8d4c`; it updates the NVRAM CLI assignment workspace from 1024 bytes to the full 4160-byte ABI bound and adds its exact-size test. The previous 0827 NVRAM secure-free checker run #46147228376 has now completed SUCCESS.
- On 0827: AI handoff #46147228361 PASS, HTTPD #46147228338 PASS, Process inventory #46147228339 PASS, NVRAM audit #46147228376 PASS.
- The CLI max-assignment update on 2b1f triggered a new NVRAM audit #46147499779 (IN_PROGRESS at last poll), HTTPD #46147499778 (IN_PROGRESS), Process inventory #46147499808 (IN_PROGRESS) and AI handoff #46147499788 (IN_PROGRESS).

## Verification boundary

- New kernel value-slot hardening: this commit. The existing NVRAM workflow compiles and executes `nvram_compaction_policy.c` under GNU89/Werror/ASan/UBSan, now extended with short replacement, whole-slot release and invalid-range tests. The semantic checker now verifies growth wipe ordering, shrink-tail wipe and tuple-unset wipe. Exact-head NVRAM and XY-C1 Build/TRX/artifact results are pending; no local test execution is available in this API-only session.
- CLI assignment-capacity source/test change on parent `2b1f66627886472ecd16a3e223505fe10e5a8d4c`: exact-head NVRAM #46147499779, HTTPD #46147499778, Process #46147499808 and AI #46147499788 were IN_PROGRESS at the last exact-head query.
- CLI sensitive-buffer erasure source/checker commit `0827abadc76f456ba9d67003cbedbdf7854d9e89`: exact NVRAM #46147228376 SUCCESS; AI #46147228361 SUCCESS; HTTPD #46147228338 SUCCESS; Process inventory #46147228339 SUCCESS.
- Earlier XY-C1 #46145600745 on production source SHA `670e513d99fa0e2d48dfc382ea795ab5c8f6ba1a` passed Build XY-C1, TRX verification and artifact upload, but predates the CLI assignment and value-slot source hardening; it is not L3 evidence for this commit.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, untouched. Physical router/hardware L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head NVRAM transaction-boundary workflow. Require the extended GNU89/Werror/ASan/UBSan compaction policy binary to print `NVRAM sensitive value-slot replacement and release erasure: PASS`, and require semantic source checks for growth, shrink and tuple-release wipes to pass.
2. Inspect exact-head XY-C1 Build XY-C1, TRX verification and artifact upload for this kernel driver source commit. Record L3 only if all three succeed on the exact SHA.
3. Check the existing CLI assignment-capacity run #46147499779 and its related HTTPD/Process/AI checks; do not repeat the already-PASS 0827 checks.
4. Continue a separate source-backed security/reliability fix after this NVRAM slice is validated. Keep the frozen backup immutable; hardware L4 remains NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. Update this file when the active task or first action materially changes; do not create a documentation-only commit merely to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: create all blobs, the complete tree, one commit, then a non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 verification boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, do not repeat completed audits, and do not ask what to do next.
