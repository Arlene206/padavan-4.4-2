# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current parent for the bounded procfs read slice: `53dda2c844e18cfb879abc3bf4b96bb50d46fd2d` (`fix(shared): bound procfs read EINTR retries`).
- Exact SHA `53dda2c`: Process inventory `44632416173`, detect_link signal `44632416162`, GPIO lifecycle `44632416043`, DNSMasq `44632416164`, VPN server `44632416149` and crond `44632417092` PASS. Process umbrella `44632416363`, NVRAM audit `44632416256` and XY-C1 `44632416166` were still running at checkpoint.
- AI docs run `44632416060` failed because `process_io_retry_policy.h` was added only to the push filter of `process-lifecycle-regression.yml`; the mandatory checker compares push and pull_request path sets. This follow-up adds the helper path to pull_request too; no production source changes.
- Previous lock-wait fix `d57cce1`: AI docs, HTTPD, Process inventory, Inadyn, DNSMasq, Process regression, VPN, MiniUPnP config and NVRAM all PASS; XY-C1 `44631856549` was still running at last poll.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival only. PR #1 remains closed/unmerged.
- No physical-router result is available. Host Actions are L2; XY-C1 Build/TRX/artifact on one exact source SHA is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Source fix on `53dda2c`: `proc_default_read_file()` now caps consecutive `read(2)` EINTR retries at eight for both normal procfs reads and its overflow probe. Unit regression checks the bound and exhaustion errno.
- The first AI-doc run exposed a workflow-only defect: Process lifecycle push and pull_request filters no longer matched after adding the new production helper path. This follow-up restores parity by adding `process_io_retry_policy.h` to both trigger path sets.
- No change to procfs source or retry policy is required for this CI correction.

## Exact first action for the next GPT

1. Verify AI docs rerun after trigger-path parity correction, then inspect exact SHA 53dda2c Process umbrella, NVRAM and XY-C1 outcomes.
2. If Process scanner regression fails, inspect its GNU89/ASan/UBSan or source-contract job logs; preserve the bounded retry behavior and UNKNOWN-on-I/O-error semantics. XY-C1 L3 requires Build, TRX verification and artifact upload on exact 53dda2c source SHA.
3. Continue with an independently evidenced cross-userland stability/security fix; avoid reopening bounded service-lock and procfs-read loops without new evidence.

## Verification boundary

- L1: shared procfs reader uses eight-retry EINTR limits for both normal and overflow-probe reads; exhaustion fails closed.
- L2: Process inventory source/test run `44632416173` PASS; Process regression `44632416363` and NVRAM `44632416256` were in progress at checkpoint. AI docs failed only on workflow push/PR path parity and this commit fixes it.
- L3: XY-C1 `44632416166` was in progress for exact source SHA 53dda2c; Build/TRX/artifact conclusion remains pending.
- L4: target procfs signal-load behavior and physical-router service supervision remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
