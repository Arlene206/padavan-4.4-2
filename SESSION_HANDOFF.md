# Cross-session execution mandate

Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with SESSION_HANDOFF.md before ending or handing off. Verify source baseline, active task, exact-head Actions, validation boundary, blockers and precise first action. If the checkpoint remains accurate, do not make a documentation-only commit.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

Before context exhaustion, finish the current safe slice, check exact-head Actions and preserve the precise first next action here.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent main for the modem WAN reconnect status slice: ea794e3ef680da5b03ca0ffdd621601c97dc7d86.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only. Startup recovery is complete; do not repeat.
- PPP IPCP address/netmask validation, WAN IPv6 teardown identity preservation and DHCPv6 generation fencing remain required. Optional WAN MTU ioctl errors remain diagnostic-only.

## Active task

USB modem hotplug lost WAN reconnect failures. The public helper try_start_usb_modem_to_wan() was void and discarded try_wan_reconnect()'s return. The deferred hotplug handler cleared usb_unplug_md and usb_hotplug_md before running the reconnect, then never restored the event flags or scheduled retry on failure. The helper now returns int (0 for existing no-op eligibility cases, otherwise the reconnect result); on modem-unplug reconnect failure the handler re-arms usb_unplug_md and alarm(5), and on modem-plug startup failure it re-arms usb_hotplug_md and alarm(5). The USB modem workflow now checks the helper-to-caller error path and tracks services_stor.c/services_usb.c in both event filters.

The previous exact-head storage run 43882865182 on ea794e3 passed all 24 steps, including the corrected stop-status safety checker. The AI documentation run 43882864787 failed because the current handoff omitted the exact checkpoint sentence required by scripts/check_ai_handoff_docs.py. This handoff restores that required phrase.

## Verification boundary

On parent ea794e3ef680da5b03ca0ffdd621601c97dc7d86:
- USB storage hotplug 43882865182: PASS, all 24 steps, including ash/C regressions, complete caller graph, checked storage eject and firmware persistence gates.
- AI handoff docs 43882864787: FAIL, step 4, exact message: current handoff does not require an end-of-session accuracy checkpoint. Source checker requires both "Mandatory handoff checkpoint" and "The checkpoint is mandatory; a document edit is conditional."; this handoff now contains both.
- AdGuardHome 43882869197, infosvr 43882865220, AliyunDrive 43882864990, process inventory/lifecycle 43882864867, NVRAM transaction 43882864826, AliDDNS signed API 43882864791: PASS.
- Firmware product-source L3 is verified on cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d: same-SHA XY-C1 run 43880814725 passed Build XY-C1, TRX verification and artifact upload. ea794e3 changed only CI checker and docs, not firmware products. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head USB modem identity and XY-C1 Actions; repair any source-contract/build regression on the exact SHA.
2. Check the AI handoff docs workflow result after restoring the required checkpoint wording.
3. Continue P1 RC startup-result propagation through independent userland service paths; preserve process identity, generation fences and lifecycle locks.
4. Keep physical-router L4 NOT VERIFIED until actual router evidence exists.
