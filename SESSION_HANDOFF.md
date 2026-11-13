## Current repository state

## 最近接力状态（2026-11-14）

- 最新 main exact SHA：479f0d7a298f6980046a85c60a62271fc830893c，直接父提交 5be7a356957d47a1387346854966f3f19c8fde01；上一代码提交 5be7a35 的父提交为 47c544fe98511bfaa5f5bb66d047272748e1b021。本轮只对 main 做了两次普通 fast-forward；远程仅有 main 分支，PR #1 仍 CLOSED / UNMERGED（head ba48e22609e0c81ad757b06756fe91d05f39cbf6），toolchain tag 仍为 7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7。
- exact 479f0d7 的 push Actions 共 3 条：DNS Forwarder [run 43487234595 / job 132098791466] COMPLETED / SUCCESS；AI docs [43487234617 / 132098791118] COMPLETED / SUCCESS；NVRAM [43487234555 / 132098790943] 在本次记录时仍 IN_PROGRESS，前 8 个步骤成功，最后的全仓 kernel/userland API 扫描尚未结束。未将 pending job 误报为完成。
- DNS Forwarder lifecycle 在 exact 479f0d7 达到 L2：持久 mode-0600 锁文件、锁目录 owner/权限及 symlink/hardlink 检查、跨进程 flock、全生命周期锁、FD 9 不泄漏到 daemon；NVRAM 配置在锁内读取；start 幂等且有 daemon 出现 postcondition；stop 有有界等待与退出 postcondition；restart 在 stop 未完成时禁止拉起替代实例；disabled restart 只 stop、不 start；launcher 返回码保留。三个 RC wrapper 改为返回 int，restart 由单次脚本事务执行；PID 1 restart notification 合并服务返回状态，失败时保留队列 marker。DNS Forwarder job 完整的 CWD/source contract 和 shell lifecycle regression 均 PASS。
- 第一次 exact 5be7a35 的 37 条 push workflows 中 36 SUCCESS、1 FAILURE：DNS Forwarder 的源契约步骤 PASS，生命周期 test 因 fixture 缺陷失败——fake pidof 以文件非空代表 daemon，但 start-stop-daemon stub 和两处种子状态创建了零字节文件。未隐藏失败；479f0d7 将 fake PID marker 改为非空 123，并增加 checker 合同。复跑 479f0d7 的 DNS Forwarder run 43487234595 全步骤 SUCCESS。该失败是回归夹具自身不一致，不是生产 daemon 的失败证据。
- 479f0d7 的 docs validator run 43487234617 检查了本次代码前的 handoff 状态，报告 root log 31/119、archives dev-log=8 / handoff=22 / roadmap=18；本次增加一条 dev_log 后为 32/119，仍远低于 120 条与 384 KiB early-rotation 门槛，不轮转、不改 INDEX。
- XY-C1 MIPSel 完整构建在前一代码 SHA 5be7a356957d47a1387346854966f3f19c8fde01 成功：run 43485625995 / job 132093173668，Build XY-C1、TRX verification、artifact upload 全部 PASS；artifact 14067278386，12,580,062 bytes，SHA-256 0e0cf60b7be3b653d64b23a2554a5efcd3dab9255676673dcb71aa10d90c20cc。该构建早于 479f0d7 对 launcher 返回码的生产源码修改；因此它是 5be7a35 的 L3 证据，不得声称为当前 exact main 的 L3。当前 L4 router flash/boot 仍 NOT VERIFIED。
- FTP upload final-name inode-check/unlinkat TOCTOU 仍 OPEN/BLOCKED；NFS AUTH_SYS/root-squash/client durability 仍需要真实客户端与存储设备证据。

## Active task

DNS Forwarder 的启动/停止/重启事务及失败传播已在 exact 479f0d7 完成 L2；下一批继续 DNSMasq 主体生命周期审计，扩大到整个用户态治理覆盖范围。优先建立完整 public / locked caller map，核对 start_dns_dhcpd() / stop_dns_dhcpd() / restart_dhcpd() / restart_dns() / full_restart_lan() 的锁边界、嵌套锁及 full LAN restart 顺序，再审查 stale PID / process identity、daemon readiness、stop postcondition、child wait/reap、fd 和临时文件清理、配置回滚与所有 notification result contract。对发现的具体缺陷再添加最小回归并修复；不要重复已经完成的 DNS Forwarder 锁、failed-stop gate、launcher result 或 DNS/NMBD notification normalization。

