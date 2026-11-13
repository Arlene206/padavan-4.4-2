## Current repository state

## 最近接力状态（2026-11-14）

- 恢复基线 main：233d5991afac081e2cd6784d9c212462cd519b07（docs: hand off verified NVRAM ioctl snapshot fix）；父提交为 a7a70397ec9f6458655a7f0520769c5270436258。远程 main 在本轮开始时与此 SHA 精确一致。
- 233d 精确 event=push Actions 六项全部 COMPLETED / SUCCESS：infosvr 43470668893、AI handoff docs 43470668983、AdGuardHome 43470669030、AliDDNS 43470668996、AliyunDrive 43470668891、NVRAM boundary 43470668895；每个 run 的 head_sha 均为 233d5991afac081e2cd6784d9c212462cd519b07。NVRAM 快照实现及策略扫描的完整 PASS 细节见上一记录和 roadmap；本轮没有完整 MIPS/uClibc firmware/TRX 或实机测试。
- 上轮 NVRAM_IOCTL_GET 单值快照修复已在 d551 实施、a7 修复 host 编译 include 并由 exact-head NVRAM/AI docs Actions 验证；只覆盖本驱动的单值 ioctl，不代表所有 legacy/out-of-tree nvram_get() 调用或系统级全局原子性。
- 本轮从 233d 检查到 FTP/Samba 重启未获取 safe-removal/hotplug 共用的 storage-operation file lock，而 hotplug、safe removal、deferred storage start 和 NFS 公共 applets 都使用同一进程间锁。HTTPD direct RC applets、PID 1 notification dispatcher、CLI run_ftpsamba/run_ftp/run_samba 路径都能到达这两个无锁 restart 函数；形成明确的并发卸载窗口。本次已把 lock 包在 restart_ftpd()/restart_smbd() 的完整 stop→mount inventory→start/WINS 事务外层，单一出口释放，不把锁塞进内部 run/stop 函数，以免与 start_stor_apps()/stop_stor_apps() 已持锁调用产生自锁。三条 source-contract workflow 已补充顺序/释放断言；新提交的 Actions 尚待精确 SHA 验证。
- refs：仅开发分支 main；toolchain tag 7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7；PR #1 CLOSED / UNMERGED（head ba48e22609e0c81ad757b06756fe91d05f39cbf6）。不创建/修改 branch、PR、tag、checkpoint refs；GPT Session Rotator 未触碰。
- dev_log.md 根文件本次增加到 26/119 条，未到 120-entry / 384 KiB 轮转阈值；保留历史 archives / INDEX，不做归档。
- 当前代码证据和 host/Actions 均不构成 L3 当前 HEAD MIPS/uClibc 完整 firmware/TRX 或 L4 实机验证；这些仍安排在主体开发后集中执行。

## Active task

已确认并修复 FTP/Samba 服务重启与存储安全卸载之间的跨进程锁竞态；当前待完成的是该修复的新 SHA 精确 Actions 验证。

- 根因：restart_ftpd() 和 restart_smbd() 之前依次 stop、读取 /proc/mounts、按需 start，但未持有 STORAGE_OPERATION_LOCK_TAG="storage" 的 file_lock；hotplug mdev 与 safe_remove_stor_device()/safe_remove_all_stor_devices() 则持有同一把跨进程 fcntl 锁。PID 1 的 RCN_RESTART_FTPD/SMBD 事件、HTTPD 的 run_ftp/run_samba/run_ftpsamba applet，以及 CLI 都能触发 restart；因此 restart 的 stop/inventory/start 可与另一进程卸载同一挂载点并行。
- 修改 services_stor.c：restart_ftpd()/restart_smbd() 在任何状态查询/daemon stop 前获取 storage_operation_lock_acquire(file_lock)，覆盖 mounted-storage inventory、服务重启和 Samba WINS 决策；锁获取失败 fail closed；stop 或 run 失败保留原负值/错误状态；inventory 不确定时不启动服务；统一 out 释放一把锁。
- 不在 run_ftp()/stop_ftp()/run_samba()/stop_samba() 内部二次获取 global lock：这些内部函数还由已持锁的 start_stor_apps()/stop_stor_apps() 调用；重复 file_lock 在同进程会被拒绝，可能造成启动/卸载回归。公共 restart wrapper 是正确互斥边界。独立 stop applets 和其他 storage-backed service restart 尚需继续审计。
- vsftpd、Samba、storage-hotplug 三个 workflow 已新增源码语义断言，检查锁先于 stop/inventory/start、锁在 WINS 决策之后释放且仅释放一次；已有跨进程 storage_operation_lock regression 仍覆盖 file_lock 竞争行为。
- 下一步首先检查本提交所有精确 head_sha 的 push Actions 和完整 logs，修复任何失败。通过后再扫描 MiniDLNA/Firefly/Transmission/Aria、NFS/FTP/Samba standalone stop applets 与直接 HTTPD 调用的生命周期锁语义，不假设本轮已完成整个 storage service family。

