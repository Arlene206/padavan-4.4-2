## Current repository state

## 最近接力状态（2026-11-13）

- 已按根目录 AI_BOOTSTRAP_PROMPT.md → AI_DEVELOPMENT_PROTOCOL.md → SESSION_HANDOFF.md → REFACTOR_ROADMAP.md → dev_log.md → live refs/Actions 的规定顺序恢复；唯一授权开发分支为 main。本轮代码与回归最终精确 SHA：d3e8edc792a1001d475e4e1fcbe6e06496399efd。本次文档 checkpoint 是后续提交，不能把源码 SHA 与文档提交 SHA 混淆。
- 远程 refs 检查：只有 refs/heads/main；另有未合并的历史 refs/pull/1/head 和既有 refs/tags/toolchain，均未改动。PR #1 已 closed / unmerged；没有新增分支、tag、checkpoint ref 或 PR。GPT Session Rotator 及其 checkpoint 9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74 未改动。
- 恢复时 live main 为 be42800f99d4f6d1d187f9aea235f9e71eb8ae4e。该 SHA 的 DDNSTO 生命周期与 AI handoff docs 两个 push Actions 均 SUCCESS（43437951651 / job 131927812862、43437951636 / job 131927812714）。
- 上一轮明确遗留的 DHCPv6 callback child lifecycle 已完成源码审计：追踪 net_wan6.c 的 callback session lock / generation 校验、接口约束、start 配置 generation 发布与 exec 状态传播，以及 stop 先推进 generation 再停止主进程的顺序；BusyBox DHCPv6 script 通过同步 spawn_and_wait() 等待并回收子进程，传播脚本退出状态与 exec errno，dhcp6c.c 记录失败。现有实现的 stale callback fence 和 bounded lock 有覆盖；本次未发现有证据支持再改动的独立生命周期缺陷，因此没有为制造提交而改 DHCPv6 代码。之前对应 callback policy / source contract 在精确 a0e3ea8e45d305cba0a38557096a560936cd04ae 的 targeted Actions 有通过记录；本轮审计基于当前 be428... 源码。
- 按 handoff 下一优先项转入 FRP。确认 frp_process_matches() 以前只比较可伪造的 /proc/PID/cmdline argv 元组；一个用其他程序执行但伪装成 frpc -c /tmp/frp/myfrpc.ini 的本地进程可能被误认并收到 root 发出的 TERM/KILL。修复在 trunk/user/frp/frp.sh 增加固定执行文件映射，并在信号处理前同时核对内核提供的 /proc/PID/exe 指向 /usr/bin/frpc 或 /usr/bin/frps，保留原有完整 argv 检查和信号前二次复核。
- 新增/扩展 FRP 生命周期与 stop 回归，构造 argv 完全匹配但 exe 指向 /bin/sleep 的伪冒进程，证明它不算 FRP daemon、不应被停止；正常 FRP 测试进程显式构造对应 exe 链接。Python source checker 现在同时检查固定可执行文件映射、/proc/PID/exe 和伪冒测试。
- 本轮源码提交链：ea9b86690ec5276e04253a7b57dd7bf022b69a4f（生产身份校验 + 初始回归）、81a2d0471891a4480ac68c02387956f6511051d6（修正 test fixture 的幂等 symlink 与 /proc/204 测试目录）、d3e8edc792a1001d475e4e1fcbe6e06496399efd（修复 Python checker tuple 语法）。前两次精确 SHA 的 FRP Action 失败原因均已从 job logs 确认并在后续提交修复，没有隐瞒：第一次 fixture 重复创建 symlink 且缺少 /proc/204 目录；第二次 shell tests 全通过，但 checker 因插入的 tuple marker 缩进错误而失败。最终精确 SHA 的 FRP 与 AI docs Actions 均 SUCCESS。
- 精确 d3e8edc... FRP run [43440754101 / job 131937424207] 成功：shell syntax、watchdog 合并重启、有界启动 readiness、crontab 原子管理、两种角色 boot、私有配置/成对 rename rollback（含第二 rename 失败、EXIT 中断与失败恢复保留备份）、TERM/KILL stop、锁竞争/FD 9、Python source/workflow contracts 全 PASS。精确 SHA AI handoff docs [43440754169 / job 131937424241] SUCCESS。
- FRP 成对配置回滚与异步 readiness 并非本轮新增，而是此前已有实现；本轮确认其现有测试仍通过。Linux 4.4 没有 pidfd，/proc/PID/exe 与 argv 的最后一次复核到 kill(2) 之间仍存在不可完全消除的 PID reuse 窄窗口；当前实现仅能收紧身份判断并缩小竞态，不应宣称彻底消除 PID reuse。
- 当前没有完整本地 checkout，因此没有声称本地运行测试。L3 MIPS/uClibc 当前 main、完整固件/TRX 均 NOT VERIFIED；之前祖先 SHA e9c38f80e74c5d1e80ce6edbe74310e3645be2fe 的 XY-C1 TRX 成功不能代表当前 SHA。L4 实机刷写/启动及服务运行仍 NOT VERIFIED。
- 根目录 dev_log.md 当前 114 条记录，本次新增后为 115/119；当前文件 283325 bytes，新增后仍低于 384 KiB 轮转阈值，不归档、不改 docs/dev-log/INDEX.md。

