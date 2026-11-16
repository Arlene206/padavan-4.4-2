# Cross-session execution mandate

Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and compare it with SESSION_HANDOFF.md before ending or handing off. Verify source baseline, active task, exact-head Actions, validation boundary, blockers and precise first action. If the checkpoint remains accurate, do not make a documentation-only commit.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

Before context exhaustion, finish the current safe slice, check exact-head Actions and preserve the precise first next action here.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Current product-source baseline: be65a873365598d99cccce9513e1e720203b6d8a (USB modem WAN reconnect failure propagation and retry-event repair).
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only. Startup recovery is complete; do not repeat.
- PPP IPCP address/netmask validation, WAN IPv6 teardown identity preservation and DHCPv6 generation fencing remain required. Optional WAN MTU ioctl errors remain diagnostic-only.

## Active task

USB modem hotplug no longer loses WAN reconnect errors. try_start_usb_modem_to_wan() now returns int, returns 0 for existing ineligible/no-op cases, and returns the actual try_wan_reconnect() result for attempted startup. on_deferred_hotplug_dev() now checks modem unplug/plug results, re-arms usb_unplug_md or usb_hotplug_md on failure, and schedules the existing five-second retry.

The USB modem regression workflow checks the helper-to-deferred-caller path and includes services_stor.c/services_usb.c in both push and pull_request filters. The checker includes rc.h to cover the public int prototype.

## Verification boundary

Exact-head Actions for product-source commit be65a873365598d99cccce9513e1e720203b6d8a, at last inspection:
- USB modem WDM/NDIS physical identity stability 43884258738: QUEUED.
- XY-C1 Firmware Validation 43884258941: IN_PROGRESS. Build/TRX/artifact result not yet available.
- USB storage hotplug 43884258495: IN_PROGRESS.
- Process lifecycle inventory 43884258642: IN_PROGRESS.
- NVRAM transaction 43884258482: IN_PROGRESS.
- Other independent services/security workflows (including vsftpd, WAN route, AdGuardHome, AliDDNS, VPN, OpenVPN and several lifecycle suites) were split between QUEUED, IN_PROGRESS and completed PASS at last inspection; consult exact SHA Actions rather than carrying a stale summary.
- The immediately preceding parent ea794e3 storage run 43882865182: PASS all 24 steps, including the full ash/C suites, caller graph, storage eject and persistence gates.
- The immediately preceding parent ea794e3 AI docs run 43882864787: FAIL because this handoff had omitted the required exact phrase "Mandatory handoff checkpoint" and conditional-edit statement. This document now contains the required phrase. Check the new exact-SHA docs run.
- Firmware product-source L3 remains verified on earlier cd5ccbefc26a9f71b0c971800b2f5b0d9cad450d only (same-SHA Build, TRX and artifact upload all passed). Do not extend L3 to be65a873365598d99cccce9513e1e720203b6d8a until its own exact-SHA XY-C1 stages pass. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-SHA USB modem identity, XY-C1, storage, AI handoff docs and process-inventory Actions; repair their first causal failures without weakening contracts.
2. Require same-SHA XY-C1 Build, TRX verification and artifact upload before claiming firmware L3 for be65a873365598d99cccce9513e1e720203b6d8a.
3. Continue the cross-userland P1 startup-result audit after the current modem task passes; trace every caller before altering helper APIs and preserve process identity/lock rules.
4. Keep physical-router L4 NOT VERIFIED until actual router validation evidence exists.
