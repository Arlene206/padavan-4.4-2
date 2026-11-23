### 2026-11-23 接力更新（最新快照）

- 当前产品源码 SHA：`bfca47b0c33cae7679ea2e671cc71a015ebfc5d4`，修复 GE2 PHY polling 失败传播与 EEE renegotiation stale-bit。
- 首次 Switch CLI exact-head #45271645960 失败在新增测试 fixture：`0x24` 含 duplex flag `0x20`，并非半双工样本；生产逻辑按输入正确保留 duplex。已将回归向量改为 `0x04`，并以 GNU89/Werror/ASan/UBSan 独立 policy harness 验证通过；仓库完整测试须在下一个 SHA 重跑。
- `bfca47b` 的 AI docs #45271646030 PASS；XY-C1 #45271645953 与 NVRAM #45271645977 上次检查为 IN_PROGRESS。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 冻结于 `72f4f157a084f8184b9597ca5bf334b8080e5807`，保持只读。
- 无本地 MIPS/kernel build、无 XY-C1 Build/TRX/artifact 同 SHA 全 PASS 证明、无硬件 L4 声明。
- 下个 GPT 第一项：检查本次测试修正提交 exact-SHA Switch CLI、AI docs、XY-C1 和 NVRAM；FAIL 查看日志修复，PENDING 时继续其他独立任务。然后追踪 GE analog calibration 保存/恢复顺序，选一条有界事务迁移。

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
- Source SHA `bfca47b`: AI handoff #45271646030 PASS; Switch CLI #45271645960 FAIL only because the new half-duplex regression used `0x24` (the `0x20` duplex bit was set); the test fixture is corrected to `0x04` in the current follow-up. XY-C1 #45271645953 and NVRAM #45271645977 were IN_PROGRESS at the last poll.
- Exact repository Switch CLI policy binary/source contracts must rerun on this test-only correction SHA.
- No local MIPS/kernel build or physical-router L4 is claimed. XY-C1 L3 requires Build, TRX and artifact upload all PASS on the same production-source SHA.
- Frozen backup remains untouched.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-SHA Switch CLI, AI handoff and XY-C1 Actions for the test-fixture correction; check NVRAM if it triggers. Diagnose any failure from its job log.
2. Do not treat the corrected host regression as a firmware build. Claim L3 only after XY-C1 Build, TRX and artifact upload all pass on one source SHA.
3. Continue the bounded GE analog-calibration save/restore analysis after CI state is recorded.


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