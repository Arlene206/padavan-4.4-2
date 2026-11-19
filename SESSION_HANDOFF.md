# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current main parent for this WAN checker correction: `e5bd15b2c13c33f46564d2a04863db41e7898ca7` (`fix(shared): bound MTD flash I/O EINTR retries`).
- Exact `8987975` runs: AI docs `44635983515` PASS; HTTPD security `44635983452` PASS after the prior count fix; WAN DHCP callback `44635983525` failed only because the new checker referenced undefined Python variable `wan` instead of its existing source text variable `source`. This follow-up fixes that checker typo; source safety helper itself is present and its restoration/fail-closed assertions remain.
- Exact `342631b`: firewall-startup policy `44635384398` and NVRAM audit `44635384340` PASS; XY-C1 `44635384278` remained IN_PROGRESS at last poll.
- Current production source SHA `e5bd15b2c13c33f46564d2a04863db41e7898ca7` adds bounded EINTR handling to MTD flash read/write. Exact XY-C1 `44636408751`, Process umbrella `44636409282`, Process inventory `44636408664`, NVRAM `44636408634` and AI docs `44636408628` were IN_PROGRESS at last poll.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged. No physical-router result.

## Active task

- Checker-only follow-up: the WAN DHCP workflow was validating the stronger `restore_wan_after_firewall_failure()` helper but passed the nonexistent variable `wan` to the source-body parser. The workflow already stores the source in `source`; this change corrects the reference without weakening the restoration/fail-closed assertions.

## Exact first action for the next GPT

1. Verify WAN DHCP callback rerun after the checker variable correction; also poll the exact e5 MTD Process, Process inventory, NVRAM, AI-doc and XY-C1 workflows.
2. For any source build failure, inspect exact job logs and repair it without weakening the eight-retry MTD I/O policy. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on one source SHA.
3. Continue with a new independently evidenced cross-userland stability/security defect after validation.

## Verification boundary

- WAN restoration source: production helper uses the original physical-WAN restoration routine and forces the physical interface down if the fallback firewall recovery fails. The previous workflow failure was a Python `NameError` in the added checker variable reference; this correction uses the defined `source` variable.
- MTD flash source SHA `e5bd15b`: Process/AI/NVRAM/XY-C1 exact-head runs are pending. No L2/L3 result has yet been claimed for this MTD change.
- Exact `342631b` NVRAM and firewall-startup checks PASS; its XY-C1 run remained pending.
- No local compile or physical-router L4 result is claimed.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
