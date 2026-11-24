## 2026-11-23 持续开发接力（最新）

- 本次修复的父提交 `8855dff9d6e23b00e6922f242d64a36764615b01`；备份分支 `backup/pre-plugin-overhaul-2026-11-14` 保持只读，SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`。
- RAETH compile-integrity checker 修复 `828c5bac2fec6e110188615d0ae9ec0984501b84`：Switch CLI #45313956637 **PASS**，覆盖 `static int fe_chip_name_config()` 唯一定义扫描及 DT selector/probe 错误传播契约。
- AliyunDrive #45313731223（源 SHA `564c4c2`）的 CLI/QEMU 和全部 shell lifecycle regression（含新启动实例 HTTP 503 后 TERM 清理）均通过；失败发生在 Python 源码契约检查器，其仍断言旧错误文案。THIS COMMIT 已将契约改为匹配当前测试的 probe 上下界、完整等待、单次 TERM、无 KILL、marker 清理和诊断文案。新 SHA 的 lifecycle Actions 尚待验证。
- RAETH selector 生产修复 `7f7aee3b5df94c6996743e58834ee2ec4d67eafd`：AI #45313243914 与 NVRAM #45313243814 **PASS**；XY-C1 #45313243920 最近仍为 **IN_PROGRESS**。不能用更早的 `dd0b374` 构建成功替代该 SHA 的产品构建。
- XY-C1 L3 已在 `dd0b374694cf9d1e99ca65c8b35fa53b482d377b` 验证：Build、TRX 和 artifact 上传成功，artifact #14884935723，SHA-256 `fe89a388f85d0487f26be72a609cd430b1bd72c94ca569dacd0c956cb3f99c12`。此结果仅属于 `dd0b374`。
- 第一项后续：检查本提交 exact-SHA AliyunDrive lifecycle 与 AI handoff Actions；然后核对 `7f7aee3` 的 XY-C1 构建最终状态。之后处理 RAETH `fe_tx_rx_dec()` 未检查 DT 属性读取和 `kmalloc_array()` 失败的分配/清理路径。
- 所有 CI 状态必须附带实际 HEAD SHA；PENDING 不算 PASS。备份分支不修改，不做 force push。

---

# Session Handoff

## Current repository state

- Parent main for the AliyunDrive static-contract fix: `8855dff9d6e23b00e6922f242d64a36764615b01`; backup `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- RAETH checker repair `828c5bac2fec6e110188615d0ae9ec0984501b84`: Switch CLI #45313956637 **PASS**.
- AliyunDrive lifecycle #45313731223 on `564c4c` passed bundled CLI compatibility and all shell lifecycle cases, including HTTP-503 newly launched daemon cleanup, but failed in the Python checker due to obsolete expected text. THIS COMMIT updates the static contract; exact rerun required.
- RAETH selector change `7f7aee3b5df94c6996743e58834ee2ec4d67eafd`: AI #45313243914 and NVRAM #45313243814 **PASS**; Switch CLI #45313244038 was a checker false-failure superseded by #45313956637 on later HEAD; XY-C1 #45313243920 was **IN_PROGRESS** at last poll.
- XY-C1 L3 build/TRX/artifact is complete only for `dd0b374694cf9d1e99ca65c8b35fa53b482d377b`, artifact #14884935723 with SHA-256 `fe89a388f85d0487f26be72a609cd430b1bd72c94ca569dacd0c956cb3f99c12`.

## Active task

Verify the AliyunDrive lifecycle regression and corrected Python source-contract checker on this commit's exact SHA. Confirm the HTTP-503 newly launched child reaches bounded TERM cleanup, with no KILL and no remaining process marker. Separately poll XY-C1 #45313243920 only for product source `7f7aee3`; do not transfer the earlier `dd0b374` L3 result.

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

- RAETH checker repair #45313956637 on `828c5bac2fec6e110188615d0ae9ec0984501b84`: **PASS**.
- AliyunDrive #45313731223 on `564c4c`: bundled CLI and shell lifecycle tests **PASS**; Python static contract **FAIL** due to obsolete expected fixture string. THIS COMMIT corrects the contract; no Aliyun lifecycle PASS is claimed yet.
- XY-C1 #45313243920 on `7f7aee3` was **IN_PROGRESS** at last check. L3 Build/TRX/artifact success is confirmed only on `dd0b374`, artifact #14884935723. No L4 router validation is claimed.

## Exact first action for the next GPT

1. Inspect this commit's exact-SHA AliyunDrive lifecycle and AI handoff Actions; separate shell test output from Python contract checker.
2. Poll XY-C1 #45313243920 on `7f7aee3` and confirm Build/TRX/artifact on that exact source SHA before L3 claims.
3. Continue with RAETH `fe_tx_rx_dec()` DT-property validation and checked partial-allocation cleanup. Keep archival backup unchanged.


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
