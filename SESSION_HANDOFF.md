## Live recovery checkpoint — 2026-11-14 05:24 CST

> This block supersedes the older 2026-11-14 04:24 CST state below. The prior checkpoint is retained as historical context; do not treat its SHA as live.

- Live baseline before this handoff refresh: `main` = `947d9ce8aa51a8f000bbe3186e03774c226880c2`; direct parent `85b06794590a8d3fc7fe4218b2f967493d45d9e3`. Compare confirms exactly one intervening commit, documentation-only (`SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`); product sources are unchanged.
- Remote branches: only `main`. Legacy PR #1 remains CLOSED / UNMERGED (head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`, base `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74)); no PR work was used. Remote tag `toolchain` points to `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7`.
- Exact-main push Actions for `947d9ce8aa51a8f000bbe3186e03774c226880c2`: 6/6 completed SUCCESS, all `event=push` and exact `head_sha`: AI handoff docs 43501595814; infosvr 43501595647; AdGuardHome 43501595737; AliyunDrive 43501595819; AliDDNS 43501595686; NVRAM 43501595559. AI docs workflow run metadata confirms exact SHA and success. This is Actions status evidence; no local job logs were executed in this session.
- Active unfinished task remains the shared process-inventory fail-open defect. Current source blobs: `trunk/user/shared/src/pids.c` = `f5092806d6f1a76021643b0af54eb24705cbd8fe`; `trunk/user/shared/src/shutils.h` = `6ccb679cf049374839a0c5de6a543955614dc275`; `trunk/user/rc/src/common_ex.c` = `de5cc703ce1b9e2eb0897dc52ed7e1c5ec15eb97`; `trunk/user/rc/src/services_ex.c` = `8134ee60a84d626785956a073c1c1bde3422d84d`; `service_lifecycle_policy.h` = `abe9f77b5354d8a33bce66c2575d1b1d09a2d4e2`.
- Confirmed source path: `alloc_procps_scan()` returns NULL on scan-context allocation failure or `opendir("/proc")` failure; initial `procps_scan(NULL, ...)` then returns NULL; `find_pid_by_name()` returns its preallocated empty PID list, and `pids()` reports 0. `common_ex.c::service_lifecycle_is_running()` forwards `pids()` into generic stop policy; `services_ex.c::is_dns_dhcpd_run()` and DNSMasq start/restart/readiness guards also use `pids()`. `kill_services()` has a post-failure logging check that also consumes the Boolean API. Samba and miniupnpd/other services in `services_ex.c` have additional Boolean call sites. Do not change the legacy Boolean API globally until every repository call site and consumer has been inventoried.
- Important new detail from reading `service_lifecycle_policy.h`: its callback contract is Boolean running/not-running, not tri-state; generic stop policy can escalate TERM to KILL. The correct fix must preserve an explicit UNKNOWN/error state through the policy and prevent signal escalation or successful absence claims when inventory is unknown. DNSMasq's second readiness check and static-ARP cleanup must also remain fail-closed.
- No production files or tests were changed in this session. No local checkout is available through the connected GitHub actions; repository-wide code search returned no indexed matches, so a complete `pids()` caller inventory and executable host regression cannot be honestly claimed. No commit was created for product code.
- Validation: L1 partial source review only. L2 host tests NOT RUN. Existing XY-C1 L3 firmware evidence remains on product-source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` (run `43489646254`, job `132107102854`, TRX/artifact verification as recorded below); current product sources remain identical because later commits are docs-only. L4 router flash/boot NOT VERIFIED.
- Root dev_log was 35/119 before this recovery note; this note makes it 36/119, far below rotation thresholds. Archive inventory remains dev-log=8 / handoff=22 / roadmap=18; no rotation, INDEX update, or snapshot archive is due.

## Exact first action for the next GPT

1. Fetch live `main` and exact-head push Actions again; resolve the SHA of this handoff refresh commit.
2. Obtain a complete local source checkout or another reliable whole-repository content-search path before editing. Enumerate every `pids()` / `pids_main()` / `find_pid_by_name()` caller, all relevant headers, and all stop/readiness policy consumers; include `common_ex.c`, DNSMasq, Samba, miniupnpd and every other daemon.
3. Design a checked process-inventory API that distinguishes RUNNING, ABSENT and UNKNOWN while retaining legacy Boolean compatibility only where safe. Propagate `readdir()` errors as well as initial `/proc` open/context allocation errors; distinguish normal end-of-directory from scan failure and PID-list allocation failure. Generic lifecycle policy must abort on UNKNOWN before TERM/KILL escalation.
4. Add injectable host regressions for context OOM, `/proc` open failure, mid-scan read error, empty inventory, matching process, PID-list growth OOM, and UNKNOWN during stop/reap. Compile with firmware-compatible GNU89 and `-Werror`; wire into a focused Actions workflow without weakening existing checks.
5. Migrate DNSMasq start/readiness/stop/restart and service-lifecycle consumers fail-closed; only then complete the full DNSMasq caller/identity/HUP/wait-reap/FD/temp/rollback audit and rotate to other userland modules.

---

## Current repository state

## 最近接力状态（2026-11-14）

- 最新 main exact SHA：`85b06794590a8d3fc7fe4218b2f967493d45d9e3`，直接父提交 `18561fddd214238dda124e0b8883030081ffca5d`；`85b0679` 仅更新 SESSION_HANDOFF / REFACTOR_ROADMAP / dev_log，不改变产品源码。此前产品源码提交为 `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f`，其父提交为生产修复 `ed010248a2d1bb073d7c6aa28c3bd9afcda3d07f`。远程只有 main；PR #1 CLOSED / UNMERGED（head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`），toolchain tag `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7`。
- Exact-main `85b067` push Actions are 6/6 COMPLETED / SUCCESS (event=push; every `head_sha` exactly matches live main): AI docs [43492884427 / 132118367981], AliyunDrive [43492884404], AliDDNS [43492884384], AdGuardHome [43492884381], infosvr [43492884353] and NVRAM [43492884341]. The AI docs validator passed and reported the pre-entry root log at 34/119; archive counts dev-log=8, handoff=22, roadmap=18.
- 当前产品源码 SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` 的 XY-C1 MIPSel 构建已完成 L3：run `43489646254` / job `132107102854` 的 Build XY-C1、TRX output verification、artifact upload 全部 SUCCESS。TRX 为 12,625,949 bytes；artifact `14069572898`，12,579,578 bytes，SHA-256 `c6581125da108ac1118dfcaf895bcfb2414e09554eb63b96be8bddb369c7b768`。当前 main `85b0679` 及其文档祖先 `18561f` 均仅在该已构建源码之上新增文档，因此产品源码与 L3 产物完全相同；L4 router flash/boot 仍 NOT VERIFIED。
- `ed010248` 的生产修复：`full_restart_lan()` 返回 DNSMasq lifecycle 状态；锁获取失败返回 errno，stop 失败时禁止启动第二实例，start 失败保留原始状态，锁释放失败在无先前错误时也传播；PID 1 的 `RCN_RESTART_LAN` 通过 `rc_notify_merge_action_result()` 合并状态，避免 LAN restart 中 DNSMasq 失败被错误确认成功。
- `691e4d6` 只修改 `.github/workflows/httpd-security.yml`：将 `rc.c` 纳入 push / pull_request path filters，并把 HTTPD umbrella 的 LAN restart source contract 对齐到 `dnsmasq_status`、`int full_restart_lan()` 和 PID 1 result merge；不改变产品源码。该提交后的 exact HTTPD run [`43489646217 / 132107102491`] SUCCESS。
- exact `ed010248` DNSMasq lifecycle [`43489264442 / 132105772776`] 与 NVRAM audit [`43489264608 / 132105772928`] SUCCESS。DNSMasq workflow 编译并运行 GNU89/Werror policy tests、DHCP pool / lifecycle lock tests，并通过 full LAN restart / RC notification source contracts；NVRAM log 报告 `RAW_USERLAND_BYPASS_COUNT: 0`。在该 SHA 上曾有一次 HTTPD umbrella stale assertion failure（旧变量 `dnsmasq_stop_status` 与当前源码 `dnsmasq_status` 不匹配），已由 `691e4d6` 修复并在 exact SHA 复跑通过，失败过程没有隐藏。
- 新发现尚未修复的跨模块稳定性问题：`trunk/user/shared/src/pids.c` 的 `find_pid_by_name()` 在初始 `procps_scan(NULL, ...)` 失败时，仍返回刚分配的空 PID list；`pids()` 因此把扫描上下文分配失败或 `opendir("/proc")` 失败错误映射为 “process absent / 0”。`common_ex.c::service_lifecycle_is_running()` 直接使用 `pids()`，DNSMasq locked stop 也由该回调判断进程是否存活；资源耗尽或 procfs inventory 初始化失败时可能出现 stop 假成功、ARP 清理或重复启动。该问题已从源码路径确认，但当前尚未修改、未添加回归。
- root dev_log 现为 35/119；本次新增记录后仍远低于 120 条及 384 KiB early-rotation 门槛。Archive counts 保持 dev-log=8 / handoff=22 / roadmap=18；不轮转、不改 INDEX、不创建快照。FTP final-name inode-check/unlinkat TOCTOU 仍 OPEN/BLOCKED；NFS AUTH_SYS/root-squash/client durability 仍需要真实客户端与存储设备证据。

## Active task

优先治理共享进程 inventory 的 fail-open 缺陷，然后完成 DNSMasq 全生命周期 caller / identity / reap 审计。已确认的具体路径是：`alloc_procps_scan()` 在分配失败或无法打开 `/proc` 时返回 NULL；`procps_scan(NULL, flags)` 随之返回 NULL；`find_pid_by_name()` 的 while 循环直接结束并返回初始空 PID list；`pids()` 将其解释为 0。与此同时，`common_ex.c::service_lifecycle_is_running()` 直接使用 `pids()`，因此 DNSMasq stop 的运行态判断也可能把“无法检查”当成“已停止”。DNSMasq start/readiness 的 `is_dns_dhcpd_run()` 同样基于 `pids()`，资源压力下可能把仍在运行的 daemon 误判为不存在。

下一步：

1. 设计三态 process inventory contract（running / absent / unknown-error），让 procps scan 初始化失败、目录读取错误及 PID list allocation failure 都保留错误状态；审计所有现有 `pids()` 布尔调用点，避免 unknown 被当成已就绪或已停止。
2. 将 DNSMasq start/stop/restart/readiness 与 `service_lifecycle_stop()` 改为 fail-closed：unknown 不得允许重复启动、不得确认 stop 成功、不得清除 static ARP。为 scan context OOM、`/proc` open failure、空 inventory、匹配进程和正常无匹配增加可注入 host regressions / GNU89/Werror tests。
3. 完成 `start_dns_dhcpd()`、`stop_dns_dhcpd()`、`restart_dhcpd()`、`restart_dns()`、locked helpers、`full_restart_lan()` 的全入口 caller map，核对 HTTPD/RC/服务停机路径的持锁边界；再检查 process name / argv identity、stale PID、killall signal、bounded wait/reap、HUP postcondition、FD/temp cleanup 与 config rollback。
4. 继续将治理扩展到全仓用户态，不能长期局限 DNSMasq/HTTPD。FTP final-name cleanup TOCTOU 与 NFS client semantics 按既有 OPEN/BLOCKED 约束处理。

不要重复 DNSMasq DHCP pool、resolver publication lock、lifecycle lock、failed-stop gate、launch postcondition、notification positive-errno normalization 和 full LAN restart failure propagation；也不要全局把 infosvr/crond 的 disabled/no-op 正返回解释为失败。

## Completed in this handoff

1. 按 bootstrap / development protocol、handoff、roadmap、dev_log 的规定顺序恢复项目，并核对 live main、PR、tag 与 exact-head Actions。
2. 复核 DNSMasq public wrapper / locked helper 的当前分层，检查 full LAN restart 的锁范围、stop-before-start gate、启动后置条件，以及 restart 错误是否被 PID 1 notification marker 正确保留。
3. 在恢复期间发现 main 上已有并验证的并发提交：`ed010248` 实现 `full_restart_lan()` 的 DNSMasq 错误返回及 `RCN_RESTART_LAN` 错误合并；`691e4d6` 修正 HTTPD umbrella 中随生产代码变更而过期的 source contract。两项提交由恢复前已并发运行的开发流程写入，本轮没有重复提交相同修改。
4. 检查 exact `ed010248` DNSMasq 与 NVRAM workflow 的完整步骤和日志，二者 SUCCESS；进一步调查该 SHA 上 HTTPD security workflow 的失败根因，确认是旧变量名 source assertion，并检查 `691e4d6` 上 HTTPD 全步骤成功。
5. 更新 live handoff、roadmap 和 root dev_log 的当前 SHA / Actions / 未完成任务；archive indexes 未达到轮转条件，不制造归档快照。

## Verification boundary

- 当前 main `85b06794590a8d3fc7fe4218b2f967493d45d9e3`（parent `18561fddd214238dda124e0b8883030081ffca5d`）为文档 checkpoint commit；产品源码树仍与 L3 已验证的 `691e4d6` 完全一致。
- Exact-main `85b0679` push Actions：6/6 SUCCESS，runs `43492884427`, `43492884404`, `43492884384`, `43492884381`, `43492884353`, `43492884341`；AI docs [`43492884427 / 132118367981`] 全部 steps 成功，validator 检查本次记录前的 root log 34/119。
- Current product source SHA `691e4d68a27580d0f053a8b4c9ae7a71bff03e6f` XY-C1 L3：run `43489646254` / job `132107102854`，MIPSel build、TRX verification、artifact upload 全部 SUCCESS。Artifact `14069572898`，12,579,578 bytes，SHA256 `c6581125da108ac1118dfcaf895bcfb2414e09554eb63b96be8bddb369c7b768`；TRX image 12,625,949 bytes。它覆盖当前 main 的全部产品源码；`18561f` 仅改文档，故当前代码可记 L3 VERIFIED。L4 physical router flash/boot NOT VERIFIED。
- Exact `ed010248` DNSMasq run [`43489264442 / 132105772776`] 与 NVRAM audit [`43489264608 / 132105772928`] SUCCESS。Exact `691e4d6` HTTPD umbrella [`43489646217 / 132107102491`] SUCCESS；此前的 stale source assertion 已修正。
- 本环境未执行本地交叉编译；L3 是 GitHub Actions 在 exact product-source SHA 上完成的 MIPSel firmware/TRX build。没有执行 L4 router flash/boot。当前新增的 `pids()` fail-open 缺陷仅有源码级证据，尚未修复、未添加 host regression，不得写成已关闭。
- Root dev_log 从 34 增至 35/119；archive counts dev-log=8 / handoff=22 / roadmap=18。FTP TOCTOU / NFS client durability 仍 OPEN/BLOCKED。

## Exact first action for the next GPT

1. 首先 refresh `refs/heads/main` 和 exact SHA / event=push Actions；确认是否有新提交，以及本次 handoff/doc validator run 是否通过。
2. 以 `trunk/user/shared/src/pids.c` 为首要任务：实现 process inventory tri-state，覆盖 `alloc_procps_scan()` / 初始 `procps_scan()` 失败与 PID list 分配失败，避免 unknown 被映射为空列表；先审计所有 callers，再选择兼容的 API 迁移方式。
3. 为失败路径增加可注入 host regression（OOM / failed-open / normal-empty / matched process）和 GNU89/Werror CI；确保 DNSMasq start/readiness 和 service stop 在 unknown 状态下都 fail closed，再审核其他 daemon start guards。
4. 接着完成 DNSMasq 全 caller map / process identity / stop HUP / wait-reap / fd-temp cleanup；按优先级扩大至其他用户态模块。
5. 当前产品源码 L3 已在 `691e4d6` 成功，L4 仍 NOT VERIFIED；未实际操作路由器前不得宣称 L4。FTP TOCTOU / NFS client semantics 保持 OPEN/BLOCKED。

## Completed task: DNS Forwarder lifecycle serialization and failure propagation
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

