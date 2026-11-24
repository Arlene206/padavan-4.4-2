## 2026-11-23 持续开发接力（最新）

- 当前 `main`：`828c5bac2fec6e110188615d0ae9ec0984501b84`（test(raeth): repair compile integrity checker）；备份分支 `backup/pre-plugin-overhaul-2026-11-14`：`72f4f157a084f8184b9597ca5bf334b8080e5807`，本轮未改动。
- 当前 HEAD 精确 SHA Actions：Switch CLI ioctl failure-status regression: completed/success (run 45313956637)。
- AliyunDrive WebDAV 启动就绪/清理回归（提交 `3160f80e4e2c82d2188bddc12f88589898a79327`）：AI handoff documentation contract: completed/success (run 45312872263); Process lifecycle inventory - 3160f80e4e2c82d2188bddc12f88589898a79327: completed/success (run 45312872116); AliyunDrive WebDAV lifecycle stability regression: completed/failure (run 45312872098)。若失败，优先根据 QEMU 测试日志定位“启动后未就绪”的进程回收路径；不要仅降低测试断言或延长超时掩盖泄漏。
- RAETH 编译完整性（提交 `8d0f9325ec63223b442b1aff4f95dc7566d3f421`）：AI handoff documentation contract: completed/success (run 45313394069); Switch CLI ioctl failure-status regression: completed/failure (run 45313394047)。已发现并追加修复 `scripts/check_raeth_compile_integrity.py`：重写定义扫描器，使其显式区分函数原型和定义，并自测重复定义、缺失定义和返回类型契约；需以本条记录中的 HEAD Actions 结果验证。
- 自 `8d0f9325...` 以来的新提交：
- 564c4c239f006a23bb504eba34d6a113c9cdb813: test: harden Aliyun and RAETH regression checks
- 828c5bac2fec6e110188615d0ae9ec0984501b84: test(raeth): repair compile integrity checker
- 本轮已推进的 RAETH 修复：switch GPIO/SGMII/FE switch init/RTL8367/Leopard/GMII/probe MMIO 映射失败均应 fail closed 并传播错误；HW-IOC PMU 命令索引、参数、事件、读取映射也有边界验证与回归测试。此前提交 SHA 见 Git 历史，不要重复审计已完成切片。
- 第一项后续任务：先读取本节精确 SHA 的 Actions 最终状态。若 RAETH compile integrity 仍 FAIL，检查这次定义扫描器与真实函数签名差异后修复；若 PASS，继续 XY-C1 构建/对应 SHA 验证。并行处理 AliyunDrive lifecycle QEMU 失败（若仍失败），必须依据失败日志修复真实启动/清理问题，不得伪造 PASS。
- 任何未返回的 run 均视为 PENDING/未验证；不能将较早 SHA 的 PASS 推断为当前 HEAD PASS。XY-C1 全量固件构建如无当前 SHA 对应 run，明确记录未验证。

---

# Session Handoff

## Current repository state

- Current parent `8d0f9325ec63223b442b1aff4f95dc7566d3f421`; production RAETH DT-selector fix is `7f7aee3b5df94c6996743e58834ee2ec4d67eafd`. Backup branch remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Switch CLI #45313394047 on `8d0f932` failed in `check_raeth_compile_integrity.py`: the checker searched only `void` definitions and did not count the intentional `static int fe_chip_name_config()`. THIS COMMIT corrects the definition matcher; exact rerun needed.
- AliyunDrive lifecycle #45312872098 on `3160f80` failed at the combined HTTP-503 cleanup assertion after its QEMU CLI compatibility check passed. Inspection found the fixture assumed exactly 11 probes despite asynchronous child-marker publication; THIS COMMIT makes the bounded probe expectation race-safe and prints observed counters. Cleanup behavior still needs a green exact-head test.
- XY-C1 #45313243920 on `7f7aee3` and NVRAM #45313243814 were **IN_PROGRESS** at last observation. The earlier `dd0b374` target build is a different source SHA.

## Active task

Verify this commit's exact-head Switch CLI source checker with the status-bearing `fe_chip_name_config()` definition, and the AliyunDrive lifecycle job's 503 cleanup regression with its diagnostic counters. Then inspect XY-C1/NVRAM Actions on `7f7aee3` only; do not transfer results to later commits.

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

- RAETH Switch CLI #45313394047 on `8d0f932` **FAIL** at source checker: its function regex did not count `static int`; no source-contract PASS is claimed.
- AliyunDrive lifecycle #45312872098 on `3160f80` **FAIL** at new fixture's combined assertion. The original log did not print per-counter values; THIS COMMIT makes the assertion race-safe and adds those diagnostics. No lifecycle PASS is claimed.
- XY-C1 #45313243920 / NVRAM #45313243814 belong to source `7f7aee3` and were in progress at last observation. No L3/L4 result is claimed.

## Exact first action for the next GPT

1. Check the exact push Switch CLI and AI documentation Actions for this commit; inspect the new RAETH checker output.
2. Check the exact push AliyunDrive lifecycle job and inspect the 503 cleanup counters. Fix source behavior if TERM is still not issued or marker cleanup fails.
3. Inspect XY-C1 #45313243920 and NVRAM #45313243814 on `7f7aee3`; keep these results separate from current HEAD.


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
