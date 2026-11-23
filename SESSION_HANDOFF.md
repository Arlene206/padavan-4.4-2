### 2026-11-23 接力更新（最新快照）

- 上一提交 `f432a3ef` 修正 write-policy 声明顺序；Switch CLI #45272997503 PASS，AI docs #45272997319 PASS。XY-C1 #45272997410 与 NVRAM #45272997327 当时仍在运行。
- 新增 GE analog-calibration wait status 路径：checked page-7 register 24 read/write、bounded ready polling、所有退出 fresh-read 并尝试清 trigger bit；transport error 不再与正常 timeout 混淆，ready 输出置 0。
- 保持 legacy `u8 all_ge_ana_cal_wait()` 签名不变；底层 transport/cleanup error 转成已有 `ANACAL_ERROR`，调用者进入原错误处理路径。当前只迁移 wait handshake，GE REXT/R50/TX amp/TX offset 的其他 MMD 读写仍需后续事务化。
- 新增每个 setup/poll/cleanup 位置故障注入、正状态归一化、timeout、ready 和非法参数测试；Switch CLI workflow 增加源码契约。本次 SHA Actions 待检。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 冻结于 `72f4f157a084f8184b9597ca5bf334b8080e5807`，未触碰。
- 无 MIPS kernel build、无 XY-C1 同 SHA Build/TRX/artifact 三项 PASS 证明、无硬件 L4 声明。
- 下个 GPT 第一项：检查本次 SHA Switch CLI/AI docs/NVRAM/XY-C1；随后选择一个 GE calibration 算法梳理全部 register snapshot/restore，并在错误路径恢复触发位。

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
- Prior checked-write SHA `f432a3e`: Switch CLI #45272997503 PASS; AI docs #45272997319 PASS; XY-C1 #45272997410 and NVRAM #45272997327 were pending at last poll.
- GE calibration wait status/cleanup slice is new; exact-head Actions must be checked after commit.
- XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on the same source SHA. No such result is claimed here. No hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main`; inspect this slice's exact-SHA Switch CLI, AI docs, NVRAM and XY-C1 jobs/logs.
2. Check whether the XY-C1 workflow actually ran Build/TRX/upload; never infer L3 from a host-only skipped-build success.
3. Continue with one GE calibration algorithm's full save/restore path; preserve ABI and trigger cleanup on each checked error.


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