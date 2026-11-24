## 2026-11-24 持续开发接力（最新）

- 当前 main 接力基线：`69c15a20e8ce91bf8c31e3ac600547421ba3bfd1`；归档备份 `backup/pre-plugin-overhaul-2026-11-14` 保持只读（`72f4f157a084f8184b9597ca5bf334b8080e5807`）。
- RAETH RX buffer 修复在 `a52ded9e6c0982ff7b838a59b4fb62df3fbd1815`，精确 SHA Switch CLI #45321100540 PASS。后续 RAETH 生命周期提交补齐 switch platform-device 引用释放、pseudo-netdev 注册失败回收和 NAPI 注销；#45321494411 曾由错误的 `register_netdev()` 文本断言误报，精确计数断言已在父分支修正。
- 本提交修复 LAN DHCP deconfig 缺陷：`lan_down_auto()` 原先对快照中的 `lan_dns_x` 使用 `atoi()`，而 lease resolver 发布路径使用严格整数解析；畸形值 `0;reboot` 会被 atoi 当成 0，错误切换到静态 DNS 并覆盖 resolver。修复在 route/interface/filesystem 副作用前严格解析，失败即保留现有 lease 状态。
- `e22aea0d3000117892d82992a9d88b96fe179599` 的 AI handoff workflow #45321272727 PASS；Roadmap 低于 131072 UTF-8 bytes，不要无必要扩写。
- 不得把其他 SHA 的 PASS 转移到当前提交；未拿到同 SHA XY-C1 Build、TRX 和 artifact 结果前，不宣称 L3 成功。无物理硬件 L4 结果。

---

# Session Handoff

## Current repository state

- Parent main before this implementation commit: `69c15a20e8ce91bf8c31e3ac600547421ba3bfd1`; frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- `lan_down_auto()` now parses the snapshotted `lan_dns_x` through the same `rc_wan_nvram_parse_int_value()` routine as `lan_publish_resolver_auto()`, before route deletion, interface reconfiguration, resolver staging or file publication. Malformed/overflow selector values fail closed and preserve the last committed lease.
- The existing Python source regression pins the strict parser, failure return and ordering before route side effects. The existing WAN DHCP callback workflow includes both changed paths.
- Exact-SHA RAETH follow-up checks are independent of this LAN fix. AI docs #45321272727 on `e22aea0` PASS; Roadmap remains below 131072 UTF-8 bytes. Backup stays read-only.
## Active task

Verify the exact-SHA WAN DHCP callback regression for strict LAN DNS-mode parsing, especially that malformed `lan_dns_x` is rejected before route/interface/resolver effects. In parallel inspect the latest RAETH Switch CLI rerun and XY-C1 Build/TRX/artifact status. Continue an independent stability fix while any jobs remain pending.
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

- RAETH RX sizing source fix is in `a52ded9`; its exact Switch CLI run #45321100540 PASS.
- The pseudo-registration checker defect from Switch CLI #45321494411 was corrected in a parent commit; confirm the parent SHA's rerun independently.
- This commit adds strict LAN `lan_dns_x` parsing before any DHCP deconfig route/interface/resolver side effect and updates the existing source contract. No run for this SHA exists at commit creation.
- AI handoff validation passed on `e22aea0`; do not grow `REFACTOR_ROADMAP.md` beyond 131072 bytes. No local MIPS firmware, TRX, artifact or hardware-L4 result is claimed.
## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-SHA WAN DHCP callback workflow and Switch CLI, plus XY-C1 Build → TRX → artifact results. Keep results SHA-scoped.
2. Confirm malformed `lan_dns_x` is rejected before any route, interface, resolver-stage or metadata mutation; diagnose the first failing assertion if not.
3. Continue repository-wide stability fixes while exact-SHA jobs are pending. Keep the backup branch untouched and preserve the roadmap byte limit.

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
