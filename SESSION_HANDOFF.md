## Current repository state

## 最近接力状态（2026-11-13）

- 当前 live `main` 精确 HEAD：`3a01511a527d265b40472de603bdaa7249c64b1a`（`fix(infosvr): propagate busy daemon failure to PID 1`），parent 为 `aec7b2ca6d75712f6c18a9712d4478c2c3172070`。本轮仅做仓库状态恢复与治理文档同步，没有改生产源码。
- refs 已重新核对：远程开发分支只有 `main`；`refs/tags/toolchain` 指向 `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7`。PR #1 CLOSED / UNMERGED，head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`、base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74`。未创建/移动分支、tag、PR 或 checkpoint ref。
- 当前 SHA 的全部 21 个 push Actions 均逐一匹配精确 `head_sha=3a01511a527d265b40472de603bdaa7249c64b1a`，21/21 completed success、无失败/待运行；所有触发类型均为 push。相关 job steps 与日志已检查。AI handoff validator 报告 root dev log 24/119 条、legacy archive inventory dev-log=8 / handoff=22 / roadmap=18。
- NVRAM bounded boundary 在 exact-head workflow 中通过：4179 userland source files；locked adapter 中 25 处 raw ioctl 引用；adapter 外 raw bypass=0；扫描 kernel 44042 source files，外部 kernel API references=0、writer references=0；检查 board configs=57。root:root 0600 device-node policy、共享 userland transaction lock inode/权限/争用/跨进程/crash-release 回归均 PASS。该结论关闭的是受治理的 userland 绕行与共享库事务边界，不代表 kernel API 或非协作外部进程的全局原子性。
- infosvr 最近两步修正已在当前 SHA 验证：`aec7b2c` 增加停止后进程复查，`3a01511` 保持失败返回为负值，使 PID 1 notification queue 不会把 EBUSY 当作成功确认。精确 run `43464806328` / job `132020959425` 的包长、SIGTERM、stop-before-start、RC failure propagation source contracts 全部通过。
- Crond 当前 SHA exact-head run `43464806051` / job `132020958331` 通过 GNU89/Werror/ASan/UBSan daemon startup/stop/restart policy 与 RC caller contracts。其它当前 SHA workflow（HTTPD auth/security、FRP、AdGuardHome、Adbyby、AliDDNS、AliyunDrive、VPN、ZeroTier、watchdog、GPIO、networkmap、detect_link、WAN ARP、DNS Forwarder、SkipDBv2 等）也全部在 21/21 汇总内成功。
- 本次没有当前 HEAD 的完整 MIPS/uClibc firmware/TRX Actions，也没有 L4 实机刷写/启动结果。旧 XY-C1 artifact 对应祖先 SHA `231fa506b898cf629391a6b6846ee36f3a5f78c4`，不能替代当前 `3a01511` 的 L3 证据。未做完整本地 checkout 或本地固件构建。
- 开发规则继续保持：单一 main、直接 forward commit、CAS 更新 ref、不 force-push/amend、不新建例行 handoff/roadmap archive；根 dev log 当前 24/119 条，未达到轮转阈值，不改 INDEX/archives。

## Active task

下一批优先对 Linux 4.4 NVRAM kernel ioctl 的单值读取并发边界做 L1 审计，确认并修复（若验证成立）`user_nvram_get()` 返回值读取期间的并发修改问题。当前 scanner 已明确提示：`nvram_get(param)` 在 `nvram_lock` 解锁后返回内部值指针，调用方随后才执行 `strlen()` 和 `copy_to_user()`。源码已看到 `_nvram_get()` 返回 `t->value`，`_nvram_set()` / `_nvram_unset()` / `nvram_clear()` 在同一 spinlock 下改变该数据。
- 目前应将其准确描述为潜在的 unlocked read/write consistency race；**现有证据不足以称为 use-after-free**：值字符串来自 `nvram_values` 长期分配的 slab-like buffer，普通 clear 会重置/清零，而非释放该 buffer；tuple node 本身虽会释放，但 reader 使用的是 `t->value`。仍需确认所有重置/卸载生命周期、值长度边界及所有 writer，再决定安全快照实现，不能把 copy_to_user 放进 spinlock 临界区。
- 首先检查 `trunk/linux-4.4.x/drivers/nvram/nvram.c` 与 `nvram_linux.c` 中 `_nvram_get`、`_nvram_realloc`、`_nvram_unset`、`_nvram_uninit`、module exit/open refcount 的完整关系；设计在锁内捕获受限快照、锁外 copy_to_user 的实现和并发回归。只有证明存在实际不一致或危险窗口后才改代码。
- 次级候选：`restart_ftpd()` / `restart_smbd()` 与安全卸载/存储 hotplug 所使用的 storage operation lock 是否存在跨进程竞态。当前只是锁覆盖差异线索，尚未证明实际缺陷；先跟踪通知/CLI/锁顺序，不要预设修复。