## Completed in this handoff

1. 按 bootstrap / development protocol 既定顺序恢复；核对 main、branch/tag、PR #1、文档治理阈值和最新 Actions。233d 上六个 push Actions 均 SUCCESS，SHA 完全匹配。
2. 读取 HTTPD aidisk 的 direct run/stop routes、variables.c EVM→RCN mapping、PID 1 notification dispatch、rc.c CLI applet 路径；追踪到 services_stor.c restart wrappers。
3. 对照 mdev_sd/mdev_mmc、safe_remove_stor_device/all、deferred start 和 NFS run/stop/restart 的跨进程 storage-operation lock 使用，确认只有 FTP/Samba public restart 未覆盖完整生命周期事务。
4. 实施 restart_ftpd()/restart_smbd() 外层共享锁；在锁持有期执行 stop、mount inventory、条件 start / WINS 决策，统一释放并保留失败状态；不将锁嵌入可由已持锁调用方复用的内部 start/stop helpers。
5. 更新 vsftpd、Samba、storage-hotplug workflow 的 source contract checks，并更新 live handoff/roadmap/dev log。新 SHA 的 Actions 尚未检查；没有本地完整 checkout 或声称本地测试通过。

## Verification boundary

- Recovery SHA 233d5991afac081e2cd6784d9c212462cd519b07：六个 exact event=push workflows 均 SUCCESS（infosvr 43470668893、AI docs 43470668983、AdGuardHome 43470669030、AliDDNS 43470668996、AliyunDrive 43470668891、NVRAM boundary 43470668895）；全部 head_sha 匹配 233d。该基线用于本轮源代码分析，不是本次修改的最终 CI。
- L1：已逐条追踪 mdev 与 safe-removal/NFS global lock acquisition，再比对 HTTPD/PID1/CLI FTP/Samba restart 调用链；源码显示 restart wrapper 缺少同一 global lock，而本次补齐 stop→inventory→start/WINS 范围。无 device runtime 实测。
- L2：新 commit 的 vsftpd、Samba、storage-hotplug 及 AI handoff docs Actions：PENDING exact-head run/job/log inspection；不得提前标注 HOST/ACTIONS VERIFIED。
- NVRAM kernel ioctl single-value snapshot 的 L1/L2 bounded result 在 exact a7 完成；旧 d551 首轮 include-path CI failure 已由 a7 修复并验证。
- L3 current HEAD MIPS/uClibc full firmware/TRX：NOT VERIFIED。L4 physical router flash/boot/runtime: NOT VERIFIED。无本轮本地固件 build 或实机 test。

## Exact first action for the next GPT

Fetch live main and confirm it equals the resulting commit from this code batch. Enumerate every exact event=push Action and require matching head_sha; inspect every job, step and full log, especially vsftpd, Samba share, storage hotplug and AI handoff docs. Fix any failed source assertion/host regression on main before closing this lock-race slice. Once green, continue the global storage supervision audit beyond FTP/Samba: trace standalone stop applets and restart/start/stop flows for MiniDLNA, Firefly, Transmission and Aria against safe removal and NFS/hotplug locks, avoiding nested acquisition in *_locked callers. Keep FTP upload cleanup TOCTOU blocked until safe inode-conditional semantics are proven; defer L3/L4 until the main development body is complete.

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

