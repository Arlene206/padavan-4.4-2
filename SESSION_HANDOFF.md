## 2026-11-24 持续开发接力（最新）

- 当前 main 接力基线：`dd5e71103ee4ea1e2b312d358f253f662c0ddc0f`；归档备份 `backup/pre-plugin-overhaul-2026-11-14` 保持只读（`72f4f157a084f8184b9597ca5bf334b8080e5807`）。
- LAN DHCP deconfig 的 `lan_dns_x` 已从 `atoi()` 统一改为严格整数解析，修复在 `17441ec283ac5b3da5b5e5ea7859f905a33e2c48`；该 SHA WAN DHCP callback #45321875664 PASS，AI docs #45321875753 PASS，HTTPD security #45321875813 PASS，Firewall #45321875825 PASS，process/NVRAM/route/signal regressions 均 PASS。
- 最近 RAETH 原子修复：MT7622 open 初始化失败时回收 switch 状态；`ei_close()` 先 stop kreset/kphy-poll workers、停 TX queue/RA2880，再复位硬件，防止后台线程与 reset 并发。`7627383723a2c33bc6ff7456384ec248db3236fa` Switch CLI #45322123792 PASS。
- 新发现并修复的共享 pidfile 安全问题：旧 `kill_pidfile_s()` 只拒绝 symlink/FIFO，并未验证 PID 文件属主、硬链接数和 group/world 写权限。该 helper 随调用方权限执行 `kill(pid, sig)`，因此被其他 uid 控制的普通文件可能诱导 root 管理进程向任意 PID 发送信号。当前切片要求常规文件由 effective uid 所有、单一硬链接且不允许 group/world 写，并增加回归与 CI 源码契约。
- exact SHA Actions 必须各自核对；没有同 SHA XY-C1 Build、TRX 和 artifact 全部成功前，不得宣称固件验证完成。Roadmap 保持低于 131072 UTF-8 byte 上限，无硬件 L4 结果。

---

# Session Handoff

## Current repository state

- Parent main before this implementation commit: `dd5e71103ee4ea1e2b312d358f253f662c0ddc0f`; frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- `17441ec` LAN DHCP strict `lan_dns_x` parser: exact WAN DHCP #45321875664 PASS; AI docs #45321875753 PASS; HTTPD #45321875813 PASS; Firewall #45321875825 PASS; process lifecycle #45321875859 PASS; WAN route #45321876067 PASS; NVRAM audit #45321875820 PASS. XY-C1 #45321875660 remained IN_PROGRESS at last parent poll.
- `7627383` RAETH worker shutdown ordering: exact Switch CLI #45322123792 PASS. Exact XY-C1 #45322123939 and NVRAM audit #45322123748 were still IN_PROGRESS at last poll.
- This commit hardens legacy `kill_pidfile_s()` by rejecting PID files not owned by `geteuid()`, files with `st_nlink != 1`, and files with group/world write permission. Symlink and nonregular-file rejection remain. Tests cover each metadata predicate.
- Backup branch remains read-only; no result from older SHAs may be transferred to this parent or subsequent commits.
## Active task

Run the exact-SHA legacy PID-file safety workflow and broader process lifecycle regression. Confirm the metadata predicate rejects wrong owner, hardlinks, group/world-writable files and non-regular files while accepting an owned 0600 regular PID file. Check XY-C1 Build → TRX → artifact on the current product SHA; continue independent stability work if still pending.
## Completed development slices

### HTTPD / web UI security

- `09576b630122b31bed95c161223f05a43f0cdd6f`: group compaction checks transaction-locked count against caller snapshot before row mutation; malformed, stale, negative and overflow counters fail closed.
- `beb57775ac957fb79a509f8fca9f22ee74510949`: HTML-escapes removable-media folder names at every AiDisk folder-tree text/title sink; Node VM XSS regression.
- `c89d1d777494dd13384270014c080b62c1b9f56a`, `6262b52ab098f98ceeb7466261a5916c1b4ed79e`, `3e02326eae477850f0f98fe74764b59f42c64baf`, `0136ceb43575bc4367fc288a3524ead11fcad7b7`: group Add is transaction-locked, snapshots row/cache/count, clears delete caches, publishes count last, rolls back failures, distinguishes incomplete rollback and returns HTTP 500. Fault injection covers malformed/full/missing counts, count/row read errors, partial writes and rollback failure.
- `96431bbdf8dafedf38db929ef31f9990ec9a2ab8`, `2b8333e65915adbfc7e617b81ded751b7e2a253b`, `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1`: group-delete parser consumes the full raw value, rejects oversized/truncated/malformed indexes and preserves exact legacy `-1`. Regression uses a 255-digit zero-padded prefix plus untrusted suffix.
- `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`: added `nvram_get_js`, a complete JS string literal callback reusing the tested JS writer; classified as `HTTPD_DIRECT_EJ_ESCAPED_OUTPUT`. Fixed stored XSS in DDNS hostname JS initialization.
- `6cb5c16818eb5a91382a97365692f58105e4b5c2`, `2a50ce0c6653de44fe071c13049747b406d49f3c`: fixed the HTTPD workflow source extraction and exact DDNS template assertion.
- `51bd80eff75efd16fc49e46ac612b2dc39ea41ae`: applied JS-context escaping to WAN, LAN, DHCP, IPTV, Services, SettingBackup and MACFilter ASP pages, with per-template workflow source assertions. HTML contexts remain on the HTML escape path.

