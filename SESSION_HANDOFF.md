## 2026-11-28 continuation checkpoint

- Repository `Arlene206/padavan-4.4-2`; continue on `main` only. Startup recovery was performed once; do not rerun repository/bootstrap audit.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.
- Parent for current task: `1ac1fa0919f95e11474325e344d19de89142025d` (`fix(transmission): deduplicate checked port reads`).
- Transmission checked-read change: `trmd_pport`, `trmd_rport`, `http_username`, `http_passwd` switched to `nvram get_checked`; the port reads are now exactly once and predate watch permission changes. New four-key failure injection test is wired into Process lifecycle CI. On parent 1ac1, AI docs #46140572083 PASS, Process #46140572054 IN_PROGRESS, XY-C1 #46140572053 IN_PROGRESS, USB stability #46140572097 IN_PROGRESS; Process inventory #46140572060 PASS.
- Parent 84ca Transmission attempt had a duplicate port read block; the 1ac1 correction removed it. Do not repeat the old parent as a clean verification target; only 1ac1's exact-SHA checks assess the fixed duplicate behavior.
- TTYD checked-read commit `5759332e5f3da1212d3caa937684a663c6e39a86`: lifecycle shell test PASS; Process inventory #46140033271 PASS; AI docs #46140033305 PASS; Process integration passed on d7 after updating the success banner. XY-C1 #46140033275 had remained IN_PROGRESS.
- Cache-wrap NVRAM commit `4f09b7bc0cff0aae3a13a4705edd3bf3af489b53`: NVRAM/AI/HTTPD/Process inventory PASS; XY-C1 #46139431417 previously IN_PROGRESS. No L3 without exact-SHA firmware Build/TRX/artifact success. NTP L3 evidence is limited to `c052506cbe9463f2719494d237e5323fe1c261c8`.
- Current implementation: WireGuard uses a common `required_setting()` helper for four startup reads but still used legacy `nvram get`. This task changes it to `get_checked` and adds per-key fail-closed regression coverage.
- No local MIPS build or physical router test is available. Physical L4 NOT VERIFIED; roadmap near size cap.

## Current repository state

- Parent: `1ac1fa0919f95e11474325e344d19de89142025d`.
- WireGuard patch changes its helper to get_checked and extends the existing shell fixture. It does not alter normal successful setting semantics.
- Frozen backup unchanged.

## Active task

- Verify WireGuard required-setting read failures are propagated on exact-SHA Process, AI and XY-C1 checks. Regression statuses 41–44 are independently injected per key; no interface, NAT or private key may be staged before all settings are read.
- Also resolve the current Transmission Process/XY results for SHA 1ac1. Inspect the exact failing step/log if Process is FAIL; do not infer PASS from inventory/AI.
- Continue independent work if firmware build remains pending.

## Exact first action for the next GPT

1. Fetch live main and exact-SHA Actions for the WireGuard change; verify Process/AI/XY/USB results.
2. Poll prior SHA 1ac1 Process #46140572054, XY-C1 #46140572053 and USB #46140572097. Their results apply to 1ac1 only.
3. If a test fails, read its exact job log and fix causal issue; preserve original production behavior.
4. If XY-C1 is pending, continue independent userland stability/security work. L3 only after Build, TRX and artifact all pass on same SHA.
5. Preserve backup; commit complete Git trees, use expected-SHA non-forced main updates.

## Verification boundary

- 1ac1 Transmission: AI docs PASS; inventory PASS; Process/XY/USB checks in progress at last poll.
- TTYD code SHA `5759332e5f3da1212d3caa937684a663c6e39a86`: dedicated ttyd shell regression PASS; inventory PASS; its XY-C1 was pending at last poll.
- WireGuard patch: new exact-SHA checks required.
- Physical L4: NOT VERIFIED.

## Mandatory end-of-session handoff checkpoint

Mandatory handoff checkpoint. The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. If facts materially change, update this file accurately.

# Cross-session execution mandate

- One logical task, one atomic main commit: blobs, complete tree, commit, non-forced expected-SHA update. Never force-push or publish per-file partial commits.
- Before context exhaustion, finish safe source slices, inspect exact-head Actions, and leave the next GPT an accurate first action and L1-L4 boundary.
- Keep working while independent Actions are pending; PASS continues, FAIL requires causal repair, PENDING permits independent work.
- Recover once, implement continuously, and do not repeat completed audits or ask what to do next.
