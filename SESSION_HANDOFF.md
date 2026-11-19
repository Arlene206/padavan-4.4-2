# 2026-11-19 continuation checkpoint — finish Switch CLI exact-head regression

## Current repository state

- Repository: Arlene206/padavan-4.4-2; development branch: main only.
- Frozen archival branch: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807. Preserve it byte-for-byte. PR #1 is closed and unmerged; no valid open PR work remains to port.
- Product-source baseline: 3b2c0f3dad38892f801d5778a4d74e3a8d6aaa04. The current follow-up changes only the existing Switch CLI workflow contract and canonical checkpoint records.

## Active task

Close the Switch CLI exact-head regression after the bounded ACL numeric-input hardening.

- trunk/user/utils/src/switch/switch_gsw.c now validates ACL meter and TrTCM rate/burst values before the first MMIO write. Meter range is bounded to the 15-bit /64-scaled field; CIR/PIR are bounded to 16-bit /64-scaled fields; CBS/PBS are bounded to 16-bit fields. The shared strict base-0 parser rejects signs, whitespace, partial tokens, overflow and out-of-range values.
- trunk/user/utils/src/switch/tests/switch_ipv4_parse_policy.c covers inclusive maxima and one-past-maximum rejection for those fields.
- Exact-head Switch CLI run 44457779961 failed at step "Check built switch CLI reports operational failures" before the host regression ran. Root cause: its static contract still expected 12 fail-closed exits and acl_dip_trtcm()'s old argc < 10 guard. The new parse_uint_range_argument() adds one intentional cleanup+failure exit, and production now uses the stricter argc != 10 guard. The current workflow correction updates both stale expectations and asserts cleanup-before-exit in the numeric helper.

## Verification boundary

- L1: production argument validation and pre-MMIO ordering inspected; no C product-source defect found in this bounded numeric-input slice.
- L2: Switch CLI run 44457779961 FAILED in the stale checker before parser compilation/execution. The corrected checker and skipped ASan/UBSan parser regression must pass on the new exact main SHA.
- L3: XY-C1 exact product-source SHA 3b2c0f3dad38892f801d5778a4d74e3a8d6aaa04 is VERIFIED by run 44457779695: Build XY-C1, TRX verification and artifact upload all PASS. This workflow-only follow-up does not invalidate that source baseline.
- On 3b2c0f3dad38892f801d5778a4d74e3a8d6aaa04, AI handoff docs 44457779839, Process inventory 44457779710 and NVRAM boundary 44457779788 PASS. The same SHA's Switch CLI run is the only failed relevant check found in its exact-head run set.
- L4: no physical switch/router validation was performed; NOT VERIFIED.
- No local repository checkout or local host tests were available in this connector-only environment. Do not claim local execution.

## Exact first action for the next GPT

1. Fetch live main, confirm the frozen backup is unchanged, and inspect the new exact-head Switch CLI ioctl failure-status regression and AI handoff documentation contract push runs.
2. If Switch CLI is red, inspect the first failing step and fix the actual source/test/checker defect; the corrected run must execute the GNU89 -Werror ASan/UBSan parser binary and all subsequent wait/Clause 45/Clause 22 regressions.
3. Preserve the L3 claim only for product-source SHA 3b2c0f3dad38892f801d5778a4d74e3a8d6aaa04 and its exact successful XY-C1 run. Do not infer L3 from a newer docs/workflow-only SHA.
4. Once this switch validation is closed, resume the highest-priority unfinished P1 stability/security finding outside completed WAN/LAN DHCP and Switch CLI input-preflight work. Use REFACTOR_ROADMAP.md to select the next module; do not repeat completed audits.

## Development invariants

- Every multi-file milestone is one complete Git tree and one commit on main, followed by a non-forced expected_sha ref update.
- Never create or use another development branch or PR. Never modify the frozen backup. Never weaken or remove meaningful checks or alter workflow path filters to suppress failures.
- Exact-head Actions are the CI source of truth. A PR, stale SHA, pending run, host test, firmware image or physical-device result must be labeled at its actual validation level.
