# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at the start of this regression follow-up: `8ee190c1872c4a3a9b6ad98b5da42fe1f2764e68`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- The direct IPoE DHCP launcher fix is committed at `8ee190c1872c4a3a9b6ad98b5da42fe1f2764e68`: `start_wan()` captures `start_udhcpc_wan()`, logs a nonzero spawn status, persists `wan_err = 1`, and still attempts independent IPv6.
- Exact 8ee WAN DHCP callback #43750573557 FAIL occurred only in the new source-contract ordering assertion: it used `.index(set_wan_unit_value_int(...))`, which matched the earlier static-IP `wan_err` persistence instead of the later DHCP-branch persistence. The fix changes this assertion to `.rindex(...)`; production source is unchanged.
- Exact 8ee AI handoff docs #43750573605 PASS and Process lifecycle #43750573434 PASS. Exact XY-C1 #43750573567 was `in_progress` at last poll; verify Build, TRX and artifact before claiming L3.
- Previous source SHA `485d7c782c57b3fc0cb9cf4e42dcf0a79ba6665a` had WAN callback #43749781222 PASS, AI docs #43749781375 PASS and Process inventory #43749781215 PASS; its XY-C1 job was Build in progress at last recorded poll.
- Frozen backup and archived logs/index remain untouched. No physical-router/L4 result is claimed.

## Active task

### Correct the direct IPoE DHCP error-state regression contract

The source fix at `8ee190c` is intentional and independent of IPv6: direct IPoE DHCP now captures the client launch result, sets/logs WAN error on nonzero status, persists the error and continues IPv6 setup. The host workflow failed because its new ordering assertion used the first `wan_err` persistence point, which is in the static-IP branch. Change only that assertion to compare the DHCP failure condition with the last persistence point in the function. Do not weaken the state/log/order checks or change production code to fit a faulty test.

## Verification boundary

- This regression-only follow-up is parented to `8ee190c1872c4a3a9b6ad98b5da42fe1f2764e68`.
- Exact 8ee WAN callback #43750573557 failed at the single stale `.index()` assertion; all six C compile/run policy steps before the Python contract passed.
- Exact 8ee AI handoff docs #43750573605 PASS and Process inventory #43750573434 PASS.
- Exact 8ee XY-C1 #43750573567 was `in_progress` at last poll; no L3 claim until Build, TRX and artifact all PASS on the corresponding product-source SHA.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router/L4 result is claimed.

## Exact first action for the next GPT

1. Fetch the new main SHA and inspect exact-head WAN callback / AI docs / Process results; if WAN fails again, inspect the exact assertion before changing source.
2. Check XY-C1 #43750573567 through Build, TRX verification and artifact upload, and check any newer source-triggered XY run on its exact SHA.
3. Continue WAN startup/error-state work from the next proven source issue. Preserve independent IPv6 and PPPoE asynchronous behavior.
4. Keep the exact mandatory checkpoint phrase intact; do not create no-op documentation commits.
