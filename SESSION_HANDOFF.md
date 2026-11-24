# Session Handoff

## Current repository state

- Live `main`: `51bd80eff75efd16fc49e46ac612b2dc39ea41ae`.
- Read-only backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged and never written.
- Latest logical security change: `51bd80eff75efd16fc49e46ac612b2dc39ea41ae`. It extends the JavaScript-context escaping fix to the reviewed templates; each change was published with a complete Git tree and non-forced expected-SHA update.
- The previous documentation checkpoint `c272520f59f51d63a1b174dc43e33b36a53703f4` is superseded by this state update.

## Completed development slices

### HTTPD / web UI security

- `09576b630122b31bed95c161223f05a43f0cdd6f`: NVRAM group compaction checks the transaction-locked count against the caller snapshot before mutating rows. Stale/malformed/negative/overflow counters fail closed.
- `beb57775ac957fb79a509f8fca9f22ee74510949`: escaped removable-media folder names at each AiDisk folder-tree text/title sink; Node VM XSS regression and workflow contract.
- `c89d1d777494dd13384270014c080b62c1b9f56a`, `6262b52ab098f98ceeb7466261a5916c1b4ed79e`, `3e02326eae477850f0f98fe74764b59f42c64baf`, `0136ceb43575bc4367fc288a3524ead11fcad7b7`: NVRAM group Add is transaction-locked, snapshots row/cache/count, clears deletion caches, publishes count last, rolls back failures, returns incomplete-rollback distinctly and propagates failures to HTTP 500. Fault injection covers missing/malformed/full counts, count/row read errors, partial write, applied-then-failed count update, and incomplete rollback.
- `96431bbdf8dafedf38db929ef31f9990ec9a2ab8`, `2b8333e65915adbfc7e617b81ded751b7e2a253b`, `c7bcd00de2d696a38e9dcae112b712c8dcfbcbf1`: group-delete index parsing consumes the complete raw input and rejects oversized/truncated values while preserving exact legacy `-1`. Regression includes a 255-digit zero-padded numeric prefix followed by an attacker suffix.
- `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`: added `nvram_get_js`, a complete escaped JavaScript string output callback that reuses the existing tested writer. Registered and classified it as `HTTPD_DIRECT_EJ_ESCAPED_OUTPUT`. Fixed stored XSS from `ddns_hostname_x` being placed in a single-quoted JS literal via HTML-only `nvram_get_x`.
- `6cb5c16818eb5a91382a97365692f58105e4b5c2` and `2a50ce0c6653de44fe071c13049747b406d49f3c`: corrected workflow source extraction and DDNS template assertion; the final HTTPD run for exact `2a50...` passed.
- `51bd80eff75efd16fc49e46ac612b2dc39ea41ae`: extended the safe JS callback to JavaScript-source NVRAM insertions in `Advanced_WAN_Content.asp`, `Advanced_LAN_Content.asp`, `Advanced_DHCP_Content.asp`, `Advanced_IPTV_Content.asp`, `Advanced_Services_Content.asp`, `Advanced_SettingBackup_Content.asp`, and `Advanced_MACFilter_Content.asp`. HTML form attributes/text remain on the HTML escaping helper. The HTTPD workflow now asserts the exact escaped-output markers and includes these templates in its path filters.

### IPTV / process lifecycle

Independent IPTV snapshot/lifecycle work remains on main: `99ad9397e3cf4450c6fc07758b58c3f7b51de115`, `d38250bfa213607dbf02ac65a75f60e933be34c9`, `b64511308e12091035a745055e2882a15addd166`, `57c466d3001da525511160fc30418a774b99aa85`. The too-short overflow fixture and stale lifecycle/storage checker expectations were corrected. Historical exact-SHA WAN route, process, storage and XY-C1 passes are in earlier log entries.

## Verification boundary

- Exact `2a50ce0c...` HTTPD Authentication/Security Regression `#45295707346`: **PASS**.
- Exact `2a50ce0c...` AI handoff contract `#45295707326`: **PASS**.
- NVRAM transaction-boundary audit `#45295392634` on source SHA `edc313cdf61b4c888ead94e019cc1f4c87f02ffa`: **PASS**, 15/15 steps; the group transaction implementation is unchanged by the subsequent UI template-only commit.
- XY-C1 Firmware Validation `#45295392702` on production source SHA `edc313...`: last observed **IN_PROGRESS** at Build (37/44). This build predates the seven additional ASP template updates in `51bd...`; current-main firmware verification has **not been confirmed**. Check for a new run on `51bd...`, or dispatch the supported XY-C1 workflow for that exact SHA. Do not claim L3 until Build, TRX validation and artifact upload all pass.
- The HTTPD workflow on exact current main `51bd...` has not yet been confirmed terminal at this checkpoint. Treat as **PENDING / UNVERIFIED**, not PASS or FAIL.
- No local MIPS cross-build, physical router test, or hardware L4 validation is claimed.

## Exact first action for the next GPT

1. Poll Actions for exact main `51bd80eff75efd16fc49e46ac612b2dc39ea41ae`. Confirm the HTTPD regression workflow and whether XY-C1 validation was triggered. Ensure the XY-C1 build includes the ASP files in `51bd...`; if no run exists, start the repo's supported validation workflow for this SHA.
2. For the build, check Build, TRX verification and artifact upload individually. Only then mark product L3 verified.
3. Continue the remaining `nvram_get_x()` context audit across the rest of the web UI. Seven additional pages were reviewed and their JS-source sinks switched; do not bulk-replace HTML contexts. Add a targeted marker/test for every newly confirmed JS sink.
4. Keep working on `main`, use one complete tree and atomic commit per logical task, and keep the backup read-only.

---

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
