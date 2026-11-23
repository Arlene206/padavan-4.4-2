### 2026-11-24 最新接力检查点

- 当前父 main SHA：d855df7965ca1a0e393cb9af420491eccff6d089。冻结 backup branch SHA 72f4f157a084f8184b9597ca5bf334b8080e5807 未修改。
- EPHY ioctl IRQ-off 延迟：ei_ioctl() 曾在 ephy_ioctl() 前重新 spin_lock_irq(page_lock)，而该函数同步执行多端口 FE/GE PHY calibration 和有界 MDIO busy polling，导致本地 IRQ 在整个流程内关闭。
- 本次将 page_lock 重新获取移到 ephy_ioctl() 返回之后；校准路径使用专用 checked PHY-page/MDIO locks，不访问 page_lock 保护的 CR register block。copy_from_user 错误路径和 ei_ioctl() 统一出口的锁平衡保持不变。新增 workflow contract 验证 unlock→calibration→relock 顺序。
- Parent exact Actions：Switch CLI #45285332081 PASS；AI handoff #45285331846 PASS。旧源码 SHA 6b1c7c4... 的 XY-C1 build 上次仍 pending；本次新源码需单独验证。
- 未做本地 MIPS build 或硬件实测；无 L3/L4 声明。

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

