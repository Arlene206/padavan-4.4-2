# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS static LAN route restart-status slice: `43d0ebe973f06f578282531eed4abc0fdba1bb3b`; fetch live main before the next write and use expected-SHA non-forced CAS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. One-time startup comparison found main 588 commits ahead / 1 behind; do not repeat archive recovery.

## Implemented source baseline

- Essential bridge/VLAN initialization, switch base/storm/link results, required iNIC module post-load verification and optional hw_nat offload status are propagated into PID1.
- `full_restart_wan()` fails closed if required switch VLAN reconfiguration fails before WAN startup.
- Shared network utility bridge-create and bridge-member ioctl helpers preserve the original ioctl errno across subsequent socket close; regression source assertions enforce save-before-close/restore-on-failure ordering.

## Active task

Call-site audit of `full_restart_wan()` found another ignored int-returning operation: `add_static_lan_routes(IFNAME_BR)`. After static routes are cleared as part of WAN teardown, the function must restore required static LAN routes before starting WAN. It now captures the status, logs and returns before `start_wan()` if route restoration fails. Process source contract verifies the required route result and failure-before-WAN ordering.

## Verification boundary

- On product-source parent `90ac78ccccb7acbf6ab75956999b4847a3628fd8`, Process regression `43860760675` failed only its final source contract due to an overbroad LAN-member scan; all prior process inventory, GNU89/Werror/ASan/UBSan and policy regression steps passed. Commit `43d0ebe973f06f578282531eed4abc0fdba1bb3b` repairs the checker by requiring exactly two full-line LAN member calls, each assigned to `operation_status`.
- Exact-head rerun on `43d0ebe973f06f578282531eed4abc0fdba1bb3b`: Process `43860996821` and AI handoff docs `43860997587` were IN_PROGRESS at last inspection. AdGuardHome `43860996728`, AliyunDrive `43860996644`, infosvr `43860996692` had completed SUCCESS.
- On earlier source SHA `7ce997801add3b30e42cd1cc76f14ddc0c45ef5c`, Process `43860044486` and AI docs `43860044408` passed; NVRAM `43860044318` subsequently passed. XY-C1 `43860044821` remained IN_PROGRESS on that older SHA, with Build incomplete and TRX/artifact not started. Do not infer a firmware L3 result.
- THIS route-restoration slice requires new exact-head Process and relevant checks. No same-source-SHA Build+TRX+artifact L3 or physical-router L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head Process and AI handoff docs after this source commit; fix any actual source-contract or test failure.
2. Continue scanning `full_restart_wan()` for ignored status-bearing operations, and trace their definitions/prototypes before changing them. Void best-effort cleanup helpers remain distinct.
3. Verify the shared bridge ioctl errno contracts on latest CI.
4. XY-C1 L3 requires Build, TRX verification and artifact upload to pass on the same production-source SHA; physical hardware remains L4 only.

## Verification and governance

Preserve security, architecture, compatibility, target toolchain and mandatory tests. Main only; frozen backup read-only. One full Git tree and atomic commit per logical task; non-forced expected-SHA update. Update `dev_log.md` for source changes and retain 20–119 records. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
