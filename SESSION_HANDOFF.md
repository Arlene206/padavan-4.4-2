### 2026-11-23 接力更新（最新工程检查点）

- 当前 main 基线：`103e1ce15aec380b8389b8d18a8ee2e03dc7e9bb`。GE Rext/R50/TX amplitude/TX offset 已完成独立 checked-I/O 切片；`103e1ce` 的 Switch CLI、AI docs PASS，NVRAM 和 XY-C1 Build 当时仍在执行，不能认定 L3。
- 当前源码切片：`do_ge_phy_all_analog_cal()` 的 C22 BMCR 与全部 C45 setup/RX-offset/eye-pic/bias 读写改为 checked API；snapshot 标志确保只有成功获取的 BMCR 与 `1e:145` 才会 restore。
- 编排器执行 R50、TX-offset、TX-amplitude 前清零并检查相应 completion flags；子事务失败停止 RX-offset 和 eye programming，进入 cleanup 恢复 BMCR、BG output、`1e:145` 和 `1f:273`。
- 新增 Switch CLI source contract；无 MIPS/kernel build、无同 SHA XY-C1 L3、无硬件 L4 声明。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 保持冻结。
- 下个 GPT 第一项：检查当前提交 exact-head Actions；随后审查其它 FE analog calibration 函数的 unchecked wait 与 MDIO 读写。

---
# Cross-session execution mandate

## Current repository state

### 2026-11-23 continuation — current state

- **PHY debug:** `ad5bcd2b87b9b672dd2e921b6e133fb35393093c` added checked Clause 45 debug reads and token-ring transfer/page-restore checks; Switch CLI #45270470838 PASS.
- **GE2 link polling:** `bfca47b0c33cae7679ea2e671cc71a015ebfc5d4` publishes only complete C22/CL45 samples and preserves MAC/link state on MDIO error; EEE flags refresh on renegotiation. Checker corrections in `48bc28a` have Switch CLI #45272254777 and AI docs #45272254645 PASS.
- **Legacy PHY wrappers:** status-bearing C22/Clause45 adapters preserve ABI and log failures; Switch CLI #45272997503 and AI docs #45272997319 PASS on `f432a3e`.
- **GE wait:** `87e233d` checks page-7/reg24 trigger setup, bounded polling and final trigger cleanup; Switch CLI #45273294951, AI docs #45273294849, NVRAM #45273294824 PASS; XY-C1 #45273294825 remained in progress at last poll.
- **Analog cleanup RMW:** `51f2958` added checked RMW and retained final register-zero cleanup; Switch CLI #45273676193 PASS. AI docs on that SHA failed only because its dev log had been truncated; full history was restored in docs child `bff3d15`.
- **FE wait:** current slice uses checked global trigger/poll and always attempts both local page-4/reg23 cleanup writes; TX-amp and normal pre-trigger differences are retained. Exact-head checks pending.
- **Backup:** frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- **Verification:** no same-SHA XY-C1 Build+TRX+artifact PASS and no hardware L4 claim.
- **Next:** inspect exact-head CI and continue one calibration RMW/save/restore sequence at a time.


## Active task

### GE all-analog orchestration checked I/O and child-failure propagation

- Top-level C22 BMCR and C45 setup/RX-offset/eye/bias accesses use checked APIs; saved state is restored only if its snapshot succeeded.
- R50, TX-offset and TX-amplitude child completion flags are cleared before invocation and checked after return. Failed child aborts later analog programming.
- All exits attempt BMCR/BG-output/1e:145/1f:273 restoration and preserve the first error.
- Next candidate: audit remaining FE analog functions for checked wait/MDIO status and guaranteed cleanup.


## Verification boundary
- Previous GE TX-offset SHA `103e1ce`: Switch CLI and AI docs PASS; NVRAM and XY-C1 were in progress at last poll.
- Current GE orchestrator source/workflow changes are new and require exact-head Actions.
- No local kernel/MIPS build, same-SHA XY-C1 Build+TRX+artifact L3 or physical hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main`; inspect the GE-orchestrator SHA exact Actions, including Switch CLI, AI docs, NVRAM and XY-C1 Build/TRX/artifact outcomes.
2. Diagnose exact-head FAIL from job logs; do not count skipped/incomplete product steps as L3.
3. Audit FE analog calibration wait, checked I/O and cleanup paths, beginning with `fe_cal_tx_amp()` or `fe_cal_tx_offset()`.


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