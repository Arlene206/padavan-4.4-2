### 2026-11-24 最新接力检查点

- 当前产品源码 SHA：6b1c7c42edb393ffa9f38ac35bd62dac66cb8147（FE all-analog teardown gate）。本次只修复 Switch CLI 旧 source-contract 的检查范围，不修改产品源码。
- Parent exact Actions：AI handoff #45285025900 PASS；Switch CLI #45285025962 FAIL，job #138079002575 step 30。旧 FE contract 误将 clear_ckinv_ana_txvos_checked() 和 saved-snapshot restore gates 要求在 orchestrator 函数体内；实际已提取到 ra_switch_fe_analog_cal_cleanup()，属于过时测试契约。
- 本次 contract 改为检查 callback 内的 analog cleanup 和有效 snapshot restore，并继续检查 orchestrator 中的 setup_attempted 与 ra_switch_mdio_cleanup_if_started()。GNU89 host regression 和新 FE gate contract 在 step 30 之后被 skip，尚未验证。
- 产品源码 SHA 6b1c7c42edb393ffa9f38ac35bd62dac66cb8147 的 XY-C1 #45285025914 与 NVRAM #45285025931 上次仍 IN_PROGRESS；应刷新 exact SHA。冻结 backup SHA 72f4f157a084f8184b9597ca5bf334b8080e5807 未动。无本地 MIPS build，无 L3/L4 声明。

---
# Cross-session execution mandate

## Current repository state

### 2026-11-24 continuation — RAETH PHY reliability

- FE VBG/R50/TX-amplitude/TX-offset: checked paged/C22/C45 I/O, bounded search, error propagation, fallback and cleanup.
- FE all-analog orchestrator: this source SHA gates teardown on the first attempted setup write; the original full cleanup is in a dedicated callback.
- GE analog: checked MDIO, bounded search and cleanup; EPHY errors propagate to the network ioctl caller.
- Current CI repair: this commit updates the old FE source contract to inspect the extracted callback.

## Active task

### Validate cleanup-policy regression and exact-source firmware build

- Fetch live main and exact-head Switch CLI + AI handoff Actions for the latest commit.
- Confirm the GNU89 cleanup-policy test and FE gate source contract pass.
- Refresh XY-C1 Build, TRX and artifact on product SHA 6b1c7c42edb393ffa9f38ac35bd62dac66cb8147; all must pass on identical SHA for L3.
- Then continue another concrete RAETH/userland stability issue.

## Verification boundary

- Product source SHA: 6b1c7c42edb393ffa9f38ac35bd62dac66cb8147; this commit changes workflow and continuity docs only.
- Parent exact actions: Switch CLI #45285025962 FAIL (stale inline-cleanup assertion); AI handoff #45285025900 PASS.
- XY-C1 #45285025914 and NVRAM #45285025931 were IN_PROGRESS at last query.
- No local MIPS build, same-SHA completed Build+TRX+artifact L3, device flash or hardware L4 is claimed.

## Exact first action for the next GPT

1. Fetch live main and check exact-head Switch CLI/AI handoff Actions for this workflow correction.
2. Confirm the host callback regression and FE setup-gate source contract execute and pass.
3. Refresh Build/TRX/artifact for source SHA 6b1c7c42edb393ffa9f38ac35bd62dac66cb8147, then continue another concrete stability issue.

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

