## 2026-11-24 持续开发接力（最新）

<!-- 2026-11-23 RAETH CONTINUATION SNAPSHOT -->
## 2026-11-23 — RAETH continuation snapshot

This snapshot was written against `main` **f7bd670c88520298c8c4628b1ff06e0c95251f12** (the handoff-only commit that follows this snapshot will become the new main HEAD). Frozen backup branch remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify or force-push it.

Recent implementation commits:
- `f7bd670c88520298c8c4628b1ff06e0c95251f12` — fix(raeth): tolerate unavailable pseudo interface on close
- `776c84b04a194d0260830eb14f8732df864ee7c3` — test(raeth): distinguish legacy reset task from kthread
- `2652b1bb7b775db37bfc89e8e44eab514ca1ca80` — fix(raeth): remove cancellation of uninitialized reset work
- `dd5e71103ee4ea1e2b312d358f253f662c0ddc0f` — fix(raeth): propagate MT7621 switch clock enable failures
- `7627383723a2c33bc6ff7456384ec248db3236fa` — fix(raeth): stop background workers before hardware reset
- `69c15a20e8ce91bf8c31e3ac600547421ba3bfd1` — fix(raeth): tear down MT7622 switch on open failures
- `f686f62b3200f17b19d8aeaceb3af4a85a08d967` — test(raeth): correct pseudo registration assertion
- `2563943c3c51f8bf72459b8d71b38d4a6f7254a5` — fix(raeth): remove NAPI registrations during netdev uninit
- `254514be6a9d25512ac40a7c206b4356558ccf2f` — fix(raeth): respect netdev uninit lifecycle and pseudo registration
- `e22aea0d3000117892d82992a9d88b96fe179599` — docs: fit handoff roadmap within validator limit

Exact-SHA Actions observed for baseline `f7bd670c88520298c8c4628b1ff06e0c95251f12`:
- XY-C1 Firmware Validation: in_progress (run 45323341920)
- Switch CLI ioctl failure-status regression: completed / success (run 45323341869)
- NVRAM transaction boundary audit: in_progress (run 45323341860)

The implementation slice addressed PDMA/QDMA RX partial allocation rollback and DMA unmapping, QDMA TX/FQ/SFQ staged allocation cleanup, RSS and hardware-LRO ring rollback/teardown, multi-stage DMA init unwind, IRQ request tracking/partial unwind (including disabling the GSW source before releasing its handler), open/close worker ordering and error cleanup, pseudo-netdev registration/uninit/NAPI lifecycle, MT7621 GP0 clock-enable error propagation, and removal of cancellation of the never-initialized `reset_task` work item. Regression source checks are in `scripts/check_raeth_rx_ring_cleanup.py`, wired into the Switch CLI stability workflow; relevant RAETH source paths are also used by firmware validation triggers.

Verification boundary: no local MIPS firmware build was run from this chat. Treat Actions as authoritative only for the exact SHA shown above; do not infer PASS for later documentation or source commits.

**First task for the next session:** fetch the new main HEAD and inspect its exact-SHA Actions, then continue a focused resource-lifecycle audit of `ra_switch.c::fe_sw_init()` / `fe_sw_deinit()` (platform-device references, regulator enable/disable pairing, clock state and ioremap unwind). After that, run/confirm the XY-C1 firmware build on the final source SHA. Do not repeat the completed DMA/IRQ/NAPI audits unless a new regression is found.


