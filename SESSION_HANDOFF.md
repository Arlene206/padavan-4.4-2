### 2026-11-24 最新接力检查点

- 产品源码基线：`5157b869cb62a22e04057033ffc5d6e1149a5e6e`（`fix(raeth): report failed calibration ioctls`）；当前 main 上的 `e77a76bc61a790cd6282ac0d0d70b01f081cb4a8` 只修正了 GE wait-context workflow contract 的 forward-declaration 查找。
- 当前 Switch CLI run #45283594271（exact SHA `e77a76b...`）FAIL，job #138074054184，step 28：Leopard EPHY source contract 将范围从 MT7622 分支之后开始，额外包含两个分支状态检查，导致错误的 `goto mdio_error` 计数。生产代码本身的 Leopard 部分有 9 个 checked page writes、1 个 checked page read，十个对应的 MDIO error gates。
- 本次 main commit 将 contract 起点改为 `status = leopard_ephy_cal();` 之后，使检查范围只包含 Leopard 专属本地寄存器初始化；产品源码没有更改。
- Parent SHA `e77a76bc61a790cd6282ac0d0d70b01f081cb4a8`: AI handoff #45283594232 success; Switch CLI #45283594271 FAIL（contract 范围过宽）。产品源码 SHA `5157b869...`: XY-C1 #45283497318 in_progress; NVRAM #45283497339 success。L3 尚未确认。
- 保护分支 `backup/pre-plugin-overhaul-2026-11-14` 保持只读，SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`。没有本地 MIPS/kernel 编译或硬件验证。

---
# Cross-session execution mandate

## Current repository state

### 2026-11-24 continuation — RAETH PHY calibration

- **FE VBG/R50:** checked paged/C22/C45 MDIO, bounded calibration search, timeout fallback and best-effort cleanup are implemented.
- **FE all-analog orchestration:** checked snapshots, setup, valid-snapshot restoration and child completion gates are implemented. Continue review only where register semantics establish a concrete safety issue.
- **GE Rext/R50/TX amplitude/TX offset:** checked I/O and cleanup are in place; TX amplitude and TX offset searches have hard bounds and preserve the known 0 dB fallback.
- **EPHY bring-up:** FE/GE calibration errors propagate through MT7622/Leopard runners into `fe_sw_init()`; GE wait context and retry-failure handling are present.
- **Current CI task:** THIS commit narrows the Leopard EPHY source-contract extraction to the Leopard branch after its batch calibration call; production source is unchanged.

## Active task

### Validate Leopard EPHY branch contract, then continue RAETH runtime/error review

- Fetch live main and query exact-head Switch CLI and AI handoff push runs for the new commit.
- Confirm all earlier RAETH source-contract checks pass after step 28.
- Refresh product-source SHA `5157b869...` XY-C1 run #45283497318; inspect Build/TRX/artifact steps individually and require same SHA for L3.
- After the contract suite passes, continue source review in the PHY ioctl / initialization paths for real status propagation or resource cleanup defects. Do not change common FE orchestrator cleanup without verified hardware register semantics.

## Verification boundary

- Product-source SHA: `5157b869cb62a22e04057033ffc5d6e1149a5e6e`.
- Workflow-only parent `e77a76bc61a790cd6282ac0d0d70b01f081cb4a8`: AI #45283594232 success; Switch CLI #45283594271 FAIL (Leopard checker included the two enclosing branch error gates).
- Product-source runs: XY-C1 #45283497318 in_progress; NVRAM #45283497339 success. These are not actions for the current workflow-only child SHA, but they do build/audit its unchanged source ancestor.
- No local MIPS build, complete same-SHA Build+TRX+artifact L3, router flash or hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live main and verify exact-head Switch CLI + AI handoff Actions.
2. If Switch CLI passes, resume broad RAETH source review and choose a confirmed defect for a source+regression atomic commit.
3. Refresh XY-C1 Build/TRX/artifact on product-source SHA `5157b869...`; keep host/source-contract checks separate from L3.

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
