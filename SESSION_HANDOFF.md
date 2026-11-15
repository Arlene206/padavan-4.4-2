# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current source baseline at the start of this regression follow-up: `7a28a32006f86fe47eca89567ba78a13f51b750b`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Production WAN source commit `524601f0e86e885f2858f1a5a7b571c09b8d1e1c` checks static MAN `ifconfig()` before `man_up()`, returns `EINVAL` for LAN-subnet overlap, avoids a DHCP wait delay after a failed client launch, and aborts required non-PPPoE PPTP/L2TP startup after physical setup failure while restoring WAN/firewall state. PPPoE async/no-wait and USB modem behavior remain unchanged.
- Exact WAN callback #43752014770 FAIL was the second stale checker assertion: it still counted the old message `WAN physical DHCP/ZeroConf client launch incomplete`, while the source intentionally renamed the common diagnostic to `WAN physical interface setup incomplete` after the new static setup failures became reportable. The six preceding GNU89 compile/run policy regressions passed; the successful-launch delay guard and the new static MAN/non-PPPoE abort contracts also passed.
- Exact AI docs #43752014837 PASS and Process lifecycle #43752014758 PASS on the 7a28 regression baseline.
- Exact XY-C1 #43751765026 for source SHA `524601f` was still IN_PROGRESS at the last poll; Build, TRX verification and artifact upload remain unverified.
- Backup, `docs/dev-log/INDEX.md` and archived shards remain untouched. No physical-router/L4 result is claimed.

## Active task

### Align the WAN physical-interface diagnostic regression contract

The physical WAN callers now log `WAN physical interface setup incomplete` for either a dynamic launcher failure or failed static interface configuration. The checker still counted the older DHCP/ZeroConf-only diagnostic, so it failed even though the new source deliberately broadened the message to cover both failure classes.

Update only the expected diagnostic string while preserving the requirement that both affected call sites report the status. The source and the six compiled policy regressions are unchanged in this follow-up.

## Verification boundary

- This test-only follow-up is parented to `7a28a32006f86fe47eca89567ba78a13f51b750b`; product source remains `524601f`.
- Exact WAN #43752014770 FAIL at the obsolete diagnostic count assertion; all six preceding GNU89 compile/run tests and the prior source-contract assertions passed.
- Exact AI docs #43752014837 PASS and Process lifecycle #43752014758 PASS.
- Exact XY-C1 #43751765026 checked out `524601f` and was IN_PROGRESS at the last poll; no L3 claim until Build, TRX verification and artifact upload all PASS.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router/L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head WAN callback / AI docs / Process outcomes for this diagnostic-contract correction; inspect the full failure log before changing production code if WAN still fails.
2. Verify XY-C1 #43751765026 on product source `524601f` through Build, TRX and artifact upload.
3. After the physical-WAN guard is validated, continue route-status review at `man_up_internal()`, then separately audit direct IPoE static-address `ifconfig()` error propagation. Preserve PPPoE asynchronous behavior.
4. Keep this mandatory checkpoint language intact and preserve the frozen backup and archived log index/shards.
