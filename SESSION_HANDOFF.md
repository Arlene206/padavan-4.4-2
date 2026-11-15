# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at the start of this regression follow-up: `524601f0e86e885f2858f1a5a7b571c09b8d1e1c`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Production slice `524601f0e86e885f2858f1a5a7b571c09b8d1e1c` now checks static MAN `ifconfig()`, refuses route publication when interface setup fails, returns `EINVAL` for LAN-subnet conflict and aborts non-PPPoE PPTP/L2TP on failed physical WAN setup. PPPoE async/no-wait and USB modem paths are unchanged.
- Exact 524 WAN callback #43751765114 FAIL was caused only by the pre-existing source-contract assertion still requiring the old literal `if (wait_dhcpc)` after the source intentionally changed it to `if (wait_dhcpc && launch_status == 0)`. All preceding C regression tests and the newly added static/error abort checks passed. This follow-up updates that assertion; no production code changes.
- Exact 524 AI docs #43751765238 PASS; Process inventory/lifecycle #43751765307 and the Process lifecycle inventory #43751765170 were still pending at last poll. Exact XY-C1 #43751765026 was still pending at last poll; verify Build, TRX and artifact.
- Earlier exact XY-C1 #43750573567 for source SHA `8ee190c` completed successfully, but it predates the static WAN source change in 524 and does not validate it.
- Frozen backup and archived files remain untouched. No physical-router/L4 result is claimed.

## Active task

### Align WAN regression contract with successful-launch-only wait delay

A regression test still asserted the old unconditional `if (wait_dhcpc)` guard after the source now sleeps only when the DHCP client command successfully starts. Update the test assertion to match the new condition, keeping the required `sleep(2)` check and all other source-contract assertions intact. This is test-only; do not roll back the production guard.

## Verification boundary

- This test-only commit is parented to `524601f0e86e885f2858f1a5a7b571c09b8d1e1c`.
- Exact 524 WAN callback #43751765114 failed at the stale delay assertion; prior C host regressions, static `ifconfig()` ordering test and non-PPPoE abort/restore assertions passed.
- Exact 524 AI handoff docs #43751765238 PASS. Process and XY-C1 jobs remained pending at the last poll; no new L3 claim is made.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Check the exact-head WAN callback, AI docs and Process runs for this correction; inspect the assertion log if WAN fails again.
2. Check XY-C1 Build, TRX verification and artifact upload for product source SHA 524601f; do not reuse older firmware results.
3. Continue WAN startup stability work only after this source slice is validated, preserving PPPoE asynchronous behavior.
4. Keep the exact checkpoint sentence above and do not create no-op documentation updates.
