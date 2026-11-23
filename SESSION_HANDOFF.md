### 2026-11-23 接力更新（最新快照）

- 本轮 main 源码提交：`ad5bcd2b87b9b672dd2e921b6e133fb35393093c`（`fix(raeth): fail closed on PHY debug MDIO errors`）。本次随后接力文档子提交仅压缩路线图历史段、刷新状态，不改变产品源码。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 保持冻结：`72f4f157a084f8184b9597ca5bf334b8080e5807`。
- 已完成：MTK ESW 端口/VLAN/IGMP/MLD 检查式写入和错误传播；paged PHY page-select 门控与 IRQ-safe 序列锁；reset-ready 有界轮询；GSW/ESW 链路状态只发布成功 MDIO 样本；MT7621/MT7623 TRGMII 错误传播；proc MIB 失败读显示 `ERR`、snapshot 写失败可见；PHY token-ring 与 Clause 45 调试读取现在 fail-closed。
- `ad5bcd2` 新增 Clause 45 checked-read 策略和失败注入；token-ring 调试序列检查每次写读、持有 PHY page 锁、始终尝试恢复 page 0；Leopard MMD 读失败时停止输出，避免把超时清零误报为测量值。Host regression 与 source checker 已加入 workflow；必须以 exact-SHA Actions 结果判定。
- 前一产品源码 SHA `638dd487b626b0aac718eec046573dcd1393274c` 的 Switch CLI、AI docs、NVRAM、Process Actions 均 PASS；XY-C1 #45268911701 上次检查仍 IN_PROGRESS。后续文档子提交 `7ec66ff` 的 AI docs #45269398685 因路线图 131457 UTF-8 bytes 超过 131072 限制而失败；本子提交已缩短重复历史段。
- 无本地 Linux/MIPS 交叉编译、无 XY-C1 Build/TRX/artifact 同 SHA 全 PASS 证明、无实体路由器 L4；不得将 host regression 写成固件构建通过。
- 下一项：核对 `ad5bcd2` exact-SHA Switch CLI、AI docs、NVRAM、Process、XY-C1；随后选择一个有界 PHY calibration caller 家族，将 value-only MDIO 调用迁移为 status-bearing 操作并加回归测试。

---
# Cross-session execution mandate

## Current repository state

### 2026-11-23 continuation — current state

- **Source commit:** `ad5bcd2b87b9b672dd2e921b6e133fb35393093c`, checked PHY debug reads, host failure-injection and source integration validation.
- **Backup:** frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch.
- **Parent verification:** source SHA `638dd487` Switch CLI #45268911721, AI docs #45268911599, NVRAM #45268913631 and Process #45268911977 PASS; XY-C1 #45268911701 was IN_PROGRESS. The docs-only child `7ec66ff` failed AI docs #45269398685 because roadmap was 131457/131072 UTF-8 bytes; this documentation child trims it to below 130000 bytes.
- **Completed slices:** checked MTK ESW config/IGMP/MLD; paged PHY locking and selector errors; reset timeout; GSW/ESW link status propagation; TRGMII MDIO errors; proc MIB read/write diagnostics.
- **Current slice:** token-ring debug reads/writes now propagate status internally, restore page 0 under the page lock, and suppress partial values; Leopard Clause 45 diagnostics use the checked-read policy.
- **Verification boundary:** this source commit's exact-head Switch CLI/AI docs/XY-C1/NVRAM/Process status must be checked after the docs child is merged. No kernel/MIPS build, XY-C1 firmware pass or hardware L4 is claimed.
- **Next implementation task:** inspect bounded production PHY calibration callers still using value-only `tc_phy_read_dev_reg()` / `tc_mii_read()`; migrate one safe caller set to checked status while preserving public ABI and add failure regression.
- **Working rule:** source → tests → atomic tree/commit → expected-SHA non-forced `main` update → exact-SHA Actions. PENDING does not block independent work.


## Active task

### RAETH PHY debug MDIO failure handling

- Source commit `ad5bcd2b87b9b672dd2e921b6e133fb35393093c` adds Clause 45 checked reads with failure-injection tests; token-ring debug checks writes and reads, holds the PHY-page lock, restores page 0 after every attempted transaction and publishes no partial value. The Leopard MMD logger stops on the first failed read.
- Parent `638dd487`: Switch CLI, AI docs, NVRAM and Process PASS; XY-C1 was in progress. The docs-only child `7ec66ff` AI docs run failed on the roadmap byte cap; this docs child trims that repeated history.
- **Next:** check exact-head Actions for `ad5bcd2`, then migrate one bounded production PHY calibration caller family from value-only MDIO wrappers to status-bearing operations.


### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Source SHA `638dd487`: Switch CLI #45268911721 PASS; AI docs #45268911599 PASS; NVRAM #45268913631 PASS; Process #45268911977 PASS; XY-C1 #45268911701 IN_PROGRESS at last check.
- Docs-only child `7ec66ff` AI docs #45269398685 failed only due roadmap size 131457/131072 UTF-8 bytes; this child trims the file. Current source commit `ad5bcd2` exact-SHA CI is pending. No kernel/MIPS or hardware L4 claimed.
- Earlier production SHA `59622f3c9612949ddd642fc738e02463f5400def`: XY-C1 Build/TRX/artifact #45252058834 PASS and NVRAM #45252058851 PASS; those historical results do not validate later source changes.
- Frozen backup remains untouched. Physical router hardware L4 and local MIPS compilation are NOT VERIFIED.
- LLDT config parser: dedicated L2 workflow #45238664960 PASS; Process umbrella #45239229205 PASS. XY-C1 #45237411963 PASS on exact source SHA `eef4417` (Build/TRX/artifact). AI docs #45239229201 needs one workflow path-filter correction (this commit).
- Shadowsocks DNS proxy preflight and upstream whitespace fixes: #45234515153 PASS, Process #45234515707 PASS and AI docs #45234515042 PASS on `f3acf67`; XY-C1 #45234516216 was pending.
- RStats `601ad73`: RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS; AI doc repair #45236207680 PASS on `3760b93`. XY-C1 #45234873441 still requires final same-SHA step confirmation before an L3 claim.
- Shadowsocks missing-executable test `5a43b18`: lifecycle #45235046114 and Process #45235046275 PASS; AI documentation repaired by #45236207680 on `3760b93`.
- Exact LLDT #45238664960 on `38d3732` passed GNU89/Werror/ASan/UBSan compile/runtime and its Python production/optional-tool checker; AI docs #45238664839 PASS. Process #45239229205 PASS on `79f4d53`. XY-C1 #45237411963 on the production source SHA `eef4417` passed Build, TRX and artifact upload. AI docs #45239229201 exposed only the path-filter asymmetry corrected in this commit. No local compile is claimed.
- Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-SHA Switch CLI, AI docs, NVRAM, Process and XY-C1 runs for source commit `ad5bcd2b87b9b672dd2e921b6e133fb35393093c`. Ensure the AI docs checker now accepts the roadmap below its 131072-byte cap.
2. Diagnose any failing host/source assertion from its job log; do not conflate it with firmware Build/TRX/artifact status.
3. Inspect remaining PHY calibration callers of `tc_phy_read_dev_reg()` / `tc_mii_read()`; pick one bounded production setup path for checked status propagation and failure regression.


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