## Active task

上一轮遗留的 DHCPv6 callback child lifecycle 已完成审计；没有证据支持为此再引入补丁。FRP 本轮补齐了实际可执行文件身份验证，并以 exact-head GitHub Actions 验证。当前 FRP 生命周期的 bounded readiness、两配置发布 rollback 与 launcher flock 已有测试覆盖，但 PID reuse 窄窗口及目标 BusyBox/procfs 语义仍需要目标环境证据。

接下来从 REFACTOR_ROADMAP.md 的全仓库治理台账轮转，不要继续机械深挖 FRP/WAN。FTP failed-upload cleanup TOCTOU 保持 OPEN/BLOCKED：Linux 4.4 没有 inode-conditional unlink-by-fd，未证明竞态安全前不改。NFS async durability / AUTH_SYS/root-squash 需要真实客户端证据。下一轮优先扫描尚未充分覆盖的服务启动/停止监督、crond、存储与 DNS/解析器路径，从可复现、可在主机侧证明的稳定性 BUG 中选一个有界修复点；严重安全漏洞或系统故障可以打断该轮转顺序。

1. 已完成：DHCPv6 callback session generation / interface 校验、同步 spawn/wait/reap 和退出状态/errno 传播的源码调用链审计；本轮未发现新的可确认缺陷，未做无必要改动。
2. 已完成：FRP process identity 同时校验 kernel /proc/PID/exe 固定路径与完整 argv tuple，并在 signal 前复核；新增 argv spoof 与 stop decoy regression，精确 d3e Actions 通过。
3. 已有覆盖：FRP 异步 daemon bounded readiness 与 client/server 成对 config publication rollback（包括失败注入、EXIT 回滚和备份保留）；本轮核对现有实现和 regression，不把它们误记成本轮新增代码。
4. 保持约束：FTP 上传清理 TOCTOU 未证明安全前不更改；NFS durability / AUTH_SYS / root-squash 需要真实 client evidence；不将 ancestor build/artifact 当成当前 main 的 L3 证据。

## Verification boundary

- 代码 main SHA d3e8edc792a1001d475e4e1fcbe6e06496399efd 的 FRP Action [43440754101 / job 131937424207] 与 AI docs Action [43440754169 / job 131937424241] 均 completed SUCCESS；两者 head_sha 均精确匹配 d3e。
- FRP host regressions 明确 PASS：FRP watchdog detects exact-config daemon state and dispatches one replacement、FRP asynchronous daemon startup uses bounded readiness and fail-closed cleanup、FRP managed watchdog crontab preserves operator jobs and publishes atomically、boot client-only/server-only/combined、private runtime/config 与 paired second-rename rollback / EXIT restoration、bounded TERM/KILL、stable lock / contention / FD 9，以及 FRP asynchronous restart, private credential storage and workflow contracts。测试由 GitHub Actions 运行，不是本地运行或目标设备验证。
- 前两个 FRP exact-code-head runs 的失败和最终修复记录见上文；不得把它们隐藏，最终绿色结果仅归属于 d3e。
- 当前 SHA 无 MIPS/uClibc 完整固件/TRX build；祖先 e9 的 XY-C1 build/artifact 只作为历史证据。没有 L4 路由器刷写、启动、WAN/LAN 或真实进程 stop/start 测试。
- Linux 4.4 PID reuse race 不能靠单纯的 procfs 读后 kill 完全消除；FRP identity 仍应保留 signal 前 revalidation，后续如要进一步收敛必须考虑目标内核能力与不误杀保障。
- 之前 WAN bridge rollback 的限制仍有效：它是保守补偿，不是全局原子事务；IPv6 sysctl side effects、可能部分写入的 M2U 属性、预存 VLAN MAC/egress priority、既有 WAN bridge MAC/MTU/STP 等属性未完整 snapshot/restore。

## Exact first action for the next GPT

重新获取 live refs/heads/main、git/refs、精确 HEAD 和最新 Actions；核对本 handoff 文档提交 SHA 对应的 AI docs workflow。然后打开 REFACTOR_ROADMAP.md 全仓库 Priority queue，从 FRP/WAN 之外找一个可复现的 userland 稳定性 BUG，先追调用链和状态，再写 host regression/source contract 并做最小代码修复。FTP TOCTOU 未能证明 inode 安全 unlink 前保持 BLOCKED；NFS 相关判断需要真实客户端证据。只在完成一个有界批次后再考虑下一个模块。

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