- handle_notifications() 当前只将 action_result < 0 视为失败；新服务调用如果返回正 errno，必须按该 API 契约归一化后合并。已有 DNSMasq/NMBD notification helper 的正 errno / 负错误处理不要重复实现。
- restart_infosvr() / restart_crond() 对 disabled/no-op 有正返回值语义；不得全局将 notification handler 所有非零值机械地视为失败。先追踪每个 service API 的结果契约。
- 组合通知（例如 WINS）应尽可能完成后续独立收敛动作，同时保留前序失败结果；不得因早期错误意外跳过后续操作。
- FTP final-name cleanup inode-check/unlinkat TOCTOU 继续 OPEN/BLOCKED；没有证明 Linux 4.4 下 race-safe 前不得改动。NFS AUTH_SYS/root-squash/durability 继续等待真实 client/device evidence。

## Completed in this handoff

1. 严格按 bootstrap / development protocol、handoff、roadmap、dev_log 顺序恢复；核对 exact main、refs、PR/tag 与 Actions。
2. 在 DNS Forwarder 中发现原 C restart 由独立 stop/start 调用组成，缺少跨进程互斥；原 shell launcher 没有稳定锁、重复 start 防护、stop 成功门、daemon postcondition，且 C notification 丢弃 restart 结果。
3. 修改 launcher、RC wrapper、头文件和 PID 1 notification；增加完整 shell lifecycle regression、BusyBox feature/source contracts 及 push/pull_request 对称 path filter。
4. 根据第一次 CI 失败检查完整 job log，确认是 test fixture 的零字节进程 marker bug；修正 fixture，并同时保留 start-stop-daemon 的真实非零退出码。随后 exact-head DNS Forwarder 与 docs Actions PASS。
5. 记录 5be7a35 上 XY-C1 构建成功，但明确该构建不覆盖其后的 479f0d7 launcher 源码修改；当前 L3/L4 状态如上。

## Verification boundary

- 本轮源码实现提交：5be7a356957d47a1387346854966f3f19c8fde01（parent 47c544fe98511bfaa5f5bb66d047272748e1b021），message fix(dns-forwarder): serialize lifecycle and verify daemon transitions。随后修正提交：479f0d7a298f6980046a85c60a62271fc830893c（parent 5be7a356957d47a1387346854966f3f19c8fde01），message test(dns-forwarder): correct lifecycle process fixture；该修正还使生产 launcher 保留 start-stop-daemon 原始失败退出码。
- exact 479f0d7 DNS Forwarder run 43487234595 / job 132098791466：checkout、CWD/source contracts、完整 lifecycle lock/failure-gate regression 均 SUCCESS；AI docs run 43487234617 / job 132098791118 SUCCESS。NVRAM run 43487234555 / job 132098790943 在本记录时仍运行最后的 repository-wide API audit，下一轮需先复核最终状态。
- 5be7a35 的 XY-C1 L3 evidence：run 43485625995 / job 132093173668 完整 MIPSel 构建、TRX verify 和 artifact upload PASS；artifact 14067278386 / 12,580,062 bytes / SHA256 0e0cf60b7be3b653d64b23a2554a5efcd3dab9255676673dcb71aa10d90c20cc。不是当前 479f0d7 的 exact-source build。当前 main L3 尚未验证，L4 仍 NOT VERIFIED。
- 5be7a35 initial exact-head workflow 有一个已调查并修复的 test-fixture failure（DNS Forwarder pidof stub 对空文件 marker 的判定不一致）；后续 479f0d7 对应回归 SUCCESS。当前 exact-head 479f0d7 的其余 workflow 尚有 NVRAM job pending，不能将它写成全绿。
- 本次 root dev_log 从 31→32/119；归档数量未达到轮转条件，docs-dev-log INDEX 与既有 archives 未改动。AI docs validator 对更新后的 docs 将在本次文档 fast-forward 后再次运行。
- FTP TOCTOU / NFS client semantics 仍 OPEN/BLOCKED；host/Actions mocks 不能替代真实设备或 client 证据。

## Exact first action for the next GPT

1. 重新获取 refs/heads/main 并核对是否已有后续 commit；列出 exact head_sha + event=push 的全部 Actions。重点查看当前文档提交触发的 AI docs run，以及本轮尚 pending 的 NVRAM run 43487234555（如仍执行中，先读 job steps/log）。
2. 继续 DNSMasq 全生命周期审计：从服务 API 定义开始，列出所有 public/locked 调用者及嵌套锁边界；优先追踪 full_restart_lan() 和 DHCP/DNS stop/restart 组合、PID identity/readiness 与 wait/reap，再决定最小修复。
3. 发现确定缺陷后，添加 GNU89/Werror 或目标 shell host regression 与精确 source contracts，按普通 fast-forward 提交 main；按需更新单一 live handoff、root dev_log 和有实质状态变化的 roadmap，不制造无意义 archive/index/doc-only churn。
4. FTP upload final-name TOCTOU 继续保持 OPEN/BLOCKED；NFS durability/AUTH_SYS/root-squash 等待真实 client/device evidence；未实测不得声称 L4。

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

