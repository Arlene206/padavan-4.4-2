# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- one atomic main commit per logical slice. Create all changed blobs and one complete tree based on live main, parent the commit to the live SHA, and update with force=false plus exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before context exhaustion, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Live main parent for this handoff correction: `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`.
- Production source SHA under test: `10b57237ced5487a0d17e0bb09425ea96973a78e`. The shared `dhcp_lease_nvram_publish()` validates all entries, attempts every candidate write despite an individual failure, preserves the first publication error, and restores the complete previous tuple in reverse order.
- WAN `udhcpc_renew()` publishes changed DNS/domain/WINS/lease duration through one serialized NVRAM transaction; it returns publication errors and withholds resolver refresh unless the snapshot succeeds.
- PPP ip-up reuses the shared policy for `ipaddr/netmask/ifname_t/gateway/dns` under a transaction lock, retries complete tuple restoration under a fresh transaction after write/end failures, and retains interface rollback and WAN route gating.
- The attempt-all injected GNU89 regression passed on source SHA `10b5723`. WAN DHCP callback exact-head run `44689620059` passed on checker/handoff SHA `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`; NVRAM transaction boundary run `44689024044` passed on source SHA `10b5723`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only; branch inventory last verified only `main` and this backup.
- Legacy PR #1 is CLOSED / NOT MERGED; AliDDNS HMAC/CA-bootstrap work is already present and evolved on main. Do not repeat that port audit.

## Active task

1. Poll XY-C1 run `44689620163` for checker/handoff SHA `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`. It was IN_PROGRESS at the last poll; verify detector, MIPS build, TRX and artifact upload on the production-source SHA before claiming L3.
2. AI handoff run `44689620108` failed because this handoff rewrite omitted the required headings `## Verification boundary` and `## Exact first action for the next GPT`; both sections are restored below. Re-run the validator on this correction SHA.
3. The earlier WAN route/XY-C1 source-contract failure was a stale MAN teardown assertion. The MAN implementation correctly uses checked `rc_wan_nvram_set_all_values()`, not the unrelated DHCP lease publisher. The checker has been corrected; verify exact-SHA XY-C1 source checks now pass.
4. Continue the next production task after this snapshot slice: audit `udhcpc_bound()` for individual address-change lease writes that can leave a partial runtime tuple. Implement a fail-closed snapshot/rollback and focused injected regression in a separate atomic source commit.

## Verification boundary

- PASS: WAN DHCP callback run `44689620059` on exact SHA `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`; NVRAM transaction boundary run `44689024044` and injected GNU89/ASan/UBSan lease policy regression on production source SHA `10b57237ced5487a0d17e0bb09425ea96973a78e`.
- FAIL, corrected in this documentation commit: AI handoff validation run `44689620108`, because the handoff lacked the two required headings. The checker explicitly requires `## Verification boundary` and `## Exact first action for the next GPT`.
- PENDING at last poll: XY-C1 run `44689620163` on SHA `b5146272dd0149586fb5bb27d5aa43d85f2bb1fd`.
- The earlier WAN route run `44689024130` and XY-C1 run `44689024255` failed at the stale MAN teardown assertion before this checker repair. The updated MAN contract uses the correct checked writer.
- L3 is NOT VERIFIED until a single production-source SHA passes detector, MIPS build, TRX verification and artifact upload. Physical-router L4 is NOT VERIFIED without hardware evidence.

## Exact first action for the next GPT

Fetch live main and inspect exact-SHA AI handoff, XY-C1, WAN DHCP callback, WAN route and NVRAM boundary Actions. Fix any failures from their job logs without weakening checked-write, attempt-all, full-rollback or route-gating behavior. Once XY-C1 completes, establish whether its run actually executed the production detector/build/TRX/artifact steps on source SHA `10b5723`; host-only or checker-only success is not L3. Then implement and test the `udhcpc_bound()` address-change snapshot as the next independent source task.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. A same-production-SHA detector, MIPS build, TRX verification and artifact upload are required for L3. Never infer L4 from CI.
