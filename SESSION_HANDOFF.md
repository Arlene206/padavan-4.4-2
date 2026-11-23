### 2026-11-23 接力更新（当前实时检查点）

- live `main`：`02f174a93bf275930cede271d233dcf2c57438fa`；最近一个修改 raeth 源码的提交：`02f174a93bf275930cede271d233dcf2c57438fa`（main 是否为纯 workflow/docs 后续提交以 Git 历史为准）。
- 已完成：FE VBG/R50/TX-offset/TX-amplitude 及 MDIX checked MDIO/wait、bounded search/fallback/cleanup；GE R50/TX-offset/TX-amplitude analog 检查；FE/GE orchestrator 和 EPHY batch/init 错误传播；校准 ioctl 重试耗尽返回 `-EIO`，且 `ei_ioctl()` 不再丢弃 `ephy_ioctl()` 返回值。
- GE all-analog retry bounded=NOT CONFIRMED。下一位 GPT 应以 live 源码核验，不要只相信本记录。
- 源码 SHA `02f174a93bf275930cede271d233dcf2c57438fa` 的 exact push Actions：
- AI handoff documentation contract #45285734852: in_progress/PENDING
- Switch CLI ioctl failure-status regression #45285734872: in_progress/PENDING
- XY-C1 Firmware Validation #45285734450: in_progress/PENDING; target=Build XY-C1=pending,Verify TRX output=pending,Run actions/upload-artifact@cf430e030ddbb5b0abf93d22962f4752f3646cd9=pending
- NVRAM transaction boundary audit #45285734861: in_progress/PENDING。
- 同一 SHA 的 XY-C1 Build + TRX + artifact 三项全部 PASS：NO / NOT VERIFIED。host source contracts 不等于 MIPS/kernel target build；无实体设备硬件 L4 声明。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 当前 SHA：`72f4f157a084f8184b9597ca5bf334b8080e5807`；本轮未修改。
- 下一项：确认最新 exact-SHA Actions 和 GE retry bounded 实际源码；修完任何 contract/target FAIL 后，转向 raeth 以外的 Padavan userland 稳定性缺陷（优先 rc/NVRAM/httpd），仍按源码—测试—原子提交—SHA Actions 工作。

## Active task

### EPHY calibration failure propagation and bounded retries

- FE/GE analog production paths use checked paged MDIO and checked wait status; failure suppresses completion flags and cleanup retains the first transport error.
- EPHY batch/init now propagate FE/GE calibration status. Calibration ioctl retries are bounded and report `-EIO`; `ei_ioctl()` passes the result to the netdev caller.
- Workflow contracts were adjusted to use the current production function signatures and stable source block boundaries. Verify these on the current source SHA, not historical PASS runs.

## Verification boundary
- Source verification SHA: `02f174a93bf275930cede271d233dcf2c57438fa`; Actions listed above are its exact push runs.
- Current main tree: `02f174a93bf275930cede271d233dcf2c57438fa`. When main is a later workflow/docs-only commit, do not call the source parent’s Actions a PASS for the newer tree.
- XY-C1 L3 is NOT VERIFIED; no physical hardware L4 claimed.

## Exact first action for the next GPT

1. Fetch live `main`; check exact SHA Actions for `02f174a93bf275930cede271d233dcf2c57438fa` and current tree `02f174a93bf275930cede271d233dcf2c57438fa`, reading failing Switch CLI job logs.
2. Confirm GE orchestrator retry is bounded to three attempts and that all init/ioctl callers propagate the returned error.
3. Continue with one independent userland stability/security defect after the EPHY source slice is verified; add a regression contract, commit atomically and check same-SHA Actions.

---
# Cross-session execution mandate

## Current repository state

### 2026-11-24 continuation — RAETH PHY reliability

- FE all-analog teardown gates on first attempted setup write; early snapshot errors skip hardware teardown.
- FE/GE calibration uses checked MDIO, bounded search, error propagation and best-effort cleanup.
- EPHY ioctl propagates calibration status to ndo_do_ioctl. THIS commit releases page_lock during long synchronous PHY calibration and reacquires it before the shared exit.

## Active task

### Validate EPHY IRQ-state contract and exact-source build, then continue independent stability work

- Check exact-head Switch CLI and AI handoff Actions for THIS commit.
- Confirm EPHY ioctl unlocks page_lock around calibration and relocks before return.
- Check XY-C1 Build/TRX/artifact on this exact source SHA; L3 requires all PASS on identical SHA.
- Continue a distinct confirmed stability/security defect in RAETH/userland.

## Verification boundary

- Parent source: d855df7965ca1a0e393cb9af420491eccff6d089. THIS commit changes raether.c, Switch CLI workflow and continuity records.
- Parent exact Actions: Switch CLI #45285332081 PASS; AI handoff #45285331846 PASS.
- No local cross-build, same-SHA full target artifact or hardware verification is claimed.

## Exact first action for the next GPT

1. Fetch live main and exact-head Switch CLI/AI handoff push runs.
2. Verify the EPHY lock-state contract and XY-C1 Build/TRX/artifact on the new source SHA.
3. Continue the next confirmed stability defect after checks pass.

---
## Cross-session operating constraints
Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on `main`; each logical source or CI change is one atomic main commit: create all changed blobs and one complete Git tree before creating that commit, then update `main` using a non-forced expected-SHA compare-and-swap. Do not ask the user what to do next.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop
- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.

