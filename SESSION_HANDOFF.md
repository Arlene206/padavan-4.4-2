# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- one atomic main commit per logical slice. Create all changed blobs and one complete tree based on live main, parent the commit to the live SHA, and update with force=false plus exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before context exhaustion, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Parent main SHA for this checker repair: 9cadbbd9c116db4b8c0cc3d4f8e312800575f173.
- Production source SHA under test: `10b57237ced5487a0d17e0bb09425ea96973a78e`. The shared DHCP lease policy attempts every candidate write after errors and restores the complete previous tuple in reverse order.
- WAN `udhcpc_renew()` publishes changed DNS/domain/WINS/lease options through one serialized NVRAM snapshot. PPP ip-up reuses the policy for the five-field runtime tuple and retries complete restoration under a fresh transaction after write/end failures.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Branch inventory last verified only `main` and this backup.
- Legacy PR #1 is CLOSED / NOT MERGED; AliDDNS HMAC/CA-bootstrap work is already present and evolved on main. Do not repeat that port audit.

## Active task

1. The DHCP lease policy GNU89/ASan/UBSan executable passed on source SHA `10b5723`, including attempt-all and full tuple rollback. WAN DHCP callback then failed only because `check_lan_dhcp_deconfig.py` still required the old prefix-only rollback marker; this commit makes the checker require whole-vector restore and the new regression.
2. WAN route and XY-C1 source checks failed because the MAN teardown contract expected the unrelated `dhcp_lease_nvram_publish()` helper. The MAN implementation correctly uses `rc_wan_nvram_set_all_values()` and its checked errno-aware field writer; this commit fixes the validator only.
3. Exact-doc SHA `9cadbbd` AI handoff validation passed. Poll exact-source NVRAM, WAN DHCP, WAN route and XY-C1 Actions after this repair.

## Verification boundary

- PASS: injected DHCP lease policy GNU89/ASan/UBSan executable on source SHA `10b5723`; AI handoff documentation run `44689130545` on documentation SHA `9cadbbd`.
- FAIL, corrected here: WAN DHCP callback `44689024028` (stale LAN prefix-rollback assertion); WAN route `44689024130` and XY-C1 `44689024255` (stale MAN teardown helper assertion).
- NVRAM transaction boundary run `44689024044` was PENDING at checkpoint. XY-C1 did not reach MIPS build, TRX or artifact upload because the shared source checker failed. L3/L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA WAN DHCP callback, WAN route, XY-C1, AI docs and NVRAM boundary Actions triggered by this correction.
2. Inspect any failed logs and correct actual source/contract defects without weakening checked-write, attempt-all, full-rollback or route-gating behavior.
3. Claim L3 only after same-source-SHA detector, MIPS build, TRX verification and artifact upload all pass. Keep physical-router L4 NOT VERIFIED without device evidence.
4. Once this slice is green, audit the address-change `udhcpc_bound()` path for unchecked individual lease writes, add injected regression coverage and commit the next atomic source change.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. A same-SHA detector, MIPS build, TRX verification and artifact upload are required for L3. Never infer L4 from CI.
