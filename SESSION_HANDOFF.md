# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- Create all changed blobs, one complete tree and one commit with the current main parent; update main with non-forced compare-and-swap using that exact SHA.
- Verify Actions by exact `head_sha`. A pending run does not block independent work; a failed relevant run must be triaged before claiming validation.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter or delete it. Develop on main.

## Current repository state

- Current parent for the shared wait-loop hardening slice: `16b755661ec1a33cd255dec10d4bac389816a8b3` (`ci(process): align retry policy trigger paths`).
- The procfs bounded-read source SHA `53dda2c844e18cfb879abc3bf4b96bb50d46fd2d` passed Process inventory `44632416173`, detect_link `44632416162`, GPIO `44632416043`, DNSMasq `44632416164`, VPN `44632416149`, crond `44632417092` and NVRAM `44632416256`. Process umbrella `44632416363` and AI docs `44632416060` initially failed only because Process workflow push/PR path filters differed; workflow/doc follow-up `16b7556` reruns AI docs `44632968248` and Process regression `44632969368` PASS.
- Exact XY-C1 `44632416166` for production source SHA `53dda2c` remained IN_PROGRESS at last poll; Build/TRX/artifact not yet confirmed. The previous shared-helper source SHA `213a077` passed XY-C1 `44631039226` (Build, TRX verification and artifact upload).
- Previous service-lock fixes: e736 MiniUPnP EINTR cap; 213 shared eight-retry EINTR policy across MiniUPnP/Inadyn/DNSMasq/VPN release; d57 full one-second acquire wait across EINTR for Inadyn/DNSMasq/VPN.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; archival only. PR #1 remains closed/unmerged.
- No physical-router result is available. Host Actions are L2; XY-C1 Build/TRX/artifact on one exact production SHA is L3; device behavior is L4 NOT VERIFIED.

## Active task

- Confirmed a remaining signal-storm reliability gap in shared `service_wait_policy.h`: `service_wait_seconds_with()` retried the unslept integer remainder forever, and `service_wait_timespec_with()` retried every nanosleep EINTR without a limit. Repeated immediate signals could keep lifecycle/startup callers trapped inside a wait.
- This slice adds a common eight-consecutive-EINTR ceiling to both wrappers. Normal remainder retries and invalid/error handling are preserved; exhaustion returns `-1/EINTR` so callers fail closed instead of hanging.
- The GNU89 wait-policy regression adds repeated-EINTR exhaustion cases for both `sleep()` and `nanosleep()`. Existing Process workflow compiles this test under `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined`; XY-C1 already watches the shared policy header.
- No local compilation or physical-router result is claimed for this source slice.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Process lifecycle, service-wait regression, AI-doc, NVRAM and XY-C1 Actions for this change.
2. On FAIL, inspect job logs and correct the demonstrated issue while retaining both eight-interruption ceilings and exact remaining-time handling. XY-C1 L3 requires Build, TRX verification and artifact upload all SUCCESS on the same source SHA.
3. Continue another independently evidenced cross-userland stability/security issue; do not reopen the fixed MiniUPnP/service lock EINTR, acquisition wait, or procfs read loops without new evidence.

## Verification boundary

- L1: shared seconds/nanosleep wait loops now fail with EINTR after eight consecutive interrupted calls; the remaining-time contract is preserved before exhaustion.
- L2: GNU89 Process wait-policy regression adds bounded storm cases; exact-head Process/AI/NVRAM Actions are pending.
- L3: XY-C1 already watches `service_wait_policy.h`; same-SHA Build/TRX/artifact remains pending.
- L4: target signal timing and physical-router service behavior remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, product-source baseline, exact-head Actions, module state and this first action. Update this file only when resumable facts materially change; do not create no-op SHA-refresh commits.

## Development invariants

- Main-only; never modify the frozen backup or develop on a legacy branch/PR.
- One complete Git tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken regressions.
- Report exact SHA Actions and keep L1/L2/L3/L4 distinct. Host success is not firmware/hardware success.
