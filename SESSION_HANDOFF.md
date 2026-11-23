### 2026-11-23 接力更新（最新工程检查点）

- 当前 main 父提交：`d0c858d552b211a7f8b8faeaee080fe3f6ce332d`（FE R50 checked-MDIO）。该 SHA 的 Switch CLI #45278800489、AI docs #45278800567、NVRAM #45278800373、XY-C1 #45278800520 上次检查均为 IN_PROGRESS，尚无完成结果。
- 最新源码切片：`fe_cal_vbg()` 的全部 C22 页寄存器 setup、RMW、comparator、wait 和输出编程改为 checked API；MDIO 传输/cleanup 错误不再被当作零值或成功。
- 搜索码步进显式限制在 6-bit 范围，Leopard 的 `ZCAL_TO_REXT_TBL` 索引增加 `ARRAY_SIZE` 检查；not-ready、saturation 和 254 次穷尽保留 0x20 fallback。
- VBG 所有退出都尝试 common analog clear、port/common page-3 register-25 reset；completion flag 仅在输出与 cleanup 全部成功且 calibration 真正完成后发布。新增 6-bit wrap host regression 和 Switch CLI source contract。
- 新 FE VBG 切片尚需 exact-head Actions；无本地 MIPS/kernel build、无同 SHA XY-C1 Build/TRX/artifact L3、无硬件 L4 声明。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 仍冻结于 `72f4f157a084f8184b9597ca5bf334b8080e5807`，未修改。
- 下个 GPT 第一项：检查本次 FE VBG 源码 SHA 的 Switch CLI、AI docs、NVRAM 与 XY-C1 Build/TRX/artifact；之后迁移 `fe_cal_tx_offset()` 的未检查 C22 操作与 cleanup。

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

### FE VBG checked MDIO, bounded calibration code and cleanup

- C22 setup, page-7 update-bits, local page-3 setup, comparator samples, wait and final output writes all use checked operations.
- Six-bit stepping is explicit; table access is range-checked. The existing 0x20 not-ready/saturation/exhaustion fallback remains.
- Every exit attempts checked common analog clear and both port/common page-3 register-25 resets; the completion flag is cleared at entry and only published after successful calibration and cleanup.
- Next bounded candidate: convert `fe_cal_tx_offset()` to checked C22 page I/O and fail-safe cleanup.




## Verification boundary
- Parent FE R50 SHA `d0c858d552b211a7f8b8faeaee080fe3f6ce332d`: Switch CLI #45278800489, AI docs #45278800567, NVRAM #45278800373 and XY-C1 #45278800520 were IN_PROGRESS at last poll; no completed result or L3 claim was available.
- Current FE VBG source, policy regression, workflow and handoff changes require their own exact-head Actions.
- No local kernel/MIPS build, same-SHA XY-C1 Build+TRX+artifact L3 or physical hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and identify this FE VBG source commit SHA; inspect its exact push Actions, especially Switch CLI and XY-C1 Build/TRX/artifact.
2. Diagnose exact-head failures from logs; do not count queued, in-progress or skipped target work as L3.
3. Continue with `fe_cal_tx_offset()` as the next independent checked-I/O/cleanup slice.


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