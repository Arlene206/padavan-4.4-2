# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Parent at start of this atomic slice: `4179895832c0d2f3e26b1996f362ed02d2dbd682`.
- `net6.c` now sends runtime interface names to `ip -6 addr/route/neigh flush` as separate argv elements. The old `doSystem("...%s...", ifname)` calls crossed `system()`'s shell boundary; the new calls use the existing `eval()` argv macro.
- Existing input rejection, absent-interface no-op handling and return-status propagation are unchanged. The WAN route workflow now asserts all three exact argv vectors and forbids reverting those wrappers to `doSystem()`.
- The current global IPv6 sysctl status-propagation slice remains in place. Its standalone GNU89/ASan/UBSan policy binary passed locally in this session; the new IPv6 flush source-contract assertions also pass locally.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Continue the IPv6 route/tunnel shell-boundary audit in `net_wan6.c`. Address only commands whose dynamic operands have verified provenance and validated representation; preserve `WAN6_SIT_CHECK` / `WAN6_NATIVE_CHECK` status capture and rollback ordering. Prioritize remaining tunnel creation, 6rd configuration, MTU and route operations. Do not repeat the completed per-interface flush change.

## Verification boundary

- The new WAN route workflow contract was run locally against the edited source and passed. Its exact-head GitHub push run has not yet been independently observed.
- For parent `4179895832c0d2f3e26b1996f362ed02d2dbd682`, combined legacy commit status returned no contexts; the available workflow-run wrapper only exposes PR-triggered runs. This is an observability limitation, not evidence of Actions PASS or FAIL.
- Product-source MIPS/uClibc L3 (XY-C1 build, TRX and artifact on one SHA) and physical-router L4 remain **NOT VERIFIED**.

## Exact first action for the next GPT

1. Read the new live `main` head once and retrieve its exact push Actions through an available authoritative route; resolve any first causal failure without re-auditing already passed parent work.
2. Continue the bounded `net_wan6.c` audit: validate SIT/6rd address and prefix provenance, convert only the remaining safe dynamic shell commands to argv-based `eval()`, and extend the WAN route/DHCP source regressions.
3. Keep the backup branch frozen. Do not claim product-source L3 until XY-C1 Build, TRX verification and artifact upload pass on one exact SHA; physical-router L4 remains NOT VERIFIED.
