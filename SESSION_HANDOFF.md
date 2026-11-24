## 2026-11-24 持续开发接力（最新）

- 当前 main 接力基线：`3e13e241219d08b26aeb57795f553504783c1b06`；归档备份 `backup/pre-plugin-overhaul-2026-11-14` 保持只读（`72f4f157a084f8184b9597ca5bf334b8080e5807`）。
- LAN DHCP `lan_dns_x` 严格解析修复 `17441ec` 的 WAN DHCP #45321875664 PASS、AI docs #45321875753 PASS；HTTPD、Firewall、process inventory、NVRAM、WAN route、signal regressions 均 PASS。
- RAETH MT7622 open-failure switch cleanup 与 `ei_close()` worker 停止顺序合入 `7627383`；精确 Switch CLI #45322123792 PASS。
- 共享 PID-file 的 owner/nlink/mode hardening 已合入 `3e13e241219d08b26aeb57795f553504783c1b06`；legacy wrapper PIDfile #45322580536 PASS。但 process lifecycle regression #45322580591 的 strict GNU89 compile 在 `S_ISREG` 未被 glibc 暴露时 FAIL；本提交将 helper 改成可在 strict GNU89 下使用的 `S_IFMT/S_IFREG` 比较。其余 exact-SHA jobs 要重新按修复提交观察。
- XY-C1 #45322580624 的 Build job仍在进行（Build XY-C1 / TRX / artifact upload 尚 pending at last poll）。无同 SHA firmware artifact 或硬件 L4 结果可提前宣称。
- Roadmap 维持低于 131072 UTF-8 byte 上限；备份分支只读。

---

# Session Handoff

## Current repository state

- Parent main before this compatibility correction: `3e13e241219d08b26aeb57795f553504783c1b06`; frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- `3e13e24` introduced legacy PID-file metadata rejection (effective-UID owner, single link, no group/world write) and regression cases. Legacy wrapper PID-file run #45322580536 PASS.
- Process lifecycle run #45322580591 failed in its compile step for `shutils_pidfile_policy.c`: `S_ISREG` was not defined by libc under that strict `-std=gnu89` test compile. The policy now checks `(st_mode & S_IFMT) == S_IFREG`, using the POSIX mode constants already exposed by the platform headers; checker contract was updated accordingly.
- Exact SHA XY-C1 #45322580624 and NVRAM #45322580583 were IN_PROGRESS on `3e13e24`; statuses do not validate this correction.
- Backup branch remains read-only; no cross-SHA status transfer.
## Active task

Verify strict GNU89/ASan/UBSan process lifecycle regression and legacy-wrapper PID-file safety after the `S_IFMT` compatibility correction. Then inspect XY-C1 Build → TRX → artifact status on the newest product SHA; continue with an independent defect if builds remain pending.
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

- The original `3e13e24` metadata hardening passed the legacy-wrapper PID-file workflow #45322580536.
- The process lifecycle regression #45322580591 failed only while compiling the header test under strict GNU89 because `S_ISREG` was not exposed. This commit changes the regular-file predicate to `S_IFMT/S_IFREG`; a fresh exact-SHA run is required.
- The other 3e13e24 jobs: AI docs #45322580538, WAN DHCP #45322580657, HTTPD #45322580600, GPIO #45322580616, DNS Forwarder #45322580636, process lifecycle inventory #45322580712 were observed PASS; XY-C1 #45322580624 and NVRAM #45322580583 were pending.
- No local router/MIPS build, TRX, artifact or hardware L4 result is claimed.
## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-SHA process lifecycle, legacy-wrapper PID-file, AI docs, NVRAM and XY-C1 Actions for the strict-GNU89 compatibility correction.
2. Confirm the metadata test passes with `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` and production source contract.
3. Inspect XY-C1 Build, TRX verification and artifact upload on the latest exact product SHA; retain pending state if not done.
4. Continue another independent stability issue if Actions are pending; keep backup read-only and Roadmap below byte limit.

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
