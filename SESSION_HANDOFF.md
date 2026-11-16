# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS essential bridge/VLAN initialization status slice: `9f71cebf2e323d8dc84bca7cfd312d027ca5aab1`; refetch live main before any later write and use expected-SHA non-forced CAS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. The one-time backup comparison found main 588 commits ahead / 1 behind; do not repeat archive/backup recovery.
- Root `dev_log.md` has 102 newest-first records before THIS entry; this entry makes 103. Keep 20–119 and preserve archive shards/indexes verbatim.
- Startup governance and cross-session protocol repairs are complete. No open PR was found at the last startup checkpoint.

## Active task completed in this slice

Essential LAN bridge initialization now reports checked setup failures without short-circuiting independent operations:

- `init_bridge()` returns first-error status and records switch VLAN/bridge mode, MAC interface address/IFUP, switch PHY power, configured VLAN creation, bridge creation/STP/forward delay/address, bridge member attachment, and final bridge IFUP.
- Duplicate bridge-add `EEXIST` is accepted only if the kernel confirms the named interface is an existing bridge through `br_bridge_is_empty()`. Bridge member attachment already uses fail-closed full inventory and idempotent membership checks; existing shared policy tests are now run by the Process workflow.
- `switch_config_vlan()` attempts all VLAN rule writes and bridge-mode update while preserving the first failure. Optional iNIC isolation reset is separately logged and is not reclassified as an essential boot failure.
- `init_router()` merges bridge initialization status into PID 1 boot health. Switch config/VLAN notification handlers now preserve status results. Optional multicast configuration remains separate in `config_bridge()`.
- Added GNU89/Werror/ASan/UBSan policy regression coverage and Process source contracts. The standalone bridge-init policy test passed locally before publication. No full firmware build or router hardware test is claimed.

## Verification boundary

- Source/test parent before this commit: `9f71cebf2e323d8dc84bca7cfd312d027ca5aab1`.
- On parent `1f232209a9aa41d520939c18ab783609e9b791b0`, Process run `43852281303` failed only its loopback function-definition assertion after preceding policy tests passed. Commit `9f71cebf2e323d8dc84bca7cfd312d027ca5aab1` corrected that source contract; an exact-head Process rerun for the bridge change is pending publication.
- XY-C1 `43852281447`, USB storage `43852281602`, and NVRAM `43852281469` were IN_PROGRESS at the previous handoff inspection on an ancestor SHA; their current exact status must be checked, not assumed.
- Local host test: GNU89/Werror/ASan/UBSan bridge-init policy regression PASS.
- No exact-head CI result for THIS commit yet. No same-SHA Build+TRX+artifact L3 and no physical-router L4 result established.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA Process, XY-C1, USB storage and NVRAM Actions for THIS change. Resolve the first causal failure before treating downstream evidence as valid.
2. Record XY-C1 L3 only when Build, TRX verification and artifact upload all pass for the same production-source SHA; hardware behavior remains L4-only.
3. Continue evidence-based RC startup result coverage, particularly the currently void `switch_config_base()`, `switch_config_storm()`, and `switch_config_link()` call graphs. Distinguish mandatory Ethernet forwarding setup from board-/feature-specific optional capabilities.
4. Update this handoff with exact run IDs/results at the next material source commit or before context exhaustion.

## Governance constraints

Preserve security, architecture, compatibility, target toolchain and mandatory tests. Main only; frozen backup read-only. One complete Git tree and one atomic commit per logical task, non-forced expected-SHA update. Update `dev_log.md` for each source/documentation change and keep 20–119 records. Distinguish host tests, source contracts, exact-SHA CI, firmware/TRX and physical-router evidence. Never claim an unrun result.
