### 2026-11-23 接力更新（最新快照）

- 文档 child `bff3d15326e573f5210ac2a121e21c7485744ece` 已恢复完整 dev_log 历史。其前一源码 SHA `51f2958` 的 Switch CLI #45273676193 PASS；AI docs #45273675795 因当时日志只剩一个条目而 FAIL，修复后 child 的 AI docs #45273887255 正在执行。XY-C1 #45273675906 仍在 Build，NVRAM #45273675699 仍执行中（上次检查）。
- 本轮新增 FE analog wait status 路径：TX-amp 保留“触发前只清 FE_CAL_COMMON local page4/reg23”的历史行为；普通 FE wait 预清 common+selected port。两者复用 checked page7/reg24 GE trigger/poll 事务，并且无论错误或成功都尝试清理 common 和 selected-port local page4/reg23。
- 旧 u8 ABI 不变；MDIO transport/cleanup error 返回既有 `ANACAL_ERROR`；普通 not-ready timeout 仍返回 0。新增 paired-cleanup policy regression，证明两次 cleanup 都会尝试且保留首个错误；Switch CLI source contract 检查生产路径。
- 本次源码 commit exact-SHA Actions 待检查。无 MIPS build、无 same-SHA XY-C1 Build/TRX/artifact L3、无硬件 L4 声明。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 保持冻结。
- 下个 GPT 第一项：检查本次 FE wait change 的 Switch CLI/AI docs/XY-C1/NVRAM；再继续逐条迁移 GE/FE calibration RMW/save-restore。

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

### FE/GE analog calibration wait and cleanup

- GE page-7/register-24 trigger handshake is status-bearing and attempts trigger clear on all exits.
- FE TX-amp and normal FE waits share the checked handshake but preserve their different local page-4/register-23 pre-trigger resets.
- Both FE wait variants always attempt common and selected-port local register cleanup; transport/cleanup errors map to existing `ANACAL_ERROR` without changing exported `u8` signatures.
- `clear_ckinv_ana_txvos()` uses checked RMW and still attempts its final zero write.
- **Next:** check exact-head Actions for this FE slice, then migrate one other calibration RMW or save/restore sequence.



### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- `51f2958`: Switch CLI #45273676193 PASS; AI docs #45273675795 failed on the truncated one-entry log archive, repaired by docs child `bff3d15` (AI validation #45273887255 in progress at last poll). XY-C1 #45273675906 and NVRAM #45273675699 also in progress at last poll.
- Current FE-wait source slice is new; exact-SHA Switch CLI/AI docs/NVRAM/XY-C1 remain to be checked.
- No same-SHA XY-C1 Build+TRX+artifact PASS or physical hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect the FE-wait commit's exact-SHA Switch CLI, AI docs, NVRAM and XY-C1 jobs/logs.
2. Verify Build, TRX and artifact upload all pass on the same source SHA before claiming firmware L3; skipped stages remain host-only.
3. Continue one bounded GE/FE calibration RMW or save/restore transaction; preserve legacy ABI and all cleanup attempts.


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