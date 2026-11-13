## Current repository state

## 最近接力状态（2026-11-14）

- 最新 main：THIS COMMIT；父 SHA 为 `a64c9923a24550177c76f88ea81694725a0230bd`。本批代码和测试以普通 fast-forward 提交到 main，不改写历史。恢复时 main 只有一个分支；PR #1 仍 CLOSED / UNMERGED（head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`）；toolchain tag `7f7d3ea41ebdd7eeab8a303f3d4ad8dddb54a8a7` 未修改。
- 之前遗留的 XY-C1 L3 检查已完成：功能源码 SHA `2d5f44b1978c3272fa85633e273297c16bd34283` 的 run `43476679718` / job `132062194706` 完成 Build XY-C1、TRX verification、artifact upload；artifact `14064545568`，12,580,294 bytes，SHA256 `0fba859e6561464feaade2813bd2104d6e789c3829c4f5b30cb5875fa2d40e4d`。这是 2d5f44b 的 L3 证据，不代表本次新改动已完成 L3。
- Storage standalone-stop slice：六个对外 stop API（FTP、Samba、MiniDLNA、Firefly、Transmission、Aria）现在各自先取得共享 storage-operation fcntl lock，再调用私有 `*_locked()` helper；取锁失败即拒绝停止。六个 restart wrapper 和已经持有全局锁的 `stop_stor_apps()` 改为调用私有 helper，避免同进程对同一 fcntl 锁递归 lock/unlock 破坏外层互斥。
- `stop_ftpsamba` 改为一个原子停止事务：只取一次共享锁，依次尝试 FTP 与 Samba，即使 FTP stop 失败也继续尝试 Samba，并保留第一个非零失败结果；只有两个服务都停止成功才返回成功。RC 对 MiniDLNA、Firefly、Transmission、Aria 的 stop applet 也不再丢弃返回状态。
- Storage-hotplug、vsftpd、Samba 三条 workflow 的 source contracts 已同步调整：覆盖 public wrapper lock/fail-closed/单次 unlock、private helper 使用、已持锁调用无递归、组合停止顺序/错误合并和 RC 状态传播。path filter 的 push / pull_request 均保留。
- 上一主线 DNS resolver 修复仍有效：`fill_dnsmasq_servers()`、`update_resolvconf()`、`lan_up_manual()` 在共享锁失败时拒绝 staging/publication/路由副作用；LAN auto 和 DHCP lease deconfig 原有 guard 保留。3ef3eed exact-head DNSMasq run `43476935451` / job `132063063482` 和 AI docs run `43476935879` / job `132063065994` 均 SUCCESS。
- 本次代码提交触发后的 Actions 需按新 SHA 单独核对：storage-hotplug、vsftpd、Samba、AI docs，以及 XY-C1。当前提交的 L2/L3 结果在本 handoff 形成时尚未核验；不得用 a64、3ef3eed 或 2d5f44b 的历史结果代替。
- 本地使用 `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` 对现有 stop postcondition + 新增 group-status merge policy 回归进行了独立编译运行，PASS；源码调用图和三个 workflow 新增断言完成 preflight，PASS。没有完整本地 checkout，也没有完整固件编译；L4 实机验证从未执行。
- 根 `dev_log.md` 本次更新后为 29/119，未达到 120 条或 384 KiB 轮转门槛；不新增归档，不修改 `docs/dev-log/INDEX.md`。FTP upload failed-final-name cleanup 的 inode-check / unlinkat TOCTOU 仍 OPEN/BLOCKED；NFS AUTH_SYS/root-squash/client durability 仍需真实客户端/设备证据。

## Active task

本批已完成前一 handoff 指定的 standalone storage stop applet 串行化源码切片。下一步先核实 THIS COMMIT 的全部精确 SHA push Actions（尤其 storage-hotplug、vsftpd、Samba、XY-C1 与 AI docs），再从全仓库 P2 台账转向 DNS Forwarder/DNSMasq lifecycle 与进程监督审计，继续扩大治理模块范围。

- 已把 `stop_ftp`、`stop_samba`、`stop_dms`、`stop_itunes`、`stop_torrent`、`stop_aria` 拆为 public lock-taking wrapper + private `*_locked()` helper。restart wrappers 和 `stop_stor_apps()` 复用 private helper；任何已持锁路径不得再次调用 public wrapper。
- `stop_ftpsamba()` 负责一个锁内的 FTP+Samba group stop；保留首个失败状态同时继续尝试第二个服务。RC applet 的 stop 退出码必须继续经 `main()` 返回给调用者。
- 新增/更新的 host regression 与 source contracts 已在本次提交中；精确 Actions 尚待核验。若 workflow failure，先区分 C source/build 失败与 source-check/path-filter harness 失败，记录真实失败并在 main 上修复，不得将旧 SHA 的绿灯当作新 SHA 证据。
- DNS Forwarder/DNSMasq 下一轮关注 start/stop postcondition、reload/启动并发、stale PID、child wait/reap、fd/临时文件回收、错误回滚；不要重复已完成的 dservers/resolv 锁失败保护。
- FTP failed-final-name cleanup 的 inode-check/unlinkat TOCTOU 继续 OPEN/BLOCKED；未证明 Linux 4.4 上安全的 inode-conditional unlink 语义前不得更改该路径。NFS 客户端 durability / AUTH_SYS / root-squash 仍需设备/客户端证据。

