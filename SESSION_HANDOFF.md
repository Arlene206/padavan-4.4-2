## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. Startup protocol and handoff recovery were completed once; do not repeat the initial audit.
- Main immediately before this source slice: `fc724d8efc810d2f063bb7cef7427eab118f84ce`. The WAN/VIPTV DHCP RELEASE-before-stop fix at this parent has exact-head WAN teardown and WAN DHCP callback workflow PASS results.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. PR #1 remains closed/unmerged; its CA trust-store feature is integrated.

## Active source task

- Bound networkmap ASUS IBOX `ProductID[32]` consumption to its fixed packet field. The legacy consumer passed the field to `is_valid_hostname()` (uses `strlen()`) and a `%s` diagnostic without verifying NUL within the field. A malformed peer response could cause adjacent firmware-version/mode bytes to be interpreted as part of the product identifier. This is a fixed-field parsing defect; the 512-byte receive buffer is zeroed, so this is not claimed as an out-of-buffer read.
- Production change adds `networkmap_fixed_field_is_nul_terminated()`, gates hostname validation/copy on the ProductID boundary, and uses precision-limited `%.*s` logging. Host regression covers missing terminator, final-byte terminator, empty field, adjacent-byte terminator, null pointer and zero capacity.
- Local isolated test with `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined`: PASS. Exact-head networkmap checker, tests and XY-C1 Build/TRX/artifact are pending on this atomic source commit.

## Exact-SHA CI checkpoint at the parent

- `fc724d8efc810d2f063bb7cef7427eab118f84ce`: WAN interface teardown #46148954046 PASS; WAN DHCP callback #46148954142 PASS; AI handoff documentation #46148954052 PASS. XY-C1 #46148953975 is still in progress at Build XY-C1; TRX/artifact not yet complete.
- Prior fully validated production checkpoint `13339bed554e334b2e9c0176533d6e4a97c7907e`: XY-C1 #46147869371 PASS with Build XY-C1, TRX verification and artifact upload; Process inventory #46147869397 PASS.
- NVRAM boundary audit #46148136209 and handoff validation #46148136319 PASS at the earlier NVRAM fix SHA; do not repeat that audit without a new defect.

## Verification boundary

- Local networkmap fixed-field host test PASS (GNU89, Werror, ASan/UBSan).
- Exact-head networkmap workflow and XY-C1 Build/TRX/artifact for this newer production tree: PENDING. Do not claim L3 until one exact production-source SHA passes Build, TRX and artifact upload.
- Physical router L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Read live `main` and inspect exact-head Actions for this networkmap packet-field commit: networkmap signal/packet-string regression, AI handoff documentation and XY-C1 firmware validation.
2. If any checker/test fails, inspect the failing step and make the smallest causal source/test correction in one complete-tree atomic commit. If XY-C1 remains pending, continue an independent non-networkmap stability/security slice.
3. After this guard passes, continue on an uncompleted userland module; do not restart broad inventories or revisit completed NVRAM/Samba/HTTPD work without new evidence.
4. Keep the frozen backup unchanged. Distinguish CI Build/TRX/artifact (L3) from physical-router L4.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. Inspect live repository state and exact-head Actions before ending. Update this file only if the active task or first actionable inspection materially changes; do not create a documentation-only commit to record routine CI transitions.

## Cross-session execution mandate

- One logical task, one atomic main commit: all blobs → one complete tree → one commit → non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, preserve security/architecture/compatibility and necessary tests, and do not ask what to do next.
