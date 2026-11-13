## Current repository state

## 最近接力状态（2026-11-14）

- 最新 main exact SHA：`691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`，直接父提交 `ed010248a2d1bb073d7c6aa28c3bd9afcda3d07f`；其父提交为 `eb24238333a4424ae6cfb0f807b6ce14fc9223dc`。本次恢复期间 main 已由两个普通 fast-forward 提交推进：`ed01024` 修复 LAN restart 的 DNSMasq 错误传播，`691e4d6` 对齐 HTTPD umbrella 的 source contract。远程仅有 main；PR #1 仍 CLOSED / UNMERGED（head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`），toolchain tag 仍为 `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7`。
- `ed010248` 的生产修复：`full_restart_lan()` 改为返回 DNSMasq lifecycle status；锁获取失败返回 errno，stop 失败时禁止启动第二实例，start 失败保留原始状态，锁释放失败在没有先前错误时也向上传播；PID 1 的 `RCN_RESTART_LAN` 分支通过 `rc_notify_merge_action_result()` 合并结果，避免 DNSMasq 故障被当作 LAN restart 成功而清除 incomplete marker。
- `691e4d6` 只修改 `.github/workflows/httpd-security.yml`：HTTPD umbrella 将 `rc.c` 纳入 push / pull_request path filters，并将 LAN restart source contract 对齐到 `dnsmasq_status`、`int full_restart_lan()` 及 PID 1 result merge。该提交没有改动生产源码。
- exact `691e4d6` push Actions 共 3 条：HTTPD Authentication and Security Regression [run `43489646217` / job `132107102491`] SUCCESS；AI docs [run `43489646340` / job `132107103804`] SUCCESS；XY-C1 Firmware Validation [run `43489646254` / job `132107102854`] 在最近一次检查时仍处于 Build XY-C1 / IN_PROGRESS，不能宣称 L3 已完成。
- exact 生产源码提交 `ed010248` 的 DNSMasq lifecycle workflow [run `43489264442` / job `132105772776`] 全步骤 SUCCESS：GNU89/Werror restart policy、DHCP range、跨进程 lock、private atomic Inadyn publication 及 daemon postcondition / RC result propagation source contract 均 PASS。NVRAM transaction audit [run `43489264608` / job `132105772928`] SUCCESS，日志报告 `RAW_USERLAND_BYPASS_COUNT: 0`，kernel ioctl / caller contract PASS。上述生产源码在 `691e4d6` 中未改变。
- 已调查 exact `ed010248` 的唯一失败 workflow：HTTPD Authentication and Security Regression [run `43489264377` / job `132105772925`] 在 “Verify DNSMasq generated file trust boundaries” 失败。完整日志显示这是旧 source contract 仍断言 `dnsmasq_stop_status`，而生产函数已改为 `dnsmasq_status`；不是实际 C 编译或运行回归。提交 `691e4d6` 修正断言与路径过滤，exact `691e4d6` 的 HTTPD workflow 已全步骤 SUCCESS。失败有记录，没有隐藏。
- root `dev_log.md` 当前更新为 33/119 条；archive counts 仍为 dev-log=8 / handoff=22 / roadmap=18。距离 120 条及 384 KiB early-rotation 门槛仍有余量；不轮转、不改 INDEX，不创建 handoff / roadmap snapshot。
- 当前 exact main 的 XY-C1 build 仍待复核结果；此前更早 SHA 的构建不能替代当前 SHA 的 L3 证据。L4 router flash/boot 仍 NOT VERIFIED。FTP final-name inode-check/unlinkat TOCTOU 仍 OPEN/BLOCKED；NFS AUTH_SYS/root-squash/client durability 仍需要真实客户端与存储设备证据。

## Active task

继续 DNSMasq 主体生命周期和全仓用户态治理，不重复本次已经落地的 LAN restart 错误传播。当前已确认的基本锁结构：public `start_dns_dhcpd()` / `stop_dns_dhcpd()` / `restart_dhcpd()` / `restart_dns()` wrapper 获取统一 lifecycle lock 后调用 locked helper；`full_restart_lan()` 自己持锁并调用 `stop_dns_dhcpd_locked()` / `start_dns_dhcpd_locked()`，避免重复获取同一把锁。已追踪 `restart_dhcpd_locked()` 的配置预备、旧进程 stop postcondition、disabled stop-only 和 launch readiness 检查；LAN restart 的 DNSMasq stop/start/lock 错误现在会返回给 PID 1 notification accounting。