## Completed in this handoff

1. 已恢复并核对 live main `3a01511a527d265b40472de603bdaa7249c64b1a`；确认唯一远程 branch、PR #1 与 toolchain tag 状态。
2. 对该 SHA 的 21 个 push Actions 全部逐一匹配精确 SHA，21/21 success；核对了关键 job steps 与日志，不使用 parent SHA 测试代替当前结果。
3. NVRAM root:root 0600 policy、共享 userland transaction lock 和全树边界审计 exact-head PASS；输出 4179/25/0、44042/0/0、57 的准确计数，结论限定在 userland 和扫描器覆盖范围。
4. infosvr EBUSY 失败负返回修正、crond 生命周期 policy 与 RC contracts 在当前 SHA 的专用 Actions PASS。
5. 更新 live handoff/roadmap 以反映最新 CI 与下一轮 kernel NVRAM 并发读取审计；未新增 dev-log 记录、archive 或 INDEX 项，因为本次没有独立生产代码提交且根日志远低于轮转阈值。

## Verification boundary

- Current exact main: `3a01511a527d265b40472de603bdaa7249c64b1a`.
- Exact-head push Actions: 21/21 SUCCESS。关键 runs：infosvr `43464806328` / job `132020959425`；Crond `43464806051` / job `132020958331`；NVRAM boundary `43464805917` / job `132020957236`；HTTPD Authentication/Security `43464806109`；AI handoff docs `43464806107` / job `132020958837`。完整清单可由 exact SHA Actions API 重新取得。
- NVRAM audit log：4179 userland files / adapter raw references 25 / external bypass 0；44042 kernel files / API refs 0 / writer refs 0；57 board configs。两个 NVRAM GNU89/Werror host binaries 及测试步骤全成功。scanner 输出的 `user_nvram_get` unlocked pointer follow-up 仍 OPEN，不因 scanner PASS 自动关闭。
- AI docs exact-head check：root log=24/119，batch=100、retain=20；archive inventory dev-log=8、handoff=22、roadmap=18。没有触发根 dev log 归档。
- L1/L2 evidence only for current changes. Current-head L3 MIPS/uClibc firmware/TRX and L4 router flash/boot remain NOT VERIFIED. No complete local checkout/build and no physical router test.
- No branch/PR/tag/checkpoint mutation; existing PR #1 remains closed/unmerged.

## Exact first action for the next GPT

Re-fetch live `main` and inspect current exact SHA before edits. Then finish the NVRAM kernel read-lifetime audit: inspect the full implementation and callers of `nvram_get()`, `_nvram_get()`, `_nvram_realloc()`, `_nvram_unset()`, `_nvram_uninit()`, module init/exit and ioctl `user_nvram_get()`. Establish whether the unlocked `strlen()` + `copy_to_user()` can observe a concurrently modified/reset value; distinguish stale/inconsistent data from actual freed-memory access. If reproducible, add a bounded snapshot path that copies under the spinlock and performs `copy_to_user()` only after unlocking, with a focused GNU89/kernel-policy regression and exact-head Actions. Do not claim or fix a UAF unless the lifetime proof supports it. After that, investigate (without presuming a bug) the storage-operation-lock coverage around FTP/Samba restart versus safe removal/hotplug.
## Completed task: Adbyby configured hosts-source list / hosts-list download hardening

The end-to-end producer/consumer chain was traced: the ROMFS source template is installed by the Adbyby Makefile, seeded into persistent /etc/storage/adbyby_host.sh by copyscripts.sh, edited by the Adbyby UI textarea through HTTPD persistence, and consumed by the lifecycle-locked runtime on start/update/manual refresh. The former curl -k plus shell-word-split loop and shared /tmp/host.txt output are replaced by quoted per-line HTTPS requests through the shared firmware-CA/HTTP-200/timeout/retry/rate/size policy. Each required source is downloaded into a unique private same-filesystem stage; IPv4/IPv6 address mappings, DNS labels, record and total byte limits are validated; the complete aggregate is deduplicated and atomically published as mode 0644. Any required-source failure preserves a validated last-good cache; without one, no partial file or managed dnsmasq include is enabled. The UI status count follows the published mapping count.

