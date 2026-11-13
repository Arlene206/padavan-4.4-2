## Current repository state

## 最近接力状态（2026-11-14）

- 当前代码基线 main：79f79635daa46b748a0a1f987bf56c9e75ab855c（fix(storage): serialize media service restarts with removal），父提交 2c2c134a22ae9d4e6a350233365b0c604083a4f9。2c2 是此前并行落地的 FTP/Samba restart lock 修复；本轮在此基础上完成其余四类 storage-backed add-on restart wrapper 的锁与 fail-closed 修复。
- 本轮代码提交 79f7963 修改 trunk/user/rc/src/services_stor.c 与 .github/workflows/storage-hotplug-stability.yml：restart_dms()、restart_itunes()、restart_torrent()、restart_aria() 现在在状态观察/stop 前获取共享 storage-operation fcntl lock，覆盖 stop、mount inventory、条件启动及 Transmission/Aria 防火墙状态协调；mount inventory 返回负值时不启动依赖存储的服务，锁获取失败也 fail closed。FTP/Samba 两个 wrapper 的锁修复来自父提交 2c2，现一并纳入完整六类服务重启覆盖。
- 79f7963 的 8 个 exact event=push Actions 均使用同一 head_sha。已完成 SUCCESS：storage-hotplug 43474889693、Samba 43474890036、vsftpd 43474889690、NFS 43474889702、HTTPD security 43474889735、AI handoff docs 43474889938、NVRAM boundary 43474889827；XY-C1 firmware validation 43474889913 在最后检查时仍 IN_PROGRESS（Build XY-C1 步骤执行中，TRX verify/artifact 尚未开始），不得将其写成通过。
- 79f7963 exact-head storage-hotplug 主机验证已通过：BusyBox 1.24.2/ash 生命周期回归、storage lock 跨进程 contention（2000 ms）、storage eject inventory、Optware child-result、daemon stop postcondition ASan/UBSan，以及包含四个新 restart wrapper 的 whole caller graph / lock-order source contract 均 PASS。Samba/vsftpd/NFS/HTTPD 及 AI docs 对应步骤和日志也已检查。NVRAM audit 同 SHA 完成：4,179 userland C/C++ files、raw bypass 0；44,044 kernel source files、外部 NVRAM API refs 0、writer refs 0；57 board configs；单值 snapshot contract PASS。
- 本轮没有本地完整 checkout 或本地主机测试；上述 L2 结果来自精确 SHA 的 GitHub Actions。XY-C1 构建仍在执行，因此当前 HEAD 的 L3 firmware/TRX 状态待确认；L4 实机刷写/启动/运行从未执行。
- 调用图边界：safe removal、hotplug、deferred storage startup 和 NFS 公共 applets 已使用相同 global storage lock；start_stor_apps()/stop_stor_apps() 在已持锁路径调用内部 run/stop helper。因此不能在 run_*/stop_* 内盲目再次加锁。本轮只锁 public restart wrapper，避免同进程嵌套获取导致自锁。独立 stop_ftp/stop_samba/stop_dms/stop_itunes/stop_torrent/stop_aria applets 仍需审计它们与 restart 的并发语义，不能宣称整个 service family 已全部封闭。
- refs：仍只有开发分支 main；toolchain tag 7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7；PR #1 CLOSED / UNMERGED（head ba48e22609e0c81ad757b06756fe91d05f39cbf6）。未创建/修改分支、PR、tag 或 checkpoint refs；GPT Session Rotator 未触碰。
- dev_log.md 当前根记录为 26/119，未达到 120 条 / 384 KiB 轮转门槛。本次补充一条有实质代码/CI 结果的记录后为 27/119；不创建归档，不改 docs/dev-log/INDEX.md。
- FTP upload failed-final-name cleanup 的 inode-check / unlinkat TOCTOU 仍是 OPEN/BLOCKED；没有证明 Linux 4.4 可安全地实现 inode-conditional unlink 前，不可修改该清理逻辑。NFS AUTH_SYS/root-squash/client durability 也仍需真实客户端/设备证据。

## Active task

本批 storage-backed public restart wrapper 锁修复已完成代码实现，主要相关 host/Actions 均 exact-head PASS；唯一尚未结束的 CI 项是 XY-C1 firmware validation run 43474889913。下一轮应先刷新该 run 和当前 main 的精确 SHA，然后继续审计 standalone stop applets 的锁语义。

