# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Parent for the PPP/WAN-hook argv slice: `8e09623b275fec23b35952a992e2abcbb98f39fb`.
- `net6.c` per-interface IPv6 flushes and all `net_wan6.c` SIT/6rd tunnel, MTU, route and teardown commands use argv-based `eval()`; `net_wan6.c` has no `doSystem()`. SIT MTU/TTL are bounded and checked before tunnel mutation.
- `net_wan.c` now invokes the PPP peer ping and custom WAN up/down script with argv; the shared checker covers all three and verifies PPP gateway IPv4 validation remains in place.
- Exact parent Actions: WAN DHCP `43990568766` PASS; Process `43990568846`, Firewall `43990568911`, USB modem identity `43990568798`, AdGuardHome `43990568749`, AliyunDrive `43990568662`, infosvr `43990568989`, AI handoff `43990568727`, AliDDNS `43990568732` SUCCESS. XY-C1 `43990568825` and NVRAM `43990568681` were IN_PROGRESS at last poll.
- The new PPP/WAN-hook source contract passed locally; current commit exact-head Actions have not yet been observed. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Check exact-head WAN/XY-C1/NVRAM Actions for the current commit; resolve any first causal failure. Continue the adjacent PPP lifecycle command audit in `net_ppp.c` (especially mwan dispatch and L2TP control), preserving callback fencing and status propagation.

## Verification boundary

- No shell-formatted dynamic interface/IP arguments remain in the audited PPP peer probe, custom WAN event hook, or WAN IPv6 code paths.
- Product-source XY-C1 MIPS/uClibc Build, TRX verification and artifact upload on one exact SHA remain **NOT VERIFIED**; physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch `/actions/runs?head_sha=<main SHA>`; verify current WAN, Process, Firewall and XY-C1 exact-head results. Confirm Build, TRX and artifact upload on the same SHA before reporting L3.
2. Inspect remaining `net_ppp.c` external command dispatches and lifecycle lock semantics; replace only verified dynamic shell operands with argv and add focused source tests.
3. Keep `backup/pre-plugin-overhaul-2026-11-14` frozen and preserve all one-tree, expected-SHA commit rules.