下一步先完成 public / locked API 的全量 caller map（包括 `services.c`、`init.c`、HTTPD / RC 通知和任何脚本入口），确认无遗漏的 unlocked call、意外嵌套锁或跨边界生命周期交错；再追踪 `pids("dnsmasq")` 的进程身份准确性、stale PID / 同名进程误匹配、`kill_services()` 的 signal/wait/reap 行为、stop 与 HUP reload 的 postcondition、子进程/FD 清理、临时文件失败清理及配置 rollback。每个疑点必须有具体调用路径和可证伪场景后才改代码并加最小回归。

- 不重复已经完成的 DNSMasq DHCP pool/router exclusion、resolver publication lock fail-closed、public/locked lifecycle lock、daemon launch readiness、restart failed-stop gate、DNS/NMBD notification positive-errno normalization，以及本次 `RCN_RESTART_LAN` 错误结果传播。
- notification handler 仍只将负 `action_result` 作为保留 marker 条件；新服务若返回正 errno，必须按服务自身契约先归一化。不得把 infosvr/crond 的 disabled/no-op 正返回全局视为失败。
- DNSMasq 本轮进一步审计未穷尽之前，不把现有 host/source-contract PASS 说成 live lease allocation、目标 BusyBox 运行或实机 DNS ready 的证据。
- 完成该模块的 caller/identity/reap 深审后，扩展到其他用户态治理台账中的高价值稳定性 / 安全问题，不长期局限 DNSMasq / HTTPD。
- FTP final-name cleanup inode-check/unlinkat TOCTOU 继续 OPEN/BLOCKED；Linux 4.4 无 inode-conditional unlink-by-fd，未证明竞态安全前不改。NFS AUTH_SYS/root-squash/durability 继续等待真实 client/device evidence。

## Completed in this handoff

1. 按 bootstrap / development protocol、handoff、roadmap、dev_log 的规定顺序恢复项目，并核对 live main、PR、tag 与 exact-head Actions。
2. 复核 DNSMasq public wrapper / locked helper 的当前分层，检查 full LAN restart 的锁范围、stop-before-start gate、启动后置条件，以及 restart 错误是否被 PID 1 notification marker 正确保留。
3. 在恢复期间发现 main 上已有并验证的并发提交：`ed010248` 实现 `full_restart_lan()` 的 DNSMasq 错误返回及 `RCN_RESTART_LAN` 错误合并；`691e4d6` 修正 HTTPD umbrella 中随生产代码变更而过期的 source contract。两项提交由恢复前已并发运行的开发流程写入，本轮没有重复提交相同修改。
4. 检查 exact `ed010248` DNSMasq 与 NVRAM workflow 的完整步骤和日志，二者 SUCCESS；进一步调查该 SHA 上 HTTPD security workflow 的失败根因，确认是旧变量名 source assertion，并检查 `691e4d6` 上 HTTPD 全步骤成功。
5. 更新 live handoff、roadmap 和 root dev_log 的当前 SHA / Actions / 未完成任务；archive indexes 未达到轮转条件，不制造归档快照。

## Verification boundary