- 已确认的竞态：FTP、Samba、MiniDLNA、Firefly、Transmission、Aria public restart 路径必须与 mdev hotplug、safe removal、deferred storage startup、NFS storage lifecycle 共用 STORAGE_OPERATION_LOCK_TAG="storage"。2c2 增加 FTP/Samba wrapper 锁；79f7963 增加其余四个 wrapper 锁。
- 四个新 wrapper 的核心策略：获取锁失败直接拒绝；stop 失败不继续启动；挂载清单状态必须明确为 positive 才允许启动，negative/unknown 时保持服务停止；所有正常/失败路径单一出口释放锁。Transmission/Aria 在锁内完成重启前后状态比较和所需 firewall reconcile。
- 不要把锁加到内部 run_*/stop_* helper：它们还由已经持有 global lock 的 start_stor_apps()/stop_stor_apps() 调用。继续检查每个调用者与锁边界，必要时考虑显式 *_locked helper + public wrapper 的拆分，而不是简单嵌套 lock。
- 当前未闭合项：rc.c 的 standalone stop applets 直接调用 stop_ftp()、stop_samba()、stop_dms()、stop_itunes()、stop_torrent()、stop_aria()；stop-only 不直接卸载磁盘，但它与另一个 public restart 并发时的最终状态可能不可预测。先追踪 HTTPD/PID1/CLI 入口、全部内部调用和锁持有方，再设计无嵌套锁的 stop transaction；不要在共享内部 stop helper 中盲目获取同一把锁。
- 另一个清晰的待审点：services_ex.c::fill_dnsmasq_servers() 在 file_lock("dservers") 后未检查返回值是否小于零，可能在锁失败时继续 staged publication。确认所有调用关系后，考虑 fail-closed guard 并在 .github/workflows/dnsmasq-lifecycle.yml 增加 source contract；这是后续独立切片，不属于 79f7963。
- 保持 FTP 上传清理 TOCTOU BLOCKED；不做未经证明的 inode-race 修复。完整多板固件/TRX 与实机验证按项目计划在主体开发后集中完成。

## Completed in this handoff

1. 严格按 bootstrap / development protocol 顺序恢复项目；读取 handoff、roadmap、dev log，检查当前 main 与 refs，审阅上个 GPT 的未完成任务。
2. 在父提交 2c2 的 FTP/Samba restart lock 修复基础上，确认 MiniDLNA/Firefly/Transmission/Aria public restart 同样缺少与 safe-removal 共享的 storage lock；并发现旧代码把 storage_has_mounted_devices() 的 -1 当作 C truthy 值，导致 inventory 不确定时仍可能拉起依赖磁盘的服务。
3. 提交 79f79635daa46b748a0a1f987bf56c9e75ab855c：为四个 wrapper 加入外层共享锁、stop failure gate、mounted inventory positive-only start gate 和唯一 unlock；Transmission/Aria 的防火墙状态协调留在锁内。
4. 扩展 storage-hotplug workflow 的语义检查，逐个验证四个 wrapper 的 lock→stop→inventory→positive-start→unlock 顺序、单次释放、stop-failure abort 和 unknown-inventory fail-closed。
5. 检查 79f7963 的八个 exact-head Actions：七项已完成 SUCCESS；XY-C1 run 43474889913 最后查询仍在 Build XY-C1。没有把仍运行的步骤写成 PASS；当前没有本地编译或实机测试。

## Verification boundary

- 代码提交：79f79635daa46b748a0a1f987bf56c9e75ab855c，父提交：2c2c134a22ae9d4e6a350233365b0c604083a4f9。
- Exact-head push Actions（全部 head_sha=79f7963）：storage-hotplug [43474889693/job 132056043039] SUCCESS；Samba [43474890036/job 132056043239] SUCCESS；vsftpd [43474889690/job 132056042311] SUCCESS；NFS [43474889702/job 132056042637] SUCCESS；HTTPD security [43474889735/job 132056043153] SUCCESS；AI docs [43474889938/job 132056042846] SUCCESS；NVRAM boundary [43474889827/job 132056042686] SUCCESS；XY-C1 firmware validation [43474889913/job 132056043295] 最后检查仍 IN_PROGRESS。
- Storage-hotplug logs 明确记录：BusyBox 1.24.2/ash regressions PASS；跨进程 shared storage lock blocked 2,000 ms PASS；storage eject inventory、Optware lifecycle/child result、daemon stop ASan/UBSan policy PASS；四个 add-on restart lock-order source contracts 随 whole caller graph 最终 PASS。该 workflow 只证明 L2 host/Actions bounded behavior，不等于设备上真实 hotplug/eject 证明。
- NVRAM exact-head audit PASS：4,179 userland C/C++ sources, raw bypass count 0; 44,044 kernel sources, NVRAM external API ref 0, writer ref 0; 57 board configs. 它仅支持既有单值 ioctl snapshot bounded claim，不扩展为全局 NVRAM 原子性。
- L3：XY-C1 Build XY-C1 尚在运行，TRX output verification/artifact upload 尚 pending；不能声称当前 main 的 L3 已完成。L4 router flash/boot/runtime NOT VERIFIED。
- 没有本地完整 checkout、host compile、full firmware build 或实机操作。

## Exact first action for the next GPT

1. Fetch current main; require it to equal the latest docs handoff commit (parent is 79f7963). Enumerate all event=push Actions for that exact current SHA and inspect every job/step/log. Specifically re-check XY-C1 run 43474889913 tied to 79f7963; if still in progress, report its real state and do not claim TRX success.
2. Then inspect standalone stop applets in rc.c and all callers of six storage-backed stop helpers. Decide whether a thin public lock-taking wrapper plus private *_locked function is required; preserve already-held start_stor_apps()/stop_stor_apps() call paths without recursive acquisition. Add focused host/source contract coverage before changing behavior.
3. Keep FTP failed-upload TOCTOU blocked until a genuinely inode-conditional cleanup design is proven on the project's Linux 4.4 semantics.
4. After stop-app transition audit, address fill_dnsmasq_servers() missing file_lock("dservers") failure check as a separate DNSMasq reliability change with an exact-head workflow contract.
5. Do not start a full all-board build or claim L3/L4 prematurely; preserve the project's existing plan to concentrate full firmware and device verification after mainline source governance is substantially complete.

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

