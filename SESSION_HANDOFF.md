### 2026-11-23 接力更新（最新工程检查点）

- 当前 main 工程源基线：`2dfd89151b59774bad5598636fde26a98ba2b48b`（FE analog wait 已改为 checked MDIO，并保留 TX-amp 与普通 FE wait 的历史触发差异及双路径 cleanup）。
- 当前安全切片：`ge_cal_rext()` 的全部 Clause 45 读写改为 status-bearing API；snapshot、setup、wait、比较寄存器读和结果写任一失败都停止校准并禁止发布成功标记。
- 失败/超时会在 snapshot 有效时恢复原有 0 dB fallback；不论恢复是否成功，都会继续尝试清 `1e:db` 禁用校准引擎。成功结果仅在输出提交和 disable 均成功后设置 `ge_cal_flag`。
- 回归覆盖 cleanup 顺序、restore 错误后仍执行 disable、无 snapshot、disable 错误、正错误码归一化及 invalid callback；Switch CLI workflow 增加 GE Rext 源码契约和 GNU89/Werror/ASan/UBSan policy regression。
- 基线 `2dfd891` 的 Switch CLI #45274289879、AI docs #45274290063 PASS；NVRAM #45274289994 与 XY-C1 #45274289973 当时仍在执行。本切片 exact-head Actions 待提交后检查。无 MIPS/kernel build、无同 SHA XY-C1 Build/TRX/artifact L3、无硬件 L4 声明。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 冻结于 `72f4f157a084f8184b9597ca5bf334b8080e5807`，保持只读。
- 下个 GPT 第一项：检查本次 SHA 的 Switch CLI / AI docs / NVRAM / XY-C1 exact-head push runs；随后单独审查 `ge_cal_r50()` 的四 pair 状态及 shared page-7/reg24 RMW。不得把 host tests 代替 L3，也不得声称未做的硬件 L4。

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

### GE Rext analog calibration checked I/O and cleanup

- Every Clause 45 operation in `ge_cal_rext()` uses the status-bearing helpers. A failed snapshot, setup write, wait, comparator read or result write cannot continue with fabricated values.
- On timeout/error, the valid saved `1e:e0` snapshot is restored using the legacy 0 dB fallback, then `1e:db` is always cleared. Successful output is retained only after all result writes and disable succeed.
- Fault-injection policy test and source-contract step are integrated into Switch CLI stability workflow.
- Next separate candidate: audit `ge_cal_r50()` shared page-7/register-24 transaction and per-pair outputs, then migrate one bounded section without changing legacy ABI.

### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Baseline source SHA `2dfd891`: Switch CLI #45274289879 PASS and AI docs #45274290063 PASS; NVRAM #45274289994 and XY-C1 #45274289973 were pending at last inspection.
- Current GE Rext source/test/workflow changes are new; only exact-head Actions can validate them.
- No local kernel/MIPS build, same-SHA XY-C1 Build+TRX+artifact pass or physical hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main`; inspect this new source SHA's exact push Actions for Switch CLI, AI docs, NVRAM and XY-C1, including Build/TRX/artifact job conclusions.
2. Diagnose any exact-head FAIL from job logs. If XY-C1 Build/TRX/upload are skipped or incomplete, report host-only validation, not L3.
3. Continue with `ge_cal_r50()` as a distinct bounded slice; trace its shared page-7/reg24 state and four pair-specific save/restore before editing.


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