The no-network regression covers URL query/ampersand preservation, multiple-source aggregation/deduplication, missing CA, TLS/redirect/HTTP failures, malformed/invalid/oversized hosts data, aggregate overflow, last-good fallback, atomic rename failure, cold fail-closed behavior, disabled mode and private-stage cleanup. The first exact-source run exposed a fixture-only hard-coded path mismatch in the no-cache cleanup assertion; commit 1994f29e9e8180131011fb811bd9f6402e52523f corrected the fixture to use $PROG_PATH/hosts without changing production behavior. On exact main 1994f29e9e8180131011fb811bd9f6402e52523f, Adbyby [43378151993] / job [131729407848] and AI docs [43378151976] / job [131729407933] passed, as did AliyunDrive [43378151941], AdGuardHome [43378151961] and AliDDNS [43378151950]; all job steps and full logs were inspected. This closes the configured hosts-source slice at L2 host/Actions only. No real external download, full firmware/TRX build or router test was run.

## Completed task: Adbyby anti-AD DNSMasq fragment download and lifecycle hardening

The existing NVRAM anti-AD path was reviewed end-to-end: anti_ad_link defaults to the anti-AD dnsmasq export, the UI exposes the URL and displays anti_ad_count, and the upstream export uses metadata comments plus address=/domain/ records. The implementation reuses the shared firmware-CA HTTPS downloader (HTTPS-only, HTTP 200, bounded retries/time/size/rate), uses umask 077 and a unique same-filesystem private stage, validates the complete file (nonempty address records, no injected dnsmasq directives, 16 MiB total / 512-byte line / 253-byte domain / 63-byte label limits), chmods the publication to 0644 and atomically renames it.

Review found and fixed lifecycle gaps on main: stop/update cleanup previously deleted the anti-AD last-good fragment before a refresh could fall back to it; disabling the NVRAM toggle left the old .conf active; and the wrapper rejected an unsafe destination symlink but did not unlink the link itself. The fixes retain only a verified cache while anti-AD is enabled, remove generated sibling snippets and abandoned private stage directories, remove the anti-AD fragment when disabled, unlink symlink links without touching their referents, and continue generating unrelated Adbyby DNS rules when the optional anti-AD refresh fails. A 64-byte DNS label is explicitly rejected.

Source/test/checker commits: 6fbd0693b39b859cdae9ea4c918c7621815c1ae4 and ea854cd5e7ecdc124f786775fbaf72ace2931235. The first exact-source Adbyby run on 6fbd (43368910069 / job 131699819182) passed shell syntax and all six host regression scripts but failed the semantic checker because a new assertion searched for a function's own name inside its extracted body. The checker assertion was corrected in ea854. Final exact-code-head Adbyby push [43369019121] / job [131700174947] passed every step, all shell regressions and the Python source/workflow checker. Exact-code-head AI docs [43369019133] / job [131700175086] also passed on ea854 (98/119 root entries at that checkpoint). HTTPD security umbrella [43368910153] / job [131699819465] passed all 39 steps on the exact production-source commit 6fbd. All tests used local curl/NVRAM mocks; no real network fetch, full firmware/TRX build or router test was run. This closes anti-AD at L2 host/Actions only; target MIPS/uClibc and L4 router behavior remain NOT VERIFIED.

## Completed task: Adbyby NVRAM-configured third-party custom rule download

On exact main SHA a8681f65733f08b7c932833fafe99062492f0a2a, all six triggered push Actions succeeded and every job/step/full log was inspected. Runs/jobs: Adbyby [43363263383 / 131681514167], infosvr [43363263454 / 131681514465], AI docs [43363263378 / 131681514145], AliDDNS [43363263594 / 131681514640], AliyunDrive [43363263430 / 131681513797], AdGuardHome [43363263365 / 131681513585]. The Adbyby job passed launcher/test shell syntax, hosts preservation, lifecycle serialization, managed HTTPS/MD5/cache rollback, third-party HTTPS URL/size/last-good publication, firewall snapshot and source/workflow semantic checks. AI docs validator reports 97/119 root entries and archive inventory 7/22/18.

Commit 315d738a580e154302dfdd9c57674ee64c3ada36 reuses the common HTTPS downloader for NVRAM user-configured third-party lists, validates and quotes URLs, stages each response privately, checks HTTP status/time/rate/size/CA, preserves the previous regular non-empty user.txt on failure, and atomically publishes the complete combined list before invalidating user.bin. Two intervening checker-only commits aligned its source contracts with centralized URL validation and wrapper-level umask 077; neither altered production behavior. Firmware curl is 8.21.0. This closes the custom third-party rule slice at L2 host/Actions, not L3/L4.

