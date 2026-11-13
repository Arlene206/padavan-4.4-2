## Current repository state

## 最近接力状态（2026-11-14）

- 最新 main exact SHA：`f01c05a455367e91d3302d2e302902a4eb2ff3dc`，父提交 `ec93ae5d070f34b848652f1dbe4b1a441d6cc240`。main 仅通过普通 fast-forward 更新；当前远程只有 main 分支，PR #1 仍 CLOSED / UNMERGED（head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`），`toolchain` tag 仍为 `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7`。
- 最新 SHA `f01c05a455367e91d3302d2e302902a4eb2ff3dc` 对应的 push Actions 共 28 条，28/28 COMPLETED / SUCCESS，没有 failure 或 in-progress。重点 job 已检查完整 steps / 日志：DNSMasq [run `43483330811` / job `132085197857`]，PID 1 notification [`43483330787` / `132085196895`]，AI handoff docs [`43483330899` / `132085198145`]，USB storage hotplug [`43483330558` / `132085196419`]，NVRAM [`43483330633` / `132085197401`]。这些 run 的 `head_sha` 均精确等于当前 SHA。
- DNSMasq / NMBD 通知错误状态修复已达到 L2：DNSMasq workflow 的 restart policy、DHCP range、跨进程锁、Inadyn、daemon postcondition 与新增 4 个 DNSMasq / 2 个 NMBD 调用点 source contracts 均 PASS；PID 1 workflow 以 `cc -std=gnu89 -Wall -Wextra -Werror` 编译并运行 `rc_notify_batch_policy.c`，正 errno / 负错误归一化回归 PASS；AI docs validator PASS（提交验证时 root dev_log 30/119）。Storage job 的 BusyBox ash 回归、storage eject inventory、跨进程锁、stop postcondition/group result 和 whole caller graph PASS。NVRAM 锁、snapshot、device-node、ioctl/userland bypass 合同 PASS（RAW_USERLAND_BYPASS_COUNT=0）。
- 前一个 storage 变更 commit `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 的 XY-C1 exact-source run `43481041936` / job `132077270140` 已 COMPLETED / SUCCESS，Build XY-C1、TRX verification、artifact upload 全部 PASS。TRX：`TurBoTse-XY-C1-4.4-mipsel-linux-musl-20261114.trx`；artifact ID `14065946140`，大小 12,579,764 bytes，SHA-256 `f55f38c3dbc64a73994d675c36f802c62c8f97c7b9947f259d56a87461408e06`。该 L3 只证明 `ec93ae5d070f34b848652f1dbe4b1a441d6cc240`，不覆盖当前 SHA 的后续 RC notification 变更。
- 当前 SHA 没有触发新的 XY-C1 run：该 workflow 的 push path filters 不包含 `trunk/user/rc/src/rc.c`、`rc_notify_batch_policy.h` 或对应测试。完整固件编译按项目计划安排在主体开发完成后集中进行；不得把父 SHA artifact 宣称为当前 SHA 构建。L4 router flash/boot 仍 NOT VERIFIED。
- 最新 root `dev_log.md` 更新到 31/119（约 83 KiB），未达 120 条或 384 KiB early-rotation 门槛；不归档、不改 INDEX。FTP upload final-name inode-check/unlinkat TOCTOU 仍 OPEN/BLOCKED；NFS AUTH_SYS/root-squash/client durability 仍需真实客户端与存储证据。

## Active task

DNSMasq/NMBD 通知失败状态归一化已在当前 main 完成并通过 exact-head L2。下一批继续全用户态 DNS Forwarder / DNSMasq 进程监督审计：梳理 public / locked start-stop-reload caller，检查 LAN full restart 组合事务、启动 postcondition、停止失败门、stale PID / process identity、child wait/reap、fd 与临时文件清理、配置回滚，以及其他通知调用点的 positive errno / negative errno 结果契约。发现可证实的稳定性缺陷后再实施源码与回归；不要重复本批已完成的通知 helper 与 resolver lock 门禁。

- `handle_notifications()` 只将 `action_result < 0` 视为失败；新服务调用如果可能返回正 errno，必须合并为负失败结果。现有 4 个 `restart_dhcpd()` 与 2 个 `restart_nmbd()` 通知调用均经 `rc_notify_merge_action_result()`。
- `restart_infosvr()` / `restart_crond()` 有“服务被禁用”时返回正值的语义；不要机械把 notification handler 所有非零状态全局归一化，否则会把有效的 disabled/no-op 状态误报为失败。先追踪各 service API 的结果契约。
- 组合通知（例如 WINS）应尽量完成后续必要收敛动作，同时保留前序操作失败结果；不得因早期错误无意跳过后续独立操作。
- FTP final-name cleanup inode-check/unlinkat TOCTOU 继续 OPEN/BLOCKED，未证明 Linux 4.4 下 race-safe 前不得改动；NFS AUTH_SYS/root-squash/durability 继续等待真实 client/device evidence。

## Completed in this handoff

1. 严格按顺序恢复 bootstrap / development protocol、handoff、roadmap、dev_log；核对 exact main、远程 refs、checkpoint 与 Actions。
2. 上一会话遗留的 standalone storage stop 问题由并行开发会话提交到父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240`；本轮确认 storage hotplug / vsftpd / Samba exact-head Actions PASS，不将该代码提交归功于本会话。
3. 本会话实施当前 SHA：修复 PID 1 对 DNSMasq/NMBD 通知服务失败状态的丢失；添加 policy helper、GNU89 regression、DNSMasq / PID1 workflow contracts 与 path filters；提交到 main。
4. 当前 SHA 的 28 条 exact-head push Actions 全部为 28/28 SUCCESS；DNSMasq、PID1、AI docs、storage hotplug、NVRAM 相关 jobs 的 steps/log 已检查。
5. 记录父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 的 XY-C1 L3 build/TRX/artifact 成功，但明确不把其结果扩展到当前 SHA；当前无 L4 实测。

