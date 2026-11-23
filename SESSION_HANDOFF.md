### 2026-11-23 最新接力检查点

- 当前源码/测试基线 SHA：`ee0bf75292e0436e8820d3a30cd8b9a28c2bfee1`；提交信息：`fix(raeth): check FE VBG calibration MDIO`；父 SHA：`d0c858d552b211a7f8b8faeaee080fe3f6ce332d`。
- 当前 exact-SHA Actions 快照：NVRAM transaction boundary audit #45279437099: in_progress; XY-C1 Firmware Validation #45279437381: in_progress; AI handoff documentation contract #45279437408: success; Switch CLI ioctl failure-status regression #45279437249: failure
- 当前 exact-SHA check-runs：validate-handoff-records: success; Cross-build XY-C1 firmware: in_progress; switch-cli-contract: failure; nvram-transaction-boundary: in_progress
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 当前 SHA：`72f4f157a084f8184b9597ca5bf334b8080e5807`（保持只读，不更新）。
- 最近源码工作：GE analog orchestrator 的 C22/C45 status propagation + child completion gates；FE R50 的 checked page/C45 search/set/cleanup；MT7622 R50 table index underflow/out-of-range guard 已提取为 policy helper 并加入 host regression。
- 校验边界：host policy tests 和 workflow 源码契约已添加；若 exact-SHA Actions 仍运行/失败，需先按 run/job logs 跟进。没有 MIPS/kernel build、没有同 SHA XY-C1 Build+TRX+artifact L3、没有硬件 L4 的声明。
- 下个 GPT 第一项：按上面 run IDs 刷新 exact-SHA Actions；FAIL 先取失败 job logs 修复，PENDING 继续下列独立任务。随后审查 `fe_cal_vbg()` 及 FE all-analog orchestrator 中剩余 unchecked MDIO/wait/cleanup。

---
# Cross-session execution mandate

## Current repository state

### 2026-11-23 continuation — current state

- **PHY debug:** `ad5bcd2b87b9b672dd2e921b6e133fb35393093c` added checked Clause 45 debug reads and token-ring transfer/page-restore checks; Switch CLI #45270470838 PASS.
- **GE2 link polling:** `bfca47b0c33cae7679ea2e671cc71a015ebfc5d4` publishes only complete C22/CL45 samples and preserves MAC/link state on MDIO error; EEE flags refresh on renegotiation. Checker corrections in `48bc28a` have Switch CLI #45272254777 and AI docs #45272254645 PASS.
- **Legacy PHY wrappers:** status-bearing C22/Clause45 adapters preserve ABI and log failures; Switch CLI #45272997503 and AI docs #45272997319 PASS on `f432a3e`.
- **GE wait:** `87e233d` checks page-7/reg24 trigger setup, bounded polling and final trigger cleanup; Switch CLI #45273294951, AI docs #45273294849, NVRAM #45273294824 PASS; XY-C1 #45273294825 remained in progress at last poll.
- **Analog cleanup RMW:** `51f2958` added checked RMW and retained final register-zero cleanup; Switch CLI #45273676193 PASS. AI docs on that SHA failed only because its dev log had been truncated; full history was restored in docs child `bff3d15`.
- **FE VBG:** C22/page RMW, comparator, wait and output paths now use checked APIs; six-bit stepping/table bounds are host-tested, and every exit attempts shared analog/page-3 cleanup.
- **FE R50:** checked page-7/page-3/page-4 operations and checked output setters retain bounded MT7622 table indexes and best-effort cleanup; exact-head Actions were pending on d0c858d.
- **Backup:** frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- **Verification:** no same-SHA XY-C1 Build+TRX+artifact PASS and no hardware L4 claim.
- **Next:** inspect this FE VBG commit's exact-head checks, then migrate FE TX-offset.


## Active task

### FE R50 checked MDIO, bounds and cleanup

- FE R50 setup and page-7/reg24 + page3/4 updates use checked APIs; both FE waits and comparator reads abort on transport failure.
- The legacy void setters remain for compatibility; the calibrator calls checked internal setters. MT7622 TX/RX per-port offsets are checked by a tested policy helper before lookup-table reads.
- Timeout, saturation and bounded search exhaustion use the 0x20 fallback; I/O and cleanup failures withhold `fe_cal_r50_flag`.
- Next candidate: `fe_cal_vbg()`, then FE all-analog orchestration.


## Verification boundary
- Source checkpoint: `ee0bf75292e0436e8820d3a30cd8b9a28c2bfee1`.
- Exact SHA Actions at handoff: NVRAM transaction boundary audit #45279437099: in_progress; XY-C1 Firmware Validation #45279437381: in_progress; AI handoff documentation contract #45279437408: success; Switch CLI ioctl failure-status regression #45279437249: failure
- Exact SHA check-runs: validate-handoff-records: success; Cross-build XY-C1 firmware: in_progress; switch-cli-contract: failure; nvram-transaction-boundary: in_progress
- Backup branch remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- No same-SHA XY-C1 Build+TRX+artifact L3 or hardware L4 is claimed unless the above exact-head run results explicitly show all product stages successful.

## Exact first action for the next GPT

1. Refresh exact-SHA Actions for `ee0bf75292e0436e8820d3a30cd8b9a28c2bfee1` from the run IDs above; inspect failed job logs and do not count queued/in-progress/skipped XY-C1 Build/TRX/artifact work as L3.
2. Continue with `fe_cal_vbg()` as a distinct checked-I/O/cleanup slice, then inspect FE all-analog orchestration.
3. Preserve the backup branch `72f4f157a084f8184b9597ca5bf334b8080e5807` unchanged and make each source task an atomic expected-SHA commit.


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