- 生产代码提交：`ed010248a2d1bb073d7c6aa28c3bd9afcda3d07f`（parent `eb24238333a4424ae6cfb0f807b6ce14fc9223dc`），message `fix(rc): retain DNSMasq failures across LAN restart`。它将 `full_restart_lan()` 返回值改为 int，并将 LAN restart 中 DNSMasq stop/start/lifecycle-lock 的失败结果合并进 PID 1 marker 状态。
- 当前 main：`691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`（parent `ed010248a2d1bb073d7c6aa28c3bd9afcda3d07f`），message `test(rc): align DNSMasq LAN restart source contract`。该提交只修改 `.github/workflows/httpd-security.yml`，同步 push/pull_request paths 并修复 stale source assertions；生产源码与 ed010248 相同。
- DNSMasq exact production-source workflow `43489264442 / 132105772776` on `ed010248`: all steps SUCCESS，含 GNU89/Werror policy binaries 与 full LAN restart / RC notification source contract。NVRAM workflow `43489264608 / 132105772928` on `ed010248`: SUCCESS；raw userland bypass count 0，kernel ioctl / caller contract PASS。
- HTTPD umbrella 在 `ed010248` 的 workflow `43489264377 / 132105772925` 曾失败；完整日志将失败定位为旧断言仍使用 `dnsmasq_stop_status`。`691e4d6` 对齐 checker 后，exact-main HTTPD run `43489646217 / 132107102491` 全步骤 SUCCESS，所有被跳过的后续安全契约步骤也已执行通过。
- exact-main `691e4d6` AI docs run `43489646340 / 132107103804` SUCCESS，但它验证的是本次 handoff/roadmap/dev_log 修改前的内容；本次文档提交后必须检查新触发的 docs run。
- exact-main `691e4d6` XY-C1 run `43489646254 / 132107102854` 在最后检查时处于 Build XY-C1 / IN_PROGRESS；TRX verify 与 artifact upload 尚 pending。当前 L3 不能写 PASS；L4 实机刷写 / 启动未执行。
- 本环境未运行本地目标交叉编译；本轮 host regression、GNU89 编译及源契约证据来自上述 GitHub Actions。不得将 host mocks 当作路由器实测。
- 本次 root dev_log 将从 32 增至 33 条；index/archive counts 未改变。FTP TOCTOU 和 NFS client semantics 保持 OPEN/BLOCKED。

## Exact first action for the next GPT

1. 重新获取 `refs/heads/main`，确认当前 exact SHA 与 `event=push` 的全部 Actions；首先复核 XY-C1 run `43489646254` 的最终状态、TRX verification 与 artifact digest，并确认是否有比 `691e4d6` 更新的提交。
2. 完成 DNSMasq 的完整 caller graph：对 `start_dns_dhcpd()`、`stop_dns_dhcpd()`、`restart_dhcpd()`、`restart_dns()`、locked helper 与 `full_restart_lan()` 做全仓检索，记录每个调用者、持锁状态及入口来源；随后检查 `pids()` 进程匹配与 `kill_services()` wait/reap 细节。
3. 仅在确认具体生产缺陷后，添加 GNU89/Werror / host fixture / exact source contract，按 protocol 在 main 上做普通 fast-forward；按需更新 live handoff、roadmap 和 dev_log。不得再重复本次 full-LAN failure propagation 或 HTTPD stale-contract 修复。
4. FTP upload final-name TOCTOU 继续 OPEN/BLOCKED；NFS AUTH_SYS/root-squash/durability 等待真实 client/device evidence。没有 exact-source MIPSel/TRX 与真实设备记录时，不写 L3/L4 PASS。

## Completed task: DNS Forwarder lifecycle serialization and failure propagation
## Completed task: DNS Forwarder lifecycle serialization and failure propagation

The former DNS Forwarder path could interleave concurrent C-level stop/start calls and shell operations. The launcher now owns one stable persistent lock for the full start/stop/restart transaction, validates the lock directory/file owner and modes and rejects symlink/hardlink lock paths, then uses BusyBox flock. Start reads NVRAM only after lock acquisition, is idempotent and verifies daemon presence; stop performs a bounded wait and fails while the process remains; restart never starts a replacement after failed stop. FD 9 is explicitly closed for the daemon launcher, preserving the lock only in the supervising shell. Nonzero start-stop-daemon status is propagated.

The RC service APIs now return launcher status; restart crosses one script boundary so stop+conditional start share the same lock. The PID 1 notification path merges restart failure status rather than discarding it, so a failed restart retains its incomplete marker. Regression covers initial/idempotent start, stop/idempotent stop, kill failure, daemon surviving the bounded wait, successful restart, disabled mode, launcher failure status, success-without-daemon, held lock contention, FD inheritance, and symlink/hardlink/unsafe-directory rejection. Exact 479f0d7 DNS Forwarder Actions pass. The first exact-source run caught a fixture defect (zero-byte fake daemon marker vs a pidof stub requiring a nonempty marker); this was corrected explicitly, not ignored.

---
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