## Verification boundary

- 本会话代码提交：`f01c05a455367e91d3302d2e302902a4eb2ff3dc`（parent `ec93ae5d070f34b848652f1dbe4b1a441d6cc240`），message `fix(rc): retain failed DNS service notification markers`。提交后 exact main 已复核。源码路径：`rc.c`、`rc_notify_batch_policy.h`、`tests/rc_notify_batch_policy.c`、DNSMasq / PID1 两个 workflow。
- 当前 SHA 的全部 28 条 exact-head push Actions = 28 SUCCESS / 0 failure / 0 pending。DNSMasq [`43483330811 / 132085197857`]、PID1 [`43483330787 / 132085196895`]、AI docs [`43483330899 / 132085198145`]、storage [`43483330558 / 132085196419`]、NVRAM [`43483330633 / 132085197401`] 均已核验具体 job steps / logs。DNSMasq/PID1 新增用例由 Actions 在 GNU89/Werror 下真实编译执行；本环境没有本地 checkout 或独立本地构建。
- XY-C1 L3 证据为父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 的 run `43481041936` / job `132077270140`：构建与 TRX verify PASS，artifact `14065946140` / 12,579,764 bytes / SHA256 `f55f38c3dbc64a73994d675c36f802c62c8f97c7b9947f259d56a87461408e06`。workflow 未因当前 SHA 的 RC policy-only paths 再次触发；当前 exact SHA L3 未验证。当前 L4 仍 NOT VERIFIED。
- AI doc Actions 在代码提交时验证了 root dev_log 30/119、archive inventories；本次状态记录会使根 log 成为 31/119，没有轮转或 archive 变更。
- FTP TOCTOU 与 NFS client semantics 仍为 OPEN/BLOCKED；不要将 host mocked tests 当作真实设备/client 证明。

## Exact first action for the next GPT

1. 重新读取 `refs/heads/main`，核对是否已有后续提交；对当前 main 拉取所有 exact-head push Actions。检查本次状态文档提交后的 AI handoff docs validator。
2. 延续 DNSMasq/DNS Forwarder audit：先从 `start_dns_dhcpd()` / `stop_dns_dhcpd()` / `restart_dhcpd()` / `restart_dns()` 与 `full_restart_lan()` 的所有调用点建立完整锁边界表；再追踪 `pids()` / process identity、stop postcondition 和 launch/reap 结果约定。特别注意 `restart_infosvr()`、`restart_crond()` disabled 状态正返回值的语义，不要误归类。
3. 找到确证 BUG 后新增最小 GNU89/Werror 回归和精确 caller/source contracts，实施代码修复并按协议提交 main；更新 handoff/dev_log/roadmap 的活跃状态，避免无意义 archive/index/doc-only churn。
4. FTP upload final-name TOCTOU 仍保持 OPEN/BLOCKED；NFS durability/AUTH_SYS/root-squash 要等真实 client/device evidence；L4 不可宣称。

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