## Completed in this handoff

1. 按规定顺序读取 `AI_BOOTSTRAP_PROMPT.md` → `AI_DEVELOPMENT_PROTOCOL.md` → `SESSION_HANDOFF.md` / `REFACTOR_ROADMAP.md` / `dev_log.md`，核对 main、remote refs、PR 与历史 Actions 的精确 SHA。
2. 刷新并完成上一 handoff 未结束的 XY-C1 build/TRX 任务：2d5f44b 的 exact-source job `132062194706` / run `43476679718` SUCCESS，保存 artifact ID、大小和 SHA256；明确该 L3 结果不覆盖本批源码。
3. 完成六个 storage-backed public stop wrapper 的共享锁保护，并拆出六个 `*_locked()` 私有 helper；restart 与已持锁的 storage teardown 调用 private helper，避免 fcntl 锁重入导致外层互斥提前释放。
4. 将 FTP+Samba grouped stop 改为单次获取共享锁并尽力停止两项服务，保留首个非零状态；RC 的 add-on stop applet 现在传递 stop 结果，不再无声成功。
5. 扩充 storage stop group-status host policy regression；更新 storage-hotplug、vsftpd、Samba workflows 的 public/private lock contracts、RC 状态传播断言，并保留 push/PR path filters。
6. 本地 GNU89/Werror/ASan/UBSan stop-policy host regression 和源码/调用图 preflight 均 PASS；本批 exact-head Actions、当前代码 SHA 的 XY-C1 L3 需在本次提交触发后核实。

## Verification boundary

- 本次提交父 SHA：`a64c9923a24550177c76f88ea81694725a0230bd`；本次 main 结果 SHA：THIS COMMIT。没有重写历史或改动非 main refs。
- Local host result：`storage_service_stop_policy.c` 使用 GNU89、`-Wall -Wextra -Werror`、ASan/UBSan 和 frame pointers 编译并运行，输出 `storage daemon stop postcondition and group result policy: PASS`。源码 preflight 确认六个 public stop wrapper 各有单次 unlock、锁失败 fail-closed，六个内部已持锁调用均使用 private helper，`stop_ftpsamba()` 单锁覆盖两项服务，RC 返回值传播正确。
- Workflow changes：`.github/workflows/storage-hotplug-stability.yml`、`vsftpd-stability.yml`、`samba-share-stability.yml`。这三个 workflow 对本次 SHA 的 push run/job steps 必须逐项读取；AI handoff documentation 和 XY-C1 firmware validation 也必须按 exact head 核验。当前提交的 Actions 结果在提交时 PENDING。
- Previous source L3：2d5f44b run `43476679718` / job `132062194706` SUCCESS；XY-C1 artifact `14064545568`（12,580,294 bytes；SHA256 `0fba859e6561464feaade2813bd2104d6e789c3829c4f5b30cb5875fa2d40e4d`）。不能视作本次 stop-wrapper 源码的 L3。
- Exact-head baseline：3ef3eed DNSMasq `43476935451` / job `132063063482` 与 AI docs `43476935879` / job `132063065994` SUCCESS；a64c9923 的 infosvr、AI docs、AliyunDrive、AdGuardHome、AliDDNS、NVRAM push workflows 全部 SUCCESS。这些均为历史基线，不替代本次 SHA 的验证。
- 没有完整本地 checkout、完整 all-board build 或 L4 router flash/boot/runtime 结果。FTP cleanup TOCTOU 和 NFS 客户端语义维持 OPEN/BLOCKED。

## Exact first action for the next GPT

1. 重新读取 main HEAD，精确记录新 SHA；列出 `event=push` 且 `head_sha` 完全相等的所有 Actions runs。重点检查 storage-hotplug、vsftpd、Samba、AI handoff docs 和 XY-C1；对每个 run 读取 jobs 与 steps，检查完整结论和 artifact/TRX SHA256。若某条路径未触发，确认 path filters，不能以祖先 SHA 代替。
2. 如有失败，判断是生产源码、host test、source checker 还是 workflow path-filter 问题；保存失败证据，在 main 上做最小修复并重新核验新 SHA。不得隐瞒第一次失败。
3. 全部 L2 结果确认后，转入 P2 DNS Forwarder/DNSMasq lifecycle 与 child/process supervision 审计，检查启动/停止并发、stale PID、wait/reap、文件描述符和临时资源回收；扩大到其他用户态模块，不长期停留在 storage/HTTPD。
4. 保持 FTP failed-final-name cleanup TOCTOU OPEN/BLOCKED，除非能证明 Linux 4.4 下真实安全的 inode-conditional unlink；NFS AUTH_SYS/root-squash/durability 继续等待真实 client/device evidence。

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

