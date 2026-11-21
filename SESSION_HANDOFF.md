# 2026-11-20 continuation checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Each logical task is one atomic commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real source defects with focused regressions; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, inspect the full diff, publish atomically, and preserve the exact next action here.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live `main`, verify the frozen backup SHA and check exact-SHA Actions at session end. Update this file only when the baseline, evidence, blocker or first next action materially changes. Do not make no-op commits.

## Current repository state

- Parent `main` SHA for the validation follow-up: `4e4b1e51ec58958823e2c94890d4e073450bcb87`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- WAN DHCP run `#44874862762` failed at the DHCP lease policy binary under ASan: the expanded 12-value tuple test still declared its local `values` array with 11 elements. The fake tuple and 12 loop iterations correctly expanded; this one declaration was missed and is corrected in this commit.
- AI docs run `#44874862864` failed because the handoff paragraph paraphrased rather than exactly preserving the checker-required sentence `The checkpoint is mandatory; a document edit is conditional.`. The exact required sentence is restored here.
- MiniUPnP exact-head run `#44874862770` PASSED on `4e4b1e51ec58958823e2c94890d4e073450bcb87`, including the first-error delete contract and both IPTC API variants.
- XY-C1 run `#44874862823` was still in progress at the last poll. Its Build/TRX/artifact steps must be checked before making any L3 claim. The prior full L3 evidence remains run `#44809783977` on product-source SHA `0b5eef8bd00fd6e732449bcf241949df84b4f40c`.
- Physical-router L4 remains NOT VERIFIED.

## Active task

1. Recheck the exact-head WAN DHCP callback and AI documentation Actions after this focused test/handoff correction.
2. Inspect exact-head XY-C1 Build, TRX and artifact step conclusions, and diagnose any other failed exact-head workflow at its earliest causal step.
3. When validations are green, continue the user-space reliability queue outside MiniUPnP, prioritizing a concrete P1/P2 defect with a focused regression.

## Verification boundary

- In the failed WAN run, both DHCP policy C source compilation and ASan execution reached the 12-value tuple function; UBSan/ASan pinpointed the exact out-of-bounds array declaration described above. Later workflow steps were skipped. The corrected declaration requires a fresh exact-head run.
- AI docs failed only on the exact wording requirement; the rule itself was already present semantically. The exact sentence is restored for a fresh run.
- MiniUPnP PASS on `4e4b1e51ec58958823e2c94890d4e073450bcb87` (run `#44874862770`).
- XY-C1 run `#44874862823` was pending; do not claim L3 until Build/TRX/artifact all PASS on one SHA. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

Fetch latest `main` and inspect the exact-head WAN DHCP callback, AI docs, XY-C1, HTTPD authentication and other in-progress/failed Actions. Confirm the corrected 12-entry fixture and handoff wording pass; read the first causal failure log for any other red workflow. Continue with the next independent P1/P2 source defect once the current slice is green.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
