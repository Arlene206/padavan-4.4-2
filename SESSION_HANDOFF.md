## Current repository state

## 最近接力状态（2026-11-13）

- 已按根目录 AI_BOOTSTRAP_PROMPT.md、AI_DEVELOPMENT_PROTOCOL.md 的规定顺序恢复；唯一授权开发分支仍为 main。本轮代码与回归检查最终精确 SHA：e9c38f80e74c5d1e80ce6edbe74310e3645be2fe。文档 checkpoint 将是本记录所在提交，下一轮仍须先重新获取 live refs/heads/main，不能把代码 SHA 与文档提交 SHA 混淆。
- 分支清单只有 main；没有 open PR，没有新建 branch、tag、checkpoint ref 或 PR。既有 refs/tags/toolchain 和历史 pull refs 保持不动；GPT Session Rotator 未改动。
- 本轮恢复后优先接续上轮明确未完成的 config_soft_bridges_other() 跨 bridge 批次补偿。源代码提交 de29e51b643d60e829b89027496a0244ad0176fe 实现事务 ownership ledger；后续 17956f0f7938e6430dd2a1a2515eb97f58d0ac42、d304121e7e3f8d1ca854a4da28ad58603a47e7d7、0905bf5af4863ccba54926c8f562744814c48ca2 修正对应的源码契约/Workflow path 检查；e9c38f80e74c5d1e80ce6edbe74310e3645be2fe 将 shared bridge policy include 改为项目 staging include 路径，修复 XY-C1 首次构建暴露的 include 失败。
- 当前批次事务覆盖最多四个 IFNAME_MAC2.<vid> / brwan2..5 条目：跟踪外层与 WAN-side VLAN 是否由本次创建、bridge 是否新建/原已 UP、两端 bridge-port 是否由本次 attach；失败时按逆序撤销 WAN-side VLAN port、WAN port、WAN-side VLAN、外层 VLAN，最后才考虑新 bridge。删除新 bridge 前使用 Linux 4.4 BRCTL_GET_PORT_LIST 查询完整 1024 项 port inventory；inventory 非空或查询失败时保留 bridge（并尝试保持 DOWN）。没有用 br_del_all_ifs() 清桥。策略门控与 GNU89 host test 已增加，覆盖 empty/non-empty/unknown inventory、预存/未知 membership 与删除安全条件。
- de29... 的首次 XY-C1 运行因 net_wan.c 错用相对路径引用共享头文件而失败；未隐瞒该失败。修正后的 e9c38f80... 使用 #include "bridge_membership_policy.h"，依靠现有 shared header staging/include path；随后精确 SHA 的 XY-C1 build、TRX verify、artifact upload 全部 SUCCESS。
- 精确 e9c38f80... 的 11 个 push Actions 全部 SUCCESS：WAN route [43429016219 / job 131897514715]、AI handoff docs [43429016261 / 131897515373]、WAN teardown [43429016457 / 131897515300]、HTTPD security [43429016301 / 131897515968]、WAN DHCP callback [43429016308 / 131897515048]、WAN ARP [43429016193 / 131897514913]、detect_link [43429016366 / 131897515082]、detect_internet [43429016453 / 131897515897]、miniupnpd [43429016528 / 131897515988]、VPN lifecycle [43429016754 / 131897516340]、XY-C1 firmware [43429016318 / 131897515445]。所有这些 run 的 head_sha 都精确等于 e9。
- WAN route Action 编译并运行 GNU89 -Wall -Wextra -Werror / ASan / UBSan bridge policy regression，输出 Bridge membership and teardown policy regression: PASS、WAN VLAN and bridge error-propagation contracts: PASS、WAN route result propagation and cleanup ordering contracts: PASS。
- XY-C1 精确 e9 的 MIPS/uClibc build、TRX 验证、artifact 上传成功，输出 trunk/images/TurBoTse-XY-C1-4.4-mipsel-linux-musl-20261113.trx；artifact ID 14039361485，上传包大小 12,576,910 bytes，上传包 SHA256 581c06a9e8fe011322516cdd3a655f9c658f383b2e1303c3643d6e05b419e839。构建并非零 warning：日志中仍有 parted/libtool 警告、缺少 modules.builtin.modinfo 的 depmod warning，以及空占位 ddnsto / zerotier-one 被 objcopy/strip 报 no sections；它们未阻断 TRX 验证及上传，后续仍可治理。
- 没有完整本地 checkout，因此没有声称本地执行过测试；host policy / source contracts 与 target build 证据全部来自精确 e9 GitHub Actions。L4 实机刷写、启动、WAN/LAN 及多 bridge 运行仍 NOT VERIFIED。
- 根目录 dev_log.md 当前 113 条；本次增加一条后为 114/120，大小低于 384 KiB，不归档、不改 docs/dev-log/INDEX.md。