- 当前 main 接力基线：`776c84b04a194d0260830eb14f8732df864ee7c3`；归档备份 `backup/pre-plugin-overhaul-2026-11-14` 保持只读（`72f4f157a084f8184b9597ca5bf334b8080e5807`）。
- LAN DHCP `lan_dns_x` 严格解析修复 `17441ec` 的 WAN DHCP callback #45321875664 PASS、AI docs #45321875753 PASS；HTTPD、Firewall、process inventory、NVRAM、WAN route/signal regressions 在该 SHA 均 PASS。
- RAETH 最近连续修复包括 MT7621 GP0 clock enable error propagation、MT7622 open-failure switch teardown、先停止 kreset/kphy workers 再 reset、删除未初始化/从未 queue 的 `reset_task` work item。`7627383` Switch CLI #45322123792 PASS；后续 checker 更新需按新 SHA 单独核验。
- `3e13e24` 对 legacy `kill_pidfile_s()` 增加 PID 文件 owner=effective UID、single-link、禁止 group/world write 校验。`fdc7ac1` 将 regular-file predicate 改为 strict-GNU89 可用的 `S_IFMT/S_IFREG`。`fdc7ac1` legacy wrapper #45322826292 PASS、AI docs #45322826306 PASS、process inventory #45322826437 PASS；process lifecycle #45322826402 失败在 integration checker 的旧 `S_ISREG` 断言，不是 C compile。本提交更新断言并使 policy header 自包含。
- XY-C1 workflow 原先只监听 `shutils.c`，漏掉新 include 的 `shutils_pidfile_policy.h`，所以 header-only 安全策略不会触发固件编译。本提交补上 push path 和 source-contract assertion；必须检查同 SHA XY-C1 是否实际触发。
- `fdc7ac1` XY-C1 未触发；无本轮 policy-header 同 SHA MIPS build、TRX、artifact 或硬件 L4 结果可宣称。Roadmap 低于 131072 UTF-8 byte 上限，避免无关扩写。

---

# Session Handoff

## Current repository state

- Parent main before this checker/CI-trigger correction: `776c84b04a194d0260830eb14f8732df864ee7c3`; frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Shared PID metadata policy is in `3e13e24`; its strict-GNU89-compatible predicate is in `fdc7ac1`. Legacy wrapper PID-file #45322826292 PASS; AI docs #45322826306 PASS; process lifecycle inventory #45322826437 PASS.
- Process lifecycle regression #45322826402 failed only at `scripts/check_process_lifecycle_integration.py`, which still required the removed inline `S_ISREG(st.st_mode)` check. Its standalone C test compilation had advanced past the earlier GNU89 macro failure. The checker now requires the production metadata helper and its owner/link/mode rules.
- The XY-C1 push path included `shutils.c` but omitted included header `shutils_pidfile_policy.h`. Added the header path and checker assertion, so header-only security changes will trigger the target build.
- `shutils_pidfile_policy.h` now includes `<sys/stat.h>` itself; downstream tests do not depend on include order.
- Backup branch remains read-only; no XY-C1 build/TRX/artifact result or physical-router L4 result is claimed.
## Active task

Verify the process lifecycle integration contract and strict-GNU89 PID metadata regression on the new exact SHA. Confirm the XY-C1 workflow is triggered by the policy-header path and follow its Build → TRX verification → artifact upload. Continue an independent stability/security fix while any jobs remain pending.
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

- `3e13e24` legacy wrapper PID-file workflow #45322580536 PASS; `fdc7ac1` legacy wrapper #45322826292 PASS and AI docs #45322826306 PASS.
- `fdc7ac1` process lifecycle #45322826402 failed at the integration source checker because it still expected inline `S_ISREG`; this commit updates it to the actual metadata helper. Strict-GNU89 compile no longer failed at the metadata predicate after the `S_IFMT/S_IFREG` correction.
- XY-C1 had not triggered for `fdc7ac1` because its path filter omitted `shutils_pidfile_policy.h`; the current commit explicitly adds that path.
- No same-SHA XY-C1 build/TRX/artifact or physical hardware result is available at commit creation.
## Exact first action for the next GPT

1. Fetch live `main`; inspect exact-SHA process lifecycle regression, legacy-wrapper PID-file, process inventory, AI docs and XY-C1 workflows.
2. Confirm the metadata policy header compiles with `-std=gnu89 -Wall -Wextra -Werror`, the policy unit test passes with ASan/UBSan, and the integration checker accepts the production enforcement.
3. Confirm XY-C1 triggered because `shutils_pidfile_policy.h` is now in push paths; inspect Build → TRX verification → artifact upload on that exact SHA.
4. Continue a separate repository-wide stability/security fix while runs are pending. Keep backup read-only and preserve the roadmap byte limit.

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
