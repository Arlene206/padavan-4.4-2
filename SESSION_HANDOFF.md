## 2026-11-28 continuation checkpoint

- Repository `Arlene206/padavan-4.4-2`; continue on `main`; bootstrap recovered once. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Parent: `65ed152afe517c36ee6e05c34f1f4942556698a6` (`fix(pppoe): use checked reads for MWAN settings`).
- DDNSTO source change SHA `3f1cab20589e858b2db951b18821194249424957`: DDNSTO test failed once due stale NVRAM log; correction `5065efb84e8a58c1a56f4313eb4bfed08107f4a2` clears log; exact DDNSTO #46141395670 PASS, Process inventory #46141395659 PASS, AI docs #46141395666 PASS. XY-C1 #46141285573 for production SHA 3f remained IN_PROGRESS last poll.
- `mwan.sh` PPPoE helper migration parent `5065`: eight required settings now use checked getter; test injects statuses 41–48 and validates no interface, option-file or PPPD side effects. On `65ed` WAN lifecycle workflow #46141657596 PASS, Process inventory #46141657586 PASS, AI docs #46141657624 PASS. Note: #46141657596 is the VPN-server lifecycle job and is not the PPPoE-MWAN shell regression; the latter is in XY-C1's `Verify PPPoE-MWAN cleanup fail-closed and partial launch rollback` step. XY-C1 #46141657601 was IN_PROGRESS with this step still pending at last poll.
- The current follow-up extends checked getter semantics to `detect.sh`: six required/preflight reads (PPP count, 443 selector, static-rule enable/count, dynamic static IP/interface) now use `nvram get_checked`, plus tests for PPP-count failure and the pre-existing selector failure matrix. Existing XY workflow executes both shell regression scripts and static checker.
- Aria2 SHA `76646cffc52d09fcb9b7a67052d5ae64b1b14462`: Process/inventory/USB PASS, AI duplicate path-filter problem fixed by 570 with Process #46141148152 and AI #46141148176 PASS; XY #46141026720 was IN_PROGRESS last poll.
- Transmission + WireGuard test SHA `7e50fa510ebc14d98442ded1cd8caa1e93085147`: Process #46140791596 PASS, inventory #46140791606 PASS, AI #46140791629 PASS.
- NVRAM cache-wrap SHA `4f09b7bc0cff0aae3a13a4705edd3bf3af489b53`: XY-C1 Build PASS + artifact ID 15261597583. TTYD SHA `5759332e5f3da1212d3caa937684a663c6e39a86`: XY-C1 success. Exact-SHA evidence only. NTP L3 only on `c052506cbe9463f2719494d237e5323fe1c261c8`.
- No local MIPS/kernel build or router hardware; physical L4 NOT VERIFIED. Roadmap near cap.

## Current repository state

- Parent: `65ed152afe517c36ee6e05c34f1f4942556698a6`.
- Current commit modifies `detect.sh`, its NVRAM failure regression and `check_pppoe_mwan_status.py`. It follows the prior `mwan.sh` migration; both avoid treating legacy getter backend failures as valid empty values.
- Frozen backup is unchanged.

## Active task

- Verify the combined PPPoE-MWAN `mwan.sh` and `detect.sh` changes through exact-SHA XY-C1. Require both shell tests, static checker, XY-C1 build, TRX verification and artifact upload.
- The prior `mwan.sh` task on SHA `65ed` has WAN/Process/AI PASS, but the WAN workflow was VPN-specific. XY-C1 #46141657601 was IN_PROGRESS and still must run the PPPoE-MWAN test step.
- Poll prior DDNSTO XY #46141285573 and Aria2 XY #46141026720 independently; exact SHA only.

## Exact first action for the next GPT

1. Fetch current main and exact-SHA Actions for the `detect.sh` checked-read commit.
2. Inspect XY-C1 job step outcomes for `Verify PPPoE-MWAN cleanup fail-closed and partial launch rollback`; confirm both tests and static checker pass before build/TRX/artifact.
3. Poll parent XY runs #46141285573 (DDNSTO SHA 3f), #46141026720 (Aria2 SHA 766), and #46141657601 (65ed PPPoE helper) separately.
4. If any test fails, read the exact job log and fix the cause; do not treat PENDING as failure. Keep backup immutable and only use complete-tree atomic commits with expected-SHA, non-forced main updates.

## Verification boundary

- `5065`: DDNSTO test, Process inventory and AI docs PASS; production XY #46141285573 pending on source SHA 3f.
- `65ed`: WAN VPN-server workflow PASS, Process inventory PASS, AI docs PASS; actual PPPoE-MWAN checks in XY-C1 #46141657601 still pending.
- Current detector change: exact-SHA XY-C1 is required.
- Aria2 XY #46141026720 pending; Transmission/WireGuard 7e workflow tests passed.
- Physical L4: NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. If facts materially change, update this file accurately.

# Cross-session execution mandate

- One logical task, one atomic main commit: blobs, complete tree, commit, non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, and do not repeat completed audits or ask what to do next.
