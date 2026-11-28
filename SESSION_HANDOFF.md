## 2026-11-28 continuation checkpoint

- Repository `Arlene206/padavan-4.4-2`; main-only; bootstrap restored once. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify.
- Parent: `7e50fa510ebc14d98442ded1cd8caa1e93085147` (`test(transmission): fix NVRAM fixture initialization`).
- Transmission NVRAM fix: ports and RPC credentials use `nvram get_checked`, exact-once read-order fixture and CI checker. Initial process run #46140572054 failed because fixture created `$BIN` after writing stub; 7e follow-up creates dirs first and uses `grep --` for `-u`/`-v`. At last poll, 7e Process #46140791596 and Process inventory #46140791606 were IN_PROGRESS; AI docs #46140791629 was IN_PROGRESS.
- WireGuard fix on parent `f0d35e73cabeefdde88e132ac94df6bc4d660603`: four required settings now use `nvram get_checked`; per-key error injection and no side effects are added to lifecycle test. AI docs #46140718064 PASS, inventory #46140718059 PASS; Process #46140718061 was cancelled when 7e superseded it, so WireGuard regression still needs exact-SHA execution.
- New Aria2 task: `aria.sh` had unchecked `nvram get` for `aria_pport`, `aria_rport`, `http_proto`. New helper reads all three with `get_checked`, preserves empty-port defaults, snapshots HTTP selector once, and runs before storage/config/permission side effects. Regression covers statuses 41/42/43, exact call order, failed read no-storage side effects and successful empty/non-empty settings. Python checker and workflow path filters are extended to execute it.
- Prior 1ac Transmission build #46140572053 still IN_PROGRESS; its Process #46140572054 failed only in fixture bootstrap (fixed in 7e), USB stability #46140572097 PASS, AI #46140572083 PASS, inventory #46140572060 PASS.
- NVRAM cache-wrap SHA `4f09b7bc0cff0aae3a13a4705edd3bf3af489b53`: XY-C1 run #46139431417 eventually SUCCESS; Build job SUCCESS and artifact `XY-C1-firmware-4f09b7bc0cff0aae3a13a4705edd3bf3af489b53` ID 15261597583 exists. This is L3 evidence for that exact SHA only, not later commits. NTP L3 evidence only on `c052506cbe9463f2719494d237e5323fe1c261c8`.
- TTYD source SHA `5759332e5f3da1212d3caa937684a663c6e39a86`: lifecycle regression PASS, process inventory PASS, AI docs PASS; XY-C1 #46140033275 remained IN_PROGRESS at last poll.
- No local MIPS toolchain/kernel build or physical hardware available. L4 NOT VERIFIED. `REFACTOR_ROADMAP.md` near size cap.

## Current repository state

- Parent: `7e50fa510ebc14d98442ded1cd8caa1e93085147`.
- Aria2 patch modifies startup script, adds a regression, updates the Python checker and Process workflow path filters, and updates logs/handoff in one atomic tree/commit.
- Frozen backup remains unchanged.

## Active task

- Verify Aria2 exact-SHA Process/AI/XY Actions, especially new `aria2_nvram_read_fail_closed.sh` test via `check_optional_app_start_status.py`.
- Also confirm the Transmission fixture correction and WireGuard checked-read test on their 7e exact Process run; if Process fails, inspect exact failing step/log and repair.
- L3 requires exact same-SHA XY-C1 Build, TRX and artifact checks. Hardware L4 is not verified.

## Exact first action for the next GPT

1. Fetch live main and exact-SHA Actions for the Aria2 patch.
2. Verify the checker passes both the Transmission NVRAM fixture and WireGuard per-key fail-closed fixture, and executes the new Aria2 fixture.
3. Inspect exact failing logs if Process fails; do not weaken tests to hide real defects.
4. Poll the latest XY-C1 and ensure Build, TRX validation and artifact upload all PASS on a single SHA.
5. Continue distinct source fixes while unrelated Actions are PENDING; keep backup immutable and use complete-tree, expected-SHA, non-forced main updates.

## Verification boundary

- NVRAM cache-wrap SHA `4f09b7bc0cff0aae3a13a4705edd3bf3af489b53`: XY Build PASS + artifact ID 15261597583; applies only to that SHA.
- TTYD SHA 575: shell regression/inventory/AI PASS; XY pending.
- WireGuard SHA f0: AI/inventory PASS; Process run cancelled by superseding commit before test step, needs rerun.
- Transmission fixture correction SHA 7e: Process and inventory pending at last poll; AI pending.
- Aria2 patch: exact-SHA validation required.
- Physical L4: NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. If facts materially change, update this file accurately.

# Cross-session execution mandate

- One logical task, one atomic main commit: blobs, complete tree, commit, non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, and do not repeat completed audits or ask what to do next.
