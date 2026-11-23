### 2026-11-23 接力更新（最新快照）

- 当前 main 之前的 GE2 polling 修复：`bfca47b0c33cae7679ea2e671cc71a015ebfc5d4`；半双工测试 fixture 修正 `5abf4a5c4b3fb08b6c3f5e8d13418679aec4dd58`。
- Source-contract delimiter 修正提交 `48bc28ab58076e9ac90a0a9546e8ca91996caa56`：Switch CLI #45272254777 PASS，AI docs #45272254645 PASS。`5abf4a5` 的 NVRAM #45271922399 PASS；XY-C1 #45271922442 workflow PASS，但 Build/TRX/artifact upload 因 production inputs 未变化而 SKIPPED，仅为 host-only 验证，不构成 L3。
- 本轮新源码切片：四个 legacy C22/Clause45 PHY read/write wrapper 现在经 status-bearing checked adapter，失败时保留原 ABI、读值初始化为 0，并报告 rate-limited I/O 错误。新增 C22/45 write status policy 与负 errno / 正状态 / invalid callback fault injection。
- 这只是让遗留调用可观测，尚未让 GE analog calibration 整条调用链中止；长校准函数中的寄存器保存/恢复契约仍待专门迁移，未做批量替换。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 冻结于 `72f4f157a084f8184b9597ca5bf334b8080e5807`，本轮未修改。
- 无本地 MIPS/kernel build、无 XY-C1 L3 Build/TRX/artifact 同 SHA 证明、无硬件 L4 声明。
- 下个 GPT 第一项：检查本次 wrapper 修复 exact-SHA Switch CLI/AI docs；处理 FAIL。随后继续分析 GE calibration wait helper 的状态传播与清理，确保任何 MDIO 错误不会与校准超时混为一谈。

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
- GE2 polling contract fix SHA `48bc28a`: Switch CLI #45272254777 PASS; AI handoff docs #45272254645 PASS.
- Its parent source/test SHA `5abf4a5`: NVRAM #45271922399 PASS. XY-C1 #45271922442 PASS as a workflow, but firmware Build/TRX/artifact upload were all SKIPPED because no firmware production inputs changed; host-only result, NOT L3.
- Current legacy wrapper MDIO status slice is newly committed; exact-head Switch CLI/AI docs runs must be inspected.
- No local MIPS/kernel build or physical hardware L4 is claimed. Frozen backup remains untouched.

## Exact first action for the next GPT

1. Fetch live `main` and inspect this commit's exact-SHA Switch CLI and AI handoff Actions; diagnose any FAIL from job logs.
2. Continue the GE analog-calibration status propagation audit with `all_ge_ana_cal_wait()` and the `ge_cal_rext/r50/tx_amp/tx_offset` callers. Separate MDIO transport failures from calibration timeout and preserve trigger-bit cleanup on every exit.
3. Keep the legacy ABI stable; do not bulk-replace value-only wrappers without a complete restoration plan.


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