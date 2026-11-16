# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and complete an accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent for THIS LAN definition and DNSMasq/Process contract correction: `be64a1990828e92895b4ce507216c509e1b35ff3`. Refetch live main before every further write and use expected-SHA non-forced compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Startup governance/archive/PR recovery are complete; do not repeat.
- Root `dev_log.md` has 96 newest-first records before THIS entry; it will contain 97 afterward. Maintain 20–119 records and preserve archive shards/indexes.

## Active task

Fix the real compile-blocking return-type regression introduced while converting LAN helpers to status-returning APIs:
- `start_lan()` had `void\nint\nstart_lan(...)`; `lan_up_manual()` had `void\nlan_up_manual(...)` despite returning error values and having int prototypes. THIS commit removes the stale void tokens.
- The Process workflow now asserts the production definitions are `int`, not only that their bodies and caller status merges look correct.
- DNSMasq lifecycle's manual resolver lock-failure contract now expects `return saved_errno;` from the status-returning helper rather than a bare void return.
- The XY-C1 driver source checker was independently corrected by commit `be64a1990828e92895b4ce507216c509e1b35ff3` to validate exact NDIS/RAS driver arrays, aligned params and verified module-family loading.

## Verification boundary

Exact-head observations on parent `be64a1990828e92895b4ce507216c509e1b35ff3`:

- Process inventory/lifecycle regression `43849012514`: SUCCESS, including the LAN source/caller contract from the previous slice. THIS correction adds definition-level return-type assertions.
- AI handoff documentation `43849012491`, infosvr `43849012433`, AdGuardHome `43849012473`, AliDDNS `43849012496`, AliyunDrive `43849012441`: SUCCESS.
- XY-C1 firmware run `43849012639`: IN_PROGRESS at `Build XY-C1`; it had passed the corrected `Verify XY-C1 USB modem driver configuration` step and all preceding policy regressions. This run is against parent `be64a1990828e92895b4ce507216c509e1b35ff3`, which still has the LAN return-prefix bug fixed by THIS commit; no TRX/artifact result yet.
- NVRAM transaction audit `43849012584`: IN_PROGRESS at raw userland ioctl/kernel NVRAM API audit after lock, permission, contention and crash-release regressions passed.
- On earlier parent `5c74753c5af2951f477f733b1f3d6e1d78dc1824`, DNSMasq run `43848474699` failed at embedded Python line 107 because the contract expected `return;`; production now returns `saved_errno`. THIS commit updates the source assertion; exact-head DNSMasq rerun will be pending.
- The previous parent `5c74753` XY-C1 run `43848474467` failed before build at the stale direct-module-load checker; that checker passed on `be64a1990828e92895b4ce507216c509e1b35ff3` run `43849012639`.
- No confirmed current-source Build+TRX+artifact L3 result or physical-router/L4 result is established yet.

## Exact first action for the next GPT

1. Inspect this commit's exact-head Process, DNSMasq and XY-C1 workflows. The XY run on parent `be64a1990828e92895b4ce507216c509e1b35ff3` was already at Build; also require its TRX verification and artifact upload to succeed before recording L3. Confirm target compilation after the LAN definition fix.
2. Audit `config_bridge()` / bridge setup result APIs carefully. `br_set_param_int()`, `brport_set_m2u()`, `phy_igmp_static_port()` and `phy_igmp_snooping()` return status, but unsupported multicast sysfs attributes or board-specific switch ioctls may be expected. Do not convert optional feature absence into boot-fatal errors without capability-specific semantics.
3. Continue wider RC/service-start result propagation beyond this completed LAN start slice; separately verify selected-device driver binding, since module-family presence alone does not prove the chosen USB device bound to the driver.

## Governance constraints

Preserve security, architecture, compatibility, target toolchain and mandatory tests. Main only; frozen backup read-only. One complete Git tree and one atomic commit per logical task, non-forced expected-SHA update. Update `dev_log.md` for source/documentation changes; keep 20–119 records and preserve archives/indexes verbatim. Distinguish host tests, source contracts, exact-SHA CI, firmware/TRX and physical-router evidence. Never claim an unrun result.