## Completed task: Adbyby lifecycle serialization

On exact main SHA `ceeb24d6b17e03f1777c240bdf0aca9fa6481194`, Adbyby [43346579501] / job [131627282482] passed shell syntax, all host regressions and the semantic caller/workflow checker. The regression covered lifecycle lock inode/permissions, symlink/hardlink rejection, bounded contention, non-interleaved stop/start, stop-failure gating, updateadb dispatch and FD-9 closure. The checker additionally validated managed hosts-entry preservation and the C RC restart caller. This closes the lifecycle serialization L2 task; it does not validate the new download code in THIS COMMIT.

## Completed task: Adbyby managed dnsmasq hosts-entry preservation

On exact main SHA `6fa50e11532ca5c68a9fdb3ffc95a8415841e794`, Adbyby [43336402301] / job [131593559948] passed shell syntax, extracted-helper regression, firewall snapshot/failure-cleanup regressions, and Python caller/path-filter contract. The helper removes only the exact anchored generated `addn-hosts=$PROG_PATH/hosts` line after escaping BRE-special path characters. Operator-owned addn-hosts, hostsdir, conf-file, address directives, comments, backup paths, duplicate managed lines, metacharacter paths, idempotence and missing-config behavior are covered. The other six exact-head workflows also passed: infosvr [43336402343], AliyunDrive [43336402492], AliDDNS [43336402424], AdGuardHome [43336402256], AI docs [43336402234], HTTPD security [43336402226]. All job steps/logs were inspected. This is L2 host/Actions evidence only, not target BusyBox/MIPS or live-router evidence.

## Completed task: FRP asynchronous watchdog restart dispatch

On exact main SHA edd2b4e231819fd748dea032324cd45904ddf172, check_frp() aggregates both enabled frpc/frps states before dispatching the shared restart script once. This prevents a second kill/restart while background daemon PIDs are still invisible. The unreachable WAN-failure log now occurs before its return, and the ping check uses POSIX if/status syntax. The host fixture deliberately keeps both PIDs absent and verifies both/one missing, alive, disabled and WAN-down cases. FRP [43334213696] / job [131586165339] passed shell syntax, lifecycle regression and semantic source/path-filter contract. AI docs [43334213673], infosvr [43334213527], AdGuardHome [43334213750], AliDDNS [43334213640] and AliyunDrive [43334213535] also passed on the same exact SHA; all steps/logs inspected. The preceding fixture version failed before assertions because it sourced the launcher under set -u without a positional argument; the fixture now supplies an explicit source-only argument. This is an L2 host/Actions result, not target BusyBox/MIPS or live daemon readiness evidence.

## Completed task: AliDDNS lifecycle process identity and bounded stop

Traced the HTTPD `variables.c` → PID 1 `rc.c` → `services.c` → `/usr/bin/aliddns.sh` control route. The previous `kill_ps()` selected processes with broad `ps | grep | awk | eval` matching and sent TERM/KILL without confirming exit.

Current implementation:
1. Removes generic `kill_ps()`; enumerate `/proc/[0-9]*/cmdline` and match only the exact launcher path followed by the final `keep` argument. Convert NUL separators to newlines and restore the trailing boundary that command substitution strips. Reject nonnumeric/reserved PIDs and the current shell.
2. Revalidate the exact argv immediately before signalling. Send TERM, allow at most five one-second intervals, send KILL only to still-matching keeper PIDs, then allow at most two more seconds and return failure if the keeper remains.
3. Make stop failure a hard gate: a failed close prevents status mutation or replacement keeper startup. Restart/start/stop/check still run through the stable-inode lifecycle lock and the RC restart remains one atomic locked action.
4. The host regression covers same-path/final-`keep` matching, current-shell exclusion, similar-path and wrong-action decoys, extra argv rejection, graceful TERM, bounded KILL escalation and stubborn-process failure preventing restart. The HTTPD umbrella contract now validates the exact keeper-stop helpers rather than requiring the removed grep-based `kill_ps`.

Intermediate CI failures were investigated, not hidden: the first regression failed because the test fixture used a literal dollar path instead of the test shell PID; the next exposed command substitution removing the trailing newline; the next semantic check itself was corrected to assert the final-argument pattern and literal `$$` current-shell exclusion. Current code/test/checker exact-head run [43306180515] is green. The HTTPD workflow's obsolete `kill_ps "$ALIDDNS_HELPER"` assertion was replaced with checks for `aliddns_stop_keeper`, `aliddns_keeper_pid_matches` and absence of `kill_ps`; HTTPD run [43305744811] is green on the same production source tree.

