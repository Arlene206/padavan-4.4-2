## 2026-11-28 continuation checkpoint

- Repository `Arlene206/padavan-4.4-2`; main-only; bootstrap recovered once. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify.
- Parent: `76646cffc52d09fcb9b7a67052d5ae64b1b14462` (`fix(aria2): fail closed on NVRAM startup reads`).
- Aria2 startup now reads `aria_pport`, `aria_rport`, `http_proto` via `nvram get_checked` before storage/config/permissions/launch. Regression injects statuses 41/42/43, verifies exact read order and no storage creation, and checks successful empty defaults plus normal values. The AI docs run #46141026688 FAILED because `scripts/check_optional_app_start_status.py` was added to path filters even though it was already in both lists. This follow-up removes only the duplicate entries; existing checker path plus new Aria2 source/test path filters remain in both push and pull-request allowlists.
- At last poll on 766: Process #46141026685, inventory #46141026697, XY-C1 #46141026720 and USB stability #46141026746 were IN_PROGRESS. AI docs #46141026688 failed as above. New exact-head runs required after path correction.
- Aria2 checker/test are executed through `scripts/check_optional_app_start_status.py`; the Process workflow path filter now contains the Aria2 script and new NVRAM fixture. The checker statically pins checked reads and executes the new shell regression.
- Transmission parent `1ac1fa0919f95e11474325e344d19de89142025d`: AI docs PASS, USB PASS, inventory PASS; Process failed only because its NVRAM fixture wrote `bin/nvram` before creating bin; the 7e correction fixed this. On SHA `7e50fa510ebc14d98442ded1cd8caa1e93085147`, Process #46140791596 PASS, inventory #46140791606 PASS, AI docs #46140791629 PASS. This exact tree included Transmission fixture correction and WireGuard `get_checked` source/test; both dedicated tests passed in Process.
- WireGuard source patch was introduced at `f0d35e73cabeefdde88e132ac94df6bc4d660603`; Process on f0 was cancelled by the subsequent 7e push before that test ran, but WireGuard regression subsequently passed in Process #46140791596 on 7e exact SHA.
- NVRAM cache-wrap SHA `4f09b7bc0cff0aae3a13a4705edd3bf3af489b53`: XY-C1 Build job PASS, artifact ID 15261597583 uploaded. Applies only to that exact SHA. NTP L3 only on `c052506cbe9463f2719494d237e5323fe1c261c8`.
- TTYD source SHA `5759332e5f3da1212d3caa937684a663c6e39a86`: Process lifecycle regression/inventory/AI PASS and XY-C1 run #46140033275 SUCCESS (Build result for that SHA only).
- No local MIPS toolchain/kernel build or physical hardware; L4 NOT VERIFIED. `REFACTOR_ROADMAP.md` near cap.

## Current repository state

- Parent: `76646cffc52d09fcb9b7a67052d5ae64b1b14462`.
- This follow-up only removes duplicate `scripts/check_optional_app_start_status.py` entries from two path-filter lists. Aria2 path entries remain in both lists.
- Frozen backup unchanged.

## Active task

- Verify AI docs path-filter contract after the duplicate removal, and confirm Process runs `aria2_nvram_read_fail_closed.sh` from `check_optional_app_start_status.py`.
- Verify Aria2 NVRAM failure injections for all 3 keys, successful empty defaults and non-empty snapshots.
- Inspect exact-head XY-C1 Build/TRX/artifact; no L3 claim except exact SHA with all checks passing.

## Exact first action for the next GPT

1. Fetch live main and exact-SHA AI/Process/inventory/XY-C1/USB Actions after this follow-up commit.
2. Confirm AI docs succeeds with duplicate-free, matching push/PR path sets; confirm Process runs Aria2's new test.
3. If Process fails, inspect its exact failing step/log and fix cause; don't weaken its assertions.
4. Continue independent implementation while XY or USB jobs are pending. Backup immutable; no force-push; full-tree expected-SHA updates only.

## Verification boundary

- SHA 7e: Process PASS (Transmission fixture and WireGuard lifecycle test both explicitly PASS), inventory PASS, AI docs PASS.
- SHA 766: Aria2 source/test introduced; AI docs failed on duplicate checker path entry; Process/XY/USB in progress at last poll.
- Follow-up commit: path-filter correction; exact-SHA verification required.
- NVRAM cache-wrap SHA `4f09b7bc0cff0aae3a13a4705edd3bf3af489b53`: XY-C1 Build PASS + artifact; exact-SHA only.
- TTYD SHA 575: XY-C1 SUCCESS; exact-SHA only.
- Physical L4: NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. If facts materially change, update this file accurately.

# Cross-session execution mandate

- One logical task, one atomic main commit: blobs, complete tree, commit, non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, and do not repeat completed audits or ask what to do next.
