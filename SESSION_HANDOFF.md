# 2026-11-19 continuation checkpoint — WAN final-return contract

## Current repository state

- Repository `Arlene206/padavan-4.4-2`, active branch `main`.
- Last committed slice `4f2ae0b46f16b60230888d6422fc9c3c505b90ef` hardens MT7621 switch CLI invalid usage and truncated ACL commands. Fetch live main before every write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never touch. PR #1 remains closed/unmerged.
- WAN chronological first-error production work includes USB NDIS/RAS, callback-link setup, VLAN and APCLI/WISP preflight failures. Checker count now correctly expects 21 VLAN-result aggregates; a follow-up checker order assertion is the current demonstrated CI failure.

## Active task

On parent `cc3f3d9d3c9261e394f87a20c93dd9874f870c79`, USB modem `44445491851`, Firewall `44445491599` and Process lifecycle `44445491797` all progressed beyond the 21-count assertion but failed at checker line 380:
`wan_start.index("set_passthrough_pppoe(1)") < wan_start.index("return wan_start_status;")`.

The three newly fixed fail-fast preflights legitimately contain earlier `return wan_start_status;` statements. The final return must be compared with the last occurrence, as this same checker already does elsewhere at line ~247. This follow-up changes only that static check to `rindex()`; no production source changes.

## Verification boundary

- L1: WAN first-error runtime change reviewed; callback errno is captured before logging and the VLAN/APCLI failures merge before rollback and return.
- L2: WAN checker exact-parent Actions all failed only at the above premature first-return comparison. Exact-head correction checks pending.
- Switch CLI source fix commit `4f2ae0b46f16b60230888d6422fc9c3c505b90ef`: Switch CLI, AI docs, Process inventory, NVRAM boundary Actions pending at last query.
- L3: XY-C1 Build/TRX/artifact still require exact same-SHA inspection.
- L4: real router WAN/modem and switch/PHY/MDIO behavior NOT VERIFIED; no local C test or MIPS build in this connector-only session.

## Exact first action for the next GPT

1. Fetch live main, backup and all exact-head run results for latest commits.
2. Verify the WAN checker now finds the final return; inspect any subsequent source assertion rather than loosening it. Re-run USB modem, Firewall, Process lifecycle and WAN route where applicable.
3. Inspect the exact `4f2ae0b` Switch CLI source contract and its Action result; this source change must prove argc guard precedes argv[3] and usage exits nonzero.
4. Confirm XY-C1 Build, TRX and artifact upload on one exact SHA before claiming L3.
5. Continue an independent P1 reliability/security defect in Padavan userland after current FAIL items close.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Create all blobs and one complete tree, then one commit with the exact live main parent; update main via non-forced expected-SHA compare-and-swap. Continue across chats without waiting on unrelated pending jobs. Backup remains immutable.

## Before context exhaustion

Record exact source SHA, tests, Action conclusions and unverified boundaries before ending; never claim tests, builds, TRX, artifact upload or hardware verification that did not occur.
