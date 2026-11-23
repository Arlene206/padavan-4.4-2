### 2026-11-23 接力更新（最新工程检查点）

- 当前 main 基线：`d524e61986a573eaa92c800893273e79f78f2f6f`，GE Rext 已在前一提交改为 checked Clause 45 I/O 和可靠 restore/disable；该 SHA Switch CLI #45275576269、AI docs #45275576266 PASS，NVRAM #45275576404 与 XY-C1 #45275576662 仍在执行。
- 当前源码切片：GE R50 common setup 和四 pair 校准不再调用 value-only PHY wrapper。分页寄存器、port-local RMW、pair selector、GE wait/comparator、四组 packed output 和 readback 均检查 MDIO 状态。
- 任一传输错误停止剩余 pair；所有退出都尝试 checked analog cleanup 和最终 Clause 45 `1e:db=0`。超时/saturation 保留原有 0 dB fallback；只有两项 cleanup 成功才设置 `ge_cal_r50_flag`。
- 新增 Switch CLI source contract；复用已由 GNU89/Werror/ASan/UBSan 覆盖的 checked update-bits policy。此提交 exact-head Actions 待检查，未声称 MIPS/kernel build、XY-C1 L3 或硬件 L4。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 冻结于 `72f4f157a084f8184b9597ca5bf334b8080e5807`。
- 下个 GPT 第一项：检查本次 SHA 的 Switch CLI / AI docs / NVRAM / XY-C1 exact-head runs；然后迁移 `ge_cal_tx_amp()` 的 checked save/restore 与错误传播。

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

### GE R50 four-pair calibration checked I/O and cleanup

- Checked common setup preserves the public legacy void ABI; the GE R50 body checks all page-register and Clause 45 operations.
- The shared page-7/register-24 RMW changes only z-calibration bits. FE ports 1..4 page3/reg25 disables, each pair-selector write, calibration wait/comparator and all pair result fields are checked.
- All exits attempt checked common analog cleanup and Clause 45 calibration disable. First error is retained, and the completion flag is withheld on transport/cleanup failures.
- Next bounded candidate: migrate `ge_cal_tx_amp()` with checked snapshots, wait status and guaranteed analog-disable cleanup.

### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Previous GE Rext SHA `d524e619`: Switch CLI #45275576269 PASS and AI docs #45275576266 PASS. NVRAM #45275576404 and XY-C1 #45275576662 were in progress at last poll.
- The GE R50 source and static-contract changes are new; exact-head Actions still need inspection.
- No local kernel/MIPS build, same-SHA XY-C1 Build+TRX+artifact L3 or physical hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main`; inspect this GE R50 commit's exact push Actions for Switch CLI, AI docs, NVRAM and XY-C1, including Build/TRX/artifact conclusions.
2. Diagnose any exact-head FAIL from its job logs; never treat queued/running/skipped XY-C1 work as L3.
3. Continue with `ge_cal_tx_amp()` as a separate slice; inspect its FE/GE mixed page selection and cleanup before touching TX-offset calibration.


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