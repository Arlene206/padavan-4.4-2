# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current regression baseline: `a976ce23dc4ea5dad63ef17565e96bf4d1e3af5b`; product source baseline: `524601f0e86e885f2858f1a5a7b571c09b8d1e1c`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Product source `524601f` gates static MAN route publication on successful interface configuration, returns `EINVAL` for LAN-subnet overlap, skips the two-second wait after failed DHCP spawn, and aborts required non-PPPoE PPTP/L2TP startup after physical WAN setup failure while restoring WAN/firewall state. PPPoE async/no-wait and USB modem behavior remain unchanged.
- Exact XY-C1 #43751765026 for product source `524601f` COMPLETED SUCCESS: Build XY-C1, TRX verification and artifact upload all PASS. This establishes L3 for that product source; it does not establish physical-router/L4 behavior. Later commits up to this baseline changed the WAN checker and docs only, not product code.
- Exact WAN callback #43752539339 FAIL was a checker-only ordering false positive: it compared the restore call with the first `continue;` anywhere in the whole PPP branch, which matched an earlier UPnP/interface-preflight abort. The new physical-WAN failure block itself is ordered correctly. Six preceding GNU89 compile/run regressions and all earlier source assertions passed.
- Exact AI docs #43752539348 PASS and Process lifecycle #43752539327 PASS on the current baseline.
- Frozen backup, `docs/dev-log/INDEX.md` and archived shards remain untouched. No physical-router/L4 result is claimed.

## Active task

### Scope the PPP failure-order regression to the failure block

The PPP branch contains an earlier, independent `continue;` for miniupnpd/interface preflight. The checker searched the whole branch and therefore falsely concluded that the new physical-WAN failure guard did not restore WAN state before continuing. Slice the branch from `abort_guard` first, then assert that `restore_wan_after_ppp_start_failure()` precedes the `continue;` within that scoped failure block. Keep the existing guard and diagnostic assertions.

This follow-up is test-only; the product source already preserves the intended ordering.

## Verification boundary

- Test-only follow-up parent: `a976ce23dc4ea5dad63ef17565e96bf4d1e3af5b`; no product source change.
- Exact WAN #43752539339 FAIL at the broad first-`continue;` ordering assertion. All six GNU89 compile/run tests, the successful-launch wait check, static MAN `ifconfig()` guard and non-PPPoE failure guard checks passed.
- Exact AI docs #43752539348 PASS and Process lifecycle #43752539327 PASS.
- Exact XY-C1 #43751765026 on product source `524601f`: Build, TRX verification and artifact upload PASS (L3 verified for that source SHA).
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head WAN callback, AI docs and Process outcomes for this scoped-order assertion. If WAN fails again, locate the exact failing assertion from the full job log.
2. Continue route status auditing at `man_up_internal()`: static MAN calls currently use the void `man_up()` wrapper, so determine whether route failure must flow to `launch_wanx()` and required PPTP/L2TP gating without duplicate logging.
3. Separately inspect direct IPoE static-address `ifconfig()` return handling; preserve independent IPv6 and PPPoE asynchronous behavior.
4. Keep the mandatory checkpoint sentence intact and preserve frozen backup plus archived log index/shards.
