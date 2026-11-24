## Current repository state

- Live branch: `main`; parent for the SmartDNS stop-contract fixture/workflow slice is `af57e892ed4f1671cd1262feb83c0a98eaea9272`. It adds a runtime guard forbidding name-wide killall, strengthens the source checker to pin PID/start_time revalidation, and ensures both Process workflow push and pull-request filters watch the complete `trunk/user/smartdns/**` subtree.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 remains closed and unmerged.
- Use only atomic main commits: complete one tree and one commit, then non-forced expected-SHA compare-and-swap. No branches, PRs or force pushes.

## Active task

### SmartDNS process lifecycle validation

- Process inventory #45425821194 on `822d8a8c40808290825d838d746d7fec034bb2fc` passed `smartdns_process_lifecycle.sh` runtime cases, then failed because `scripts/check_smartdns_lifecycle.py` expected removed `_smartdns_process_name` and name-wide `killall` behavior. The checker-contract mismatch was corrected by `af57e892ed4f1671cd1262feb83c0a98eaea9272`; this slice adds missing test-level no-killall assertion and triggers Process regression for any SmartDNS source/helper/test modification.
- Current helper separates generic Adbyby detection (`smartdns_named_process_state`) from exact SmartDNS executable/argv/start_time inventory; TERM/KILL uses PID-scoped signals with identity revalidation. UNKNOWN procfs state prevents unsafe escalation. Linux 4.4 retains a final validate-to-signal race without pidfd.

## Verification boundary

- On `822d8a8`, Process #45425821194 failed only after the runtime test passed, at stale source checker assertions. AI docs #45425821211 passed. XY-C1 #45425821347 was still in progress (Build active, TRX/artifact pending) when last inspected.
- Checker contract correction on `af57e892` launched Process inventory #45427108566 (IN_PROGRESS at last inspection); inspect its exact logs and this new commit's Process/AI runs. Do not claim L2 until the whole Process job passes.
- DNS Forwarder #45424750772, Process #45424750807 and AI #45424750770 on `0761c14` passed; XY-C1 path trigger correction is `822d8a8`. L3 requires Build, TRX and artifact all PASS on a single exact production-source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current `main`; inspect exact-head Process inventory and AI handoff for this no-killall test/filter update, including the result/logs of #45427108566 on parent `af57e892`.
2. Poll XY-C1 #45425821347 for parent `822d8a8` and confirm Build, TRX and artifact; diagnose failures without attributing results to a different SHA.
3. After validation, continue a separate P1 reliability/security defect outside DNS Forwarder/SmartDNS, with a real regression and atomic commit. Do not wait idle on PENDING actions.

## Mandatory handoff checkpoint

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once, then implement. Keep work source-led. Before context exhaustion, finish a safe slice, record tests and exact-SHA CI state, and leave the next GPT one precise first action. Create all blobs, one full tree, one commit; update `main` only using a non-forced expected-SHA compare-and-swap. Never force-push or overstate tests/CI/hardware.
