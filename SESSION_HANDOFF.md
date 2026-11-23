### 2026-11-23 接力更新（最新快照）

- 当前 main 产品源码 SHA：`51f29580ebfb44d9ecbcbd4ec6bf8a55ff6e77c7`（`fix(raeth): check analog calibration cleanup RMW`）。新增 checked update-bits policy；`clear_ckinv_ana_txvos()` 的三个 RMW 在首次 MDIO 失败时停止，但始终 best-effort 执行原最终 page-7/reg24 写零。
- 测试：覆盖成功时保留无关位、read timeout、positive error 归一化、write failure、输出清零、invalid callbacks；Switch CLI source contract 检查生产 helper 的三个 RMW 和 final cleanup。
- `51f2958` 的 AI docs #45273675795、Switch CLI #45273676193、XY-C1 #45273675906、NVRAM #45273675699 当时均在执行（AI docs queued、其它 in progress）；必须检查 exact SHA 结果。无 MIPS kernel build 或硬件 L4 声明。
- 修正文档问题：`51f2958` 更新 dev_log 时意外只写入新条目，丢掉了旧历史；本提交恢复 parent `87e233d` 的完整 58 KB log 历史并前置最新 checkpoint。源码无变化。
- 前一 SHA `87e233d`：Switch CLI #45273294951 PASS、AI docs #45273294849 PASS、NVRAM #45273294824 PASS；XY-C1 #45273294825 在上次检查仍 in progress。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 冻结于 `72f4f157a084f8184b9597ca5bf334b8080e5807`，持续只读。
- 下个 GPT 第一项：检查最新 docs child commit exact-SHA AI docs；同时检查 `51f2958` 的 Switch CLI/XY-C1/NVRAM job 细节；XY-C1 Build/TRX/upload 若未同 SHA 全 PASS，不得报告 L3。然后逐个处理其他 analog calibration RMW/save-restore 风险。

---
# Cross-session execution mandate

## Current repository state

### 2026-11-23 continuation — current state

- **PHY debug:** `ad5bcd2b87b9b672dd2e921b6e133fb35393093c` added checked Clause 45 debug reads and token-ring transfer/page-restore checks. Switch CLI #45270470838 PASS.
- **GE2 link polling:** `bfca47b0c33cae7679ea2e671cc71a015ebfc5d4` now publishes only complete C22/CL45 samples; failed reads preserve the previous MAC/link state. EEE capability bits are rebuilt on successful renegotiation. Fixture/checker fixes are in `5abf4a5` and `48bc28a`; Switch CLI #45272254777 and AI docs #45272254645 PASS.
- **Legacy PHY wrappers:** `d887063` and `f432a3e` introduced checked C22/Clause45 write helpers while preserving old wrapper signatures. Switch CLI #45272997503 and AI docs #45272997319 PASS on `f432a3e`.
- **GE calibration wait:** `87e233d30dd532b6bfd1589d2469ced8d8f42a16` checks setup/poll/cleanup MDIO I/O on page-7 register 24; transport failure is distinct from timeout and cleanup clears the trigger. Switch CLI #45273294951, AI docs #45273294849 and NVRAM #45273294824 PASS. XY-C1 #45273294825 remained in progress at last poll.
- **Analog cleanup RMW:** current source `51f29580ebfb44d9ecbcbd4ec6bf8a55ff6e77c7` adds checked update-bits for `clear_ckinv_ana_txvos()`; first intermediate failure stops remaining RMW, but final register-zero cleanup is always attempted.
- **Docs integrity:** the latest source commit truncated `dev_log.md`; current documentation child restores the entire history from `87e233d` plus the latest checkpoint. No source files changed in the documentation repair.
- **Backup:** frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify.
- **Verification boundary:** `51f2958` exact-head Switch CLI/AI/XY-C1/NVRAM outcomes must be checked. No same-SHA XY-C1 Build+TRX+artifact PASS or physical hardware L4 is claimed.
- **Next:** inspect exact-head CI, then trace remaining GE calibration MMD RMW/save/restore sequences individually.


## Active task

### Fail-safe analog calibration cleanup

- GE wait helper uses status-bearing page-7/register-24 setup, bounded poll and final trigger cleanup; MDIO transport errors map to the existing `ANACAL_ERROR` compatibility sentinel.
- `clear_ckinv_ana_txvos()` now uses checked read-modify-write for masks `0x2000`, `0x4000`, and `0x1000`; it stops intermediate RMW on error and always attempts the original final write-zero operation.
- Legacy void ABI is preserved; failures are rate-limited in logs.
- **Next:** check exact-head Actions for `51f2958`, then review other calibration RMW/save/restore functions before expanding checked conversions.



### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- `87e233d`: Switch CLI #45273294951 PASS, AI docs #45273294849 PASS, NVRAM #45273294824 PASS; XY-C1 #45273294825 was still running at last poll.
- `51f2958`: exact-head Switch CLI #45273676193, AI docs #45273675795, XY-C1 #45273675906 and NVRAM #45273675699 were pending/in progress at checkpoint creation. The documentation restore child triggers a fresh AI-docs validation; source CI must still be checked on `51f2958`.
- No same-SHA XY-C1 Build+TRX+artifact PASS, no local MIPS kernel build and no hardware L4 claim.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the documentation-repair commit's AI docs run plus `51f2958` exact-SHA Switch CLI, NVRAM and XY-C1 jobs/logs.
2. Confirm whether Build, TRX verification and artifact upload actually ran and passed on `51f2958`; skipped steps do not qualify as firmware L3.
3. Continue the remaining GE calibration audit with one register RMW or save/restore sequence per atomic source/test commit.


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