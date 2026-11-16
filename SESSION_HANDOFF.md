# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeated audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into half-commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Production tri-state HW NAT/FTP/SFE source changes are in `1554a5f3fbb1bf13c3325ad765b7e806d74d1537`; current parent at this checkpoint is `99d8562d8b007e83ceaff397f7c11337b9c6277a`.
- Exact-head 99d856 Process `43971105805`, Firewall Startup `43971105848` and XY-C1 `43971105719` all failed at the last obsolete common checker expectation for radio-policy arguments. The source now writes radio sysfs only inside `if (hwnat_loaded == 1)` and passes literal 1; this follow-up tests that verified-state condition instead of the historical unverified argument. Those checks stopped before build/TRX.
- Exact 99d856 AI handoff `43971105596` failed because the handoff did not include the validator's exact sentence `The checkpoint is mandatory; a document edit is conditional.`; the current file now includes that exact checkpoint rule.
- Exact 99d856 PASS: NVRAM `43971105616`, USB modem identity `43971105688`, infosvr `43971105912`, HTTPD `43971105585`, AliyunDrive `43971106167`, AliDDNS `43971105599`, AdGuardHome `43971106008`.
- No same-SHA firmware Build/TRX/artifact succeeded for 1554/99d856 because the XY-C1 run stopped at the shared checker. Earlier d287 `43967039935` L3 does not apply to these later source changes. Current-source L3 is **NOT VERIFIED**.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains recorded at `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Never alter it. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Complete exact-head CI validation for strict module-state transitions in HW NAT, FTP and SFE. Common checker now validates the HW NAT strict probe/status return and requires radio writes only after `hwnat_loaded == 1`; the handoff now contains the exact accuracy-checkpoint sentence required by CI. Next, inspect any exact-SHA failure and continue implementation without waiting for independent pending runs.

## Verification boundary

- Existing unit regressions exercise tri-state probe errors, strict load/unload postconditions, busy modules and first-error retention.
- Exact 99d856 common checker still failed on the stale radio argument assertion; this commit replaces it. The AI validator also failed on missing exact wording; this commit fixes it.
- Same-SHA Process, Firewall Startup, XY-C1, HTTPD and AI handoff must pass before source slice is called CI-verified. Firmware L3 requires one SHA to pass Build, TRX and artifact upload. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-head Process, Firewall Startup, XY-C1, HTTPD and AI handoff runs for this checker/doc correction. Fix every real failure from the corresponding job log.
2. Continue independent `flush_conntrack_table()` investigation: the declaration is currently `void flush_conntrack_table(char *ip);` and PID 1's `RCN_RESTART_NETFILTER` call ignores the result. Locate definition and all callers before changing API semantics; add injected regression for failure propagation.
3. Keep one logical slice per complete-tree atomic main commit with non-forced expected-SHA CAS. Keep backup read-only; do not claim physical-router L4 without hardware evidence.
