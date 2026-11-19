# 2026-11-19 continuation checkpoint — chronological WAN startup error preservation

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Parent `main` before this source slice was `25de09a671c77fb2633f98c4eab34c28cd21bd22`; resolve the child commit from the live `main` ref. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it. PR #1 remains closed/unmerged and is not a development path.
- Latest parent SHA `25de09a671c77fb2633f98c4eab34c28cd21bd22` includes the detect_wan gateway ARP result fix and checker correction. Exact-head WAN gateway ARP `44440477965` and AI handoff documentation `44440477700` are SUCCESS. They do not validate any later source SHA.
- Earlier storage source `1ab298331a6e214224f9ac0675264b221dca0f8b`: HTTPD `44438726006`, Process inventory `44438725935`, USB storage `44438726074`, and NVRAM boundary `44438726126` passed; XY-C1 `44438725933` remained IN_PROGRESS. Follow-up `ff715b1dde53b4eb8c3c6345555196dc717ddc27`: HTTPD `44439771305` passed; NVRAM boundary `44439771163` was cancelled. Do not transfer these results to later SHAs.
- PR #1's old FRP copy behavior is superseded by main's private-permission and symlink-refusal hardening. No local shell test, MIPS build, TRX verification or hardware test was run here. Target BusyBox runtime and physical-router L4 remain NOT VERIFIED.

## Active task

This slice fixes a chronological first-error bug in `start_wan()`. Previously, early WAN failures were recorded in `wan_start_status`, but USB NDIS/RAS failures used a separate `modem_start_status`. The final merge always passed the modem result first, so a later modem failure could mask an earlier NAT/sysctl/interface error.

The fix removes the second accumulator, merges NDIS/RAS failures into `wan_start_status` at their execution points, and returns the shared chronological result. A GNU89 policy regression models an earlier WAN error followed by USB modem and final adjunct failures. Static contracts were updated:
- `scripts/check_modem_wan_start_status.py`
- `scripts/check_wan_firewall_status.py`
- `scripts/check_wan_initial_mtu_status.py`

USB-modem, Process lifecycle, WAN-route and AI handoff workflows already watch these production, test, checker and document paths; no workflow expansion is needed.

## Verification boundary

- L1: PASS — split accumulators could return a later USB modem failure before an earlier WAN failure; source now uses one accumulator.
- L2: PENDING — exact-head USB modem GNU89/ASan/UBSan test plus Process lifecycle and WAN-route source contracts.
- L3: PENDING — `net_wan.c` changed; inspect exact-head XY-C1 Build, TRX verification and artifact upload.
- L4: NOT VERIFIED — physical NDIS/RAS modem bring-up and WAN recovery on router hardware.

## Exact first action for the next GPT

1. Fetch live `main` and resolve the exact commit descended from parent `25de09a671c77fb2633f98c4eab34c28cd21bd22`; verify backup still equals `72f4f157a084f8184b9597ca5bf334b8080e5807`.
2. Inspect exact-SHA USB modem identity, Process lifecycle, WAN-route, AI docs and XY-C1 runs. Read all failing job logs; do not wait on PENDING.
3. Fix demonstrated regressions without weakening contracts. Once this slice passes, select a separate P1 stability issue elsewhere in Padavan userland.
4. At the next material checkpoint, update this handoff with the resolved commit SHA and exact run IDs/results.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and exact-head Actions before ending. Compare source baseline, active task, workflow states, validation boundary, blockers and first action with this handoff. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Create the complete Git tree from the exact parent and update `main` only with a non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Do not ask the user what to do next. Keep the backup immutable.

## Before context exhaustion

Finish the safe slice and commit it atomically. Record exact commit, tests, exact-SHA Actions and unverified boundaries. Leave the next GPT one concrete first action; do not claim local tests, builds, artifacts or hardware checks that were not performed.
