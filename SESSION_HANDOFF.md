## 2026-11-28 continuation checkpoint

- Repository Arlene206/padavan-4.4-2; main-only development. This follow-up parent is de86bafde7e9399b2eb2653de8feae338007b10a.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and is immutable. PR #1 remains closed/unmerged. Startup recovery completed once; do not repeat it.
- AliyunDrive product source 19d51bab853ed2b904e44333e3f6ce0bd72059a4 reached L2 PASS and exact XY-C1 #46135763583 L3 PASS: firmware Build, TRX verification and artifact upload all succeeded. Checker/docs-only descendants do not change firmware inputs. Physical L4 remains unverified. Aliyun's final procfs-to-kill syscall race remains documented.
- NTP resolver bounds hardening was introduced in de86bafde7e9399b2eb2653de8feae338007b10a. Its first commit accidentally emitted a two-backslash C character literal in memchr; THIS follow-up corrects it to the C NUL literal and aligns the source checker. Intermediate Actions on de86baf test a broken spelling and are not proof for the corrected SHA.
- The NTP regression covers unterminated server0 and server1, verifies ENAMETOOLONG and byte-for-byte preservation. Existing watchdog workflow filters cover changed source, test and checker files.
- REFACTOR_ROADMAP.md remains near its size cap; avoid status-only edits.

## Current repository state

- Parent before this follow-up: de86bafde7e9399b2eb2653de8feae338007b10a.
- NTP files: trunk/user/rc/src/watchdog_ntp_policy.h, trunk/user/rc/src/tests/watchdog_ntp_policy.c, scripts/check_watchdog_ntp_lifecycle.py.
- Intermediate SHA de86baf Actions: Watchdog #46136909848, NVRAM #46136909868, AI docs #46136909844, Process #46136909846, XY-C1 #46136909821 were in progress at last poll. They do not validate this literal correction.
- No local checkout or host compile available; fresh exact-head Actions are required.

## Active task

- Ensure watchdog_ntp_bounded_string_length calls memchr(value, '\0', value_size) with one C backslash.
- Python source contract must match the same expression.
- Resolver validates both input buffers before mutation, preserves one-server mirroring and pool.ntp.org fallback, and returns ENAMETOOLONG for unterminated buffers.

## Exact first action for the next GPT

1. Fetch live main and confirm the corrected-literal commit SHA.
2. Inspect intermediate run logs for #46136909848, #46136909868, #46136909844, #46136909846, #46136909821 only to understand old failures; do not use them as corrected-SHA evidence.
3. Check corrected exact-SHA Watchdog NTP host/source contract, Process inventory, AI docs, NVRAM and XY-C1 Build/TRX/artifact runs.
4. Repair a failing step causally in a new atomic commit; PENDING permits independent work.
5. L3 requires corrected product SHA Build, TRX and artifact upload PASS. L4 remains NOT VERIFIED.

## Verification boundary

- L1: explicit-capacity resolver previously used unbounded strlen; both inputs are now bounded before mutation. This follow-up corrects the C NUL literal spelling.
- L2: NOT VERIFIED for this correction pending exact-head Watchdog NTP ASan/UBSan regression and source checker. No local run.
- L3: prior Aliyun source L3 PASS on 19d51; new NTP source requires its own exact-SHA XY-C1 Build/TRX/artifact pass.
- L4: NOT VERIFIED; no physical router validation.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Before ending, inspect live main, exact-head Actions, active task and validation boundary. Update only when facts materially change; do not create a no-op commit.

# Cross-session execution mandate

- One logical multi-file task, one atomic main commit: create all blobs, one tree and one commit, then update main with non-forced expected-SHA compare-and-swap. Never force-push or publish per-file intermediate commits.
- Before context exhaustion, finish the safe slice, publish it, inspect exact-head Actions and leave a precise first action and truthful L1-L4 boundary.
- Recover once, then execute; do not repeat completed audits or ask what to do next.
- PASS permits the next issue, FAIL requires causal repair, PENDING permits independent work.
