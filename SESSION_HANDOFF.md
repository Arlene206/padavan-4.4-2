### 2026-11-24 最新接力检查点

- 当前父 main SHA：`f7183840c0de8e8096d60940eb2e5d4b60687839`（`test(raeth): match GE orchestrator status API`）。冻结备份 `backup/pre-plugin-overhaul-2026-11-14` SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`，保持只读。
- FE all-analog 真实早期失败问题：两个 snapshot reads（local page0/reg26、reg30）失败时跳转公共 cleanup，但在这之前本次调用尚未开始 PHY setup；旧 cleanup 仍清共享 analog control 并写 page1/reg26、BMCR `0x3100`、advertisement `0x05e1`，可能不必要重置 PHY。
- THIS commit 在第一个 setup MDIO write 之前设置 `setup_attempted`（为覆盖写事务错误但硬件可能已收到的情况）；初始 snapshot 失败时跳过硬件 teardown；已尝试 setup 写入后继续完整 best-effort cleanup。清理被提取到 `ra_switch_fe_analog_cal_cleanup()`，保持所有原 cleanup、saved snapshots 条件恢复及 first-error 语义。
- 新增 `ra_switch_mdio_cleanup_if_started()` 以及 GNU89 callback fault-injection test（setup 前跳过、setup 后调用、错误透传、正状态归一化、NULL callback）。Switch CLI workflow 增加 ASan/UBSan host test 和源码顺序检查。
- Parent exact Actions：Switch CLI #45284655414 PASS；AI handoff #45284655394 PASS。产品源码 SHA `30f6a0e...` 的 XY-C1 #45284437855 当时 IN_PROGRESS、NVRAM #45284437839 SUCCESS。THIS commit 的新 run 必须按 exact SHA 核验。
- 没有本地 MIPS/kernel build 或硬件测试；无 L3/L4 声明。

---
# Cross-session execution mandate

## Current repository state

### 2026-11-24 continuation — RAETH PHY reliability

- **FE VBG/R50/TX amplitude/TX offset:** checked paged/C22/C45 I/O, bounded searches, failure propagation, fallback and cleanup are in place.
- **FE all-analog orchestrator:** snapshots and child completion gates are checked. THIS commit prevents teardown writes when snapshot reads fail before setup starts; the original complete best-effort teardown remains after the first attempted setup write.
- **GE calibration:** Rext/R50/TX amplitude/TX offset use checked I/O, bounded searches and cleanup; errors propagate through EPHY initialization.
- **Leopard EPHY:** local MDIO initialization is checked and fail-fast.
- **EPHY ioctl:** calibration error status propagates through `ei_ioctl()`; exhausted FE/GE retry sequences return `-EIO`.

## Active task

### Validate FE cleanup gate, then continue independent stability fixes

- Fetch live main and exact-head Switch CLI + AI handoff push runs for THIS commit.
- Require the GNU89 policy test with ASan/UBSan and FE source-contract step to pass.
- Refresh same-SHA XY-C1 Build, TRX and artifact upload; L3 only when all three pass on identical source SHA.
- Continue a concrete independent stability defect outside completed checked-MDIO calibration migration.

## Verification boundary

- Parent: `f7183840c0de8e8096d60940eb2e5d4b60687839`; THIS commit changes `ra_switch.c`, `ra_switch_mdio_policy.h`, `tests/ra_switch_mdio_policy.c`, Switch CLI workflow, `dev_log.md`, and `SESSION_HANDOFF.md`.
- Parent exact Actions: Switch CLI #45284655414 PASS; AI handoff #45284655394 PASS.
- Previous source SHA `30f6a0e...`: XY-C1 #45284437855 IN_PROGRESS (last observed); NVRAM #45284437839 SUCCESS. These do not verify THIS commit.
- No local cross-build, same-SHA complete XY-C1 L3, router flash or hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Switch CLI and AI handoff Actions; read logs for any failures.
2. Verify the new cleanup callback host test/source contract, then refresh XY-C1 Build/TRX/artifact for the new source SHA.
3. Continue another confirmed stability issue in RAETH/userland; do not repeat completed PHY checked-I/O migrations.

---
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

