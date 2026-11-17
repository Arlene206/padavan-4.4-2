# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Current exact parent for the PPP source-contract repair is `5327f7626fc307c8db5e2de670e5433cd0eef55b`.
- Production commit `5327f762` migrated the PPP peer probe in `wait_ppp_up()` and custom WAN up/down hooks to argv-based `eval()`; the command/source change itself had no compiler or runtime failure in the affected workflows.
- Exact-head results on `5327f762`: WAN DHCP `43990883611` PASS; Firewall `43990883471` FAIL; WAN route `43990883432` FAIL; Process inventory `43990883425` FAIL; XY-C1 validation `43990883486` FAIL; NVRAM `43990883715` was still running at last poll. The same stale PPP source contract caused the Firewall, Process and XY-C1 failures: `scripts/check_modem_wan_start_status.py:203` still searched for `doSystem("ping -c1 %s", ppp_gate)` after the C source had changed to `eval("ping", "-c1", ppp_gate)`. WAN route had the same stale marker in its embedded checker at Python line 300.
- This follow-up updates both checkers to require the exact argv probe and reject the former shell-formatted command. No production C change is needed for these CI failures.
- The WAN IPv6 SIT/6RD/native route and per-interface IPv6 cleanup migrations remain argv-based; `net_wan6.c` contains no `doSystem()`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Rerun the exact-head Firewall, WAN route, Process inventory and XY-C1 workflows after the PPP checker correction. Inspect any newly exposed failures rather than assuming the shared checker was their only possible failure. Resume PPP L2TP/mwan command audit when the regressions clear.

## Verification boundary

- The failing jobs reached the final source-contract assertions after preceding host regressions passed. The current failure is deterministic test/checker drift, not an observed PPP runtime regression.
- Product-source MIPS/uClibc L3 remains **NOT VERIFIED**; the XY-C1 job stopped in a host regression before the firmware build. Same-SHA Build, TRX verification and artifact upload are still required. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch exact-head results for Firewall, WAN route, Process, XY-C1 and NVRAM after this checker-only follow-up. Resolve any remaining cause using the job log.
2. If those regressions clear, inspect remaining `net_ppp.c` external command dispatches and L2TP/mwan lifecycle semantics; use argv only for verified dynamic operands and preserve callback fencing.
3. Confirm a same-SHA XY-C1 Build, TRX verification and artifact upload. Do not change the frozen backup; use non-forced expected-SHA main updates.