- NVRAM HTML/JavaScript output-context hardening (this commit): finished the 174-asset audit by migrating the remaining 89 JS-source `nvram_get_x` sinks; all 117 such sinks are now protected across `51bd80e`, `453aefd` and this slice. The three DDNS JS callsites use the JS-string writer, and generic HTML NVRAM output escapes apostrophes. A GNU89 ASan/UBSan byte regression and a source-template context checker are wired into HTTPD push/PR CI.

- WOL UI DOM XSS hardening (this commit): build client rows as DOM elements and render client MAC/name and vendor names only through jQuery `.text()`. Vendor API results are normalized to a string before rendering/caching. The HTTPD checker now rejects raw row concatenation of client data and `.html()` sinks for vendor labels.
- Syslog HTML output hardening (`674adb3`, verified by `28613f4`): route `/tmp/syslog.log` emitted by both `nvram_dump()` and `dump_syslog()` through the shared HTML byte encoder. The regression includes a `</textarea><script>` payload; exact HTTPD `#45302967232` and AI handoff `#45302967200` on `28613f4`: **PASS**.
- Language-option HTML output hardening (`a01de6b`): dictionary-derived labels pass through `write_html_text()`, while `<option value>` uses `write_html_attribute()`. Exact HTTPD `#45303339945` and AI `#45303340000`: **PASS**.

- Wireless scan/BSSID JSON hardening (`8f1d1d5`): serialize nearby AP SSIDs, BSSID/channel/signal strings and both NVRAM-sourced BSSID JS values using `httpd_json_escape_next()`; the AP-client caption now uses a constant `%s` format. Exact HTTPD `#45304169207`, AI `#45304169314` and process inventory `#45304169499`: **PASS**.
- Uptime format/output hardening (`f3c83db`): capture `rfctime()` and serialize it through `write_html_text()` rather than passing a dynamic value as `websWrite()`'s format argument. Exact HTTPD `#45303585359` and AI `#45303585354`: **PASS**. All non-XY-C1 gates on this SHA pass; XY-C1 `#45303585432` remains IN_PROGRESS.
- AiDisk JavaScript output hardening (code committed in `eae527b` and `cd7e54d`): all 48 dictionary-backed action-error messages are serialized through `write_script_message()` and the shared JSON encoder; USB enum and modem transport labels now use the same string encoder. Exact `1027e90` HTTPD/AI checks pass; the `cd7e54d` child Actions remain pending.

### IPTV / process lifecycle

Independent snapshot/lifecycle changes remain in `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, and `57c466d3001da525511160fc30418a774b99aa85`. The too-short overflow fixture and stale checker expectations were corrected; historical exact-SHA WAN/process/storage/XY-C1 results remain in earlier dev-log entries.

## Verification boundary

- LAN DNS-mode strict parsing is in `17441ec`; WAN DHCP callback #45321875664 PASS and AI docs #45321875753 PASS.
- RAETH close worker ordering is in `7627383`; exact Switch CLI #45322123792 PASS.
- Current commit changes legacy PID-file metadata policy, production `kill_pidfile_s()` enforcement, sanitizer regression coverage and source contracts. No Actions for the new SHA exist at commit creation.
- Parent product SHA `7627383`: XY-C1 #45322123939 was IN_PROGRESS; NVRAM audit #45322123748 was IN_PROGRESS. No local MIPS build/TRX/artifact/hardware result is claimed.
- Keep Roadmap below 131072 UTF-8 bytes; no hardware L4 results.
## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-SHA legacy wrapper PID-file safety, process lifecycle regression, XY-C1 and NVRAM runs.
2. Ensure PID-file metadata hardening passes GNU89 `-Wall -Wextra -Werror` with ASan/UBSan and source-contract checks; diagnose regressions before moving on.
3. Check exact product SHA XY-C1 Build → TRX verification → artifact upload. Do not transfer statuses across SHAs.
4. Continue another independent stability/security fix while pending jobs run. Keep backup read-only and the Roadmap byte limit intact.

# Cross-session execution mandate

Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on `main`; each logical source or CI change is one atomic main commit: create all changed blobs and one complete Git tree before creating that commit, then update `main` using a non-forced expected-SHA compare-and-swap. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
