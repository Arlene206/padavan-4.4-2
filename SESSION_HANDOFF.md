### 2026-11-23 接力更新（最新快照）

- 本轮 main 基线：`72076155464bb033d5dc61d5d141e76cd08c2181`，已包含 PHY debug fix `ad5bcd2b87b9b672dd2e921b6e133fb35393093c`。
- 新修复 GE2 `phy_polling_thread()`：六次 Clause 22/45 读取聚合为完整快照；任何读取失败均丢弃快照并保留先前 MAC/link 状态，不再把 MDIO 超时误判为 link-down。只有确认的 link-down 样本才触发原 force-down 操作。
- 修复 EEE capability stale-bit：重新协商未再支持 EEE 时清除旧 1G/100M EEE 位；每个成功样本更新 duplex/speed/flow-control/EEE 位，并保留无关位。
- 回归覆盖六个读点故障注入、正错误码归一化、完整样本顺序、link-down 短路、无效 callback、失败状态保留和 stale EEE 清除；Switch CLI workflow 新增源码 contract。相关生产/测试/头文件已由 Switch CLI 与 XY-C1 path filters 覆盖。
- 前一 debug source SHA `ad5bcd2`：Switch CLI #45270470838 PASS；AI docs #45270470371 因继承 roadmap 超 385 bytes FAIL，之后 docs-only SHA `7207615` 的 AI docs #45270615085 PASS；XY-C1 #45270470239、NVRAM #45270470405 当时 IN_PROGRESS。本次新提交 exact-SHA Actions 尚待检查。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 冻结于 `72f4f157a084f8184b9597ca5bf334b8080e5807`，本轮未修改。
- 无本地 MIPS/kernel build、无 XY-C1 Build/TRX/artifact 同 SHA 全 PASS 证明、无硬件 L4 声明。
- 下个 GPT 第一项：检查本次提交 exact-SHA Switch CLI、AI docs、NVRAM、XY-C1；处理 FAIL，PENDING 时继续独立任务。随后分析 GE analog calibration 的寄存器保存/恢复契约，不可盲目批量替换兼容 wrapper。

---
# Cross-session execution mandate

## Current repository state

### 2026-11-23 continuation — current state

- **Previous source commit:** `ad5bcd2b87b9b672dd2e921b6e133fb35393093c` checked Clause 45 debug reads and serialized token-ring page/control/data access. Switch CLI #45270470838 PASS. Its AI docs run #45270470371 failed on the inherited oversized roadmap; docs-only main child `72076155464bb033d5dc61d5d141e76cd08c2181` corrected the file and AI docs #45270615085 PASS.
- **Current source slice:** GE2 polling obtains one complete C22/CL45 sample. Any failed transfer is discarded and does not write MAC link state or force link-down; a successful down sample retains the original force-down behavior.
- **EEE state:** capability rebuild clears old 1G/100M EEE bits before applying the partner-negotiated flags, fixing stale EEE after renegotiation.
- **Tests:** injected failures at each of six reads, positive status normalization, full success/read order, link-down short-circuit, invalid callbacks and stale EEE cleanup. Source-contract checks added to Switch CLI.
- **Backup:** frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify.
- **Verification boundary:** this slice's exact-SHA Actions are pending. Previous XY-C1 #45270470239 and NVRAM #45270470405 for `ad5bcd2` were in progress at last poll. No kernel/MIPS build or hardware L4 claimed.
- **Next task:** after checking exact-head Actions, trace GE analog-calibration save/restore ordering and select one bounded checked transaction.
- **Working rule:** source → tests → atomic tree/commit → expected-SHA non-forced `main` update → exact-SHA Actions; PENDING does not block independent work.


## Active task

### GE2 PHY polling MDIO failure and renegotiation state

- Prior debug source `ad5bcd2b87b9b672dd2e921b6e133fb35393093c`: checked Clause 45 debug reads and serialized token-ring transaction/page restoration; Switch CLI #45270470838 PASS.
- Current change replaces unchecked periodic PHY polling with a complete fail-closed snapshot; a timeout can no longer masquerade as link-down or publish fabricated capabilities.
- Per-success state assembly clears stale EEE bits when a link renegotiation changes advertised capabilities.
- **Next:** check exact-head Actions, then inspect GE analog-calibration save/restore dependencies before making another bounded conversion.



### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Prior debug source SHA `ad5bcd2`: Switch CLI #45270470838 PASS; AI docs #45270470371 failed only on the inherited roadmap byte cap, fixed on docs-only child `7207615` with AI docs #45270615085 PASS. XY-C1 #45270470239 and NVRAM #45270470405 were in progress at last check.
- This GE2 polling slice is new; exact-head Switch CLI/AI docs/NVRAM/XY-C1 statuses must be fetched after commit.
- No local MIPS/kernel build or physical-router L4 is claimed. XY-C1 L3 requires Build, TRX and artifact upload all PASS on one production-source SHA.
- Frozen backup remains untouched.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-SHA Switch CLI, AI handoff, NVRAM and XY-C1 runs for this GE2 polling commit; diagnose every FAIL from its job log.
2. If XY-C1 is PENDING, continue independent work. Claim L3 only after Build, TRX and artifact upload pass on the same production-source SHA.
3. Trace `ge_cal_rext()`, `ge_cal_r50()`, `ge_cal_tx_amp()`, `ge_cal_tx_offset()` and `do_ge_phy_all_analog_cal()` for save/restore ordering before converting legacy MDIO wrappers.


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