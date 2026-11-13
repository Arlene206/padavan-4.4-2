## Current repository state

## 最近接力状态（2026-11-13）

- 已按根目录 `AI_BOOTSTRAP_PROMPT.md`、`AI_DEVELOPMENT_PROTOCOL.md` 的规定顺序恢复。唯一授权分支仍为 `main`；本轮恢复基线为 `98079bdb24d4191318ee95ca966835a8a723be95`。
- 基线 `98079bdb...` 的 15 个 push Actions 全部以该 SHA 完成 SUCCESS，包含 WAN route/VLAN、WAN teardown、DHCP callback、AI handoff 和 XY-C1。该 SHA 的 XY-C1 run [43414346419] / job [131848343935] 成功生成并上传非空 TRX `TurBoTse-XY-C1-4.4-mipsel-linux-musl-20261113.trx`。
- 本轮在基线之后形成四个连续 main 提交：`ea38b3efc0de62580feed33bf589e1f989fb69a1`（WAN soft-bridge 回滚初版）、`36c7a7984d2c2ca57f1f5169860faddc723d55f5`（所有权策略回归测试）、`fcac7ee169e99393cd3864805dedb76e70ab133f`（修复 WAN/Wireless 源码契约）、`44b0353cd3fc7bfd582e055cc768373819c6c0d2`（使最终 bridge 激活失败也进入回滚，并整理 Linux 4.4 race 注释）。
- `36c7...` 的初次精确 SHA 检查曾因新 helper 名称未同步到两个静态源码契约而失败；桥接策略 ASan/UBSan 测试本身已 PASS。随后 `fcac...` 更新 WAN 与 Wireless 契约，`44b...` 又补上最终 `ifconfig(...IFUP...)` 的回滚入口。不要把中间失败记成最新 HEAD 失败，也不要隐去该修复过程。
- 当前源码检查点：`44b0353cd3fc7bfd582e055cc768373819c6c0d2`。该 SHA 的 12 个 push workflow 中，11 个已经完成 SUCCESS；唯一仍在运行的是 XY-C1 Firmware Validation [43419600758] / job [131865973911]。WAN route [43419600585]、Wireless [43419600810]、WAN teardown [43419600755]、WAN DHCP callback [43419600832]、HTTPD security [43419600791]、AI handoff [43419600775] 等均以完全相同的 `head_sha=44b0353...` 成功。XY-C1 仍须在下一轮核验结束状态和 TRX 输出，当前不可宣称最新 HEAD 已有 L3 成功。
- 本轮没有创建分支或 PR，没有修改 GPT Session Rotator，也没有触碰固件产物。

## Active task

继续完成 WAN/LAN 软件桥部分失败的资源回滚，但不要把局部事务误报成整个网络拓扑的原子事务。

1. 已完成阶段一：`config_soft_bridge_wan()` 引入调用方所有权跟踪。新增 `create_vlan_iface_tracked()`，仅在该次调用真正创建且完整配置成功后报告 VLAN 所有权；新增 `br_add_del_if_tracked()`，仅在本次 bridge ioctl 真正改变 membership 时报告所有权，既有 membership 与并发幂等结果都不会被当成自己创建。失败后按反向顺序撤销本次加入的端口；只有确认安全才删除本次 VLAN 和新建 bridge；尽可能恢复原先已存在 bridge/VLAN 的 UP/DOWN 状态；不确定并发归属时保留对象而不是误删。最终 bridge 激活失败现已统一进入 rollback。策略单测已加入 WAN route workflow 的 GNU89 + ASan/UBSan 步骤。
2. 下一项：修复 `config_soft_bridge_lan()` 的补偿路径。重点是先创建 `eth2.1` 后从 `br0` 摘除物理 `eth2`、再挂入 VLAN 的交接窗口；如果后续加入、移除 VLAN 或 multicast 参数设置失败，需恢复原始 membership 和接口状态，仅撤销本次新建对象，保留既有/并发成员。
3. 随后处理 `config_soft_bridges_other()` 的批次回滚：某个后续 LAN port VLAN/bridge 创建失败时，按逆序撤销本轮之前成功创建的 `brwan2..5`、`eth3.<vid>` 与对应 WAN VLAN/端口状态；必须跟踪每个对象归属，不能用全桥清空函数误删已有成员。
4. WAN bridge 两阶段完成后，审计 `net_wan6.c` DHCPv6 callback 子进程监督、PID/代际关联、停止竞态与陈旧 child 清理。然后回到全仓库治理台账，扩大服务生命周期、资源释放、安全配置和错误传播覆盖，避免长期困在网络单模块。

## Verification boundary

- 最新代码 SHA 是 `44b0353cd3fc7bfd582e055cc768373819c6c0d2`。该 SHA 的 12 个 push Actions 中 11 个 SUCCESS；XY-C1 run [43419600758] 尚处于 `Build XY-C1`，TRX 验证与上传步骤仍 pending。请不要把先前 `98079bdb...` 的 L3 成功套用到这次修改上。
- 最新 WAN route run [43419600585] 的桥接所有权策略 sanitizer 用例输出 `Bridge membership and teardown policy regression: PASS`，源码检查输出 `WAN VLAN and bridge error-propagation contracts: PASS`；Wireless run [43419600810] 的桥接策略共享回归和无线状态契约均通过。WAN DHCP callback、WAN teardown、HTTPD 安全、VPN、ARP socket、detect_link / detect_internet、miniupnpd、AI handoff 均为精确 SHA 成功。
- 中间提交 `36c7...` 的 WAN/Wireless 源码契约失败已在后续 main 提交修复；当前精确 SHA 的对应两个 workflow 已通过。检查 run 的 `head_sha`，不要只看默认分支上不匹配 SHA 的绿勾。
- 当前会话没有可用的完整本地 checkout，故没有声称本地主机侧测试执行。Host sanitizer / 静态源码验证来自上述 GitHub Actions。最新 `44b...` 的 XY-C1 MIPS/uClibc 构建仍待结果；L4 实机刷写、启动、LAN/WAN/多桥拓扑测试均 NOT VERIFIED。
- `config_soft_bridge_wan()` 当前回滚范围是新建对象与本次新增 membership，并尽量恢复接口 UP/DOWN；对预先存在 bridge 的 MAC/MTU/STP 等参数及 IPv6 辅助副作用尚未实现全面恢复。并发归属模糊时有意保留对象并记录日志，这是保守策略，不应误报为完全原子回滚。

## Exact first action for the next GPT

先获取 live `refs/heads/main` 并记录文档检查点提交后的精确 SHA；检查该 SHA 的 AI handoff documentation contract 和所有被触发 Actions，尤其核对本轮 XY-C1 run [43419600758] 是否已 SUCCESS、`Verify TRX output` 是否成功以及 artifact 是否上传。若 XY-C1 失败，先读该 job 最早的因果错误并修复，不得用旧 SHA 的固件结果替代。随后从 `config_soft_bridge_lan()` 开始，沿用本轮 tracked VLAN / bridge membership API 设计补偿回滚，再做 `config_soft_bridges_other()` 的跨 bridge 批次回滚；每轮源码修改后同步最小必要测试和 workflow 路径，精确 SHA 验证通过后再继续 DHCPv6 child lifecycle 及全仓库优先队列。

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

