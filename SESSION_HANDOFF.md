## Current repository state

## 最近接力状态（2026-11-14）

- 最新 main：THIS COMMIT；本批以 `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 为父提交，在 main 上 fast-forward，不改写历史。远程仅有 main 分支；PR #1 仍 CLOSED / UNMERGED（head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`）；tag `toolchain` 仍为 `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7`。
- 父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 的 exact-head push Actions 共 35 条：34 条已 COMPLETED / SUCCESS，唯一仍进行中的是 XY-C1 run `43481041936` / job `132077270140`（Build XY-C1 尚在执行，TRX verify 和 artifact upload 尚未开始）。Storage hotplug `43481041828` / job `132077270277`、vsftpd `43481042213` / job `132077270992`、Samba `43481042272` / job `132077271289`、AI docs `43481042175` / job `132077271149`、NVRAM `43481042215` / job `132077271729`、DNSMasq `43481042518`、PID 1 notification `43481042145` 均 SUCCESS，head_sha 完全等于该父 SHA。storage/vsftpd/Samba/AI/NVRAM 完整 job steps 与日志已检查；storage caller graph/group-stop contracts、vsftpd/Samba source contracts、AI handoff docs 29/119 均 PASS。
- Storage standalone-stop slice 已在 `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 完成：六个对外 stop API 使用共享 storage-operation fcntl lock 和私有 `*_locked()` helper；重启与已持锁 teardown 不递归获取锁；FTP+Samba 单锁 group-stop 在 FTP 失败时仍尝试 Samba，并保留首个失败；RC add-on stop applet 传递退出状态。以上 L2 由该 SHA 的 storage-hotplug、vsftpd、Samba Actions 验证；本批未改动这些存储源码。
- 本轮扩大到 DNSMasq 与 PID 1 通知边界，发现新的返回码丢失：`handle_notifications()` 只把负 `action_result` 视为失败，但 `restart_dhcpd()` 的锁、准备和启动失败路径可返回正 errno；RADV、VLMCSD、WINS、DHCPD 通知还直接丢弃重启结果，NMBD 与 WINS 通知也丢弃 `restart_nmbd()` 的结果。失败操作因此可能被错误清除 incomplete marker，通知队列将其视作成功。
- 新增 `rc_notify_merge_action_result()`：把任意非零 service status（正 errno 或负错误）归一化为 PID 1 所需的负失败结果，并保留已有失败。四个 DNSMasq 和两个 NMBD 通知调用点现在都通过该 helper 合并状态；WINS 仍继续执行 DHCPMasq 和后续 VPN reapply，不因前一步失败而跳过剩余操作。
- 增加 GNU89 host regression，覆盖成功、正 errno（EBUSY/EIO）、负错误以及已有失败保持；DNSMasq workflow 的 caller contracts 验证所有四个 DNSMasq 与两个 NMBD 调用点都已处理，PID 1 workflow 验证归一化 helper。两个 workflow 的 push / pull_request filters 均已纳入 helper/test。当前代码提交后的 Actions 尚待 exact-head 核验。
- 本轮没有完整本地 checkout，也没有在本环境运行新的 C host binary；测试将由当前 SHA 的 PID 1 / DNSMasq Actions 编译执行。父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 的 storage / Samba / vsftpd 等 L2 结果不替代本轮新改动的 Actions。
- 父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 的 XY-C1 exact-source build 尚无结论；旧源码 `2d5f44b1978c3272fa85633e273297c16bd34283` 的 run `43476679718` / job `132062194706` 已成功完成 TRX 与 artifact（ID `14064545568`，12,580,294 bytes，SHA256 `0fba859e6561464feaade2813bd2104d6e789c3829c4f5b30cb5875fa2d40e4d`），只覆盖那个旧功能源码 SHA。L4 实机验证仍 NOT VERIFIED。
- 根 `dev_log.md` 更新后为 30/119；未到 120 条或 384 KiB 轮转门槛，不新增归档、不改 INDEX。FTP upload failed-final-name inode-check/unlinkat TOCTOU 仍 OPEN/BLOCKED；NFS AUTH_SYS/root-squash/client durability 仍需真实客户端/设备证据。

## Active task

本批修复 DNSMasq / NMBD 服务失败状态在 PID 1 通知队列中的归一化与传播问题。当前 main 的新 commit Actions 尚待核验；先逐项检查 DNSMasq、PID 1 notification、AI handoff docs 和 XY-C1 的 exact-head 结果。通过后继续 DNS Forwarder/DNSMasq 进程监督审计，重点查 start/stop/reload 并发边界、stale PID、wait/reap、fd/temporary-file 清理、失败回滚与 LAN 全量重启交互。

- notification queue 只在 `action_result < 0` 时保留 incomplete marker；所有返回码可能为正 errno 的服务调用必须先归一化，禁止直接把正错误码赋给 `action_result`。
- 所有四个 `restart_dhcpd()` 通知调用（RADV、VLMCSD、WINS、DHCPD）和两个 `restart_nmbd()` 调用（NMBD、WINS）均应继续经过 `rc_notify_merge_action_result()`；新增服务调用时同步增加 workflow source assertion 和正/负错误回归。
- WINS 等组合通知即使前一服务失败，也要继续尝试后续服务/收敛操作，同时保留失败结果；不能因早期失败短路而留下半更新配置。
- 不重复已完成的 `dservers` / `resolv` 锁失败保护；也不把祖先 SHA 的 CI 当成本批证据。
- FTP failed-final-name cleanup 的 inode-check/unlinkat TOCTOU 继续 OPEN/BLOCKED，未证明 Linux 4.4 安全语义前不得改动；NFS AUTH_SYS/root-squash/durability 继续等待真实 client/device evidence。

## Completed in this handoff

1. 按要求顺序读取启动/开发协议、handoff、roadmap、dev log；核对 main、唯一分支、tag 与 PR #1 状态。
2. 核对父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 的 35 条 push Actions：34 条 SUCCESS，仅 XY-C1 run `43481041936` 仍在 Build 阶段；已检查 storage、vsftpd、Samba、AI docs、NVRAM 的相关 job 和日志。
3. 完成 storage standalone-stop 任务的 caller/lock 审计与 L2 核验；这部分实际代码由父 SHA 提交，本批未重复修改 storage 源码。
4. Trace PID 1 `handle_notifications()` 的 marker 生命周期，确认 positive errno 和被忽略的服务返回值会让失败通知误判成功。
5. 归一化所有 DNSMasq/NMBD notification result，扩展 host regression 和两个 workflow 的源代码合同与 path filters；本批 exact-head Actions 交由下一步核验。

## Verification boundary

- 本批 main 结果 SHA：THIS COMMIT；parent：`ec93ae5d070f34b848652f1dbe4b1a441d6cc240`。仅普通 fast-forward，远程 main 之外的 refs 未改动。
- 父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 上 storage-hotplug [`43481041828 / 132077270277`]、vsftpd [`43481042213 / 132077270992`]、Samba [`43481042272 / 132077271289`]、AI docs [`43481042175 / 132077271149`]、NVRAM [`43481042215 / 132077271729`]、DNSMasq [`43481042518`]、PID 1 [`43481042145`] 全部 SUCCESS。storage/vsftpd/Samba/AI/NVRAM jobs 的 steps/logs 已检查。它们证明父 SHA 的相关源码，不替代本批 rc notification 改动的精确 Actions。
- 父 SHA `ec93ae5d070f34b848652f1dbe4b1a441d6cc240` 的 XY-C1 run `43481041936` / job `132077270140` 最后检查仍在 Build XY-C1；TRX verify 与 artifact upload 未开始。必须刷新状态；不得以旧 `2d5f44b` artifact 作为本批 L3。
- 本批新增测试在 `tests/rc_notify_batch_policy.c`，由 PID 1 notification workflow GNU89/Werror 编译；DNSMasq workflow 另外校验四个 DNSMasq/两个 NMBD 通知调用的状态传播和 helper/test 对称路径。新 commit 下 DNSMasq、PID 1、AI docs、XY-C1 Actions 都必须核验。没有在本环境执行新的 host binary；没有完整固件本地编译或 router flash/boot。
- FTP TOCTOU 及 NFS client semantics 维持 OPEN/BLOCKED；L4 仍 NOT VERIFIED。

## Exact first action for the next GPT

1. 重新读取 `refs/heads/main` 并记录 exact SHA；列出所有 `event=push` 且 `head_sha` 完全相等的 runs。重点读取 DNSMasq workflow、PID 1 notification queue、AI handoff docs、XY-C1 build/TRX/artifact。逐 job 检查完整 steps 与日志；构建成功时记录 artifact ID、大小、SHA256。
2. 若任何 workflow failure，区分 production C/test/source checker/path-filter harness，保留首轮真实失败证据，在 main 上最小修复并重新核验新 SHA；绝不拿祖先 SHA 的通过替代当前。
3. 全部 L2 验证后继续 DNS Forwarder/DNSMasq 生命周期监督审计：公共 start/stop/reload 和 LAN full restart 的锁调用图、DNSMasq process identity/stale PID、停止 postcondition、daemonization child/reap、fd/stage 清理、reload 失败和配置回滚。找到确证稳定性 BUG 后立即实施代码和 host/source regression。
4. FTP cleanup TOCTOU 未证明 race-safe 之前保持 OPEN/BLOCKED；NFS durability/AUTH_SYS/root-squash 等等待真实客户端与设备证据；不宣称 L4。

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