## Active task

上轮遗留的 config_soft_bridges_other() 跨 bridge 批次回滚已实现并在精确 e9 SHA 的 host Actions 与 XY-C1 MIPS/uClibc build 中验证。当前优先项转为 DHCPv6 callback 子进程生命周期审计；完成该有界网络项后，必须回到 REFACTOR_ROADMAP.md 全仓库治理台账轮转，不能长期局限于 WAN/HTTPD/SkipDB。

1. 已完成：WAN bridge phase 1（config_soft_bridge_wan()）跟踪本次创建的 VLAN/bridge/member 并对失败执行保守逆序补偿；归属不明确时保留对象。
2. 已完成：LAN bridge 单次转换（config_soft_bridge_lan()）跟踪接口/VLAN membership 改动，失败时只撤销本次变更并保留原始 errno；策略及回归测试有 GNU89 + ASan/UBSan Actions 证据。
3. 已完成：config_soft_bridges_other() 多 bridge batch ledger，记录最多四个外层 VLAN、WAN-side VLAN、bridge 和两个 port 的归属；失败后按构造逆序补偿，并在 bridge port inventory 明确 empty 时才删除本次创建的 bridge。不得将本轮描述为完整拓扑全局原子事务。
4. 下一项：审计 trunk/user/rc/src/net_wan6.c DHCPv6 callback 子进程的 PID/generation 关联、spawn 成功与脚本 exit status、wait/reap、stop/restart race、stale child 清理及 errno/status 传播。追踪完整调用链与状态机，先构造可重复 host policy / mock 测试和 source contract，再修复最小稳定性缺陷。特别检查 callback child 与 dhcp6c 主进程 generation 是否严格关联，避免旧 generation 子进程影响新会话。
5. DHCPv6 子进程治理后，从全仓库 priority queue 选择非单一网络模块的稳定性 BUG；优先级继续考虑严重安全问题、进程监督、资源释放、文件原子发布、并发与错误传播。FRP PID/argv identity 与双文件 publication rollback、FTP 上传清理 TOCTOU（保持 BLOCKED，未证明安全前不改）、NFS durability/兼容性、Wi-Fi bridge detach 和其它用户态模块仍需持续审计。


## Verification boundary

- 当前代码 main SHA：e9c38f80e74c5d1e80ce6edbe74310e3645be2fe。精确 SHA 11/11 push Actions 全 SUCCESS；关键 run：WAN route [43429016219 / job 131897514715]、AI handoff [43429016261 / 131897515373]、WAN teardown [43429016457 / 131897515300]、XY-C1 [43429016318 / job 131897515445]，以及 HTTPD、WAN DHCP callback、WAN ARP、detect_link、detect_internet、miniupnpd、VPN 同 SHA 全通过。
- WAN route host job 使用 GNU89 -Wall -Wextra -Werror 和 ASan/UBSan，bridge membership/teardown policy binary PASS，WAN VLAN/bridge error propagation 与 route status/cleanup source contracts PASS。
- 精确 e9 XY-C1 Workflow 成功完成 MIPS/uClibc build、TRX output verification 与 artifact upload：TurBoTse-XY-C1-4.4-mipsel-linux-musl-20261113.trx；artifact ID 14039361485，上传包 12,576,910 bytes。此前 de29 上的第一次 build 因 include 路径失败；e9 修正为 staging include 后精确 e9 build 成功。日志仍有上述非致命 warnings/errors，不能描述成零 warning。
- 当前会话无完整本地 checkout；没有本地测试/编译结果可报告。所有 host sanitizer/source contract 和固件构建证据均来自 exact-head GitHub Actions。无 L4 实机刷写/启动或真实多桥拓扑测试。
- 回滚是保守补偿，不是全局原子事务。仍未完整恢复 IPv6 sysctl side effects、可能部分写入的 brport_set_m2u() 属性、预存 VLAN 的 MAC / egress priority，以及既有 WAN bridge 的 MAC/MTU/STP 等属性。WAN-side bridge setup 现有 IPv6 sysctl 变更也未做完整 snapshot/restore。后续不得把现状表述为“所有接口属性完全回滚”。


## Exact first action for the next GPT

重新获取 live refs/heads/main 并核对 SHA；确认当前文档提交后的 AI handoff contract Actions 精确通过。然后从 trunk/user/rc/src/net_wan6.c 的 DHCPv6 callback child lifecycle 入手：定位所有 spawn/exec、PID/generation 保存、wait/reap、stop/restart 与错误返回调用点，画出父子生命周期状态转移，检查 stale child 和竞态；先写 GNU89 host policy/mock 与 source contracts，再按最小差异修复。通过精确 SHA Actions 后回到全仓库治理台账，选择下一项稳定性 BUG，不要只在 WAN 模块打转。


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

