### 2026-11-23 接力更新（最新工程检查点）

- 当前 main 基线：`7696612da682e35f95219c14e0a79f59b5a85ecb`。GE orchestrator 最近提交 SHA 为 `7696612`；其 exact-head Switch CLI、AI docs、NVRAM PASS，XY-C1 Firmware Validation #45278137921 仍 in_progress，不能宣称 L3。
- 当前源码切片：FE R50 common page-7/reg24、FE local page3/4、comparator 及两处 FE calibration wait 全部改为 checked API；MDIO transport 错误不会再当成零值继续。
- 新增兼容性保留的 checked Leopard/MT7622 output setters。MT7622 per-port subtraction 前检查 offset、table index 和 port，修复潜在 unsigned underflow / 64-entry table 越界。
- 普通 not-ready/saturation 仍使用 0x20 fallback；254 次边界穷尽也退回 fallback，而非发布未经确认 code。所有退出都尝试 common analog clear 与 port/common page3 reset；只有输出写和 cleanup 全部成功才置 `fe_cal_r50_flag`。
- 新增 Switch CLI source contract；无本地 MIPS build、无本次 SHA XY-C1 Build/TRX/artifact L3、无硬件 L4 声明。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 冻结不变。
- 下个 GPT 第一项：检查本次 FE R50 exact-head Actions；然后审查 `fe_cal_vbg()` 与 FE top-level analog orchestration 的剩余 unchecked MDIO/waits。

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

### FE R50 checked setup, search, result programming and cleanup

- FE page-7/reg24 and page3/4 RMW operations use checked status-bearing helpers; both waits and comparator reads abort on MDIO failure.
- The Leopard/MT7622 setters retain their legacy void entry points, while FE calibration calls checked internal helpers and checks table bounds before MT7622 offset subtraction/indexing.
- Timeout, saturation and bounded search exhaustion use the 0x20 fallback; transport and result-write failures do not publish completion. All exits attempt common analog clear and local page3 resets.
- Next bounded candidate: inspect `fe_cal_vbg()` for checked I/O and guaranteed cleanup.


## Verification boundary
- GE orchestrator SHA `7696612`: Switch CLI, AI docs and NVRAM PASS; XY-C1 run #45278137921 was still in progress at last check.
- Current FE R50 source/workflow contract changes are new and require their own exact-head Actions.
- No local kernel/MIPS build, same-SHA XY-C1 Build+TRX+artifact L3 or hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main`; inspect this FE R50 SHA's exact push Actions for Switch CLI, AI docs, NVRAM and XY-C1 Build/TRX/artifact outcomes.
2. Diagnose exact-head failures from their logs; queued or skipped target work is not L3.
3. Continue with `fe_cal_vbg()` as a separate checked-I/O/cleanup slice.


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