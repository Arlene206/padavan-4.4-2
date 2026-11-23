### 2026-11-23 接力更新（最新工程检查点）

- 当前 main 基线：`eeae2974c079c84ee5684244c904a212080849c2`。GE TX-amplitude `eeae2974` 已提交，exact-head Switch CLI / AI docs / XY-C1 仍在执行；先前 GE R50 `ca05cf9` 的 Switch CLI / AI docs / NVRAM PASS，XY-C1 Build 仍在执行。禁止将未完成的 XY-C1 运行报告为 L3。
- 当前源码切片：`ge_cal_tx_offset()` setup、pair selector、四组控制寄存器 snapshot、page-7 comparator、两处 GE wait 及结果写全部改为 checked MDIO API。transport error 立即停止后续 pair；普通 not-ready 仍回退到既有 0x20 值，且不再读取无效 comparator。
- 所有退出都会执行 checked common analog clear，并逐个尝试八个强制 DAC 寄存器、DB/DC/3E/DD cleanup；cleanup 组中途失败仍继续后续写入并保留首错误；只有所有清理成功才设置完成标记。
- workflow 增加生产代码 source contract。无 MIPS/kernel build、无同 SHA XY-C1 Build/TRX/artifact L3、无硬件 L4 声明。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 保持冻结。
- 下个 GPT 第一项：检查本次 SHA 的 Switch CLI / AI docs / NVRAM / XY-C1 exact-head Actions；然后审查 FE analog calibration 其它调用点与共享 cleanup 依赖，不重复修改已覆盖的 GE Rext/R50/TX-amp/TX-offset 函数。

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

### GE TX-offset four-pair calibration checked snapshots and cleanup

- Setup, per-pair C45 writes, four saved control-register reads, comparator samples and GE wait operations use checked APIs.
- Ordinary not-ready timeout retains the existing 0x20 fallback; transport errors abort the remaining pair algorithm.
- Every exit attempts common analog clear and the full forced-DAC/DB/DC/3E/DD cleanup group, retaining the first error and withholding the completion flag on cleanup failure.
- Next action: inspect exact-head Actions, then audit remaining FE/GE analog calibration paths without reopening migrated GE Rext/R50/TX-amp/TX-offset work.


### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Previous TX-amplitude SHA `eeae2974`: exact-head Switch CLI / AI docs / XY-C1 statuses were still in progress at last check.
- Current TX-offset source/workflow changes are new and require exact-head Actions.
- No local kernel/MIPS build, same-SHA XY-C1 Build+TRX+artifact L3 or hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main`; inspect this TX-offset SHA exact push Actions for Switch CLI, AI docs and XY-C1 (and NVRAM if scheduled), including Build/TRX/artifact outcomes.
2. Diagnose exact-head failures from job logs; never equate queued, in-progress or skipped target steps with L3.
3. Audit remaining FE analog calibration call sites and shared cleanup dependencies without repeating the GE Rext/R50/TX-amp/TX-offset migrations.


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