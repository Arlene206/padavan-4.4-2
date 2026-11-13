## 2026-11-13 14:24 CST (UTC+8) — Close Adbyby configured hosts-source download hardening

- Recovered the live main and handoff after the configured /etc/storage/adbyby_host.sh task had already landed. Current validated source/test SHA is 1994f29e9e8180131011fb811bd9f6402e52523f; production implementation is 76f514cb5111c6787c4a6eda325619062845bace, and the tip commit corrects only the regression fixture's custom dnsmasq path.
- Inspected the failed first exact-source Adbyby run [43377852791] / job [131728447338]: shell syntax and prior tests passed, while the no-cache cleanup assertion used hard-coded /tmp/adbyby/hosts although the helper correctly received the temporary $PROG_PATH/hosts. The fixture correction in 1994f29e9e8180131011fb811bd9f6402e52523f fixes the assertion without changing production behavior.
- On exact main SHA 1994f29e9e8180131011fb811bd9f6402e52523f, all five triggered workflows completed SUCCESS: Adbyby [43378151993] / job [131729407848], AI handoff docs [43378151976] / job [131729407933], AliyunDrive [43378151941] / job [131729407811], AdGuardHome [43378151961] / job [131729407913], and AliDDNS [43378151950] / job [131729407894]. Every job fetched/checked out this exact SHA, every job step succeeded, and the full logs were inspected. Adbyby shell syntax, all seven regression scripts, configured host-source HTTPS/format/aggregate/cache/atomic-publication test, and producer-chain/source/workflow checker all PASS. AI docs reports 101/119 root records and archive inventory 7/22/18 at this source/test checkpoint.
- L2 configured hosts-source download is CLOSED. The implementation removes the insecure curl -k, URL word-splitting and shared /tmp/host.txt path; uses quoted bounded firmware-CA HTTPS for every configured source; validates hosts IPv4/IPv6 mappings, DNS labels, record size and aggregate size; stages privately on the target filesystem; atomically publishes the deduplicated complete file; and preserves validated last-good data or fails closed.
- Changed documents in this follow-up: SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. The next bounded task is the FRP full RC/UI/cron/shared-script audit. FTP failed-upload cleanup remains OPEN/BLOCKED on Linux 4.4 final-name TOCTOU; NFS requires real-client evidence; current-main L3 MIPS/uClibc firmware/TRX and L4 router remain NOT VERIFIED.
- The execution environment still has no usable full local repository checkout (GitHub DNS resolution failed); no local shell/Python test is claimed. Network requests were mocked in regression tests; no real external download, full firmware/TRX build or router test was run.
- Root dev-log count: 101 before this record and 102 afterward; still below 120-entry / 384 KiB early-rotation / 512 KiB hard limits. Archive/index inventory remains dev-log=7, handoff=22, roadmap=18; no rotation or index edit is due.
- Exact-head AI docs validation for this handoff/roadmap/log update is the next required action; require the resulting documentation SHA and 102/119 root records.
- Resulting main commit: THIS COMMIT.

---


## 2026-11-13 14:21:06 CST (UTC+8) — Correct Adbyby host-list test fixture path

- Follow-up to implementation commit `76f514cb5111c6787c4a6eda325619062845bace`. The exact-head Adbyby workflow run [43377852791] / job [131728447338] executed shell syntax and all prior regressions successfully, then the new test failed at the no-cache cleanup assertion.
- Root cause was a test-fixture path mismatch, not the production cleanup: `hosts_ads` correctly passed its temporary `$PROG_PATH/hosts` to the existing exact-line cleanup helper, while the fixture hard-coded `addn-hosts=/tmp/adbyby/hosts`. The helper therefore had no matching line to remove. The regression now writes and asserts the actual temporary path, preserving the same production code and exercising the real cleanup contract.
- Corrected the checker path tuple's missing comma before the initial commit was published; the committed checker already contains the separated path entries. The first exact-head AI handoff validator passed at run [43377852866] / job [131728447504], root log=100/119 and archive counts 7/22/18.
- This is a test correction only. No production behavior is changed in this follow-up. Other exact-head Actions from the first commit were still running when inspected; they must be checked again against their own exact SHA before closing the slice.
- Local checkout remains unavailable due to this execution environment's DNS failure resolving github.com. No local shell/Python result is claimed; no firmware build or router test was run.
- Root log now has 101/119 records; no rotation is due. The Adbyby host-list regression must be rerun on this commit SHA. If it passes, continue to check every exact-head workflow and then update the live handoff/roadmap to the next FRP RC/UI/cron/shared-script audit.
- Resulting main commit: THIS COMMIT.

---

## 2026-11-13 14:15:28 CST (UTC+8) — Harden configured Adbyby hosts-source downloads

- Base main SHA: `b6dcef4fcf304b0b0d84a47dda2e611f18f2765f`; active branch remains `main`.
- Resumed the live handoff task after verifying the documentation checkpoint AI handoff run [43370479491] / job [131704801095] SUCCESS on exact SHA `b6dcef4fcf304b0b0d84a47dda2e611f18f2765f`; validator reported root log 99/119 and indexed archive counts 7/22/18. The same exact-head push had infosvr [43370479485], AdGuardHome [43370479484], AliyunDrive [43370479477] and AliDDNS [43370479464] all SUCCESS; every job step and complete log was inspected.
- Traced the host-list configuration chain: ROMFS template `trunk/user/adbyby/adbyby_host.sh` is installed by the Adbyby Makefile and copied to `/etc/storage/adbyby_host.sh` by `copyscripts.sh` only when no operator file exists. The Adbyby UI exposes both `hosts_ad` and `scripts.adbyby_host.sh`; HTTPD marks the latter as a persisted File variable with `EVM_RESTART_ADBYBY`. The runtime is called from start, update and the lifecycle-locked manual `F` action.
- Replaced `hosts_ads()` insecure `curl -k`, shell-word-split URL iteration and shared `/tmp/host.txt` output with per-line quoted HTTPS requests through the existing firmware-CA / HTTP-200 / bounded retry-time-rate-size helper. Each source now has a private unique stage under `$PROG_PATH` (same filesystem as the final file), bounded response size, strict hosts-file syntax validation (IPv4/IPv6, domain labels, comments and line bounds), and one complete aggregate size limit. Exact duplicate records are removed before a single same-filesystem rename publishes mode 0644.
- A failed or malformed required source never truncates or deletes the live hosts file. A validated last-good cache remains active and its count/config entry is retained; absent a valid cache, the refresh fails closed and the managed dnsmasq entry is removed. URL values are not written to logs.
- Added a no-network regression covering query/ampersand URL non-splitting, multi-source merge/deduplication, IPv4/IPv6 data, TLS/redirect/HTTP failures, invalid URL/whitespace, missing CA, malformed and oversized bodies, aggregate overflow, last-good fallback, failed final rename, no-cache fail-closed behavior, disabled mode and private-stage cleanup. Extended the Python semantic checker to verify the ROMFS/boot/UI/HTTPD/runtime producer-consumer chain, plus symmetric workflow paths and commands.
- Changed paths: `trunk/user/adbyby/adbyby.sh`, new `trunk/user/adbyby/tests/host_list_download.sh`, `scripts/check_adbyby_hosts_config.py`, `.github/workflows/adbyby-config-stability.yml`, and this log.
- Local full checkout is unavailable (clone attempt failed because this execution environment cannot resolve github.com); no local shell or Python test result is claimed. Exact-main Actions for the resulting commit are PENDING; no real network request, MIPS/uClibc firmware/TRX build or router test was run.
- Root log was 99/119 records; this entry brings it to 100/119. File remains below the 384 KiB early-rotation target and 512 KiB hard limit; archive/index inventory remains dev-log=7, handoff=22, roadmap=18. No rotation is due.
- Resulting main commit: THIS COMMIT.
- Next: inspect the Adbyby and AI handoff Actions on this exact pushed main SHA. Require shell syntax, all pre-existing regressions, the new no-network host-list regression, source/producer chain checker and documentation validator to pass. Then update the live handoff/roadmap to resume the FRP RC/UI/cron/shared-script audit; keep FTP TOCTOU blocked and NFS/L3/L4 unverified.

---

## 2026-11-13 13:01 CST (UTC+8) — Close Adbyby anti-AD fragment lifecycle hardening

- Source-code base main SHA: `1c49f1169dd8a5ab153a04a4bdc86aaa5b8fb50d`; exact source/test/checker checkpoint: `ea854cd5e7ecdc124f786775fbaf72ace2931235`.
- The anti-AD implementation was audited after tracing anti_ad_link through NVRAM defaults/UI and inspecting the published dnsmasq export format. It reuses the shared firmware-CA HTTPS downloader, HTTP 200 check, private same-filesystem stage, 16 MiB cap, 512-byte line cap, full address=/domain/ validation, atomic rename and last-good content fallback.
- Found lifecycle gaps: del_dns() removed the last-good anti-AD file before refresh fallback; disabling the NVRAM toggle left the generated .conf active; and a rejected destination symlink stayed in conf-dir. Commit 6fbd0693b39b859cdae9ea4c918c7621815c1ae4 fixes those paths: disabled mode resets the count and removes the file/link itself, stop cleanup retains only a fully validated cache while enabled and removes other generated files/stale private stages, and link referents are never followed. Unrelated Adbyby DNS rules continue when optional anti-AD refresh fails.
- The validator now also enforces 63-byte DNS label bounds (in addition to existing 253-byte domain and 16 MiB file caps). The no-network fixture covers a 64-byte label, disabled mode, wrapper symlink unlink without referent mutation, non-regular destination rejection, last-good stop cleanup and cache deletion while disabled.
- The first exact-source Adbyby run on 6fbd (43368910069 / job 131699819182) passed shell syntax and all six host regressions but failed the semantic checker because a new assertion searched for a function's own name in its extracted body. The checker was corrected in ea854cd5e7ecdc124f786775fbaf72ace2931235. Final exact-code-head Adbyby [43369019121] / job [131700174947] passed shell syntax, all host regressions and the source/workflow checker. Exact-code-head AI docs [43369019133] / job [131700175086] also passed (98/119 entries at that checkpoint). HTTPD umbrella [43368910153] / job [131699819465] passed all 39 steps on source commit 6fbd. All transfer tests used local curl/NVRAM mocks; no real network download was performed.
- L2 host/Actions is CLOSED for anti-AD only. No full firmware/TRX build or router test was run; L3 MIPS/uClibc and L4 hardware remain NOT VERIFIED.
- Root-log order is restored newest-first: this entry, 12:38, 12:25, the preserved 12:10 b774 milestone, then 12:05. Adding this entry makes 99/119; no archive rotation is required (archive inventory remains dev-log=7, handoff=22, roadmap=18). Exact AI docs validation of this docs checkpoint is pending.
- Source checkpoint changed only adbyby.sh, anti_ad_download.sh and check_adbyby_hosts_config.py. Existing Adbyby workflow path filters already covered these paths symmetrically, so no workflow edit was required. This docs checkpoint updates the handoff, roadmap and root dev log for the completed task transition.
- Next: harden /etc/storage/adbyby_host.sh configured source-list / hosts-list downloads with the URL producer audit, bounded HTTPS, per-source staging, complete hosts validation, aggregate atomic publication and last-good retention.

---


## 2026-11-13 12:38 CST (UTC+8) — Place Adbyby umask contract at the wrapper boundary

- Base main SHA: `267bfb4e2d669c07d0daee918302e8780abc5958`. The exact-head run after the previous checker correction, Adbyby [43363045488] / job [131680816661], passed shell syntax and all five host regressions, then failed the semantic checker at the stale marker `umask 077` being required inside `adbyby_prepare_user_rules_transaction()`. Production code intentionally applies the private umask in its wrapper subshell before invoking the transaction helper.
- All other exact-267b push workflows passed: AI docs [43363045630] / job [131680816799], infosvr [43363045669] / job [131680817373], AdGuardHome [43363045535] / job [131680816777], AliDDNS [43363045485] / job [131680816288] and AliyunDrive [43363045498] / job [131680817636]. Exact checkout SHA confirmed; all steps passed. The AI docs validator passed.
- THIS COMMIT moves the source assertion to the actual wrapper function and keeps transaction assertions on the function that creates the private stage and publishes the complete list. No production source or host test changes.
- Third-party custom-rule L2 remains pending exact-main Actions; no local checkout/host execution is claimed.
- Root log count: 96→97/119, below rotation thresholds; archive inventory remains 7/22/18. No archive/index rotation.
- Changed paths: `scripts/check_adbyby_hosts_config.py`, `SESSION_HANDOFF.md` and this log. No firmware, branch, PR or GPT Session Rotator changes.
- Resulting main commit: THIS COMMIT.
- Next: inspect all exact-head Actions for the corrected checker; then continue anti-ad and hosts-list downloader hardening only after third-party rule L2 is green.

---

## 2026-11-13 12:25 CST (UTC+8) — Align Adbyby source checker with shared HTTPS validator

- Base main SHA: `315d738a580e154302dfdd9c57674ee64c3ada36`. Exact-head Actions for the third-party custom-rule change: Adbyby [43362382205] / job [131678687484] FAILED only at the semantic source checker; shell syntax and all five host regressions passed, including URL quoting, invalid URL rejection, TLS/redirect/HTTP fallback, oversized body rejection, last-good preservation and atomic publication. Failure was a stale assertion: it required a literal scheme check inside the managed fetch helper, although URL validation had been centralized in `adbyby_valid_https_url()`; the firmware CA readability checks remained in the shared downloader.
- The other six exact-315d push Actions all passed: infosvr [43362382242], AdGuardHome [43362382228], AI docs [43362382291], AliDDNS [43362382334], AliyunDrive [43362382226] and HTTPD [43362382225]. Every checkout log confirmed exact SHA `315d738a580e154302dfdd9c57674ee64c3ada36`; HTTPD's 39-step umbrella passed. AI docs reported 95/119 root entries and archives 7/22/18.
- THIS COMMIT aligns the semantic contract with the implementation: require the shared HTTPS URL validator call and a readable, non-empty firmware CA bundle. No production source or host-test behavior changes.
- Third-party custom-rule L2 remains pending until exact resulting-main Adbyby checker and all sibling Actions pass. No local checkout or local test execution is claimed.
- Root log count: 95→96/119, below rotation triggers; archive inventory remains 7/22/18. No archive/index mutation.
- Changed paths: checker, `SESSION_HANDOFF.md` and this log. No branch/PR, firmware artifact or GPT Session Rotator modification.
- Resulting main commit: THIS COMMIT.
- Next: inspect all exact-head Actions. If green, harden remaining anti-ad and hosts-list downloads, then continue FRP caller-path review.

---

## 2026-11-13 12:10 CST (UTC+8) — Close Adbyby third-party rule download hardening

- Live main SHA: a8681f65733f08b7c932833fafe99062492f0a2a. NVRAM-configured third-party Adbyby rule download slice is now L2 CLOSED on this exact SHA.
- All six triggered main push Actions completed SUCCESS and all exact checkout SHAs, steps and full logs were inspected: Adbyby [43363263383] / job [131681514167], infosvr [43363263454] / job [131681514465], AI docs [43363263378] / job [131681514145], AliDDNS [43363263594] / job [131681514640], AliyunDrive [43363263430] / job [131681513797], AdGuardHome [43363263365] / job [131681513585]. Adbyby sh -n, hosts preservation, lifecycle lock, stock HTTPS/MD5/cache rollback, third-party HTTPS/URL/size/last-good publication, firewall snapshot and semantic source/workflow checker all PASS. AI docs reports root log 97/119 and archives 7/22/18.
- Production commit 315d738a580e154302dfdd9c57674ee64c3ada36 closes the insecure NVRAM third-party rules path: TLS verification / firmware CA / HTTPS-only, quoted and validated URLs, transfer timeout/retries/rate/size limits, unique mode-0700 staging, accepted line filtering, last-good cache preservation and atomic complete-list publication. Checker-only corrections 267bfb4e2d669c07d0daee918302e8780abc5958 and a8681f65733f08b7c932833fafe99062492f0a2a fixed stale assertions (centralized URL check and wrapper-level umask); final exact-head Actions green.
- Firmware curl package is 8.21.0 (trunk/libs/libcurl/Makefile), confirming shared downloader flags --proto, --max-filesize and --limit-rate exist in project build.
- Next Adbyby scope: anti-ad dnsmasq fragment via anti_ad_link, then host-source downloads. Inspect actual configuration producer/default and supported dnsmasq fragment syntax before enforcing a format allowlist. Both still contain curl -k and fixed destinations. After these, return to FRP whole caller-path audit. FTP cleanup remains OPEN/BLOCKED on the Linux 4.4 final-name TOCTOU; NFS needs client evidence; L3/L4 remain NOT VERIFIED.
- Corrective note: the first docs checkpoint commit accidentally replaced the rolling root log rather than prepending to it. The exact preceding 97-entry log from a8681f65733f08b7c932833fafe99062492f0a2a is restored below this record, returning the root log to 98/119 entries. The AI docs Action on intermediate b774286064475c5d09c57fe20f342623740df1e8 correctly failed with “found 1”; the restored file still needs exact-head validator confirmation.
- Root log count after restoration: 97→98/119; size remains below 384 KiB; archives remain dev-log=7, handoff=22, roadmap=18. No rotation or archive/index changes.
- Changed paths: SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md. This checkpoint closes the previous active task and names the next concrete one; no firmware, branch/PR or GPT Session Rotator changes.
- Resulting main commit: THIS COMMIT.
- Next exact action: inspect all anti_ad_link producers/defaults and sample config grammar, then implement and test safe same-filesystem download/publication with last-good retention.

---

## 2026-11-13 12:05 CST (UTC+8) — Harden Adbyby third-party rule downloads

- Base main SHA: `42d4af7e6c0ceeb8508f1446b4228068318208dc`; exact-head stock managed-rule downloader is L2 CLOSED on this SHA. All six triggered main push Actions succeeded with every job step/full log and exact checkout SHA inspected: Adbyby [43359578905] / job [131669629305], infosvr [43359578915] / job [131669629661], AdGuardHome [43359578954] / job [131669629841], AI docs [43359579010] / job [131669629883], AliDDNS [43359578908] / job [131669629322] and AliyunDrive [43359578896] / job [131669629383].
- Cross-module Adbyby audit found NVRAM-configured third-party rules still used `curl -k`, unquoted URL expansion and predictable `/tmp/adbyby/user2.txt`. `add_rules()` also deleted prior `user.txt` and compiled `user.bin` before remote response validation, so transient failures could leave incomplete rules.
- THIS COMMIT adds strict bounded HTTPS URL validation (no whitespace/userinfo), firmware CA verification, HTTPS-only protocol, exact HTTP 200, bounded connect/total time, retries, transfer rate and max file size plus post-download byte validation. Each URL is passed as one quoted argument and staged in a unique mode-0700 directory. Existing accepted filter-rule syntax is retained. The complete combined user list is published by same-filesystem rename only after successful preparation; failures keep a prior non-empty regular `user.txt`, or fail closed if no last-good list exists. `user.bin` is invalidated only after successful publication.
- Added `trunk/user/adbyby/tests/external_downloads.sh`; expanded the managed download fixture for shared protocol/rate/size policy; updated source checker and symmetric push/pull workflow path/commands. Exact resulting-main Actions are PENDING. No local checkout or local host/syntax/Python test is claimed; no target BusyBox/MIPS, full firmware/TRX or hardware result is claimed.
- Anti-ad (`anti_ad_link`) and host-list downloads still contain `curl -k` and fixed destinations; they remain the next separate implementation slice.
- Root log count: 94→95/119, below 120-entry rotation and 384 KiB early-rotation thresholds; archive inventory remains 7/22/18. No archive/index mutation.
- Changed paths: `trunk/user/adbyby/adbyby.sh`, `trunk/user/adbyby/tests/rules_download.sh`, new `trunk/user/adbyby/tests/external_downloads.sh`, `scripts/check_adbyby_hosts_config.py`, `.github/workflows/adbyby-config-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and `dev_log.md`. No branch/PR, firmware artifact or GPT Session Rotator change.
- Resulting main commit: THIS COMMIT.
- Next: inspect every exact-head Action triggered by this commit; repair failures until Adbyby and all sibling workflows are green. Then apply the hardened publication model to anti-ad and host-list downloads before returning to FRP caller-path review.

---

## 2026-11-13 11:25 CST (UTC+8) — Fix Adbyby subshell-function contract extractor

- Base main SHA: `e39778793032c5bf87208304a298f6ac8e77bd7a`; live main matched immediately before the correction.
- Inspected all seven exact-head push Actions and every job step/log on that SHA. AliyunDrive [43358743725] / job [131666935595], HTTPD security umbrella [43358743676] / job [131666935386], AdGuardHome [43358743708] / job [131666935118], AI docs [43358743714] / job [131666935170], infosvr [43358743847] / job [131666935354] and AliDDNS [43358743752] / job [131666935129] succeeded. HTTPD passed all 39 steps; the docs validator reported root log 93/119 and archives 7/22/18.
- Adbyby [43358743855] / job [131666935611] passed shell syntax and all four host regressions, including the new private HTTPS staging, MD5, cache fallback and publication rollback suite. Its only failure was the semantic checker: the extractor regex omitted the closing `)` in `adbyby_refresh_managed_rules()` before the POSIX subshell body.
- Corrected the semantic checker to match the subshell function signature `name()\n(`. This is a checker/parser correction; no production source or test behavior changed in this follow-up.
- Exact resulting-main Actions after the checker correction are PENDING; the managed-rule downloader remains L2 pending until the exact pushed SHA has an all-green Adbyby run and the other triggered workflows are inspected.
- Root log count: 93→94/119; root file remains below the 384 KiB early-rotation target. Archive inventory stays 7/22/18; no archive or index rotation.
- Changed paths: `scripts/check_adbyby_hosts_config.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and `dev_log.md`. No branch/PR, firmware binary/bundle or GPT Session Rotator change.
- Resulting main commit: THIS COMMIT.
- Next: recheck exact-main Actions for the parser correction. Then harden the remaining third-party-rule, anti-ad and hosts download paths with HTTPS, URL/size/time validation, unique private staging and last-good publication; afterward return to the FRP full caller-path audit.

---

## 2026-11-13 10:55 CST (UTC+8) — Harden Adbyby managed rule download and publication

- Base main SHA: `ceeb24d6b17e03f1777c240bdf0aca9fa6481194`; live main matched before constructing this change. Only the main branch exists; no checkpoint branch or development PR was created.
- Recovered the previous task first. On the exact base SHA, Adbyby [43346579501] / job [131627282482] passed shell syntax, managed dnsmasq hosts preservation, lifecycle lock/concurrency/update routing, firewall snapshot regression and the Python source/workflow contract. AdGuardHome [43346579547], AliDDNS [43346579542], AliyunDrive [43346579532], infosvr [43346579502] and AI docs [43346579436] also passed on that same exact SHA; all job steps/logs and checkout SHAs were inspected. This closes the preceding Adbyby lifecycle serialization task at L2.
- The stock `lazy.txt` / `video.txt` updater used predictable shared files under `/tmp`, disabled certificate validation with `curl -k`, did not require HTTP 200, and could leave the first rule replaced if the second rename failed. THIS COMMIT replaces that flow with a unique mode-0700 staging directory under `/tmp` and `umask 077`; downloads require HTTPS, the firmware CA bundle, curl failure handling, HTTP 200, a five-second connect timeout, a 30-second total timeout and bounded retries. The manifest must contain 32-hex MD5 fields, and both downloaded payloads must match before publication.
- Remote download/manifest/digest failures retain an existing pair of non-empty regular cache files; with no usable pair, rule preparation returns failure. Start and update/restart now stop before launching the daemon if rule preparation cannot establish a safe state. The two published files use same-filesystem atomic renames; if the second rename fails, the first is rolled back. If rollback itself fails, the private staging directory is retained for recovery. This is not a single crash-atomic two-file transaction.
- Added `trunk/user/adbyby/tests/rules_download.sh` for valid update, no-op, TLS/CA failure, redirect, HTTP failure, corrupt/partial payload, missing CA/cache and second-rename rollback. Wired shell syntax, host regression, source checker and both push/pull_request path filters.
- Exact resulting-main Adbyby/AI-doc Actions are PENDING. No host test, semantic checker, target BusyBox/MIPS, full firmware or hardware result is claimed until the exact pushed SHA is inspected. No local full checkout was available in the recovery environment.
- Scope is the two stock managed rule files only. Legacy third-party rules, `anti_ad_link` and host-list downloads still need a separate follow-up for `curl -k`, URL validation, shared temp paths, bounded size/time and atomic publication.
- Root log count: 92→93, below the 119-entry / 120-trigger rotation threshold; existing archives remain 7/22/18. No archive or index mutation.
- Changed paths: `trunk/user/adbyby/adbyby.sh`, new `trunk/user/adbyby/tests/rules_download.sh`, `scripts/check_adbyby_hosts_config.py`, `.github/workflows/adbyby-config-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and `dev_log.md`. No firmware bundle/binary, branch, PR or GPT Session Rotator modification.
- Resulting main commit: THIS COMMIT.
- Next: fetch main and inspect the exact-head Adbyby and AI docs workflow logs; repair any failures. Then harden remaining custom-rule, anti-ad and hosts download paths before returning to full FRP caller-path review.

---

## 2026-11-13 08:46 CST (UTC+8) — Parse C restart contract correctly in Adbyby checker

- Base main SHA: `cb7d3264b6398b0e7025c5ecb041169e0c805884`; live main matched before the change.
- After the firewall fixture correction, the exact-head Adbyby workflow [43346210843] / job [131626087357] passed shell syntax and all host regressions: managed dnsmasq preservation, lifecycle locking/concurrency/update routing and firewall snapshot failure-cleanup. Its Python semantic checker then failed only because `function_body()` recognizes shell functions, while `restart_adbyby` is C syntax `void restart_adbyby(void) { ... }`.
- Replaced that C lookup with a dedicated anchored C-signature regex and explicit missing-function failure. This retains the assertion that RC restart is one `/usr/bin/adbyby.sh restart` invocation.
- HTTPD authentication umbrella [43346210847] / job [131626087850] passed all 39 steps on the same exact SHA, including the shared firewall regression and add-on firewall hook checks. infosvr [43346210958], AliyunDrive [43346210796], AliDDNS [43346210808], AdGuardHome [43346210772] and AI docs [43346210862] also passed. Every job's steps/logs must be checked again on the resulting exact SHA.
- Current follow-up Actions are PENDING; do not claim lifecycle L2 until exact-main Adbyby checker and all triggered workflows pass.
- Root log count: 91→92, below rotation thresholds; archive inventory remains 7/22/18.
- Changed paths: Adbyby semantic checker, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. No source implementation change, firmware, branch/PR or GPT Session Rotator change.
- Resulting main commit: THIS COMMIT.
- Next: inspect the exact-head Adbyby semantic checker and all triggered workflows; once green, continue private download staging/TLS/integrity.

---
## 2026-11-13 08:42 CST (UTC+8) — Align Adbyby firewall fixture with locked NVRAM initialization

- Base main SHA: `c54c6c71b308ef4b188bfd32ea8431e6ae8fa5a0`; initial post-push head matched exactly.
- New lifecycle lock regression passed on that exact SHA: it verified the `B` action remains narrow, `updateadb` reaches the rule updater, RC restart stop/start ordering, disabled and failed-stop behavior, concurrent serialization, stable inode, symlink/hardlink/permissive-file rejection, failure-status propagation and FD-9 closure.
- Adbyby [43344906688] / job [131621789193] failed in the existing `firewall_snapshot.sh` direct-helper fixture after the new launcher intentionally stopped taking NVRAM snapshots at source time. The test directly calls `ip_rule()` / `add_rule()` after dot-sourcing, without the locked dispatcher, leaving `adbyby_ip_x` and `wan_mode` empty; the resulting numeric test warning caused the final firewall scenario to fail. HTTPD authentication umbrella [43344906734] / job [131621789538] encountered the same shared fixture failure in its add-on firewall staging step. The remaining 17 exact-head workflows passed.
- Corrected the fixture to call `adbyby_refresh_nvram_state` immediately after dot-sourcing, matching the production invariant, and added a semantic checker assertion so future fixture edits cannot silently drop the initialization.
- Current follow-up SHA Actions are PENDING; rerun exact-head Adbyby and HTTPD umbrella jobs and inspect all triggered steps/logs before claiming L2.
- Root log count: 90→91, below the 120-record/384 KiB rotation thresholds; archive inventory remains 7/22/18. No archive/index rotation needed.
- Changed paths: firewall snapshot host fixture, Adbyby semantic checker, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. No firmware, branch/PR or GPT Session Rotator changes.
- Resulting main commit: THIS COMMIT.
- Next: verify exact-head Actions; if all are green, close lifecycle serialization at L2 and proceed to private download staging/TLS/integrity audit.

---
## 2026-11-13 08:34 CST (UTC+8) — Serialize Adbyby lifecycle and update actions

- Recovery base main SHA: `6fa50e11532ca5c68a9fdb3ffc95a8415841e794`. On that exact SHA, Adbyby [43336402301] / job [131593559948], infosvr [43336402343] / job [131593560012], AliyunDrive [43336402492] / job [131593560634], AliDDNS [43336402424] / job [131593560363], AdGuardHome [43336402256] / job [131593559789], AI docs [43336402234] / job [131593559451] and HTTPD security [43336402226] / job [131593559966] all SUCCESS; every step/log inspected. AI docs reports root log=89/119 and archive inventory 7/22/18.
- Full caller tracing covered HTTPD `EVM_RESTART_ADBYBY` through PID 1 `rc.c` and `services.c`; RC restart previously ran separate stop/start script processes. HTTPD's update notification sent `updateadb`, but the shell dispatcher had no matching case.
- Public Adbyby actions now run through persistent `/tmp/.adbyby.lifecycle.lock`, validating directory/file type, owner, mode, symlink and hard-link state before bounded `flock -n 9`. The inode is never unlinked. Mutable NVRAM is refreshed after lock acquisition.
- RC restart now dispatches one `restart` transaction; `updateadb` reaches `adbyby_uprules`. The daemon and DHCP restart helper close FD 9 so detached children cannot retain the lock. A no-network regression covers transaction ordering, concurrent action serialization, update routing, disabled restart, stop failure, lock-file attacks, inode stability and FD cleanup.
- Exact resulting-main Adbyby / AI docs / HTTPD Actions are PENDING. Do not claim L2 until every triggered exact-head job and relevant log passes.
- Fixed shared download paths remain. Next audit should use private unique staging and verify HTTPS certificate checking, checksum-manifest handling, partial downloads, failure propagation and atomic publication.
- FTP upload cleanup remains OPEN/BLOCKED (fstatat/unlinkat final-name TOCTOU; Linux 4.4 lacks inode-conditional unlink-by-fd). NFS remains gated on real-client evidence.
- No local full checkout or local whole-project tests claimed (github.com DNS resolution unavailable); no current-main MIPS/uClibc build, firmware/TRX or router test claimed.
- Root log count: 89→90, below 120-record rotation and 384 KiB early-rotation thresholds; archive inventory remains 7/22/18 and no archive/index change is needed.
- Changed paths: Adbyby launcher, lifecycle-lock host regression, RC services caller, semantic checker, Adbyby workflow, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log. No branch/PR, firmware binary or GPT Session Rotator changed.
- Resulting main commit: THIS COMMIT.
- Next: verify exact resulting-main Actions; if green, close lifecycle serialization at L2 and continue private download staging/integrity review.

---
## 2026-11-13 08:18 CST (UTC+8) — Preserve operator dnsmasq directives during Adbyby hosts cleanup

- Recovery base main SHA: edd2b4e231819fd748dea032324cd45904ddf172. FRP duplicate watchdog restart prevention is L2 CLOSED on this exact SHA: FRP [43334213696] / job [131586165339], AI docs [43334213673] / job [131586165454], infosvr [43334213527] / job [131586164710], AdGuardHome [43334213750] / job [131586165190], AliDDNS [43334213640] / job [131586164869] and AliyunDrive [43334213535] / job [131586164554] all SUCCESS; every step/log inspected. AI docs reports root log=88/119 and archive inventory 7/22/18.
- Cross-module audit identified an Adbyby stability bug: hosts_ads() and del_dns() used broad sed -i '/hosts/d' against shared /etc/storage/dnsmasq/dnsmasq.conf, deleting unrelated addn-hosts, hostsdir, conf-file, comments, address directives and other settings containing hosts.
- This patch adds adbyby_remove_generated_hosts_config(), which escapes BRE-special path characters and deletes only the anchored exact generated addn-hosts=$PROG_PATH/hosts entry. Both callers use it; hosts_ads() refuses to append when cleanup fails, and del_dns() logs cleanup failure without sweeping unrelated configuration.
- A no-network regression extracts the production helper verbatim and checks operator-owned directives, duplicate managed-line removal, metacharacter paths, idempotence and missing-config no-op. A dedicated Actions workflow also executes the existing firewall snapshot regression and checks caller order plus symmetric push/pull path filters.
- Exact resulting-main Adbyby Actions are PENDING. No L2 closure until shell syntax, extracted-helper preservation test, firewall snapshot regression and semantic source/workflow checker pass on that SHA.
- FTP failed-upload cleanup remains OPEN/BLOCKED: fstatat/unlinkat leaves a final-name race; Linux 4.4 lacks inode-conditional unlink-by-fd. Do not weaken STOR/APPE/REST/STOU or concurrent external-writer semantics.
- No local full clone/project tests (github.com DNS resolution unavailable); no MIPS/uClibc object build, firmware/TRX or physical-router test is claimed.
- Root log count: 88→89, below 120-record rotation and 384 KiB early-rotation thresholds; no archive/index rotation (inventory remains 7/22/18).
- Changed paths: Adbyby production script, extracted-helper regression, semantic checker, new path-filtered workflow, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md. No branch/PR, firmware binary or GPT Session Rotator changed.
- Resulting main commit: THIS COMMIT.
- Next: inspect every Adbyby workflow step/log on the exact resulting SHA; if green, continue shared temporary-file/download integrity and lifecycle audit, then return to FRP broader caller paths.

---

## 2026-11-13 08:00 CST (UTC+8) — Repair FRP host fixture setup and restore mandatory Active task handoff heading

- Base main SHA: `cb592664e3d95cd0c1a42f51bb3805fa8f1db307`.
- Exact-head FRP workflow [43333265560] / job [131582968032]: launcher `sh -n` PASS; host regression failed before exercising assertions because the sourced launcher evaluated its final `case $1` while the test shell had `set -u` and no positional parameter. The semantic checker was skipped. This is a host-fixture setup defect, not evidence of a production-shell failure. The fixture now sets a harmless source-only positional argument before dot-sourcing.
- Exact-head AI docs [43333265414] / job [131582967557] failed because this handoff consolidation omitted the mandatory exact heading `## Active task`. Infosvr [43333265323] / job [131582967713] passed both GNU89 ASan/UBSan packet/signal policy binaries but its semantic source checker rejected the same missing heading. Restore the required heading and record it as a permanent handoff contract.
- Other same-SHA workflows succeeded: AliDDNS [43333265442], AliyunDrive [43333265330], AdGuardHome [43333265313].
- This follow-up repairs the test invocation and handoff structure. Exact-head reruns are pending; no full host, firmware, target or hardware result is claimed.
- Root log count: 87→88 records, below the 120-record rotation threshold and 384 KiB early-rotation target; archive inventory remains 7/22/18; no archive/index rotation.
- Changed paths: FRP host test, `SESSION_HANDOFF.md` and this log. The FRP launcher/workflow implementation in `cb592664e3d95cd0c1a42f51bb3805fa8f1db307` is unchanged.
- Resulting main commit: THIS FOLLOW-UP.
- Next: verify exact-head FRP, AI docs and infosvr actions; then continue the Adbyby whole-module audit. FTP upload cleanup TOCTOU remains OPEN/BLOCKED.

---

## 2026-11-13 07:48 CST (UTC+8) — Aggregate FRP watchdog restarts across enabled daemons

- Base main SHA: f08e11546b267b717253c0619e7eea86372146fe.
- Whole-tree audit found that `check_frp()` called `frp_start` separately for each enabled daemon whose PID was not yet visible. `frp_start` runs the shared `frp_script.sh`, which kills both daemons and backgrounds each enabled replacement. If both are still in asynchronous startup, one watchdog pass can invoke it twice and kill/relaunch the first startup.
- The watchdog now checks both enabled-service states first and issues at most one shared restart per pass. The WAN failure logger was also unreachable after `return 2`; it now runs before the return, and the ping success condition uses POSIX `if` syntax.
- Added a POSIX-shell host regression that intentionally keeps both PIDs absent after the simulated background start, covering both missing, either missing, already-running, disabled and WAN-down cases. Added symmetric push/pull path triggers and a Python source-contract checker.
- FTP follow-up review confirmed the documented OPEN/BLOCKED boundary: `ftp_acl_path_unlink_opened_entry()` performs `fstatat()` identity validation followed by `unlinkat()`, leaving a final-name replacement race. Linux 4.4 lacks inode-conditional unlink-by-fd; do not weaken external-writer semantics or claim this closed without a safe design.
- Local full checkout/host test execution unavailable; the new exact-head FRP workflow is pending. No host/target/build result is claimed.
- Root log count: 86→87 records, below the 120-record rotation threshold and 384 KiB early-rotation target; archive inventory remains 7/22/18, no archive rotation.
- Changed paths: `trunk/user/frp/frp.sh`, new FRP host test/workflow/source checker, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log. No binary bundle, branch, PR or GPT Session Rotator changed.
- Resulting main commit: THIS COMMIT.
- Next: verify the FRP lifecycle workflow on the exact resulting main SHA, then continue the Adbyby lifecycle/configuration audit; FTP cleanup TOCTOU remains OPEN/BLOCKED pending a race-safe design.
 
---

## 2026-11-13 07:22 CST (UTC+8) — Synchronize credential/log-filter regression failure marker

- Base main SHA: 6e384bf903ad6b856ed0054fe0f07fc41869798e.
- Exact-head AliyunDrive [43327886559] / job [131564888160]: launcher syntax PASS, QEMU exact MIPSel v1.2.1 credential-env capability PASS, and every host test passed: 0700/0600 token storage, owner/symlink/hardlink checks, umask 077, RUST_LOG filter, exact env/no-argv privacy, bounded stop/start, duplicate-start and lock/readiness.
- The last semantic-checker stage failed because its negative-marker inventory still expected "FAIL: credentials were not passed as three exact environment values"; the strengthened host fixture emits "FAIL: credentials and log filter were not passed as exact environment values". This was a stale marker, not a regression. The checker is updated to require the actual current error wording.
- Other exact-head Actions all SUCCESS: AI docs [43327886525] / job [131564888272] (root log=85/119, archives 7/22/18); AliDDNS [43327886574] / job [131564887809]; AdGuardHome [43327886676] / job [131564888329]; infosvr [43327886567] / job [131564888347].
- Exact-head Actions for THIS checker correction are PENDING. No token-at-rest L2 closure until the full checker and all other path workflows pass on the resulting SHA.
- Root log count: 85→86 records, below the 120-record rotation threshold and 384 KiB early-rotation target; archive inventory remains 7/22/18; no archive/index rotation.
- Changed paths: AliyunDrive source/RC checker, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, and this log. No production launcher, host test, bundle, branch, PR or GPT Session Rotator changed.
- Resulting main commit: THIS COMMIT.
- Next: rerun exact-head QEMU, host security/lifecycle and source/RC checks; verify all path-triggered actions, then continue FTP cleanup TOCTOU review.

---

## 2026-11-13 07:12 CST (UTC+8) — Match FD-9 source contract to the background child subshell

- Base main SHA: 2ebbd0eba0bfbdb09ae1361f2194bac600c29b34.
- Exact-head AliyunDrive run [43327186519] / job [131562505132]: shell syntax and QEMU packaged v1.2.1 credential-env capability PASS; all host tests passed, including private token workdir, 0700/0600 permissions, owner/symlink/hardlink checks, child umask 077, RUST_LOG, exact credential env/no-argv, bounded stop/start, duplicate-start, lock and startup-presence checks.
- The last source/RC contract failed because the checker still required the prior direct-background string '>/dev/null 2>&1 9>&- &' and did not account for the new background subshell. Production code closes FD 9 with exec 9>&- inside the subshell before starting the daemon, so the daemon cannot keep the lifecycle lock held; the actual group redirection is different. This follow-up updates the source contract to require that early close and the actual child group.
- Other exact-head Actions all SUCCESS: AI docs [43327186729] / job [131562505235] (root log=84/119, archives 7/22/18), AliDDNS [43327186581] / job [131562504565], AdGuardHome [43327186702] / job [131562504857], infosvr [43327186506] / job [131562504329].
- Exact-head Actions for THIS COMMIT are PENDING. No token-hardening L2 closure until source/RC checker passes together with the already-passing host/QEMU steps.
- Root log count: 84→85 entries, below the 120-record rotation threshold and 384 KiB early-rotation target; archive inventory remains 7/22/18; no archive/index rotation.
- Changed paths: AliyunDrive semantic source/RC checker, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log. No production launcher, host fixture, binary, branch, PR or GPT Session Rotator changed.
- Resulting main commit: THIS COMMIT.
- Next: verify exact-head AliyunDrive source/RC, QEMU, host security/lifecycle, and all sibling path-triggered Actions; then resume FTP cleanup TOCTOU.

---

## 2026-11-13 07:03 CST (UTC+8) — Update source checker for configurable AliyunDrive workdir

- Base main SHA: a67610618da02275f4bad8433e5f6755a7f3d846.
- Exact-head AliyunDrive run [43326575598] / job [131560419812]: shell syntax and QEMU SHA-bound v1.2.1 credential-env capability PASS. All host regressions passed, including private workdir/token permissions, owner/mode checks, symlink/hardlink rejection, inherited umask 077, RUST_LOG filter, exact env/no-argv credential privacy, bounded stop/start, duplicate-start and lock/readiness.
- The final source/RC checker failed only because its existing argv contract still required --workdir "/tmp/$NAME" instead of the new configurable --workdir "$ALD_WORK_DIR". This stale static assertion—not launcher behavior or the host fixture—was the only AliyunDrive failure. The source checker is updated to require the actual configured workdir parameter.
- Exact same-SHA unrelated workflows all SUCCESS: AI docs [43326575671] / job [131560419539] (root log=83/119; archives dev-log=7, handoff=22, roadmap=18), AliDDNS [43326575555] / job [131560419469], AdGuardHome [43326575649] / job [131560419668], infosvr [43326575524] / job [131560419403].
- Exact-head Actions for THIS checker correction are PENDING; do not claim token-hardening L2 closure until the full source checker succeeds alongside the already-passing host regression and QEMU probe.
- Root log count: 83→84 records, below the 120-record rotation threshold and 384 KiB early-rotation target; archive inventory remains 7/22/18. No archive/index rotation.
- Changed paths: AliyunDrive source/RC checker, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log. No production launcher, host test, binary, GPT Session Rotator, branch or PR changed.
- Resulting main commit: THIS COMMIT.
- Next: inspect all exact-head workflows; verify the source checker, QEMU, host privacy/workdir/umask/lifecycle cases pass together, then proceed to FTP cleanup TOCTOU review.

---

## 2026-11-13 06:56 CST (UTC+8) — Align workdir argv fixture with isolated test path

- Base main SHA: 2cce075b10faf5b044b5a3b75bb06ae7335dd0af, direct child of the previously green credential-redaction checkpoint dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb.
- Exact-head AliyunDrive run [43325869188] / job [131558034165]: launcher shell syntax PASS; QEMU ran packaged MIPSel daemon 1.2.1 and SHA-bound credential-env capability report PASS. The host fixture reached the argv equality check but compared the test override /tmp/.../aliyundrive-webdav-workdir against a hard-coded /tmp/aliyundrive-webdav expectation. The launcher argument is correct; this is a stale fixture path. The source/RC checker and later umask/env tests were skipped, so no full L2 pass is claimed.
- On the same exact SHA, AI docs [43325869229] / job [131558034089] passed (root log=82/119; archive inventory 7/22/18); AliDDNS [43325869222] / job [131558033911], AdGuardHome [43325869134] / job [131558033780], and infosvr [43325869414] / job [131558034824] all passed.
- This follow-up removes the quoted expected-argv heredoc delimiter and interpolates ALD_WORK_DIR, so the test asserts the actual isolated path rather than a production-only hard-coded path. No production launcher behavior changes in this correction.
- Exact-head AliyunDrive and AI-docs reruns for THIS COMMIT are PENDING; no secure-workdir L2 closure is claimed until the host fixture proceeds through all remaining assertions and the semantic checker passes.
- Root log count: 82→83 complete entries, below the 120-record rotation threshold and 384 KiB early-rotation target; archive inventory remains dev-log=7, handoff=22, roadmap=18. No archive/index rotation.
- Changed paths: lifecycle test fixture, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log. No production source, bundled binary, GPT Session Rotator, branch or PR changed.
- Resulting main commit: THIS COMMIT.
- Next: inspect exact-head AliyunDrive run logs and verify permissions, symlink/hardlink, child umask/RUST_LOG, lifecycle and source/RC checks all pass together.

---

## 2026-11-13 06:50 CST (UTC+8) — Harden AliyunDrive persisted refresh-token permissions and suppress token INFO logging

- Base main SHA: dbf15f4bd8472da954a9f5ce68e4b4b57f09f2bb; remote branch inventory contains only main. Credential argv redaction and its full L2 source/host validation are closed on this exact base.
- Exact-base path workflows all completed SUCCESS: AliyunDrive [43321989293] / job [131544724607]; AI handoff docs [43321989151] / job [131544725051]; AliDDNS [43321989235] / job [131544724203]; AdGuardHome [43321989308] / job [131544724394]; infosvr [43321989090] / job [131544723517]. AliyunDrive QEMU verified bundled daemon 1.2.1 and all three credential env names against binary SHA-256 237c0ab2c14197c5a4e4bd8975f884979a29b35cad31b8e2354e084c19413891.
- Reviewed exact upstream messense/aliyundrive-webdav v1.2.1 source: src/drive/mod.rs (blob b93f1194a9bc180d70fbd7f93104bd0f6eae60c8) persists rotated tokens with create_dir_all + tokio::fs::write, leaving new-file mode to umask and preserving an existing file's mode; src/main.rs (blob a234d0ee21420294a8f6624d184d25ec0ae696b2) includes the rotated refresh token in an INFO event.
- Added a fail-closed launcher workdir guard: directory must not be a symlink, must be owned by the invoking UID, and is forced to 0700; an existing token must be a same-owner regular non-symlink file with a single hardlink and is forced to 0600. The daemon child inherits umask 077, and RUST_LOG=aliyundrive_webdav=warn suppresses the upstream token-bearing INFO event while retaining warning/error logs. The child closes inherited FD 9 before exec so the background daemon cannot retain the lifecycle lock.
- Expanded host tests for fresh and existing permissions, hardlink and symlink rejection, target non-modification, exact credential/log-filter environment, and inherited umask/file mode. The semantic checker requires the launcher security contract and BusyBox STAT, stat-format, ID, CHMOD, and MKDIR applets.
- Exact-head Actions for THIS COMMIT are PENDING. No local full checkout or local shell/Python execution is claimed; no L2 result is claimed before the full path-triggered workflow finishes on the resulting SHA.
- Root log count: 81→82 complete records, below the 120-record rotation threshold and 384 KiB early-rotation target; the 512 KiB hard cap is preserved. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no archive or index rotation is due.
- Changed paths: AliyunDrive launcher, lifecycle host regression, source/RC semantic checker, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, and this log. No bundled binary, GPT Session Rotator, branch, PR or archived snapshot changed.
- Resulting main commit: THIS COMMIT.
- Next: verify every path-triggered workflow on the exact resulting SHA, especially QEMU env capability, private workdir/symlink/hardlink/umask cases, argv privacy and the source/RC checker. If green, resume FTP upload-cleanup TOCTOU design, then NFS client evidence and FRP/Adbyby whole-tree audit.

---

## 2026-11-13 06:39 CST (UTC+8) — Repair AliyunDrive semantic checker after credential argv regression update

- Base main SHA: `bf70e759b61e35022aa25bfe89a1c7cb3c0e24c7`; branch inventory remains `main` only.
- Exact-head AliyunDrive run [43321322383] / job [131542407443] on that SHA: shell syntax PASS; QEMU MIPSel `--version/--help` PASS, SHA-bound report confirmed all three env mappings; host lifecycle stop/start/readiness and environment/argv regression PASS. The final semantic source/RC checker failed on one obsolete required marker: it expected the prior host-test string “NVRAM values ... argv”, while the updated regression now emits “non-secret NVRAM values ... argv” and separate credential-environment/no-leak failure markers. This was a checker-only mismatch, not a launcher/QEMU/host regression failure.
- Other exact-head push workflows all SUCCESS on `bf70e759b61e35022aa25bfe89a1c7cb3c0e24c7`: AI docs [43321322563] / job [131542407353] (root log=80/119; archives 7/22/18), infosvr [43321322817] / job [131542409055], AliDDNS [43321323446] / job [131542412248], AdGuardHome [43321323806] / job [131542416886]. The AliyunDrive job failure is fully attributed to the stale marker assertion.
- THIS COMMIT updates `scripts/check_aliyundrive_webdav_lifecycle.py` to assert the new non-secret argv failure message plus explicit credential-env and credential-argv-leak markers. No production source/test behavior changed in this repair.
- Exact-head reruns for THIS COMMIT are PENDING; no local full checkout or local shell/Python execution is claimed.
- Root log count: 80→81 entries, under the 120-record and 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no rotation or index change is due.
- Changed paths: lifecycle semantic checker, live handoff, live roadmap, and this log entry. No archive, binary, GPT rotator, branch or PR changed.
- Resulting commit: THIS COMMIT.
- Next: inspect the AliyunDrive, AI docs and any other exact-head push workflows on the resulting main SHA. Require QEMU SHA-binding, shell syntax, host environment/argv, and semantic caller/source contracts all PASS before recording L2 closure; then inspect daemon workdir token persistence and permissions.

---

## 2026-11-13 06:29 CST (UTC+8) — Correct AliyunDrive credential capability evidence and redact daemon argv

- Base main SHA: `7309ae404e0172edac20ac91d710da74cf39722f`; branch inventory contained only `main`, legacy PR #1 remains closed/unmerged, and GPT Session Rotator was not modified.
- Exact-base AliyunDrive workflow [43318689422] / job [131533341859] and AI handoff docs [43318689207] / job [131533339554] were SUCCESS on that SHA. QEMU ran the packaged daemon: `aliyundrive-webdav 1.2.1`. Its `--help` explicitly advertised `REFRESH_TOKEN`, `WEBDAV_AUTH_USER`, and `WEBDAV_AUTH_PASSWORD`, plus all existing launcher options (`--domain-id`, `--workdir`, `--no-trash`, `--read-only`).
- Exact binary evidence: 3,029,740 bytes; SHA-256 `237c0ab2c14197c5a4e4bd8975f884979a29b35cad31b8e2354e084c19413891`; ELF32 little-endian MIPS32R2/O32, `e_flags=0x70001007`, no `PT_INTERP` (static). All 56 board templates explicitly disable the optional AliyunDrive package; one shared executable is installed behind the package gate.
- Found a false negative in the earlier checker: scanning for contiguous env-name bytes in the ELF did not reflect the CLI's actual Clap env configuration, while QEMU `--help` showed the compiled `[env: NAME=]` mappings. Updated `scripts/test_aliyundrive_webdav_binary.py` to require each mapping and emit a report bound to the binary SHA-256; `scripts/check_aliyundrive_webdav_lifecycle.py` now validates that report instead of raw-string inference.
- Switched `trunk/user/aliyundrive-webdav/aliyundrive-webdav.sh` to pass refresh token, WebDAV username and password as child-process environment assignments, removing `-r/-U/-W` secret values from argv. Updated `trunk/user/aliyundrive-webdav/tests/aliyundrive_webdav_lifecycle.sh` to independently assert exact environment values, preserve non-secret argv order/quoting and fail on any credential argv leak. Domain-id and optional no-trash/read-only policy remain unchanged.
- Tests and exact-head validation for THIS COMMIT are PENDING; no local full checkout or local test run is claimed. Base-SHA QEMU/lifecycle/source and docs workflows were green as listed above.
- Root log count: 79→80 complete records; size remains below 384 KiB and the 512 KiB hard limit. No archive rotation or `docs/dev-log/INDEX.md` change is due; existing log/handoff/roadmap archives remain untouched.
- Changed paths: QEMU binary probe, lifecycle source checker, AliyunDrive launcher, lifecycle host regression, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and `dev_log.md`. No bundled binary, GPT rotator, or archived snapshot changed.
- Resulting commit: THIS COMMIT.
- Next: verify AliyunDrive and AI docs `push` Actions on the resulting exact main SHA. Then inspect `/proc/PID/environ` and `--workdir` token persistence/permissions before closing the residual credential review; proceed to FTP cleanup TOCTOU afterward.

---

## 2026-11-13 06:12 CST (UTC+8) — Extend AliyunDrive bundled ELF and board-profile audit

- Base main SHA: `f53bdfb13aefa89201481628e6d0bcf07e47c2e5`; only `main` was present. All five exact-head workflows were SUCCESS: AI docs [43313655530], AliyunDrive [43313655603] / job [131515924146], infosvr [43313655609], AliDDNS [43313655587], and AdGuardHome [43313655487].
- The packaged 3,029,740-byte binary was previously identified as ELF32 little-endian MIPS (`e_machine=8`). Firmware compiler config says `mips32r2` / `mipsel-linux`, and uClibc config says 32-bit MIPS O32 little-endian. Exact MIPS ISA/ABI flags, PT_INTERP and runtime loader were not yet measured.
- Extended `scripts/check_aliyundrive_webdav_lifecycle.py` to parse ELF `e_type/e_machine/e_flags`, MIPS ISA and ABI flags, PT_INTERP, binary SHA-256 and bounded printable CLI/version/environment strings. It also scans every `trunk/configs/templates/*.config` and reports enabled/disabled/unset `CONFIG_FIRMWARE_INCLUDE_ALDRIVER` profiles. The checker requires broad ELF32/LE/MIPS compatibility and expected firmware compiler/uClibc target settings; it does not alter credential transport.
- Updated handoff/roadmap with the latest exact-head green workflows and current open argv disclosure. No binary replacement has been selected or installed. Upstream v1.1.1 source has Clap env support, but the bundled ELF lacks those env names and its source provenance remains unknown.
- This diagnostic commit's exact-head Actions are pending. No local checkout or local shell/Python execution is claimed; the five green runs above apply to parent `f53...`.
- Root dev_log count: 78→79, below 120-entry rotation and 384 KiB target. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no shard/index or routine snapshot changes.
- Next: inspect exact-head e_flags/interpreter/CLI/version output and all board-template settings; compare checksum-verified MIPSel candidates and prove ABI, loader, domain-id, workdir, read-only and no-trash compatibility before changing the installed binary or credential argv.

---

## 2026-11-13 05:42 CST (UTC+8) — Guard AliyunDrive credential transport against unsupported binary interfaces

- Main/source base: `0460f705b6ea3bbe1bcc9cfa61d66b677e5612da`; development remained on `main`. Only `main` was present in the remote branch inventory; legacy PR #1 remains closed/unmerged and GPT Session Rotator remains unchanged.
- Resumed the handoff's open AliyunDrive secret exposure. The launcher passes refresh token, WebDAV username and password as `-r/-U/-W`, which exposes those values in the daemon command line.
- Trial commit `f8fbf44529ed1be8c9b5462e6e219392d6b9a4a9` moved the values to `REFRESH_TOKEN`, `WEBDAV_AUTH_USER` and `WEBDAV_AUTH_PASSWORD` environment variables and extended argv/environment mock assertions. Launcher syntax and host lifecycle regressions passed, but exact-binary source contract [43310466543] / job [131504899043] failed: the packaged executable did not contain all three compiled environment-variable names. The env-only invocation was reverted; it was not allowed to silently break daemon startup.
- Commit `0460f705b6ea3bbe1bcc9cfa61d66b677e5612da` restored the known-working quoted argv options and changed `scripts/check_aliyundrive_webdav_lifecycle.py` to inspect the exact packaged executable. It requires env transport only when all three names are embedded in the binary; otherwise it requires the existing argv contract and emits an explicit warning. Exact-head AliyunDrive run [43312330424] / job [131511324570] SUCCESS: shell syntax, bounded stop, duplicate-start/argv integrity, serialized lifecycle lock, async process-presence startup and RC/source contracts all PASS. The warning remains; credentials are NOT redacted.
- The first binary classifier had an incorrectly escaped ELF magic literal, so the Actions log reported `unknown`; this handoff checkpoint corrects it using explicit ELF magic bytes and also prints the missing environment-variable names. The next exact-head run will establish the ELF class, byte order and `e_machine` value without inferring the binary's version.
- AI handoff docs run [43312330419] / job [131511324768] failed on the same source SHA because `SESSION_HANDOFF.md` lacked the required `## Active task` heading. This update restores that required heading and retains the prior AliDDNS work under a completed-task heading; the docs workflow on this exact checkpoint must be checked.
- Next: inspect exact ELF ABI/version and enumerate every board/configuration that uses this shared binary; locate or build an ABI-compatible daemon with verified environment credential support before changing launcher arguments. Preserve lifecycle locking and bounded startup behavior. FTP cleanup TOCTOU, NFS real-client evidence and whole-tree FRP/Adbyby remain queued. Environment variables remove ordinary argv exposure but remain readable to privileged root through `/proc/PID/environ`.
- No local repository checkout was available; local shell/Python execution is NOT CLAIMED. GitHub Actions provided the test evidence above. Current-main L3 firmware/TRX and L4 physical-device validation remain NOT VERIFIED.
- Root log count: 77→78 complete records, below the 120-record / 384 KiB rotation thresholds and 512 KiB hard cap. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no snapshot/index rotation is due.

---

## 2026-11-13 05:02 CST (UTC+8) — Close AliDDNS keeper identity and bounded-stop regression

- Source/test base SHA: `70db3735b4c129622914cee610ac02e8b99fb017`, reached directly on `main`; only the `main` branch was present. Legacy PR #1 remains closed/unmerged; GPT Session Rotator was not modified.
- Completed the previous handoff's AliDDNS `kill_ps()` task. Removed the broad `ps | grep | awk | eval` process selection. The launcher now enumerates `/proc/[0-9]*/cmdline`, converts NUL argv delimiters, preserves the terminal argv boundary lost by command substitution, and matches only the exact script path followed by the final `keep` token; it excludes the current shell and rejects lookalike path/action/extra-argument decoys.
- Added bounded process shutdown: revalidate the exact keeper argv before each signal, TERM with at most 5 one-second waits, KILL only still-matching PIDs, then at most 2 more seconds for the disappearance postcondition. A remaining keeper or unreadable /proc causes failure; start/restart will not mutate status or launch a replacement after a failed close.
- Expanded `trunk/user/aliddns/tests/aliddns_lifecycle.sh` for exact argv/final-token selection, self exclusion, decoys, graceful TERM, bounded escalation and stubborn-process restart gating. Expanded `scripts/check_aliddns_api_transport.py` to validate the exact process identity, no broad `ps/grep/eval`, bounded signals and fail-closed start ordering. Updated the HTTPD umbrella root-managed script contract to require the keeper-specific stop helpers and reject reintroduction of `kill_ps`.
- Intermediate CI failures were diagnosed and fixed, not hidden: run [43303863400] failed the first fixture because it used a literal dollar path instead of the test shell PID; [43304373740] then exposed command substitution stripping the final newline; [43305744923] ran host regressions but caught an incorrect semantic-checker assertion; the final checker fix landed in this SHA. An empty tree-identical commit `a8e30e38cf2c35387e5c7e409b314eb03f8878dd` exists in the main history and changed no files.
- Final L2: AliDDNS/API/keeper workflow [43306180515] / job [131489962084] SUCCESS and AI handoff docs [43306180459] / job [131489961692] SUCCESS on exact SHA `70db3735b4c129622914cee610ac02e8b99fb017`. Every job step and full log inspected; final-argv fixture markers, TERM/KILL bounded-stop marker, stubborn-process restart gate and all API/CA/HMAC tests PASS. HTTPD umbrella/security workflow [43305744811] / job [131488486331] passed on exact production-source SHA `8eec765e9a91c617eb2eb6fab582321a8e04966f`; the only later change before `70db...` is the semantic checker. A second full HTTPD run [43304853601] / job [131485302998] passed on `c30e9b29abc093963a4276ca22eeb2c7e18f1001`.
- An XY-C1 firmware workflow was auto-triggered by the existing path filter because commit `c30e9b29abc093963a4276ca22eeb2c7e18f1001` changed `.github/workflows/httpd-security.yml`. Run [43304853704] / job [131485300937] succeeded and uploaded artifact [13984450792]; the TRX image was 12,622,125 bytes, archive 12,575,877 bytes, uploaded-archive SHA-256 `4f24e15fab089dd7d7615c2c489eac8f6ccba439ab84e27fdb066ca70ab0f677`. This automatic ancestor build was not deliberately scheduled; source commits `8eec...` and `70db...` followed it, so current-main L3 remains NOT VERIFIED. Do not treat this as the planned project-complete build.
- No local full repository checkout was available; local shell/Python runs are NOT CLAIMED. GitHub Actions ran the launcher `sh -n`, host regressions and semantic checks. Current main L3 MIPS/uClibc/full firmware/TRX and L4 hardware behavior remain NOT VERIFIED. Linux 4.4 has no pidfd; a narrow PID-reuse race remains between the final `/proc` check and signalling.
- Root log count: 76→77 complete records, below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no archive/index rotation is due.
- Changed production/test/checker/workflow paths: `trunk/user/aliddns/aliddns.sh`, `trunk/user/aliddns/tests/aliddns_lifecycle.sh`, `scripts/check_aliddns_api_transport.py`, `.github/workflows/httpd-security.yml`; live continuation docs updated in this checkpoint: `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this `dev_log.md`. No archive snapshots or index changes.
- Next: investigate AliyunDrive refresh-token/auth-password exposure in process argv; inspect the daemon's supported secure config/stdin/descriptor/private-file interface before changing arguments. Preserve the existing lifecycle lock and bounded startup-process check; then continue FTP cleanup TOCTOU and FRP/Adbyby whole-tree audit.

---

## 2026-11-13 03:25 CST (UTC+8) — Serialize AliDDNS RC/UI lifecycle transitions

- Base main SHA: `18234ff0a41eb5343aaf58207b3cf641bbf4a244`; only `main` was present. Closed legacy PR #1 was inspected, not merged: its AliDDNS implementation, HMAC helper, package and host-test files are already byte-identical on main; the main workflow/checker contain the integrated coverage.
- Resumed and closed the previous AliyunDrive task before opening this module. On exact SHA `18234ff0a41eb5343aaf58207b3cf641bbf4a244`, all five push workflows completed SUCCESS: AI docs [43287688283] / job [131425309408], AdGuardHome [43287688231] / job [131425309147], AliyunDrive [43287688178] / job [131425308994], infosvr [43287688185] / job [131425309273], and AliDDNS API security [43287688650] / job [131425310006]. Every step and complete job log was inspected. AliyunDrive lifecycle markers for bounded stop, duplicate-start/argv, serialized transitions and async startup readiness all PASS; AI docs reported root log=75/119, archives 7/22/18.
- Traced AliDDNS's active control chain: HTTPD `variables.c` maps the enable/config/persistent-script fields to `EVM_RESTART_ALIDDNS`; PID 1's `rc.c` dispatches `RCN_RESTART_ALIDDNS`; `services.c` previously called stop then start as two separate launcher invocations. The shell's public start/stop/check paths had no common inter-process exclusion, so concurrent UI/RC notifications could interleave transitions.
- THIS COMMIT adds a stable `/tmp/.aliddns.lifecycle.lock` inode, checks directory/file type, owner, mode and link count, and uses the target BusyBox descriptor-mode `flock -n 9` with a bounded 10-second wait that fails closed. The lock file is never unlinked. Start, stop, restart and check share the same locked dispatcher; RC restart is now one shell `restart` action under a single lock.
- Detached `keep` and self-restart children explicitly close FD 9 so they cannot retain the launcher's lifecycle lock. The target config already contains `CONFIG_FLOCK=y`; BusyBox 1.24.2 `flock.c` accepts and locks an inherited descriptor. The semantic checker now asserts the UI→RC→service path, target config/implementation and workflow trigger coverage.
- Added `tests/aliddns_lifecycle.sh` for secure lock-file creation/rejection (symlink, hardlink and permissive mode), cross-process contention, zero-wait failure, action status propagation, FD release and atomic restart/stop/check dispatch. The new workflow step and full AliDDNS semantic contracts are pending exact-head Actions on THIS COMMIT. This environment has no local repository checkout; local `sh -n`, shell host test or Python checker execution is NOT CLAIMED.
- Explicit residual: legacy `kill_ps()` still uses broad `ps | grep` matching and sends TERM/KILL without a bounded process-disappearance postcondition. This commit serializes control transitions but does not claim safe process identity or confirmed stop. AliyunDrive secret argv, FTP upload-cleanup TOCTOU, NFS real-client evidence and FRP/Adbyby audit remain open.
- XY-C1 L3 remains verified only on `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`; this AliDDNS lifecycle slice has no target MIPS/uClibc build, firmware/TRX or L4 test. L4 remains NOT VERIFIED.
- Root `dev_log.md`: 75→76 complete records, below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no archive or index rotation is due.
- Changed paths: `trunk/user/aliddns/aliddns.sh`, new `trunk/user/aliddns/tests/aliddns_lifecycle.sh`, `scripts/check_aliddns_api_transport.py`, `.github/workflows/aliddns-api-transport.yml`, `trunk/user/rc/src/services.c`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Resulting main commit: THIS COMMIT.
- Next: inspect exact-head AliDDNS lifecycle/API and AI handoff docs push jobs, require every lifecycle lock/atomic restart/secret-safe API test and archive validator to pass, repair any first causal failure on `main`; then design a safe process-identity and bounded-stop policy for `kill_ps()`.

---

## 2026-11-13 03:10 CST (UTC+8) — Make AliyunDrive readiness fixture schedule async daemon

- Base main SHA: `5488ed5adbaf6afb1209014280305cfc093168cd`. The previous exact-head retry [43286837341] / job [131422349252] again passed launcher `sh -n` but failed the lifecycle host regression at `enabled start failed`; source/RC semantic check was skipped.
- The prior correction had reset `TEST_ENABLE=1`, so the enabled launch path was entered. The remaining fixture defect was its overridden `sleep()`: it only incremented a counter and returned without delaying or yielding, allowing all readiness polls to finish before the asynchronously launched mock shell could create the readiness marker.
- THIS COMMIT updates only the host regression fixture: the sleep shim now performs a real 50 ms scheduler yield after counting each poll; startup timeout is simulated by an explicit failure-sentinel file rather than an exported mode variable. The test still requires an argv log, exactly the configured bounded poll count and absence of the readiness marker; the normal enabled and special-domain starts can now publish the marker asynchronously.
- The prior exact-head AI docs [43286837520] and infosvr [43286837409] passed on `5488ed5adbaf6afb1209014280305cfc093168cd`. AdGuardHome [43286837432] and AliDDNS [43286837560] were in progress at last observation; inspect their final status separately.
- The lifecycle L2 result is still OPEN pending a fresh AliyunDrive push Actions run on THIS COMMIT. This session has no local repository checkout; no local shell syntax, host test or Python checker execution is claimed.
- Root `dev_log.md`: 74→75 complete records, below the 120-record / 384 KiB archive thresholds. No archive shard or index rotation is due. XY-C1 L3 remains verified only on `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`; L4 is NOT VERIFIED.
- Next: inspect exact-head AliyunDrive and AI docs push runs for THIS COMMIT and read complete job logs. Require both start/stop contention cases, startup timeout, successful asynchronous readiness, argv/domain-profile cases and semantic BusyBox-flock/RC checks to pass. Fix any first causal failure on `main`, then resume AliDDNS lifecycle concurrency and FRP/Adbyby review without dropping argv-secret, FTP TOCTOU or NFS evidence items.

---

## 2026-11-13 03:05 CST (UTC+8) — Correct AliyunDrive startup-timeout host fixture state

- Base main SHA: `23f39e16d3e06c40078c139289d4407e123529de`. The implementation commit added shared BusyBox-flock serialization for AliyunDrive start/stop and a bounded 10-second process-presence startup postcondition.
- Exact-head AliyunDrive push run [43285606600] / job [131418092716] checked out this exact SHA. Launcher `sh -n` passed. The lifecycle host regression failed at `startup timeout was reported successful`; semantic source/RC checks were skipped.
- Root cause is in the new regression fixture, not evidence of a production-source defect: the preceding disabled-service test leaves `TEST_ENABLE=0`, so the startup-timeout scenario followed the disabled stop path and returned success without invoking the mock daemon. The correction explicitly sets `TEST_ENABLE=1` for the timeout case and asserts that the mock argv log is created, exactly the configured poll count elapses and no readiness marker appears.
- Same-SHA exact-head push workflows completed SUCCESS: AI handoff docs [43285606136], AdGuardHome [43285606409], infosvr [43285605946] and AliDDNS [43285606280]. AliyunDrive L2 remains open until its fresh exact-head lifecycle regression and semantic source contract both pass.
- The follow-up test-only correction and updated handoff/roadmap/root log are THIS COMMIT. No production launcher change is made in this correction. This environment has no local repository checkout; no local `sh -n`, shell regression or Python checker execution is claimed.
- Root `dev_log.md`: 73→74 complete records, below the 120-record / 384 KiB archive thresholds; no archive/index rotation is due.
- XY-C1 L3 evidence remains tied only to `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`; no firmware/TRX build was run for this lifecycle iteration. L4 remains NOT VERIFIED.
- Next: inspect exact-head push runs for the new main SHA, especially AliyunDrive lifecycle and AI docs. Require the corrected timeout fixture, all lock/readiness lifecycle cases, the BusyBox-flock/RC semantic contract and archive validator to pass; repair the first causal failure directly on `main`. After closing this regression, continue with AliDDNS lifecycle concurrency and whole-tree FRP/Adbyby audit while keeping argv-secret exposure, FTP TOCTOU and NFS evidence open.

---

## 2026-11-13 02:31 CST (UTC+8) — Serialize AliyunDrive lifecycle and gate startup on process visibility

- Base main SHA: `04abdb06ccc82ba6e7ffb65c028a0db3d3889272`. The previous P1 storage-hotplug host BusyBox issue is closed on this exact SHA: run [43278371123] / job [131392758595] passed the host BusyBox 1.24.2 link and every downstream storage regression; AI handoff docs [43278370932] / job [131392757241] also passed. The XY-C1 full MIPS/TRX/artifact milestone remains verified only on `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`.
- Whole-tree triage returned to AliyunDrive WebDAV. Its previous bounded stop and sequential duplicate-start checks did not serialize concurrent RC/UI start and stop notifications, and the launch path returned immediately without confirming that the daemon became observable.
- THIS COMMIT wraps both public start and stop in a nonblocking BusyBox `flock` lock on a stable `/var/lock/aliyundrive-webdav.lock` inode. The lock remains held through the transition and startup check; it is not unlinked, the disabled-start path calls the unlocked stop helper to avoid nested acquisition, and FD 9 is closed in the daemon child to prevent the service retaining the lock.
- Start now waits at most 10 seconds for `pidof aliyundrive-webdav` and returns failure if no process becomes observable. This is a process-presence postcondition only, not an HTTP/WebDAV readiness probe.
- Extended the POSIX-shell lifecycle regression with both start and stop lock-contention cases, a valid mock daemon that becomes visible, a launched-but-never-visible timeout case, and the existing stop/duplicate-start/argv/domain-profile coverage. Extended the semantic contract to require target `CONFIG_FLOCK=y`, BusyBox `flock` nonblocking/unlock support and symmetric push/pull workflow coverage for the dependencies.
- Changed paths: `trunk/user/aliyundrive-webdav/aliyundrive-webdav.sh`, `trunk/user/aliyundrive-webdav/tests/aliyundrive_webdav_lifecycle.sh`, `scripts/check_aliyundrive_webdav_lifecycle.py`, `.github/workflows/aliyundrive-webdav-lifecycle.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- This session has no local repository checkout, so local `sh -n`, host regression or Python checker execution is NOT CLAIMED. The exact-head AliyunDrive lifecycle and AI handoff documentation push Actions for THIS COMMIT are pending; do not close L2 until both pass and every job step is inspected.
- Root `dev_log.md`: 72→73 complete records, below the 120-record and 384 KiB archive thresholds; no archive or index rotation is due. Existing archives and `docs/dev-log/INDEX.md` remain unchanged.
- Resulting main commit: THIS COMMIT. L3 target build and L4 physical-router validation are not part of this lifecycle change; L4 remains NOT VERIFIED.
- Next: inspect AliyunDrive lifecycle and AI handoff docs push runs whose `head_sha` equals this commit; repair the first causal failure directly on `main`. After L2 closes, continue outside this module with AliDDNS lifecycle concurrency and the whole-tree FRP/Adbyby audit. Keep AliyunDrive argv-secret exposure, FTP TOCTOU and NFS real-client evidence open.

---

## 2026-11-13 02:04 CST (UTC+8) — Repair storage-hotplug host BusyBox shared-header setup

- Base main SHA: `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`. Exact-head XY-C1 push [43273846470] / job [131376901927] PASSED all header prechecks, the MIPS firmware build, TRX output verification and artifact upload. The uploaded `XY-C1-firmware-fcdc05e76e6522f9f35b59b71bd222b884cb2ce4` artifact is 12,573,414 bytes (SHA-256 `f4adfa9a13224e519cd580946e30e6a3d336b3da410f0842051eb697a7baee08`).
- Six other exact-head push workflows passed on this SHA: infosvr [43273846610], AliyunDrive [43273846558], AdGuardHome [43273846504], AI handoff docs [43273846442], HTTPD security [43273846549] and AliDDNS [43273846513].
- Storage-hotplug [43273846489] / job [131376902484] failed in the direct host BusyBox 1.24.2 build at `dhcp6c_common.c:38`: the newly shared `dhcp6c_duid_file_policy.h` is not found. Earlier shell and Optware lifecycle checks passed. The host workflow copies the firmware config then calls BusyBox make directly, bypassing the package.mk `MAKE_VARS` path that supplies production `EXTRA_CFLAGS`; board config currently keeps `CONFIG_EXTRA_CFLAGS=""`.
- THIS COMMIT gives the disposable host `.config` the same shared source include through `CONFIG_EXTRA_CFLAGS`, verifies Kconfig retained the exact include after `oldconfig`, and extends the semantic contract to check the DHCPv6 source include, production package flag, BusyBox flag mapping and host build propagation. The tracked firmware board config and production BusyBox behavior are unchanged.
- Changed paths: `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root `dev_log.md`: 71→72 complete records, below the 120-record and 384 KiB archive thresholds; no archive/index rotation is due.
- Resulting main commit: THIS COMMIT. Exact-head storage-hotplug and AI handoff docs results are pending this commit; target L3 XY-C1 build/TRX/artifact is verified on the base SHA, L4 is NOT VERIFIED.
- Next: inspect storage-hotplug and AI docs push runs for the exact new main SHA; require the BusyBox host build and every downstream storage regression to pass, then resume whole-tree stability/security triage.

---

## 2026-11-13 03:01 CST (UTC+8) — Fix BusyBox ROMFS recursive Kbuild header path

- Base main SHA: a3247c882afdc66b781d060b47e4182b9ad197e1; exact-head XY-C1 43270739238 / job 131366274506.
- Samba shared-header, DHCPv6 DUID staged-header and vsftpd self-contained-header checks all passed. The MIPS build compiled RC and vsftpd, progressed into user-space packages, then failed at ROMFS busybox: networking/udhcp/dhcp6c_common.c:38 fatal error: dhcp6c_duid_file_policy.h: No such file or directory.
- Full job log distinguishes two BusyBox Kbuild invocations. The initial BUILD busybox command included CFLAGS=... -I$(ROOTDIR)/user/shared/src, and dhcp6c_common.o compiled successfully. The later custom romfs recipe called $(MAKE) ... $(MAKE_FLAGS) install without package $(MAKE_VARS); the recursive build lost its CFLAGS/EXTRA_CFLAGS and recompiled without the header path. BusyBox precedes shared in package order, so the source-tree include path is intentional and safer than depending on a not-yet-built staged header.
- THIS COMMIT prefixes the BusyBox romfs recursive make with +$(MAKE_VARS), preserving CFLAGS on the re-entry while retaining $(ROOTDIR)/user/shared/src include resolution.
- The DUID regression is strengthened to assert the BusyBox source include, BusyBox-before-shared ordering, shared source include flag, the package.mk MAKE_VARS CFLAGS expansion, the BusyBox romfs propagation and a standalone GNU89 compile through the exact source include directory.
- The XY-C1 workflow already filters trunk/user/busybox/Makefile and the BusyBox DHCPv6 source, so these changes trigger the next exact-head validation.
- Target build failure on base SHA; no TRX or artifact success claimed. L4 router validation NOT VERIFIED.
- Root dev_log.md: 70→71 complete records, still below 120-record and 384 KiB archive thresholds; no archive/index rotation due.
- Changed paths: trunk/user/busybox/Makefile, scripts/check_dhcp6c_duid_header_staging.py, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log.
- Resulting main commit: THIS COMMIT.
- Next: verify exact-head XY-C1 DUID/BusyBox/vsftpd prechecks, fix the next first-causal build failure, and continue until firmware build, non-empty TRX verification and artifact upload all pass on one SHA.

---

## 2026-11-13 02:32 CST (UTC+8) — Isolate the vsftpd header probe from system include lookup

- Base main SHA: \`3086848304e5324d2255ed005f82b7ebc84503ce\`.
- Exact-head XY-C1 [43270075912] / job [131363959378] passed Samba and DUID staged-header checks, then failed the newly introduced vsftpd public-header probe before starting the target firmware build. Its \`cc\` command used \`-I trunk/user/vsftpd/src\`; the folder contains its own \`features.h\`, so placing it in angle-bracket search order shadowed the host glibc \`features.h\`. The resulting \`__GLIBC_USE\` preprocessing errors are caused by the probe include mode, not by the production header.
- The previous first causal vsftpd production error was \`asus_ext.h:55:27: unknown type name 'size_t'\`, already fixed with \`#include <stddef.h>\` and validated by the separate vsftpd config workflow [43270075357] PASS.
- THIS COMMIT changes only the XY-C1 probe search option to GCC \`-iquote trunk/user/vsftpd/src\`; this resolves the quoted project header without exposing the local \`features.h\` to system angle-bracket include lookup. No change is made to the production header.
- Same-SHA HTTPD [43270075491] and AI handoff docs [43270075268] PASS. Samba and DUID source checks PASS in the XY-C1 run.
- Target MIPS firmware build was skipped because the host precheck failed; TRX validation/artifact upload NOT VERIFIED. L4 hardware validation NOT VERIFIED.
- Root \`dev_log.md\`: 69→70 complete records, below 120-record rotation threshold and 384 KiB early-rotation target; no archive/index rotation is due.
- Changed paths: \`.github/workflows/xy-c1-firmware-validation.yml\`, \`SESSION_HANDOFF.md\`, \`REFACTOR_ROADMAP.md\` and this log.
- Resulting main commit: THIS COMMIT.
- Next: check the exact-head XY-C1 push, require all pre-build contracts to pass and continue through build, non-empty TRX validation and artifact upload; repair the next causal failure if one appears.

---

## 2026-11-13 02:24 CST (UTC+8) — Fix next XY-C1 blocker: vsftpd size_t and DUID contract

- Base main SHA: \`f80a5f6b89d7e7fca7e3f7d2f5fe55413ce409ad\`; branch policy remains main-only.
- Exact-head XY-C1 run [43266929320], job [131352848613], on this SHA: Samba shared-header contract PASS; DHCPv6 DUID staged-header GNU89 contract PASS; RC progressed beyond the prior \`net_wan6.o\` missing-header failure. First fatal compile error occurred at \`trunk/build/vsftpd/main.o\`: \`asus_ext.h:55:27: unknown type name 'size_t'\`. The source header used \`size_t\` but included only \`<limits.h>\`; the proper self-contained declaration source is \`<stddef.h>\`.
- Same-SHA HTTPD security [43266928890] failed at its DUID source contract only: the assertion still expected the superseded relative include in \`net_wan6.c\`. The correct include is now staged-public \`<dhcp6c_duid_file_policy.h>\`; update the assertion and check the obsolete source-relative path is absent.
- This commit adds \`#include <stddef.h>\` to \`trunk/user/vsftpd/src/asus_ext.h\`, adds a host GNU89 \`-Werror -fsyntax-only\` standalone-header check, adds XY-C1 trigger coverage for the vsftpd header and HTTPD workflow contract, and corrects the HTTPD DUID assertion.
- AI handoff docs [43266928614] PASS. WAN DHCP [43266928617] remains a separate contract matcher failure.
- Full XY-C1 build, TRX validation and artifact upload: NOT VERIFIED. The next exact-head push run must pass all three source/header checks and continue through target build. L4 hardware validation: NOT VERIFIED.
- Root \`dev_log.md\`: 68→69 complete records, below the 120-record rotation threshold and 384 KiB early-rotation target; file remains below the 512 KiB hard cap. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no archive/index rotation is due.
- Changed paths: \`trunk/user/vsftpd/src/asus_ext.h\`, \`.github/workflows/xy-c1-firmware-validation.yml\`, \`.github/workflows/httpd-security.yml\`, \`SESSION_HANDOFF.md\`, \`REFACTOR_ROADMAP.md\` and this log.
- Resulting main commit: THIS COMMIT.
- Next: inspect the exact-head XY-C1 push run for this commit. If another first-causal failure appears, repair it directly on \`main\` and repeat until firmware build, non-empty TRX verification, artifact upload and the HTTPD DUID contract all pass on one exact main SHA.

---

## 2026-11-13 01:02 CST (UTC+8) — Fix XY-C1 RC IPv6 DUID staged-header lookup

- Base main SHA: \`9a508588a33c220b334d1dd40012eb5fd5ac89ec\`; remote branch inventory contained only \`main\`.
- Exact-head XY-C1 push run [43263025719], job [131339067215], checked out this SHA. The kernel, BusyBox and HTTPD stages completed; RC compilation reached \`net_wan6.o\` and failed at \`net_wan6.c:34\` because \`../../shared/src/dhcp6c_duid_file_policy.h\` is a source-tree-relative path that is not valid in the staged package tree.
- Confirmed \`trunk/user/shared/src/Makefile\` installs root-level headers into \`$(DESTDIR)/include/shared\`, the RC package adds \`-I$(STAGING_DIR)/include/shared\`, and \`trunk/user/Makefile\` builds shared before RC. Therefore the narrow fix is to include \`<dhcp6c_duid_file_policy.h>\` from the staged include path; no duplicate header or runtime-policy change is required.
- THIS COMMIT switches \`net_wan6.c\` to the public staged include name, adds a GNU89 syntax-only regression against a temporary \`stage/include/shared\` layout, and expands XY-C1 push filters to cover the IPv6 RC source and checker. The contract runs before the target build.
- On the base SHA, AI docs [43263025362] and HTTPD security [43263025674] SUCCESS. Separate failures storage-hotplug [43263025900] and WAN DHCP [43263025911] are unrelated to this target compiler error and remain explicit follow-up CI work.
- Full local repository checkout/target cross-build: NOT RUN. The new regression and exact-head XY-C1 firmware build, non-empty TRX validation and artifact upload await push Actions. No TRX or artifact success is claimed. L4 hardware/router validation: NOT VERIFIED.
- Root \`dev_log.md\`: 67→68 complete records, below the 120-record rotation threshold and 384 KiB early-rotation target; file remains below the 512 KiB hard cap. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no archive/index rotation is due.
- Changed paths: \`trunk/user/rc/src/net_wan6.c\`, new \`scripts/check_dhcp6c_duid_header_staging.py\`, \`.github/workflows/xy-c1-firmware-validation.yml\`, \`SESSION_HANDOFF.md\`, \`REFACTOR_ROADMAP.md\` and this log.
- Resulting main commit: THIS COMMIT.
- Next: inspect the exact-head XY-C1 push run for this commit. Require the DUID staged-header regression to pass; then repair only the next first-causal compiler or image-assembly error and continue until build, non-empty TRX validation and artifact upload succeed on one exact main SHA.

---

## 2026-11-13 00:43 CST (UTC+8) — Repair XY-C1 RC ctype/prototype errors and restore Samba host regression

- Base main SHA: `98dedc29c55b2699857c05f3c708188e0f7d56dc`; branch inventory at recovery contained only `main`.
- Exact-head XY-C1 push run [43259091641], job [131325219123], checked out this SHA. The new Samba shared-header staging regression passed; kernel, BusyBox DHCPv6 and HTTPD work progressed, and the target build reached RC `services_stor.o`. The first causal C errors were: `shutils.h` defined the fallback `isblank(c)` macro before libc `ctype.h` declared `isblank(int)`, causing macro expansion in the system header; and `services_stor.c` implemented `int stop_stor_apps(void)` while `rc.h` declared it `void`. TRX verification and artifact upload were skipped after the build failure.
- Exact-head HTTPD security run [43259091559], job [131325220100], reached the Samba share host regression and failed because its compile command omitted `-I trunk/user/shared/src/include`, required by the corrected public `<router_account_policy.h>` include. Earlier regression steps passed.
- THIS COMMIT includes the narrow compile fixes: include system `<ctype.h>` before the shared `isblank` fallback; update the RC declaration to `int stop_stor_apps(void)` matching its implementation and status-checking callers; and add the shared public-header include directory to the first HTTPD Samba host compile.
- Extended `scripts/check_samba_shared_header.py` to enforce ctype include order, the status-returning RC prototype and XY-C1 path coverage. Extended the storage-hotplug source contract for the prototype and XY-C1 push paths to watch both changed headers. Updated the HTTPD workflow so its missing include-path regression is covered on push.
- On this base SHA, exact-head AI docs [43259091723], infosvr [43259091710], AdGuardHome [43259091774], AliyunDrive [43259091586] and AliDDNS [43259092023] all SUCCESS. HTTPD and XY-C1 were the two failures being repaired.
- Full local repository checkout/target cross-build: NOT RUN in this environment. All code, workflow and documentation changes in THIS COMMIT await exact-head push Actions; no TRX or artifact success is claimed. L4 hardware/router validation: NOT VERIFIED.
- Root `dev_log.md`: 66→67 complete records, below the 120-record rotation threshold and 512 KiB hard cap. Archive inventory remains dev-log=7, handoff=22, roadmap=18; no archive/index rotation is due.
- Changed paths: `trunk/user/shared/src/shutils.h`, `trunk/user/rc/src/rc.h`, `.github/workflows/httpd-security.yml`, `.github/workflows/xy-c1-firmware-validation.yml`, `.github/workflows/storage-hotplug-stability.yml`, `scripts/check_samba_shared_header.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Resulting main commit: THIS COMMIT.
- Next: inspect exact-head XY-C1, HTTPD security, storage-hotplug and AI docs Actions on the resulting main SHA. Fix only the next first-causal XY-C1 build error and repeat until kernel/userspace build, non-empty TRX verification and artifact upload all pass.

---

## 2026-11-13 00:24 CST (UTC+8) — Repair XY-C1 RC Samba shared-policy header lookup

- Base main SHA: `696278e438243af3ee67ab6e403dc27c7e0753df`.
- The latest XY-C1 push run before this repair, [43233088562](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43233088562), checked out `4890bd4f800d3d1f1f282061f8fe7ea8e533a969` and failed while compiling the RC package's `services_stor.o`. That source includes the local `samba_share_policy.h`, whose line 6 referenced `../../shared/src/include/router_account_policy.h`. The RC source is staged under `trunk/build/rc`, so the source-tree-relative path is invalid there.
- Confirmed the canonical header is installed by the shared package at `$(STAGING_DIR)/include/shared`, and the RC package explicitly adds `-I$(STAGING_DIR)/include/shared` to `TARGET_CFLAGS`. The compiler log showed that exact staged include directory.
- Change the policy header to include `<router_account_policy.h>`; no account validation, ACL or Samba behavior changes. Add a host GNU89 syntax-only regression that compiles the actual policy header against a fixture matching the staged include layout and checks package installation/build order and RC flags.
- Extend the push-only XY-C1 workflow to run the regression and trigger on all RC/shared-header inputs involved in the contract.
- Local full target build: NOT RUN (no full checkout in this environment). The new regression and exact-head MIPS build, non-empty TRX validation and artifact upload are PENDING the push Actions run. L4 hardware/router validation: NOT VERIFIED.
- Resulting main commit: THIS COMMIT.
- Next: inspect the XY-C1 run whose `head_sha` equals the resulting main SHA; fix only its next first-causal error and repeat until build, TRX validation and artifact upload all pass.
---

## 2026-11-12 22:40 CST (UTC+8) — Port and verify legacy PR #1 AliDDNS security work on main

- Base main SHA: `fd9275273c385ec266333fecb813fe7d18c69ecb`; resulting main commit: `74b66ff960457379518d13c723c44a3515e6a228`. Imported valid unmerged changes from legacy PR #1 head `ba48e22609e0c81ad757b06756fe91d05f39cbf6` without merging the diverged PR or committing to its branch.
- Ported CA trust-store setup in `copyscripts.sh`: checked directory creation, tar extraction result and readable CA bundle postcondition; unrelated startup asset seeding is preserved and CA failure propagates to the existing synchronous RC caller.
- Ported OpenSSL 3 EVP_MAC HMAC-SHA1 helper, package/ROMFS integration and `aliddns.sh` wrapper so AccessKeySecret is streamed over stdin rather than passed as an OpenSSL process argument. Added byte-equivalence/RFC 2202, boundary, process argv/environment and CA bootstrap regressions.
- Ported semantic checker and AliDDNS transport workflow; workflow is push-only, and checker requires a main-push trigger with all production/test paths while rejecting `pull_request`.
- Exact-head Actions on main SHA `74b66ff960457379518d13c723c44a3515e6a228`: AliDDNS [43251115223], AI docs [43251115176], infosvr [43251115160], AdGuardHome [43251115222], HTTPD [43251115204] and AliyunDrive [43251115533] all SUCCESS. The AliDDNS job executed all syntax, transport, IPv4, DNS, custom-script, CA-bootstrap, HMAC privacy and semantic-contract steps.
- PR #1 was closed (not merged) after the import and exact-head checks passed. Closing comment [6828320634](https://github.com/Arlene206/padavan-4.4-2/pull/1#issuecomment-6828320634) documents the port. The legacy branch remains untouched/read-only because no safe branch-delete operation is exposed; no unmerged content was discarded.
- GPT Session Rotator remained unchanged. Target MIPS/uClibc helper integration, full firmware/TRX and L4 behavior remain NOT VERIFIED.
- Resulting commit: `74b66ff960457379518d13c723c44a3515e6a228`.
## 2026-11-12 22:36 CST (UTC+8) — Restore the required handoff state heading

- Base main SHA: `bb4dd126091d90950206f6d70dd2432a691a9301`; follow-up commit `fd9275273c385ec266333fecb813fe7d18c69ecb` restored the mandatory `## Current repository state` heading that was omitted when the handoff's first state bullet was replaced.
- The omission caused AI handoff documentation run [43250319806] and infosvr contract run [43250319742] to fail on the prior exact SHA. After the correction, AI docs [43250648231] and infosvr [43250648516] both passed on exact main SHA `fd9275273c385ec266333fecb813fe7d18c69ecb`.
- Resulting commit: `fd9275273c385ec266333fecb813fe7d18c69ecb`.
## 2026-11-12 22:31 CST (UTC+8) — Enforce main-only development and push-only validation

- Base main SHA: `4890bd4f800d3d1f1f282061f8fe7ea8e533a969`.
- Updated `AI_BOOTSTRAP_PROMPT.md` and `AI_DEVELOPMENT_PROTOCOL.md`: `main` is the only authorized development/commit/push branch; no feature/fix/temp/checkpoint branches or PR development/validation; every independent fix is committed directly to main; only exact-head `push` Actions validate it; red checks are fixed on main; no force-push/history rewrite; unfinished work is handed off without a branch.
- Added safe legacy PR #1 handling: inventory and preserve valid unmerged content, port it to main, verify on main, then close the obsolete PR. Explicitly prohibit discarding valid work or continuing to develop on the legacy branch.
- Declared GPT Session Rotator immutable: preserve its current candidate-SHA, 12-minute quiet-period and de-duplication behavior; do not reset/replay `9d5a5cb` or manually trigger it.
- Removed the `pull_request` trigger from `.github/workflows/ai-handoff-docs.yml`; documentation validation remains available on `push` to main and manual dispatch. The rotator itself was not modified.
- Local `scripts/check_ai_handoff_docs.py`: NOT RUN (no local repository checkout). Exact-head AI documentation Actions: PENDING for the resulting main SHA.
- Current main baseline failures before this governance commit: WAN DHCP source contract [43233088640] fails while locating `static int man_down_internal(`; XY-C1 build [43233088562] fails later at `samba_share_policy.h` include lookup. These remain open and must be resumed on main after this bounded governance change.
- Resulting commit: THIS COMMIT.
## 2026-11-12 21:58 CST (UTC+8) — Fix duplicate internal declarations blocking RC target build

- Base main SHA: `10ebefe94bf344d19d8506287125f52333596bde`. Exact-head XY-C1 run [43230552179](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43230552179), job [131224406701](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43230552179/job/131224406701), progressed past the previously fixed kernel, BusyBox DHCPv6 and HTTPD compile blockers. The MIPS kernel completed; `xt_webstr.o`, `dhcp6c_common.o`, `web_ex.o` and the HTTPD link all completed.
- The next first-causal failure was `trunk/user/rc/src/net_wan.c`: duplicate declaration specifiers at lines 1592, 3230 and 3276 (`static int` repeated), plus lines 3547 and 3617 (`int` followed by `static int`). The affected helpers are referenced only inside this translation unit and have no public header declaration; each should be a single private `static int` definition.
- THIS COMMIT removes only the redundant declaration specifiers from `man_down_internal()`, `zcip_bound()`, `zcip_viptv_bound()`, `manv_route_deconfig()` and `zcip_viptv_deconfig()`. Their bodies, callback order, return values and WAN state/route behavior are unchanged.
- The WAN DHCP callback source contract now checks the five private declaration signatures, preventing duplicate specifiers from recurring unnoticed. The XY-C1 validation push trigger now includes `net_wan.c`, ensuring future WAN target-source edits run the complete MIPS firmware build.
- Prior exact-head HTTPD security [43230551897] and AI handoff docs [43230551924] completed SUCCESS on `10ebefe94bf344d19d8506287125f52333596bde`; the HTTPD security job passed all 40 steps, and the docs validator reported root log=61/119 (archives 7/22/18). This new WAN source contract and RC compile fix await exact-head Actions.
- Target RC compile / full firmware / TRX / artifact: PENDING exact-head XY-C1 run. L4 router validation: NOT VERIFIED.
- Resulting commit: THIS COMMIT.
- Next: inspect exact-head WAN callback, AI docs and XY-C1 runs. Confirm `net_wan.o` compiles, then continue through any remaining first-causal build errors until TRX validation and upload both pass.

---

## 2026-11-12 21:42 CST (UTC+8) — Make HTTPD NVRAM restore error branch compile with logging macro

- Base main SHA: `e0e53f6d2eb9d5f8836ea7c1335e7c9d76e2faea`. The second XY-C1 retry [43228446974](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43228446974), job [131217642154](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43228446974/job/131217642154), confirmed both preceding fixes: kernel build completed and `net/netfilter/xt_webstr.o` passed; BusyBox staged build compiled `networking/udhcp/dhcp6c_common.o` with the resolved `-I.../trunk/user/shared/src` path. No earlier error remains.
- The next first-causal target compile failure is in `trunk/user/httpd/src/web_ex.c:4681`, inside `do_restore_nv_cgi()`: `httpd_log` is defined in `httpd.h` as a statement-like macro that already emits a trailing semicolon. It was used as the unbraced body of `if (ret != 0)` immediately before `else`; macro expansion introduced an empty statement between the if body and else, producing `expected '}' before 'else'`.
- THIS COMMIT braces both error-log branches. It does not change restore/commit gating, watchdog recovery, status propagation or reboot behavior. The HTTPD source contract now asserts this semicolon-safe braced if/else shape, and XY-C1 validation also triggers on `web_ex.c` changes.
- Exact-head prior HTTPD umbrella [43228446901] and AI handoff docs [43228446814] on `e0e53f6d2eb9d5f8836ea7c1335e7c9d76e2faea` completed SUCCESS; HTTPD's DHCPv6 DUID regression and the package include-path contract both printed PASS. Its compiler source now has a new focused assertion.
- Target compiler / HTTPD object build / full firmware / TRX: PENDING exact-head Actions. L4 router validation: NOT VERIFIED.
- Resulting commit: THIS COMMIT.
- Next: inspect exact-head XY-C1, HTTPD security and AI docs. Confirm `web_ex.o` compiles, then proceed through remaining userland packages and firmware image assembly; fix only the next first-causal failure.

---

## 2026-11-12 21:28 CST (UTC+8) — Fix BusyBox DHCPv6 shared-header lookup in staged package builds

- Base main SHA: `b4b8074608aa118c6fdf99e2e70bc0ed6826b1ea`. The first narrow kernel repair compiled successfully: the exact-head XY-C1 log for [43226143358](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43226143358), job [131209716854](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43226143358/job/131209716854), contains `CC net/netfilter/xt_webstr.o` with no IP_OFFSET error. The build proceeded into the BusyBox package.
- The next first-causal failure was `networking/udhcp/dhcp6c_common.c:38: fatal error: ../../../../shared/src/dhcp6c_duid_file_policy.h: No such file or directory`. The tracked source-relative include was evaluated after `package.mk` copied BusyBox into `trunk/build/busybox-1.24`; that staging directory has no sibling `shared/src`.
- THIS COMMIT changes the BusyBox source to include the shared policy by basename and adds `-I$(ROOTDIR)/user/shared/src` through the package's existing `EXTRA_CFLAGS` / `MAKE_VARS` mechanism. This preserves one canonical shared header and works from the staged build tree rather than relying on the source checkout's relative depth.
- The existing HTTPD security contract now checks the basename include, package include search path and symmetric push/PR filters. XY-C1 firmware validation is also triggered by changes to the BusyBox source, package Makefile or shared header.
- Target compiler / full firmware / host DUID regression: PENDING exact-head Actions. L3 firmware/TRX and artifact: NOT VERIFIED. L4 router validation: NOT VERIFIED.
- Resulting commit: THIS COMMIT.
- Next: inspect exact-head XY-C1, HTTPD security and AI docs logs. Confirm BusyBox reaches past `dhcp6c_common.o`; fix only the next first-causal build error and repeat until the firmware/TRX/artifact steps pass.

---

## 2026-11-12 20:22 CST (UTC+8) — Repair XY-C1 xt_webstr IP_OFFSET kernel-header dependency

- Base main SHA: `9d5a5cb4ab574c0e610ed7a0815c0b1ad749da74`. The failed XY-C1 Firmware Validation run was [43213681479](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43213681479), job [131167417183](https://github.com/Arlene206/padavan-4.4-2/actions/runs/43213681479/job/131167417183), checked out this exact SHA.
- The MIPS kernel build stopped at `trunk/linux-4.4.x/net/netfilter/xt_webstr.c:364`: `IP_OFFSET` was undeclared. The symbol is defined as `0x1FFF` in the kernel-internal `include/net/ip.h`; the source included `<linux/ip.h>` (which defines `struct iphdr`) but not the internal header that owns the fragment-offset mask.
- THIS COMMIT adds the explicit `<net/ip.h>` dependency without changing the existing non-initial IPv4-fragment rejection condition or first-fragment behavior. The Xtables source contract now checks both the include and the canonical kernel definition; its push/PR filters also watch `include/net/ip.h`.
- The XY-C1 validation workflow now runs on pushes changing `xt_webstr.c` (and its own trigger definition), in addition to manual dispatch. This ensures the actual `mipsel-linux-musl` firmware build and TRX verification execute for this fix.
- Local full checkout / target compiler execution: NOT RUN. Xtables host regression and semantic contract: PENDING exact-head Actions. XY-C1 L3 build/TRX/artifact: PENDING exact-head Actions. L4 router validation: NOT VERIFIED.
- Resulting commit: THIS COMMIT.
- Next: inspect all Xtables and XY-C1 jobs on the exact resulting SHA. If the compiler reveals another genuine blocker, fix that blocker narrowly and repeat; close only after kernel and userspace build, firmware image generation, non-empty TRX validation and artifact upload all succeed. Then reconcile any unrelated triggered workflows.

---

## 2026-11-12 18:41 CST (UTC+8) — Harden AliDDNS public-IP, signed URL and concurrent DNS lookup paths

- Production/test code SHA is `6786ad5c89aa4b871f501c72d89ee2c5f8d53be4`; the final semantic checker SHA after the exact fixture-label correction is `79b73e050bb00b6d9d2916d3955be9d1b8d95317`. The handoff / roadmap checkpoint refreshes are committed at `a786bd9edfe1440afb67c69e7fe86be7ac5c2703`, `df3acd6bf22ba64478dc94026cb2b8572fb71065`, `db42e82b784578b3a1fb3cfc1cd79a0a3ecfb4a9` and `b7a71a16734ddcef847802b01b3a9e203c365358`. Latest code/test/checker commits in this batch: DNS scratch isolation `95845208777085bc1e736f0c06f159182c336f2a`; DNS regression `cd560bf3f9ccfb497703d14f5f5d61f446cb55e5`; signal-time child reaping `d562108ca744bd73631e985ee1532ea90a552878`; source contract `54bd3e150e2e8faa2a4c15c21964fe24e903aa7c`; workflow coverage `eb403051ddbd6d451998185b1c4206fc4ab2d41f`; path-filter/test fixture follow-ups `999a0c89003456b80788671e28812685aafa226c`, `b396f635f0b4817b2977958b97c0f00007ebc88a`, `241752ae026fc674a10812f53ce4cc32ae111527`, `6786ad5c89aa4b871f501c72d89ee2c5f8d53be4` and final marker-contract commit `79b73e050bb00b6d9d2916d3955be9d1b8d95317`.
- Resumed the prior AliDDNS discovery/lifecycle handoff. The insecure public IPv4 discovery fallback has been replaced by the existing ROMFS helper `aliddns-ipv4.sh` using `https://api.ipify.org`, the firmware CA bundle, HTTPS-only/IPv4-only curl, bounded connect/total timeout, HTTP 200 and strict IPv4 validation. Stock `ddns_script.sh` uses this helper while retaining IPv6. `initconfig()` only installs a default persistent script when no non-empty `/etc/storage/ddns_script.sh` already exists, preserving user customization. The stock paths no longer use `curl -k`, `wget --no-check-certificate` or plaintext HTTP providers.
- Follow-up commit `5157b342e0584404d09b4ce334cd8506c5bb1e1f` had already moved the signed AliDNS URL (AccessKeyId + HMAC Signature) from curl argv to curl stdin with `--config -`; URL config metacharacters/control delimiters are rejected before serialization. The API HMAC canonicalization and URL parameters remain unchanged. Important remaining risk: AccessKeySecret still enters OpenSSL's `-hmac` argv and is NOT yet hidden from process listings.
- The same commit makes RC execute `copyscripts.sh` synchronously before asynchronous `autostart.sh`, removing the previous CA-copy race by source call order. The script's internal tar error propagation/readable CA postcondition and real boot timing remain open; `send_request()` fails closed if the CA file is absent.
- Fixed the local DNS lookup helper: `arNslookup()` / `arNslookup6()` use a common family-aware resolver. Atomic private directory allocation under `umask 077` avoids collisions when concurrent shell descendants share `$$`, without assuming BusyBox mktemp. The lookup is bounded, signals/reaps only its own resolver PID, rejects timeouts and nonzero exit even when a partial address is printed, validates every candidate as one strict IPv4/IPv6 value and removes private scratch data on normal exit and handled signals.
- Added `tests/aliddns_nslookup_lifecycle.sh` for valid/invalid v4/v6 candidates, empty response, resolver nonzero exit with partial output, timeout with partial output, child termination, unrelated resolver survival, same-$$ concurrent calls and scratch cleanup. The committed test script passed `sh -n` and the isolated host fixture run against extracted production functions, with output `AliDDNS bounded, isolated DNS lookup and strict IPv4/IPv6 lifecycle regression: PASS`. This was NOT a full repository checkout, and the complete Python semantic checker was not run locally.
- The AliDDNS workflow now runs the DNS lifecycle regression and shell syntax checks. Its push and pull_request path filters were restored to identical coverage for production source, helper, test, package, CA/bootstrap and RC/UI inputs. The source checker enforces private scratch, PID/timeout/reap/validation contracts, test markers and filter symmetry. Two checker false negatives (escaped private-path literal and case-sensitive "Concurrent calls share $$" fixture label) were corrected. Workflow and checker contents were inspected, but their execution on a full checkout has not been observed.
- At recovery, remote branch inventory contained only `main`; no checkpoint branch. On the final checker head `79b73e050bb00b6d9d2916d3955be9d1b8d95317`, classic combined status and PR-triggered run query were not yet independently re-fetched after the last checker-only commit and the available commit workflow-runs query returned no PR-triggered runs. The preceding exact-head query on `6786ad5c89aa4b871f501c72d89ee2c5f8d53be4` returned no classic statuses and no PR-triggered runs. That wrapper does not enumerate push-triggered runs; the Actions page was not retrievable through the available web interface. Therefore exact-head Actions / L2 for this batch remains NOT VERIFIED. Do not infer success from empty statuses.
- L1 source review and the isolated DNS host fixture are recorded above. L3 production MIPS/uClibc, full firmware/TRX and L4 hardware/router testing were not run. Target BusyBox/ash behavior and startup timing remain NOT VERIFIED.
- Open next: verify exact-head AliDDNS Actions and run the full repository semantic checker; then assess a method to keep AccessKeySecret out of OpenSSL argv without altering HMAC-SHA1 output, check that synchronous CA extraction actually leaves a readable bundle before autostart, and finish AliDDNS lifecycle concurrency. After AliDDNS, resume AliyunDrive concurrent start/stop, readiness and daemon-argv secret exposure. Preserve FTP upload-cleanup TOCTOU as OPEN/BLOCKED pending a proved Linux 4.4-safe design; NFS requires real-client evidence; then FRP/Adbyby.
- Root `dev_log.md` grows 57→58 complete records. It remains well below the 120-entry / 384 KiB early / 512 KiB hard limits; no archive rotation or `docs/dev-log/INDEX.md` update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.

---

## 2026-11-12 17:42 CST (UTC+8) — Close AliDDNS signed API transport security at L1/L2

- Exact source/test/checker main SHA: `c5d33928b7857021b3a1af28b95293ff546a0359`. The production change is commit `0ef6dd888d49e0302b0bcdd3c5a722a4c206f5b5`; the follow-up test-only commit `c5d33928b7857021b3a1af28b95293ff546a0359` drains both mocked OpenSSL pipeline inputs so the host harness does not leave producer writes with no reader.
- The signed `send_request()` previously sent AccessKeyId, timestamp/nonce and HMAC-SHA1 signature query data to plain HTTP. It now calls the official AliDNS HTTPS endpoint only, pins `/etc/ssl/certs/ca-certificates.crt` with `--cacert`, allows HTTPS only, refuses redirects, bounds connect/total time to 5/20 seconds and requires HTTP 2xx. Missing CA material fails before curl is invoked; certificate/HTTP failures propagate. Existing GET/HMAC-SHA1 canonicalization and parameter order are preserved.
- Verified the actual firmware trust-store route rather than assuming it from the curl build flag: `trunk/user/scripts/files/etc_ro/certs.tgz` contains `certs/ca-certificates.crt`; `/usr/bin/copyscripts.sh` extracts the archive into `/etc/ssl`. `rc.c` invokes that copy script and then autostart, both asynchronously. The curl package is built with OpenSSL and the same CA bundle path; the libcurl build enables the curl CLI whenever AliDDNS is enabled. The source checker verifies the archive member, copy/extract command, build flags and ROMFS/feature gates.
- Also fixed API error-state handling: failed record lookups are no longer treated as an empty record list that authorizes a duplicate AddDomainRecord; query/update/add results must contain a RequestId and no API Code before record IDs or success state are published. The existing three-attempt record discovery behavior and A/AAAA request selection remain.
- Traced the existing producer path: HTTPD NVRAM variables and persistent `scripts.ddns_script.sh` edits emit `EVM_RESTART_ALIDDNS`; PID 1 dispatches `RCN_RESTART_ALIDDNS`; the RC service wrapper calls stop/start. Boot setup invokes `copyscripts.sh` and `autostart.sh`; autostart starts AliDDNS only when enabled.
- Added a POSIX-shell fake-curl regression plus semantic checker and symmetric push/pull-request workflow. Host regression covered the HTTPS URL/flags and signed query shape, missing CA fail-closed behavior, simulated certificate failure, HTTP failure, redirect rejection, three failed lookups without record creation, API error on update, successful update publication and successful empty-record add. Exact-head Actions [43192612755] and [43192618164] both completed SUCCESS; every job step and full log inspected, and both checkout logs reported `c5d33928b7857021b3a1af28b95293ff546a0359`. PASS markers: shell syntax; HTTPS-only/CA/timeout/HTTP-status regression; API failure-propagation/publication regression; transport/firmware-trust-store/RC integration contracts; handoff docs validator.
- No complete local checkout or target device was used. These are L1 source and L2 host/Actions results only. The test mocks curl and does not verify a real AliDNS connection, target OpenSSL parsing, BusyBox/ash behavior, MIPS/uClibc linking, firmware/TRX or router boot behavior. The background `copyscripts.sh` versus `autostart.sh` ordering is source-traced but not runtime-proven; the client fails closed when CA is absent.
- Remaining AliDDNS risks: public-IP discovery fallbacks still include `curl -k`, `wget --no-check-certificate` and HTTP providers; the persistent user-overridable `/etc/storage/ddns_script.sh` must be audited separately. AccessKeyId and the short-lived HMAC Signature remain in the curl URL argv (the AccessKeySecret is not in that URL). Concurrent AliDDNS lifecycle/start/stop behavior is not closed. Do not describe the complete DDNS feature as secure.
- Existing separate open items remain: AliyunDrive refresh-token/auth-password argv exposure and concurrent-start/readiness gaps; FTP failed-upload cleanup TOCTOU P2 OPEN/BLOCKED; NFS async durability/AUTH_SYS/root-squash real-client evidence; FRP/Adbyby follow-up.
- Root `dev_log.md` grows 56→57 records, below the 120/119 rotation threshold, 384 KiB target and 512 KiB hard limit. Archive shards and `docs/dev-log/INDEX.md` remain unchanged (dev-log=7, handoff=22, roadmap=18).
- Changed source/test paths in the implementation batch: `trunk/user/aliddns/aliddns.sh`, `trunk/user/aliddns/tests/aliddns_api_transport.sh`, `scripts/check_aliddns_api_transport.py`, `.github/workflows/aliddns-api-transport.yml`, `trunk/libs/Makefile`, `trunk/libs/libcurl/Makefile`; this governance update aligns the live handoff and whole-tree dashboard.
- Next: audit all stock and user-configurable AliDDNS external-IP discovery paths and their TLS/provider behavior; test IPv4/IPv6 result validation and bounded failure/retry. In parallel trace the CA-install/autostart race and AliDDNS concurrent start/stop. Preserve the closed signed API path. Do not re-open AliDDNS API transport without new evidence.

---

## 2026-11-12 16:30 CST (UTC+8) — Close AliyunDrive WebDAV bounded stop and argv integrity at L1/L2

- Source/test/checker base SHA: `d8dbb93083f616be8ecfdb84abfb3f104436960d`; the code change was introduced by `bf915c092d4c5478e5a99ed00f796dd78c58756e`, then the sourced-shell test harness was made nounset-safe in `d8dbb93083f616be8ecfdb84abfb3f104436960d`.
- Audited `trunk/user/aliyundrive-webdav/aliyundrive-webdav.sh` and its RC/UI/ROMFS wiring. The previous `kill_ald()` cached the numeric PID from `pidof`, used `killall` for TERM and then sent SIGKILL to the cached numeric PID immediately. That could target a reused PID after the original process exited and did not prove the daemon had stopped. `start_ald()` also launched duplicates on repeated notifications and expanded NVRAM values unquoted.
- Replaced shutdown with a bounded five-second TERM grace, a fresh process-name-based `killall -9` escalation and a final process-absence postcondition; persistent processes now return failure. Removed the cached numeric PID path. Start now no-ops when the daemon is already observable and constructs options using positional parameters so host/root/token/user/password/cache values are each passed as one quoted argv item. Preserved the special domain-id 99999 option policy and the existing RC/ROMFS integration.
- Added POSIX-shell host regression coverage for no-process stop, graceful TERM, TERM timeout followed by name-based KILL, a process surviving SIGKILL, sequential duplicate-start suppression, literal argv values with spaces/wildcards/metacharacters, and the special domain profile. Added semantic source/RC/package/HTTPD producer and symmetric workflow path-filter checks.
- Exact-head Actions on `d8dbb93083f616be8ecfdb84abfb3f104436960d`: AliyunDrive lifecycle [43182561404] and AI handoff docs [43182561374] both completed SUCCESS. Every step and full job log inspected; checkout `git log -1` matched the exact SHA. Shell syntax, complete host regression and semantic integration contract all PASS. Docs validator reported root log=55/119 before this governance entry and archive inventory dev-log=7, handoff=22, roadmap=18.
- No full local repository checkout, production toolchain, target BusyBox or router was available. This closes only L1/L2 host/Actions behavior; simultaneous start races, daemon readiness and refresh-token/auth-password visibility in process argv remain open. MIPS/uClibc, firmware/TRX and L3/L4 remain NOT VERIFIED.
- Changed paths: `trunk/user/aliyundrive-webdav/aliyundrive-webdav.sh`, new `trunk/user/aliyundrive-webdav/tests/aliyundrive_webdav_lifecycle.sh`, new `scripts/check_aliyundrive_webdav_lifecycle.py`, new `.github/workflows/aliyundrive-webdav-lifecycle.yml`; this handoff/roadmap/log update reconciles the active checkpoint and next module.
- Root `dev_log.md` grows 55→56 complete records, below 120/119 rotation and 384 KiB early target; no archive or INDEX update is due.
- Next: rotate to AliDDNS. Audit `send_request()` in `trunk/user/aliddns/aliddns.sh`: HMAC-signed API parameters currently travel to `http://alidns.aliyuncs.com`. Confirm HTTPS and target CA-verification support, test certificate failure/downgrade handling, and do not use `-k`. Keep AliyunDrive argv-secret/concurrent-start gaps, FTP TOCTOU and NFS client-evidence blockers visible.

---

## 2026-11-12 15:22 CST (UTC+8) — Close AdGuardHome resolver transaction at L1/L2

- Exact main SHA: `14d05006beaa930d4870db870b0f7fbc7ff9fb54`; remote branch inventory contains only `main`, no checkpoint branch.
- Exact-head Actions all SUCCESS: AdGuardHome [43173914321], infosvr [43173914235], AI handoff docs [43173914225]. Every job's steps and full logs were inspected; each checkout's `git log -1` matched the exact SHA.
- AdGuard workflow passed launcher `sh -n`, all host regressions and semantic source contracts. Output markers: secret config mode/symlink/atomic publication PASS; dnsmasq resolver publication/status/byte-exact rollback PASS; IPv4/IPv6 redirect/dedup/rollback PASS; duplicate-start/bounded-stop PASS; config privacy/bounded firewall cleanup/RC integration contracts PASS. The resolver transaction's earlier host fixture and stale source-checker marker errors were corrected in their follow-up commits.
- infosvr GNU89 packet/signal regressions and source contracts passed. AI docs validator passed with root log=54/119 and archives dev-log=7, handoff=22, roadmap=18.
- AdGuardHome resolver enable/disable now uses same-directory staged publication, saves an exact backup, checks dnsmasq reload status, restores and retries the previous file on failure, and propagates errors through start/stop gates. This closes resolver mutation L1/L2 at this exact host/Actions SHA; it does NOT prove target BusyBox, router dnsmasq/firewall runtime, MIPS/uClibc or hardware behavior.
- Remaining AdGuard audit: concurrent RC/UI start/stop/restart calls remain unserialized. RC wrapper and `RCN_RESTART_ADGUARDHOME` dispatch have been traced, but all UI producers/notifications and a BusyBox ash-compatible lock/stale-owner strategy still need source tracing and proof.
- Root `dev_log.md` grows 54→55 complete entries; below the 120-record / 384 KiB rotation thresholds and 512 KiB hard limit. No archive or `docs/dev-log/INDEX.md` update is due; archive inventory remains 7/22/18.
- Next: identify every UI and RC producer of AdGuardHome lifecycle operations, inspect available target lock primitives/build config, then implement bounded cross-process serialization with contention and stale-lock regressions. Preserve failed-state information and avoid unsafe stale-lock reclamation races.

---

## 2026-11-12 15:15 CST (UTC+8) — Align AdGuardHome source contract with reordered stop cleanup

- Base main SHA: `5f7b186b4fdde7af688762463c72bc7699f673d8`. Exact-head checks: AdGuard [43173678041] shell syntax and the complete host regression PASSED, including dnsmasq enable/disable, exact-byte rollback and start/stop fail-closed tests. Its semantic source contract then FAILED only because the checker still required the old stop-failure fixture message after the test assertion was deliberately renamed for the new cleanup order. AI docs [43173678214] and infosvr [43173678040] both SUCCESS; full jobs/logs inspected.
- Updated `scripts/check_adguardhome_config.py` to assert the current regression message. No production source or behavioral test changed in this correction. The AdGuard workflow rerun on the resulting exact SHA is pending; do not close resolver L2 until its semantic checker passes alongside the host suite and docs contract.
- Root `dev_log.md` grows 53→54 entries; below rotation thresholds. Archive inventory and `docs/dev-log/INDEX.md` remain unchanged.
- Next: fetch exact-head workflow runs and inspect all logs; then proceed to BusyBox ash-compatible serialization of concurrent AdGuardHome start/stop/restart callers only after resolver L2 closes.

---

## 2026-11-12 15:13 CST (UTC+8) — Correct AdGuardHome resolver regression printf fixtures

- Base main SHA: `923363a8e4c630e7f890c3f307561b71b1ae6b33`; this is a test-fixture correction to the resolver transaction batch committed immediately before it.
- Source review of the new host cases found that three expected-output `printf` formats contained two literal backslashes before `n` instead of the single escape required for newline comparison. This would compare against backslash characters rather than the multi-line mock log. Corrected the expected formats and added a no-op logger mock to keep the resolver regression hermetic on CI hosts.
- No production source behavior changed in this correction. The exact-byte rollback cases, start firewall rollback/no-launch case and stop fail-closed case remain unchanged; the full AdGuardHome workflow must validate the corrected fixture on one exact SHA.
- No full local checkout is available here; local shell/regression/docs checks were not run. Exact-head AdGuardHome and AI docs Actions remain pending.
- Root `dev_log.md` grows 52→53 entries; well below rotation thresholds. Archive inventory and `docs/dev-log/INDEX.md` remain unchanged.
- Next: inspect the corrected exact-head AdGuardHome regression and AI docs jobs completely; fix any further genuine failures before closing resolver L2. The next implementation after L2 remains cross-process AdGuardHome start/stop serialization across RC/UI callers.

---

## 2026-11-12 15:08 CST (UTC+8) — Make AdGuardHome resolver updates transactional

- Base main SHA: `31fbec7bf87bf2d8649003f00024ac218de4af59`; remote branches at recovery contained only `main`, with no checkpoint branch.
- First completed the handoff's pending firewall cleanup/error-gating validation. On exact SHA `31fbec7bf87bf2d8649003f00024ac218de4af59`, AdGuard [43164341684], infosvr [43164340736] and AI docs [43164340719] all SUCCESS. Full job steps/logs inspected; shell regression markers for config privacy, dual-stack redirect cleanup/rollback, sequential lifecycle and bounded/error-aware firewall cleanup all PASS; docs validator reports 51/119 and archive inventory 7/22/18.
- Continued the AdGuardHome resolver audit. Previous `change_dns()` edited the live dnsmasq config in place through unchecked `sed`/append commands, ignored `restart_dhcpd` status and allowed daemon startup after resolver mutation/reload failure. `del_dns()` likewise ignored edit/reload failures; stop deleted its workdir without knowing resolver removal had succeeded.
- THIS COMMIT stages the dnsmasq file beside the live config, preserves a byte-for-byte rollback copy, checks copy/edit/append/rename/reload results, and restores/reloads the original file if the new resolver config is rejected. `change_dns()` and `del_dns()` now propagate failure. Startup removes newly published firewall redirects and aborts before daemon launch on resolver failure. Stop does not stop the daemon or remove its workdir until firewall cleanup and resolver removal/reload succeed.
- Added host regression cases for successful enable/disable, exact previous-byte restoration after failed enable and disable reloads, retry reload attempts, startup firewall rollback/no launch, and stop preserving the daemon/workdir on resolver failure. Semantic source contracts cover staged publication, reload/rollback ordering and lifecycle error gates.
- L1 source review completed. No local full checkout was available, so local `sh -n`, host regression and `scripts/check_ai_handoff_docs.py` were NOT RUN. Exact-head AdGuardHome and AI docs Actions for THIS COMMIT are PENDING; do not claim L2 closure until both are inspected on the resulting SHA.
- Changed paths: `trunk/user/adguardhome/adguardhome.sh`, `trunk/user/adguardhome/tests/adguardhome_config_security.sh`, `scripts/check_adguardhome_config.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root `dev_log.md`: 51→52 complete records; below 120-record / 384 KiB triggers and 512 KiB hard limit. No archive or INDEX update due; archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Remaining: simultaneous RC/UI start/stop callers are not serialized; the sequential `pidof` guard is not a concurrency lock. Next inspect exact-head Actions, then trace all AdGuardHome caller/notification paths and implement/test BusyBox ash-compatible inter-process serialization. After that rotate to AliyunDrive WebDAV, FRP, AliDDNS or Adbyby. FTP failed-upload TOCTOU remains OPEN/BLOCKED; NFS remains real-client-evidence gated. MIPS/uClibc, firmware/TRX and L3/L4 remain NOT VERIFIED.

---
## 2026-11-12 13:38 CST (UTC+8) — Correct AdGuardHome bounded-cleanup regression fixture

- Base main SHA: `10a65f67d4e78948eb0a1a283088a24d21dfbf34`; parent `e023a2c0497bd386f0964f329462ddb878f24fde`; remote branch inventory contains only `main`, with no checkpoint branch.
- Exact-head Actions on base SHA `10a65f67d4e78948eb0a1a283088a24d21dfbf34`: AdGuardHome [43162860458] FAILED at the bounded duplicate-cleanup assertion; AI handoff docs [43162860481] and infosvr [43162860639] both SUCCESS. Full job steps/logs inspected. Shell syntax and the secret-config regression passed before the AdGuard test stopped.
- Root cause was a test-fixture off-by-one, not the production cleanup loop: the fixture already contained the initially installed IPv4 TCP rule, then appended 65 additional copies, creating 66 total. Correct 64-delete behavior therefore left two copies, while the assertion expected one.
- THIS COMMIT corrects the fixture to append 64 copies to the existing one (65 total), retaining the assertion that the bounded pass reports excess state and leaves exactly one rule for the next retry. Production shell code is unchanged in this correction.
- The host test was not run from a full repository checkout in this environment. A focused shell reproduction of the 65-total / 64-delete / retry arithmetic passed; this is not a substitute for the repository regression or exact-head Actions.
- Exact-head AdGuardHome and AI handoff-doc workflows after this fixture correction are pending. Do not close the firewall-cleanup L1/L2 slice until the full regression, semantic source contract and docs validator pass on one exact SHA.
- Root `dev_log.md` grows 50→51 complete records; below the 120-record / 384 KiB rotation triggers and 512 KiB hard limit. No archive or INDEX update is due; archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next: inspect all exact-head Actions for the corrected commit, including every step/full log. If green, close bounded/error-aware firewall cleanup at L1/L2; then resume concurrent AdGuardHome start/stop serialization and transactional `change_dns()` / `del_dns()` rollback. MIPS/uClibc, complete firmware/TRX and L3/L4 remain NOT VERIFIED.

---

## 2026-11-12 13:26 CST (UTC+8) — Fail closed on AdGuardHome firewall cleanup errors

- Base main SHA: `e023a2c0497bd386f0964f329462ddb878f24fde`; remote branch inventory contains only `main`. This batch resumes the explicit AdGuardHome follow-up after the address-independent `addrtype LOCAL` redirect slice closed on `9a29d68d96f6546e18a2953633ab7e506d13cdaa`.
- Continued the AdGuardHome lifecycle audit. The redirect cleanup loop treated every nonzero `iptables -C` result as “no rule”, although xtables also returns errors for lock contention, unavailable tables or invalid operations. The loop was unbounded under concurrent rule insertion. The launcher also ignored `set_iptable()` failure and proceeded with DNS/daemon startup; stop removed the runtime directory and resolver state before confirming firewall cleanup. IPv6 table probing similarly treated every command failure as an IPv6-disabled profile.
- Bounded each exact duplicate-removal pass to 64 deletions; status 1 means absent, status 0 means delete, and other statuses now fail. A 65th remaining duplicate is reported as cleanup failure so a later retry can resume. Distinguish an absent `ip6tables` binary from a present binary whose NAT-table probe fails. Startup now requires successful firewall preparation before changing resolver state or launching; stop requires successful firewall cleanup before removing workdir/resolver state.
- Extended the POSIX-shell mock regression for 65 duplicates and retry, injected `iptables -C` failure, injected IPv6 NAT-list failure during startup with IPv4 rollback, injected IPv6 cleanup-probe failure, start fail-closed behavior and stop cleanup failure retention. Expanded semantic source contracts for bounded cleanup, return-code discrimination, IPv6 command/probe handling and start/stop ordering.
- Exact-head GitHub Actions for THIS source/test/checker batch are pending; do not claim L1/L2 closure until the exact resulting SHA has both AdGuardHome and AI handoff workflows green and every job step/log inspected. No local full checkout or direct target execution was available in this batch.
- Open items: concurrent start/stop callers remain unserialized; `change_dns()` / `del_dns()` command status propagation and DNS resolver rollback are not fixed; daemon launch/readiness remains unverified. Production BusyBox ash, MIPS/uClibc, full firmware/TRX and L3/L4 router behavior remain NOT VERIFIED.
- Root `dev_log.md` grows 49→50 complete records; below the 120-record / 384 KiB rotation triggers. No archive or INDEX update is due; archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next: inspect exact-head Actions for this batch, then design bounded serialization for all RC/UI AdGuardHome start/stop callers and implement a safe resolver restore/command-error transaction. After the AdGuardHome lifecycle is covered, rotate to AliyunDrive WebDAV, FRP, AliDDNS or Adbyby. Keep FTP upload-cleanup TOCTOU OPEN/BLOCKED pending Linux 4.4-safe concurrency proof; NFS remains real-client-evidence gated.

---

## 2026-11-12 13:17 CST (UTC+8) — Make AdGuardHome DNS redirects address-change safe

- Resumed from live main SHA `16ce845f116f271f4c60942a4cd0a272efe81a10`; the remote branch inventory contained only `main`, with no checkpoint branch.
- Continued the explicit AdGuardHome firewall/resolver audit. The previous redirect implementation enumerated interface IPv4/IPv6 addresses and generated destination-specific PREROUTING rules; `clear_iptable()` also reconstructed deletions from only the current address set. DHCP/PPPoE address changes could therefore leave old rules behind, and repeated starts could accumulate duplicates.
- Replaced the address snapshots with route-derived `-m addrtype --dst-type LOCAL` rules for IPv4 and IPv6 TCP/UDP DNS redirects, excluding IPv4 loopback and IPv6 loopback/link-local ranges. Cleanup checks and deletes every exact copy before a new publication; mode-off removes the complete current rule set. A partial IPv4/IPv6 append failure returns nonzero and rolls back the rules already installed.
- Extended the isolated POSIX-shell regression with a mocked iptables/ip6tables state model: exact v4/v6 rules, duplicate removal after a simulated address change, mode-disable cleanup, idempotent repeated cleanup, and injected IPv6 UDP append failure with cross-family rollback. Added semantic integration checks for Linux 4.4 addrtype IPv6 support, Kconfig/Makefile, iptables 1.8.7 userspace and package install, plus required IPv4/IPv6 match support in every per-board kernel config. Expanded both workflow path filters symmetrically.
- Two intermediate exact-head Actions runs exposed test-harness/checker defects, not production-code failures: the startup fail-closed test's global `set_iptable()` mock shadowed the real firewall function, and the semantic checker expected a same-line helper signature while the shell function uses a newline before its opening brace. The mock is now subshell-scoped and the checker matches the actual function boundary.
- Final exact-head Actions on SHA `9a29d68d96f6546e18a2953633ab7e506d13cdaa`: AdGuardHome [43160363720] and AI handoff validator [43160363695] both SUCCESS; every step and complete job log inspected. Host regression output confirms config-secret mode/symlink/atomic publication PASS, address-independent v4/v6 redirect/dedup/rollback PASS, and duplicate-start/bounded-stop PASS. Semantic config/privacy, firewall, kernel/iptables, all-board, RC/ROMFS/package and workflow path contracts PASS. Docs validator on this pre-governance source checkpoint reports root log=48/119 and archive inventory dev-log=7, handoff=22, roadmap=18.
- This closes only the bounded address-change-safe redirect slice at L1/L2. It does not serialize concurrent start/stop callers. `start_adg()` still ignores nonzero returns from `change_dns()` and `set_iptable()`; resolver rollback and cleanup command error propagation remain open. No full local checkout, MIPS/uClibc build, target BusyBox run, firmware/TRX build or L3/L4 router verification is claimed.
- Root `dev_log.md` grows 48→49 complete records; below the 120-record / 384 KiB rotation triggers. No archive or INDEX update is due; archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next: continue AdGuardHome with concurrent start/stop serialization and fail-closed propagation/rollback for DNS and firewall setup/cleanup. Trace RC/UI callers and add bounded host tests; then rotate to AliyunDrive WebDAV, FRP, AliDDNS or Adbyby. Keep FTP upload-cleanup TOCTOU OPEN/BLOCKED pending Linux 4.4-safe concurrency proof; NFS remains real-client-evidence gated.

---

## 2026-11-12 12:31 CST (UTC+8) — Bound AdGuardHome shutdown and prevent sequential duplicate starts

- Base main SHA: `524147c15b9847d4f42186ec444a011f1a8ca47a`; remote branch inventory contains only `main`.
- Continued the AdGuardHome caller-path audit. `start_adg()` previously relaunched the daemon on every repeated start request; `stop_adg()` removed `/tmp/AdGuardHome` before immediately sending SIGKILL and then always removed DNS/firewall redirection without proving the daemon had exited.
- Added a process-name preflight so a sequential duplicate start is a successful no-op before config/DNS/firewall/launch side effects. Added bounded five-second TERM polling; if still present, re-enumerated by process name with SIGKILL and required a second bounded absence postcondition. Failed stop now returns nonzero and retains workdir, resolver and firewall state. Cleanup only follows confirmed process absence.
- Extended the isolated POSIX-shell regression with duplicate-start no-op, no-process cleanup, graceful TERM, TERM-resistant escalation and permanently-stuck daemon cases. The test verifies exactly five TERM polls, ten total polls on persistent failure, process-name-only signaling and fail-closed cleanup. Extended semantic source contracts for caller order and cleanup gates.
- Exact-head Actions on SHA `33c1ecaa1d85aa501f51ac0b1cce9c17d7d0a6cf`: AdGuardHome [43155259286] and AI docs [43155259234] both SUCCESS. Every job step and full log inspected. Shell syntax, prior config security regression, new duplicate-start/bounded-stop regression and all config/ROMFS/package/RC/path-filter source contracts printed PASS. Docs validator reports root log=47/119 and archive inventory dev-log=7, handoff=22, roadmap=18.
- Root `dev_log.md` grows 47→48 complete records; below 120-entry/384 KiB rotation triggers. No archive or INDEX update is due.
- Remaining AdGuardHome work is not closed: starts/stops from concurrent callers are not serialized; IPv4/IPv6 PREROUTING rules are reconstructed from current interface addresses, so address changes and repeated start/cleanup may leave stale or duplicate rules; DNS resolver restoration and cleanup command errors still need audit. Target BusyBox/MIPS behavior, production MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.
- Next: inspect the complete AdGuardHome firewall/resolver call graph and choose one reproducible stale-rule or rollback defect; add focused host model and exact caller/path contracts. Keep FTP failed-upload TOCTOU OPEN/BLOCKED and NFS real-client-gated; after AdGuardHome rotate to AliyunDrive WebDAV, FRP, AliDDNS or Adbyby.

---

## 2026-11-12 12:01 CST (UTC+8) — Make AdGuardHome function-boundary scan newline-safe

- Base main SHA: `c83fd3a66ed854ec4d1d5f6a304a561b10bddfe3`.
- Exact-head AdGuardHome workflow [43153476546] checked out `e0f1cc0969287f113f20e74151138b68b4ce4acc`. Launcher syntax and the complete host config regression passed. The source-contract step failed because the first non-regex parser searched for a literal backslash-n sequence instead of a real newline; no production or host regression failure occurred.
- The checker parser is now based on `source.find(chr(10) + "}", opening + 1)`, avoiding both regex-brace quoting and newline escape ambiguity. Dedicated workflow path filters now also include the three canonical governance documents in both push and pull_request, so the final docs checkpoint is tested on its own head.
- On the same parent SHA, AI handoff docs [43153476616] and infosvr [43153476521] both SUCCESS; docs validator reported root log=46/119 and archives dev-log=7, handoff=22, roadmap=18.
- Root `dev_log.md` grows 46→47 complete records; remains below rotation triggers. No archive or INDEX update due; archives remain dev-log=7, handoff=22, roadmap=18.
- The later corrected exact-head reruns passed on SHA `524147c15b9847d4f42186ec444a011f1a8ca47a`: AdGuardHome [43154187156], AI docs [43154187224] and infosvr [43154187170] all SUCCESS; docs validator reported 47/119, archives 7/22/18. The complete AdGuardHome step list and full log were inspected; config regression, syntax and semantic RC/ROMFS/source contracts all printed PASS. No production target/MIPS/uClibc/firmware/TRX/L4 claim.
- Next: inspect exact-head AdGuardHome and AI docs jobs, every step and full logs; if green, close the config confidentiality slice and resume remaining AdGuardHome process lifecycle, concurrent starts, resolver rollback and stale firewall-rule cleanup. FTP TOCTOU remains OPEN/BLOCKED; NFS is real-client evidence-gated.

---

## 2026-11-12 11:53 CST (UTC+8) — Correct AdGuardHome source-contract function boundary parser

- Base main SHA: `67c183b0a2464bc5f3d943917c98c8435749dbc8`.
- Exact-head AdGuardHome workflow [43153199422] checked out this SHA. Shell syntax and the complete secret-config host regression passed, including mode 0600, existing-file permission repair, symlink/FIFO rejection, failed rename/chmod cleanup and startup fail-closed ordering.
- The semantic source-contract step failed before checking integrations because its Python function-boundary regex was over-escaped and failed to locate the existing `getconfig(){` signature. This is a checker parser defect; no production shell or host regression failure occurred.
- Exact-head AI handoff docs [43153199230] and infosvr packet/signal [43153199055] both SUCCESS. Every step/full log inspected. Docs validator reported root log=45/119 and archives dev-log=7, handoff=22, roadmap=18.
- THIS COMMIT replaces the brittle escaped regex with explicit signature/opening-brace/column-zero closing-brace boundary extraction. No production code or host regression changed.
- Corrected AdGuardHome and AI handoff-doc Actions on THIS COMMIT are pending; do not close L2 until the new semantic contract and docs validator pass on this exact SHA.
- Root `dev_log.md` grows 45→46 complete records; below rotation thresholds. No archive or INDEX update is due; archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next: inspect exact-head corrected AdGuardHome and AI docs Actions, every step and complete log. Require config regression, source/caller/ROMFS checks, exact-once symmetric filters and docs root log=46/119, archives=7/22/18. Then continue AdGuardHome bounded stop, duplicate/concurrent start, DNS rollback and address-change cleanup audit. FTP TOCTOU remains OPEN/BLOCKED; NFS needs real-client evidence; MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 11:49 CST (UTC+8) — Protect AdGuardHome configuration secrets with private atomic publication

- Base main SHA: `f8c154c5443514df45c01d8d74ffcb21366b7faf`; remote branch inventory contains only `main`, with no checkpoint branch.
- Resumed the explicit whole-package AdGuardHome audit from SESSION_HANDOFF. Traced package selection in `trunk/user/Makefile`, package ROMFS install, the full `adguardhome.sh` config/start/stop path, RC service wrappers, stop_services integration and `RCN_RESTART_ADGUARDHOME` dispatch.
- Found a concrete confidentiality defect: the generated `/etc/storage/adg.sh` contains the AdGuardHome UI `auth_pass` and a TLS `private_key` field, but `getconfig()` created it with mode 0755. Existing operator config files were not tightened on later starts.
- THIS COMMIT rejects symlink and non-regular config destinations; writes missing/empty defaults to a unique same-directory temporary under `umask 077`; checks staged chmod and atomic rename; cleans failed stages; and enforces mode 0600 for both generated and existing configs. Startup now stops before DNS resolver or iptables side effects when config preparation/protection fails.
- Added a host regression for generated mode under umask 000, preservation/tightening of existing config, symlink/FIFO rejection, rename/chmod failure cleanup and startup fail-closed ordering. Added a semantic source/ROMFS/package/RC caller contract and symmetric push/pull_request workflow. Exact-head AdGuardHome and AI handoff-doc Actions are pending.
- Root `dev_log.md` grows 44→45 complete records; below the 120-entry/384 KiB rotation triggers. No archive or INDEX change is due; archive inventory remains dev-log=7, handoff=22, roadmap=18.
- This is host/source-level confidentiality and publication work only. Production MIPS/uClibc, target BusyBox, live AdGuardHome process/DNS/firewall behavior, complete firmware/TRX and L4 remain NOT VERIFIED.
- Next: inspect every step and complete log for exact-head AdGuardHome config-security and AI docs Actions; require all config tests, semantic callers, exact-once symmetric filters and docs validator (45/119; archives 7/22/18) to pass. Then continue the separate lifecycle audit: bounded stop, duplicate/concurrent starts, resolver rollback and address-change-safe IPv4/IPv6 PREROUTING cleanup. FTP TOCTOU remains OPEN/BLOCKED; NFS requires real-client evidence.

---

## 2026-11-12 11:31 CST (UTC+8) — Close ZeroTier readiness lifecycle at exact-head L2

- Base code/test SHA: `3f435a03995afb1130da7d3acfecf65c8531ee9f`; parent `fdf89096d631245efab7534cd399abeb498a0acd`.
- Exact-head ZeroTier lifecycle [43148551947], AI handoff docs [43148552085] and infosvr packet/signal [43148551957] all completed SUCCESS. Every job step and complete log was inspected; all three checkouts matched the exact code/test SHA.
- ZeroTier regression output: `ZeroTier bounded stop/start, readiness and persistent-config preservation: PASS`. Semantic checker output: `ZeroTier bounded lifecycle, persistent-config and RC integration contracts: PASS`. The earlier `fdf89096...` checker false negative was corrected; no production-code regression was involved. Docs validator passed at root log=43/119 with archives dev-log=7, handoff=22, roadmap=18.
- This closes the bounded stop/start and interface/NAT-route readiness slices at L1/L2. L3 production MIPS/uClibc, target BusyBox behavior, live router networking/firewall behavior, complete firmware/TRX and L4 remain NOT VERIFIED.
- Root `dev_log.md` grows 43→44 complete records, below the rotation threshold; no archive or INDEX change is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Resulting documentation checkpoint: THIS COMMIT.
- Next: start whole-tree add-on lifecycle audit at `trunk/user/adguardhome/`, trace complete package/RC callers and process/config/resource lifecycle, select one reproducible stability defect, and add focused host/source-contract coverage. Then rotate to AliyunDrive WebDAV, FRP, AliDDNS or Adbyby. FTP TOCTOU remains OPEN/BLOCKED; NFS needs real-client evidence.

---

## 2026-11-12 11:26 CST (UTC+8) — Correct ZeroTier readiness source-contract assertions

- Base main SHA: `fdf89096d631245efab7534cd399abeb498a0acd`; parent `1bf8a93bfa47770e8005d5241f7077287c86c4c4`.
- Exact-head ZeroTier Actions [43148056612] ran the expanded lifecycle shell regression successfully: delayed interface/route discovery, both 30-second timeout cases, route-interface filtering, normal single-space route text, invalid NAT config rejection and no partial firewall publication all passed. The subsequent semantic checker failed at the readiness-wait source assertion.
- Root cause was confined to the checker: the production shell correctly compares the quoted variable `"$zerotier_wait"`, while the checker searched for an unquoted substring. Review also found the start-cleanup assertion could choose a prior configuration-generation `return 1`; THIS COMMIT scopes it to the return after the readiness-failure cleanup branch.
- Exact-head AI handoff docs [43148056614] SUCCESS, validator root log=42/119 and archives=7/22/18. Infosvr packet/signal [43148056670] SUCCESS; both steps and complete logs inspected. No production ZeroTier code or regression changes in this correction.
- Exact-head ZeroTier and AI docs reruns on THIS COMMIT are pending; do not close readiness L2 until the corrected checker and docs validator pass on one exact SHA.
- Root `dev_log.md` grows 42→43 complete records, below rotation thresholds; no archive or INDEX change. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Production MIPS/uClibc, target BusyBox behavior, live route/interface readiness, complete firmware/TRX and L4 remain NOT VERIFIED.
- Resulting commit: THIS COMMIT.
- Next: inspect all steps/full logs for exact-head ZeroTier, AI docs and all other triggered Actions. Require readiness host regression, corrected source contract and docs root log=43/119 with archive counts 7/22/18. Then rotate to AdGuardHome/AliyunDrive WebDAV/FRP/AliDDNS/Adbyby. FTP TOCTOU remains OPEN/BLOCKED; NFS needs real-client evidence.

---

## 2026-11-12 11:21 CST (UTC+8) — Bound ZeroTier interface and NAT-route readiness

- Base main SHA: `1bf8a93bfa47770e8005d5241f7077287c86c4c4`; parent `f39638ac28bae38e6ec8d2694831685b3be98d4e`.
- Closed the preceding ZeroTier stop/start task on the exact base SHA: ZeroTier lifecycle [43143884566] and AI handoff docs [43143884420] both SUCCESS. Every job step and complete log was inspected; bounded TERM/KILL, stale-numeric-PID exclusion, start gating, persistent-identity preservation, RC/ROMFS integration and docs validation all passed (root log=41/119; archives=7/22/18).
- Continued the explicitly recorded ZeroTier `rules()` follow-up. Found unbounded waits for the ZeroTier interface and (when NAT is enabled) its kernel route. The route grep demanded two spaces between `dev` and `proto`, while ordinary iproute2 output uses one; a missing route or whitespace mismatch could block RC startup forever. Firewall rules were also partially published before route readiness, and the NAT flag used an unquoted numeric test on potentially empty NVRAM.
- THIS COMMIT bounds interface and route waits independently to 30 seconds, re-checks the readiness condition at the deadline, matches `zt*` interface names and exact `dev <if> proto kernel` route tokens, validates NAT state, and postpones firewall changes until every required prerequisite is ready. `start_instance()` now attempts daemon cleanup and returns failure if readiness expires. `del_rules()` shares the same token-based route parser.
- Extended the existing host shell regression with delayed interface/route appearance, no-interface and no-route timeout cases, unrelated interface route filtering, ordinary single-space route output and no-partial-firewall assertions. Extended the existing semantic source checker. Workflow path filters already covered all changed files.
- Local full checkout is unavailable because container DNS cannot resolve github.com; the updated shell regression, source checker and docs validator were NOT RUN locally. Exact-head ZeroTier and AI-doc Actions are pending.
- Root `dev_log.md` grows 41→42 complete records, below the 120-entry/384 KiB rotation triggers and 512 KiB hard cap; no archive or INDEX change is due. Archive counts remain dev-log=7, handoff=22, roadmap=18.
- Production MIPS/uClibc, target BusyBox `ifconfig`/`ip route` behavior, live interface/route readiness, complete firmware/TRX and L4 router behavior remain NOT VERIFIED.
- Resulting commit: THIS COMMIT.
- Next: inspect every step and full log for exact-head ZeroTier lifecycle and AI handoff-doc Actions; require the new readiness regressions, source/RC/ROMFS contracts and docs validator (42/119; archives 7/22/18) to pass. Then rotate to another optional user-space service wrapper instead of repeatedly polishing ZeroTier. FTP TOCTOU remains OPEN/BLOCKED; NFS needs real-client evidence.

---

## 2026-11-12 10:35 CST (UTC+8) — Gate ZeroTier cleanup on confirmed daemon shutdown

- Base main SHA: `f39638ac28bae38e6ec8d2694831685b3be98d4e`; parent `55c88ad2e93874d7fe53520e028b749742a6d144`.
- Resumed the active whole-tree RC/add-on lifecycle audit after closing DDNSTO on the exact base SHA. DDNSTO [43140910335] and AI handoff docs [43140910294] both completed SUCCESS on `f39638ac28bae38e6ec8d2694831685b3be98d4e`; all steps and full logs were inspected. The five-case DDNSTO lifecycle regression and RC/ROMFS source contract passed; docs validator reported root log=40/119 and archives=7/22/18.
- Traced ZeroTier notification `RCN_RESTART_ZEROTIER` through `restart_zerotier()`, the service wrappers and ROMFS-installed `/usr/bin/zerotier.sh`. Found that `kill_z()` cached `pidof zerotier-one` numeric PIDs, sent TERM by name, then SIGKILL to the stale numeric snapshot. There was no bounded absence postcondition and `start_zero()` ignored the stop result.
- Found a destructive stop-failure path: `stop_zero()` removed firewall rules and routes before checking daemon termination, then unconditionally recursively deleted `/etc/storage/zerotier-one`. A surviving daemon could be left running without its network state while its persistent identity/configuration was destroyed.
- THIS COMMIT adds a five-second live-process TERM wait, fresh process-name SIGKILL escalation and a second bounded absence postcondition. Replacement startup now fails closed when old-daemon absence is unconfirmed. Stop now gates firewall/routing cleanup and persistent config removal on confirmed daemon absence.
- Added a shell regression for no-process, graceful TERM, TERM-resistant/KILL, still-running daemon, start gating, stop-failure identity/config preservation and successful cleanup ordering. Added semantic RC/ROMFS/source and exact-once symmetric workflow path-filter checks. The ZeroTier and AI-doc workflows must pass on this exact resulting SHA before L2 closure.
- The container cannot clone GitHub because DNS resolution for github.com failed. No local full checkout, local test execution or repository-wide docs check is claimed. Production MIPS/uClibc, target BusyBox/procfs process visibility, ZeroTier interface/route readiness, firmware/TRX and L4 remain NOT VERIFIED.
- Root `dev_log.md` grows 40→41 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next: inspect every step/full log for exact-head ZeroTier and AI docs Actions. If green, continue the distinct add-on audit; ZeroTier's unbounded interface and NAT-route readiness loops in `rules()` are the next follow-up. FTP cleanup TOCTOU remains P2 OPEN/BLOCKED; NFS remains real-client evidence-gated.

---

## 2026-11-12 09:53 CST (UTC+8) — Harden DDNSTO stop/restart against stale PID signals

- Base main SHA: `9e5351d7f790dca5de5ec2495291fd34edaa658c`; parent `8e1e0c75afd3b95796cd3354d763b90395c9ef05`.
- Resumed the handoff's whole-tree RC/add-on process-control audit outside the recently covered VPN server and HTTPD paths. Traced the DDNSTO caller chain: RC notification dispatch -> `restart_ddnsto()` -> `stop_ddnsto()/start_ddnsto()` -> ROMFS-installed `/usr/bin/ddnsto.sh`.
- Found a PID-reuse hazard and restart fail-open: `ddnsto.sh` cached the numeric output of `pidof ddnsto`, sent TERM using `killall ddnsto`, then unconditionally sent SIGKILL to the stale PID list. A process that exited on TERM could have its PID reused by an unrelated program. There was no bounded graceful-stop interval, no daemon-absence postcondition, and start continued even if shutdown could not be confirmed.
- Replaced the stale numeric signal with a bounded lifecycle: TERM by process name; up to five one-second polls of live `pidof`; only after timeout, SIGKILL through fresh `killall -9 ddnsto` process-name matching; then a second bounded absence check. Failure propagates from stop and prevents start from launching a replacement. No RC API, daemon argument, or ROMFS install change.
- Added five-case shell regression for no-process stop, graceful exit, TERM timeout/escalation, persistent process failure and start gating; it overrides shell `kill` to catch any numeric stale-PID signal. Added semantic source checker for lifecycle ordering, ROMFS installation, RC caller graph and exact-once symmetric push/pull_request workflow filters.
- Local host validation in an isolated workspace: `sh -n` passed for the production script and regression; the regression printed `DDNSTO lifecycle stop/start regression: PASS`; `python3 -m py_compile` passed for the source checker. The semantic checker and AI docs validator are scheduled in Actions for this exact new SHA; results must be fetched and inspected before L2 is called closed.
- The full repository could not be cloned in this environment because container DNS resolution for github.com failed. No local full-tree documentation validator, target BusyBox, MIPS/uClibc build, complete firmware/TRX or router test is claimed.
- Next work: after exact-head Actions, continue auditing the other optional-service wrappers and their RC callers for bounded process identity/stop, concurrent lifecycle control, file publication and cleanup. FTP upload-cleanup TOCTOU remains P2 OPEN/BLOCKED; NFS requires real-client evidence.
- Root `dev_log.md` grows 39→40 complete records; no rotation or INDEX update. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Production MIPS/uClibc, full firmware/TRX and L4 router/hardware behavior remain NOT VERIFIED.

---

## 2026-11-12 09:39 CST (UTC+8) — Restore required handoff marker and assert symmetric OpenVPN workflow filters

- Base main SHA: `8e1e0c75afd3b95796cd3354d763b90395c9ef05`; parent `d946bb4f6cd305924041665f100900597de05318`.
- Exact-head VPN stability [43135536988] passed all steps on the base SHA, including lifecycle lock, callback identity, shared lease lock and expanded OpenVPN callback source contract. AI docs [43135537018] failed because SESSION_HANDOFF.md no longer contained the required literal `## Active task` heading; infosvr source contracts [43135537134] failed on the same missing governance marker after both GNU89 ASan/UBSan packet and signal policy binaries passed. No production infosvr failure occurred.
- Restored the handoff's mandatory `## Active task` heading for the real next work item, documented the intermediate CI outcome and added an exact-once push/pull_request OpenVPN source-filter assertion to the VPN contract.
- Recheck the VPN, AI docs and infosvr Actions on this commit's exact SHA. The expected docs validator count is root log=39/119 and archives dev-log=7 / handoff=22 / roadmap=18.
- FTP failed-upload cleanup remains P2 OPEN/BLOCKED: Linux 4.4 has no inode-conditional unlink-by-fd; do not patch without a proved design preserving STOR/APPE/REST/STOU and safety against concurrent writers. Continue the broader RC/add-on process-control audit after CI.
- Root dev_log.md grows 38→39 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Production MIPS/uClibc, full firmware/TRX and L4 router/hardware behavior remain NOT VERIFIED.

---
## 2026-11-12 09:35 CST (UTC+8) — Close OpenVPN/RC shared lease race and repair exact-head CI contract

- Base main SHA: `d946bb4f6cd305924041665f100900597de05318`; parent `7213bce03885fac5c83b5c6e06e7eee390c259a2`. This change extends the VPN semantic source contract to inspect vpn_openvpn.c and adds that production source to both push and pull_request filters of the VPN stability workflow.
- Resumed the handoff's P2 OpenVPN/RC shared lease-lock and late-callback stop-fence task. The production implementation already present on the base SHA uses one root-owned 0620 root:nogroup lock in APP_OPENVPN builds, PID-file marker removal before snapshot clear under that lock, post-lock parent-PID validation and stale-callback hook suppression. PPP-only builds retain a root-private lease lock.
- Fixed a syntax error in scripts/check_vpn_server_lifecycle.py on commit d946bb4f6cd305924041665f100900597de05318: the FIFO and permission assertions had been split into malformed nested require() calls. On that exact SHA, VPN stability [43132697870] and AI docs [43132697890] both passed. Full logs/steps inspected; lifecycle lock, callback identity, shared lease lock, integrated source contract and handoff docs validator all printed PASS (root log=37/119; archives=7/22/18).
- Added OpenVPN-side integration contracts for PID-file ownership/parent fencing, shared-lock-before-identity-check ordering, no-follow lease reads and staged publication, stale hook suppression, root PID-marker/snapshot clear order, and restart stop/clear/start ordering. The workflow now triggers symmetrically on changes to vpn_openvpn.c; exact-head Actions for this checker expansion are the next required validation.
- FTP failed-upload cleanup remains P2 OPEN/BLOCKED: Linux 4.4 offers no inode-conditional unlink-by-fd, and the current fstatat()/unlinkat() pair cannot guarantee race-free deletion against non-cooperating writers. Do not patch without a proved design preserving STOR/APPE/REST/STOU behavior. Continue the broader RC/add-on process-control audit after exact-head CI.
- Root dev_log.md grows 37→38 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Production MIPS/uClibc object/full firmware/TRX builds, real router OpenVPN authentication/stop timing and all L4 hardware behavior remain NOT VERIFIED.

---
## 2026-11-12 08:38 CST (UTC+8) — Unify OpenVPN and RC VPN lease serialization

- Base main SHA: `6e5d19556c22f658eea802a36ede63885927f57b`. Exact-base OpenVPN [43077082467], AI docs [43077082454] and infosvr [43077082525] were SUCCESS; all steps and complete logs were inspected. The docs validator reported root log=36/119 and archives=7/22/18.
- Closed the prior separate-lock-domain design gap at source level. APP_OPENVPN builds now use one stable `/tmp/.vpns.leases.shared.lock` inode, prepared by root with owner root, group `SYS_GROUP_NOGROUP`, mode 0620. Dropped-privilege nobody callbacks use the same F_WRLCK inode as root start/stop and PPP lease updates. PPP-only builds retain the previous root-private `/tmp/.vpns.leases.lock` and do not require OpenVPN's group account.
- The shared lock validates that `/tmp` is a root-owned sticky directory; opens lock files write-only, nonblocking, no-follow and close-on-exec; and verifies regular-file type, owner/group, exact mode and single link. O_NONBLOCK plus a FIFO regression prevents a pre-created FIFO at the predictable lock path from blocking service startup.
- Found that `kill_process_pidfile()` can return immediately after sending SIGKILL, without waiting for the daemon to exit. Both `vpn_server.c` and the OpenVPN restart-clear path now unlink the root-owned OpenVPN PID marker before unlinking the lease snapshot, while holding the shared lock. A late/re-parented callback waits for the lock, then fails the PID-file/parent identity check and cannot resurrect a cleared snapshot.
- Moved the shared OpenVPN PID-file constant into `rc.h`; root clear uses it only under `APP_OPENVPN`. Callback handlers distinguish stale-daemon status `-2` (skip the external hook) from ordinary bookkeeping errors (log but still run the configured hook), preserving hook independence from inventory I/O failure.
- Lease source opens now use O_NONBLOCK/O_NOFOLLOW/O_CLOEXEC and fstat checks for regular type, callback owner and single link. Bytewise copying rejects embedded NUL, malformed or overlong records, and read errors before same-directory staged publication.
- Added root:nogroup permission/contention coverage that drops a child to nobody, plus mode, symlink, hardlink and FIFO checks. Extended parser regressions for PID mismatch/malformed/overflow, embedded NUL and long records. Added source contracts for root and OpenVPN clear ordering, shared-lock fallback for PPP-only builds, hook behavior and path-filter symmetry. Exact-head Actions for this commit are pending; do not claim L2 closure until their complete logs are inspected.
- The previously run GNU89 ASan/UBSan lease-policy and lock host tests passed before this final conditional APP_OPENVPN/FIFO extension; final exact-tree host/CI confirmation remains pending. No production MIPS/uClibc build, firmware/TRX build or physical-router test is claimed.
- Next: inspect exact-head OpenVPN, VPN server stability and AI docs Actions; docs validator must report root log=37/119, archives=7/22/18. If they pass, close L1/L2 and continue production toolchain/target-runtime verification. FTP upload-cleanup TOCTOU remains P2 OPEN/BLOCKED; NFS remains real-client evidence-gated.

---

## 2026-11-12 02:11 CST (UTC+8) — Fix OpenVPN lease snapshot parser newline scanset

- Base main SHA: 9cc9df9b4269561402b90bd4ccf3b26bc9b3cd0a; parent 9c550fe2492c9299b73aa0ca972f2349fb998207.
- Exact-head OpenVPN run [43074827945] on the base SHA passed the credential-staging and primary-config atomic-publication ASan/UBSan regressions, then failed the new lease-snapshot policy regression at test line 50 while copying a two-row inventory. The source-contract step was skipped because the policy binary aborted.
- The parser used the C format scanset `%63[^\\n]`, whose double-escaped newline excluded both a backslash and the ordinary character `n`; the valid peer name `phone` was truncated and rejected. This commit changes the scanset to `%63[^\n]` at the C-source level (one backslash escape), excluding only newline.
- Recompiled the current helper and unchanged regression with `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`; the local test printed `OpenVPN server lease snapshot parsing and filtering policy: PASS`.
- Exact-base AI documentation run [43074827991] SUCCESS and reported root log=35/119, archives dev-log=7 / handoff=22 / roadmap=18. HTTPD umbrella [43074827969] SUCCESS; all 39 job steps passed. The corrected OpenVPN workflow and docs Actions for this commit remain pending.
- Cross-module lifecycle synchronization is not closed: the new OpenVPN callback writer locks `/tmp/vpns.leases.lock` (lease path plus `.lock`), while `vpn_server.c` lifecycle clear/PPP callback code uses `/tmp/.vpns.leases.lock`. These are separate lock domains and may not serialize concurrent startup/stop clearing with OpenVPN callbacks. Keep this as the next audit; do not claim whole VPN lease lifecycle consistency.
- Root `dev_log.md` grows 35→36 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: inspect exact-head OpenVPN and AI docs Actions, every step/full log; require all three OpenVPN regressions plus the source/workflow contracts and docs validator root log=36/119, archives=7/22/18. Then resolve the lock-domain mismatch with an ownership/permission design proven for the OpenVPN `user nobody` process and root lifecycle clear; add concurrency and failure-preservation coverage. FTP cleanup TOCTOU remains OPEN/BLOCKED; NFS requires client evidence; MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 02:02 CST (UTC+8) — Close OpenVPN primary config and atomically publish client CCD files

- Base main SHA: 023d4bc4a5fbc37c93dfb6b0f8958f9ea771e030; parent 6fb66775a0e750953e75f36e37ac4354751dedac.
- Closed primary OpenVPN server/client configuration atomic publication L1/L2 on exact SHA 6fb66775a0e750953e75f36e37ac4354751dedac: OpenVPN [43073255439], AI docs [43073255430] and HTTPD [43073255419] all SUCCESS. Both GNU89 -Wall -Wextra -Werror ASan/UBSan/leak binaries and complete source/workflow contracts printed PASS. Full logs and every step inspected; docs validator root log=34/119, archives=7/22/18.
- Whole-module review of vpn_openvpn.c found per-client CCD files still used fopen("w+") on the live path, ignored fprintf/fclose failures and chmodded the file after close.
- THIS COMMIT changes openvpn_create_server_acl() to validate the CCD root with lstat (rejecting symlink/non-directory), stage each per-client CCD beside its destination using openvpn_config_file_open(), and publish with the checked openvpn_config_file_publish(). Overlong paths, stage errors and publish errors fail the ACL generation.
- The server config caller now propagates ACL failure: closes and unlinks its unpublished primary config stage and returns nonzero before daemon startup. Each CCD file retains its previous complete version when its own staging/publish operation fails.
- Scope limitation: multiple CCD files publish sequentially, so this is not a whole-directory transaction; stale CCD files for deleted clients are not reconciled.
- Exact-head OpenVPN [43073850268], AI docs [43073850351] and HTTPD umbrella [43073850247] on THIS COMMIT all SUCCESS. Every step and full logs inspected. Credential/config GNU89 sanitizer binaries PASS; extended source checker includes CCD directory, atomic stage/publish and primary config fail-closed call chain. Docs validator root log=34/119, archives=7/22/18.
- Next audit found VPN lease callbacks still have a destructive failure path: disconnect opens source and fixed /tmp temp independently; source-open failure plus successful temp-open can replace the live lease file with empty content, while read errors/writes/close/rename are unchecked. Connect append similarly ignores write/close failures. This is the next P2 stability task.
- Root dev_log.md grows 34→35 complete records; no rotation or INDEX update due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: repair fail-closed same-directory lease snapshot update, add GNU89 malformed-line/source-open/read/write/close/rename failure tests and inspect callback concurrency. FTP TOCTOU remains OPEN/BLOCKED; NFS needs real-client evidence; MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 01:57 CST (UTC+8) — Fix OpenVPN source-checker C signature newline escapes

- Base main SHA: 6e52bb21f55099b61b1bc9627b23c6ec0a1b8ea7; parent d5b5ff7ed841ea12af2eda604580722e49bf7436.
- Exact-head OpenVPN run [43072744036] checked out this SHA. Both GNU89 -Wall -Wextra -Werror ASan/UBSan/leak binaries passed: "OpenVPN client credential staging policy: PASS" and "OpenVPN config atomic publication and failure preservation: PASS".
- The source-contract step failed at line 74 in body(source, signature): two C function signature literals encoded two backslashes before n, so the helper searched for literal backslash-n instead of the newline between return type and function name. No production compile/runtime regression failed; checker stopped before completing source contracts.
- Exact-head AI handoff docs [43072744011] and infosvr [43072744135] both SUCCESS. Docs validator reported root log=33/119, archives dev-log=7 / handoff=22 / roadmap=18; all steps passed.
- THIS COMMIT reduces the two Python source literals to one newline escape. No production OpenVPN code changed for this checker defect.
- Corrected OpenVPN and AI docs Actions remain pending. Do not close the OpenVPN config L2 task until the source checker passes on one exact SHA.
- Root dev_log.md grows 33→34 complete records; no rotation or INDEX update due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: inspect exact-head OpenVPN credential/config and AI docs Actions, every step/full log. Require both sanitizer tests and source/workflow contracts plus docs root log=34/119 and archive counts 7/22/18. MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 01:55 CST (UTC+8) — Align OpenVPN fail-closed source contract with staged config writer

- Base main SHA: d5b5ff7ed841ea12af2eda604580722e49bf7436; parent d65a890e11712db880c09b48e05215a691118ab6.
- Exact-head OpenVPN run [43072227995] checked out this SHA. Both GNU89 -Wall -Wextra -Werror ASan/UBSan/leak binaries passed: "OpenVPN client credential staging policy: PASS" and "OpenVPN config atomic publication and failure preservation: PASS".
- The remaining source-contract step failed with ValueError at line 51 because an older assertion still searched for fp = fopen(conf_file, "w+"). The same source had intentionally migrated client config creation to openvpn_config_file_open(), so the test boundary was stale; the other source assertions already require staged open and atomic publish.
- Exact-head AI handoff docs [43072228055] and infosvr [43072227944] both SUCCESS. Docs validator reported root log=32/119 and archives dev-log=7, handoff=22, roadmap=18. Every job step passed; full logs showed no test errors.
- THIS COMMIT changes the client fail-closed ordering assertion to require credential publication and its failure return before openvpn_config_file_open(). No production code changed for this checker failure.
- Corrected OpenVPN and docs Actions for THIS COMMIT are pending. The previous run's two host test binaries passed; its source checker did not pass.
- Root dev_log.md grows 32→33 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: inspect corrected OpenVPN credential/config and AI-doc Actions, every step/full logs. Require both GNU89 sanitizer regressions and source contracts plus docs root log=33/119, archives=7/22/18. MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 01:51 CST (UTC+8) — Correct OpenVPN config regression shell continuations

- Base main SHA: d65a890e11712db880c09b48e05215a691118ab6; parent 8c715b2b156b6c40627d026a84007eb4059a76c6.
- Inspected exact-head Actions for d65a890e11712db880c09b48e05215a691118ab6. OpenVPN run [43071668469] failed at step 4; credential step 3 printed "OpenVPN client credential staging policy: PASS". The config compile command had two literal trailing backslashes on each of four continuation lines; the shell passed a backslash argument to the linker, which reported "ld: cannot find backslash input". The config binary and source-contract step did not run.
- Other exact-head runs passed: AI handoff docs [43071668470], HTTPD authentication/security umbrella [43071668464] and infosvr [43071668500]. Every job step in those runs completed SUCCESS; full logs showed no test errors.
- THIS COMMIT corrects only the four config compiler line continuations to one backslash each. Production source was not changed in response to this workflow syntax failure.
- Exact-head corrected OpenVPN and docs Actions are pending. No claim is made that the failed config binary ran.
- Root dev_log.md grows 31→32 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: inspect exact-head OpenVPN workflow and AI-doc workflow, every step/full log; require credential and config GNU89 -Wall -Wextra -Werror ASan/UBSan/leak regressions, source contract, and docs root log=32/119 with archives=7/22/18. MIPS/uClibc, full firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 01:47 CST (UTC+8) — Make OpenVPN server/client configuration publication atomic

- Base main SHA: `8c715b2b156b6c40627d026a84007eb4059a76c6`.
- Closed the preceding Wi-Fi shutdown bridge-detach task on that exact SHA: wireless [43070956995], AI docs [43070956985], infosvr [43070957210] all SUCCESS. Every job step and full log inspected. GNU89 wireless status policy, bridge membership/teardown regression and complete radio/membership source contracts all PASS; docs validator root log=30/119, archives=7/22/18. The earlier dfd4 failure was a multiline-string checker defect, corrected and green on 8c715.
- OpenVPN audit found both server and client config generators truncating live files with `fopen("w+")`, ignoring write/close/chmod errors and returning success; their start callers could therefore launch against an empty/partial config and destroy last-good state.
- Fix: new shared same-directory `mkstemp` writer verifies regular file owner/link count and keeps the stage 0600 while generating. Publication checks flush/ferror, EINTR-safe fsync, applies final mode 0644 only after data sync, fsyncs again, checks close, then atomically renames. Failure removes the stage, preserves errno and old destination, and is propagated by both config generators to existing startup gates.
- New GNU89 ASan/UBSan regression covers private staged mode/final mode under umask 0000, live old config before rename, RLIMIT_FSIZE EFBIG flush failure, stage cleanup, symlink replacement, rename failure and invalid/short paths. Standalone test passed: `OpenVPN config atomic publication and failure preservation: PASS`. OpenVPN workflow now compiles it and source-checks both server/client integrations; exact-head Actions pending. No complete target build claimed.
- Per-client CCD files and VPN lease snapshots remain separately unaudited. File fsync plus rename is not a parent-directory durability guarantee.
- Root `dev_log.md` grows 30→31 complete entries; no rotation or INDEX update due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: inspect OpenVPN credential/config and AI docs Actions on resulting exact main SHA; all steps/full logs. Require both sanitizer tests, RLIMIT_FSIZE failure injection, preservation/cleanup, server/client source contracts, exact-once symmetric path filters and docs root log=31/119, archives=7/22/18. Then audit CCD and VPN lease snapshots. FTP TOCTOU remains OPEN/BLOCKED; NFS needs client evidence; MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 01:43 CST (UTC+8) — Make Wi-Fi teardown source contract formatting-agnostic

- Base main SHA: dfd4e4711cf53620b7be75c9f65d430154249fc8; parent 55740d3491fba908416b930664143943b5acca1a.
- The first exact-head wireless run [43070659944] compiled and ran both GNU89 regression binaries successfully, including "Bridge membership and teardown policy regression: PASS". Its source contract then failed only because RT3352 MII intentionally split a shutdown call across two C source lines, while the checker compared a single-line literal. The failure was a checker formatting false negative, not a C compile or policy-test failure.
- Corrected the existing wireless workflow checker to normalize whitespace before verifying each exact interface-to-bridge call and ordering. This keeps semantic assertions strict while accepting legal C formatting.
- The same SHA's WAN callback [43070660302], watchdog [43070659777] and infosvr [43070659762] suites passed. AI docs [43070659951], iNIC [43070659826] and HTTPD umbrella [43070659716] were still running at the time inspected; they are not recorded as passed here.
- Corrected exact-head wireless/docs Actions are pending. The wireless lifecycle task remains open until the corrected workflow and docs validator pass on the same resulting SHA.
- Root dev_log.md grows 29→30 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: inspect every job step and full logs for exact-head wireless, AI docs, and all other triggered workflows. Require corrected GNU89 bridge policy/source contract plus docs root log=30/119 and archives=7/22/18. MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 01:40 CST (UTC+8) — Detach Wi-Fi bridge ports during radio shutdown

- Base main SHA: 55740d3491fba908416b930664143943b5acca1a; parent c22542d36e9de26780b338267f41a91d99aaf34a. main remained the only remote branch.
- Closed the preceding WAN bridge VLAN teardown task on exact SHA 55740d3491fba908416b930664143943b5acca1a: wireless bridge/VLAN regression [43069624285], AI handoff docs [43069624357], infosvr packet/signal [43069624265] and WAN callback umbrella [43069624334] all SUCCESS. Each job checked out the exact SHA; all steps passed. The bridge GNU89 -Wall -Wextra -Werror regression printed "Bridge membership and teardown policy regression: PASS"; source contracts printed "Wireless driver/restart failure state-publication contracts: PASS"; docs validator reported root log=28/119 and archives=7/22/18.
- New Wi-Fi lifecycle audit found stop_wifi_all_wl() and stop_wifi_all_rt() brought APCLI/WDS/AP/iNIC interfaces down but did not consistently detach their bridge ports. The RT3352 MII branch detached only the guest VLAN and left the conditional iNIC main port behind. When WDS configuration shrank, stale WDS bridge membership could survive a restart because only currently configured WDS ports were brought back up.
- Fix: introduce one stop helper that requires successful interface-down before bridge detach and reports either failure into aggregate shutdown status. Map the iNIC guest interface to its VLAN bridge port; detach the iNIC main port too. Restart remains fail-closed when a down or detach operation fails, while shutdown continues attempting remaining interfaces.
- Bridge membership requests are now idempotent for both add and delete. Query the complete Linux 4.4 bridge-port inventory before either ioctl; fail closed when inventory is unavailable; after add EBUSY or delete EINVAL, re-query and accept only if the requested state is now satisfied. A port owned by another bridge is not detached from that bridge.
- GNU89 bridge policy regression now covers add/delete already-satisfied states, unavailable inventory and duplicate-operation race errno mapping. The wireless Actions source contract verifies shutdown ordering, the iNIC guest-VLAN mapping, iNIC main detach, both bridge-membership directions and fail-closed restart propagation.
- Exact-head wireless and AI docs Actions for THIS COMMIT are pending. No complete local checkout, MIPS/uClibc production object, full firmware/TRX or L4 router behavior is claimed.
- Root dev_log.md grows 28→29 complete records; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: inspect the wireless radio-state and AI handoff-doc Actions on the exact resulting main SHA, every job step and full logs. Require GNU89 policy regression and lifecycle/membership contracts plus docs validator root log=29/119, archives=7/22/18 to pass. MIPS/uClibc and target board/iNIC bridge behavior remain NOT VERIFIED.

---

## 2026-11-12 01:35 CST (UTC+8) — Propagate VLAN deletion failures during WAN bridge teardown

- Base main SHA: `c22542d36e9de26780b338267f41a91d99aaf34a`; parent `c2e1c85517ed2dcbe094ed43e3cae18d7af4d972`.
- Closed the preceding DNSMasq DHCP pool task on that exact SHA: DNSMasq lifecycle [43069047780], AI docs [43069047752], HTTPD umbrella [43069047723], infosvr [43069047578], Samba [43069047831] and miniupnpd [43069047633] all SUCCESS. Every DNSMasq/docs step and full logs were inspected; DHCP policy, restart, lifecycle lock, Inadyn, parser/source and docs validator (27/119; archives 7/22/18) passed.
- New whole-network teardown review found `br_del_all_ifs()` discarded `vconfig_del_if()` return status. The WAN caller `disassembly_bridges_wan()` aggregates that result, skips bridge deletion on failure, and the full WAN restart aborts when bridge teardown fails; ignoring VLAN-delete errors bypassed that existing fail-closed gate and could leave a stale VLAN interface.
- Fix: select only dotted VLAN interface names (preserving `IFNAME_LAN` and ordinary physical ports), check `vconfig_del_if()`, and aggregate the result via `bridge_cleanup_status_update()`.
- Regression: extend the existing GNU89 bridge membership/cleanup test with VLAN selection, removal-disabled, LAN/physical-port, null/empty-name and cleanup-status cases. Extend the existing workflow source contract to require the selected VLAN delete result to reach `br_del_all_ifs()` status.
- Local standalone policy regression compiled with `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` and printed `Bridge membership and teardown policy regression: PASS`. No complete local checkout, whole-repository checker or production target build is claimed.
- Exact-head wireless and AI docs Actions are pending on THIS COMMIT. L3 MIPS/uClibc bridge/VLAN ioctl behavior, complete firmware/TRX and L4 router behavior remain NOT VERIFIED.
- Root `dev_log.md` moves 27→28 complete entries; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- Next GPT: inspect wireless bridge/membership/teardown and AI docs Actions on the exact resulting main SHA, all steps/full logs. On green, close this bounded L2 slice; target MIPS/uClibc bridge/VLAN validation remains blocked pending checkout/toolchain. FTP cleanup TOCTOU remains OPEN/BLOCKED; NFS still requires external-client evidence.

---

## 2026-11-12 01:29 CST (UTC+8) — Prevent DNSMasq DHCP pools from including the router LAN address

- Base main SHA: `c2e1c85517ed2dcbe094ed43e3cae18d7af4d972`; parent `e6801775fc3af56dd084321eca9376899b2e7d76`. Main was re-fetched before preparing this commit; it remains the only remote branch.
- Closed DNS Forwarder L1/L2 on exact SHA `c2e1c85517ed2dcbe094ed43e3cae18d7af4d972`: workflow [43066687882], AI docs [43066687814] and infosvr [43066687818] all SUCCESS. The non-root-CWD relative-path failure/absolute-path success regression, service/ROMFS/execvp contracts and ten exact-once symmetric path filters passed; docs validator reported root log=26/119 and archive inventory=7/22/18.
- DNSMasq audit found `simple_dhcp_range()` computed its fallback as network+1 through broadcast-1. For a common `192.168.1.1/24` LAN that includes the router's own address as a DHCP candidate. `chk_valid_subnet_pool()` only checked subnet membership, so it accepted ranges including the router, network/broadcast endpoints and reversed ranges.
- THIS COMMIT adds `dnsmasq_dhcp_range_policy.h` and its GNU89 sanitizer regression. The policy validates contiguous usable masks and interval ordering, excludes network/broadcast/router addresses, and chooses the larger contiguous side around the router for fallback. /31, /32, non-contiguous and zero masks fail closed.
- `prepare_dns_dhcpd()` now validates/fixes the DHCP range before generated host/ARP files are touched. If LAN IP/netmask cannot produce a safe range, preparation returns EINVAL before those side effects; the existing DNSMasq restart policy preserves a running daemon when preparation fails.
- Local host test compiled and passed under GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`: `DNSMasq DHCP pool conflict policy: PASS`. This is the standalone policy test only; production RC cross-build and the full workflow are not yet verified.
- DNSMasq lifecycle workflow now compiles/runs the new regression and checks helper integration, early fail-closed ordering, parser/source contracts and exact-once symmetric push/pull_request filters. Exact-head DNSMasq and AI docs Actions are pending.
- Root `dev_log.md` contained 26 complete records and now contains 27; no rotation or archive INDEX update is due. Existing archive shards/indexes remain unchanged.
- Next GPT: fetch the resulting exact main SHA; inspect DNSMasq lifecycle and AI docs runs, all steps and full logs. Require DHCP range policy, restart policy, cross-process lock, Inadyn sanitizer regression, corrected parser/source checks and docs validator (root log=27/119, archives=7/22/18) to pass. Production MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---
## 2026-11-12 01:16 CST (UTC+8) — Correct DNS Forwarder CWD regression fixture line endings

- Base main SHA: `e6801775fc3af56dd084321eca9376899b2e7d76`; parent `31578abaefe5b2838b7c825460e21e730176b563`.
- Exact-head DNS Forwarder workflow [43066299135] failed during its temporary executable test. The old relative slash-containing path correctly returned ENOENT from the unrelated CWD, but the subsequent absolute-path run also returned ENOENT because the fixture wrote literal `\\n` sequences into the fake shell script instead of actual line terminators.
- AI handoff docs [43066299302] passed on this base (root log=25/119; archives=7/22/18). Networkmap [43066300888], GPIO [43066298902], VPN server [43066299119], WAN ARP [43066298966], HTTPD lifecycle [43066299031], HTTPD security [43066299164], detect_link [43066299084], infosvr [43066298895], watchdog [43066298861] and crond [43066299440] passed. SkipDBv2 [43066299131] remained in progress at last inspection.
- THIS COMMIT makes the test fixture write a valid executable with actual newline terminators and checks the marker as `start`. The production RC absolute path, relative-path failure assertion, source/installer contracts and workflow filters remain unchanged.
- This is a regression-test harness correction; no product-code failure was demonstrated. Exact-head DNS Forwarder and docs reruns remain required before L2 closure.
- Local checkout is unavailable. The corrected script was reviewed, and its temporary shell fixture behavior was independently exercised with an equivalent host snippet (PASS); the complete repository checker is not claimed locally.
- Root `dev_log.md` moves 25→26 complete entries; no rotation or INDEX update is due. Archive shards/indexes remain unchanged.
- Next GPT: inspect corrected DNS Forwarder and AI docs Actions on the resulting exact main SHA, all steps/full logs. Require relative CWD failure, absolute execution success, RC/ROMFS/execvp contracts, ten symmetric exact-once filters and docs root log=26/119, archives=7/22/18. Production MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-12 01:11 CST (UTC+8) — Make DNS Forwarder startup path CWD-independent

- Base main SHA: `31578abaefe5b2838b7c825460e21e730176b563`; parent `54504272e2c4ca0f7783ba78aa52a2bfe212de1c`. Main is the only branch; no remote checkpoint.
- Closed GPIO short-press timing L1/L2 on exact SHA `31578abaefe5b2838b7c825460e21e730176b563`: GPIO [43065042288], AI docs [43065042295] and infosvr [43065042298] all SUCCESS. Startup/timing GNU89 ASan/UBSan binaries and source/caller/path contracts passed; docs validator reported root log=24/119 and archive inventory=7/22/18.
- Audited DNS Forwarder service startup: `start_dnsforwarder()` passed `usr/bin/dns-forwarder.sh`, but ROMFS installs the launcher as `/usr/bin/dns-forwarder.sh`. The slash-containing relative pathname is resolved by `execvp()` from CWD, not via PATH; non-root CWD callers therefore receive ENOENT. PID 1's normal `chdir("/")` currently masks the dependency in known boot flow.
- THIS COMMIT changes the RC start invocation to the absolute installed path and adds `scripts/check_dns_forwarder_launch.py` plus `.github/workflows/dns-forwarder-launch-path.yml`. The regression proves relative-path failure/absolute-path success from an unrelated CWD and checks service start/stop/restart wiring, ROMFS destination, `eval()`/`execvp()` use and ten exact-once symmetric workflow filters.
- Local repository checkout is unavailable; the new Python regression/source contract is NOT RUN locally. Exact-head DNS Forwarder and AI docs Actions are required for L2.
- Root `dev_log.md` contained 24 complete records before this entry and contains 25 now; no rotation or archive INDEX update is due. Archive shards/indexes remain unchanged.
- Next GPT: fetch resulting live `main`; inspect exact-head DNS Forwarder and AI docs Actions, all job steps and full logs. Require CWD regression, source/ROMFS/caller checks, symmetric path filters and docs validator (root log=25/119, archives=7/22/18) to pass. On green, close DNS Forwarder L2 and rotate to another whole-userland stability defect. Production MIPS/uClibc, full firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-11 — Fix GPIO timing workflow path-filter symmetry

- Base main SHA: `54504272e2c4ca0f7783ba78aa52a2bfe212de1c`; parent `65f070aa3dfaa73154d3ab1ecb03d82a17abc388`. Main was rechecked before this commit; only main exists.
- Exact-head GPIO workflow [43064587271] compiled and ran both GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` binaries successfully: daemon startup policy PASS and hold-timing threshold policy PASS. Its source checker then failed at `push path count: trunk/user/rc/src/gpio_btn_timing_policy.h`.
- Root cause: an earlier string replacement matched the old path pair inside its expanded replacement before reaching the `pull_request` block. As a result, both new timing paths appeared twice in push and zero times in pull_request; the checker correctly rejected this.
- THIS COMMIT normalizes both workflow event-filter sections explicitly, placing each timing header/test path exactly once in push and once in pull_request. The source checker keeps strict count assertions.
- AI docs [43064587290] passed on the base (root log=23/119, archive counts dev-log=7, handoff=22, roadmap=18); infosvr [43064587210] also passed. Corrected exact-head GPIO/docs Actions remain pending.
- Root dev_log grows 23→24 records, below the 120-entry rotation threshold; archive index/shards remain unchanged.
- Validation boundary: exact-parent local/CI startup and timing sanitizer binaries PASS; no exact-head source-contract pass yet. MIPS/uClibc, full firmware/TRX, live GPIO/LED/uevent and L4 remain NOT VERIFIED. FTP cleanup TOCTOU remains OPEN/BLOCKED; NFS evidence-gated.
- Next GPT: inspect exact-head corrected GPIO and AI docs Actions, all steps/full logs. Require both binaries, source integration, exact-once symmetric filters and docs validator (root log=24/119, archives 7/22/18) to pass; then close L2 and move across the whole-userland ledger.

---

## 2026-11-11 — Correct the GPIO short-press 800 ms boundary

- Base main SHA: `65f070aa3dfaa73154d3ab1ecb03d82a17abc388`; parent `d002eba06a914d75ac79b20867dbd3cdb6f416ca`. Main was rechecked before creating this commit; it is the only remote branch.
- Closed the resumed HTTPD daemon lifecycle task on the base SHA. HTTPD lifecycle [43063659893], HTTPD security umbrella [43063082646], AI handoff docs [43063659714] and infosvr signal/packet regression [43063659718] all SUCCESS. Dedicated HTTPD job 6/6 steps, umbrella 41/41 and docs 7/7; source contract printed PASS. Docs validator reported root log=22/119 and archives dev-log=7, handoff=22, roadmap=18.
- Found a separate front-panel timing defect in `trunk/user/rc/src/gpio_btn.c`: the initial press count is 1 and updates in 100 ms quanta (`floor(elapsed_ms / 100) + 1`). Thus 700–799 ms maps to count 8, but `btn_released_handle()` required `count < CANCEL_COUNT`, with `CANCEL_COUNT=8` described as 800 ms. The entire 700–799 ms WPS/FN short-action interval was suppressed.
- THIS COMMIT centralizes elapsed monotonic-time conversion and short/long/reset threshold predicates in `gpio_btn_timing_policy.h`. Short actions include counts 1–8 and exclude count 9 at 800 ms; >3 s long and >5 s reset thresholds are preserved. Receive timeout and hold-count accounting share the same 100 ms constant.
- New GNU89 regression covers 0/699/700/799/800 ms, 2999/3000 ms, 4999/5000 ms, negative-delta normalization and `INT_MAX` saturation. Local `-Wall -Wextra -Werror -fsanitize=address,undefined` with leak detection passed: `GPIO button hold timing and threshold policy: PASS`.
- Existing GPIO lifecycle Actions now compile/run both startup and timing policy binaries and statically check source integration, threshold boundaries, and symmetric push/pull_request path filters. Exact-head GPIO and AI docs Actions for THIS COMMIT are pending.
- Changed: `gpio_btn.c`, new timing policy header/test, `.github/workflows/gpio-btn-lifecycle.yml`, `scripts/check_gpio_btn_lifecycle.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and root `dev_log.md`. Root log grows 22→23 entries; no archive rotation/index update due.
- Validation boundary: L1 source review and isolated L2 GNU89 ASan/UBSan timing policy pass locally. No target MIPS/uClibc object, full firmware/TRX, actual key/LED/uevent or L4 router result is claimed. FTP cleanup TOCTOU remains OPEN/BLOCKED and NFS remains external-evidence gated.
- Next GPT: fetch the resulting main SHA; inspect exact-head GPIO startup/timing and AI docs Actions including every step/full logs. Require 700/799 ms inclusion, 800 ms exclusion, unchanged 3 s/5 s limits, source integration, both binaries, path filters and docs validator (root 23/119, archives 7/22/18) to PASS. Then close this L2 slice and rotate to a new whole-userland issue.

---

## 2026-11-12 01:10 CST (UTC+8) — Correct HTTPD lifecycle source-contract publication ordering

- Base main SHA: `d002eba06a914d75ac79b20867dbd3cdb6f416ca`; parent `1c4b9fe85df1ea2a8d9ec83035ea6cc295059b01`. Main was rechecked before editing; branch inventory remains main only.
- Exact-head HTTPD run [43063082671] on this SHA compiled the GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` policy binary and printed `HTTPD daemon startup, duplicate-launch, stop-state and restart policy: PASS`. The Python integration checker then failed at its start-state ordering assertion: it compared the post-probe state publication against the first `httpd_started=1`, which is the valid pre-existing-daemon branch. This was a checker false positive, not a failed policy regression.
- Exact-head AI handoff validation [43063082981] passed and printed `AI handoff documentation check: PASS (root log=21/119 entries, batch=100, retain=20, legacy/current archives: dev-log=7, handoff=22, roadmap=18)`.
- THIS COMMIT changes the checker to locate the successful-start `httpd_started=1` only after the detached-child probe loop begins, then assert it follows the successful process-presence branch and precedes firewall restart. The production HTTPD source and policy regression are unchanged in this corrective slice.
- Exact-head HTTPD lifecycle and AI docs Actions for THIS COMMIT are pending. Do not claim the source contract as passing until the corrected workflow completes on the new SHA.
- Root `dev_log.md` grows from 21 to 22 records, well below the rotation threshold. Existing archive shards/index remain unchanged (dev-log=7, handoff=22, roadmap=18).
- The previous SHA already contains the HTTPD launch/stop/restart fix and duplicate-launch preflight. Production MIPS/uClibc, full firmware/TRX, target HTTPD socket readiness, cross-process simultaneous-start locking and L4 remain NOT VERIFIED. FTP cleanup TOCTOU remains OPEN/BLOCKED.

## 2026-11-12 00:55 CST (UTC+8) — Prevent duplicate HTTPD launches on an already-running daemon

- Base main SHA: `1c4b9fe85df1ea2a8d9ec83035ea6cc295059b01`; parent `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`. The branch ref was rechecked before editing; main remains the only branch.
- Follow-up review of the HTTPD lifecycle fix found that a direct repeated `start_httpd()` call could still launch a second copy even though restart callers now stop first. The service entry point now checks `pids("httpd")` before `_eval()`: an existing daemon is marked started and returns `EBUSY`; a stale started flag is cleared only when process absence has been confirmed.
- Added `httpd_start_can_launch()` and regression cases for absent versus already-present processes. The GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` test printed `HTTPD daemon startup, duplicate-launch, stop-state and restart policy: PASS`.
- Updated the dedicated HTTPD workflow's source contract to require the pre-launch guard and state-reset ordering. Exact-head HTTPD and AI handoff Actions are pending; the preceding SHA's corresponding runs were queued at last inspection and are not claimed as passed.
- SESSION_HANDOFF and REFACTOR_ROADMAP were refreshed in place with the exact active task, verification boundary and next-GPT actions. Root `dev_log.md` grows from 20 to 21 records, below its 119-entry rotation threshold; existing archive parts remain unchanged.
- The PID 1 mdev fsync task remains L1/L2 CLOSED on exact SHA `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`. FTP cleanup TOCTOU remains OPEN/BLOCKED; production MIPS/uClibc objects, complete firmware/TRX and L4 router behavior remain NOT VERIFIED.

## 2026-11-12 00:20 CST (UTC+8) — Gate HTTPD service state on daemon lifecycle postconditions

- Base main SHA: `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`; parent `9eaf81cc6eef88f87ca44ffe10dc574a2e988250`. Main was rechecked before preparing this commit; the branch inventory remains main only.
- Previous PID 1 mdev fsync task is L1/L2 CLOSED on exact SHA `fb0e0a50f7a20d1cd18bd1841f03b0e25a019786`: mdev [43055641071], infosvr [43055640661] and AI handoff docs [43055640617] all completed SUCCESS. The two-injected-EINTR plus real-fsync GNU89 ASan/UBSan regression and source contracts passed.
- New P2 finding in `trunk/user/rc/src/services.c`: `start_httpd()` discarded `_eval()` status and unconditionally published `httpd_started=1`; this could advertise a service and open firewall ports after a failed launch or a daemon that immediately exited. `stop_httpd()` cleared the started flag before checking bounded `kill_services()` completion and ignored failure, while `restart_httpd()` launched regardless. A surviving prior daemon could therefore be hidden and duplicated.
- THIS COMMIT changes the HTTPD start/stop/restart APIs to return status. Start preserves launcher errors and polls for a live detached `httpd` process before publishing the flag or reopening firewall access. Stop only clears state after the shared lifecycle helper confirms process absence. Restart refuses to launch if stop fails.
- New `httpd_daemon_policy.h` and `tests/httpd_daemon_policy.c` cover launcher failure, child-presence postcondition, stop-state publication and restart gating. The local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer` regression printed `HTTPD daemon startup, stop-state and restart policy: PASS`. A dedicated path-filtered workflow adds service integration and RC declaration source contracts; exact-head workflow and AI-doc Actions are pending.
- Root `dev_log.md` reached 120 complete records including this entry. Per the 120-record rotation rule, the oldest 100 records are archived verbatim, oldest first, in `docs/dev-log/2026/11/part-007.md`; root retains the newest 20. Existing archive parts 001–006 are unchanged and the index is updated in this commit.
- FTP upload-cleanup TOCTOU remains P2 OPEN/BLOCKED until a Linux 4.4-safe design proves inode/parent safety while preserving STOR/APPE/REST/STOU semantics. Production MIPS/uClibc objects, complete firmware/TRX, target HTTPD socket readiness and L4 router behavior remain NOT VERIFIED.

## 2026-11-11 — Close firewall snapshot writer L2 and retry interrupted PID 1 mdev fsync

- Base main SHA: `9eaf81cc6eef88f87ca44ffe10dc574a2e988250`; parent `42800e0caaf90e0230f4e30d268e8301d0da2d70`. Branch inventory remains main only, without a remote checkpoint.
- Firewall descriptor-bound snapshot task is L1/L2 CLOSED on exact SHA `9eaf81cc6eef88f87ca44ffe10dc574a2e988250`: HTTPD [43053436665], infosvr [43053436537] and AI handoff docs [43053436425] all completed SUCCESS. The new descriptor-inheritance GNU89 ASan/UBSan regression printed PASS; transaction/status/startup binaries and corrected push/pull workflow path assertions passed; docs validator reported root log=118/119, archive counts dev-log=6, handoff=22, roadmap=18.
- New PID 1 mdev stability finding: `init_mdev_config_publish()` treated a single `fsync()` EINTR as a permanent failure, deleted the staged policy, and caused `init_mdev()` to return before kernel hotplug activation. A transient signal could leave this boot without the intended /etc/mdev.conf hotplug rules.
- THIS COMMIT retries file `fsync()` only for EINTR; other failures still fail closed. The host regression injects two EINTR returns before the real `fsync()`, requiring success after exactly three calls. Existing 0644 mode, old-policy preservation, atomic rename, and failed-publication cleanup checks remain.
- Local GNU89 `-Wall -Wextra -Werror` ASan/UBSan regression with leak detection passed: private staging + atomic publication + EINTR-safe fsync.
- Exact-head PID 1 mdev workflow and AI docs validator are pending on the resulting SHA; no repository Actions pass is claimed yet.
- Scope: `init_mdev_config_file.h`, `tests/init_mdev_config_file.c`, `.github/workflows/init-mdev-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Root log moves from 118 to 119 entries; archive rotation/index update is not due.
- Target boundary: production MIPS/uClibc, full firmware/TRX, BusyBox mdev/ash target and actual USB/storage/network hotplug L4 remain NOT VERIFIED. FTP cleanup TOCTOU stays OPEN/BLOCKED.
- Next GPT: inspect exact-resulting-SHA mdev security and AI docs Actions in full; require the injected EINTR regression and source/path checks to pass, then rotate to another userland subsystem.

## 2026-11-11 — Correct firewall CI path-filter contract after exact-head regression

- Base main SHA: `42800e0caaf90e0230f4e30d268e8301d0da2d70`; parent `02907c582c716a49c6255d2c06cbad429f5a3260`. Main ref was rechecked before editing.
- Exact-head FULLCONENAT [43052866763] and infosvr [43052866751] both completed SUCCESS on `42800e0caaf90e0230f4e30d268e8301d0da2d70`. HTTPD umbrella [43052866810] completed FAILURE at its Python source contract. The new `firewall_snapshot_fd.c` binary itself printed `firewall snapshot descriptor writer inheritance: PASS`; command-status, startup-policy and firewall transaction regression binaries preceding the checker ran without reported failure.
- Failure cause: `workflow.count('"trunk/user/rc/src/tests/firewall_snapshot_fd.c"') == 2` counted two push/PR path entries plus the checker assertion's own quoted path literal, so actual count was three. This was a test-contract defect, not a production firewall writer failure.
- THIS COMMIT replaces the self-referential count with independent parsing/assertions for the push and pull_request path blocks and separately confirms the firewall regression step references the test source. Production `firewall_ex.c` and the C regression are unchanged.
- The AI handoff docs run [43052866839] was still in progress when this correction was prepared. All exact-head jobs for the resulting SHA must be inspected before closing the task.
- Scope: `.github/workflows/httpd-security.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`. Root log moves from 117 to 118 complete newest-first records; no archive rotation or index update is due. This is a CI-contract correction attached to the in-flight code task, not a standalone docs-only commit.
- Target boundary: L3 MIPS/uClibc, complete firmware/TRX and L4 remain NOT VERIFIED. FTP upload-cleanup TOCTOU remains OPEN/BLOCKED pending a proved Linux 4.4-safe design.
- Next GPT: verify the new main SHA's HTTPD and AI docs Actions in full; require the corrected path contract and descriptor-writer regression to pass, then rotate to a different userland subsystem.

## 2026-11-11 — Close BusyBox ash lifecycle gate and bind firewall snapshots to their staged descriptor

- Base main SHA: `02907c582c716a49c6255d2c06cbad429f5a3260`; parent `21badf5dab5bc73f2768ccdbc381dcc70f6dcc0d`. Branch inventory: `main` only, no remote checkpoint.
- Exact-head Actions on the base SHA: storage [43042263807], infosvr [43042263817] and AI handoff docs [43042263966] all completed SUCCESS; all job steps and complete logs inspected. Storage built/linked repository BusyBox 1.24.2 and passed lock contention/release, teardown status, startup supervision/rollback, eject inventory, deferred Optware child-result/retry, daemon-stop postcondition and whole caller-graph/lock-order checks (steps 9–13). This closes the prior active handoff task at L1/L2 only.
- New firewall stability/security finding: `firewall_snapshot_core_table()` closed the mode-0600 `mkstemp()` descriptor before invoking `iptables-save` with shell redirection to the pathname. Reopening that shared-`/tmp` name loses the guarantee that the writer fills the exact private inode created by `mkstemp()`.
- THIS COMMIT keeps the descriptor open across `doSystem()`, explicitly clears `FD_CLOEXEC` for the system(3) shell, redirects stdout with `>&fd`, checks the writer's final close, and retains existing cleanup/restore path handling. A new GNU89 ASan/UBSan regression removes the staged pathname before shell output, then verifies bytes are written to the still-open inode. HTTPD workflow push/PR filters, compile/run step and source contract are extended accordingly.
- Local host probe for the exact descriptor-inheritance/redirection behavior passed under GNU89 with AddressSanitizer and UndefinedBehaviorSanitizer. The repository-level Actions test and docs validator are pending on the resulting commit SHA; no result is claimed before those runs complete.
- Relevant pre-existing firewall transaction cleanup test and HTTPD source contract passed in run [43009513122] at ancestor SHA `d2edf2dd08c85e0413f6469e2dccc6fa5a2df26a`; the transaction header, test, firewall integration and workflow blobs were confirmed byte-identical at base SHA `02907c582c716a49c6255d2c06cbad429f5a3260`.
- Scope: `firewall_ex.c`, new `tests/firewall_snapshot_fd.c`, `.github/workflows/httpd-security.yml`, and the live session/roadmap/log records. Root log moves from 116 to 117 records; no archive rotation or index update is due.
- Target boundary: production MIPS/uClibc, complete firmware/TRX, actual router /tmp semantics and L4 firewall/storage behavior remain NOT VERIFIED. FTP upload-cleanup TOCTOU remains OPEN/BLOCKED pending a proved Linux 4.4-safe design.
- Next GPT: fetch resulting main SHA; inspect exact-head HTTPD and AI handoff-docs runs in full. Require the new descriptor-writer GNU89 ASan/UBSan regression, source contract and path filters to pass. Then close this bounded slice at L2 and rotate to a different evidence-actionable userland module.

## 2026-11-11 22:47 CST (UTC+8) — Use BusyBox 1.24.2-compatible integer sleep in the lock regression

- Base main SHA: `21badf5dab5bc73f2768ccdbc381dcc70f6dcc0d`; parent `abff1ed57217c1fe482bf80c2c419dde66437bf0`; remote branch inventory: `main` only, no checkpoint.
- Exact-head Actions: infosvr [43041511033] SUCCESS and AI handoff docs [43041510759] SUCCESS; storage [43041510879] FAILURE. The documentation check reported root log=115/119 and archives dev-log=6, handoff=22, roadmap=18.
- Storage steps 3–8 passed. Step 9 built and linked BusyBox 1.24.2; direct ash/awk execution, descriptor-backed `flock -n 9` / `flock -u 9` smoke and the production Optware script ash syntax checks passed. Removing the unsupported `command -v` preflight allowed the lock regression to advance further.
- New exact failure: `sleep: invalid number '0.1'` from `optware_lifecycle_lock.sh`. The script polls for the nonblocking lock result at 100 ms intervals, but the firmware-matched BusyBox 1.24.2 sleep applet accepts integer intervals. The POSIX shell version of the same test passed earlier because it used the host sleep implementation; the matching BusyBox regression correctly exposed the difference.
- Fix in THIS COMMIT: use `sleep 1` and reduce the retry ceiling from 50 to 5, keeping the wait bounded at about four seconds while remaining compatible with the actual firmware applet. No production lifecycle code or tracked BusyBox config changed.
- Validation boundary: L1 compatibility diagnosis PASS. On the base SHA, BusyBox 1.24.2 build/link, direct applet smoke, syntax checks and the three preceding POSIX-shell regressions passed. The under-ash lock test reached the polling loop but did not finish; teardown/startup under ash and storage steps 10–13 were skipped. Exact-head storage/docs reruns for THIS COMMIT are pending. No local checkout, target MIPS/uClibc build, full firmware/TRX or L4 router test is claimed.
- Changed paths: `trunk/user/scripts/tests/optware_lifecycle_lock.sh`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root dev_log.md had 115 complete newest-first records before this entry and has 116 afterward; archives remain dev-log=6, handoff=22, roadmap=18. No archive/index rotation is due.
- Resulting commit: THIS COMMIT.
- Next: inspect the exact resulting main SHA's storage, infosvr and AI docs Actions. Require all under-ash Optware lifecycle regressions and storage steps 10–13 to pass; keep production MIPS/uClibc, firmware/TRX and L4 NOT VERIFIED.

---

## 2026-11-11 22:43 CST (UTC+8) — Remove optional ash command-builtin dependency from Optware lock regression

- Base main SHA: `abff1ed57217c1fe482bf80c2c419dde66437bf0`; parent `c4afed88518d2c4a9dc8c4e4dc2d0f2f129996f4`; remote branches: `main` only, no checkpoint.
- Exact-head Actions reviewed on the base: udhcpc [43040638862] SUCCESS; infosvr [43040638851] SUCCESS; AI handoff docs [43040638793] SUCCESS; storage [43040639192] FAILURE. The udhcpc GNU89 ASan/UBSan binary and corrected source contract passed; infosvr packet/SIGTERM sanitizer binaries and source contract passed; docs validator reported root log=114/119 and archive inventory dev-log=6, handoff=22, roadmap=18.
- Storage steps 3–8 passed. Step 9 successfully built/linked BusyBox 1.24.2; direct ash and awk calls, descriptor-backed `flock -n 9` / `flock -u 9` smoke, and all production-script ash syntax checks passed. The first lock regression then failed at its preflight `command -v flock`.
- Root cause: the exact firmware-derived BusyBox config disables `CONFIG_ASH_CMDCMD`; the oldconfig transcript confirms the optional ash `command` builtin was left unset. The regression harness incorrectly depended on `command -v`, even though the actual lock helper and the preceding binary smoke exercise the real flock applet. No production lifecycle function was implicated.
- Fix in THIS COMMIT: remove only that optional-builtin guard from `optware_lifecycle_lock.sh`. The real lifecycle lock acquisition remains the fail-closed test; do not enable CONFIG_ASH_CMDCMD just for the host test and do not change firmware config.
- Validation boundary: L1 source/config diagnosis PASS. On the base SHA the BusyBox host build/link, direct ash/awk/flock smoke and shell syntax checks passed. The ash lock/teardown/startup functional regressions did not complete: the first stopped at the optional command-builtin guard, and later ash tests plus storage steps 10–13 were skipped. This commit's exact-head storage/docs Actions are pending. No local repository checkout was available; no local rerun, production MIPS/uClibc build, full firmware/TRX or L4 router test is claimed.
- Changed paths: `trunk/user/scripts/tests/optware_lifecycle_lock.sh`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root dev_log.md had 114 complete newest-first records before this entry and has 115 afterward; archives remain dev-log=6, handoff=22, roadmap=18. No archive/index rotation is due.
- Resulting commit: THIS COMMIT.
- Next: reconcile storage and docs Actions on the exact resulting main SHA. Require all three Optware lifecycle regressions under BusyBox ash and storage steps 10–13 to pass; keep target MIPS/uClibc, full firmware/TRX, real router behavior and L4 NOT VERIFIED.

---

## 2026-11-11 22:34 CST (UTC+8) — Make BusyBox 1.24.2 applet validation behavioral and repair the DHCP source contract

- Base main SHA: `c4afed88518d2c4a9dc8c4e4dc2d0f2f129996f4`; parent `de95a41f5c95bf6543c78cd05a4ca3228e4af7a8`; remote branch inventory: `main` only, no checkpoint.
- Exact-head Actions reviewed on the base: storage [43036593168] FAILURE; udhcpc [43036593353] FAILURE; AI handoff docs [43036593178] SUCCESS; infosvr packet/signal [43036593109] SUCCESS. Every job's steps and complete logs were inspected.
- Storage steps 3–8 passed the NFS/Optware syntax checks and existing lock, teardown and startup POSIX-shell regressions. Step 9 successfully configured, compiled and linked BusyBox 1.24.2 after the preceding DHCP helper spelling and host-glibc `stime()` problems were corrected. It then failed at the `busybox --list` applet-presence assertion for `flock`, before the regressions ran under the built ash. The firmware config contains `CONFIG_FLOCK=y`, the disposable host config verifies it, and the build log compiled `util-linux/flock.o`.
- Correction in THIS COMMIT: replace the text-list probe with direct ash and awk execution plus an actual descriptor-backed BusyBox flock acquire/release smoke test. Preserve CONFIG_FLOCK=y and all firmware applet selection; do not relax the requirement.
- The udhcpc GNU89 ASan/UBSan policy binary passed. Its Python checker then failed because it searched for `udhcp_script_failure_prepare_retry`; the real static-inline declaration and production call are `udhcpc_script_failure_prepare_retry`. THIS COMMIT corrects only the stale source-contract spelling; no DHCP runtime implementation changes.
- AI docs validated root log=113/119 with archives dev-log=6, handoff=22, roadmap=18. Infosvr packet initialization, SIGTERM deferral sanitizer tests and source contract all passed on the same base SHA.
- Validation boundary: L1 diagnosis/source review PASS. No local checkout was available, so local tests and the whole-repository docs validator were NOT RUN. The BusyBox host build/link passed in Actions, but none of the Optware regressions under the built ash ran on the base SHA because the applet gate stopped the step. New exact-head storage, udhcpc and docs Actions are pending. This is not a production MIPS/uClibc build, full firmware/TRX or L4 router test.
- Changed paths: `.github/workflows/storage-hotplug-stability.yml`, `scripts/check_udhcpc_script_status.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root dev_log.md had 113 complete newest-first records before this entry and has 114 afterward; archive inventory remains dev-log=6, handoff=22, roadmap=18. No archive/index rotation is due.
- Resulting commit: THIS COMMIT.
- Next: inspect the exact resulting main SHA's storage, udhcpc and AI docs Actions, every step and full logs. Require the BusyBox 1.24.2 binary and applet smoke tests, all three Optware lifecycle regressions under ash, storage steps 10–13, the udhcpc sanitizer/source contracts and docs validator to pass. Keep production MIPS/uClibc, firmware/TRX, real router ash/runtime and L4 NOT VERIFIED.

---

## 2026-11-11 (UTC+8) — Repair the BusyBox host ash build exposed by Optware lifecycle validation

- Base main SHA: `de95a41f5c95bf6543c78cd05a4ca3228e4af7a8`; parent `2c769eac12cfdcb597644ebcf9c65b51423c8e4d`; remote branches: main only, no checkpoint.
- Exact-head Actions on the base: storage [43028546205] FAILURE, AI handoff docs [43028546042] SUCCESS, infosvr lifecycle [43028546103] SUCCESS. Storage steps 3–8 passed (script syntax plus lock, teardown and startup shell regressions); step 9 failed while linking BusyBox 1.24.2. The complete log showed an undefined reference caused by a misspelled `dhcpc.c` call to `udhcp_script_failure_prepare_retry()`; the header's actual static-inline API is `udhcpc_script_failure_prepare_retry()`.
- The same host build also linked BusyBox's legacy DATE applet, whose `stime()` reference is not exported by current hosted glibc. This is a host-toolchain mismatch: target firmware uses its own uClibc and deliberately enables DATE. No firmware config change is appropriate for an unrelated ash regression.
- THIS COMMIT fixes the DHCP helper call spelling and adds a source-contract check that the caller includes and uses the declared policy helper. The storage workflow derives a disposable host `.config` from the tracked board config and disables only CONFIG_DATE before `oldconfig`; it asserts DATE stays disabled in that host copy and separately asserts the tracked board config retains CONFIG_DATE=y. Firmware configuration and applet selection are unchanged; ash/awk/flock and all lifecycle-required applets remain required.
- Validation boundary: L1 diagnosis and source/config review PASS. No local repository checkout was available, so local host compilation and `scripts/check_ai_handoff_docs.py` were NOT RUN. On the base SHA, the pre-BusyBox shell regressions passed, but the BusyBox link failed as above. Exact-head storage and AI docs Actions for THIS COMMIT are pending. This is not a production MIPS/uClibc build, complete firmware/TRX or L4 router test.
- Root dev_log.md had 112 complete newest-first records before this entry and has 113 afterward; the existing archive inventory remains dev-log=6, handoff=22, roadmap=18. No archive/index rotation is due.
- Changed paths: `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c`, `.github/workflows/storage-hotplug-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Resulting commit: THIS COMMIT.
- Next: inspect the exact resulting main SHA's storage and AI docs Actions, every job step and complete logs. Require the BusyBox link, required applet checks, ash syntax checks and all three Optware lifecycle regressions to pass. Keep target MIPS/uClibc, firmware/TRX, actual router ash/runtime and L4 NOT VERIFIED.

---

## 2026-11-11 (UTC+8) — Exercise Optware lifecycle with firmware BusyBox ash 1.24.2

- Base main SHA: \`2c769eac12cfdcb597644ebcf9c65b51423c8e4d\`; parent \`4ec9d637ca7daaff93296c494467b6e88c528311\`; branch inventory: main only, no remote checkpoint.
- Closed the previous GPT's active ASUS infosvr SIGTERM task at exact-head L2. Infosvr packet/signal workflow [43024714802] and AI handoff docs [43024714674] both completed SUCCESS on exact SHA \`2c769eac12cfdcb597644ebcf9c65b51423c8e4d\`; all job steps and complete logs were inspected. GNU89 ASan/UBSan packet and signal tests plus the UDP/SIGTERM source contract printed PASS; docs validator reported 111/119 records and archives 6/22/18. Production MIPS/uClibc, firmware/TRX and L4 signal behavior remain NOT VERIFIED.
- Investigated the separately tracked FTP failed-upload cleanup TOCTOU. On Linux 4.4, \`fstatat()\` followed by \`unlinkat()\` cannot atomically bind the unlink to the previously checked inode. \`renameat2()\` exchange/no-replace operations are not conditional inode comparisons; staging would require a wider upload-publication redesign and has unresolved STOR/APPE/REST visibility/overwrite semantics. No unsafe patch was made; the issue remains P2 OPEN/BLOCKED pending a proved compatible design.
- Selected the next evidence-actionable storage item: prior Optware lock/teardown/startup regressions ran under host \`/bin/sh\`, and their child fixture shebangs were hard-coded. That did not exercise the exact firmware BusyBox shell and applet implementation. Firmware selects BusyBox 1.24.2 and enables ash, awk and flock in the common config.
- THIS COMMIT makes the lifecycle teardown/startup fixture writer honor \`OPTWARE_TEST_SHELL\` (default \`/bin/sh\`) and updates the storage workflow to host-build the exact BusyBox 1.24.2 source using the firmware common config. CI verifies ash/awk/flock and required lifecycle applets, syntax-checks Optware production scripts with ash, then runs lock, teardown and startup regressions under that binary. Child mock hooks use the same ash shebang; BusyBox awk/flock resolve through applet symlinks. Path filters cover the firmware BusyBox config, configure selector, package Makefile and pinned source tree for both push and pull_request.
- Evidence boundary: source/build selection is L1 reviewed; exact BusyBox 1.24.2 host build and ash L2 execution are pending the resulting exact-head Actions. This is not a production MIPS/uClibc binary, complete firmware/TRX build, target kernel test or L4 router result.
- Root dev_log.md had 111 complete newest-first records before this entry and has 112 afterward. At 290,184 bytes before the entry, it remains below the 384 KiB rotation target and 512 KiB hard cap; archive inventory remains dev-log=6, handoff=22, roadmap=18. No archive or index update is due.
- Changed paths: \`trunk/user/scripts/tests/optware_lifecycle_teardown.sh\`, \`trunk/user/scripts/tests/optware_lifecycle_startup.sh\`, \`.github/workflows/storage-hotplug-stability.yml\`, \`SESSION_HANDOFF.md\`, \`REFACTOR_ROADMAP.md\` and this log.
- Resulting commit: THIS COMMIT.
- Next: fetch the resulting main SHA; inspect the exact-head storage and AI docs workflows, every job step and full log. Require the BusyBox 1.24.2 host build, applet checks, ash syntax checks and all three lifecycle regressions to pass. Keep MIPS/uClibc, complete firmware/TRX and L4 NOT VERIFIED.

---

## 2026-11-11 20:21 CST (UTC+8) — Defer infosvr SIGTERM cleanup out of signal context

- Base main SHA: 4ec9d637ca7daaff93296c494467b6e88c528311; parent 14e22191f48c9ad6f3804729ab502ac81670a30b; branch inventory: main only, no remote checkpoint.
- Resumed the previous handoff's Optware/storage investigation. The corrected exact-head storage [43017662766], infosvr packet [43017662767] and AI docs [43017662804] runs all completed SUCCESS on 4ec9d637ca7daaff93296c494467b6e88c528311; every step and complete log was inspected. The storage swapoff regression and full caller-graph/lock/failure contract now pass. The target source is BusyBox 1.24.2 with ash selected and required applets configured, but the matching executable is not available here; host BusyBox 1.37.0 is not target ash evidence. Storage is closed at L1/L2 and explicitly blocked at target-runtime level.
- After closing that bounded source/L2 slice, rotated to the separate P2 infosvr daemon lifecycle risk. Finding: catch_sig_infosvr(SIGTERM) called res_cleanup() (close/remove) and exit() in signal context. The old main loop used an unbounded select() and read_packet() retried every EINTR, so a flag-only handler without bounded waits would retain a check-to-block shutdown race.
- THIS COMMIT makes the handler a single sig_atomic_t stop-flag store, installs SIGTERM through checked sigaction() with SA_RESTART disabled, adds one-second select(), SO_RCVTIMEO and SO_SNDTIMEO bounds, stops receive/send EINTR retries after shutdown, and performs socket/PID cleanup only after the main loop observes the flag. The descriptor cleanup predicate now correctly accepts fd 0.
- Regression: add a GNU89 ASan/UBSan policy test for the stop flag/coalescing and timeout constants; extend the infosvr source checker to enforce handler purity, sigaction/no-restart ordering, bounded waits, stop observation and main-context cleanup; update the existing path-filtered workflow for push and pull_request.
- Validation boundary: L1 source review PASS. The isolated signal policy test passed locally with GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined and leak detection. The updated source-contract script and exact-head infosvr/AI docs Actions are pending for the resulting SHA. Target MIPS/uClibc, actual BusyBox ash 1.24.2, complete firmware/TRX and L4 signal timing remain NOT VERIFIED.
- Root dev_log.md had 110 complete newest-first records before this entry and has 111 afterward. Below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap; archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index update is due.
- Changed paths: trunk/user/infosvr/src/infosvr.c, new signal policy header/test, scripts/check_infosvr_packets.py, .github/workflows/infosvr-packet-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md.
- Resulting commit: THIS COMMIT.
- Next: fetch resulting main SHA; inspect every infosvr and AI docs job step/full log; require the signal policy sanitizer regression and all signal/source contracts to pass before closing L2 and rotating to the next global governance item.

---

## 2026-11-11 20:08 CST (UTC+8) — Correct the storage caller-graph swapoff assertion

- Base main SHA: 14e22191f48c9ad6f3804729ab502ac81670a30b; parent 2968bb70c97e13f405389795628184f804abcf5d; branch inventory remains main only.
- Exact-head Actions: storage run [43017000972] completed FAILURE in the final “Verify whole storage caller graph, operation locks and failure propagation” step. The Optware teardown/helper syntax step and the entire Optware teardown regression step both PASSED. The failure was a stale source assertion at Python line 227 that still searched for the removed literal `swapoff /opt/.swap`; the implementation now calls `optware_lifecycle_swapoff_file /opt/.swap`. The traceback was `ValueError: substring not found`, not a product regression failure.
- Infosvr [43017000995] and AI handoff docs [43017000981] both completed SUCCESS on exact SHA 14e22191f48c9ad6f3804729ab502ac81670a30b; all steps passed, including the GNU89 ASan/UBSan infosvr policy test and documentation archive validator.
- THIS COMMIT updates the older whole-caller source contract to locate the explicit helper invocation, retaining the required ordering after service-stop failure handling and before the /opt unmount gate. No production source or shell regression behavior changes in this correction.
- Validation boundary: L1 source review unchanged; new Optware syntax and teardown host regression steps passed on 14e22191f48c9ad6f3804729ab502ac81670a30b. Matching target BusyBox ash, MIPS/uClibc objects, complete firmware/TRX and L4 hardware remain NOT VERIFIED. Corrected exact-head storage/docs Actions pending.
- Root dev_log.md had 109 complete records before this entry and has 110 afterward. Below rotation thresholds; archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index change is due.
- Changed paths: .github/workflows/storage-hotplug-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md.
- Resulting commit: THIS COMMIT.
- Next: inspect corrected exact-head storage and AI docs Actions, including full logs and every step. If green, continue matching BusyBox ash and lifecycle lock/fd/caller audit rather than claiming L3/L4 completion.

---

## 2026-11-11 19:24 CST (UTC+8) — Fail closed when Optware swap release is uncertain

- Base main SHA: 2968bb70c97e13f405389795628184f804abcf5d; parent d2edf2dd08c85e0413f6469e2dccc6fa5a2df26a; branch inventory: main only, no remote checkpoint. Storage [43009772930] and AI docs [43009772871] both passed on this exact base SHA.
- Recovered the active Optware/storage lifecycle task and traced the installed firmware path: the Optware ROMFS Makefile installs opt-umount.sh, while the scripts package publishes the shared lifecycle helper consumed at /sbin/optware_lifecycle_lock.sh.
- Finding: opt-umount.sh ignored swapoff /opt/.swap failure and continued with /home/admin and /etc/localtime cleanup plus /opt unmount. File existence was used as a proxy for activation, and no postcondition proved the swap entry had disappeared.
- THIS COMMIT adds an exact-entry /proc/swaps reader to the shared lifecycle helper. It validates the header and complete records, including read/close errors; distinguishes active, inactive and unknown inventory; attempts swapoff only for the exact active /opt/.swap path; requires command success and re-reads /proc/swaps to confirm that the entry disappeared. Unknown/malformed inventory, swapoff failure or a still-active entry now blocks home/localtime cleanup and /opt unmount. Awk, swapoff and logger child processes close lifecycle fd 9.
- Regression: extend optware_lifecycle_teardown.sh with inactive, prefix-only, successful deactivation, command-failure, still-active, missing-inventory, malformed-inventory and lock-fd inheritance cases. The storage workflow adds syntax checks and a semantic source-order contract for the early failure gate.
- Validation: L1 source review PASS. Host shell regression and exact-head storage/AI docs Actions are pending for THIS COMMIT. The regression uses host sh; matching target BusyBox ash, production MIPS/uClibc, complete firmware/TRX and L4 hardware remain NOT VERIFIED.
- Root dev_log.md: 108 complete records before this entry; 109 afterward. Below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap; archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index update is due.
- Changed paths: trunk/user/scripts/files/sbin/optware_lifecycle_lock.sh, trunk/user/optware/opt-umount.sh, trunk/user/scripts/tests/optware_lifecycle_teardown.sh, .github/workflows/storage-hotplug-stability.yml, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md.
- Resulting commit: THIS COMMIT.
- Next: inspect the exact-head storage and AI handoff workflow jobs/full logs; require all swap inventory regression cases and the unmount-order contract to pass. Then locate the matching firmware BusyBox ash implementation/runtime for the remaining lock and descriptor-inheritance audit.

---

## 2026-11-11 18:40 CST (UTC+8) — Gate storage removal on every mounted-media daemon stop result

- Base main SHA: f986ed961bbe14a8db4bc8d99219e5a1a1330826; parent 17b4583ce0511a3cbbd02cef18024fe7b30110e7; branch inventory: main only, no remote checkpoint. Exact-base AI docs [43004194407] and infosvr packet stability [43004194313] both SUCCESS; complete logs inspected.
- Traced the installed firmware path rather than treating K95 hooks as live callers. trunk/user/optware/Makefile installs opt-start/opt-mount/opt-umount but not K95aria2/K95transmission; scripts/Makefile publishes the shared /sbin lifecycle helper. Built-in Aria/Transmission use separate RC scripts and services_stor.c callers.
- Found that stop_stor_apps() propagated NFS/Samba/FTP failures but ignored MiniDLNA, Firefly, Transmission and Aria stop outcomes. The four stop wrappers discarded kill_services()/eval() status and did not confirm process absence, so safe removal could continue with a storage-backed daemon still observable.
- Changed each stop wrapper to return failure unless both the stop command succeeds and the daemon-absence postcondition holds. stop_stor_apps() now blocks removal on all four failures; manual restart callers also refuse to start over a daemon that failed to stop. Added a reusable GNU89 policy helper and a five-case regression.
- Host test: GNU89 -Wall -Wextra -Werror ASan/UBSan policy binary printed 'storage daemon stop postcondition policy: PASS'. Exact-head storage Actions are pending for THIS COMMIT; no target BusyBox ash, MIPS/uClibc, firmware/TRX or L4 result is claimed.
- Remaining next slice: opt-umount.sh ignores swapoff /opt/.swap failure before home/localtime cleanup and /opt unmount; add active/inactive /proc/swaps and swapoff-failure coverage, then continue matching-shell lock verification.
- Root dev_log.md: 107 complete newest-first records before this entry; 108 afterward. Below 120-record / 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index update is due.
- Changed paths: .github/workflows/storage-hotplug-stability.yml, trunk/user/rc/src/services_stor.c, trunk/user/rc/src/rc.h, trunk/user/rc/src/storage_service_stop_policy.h, trunk/user/rc/src/tests/storage_service_stop_policy.c, SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and dev_log.md.
- Resulting commit: THIS COMMIT.

---
## 2026-11-11 18:18 CST (UTC+8) — Close udhcpc callback gate and hand off Optware lifecycle audit

- Base main SHA: 17b4583ce0511a3cbbd02cef18024fe7b30110e7; parent a3ee4d0e628434b1e16171f457afa0791c84f7a3; branch inventory: main only, no remote checkpoint.
- Closed the DHCP callback-status fix at L2 on its exact code SHA. Dedicated udhcpc workflow [43002980464], path-triggered infosvr packet regression [43002980467] and AI handoff documentation validation [43002980526] all completed SUCCESS on 17b4583ce0511a3cbbd02cef18024fe7b30110e7. Every job/step and complete log was inspected.
- The committed udhcpc GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer test printed "udhcpc script failure retry policy: PASS"; source checker printed "udhcpc event-script status and lease-state source contracts: PASS". AI docs validator printed "AI handoff documentation check: PASS (root log=106/119 entries, batch=100, retain=20, legacy/current archives: dev-log=6, handoff=22, roadmap=18)".
- L2 closure applies only to the bound/renew DHCPACK callback gate and retry/deadline policy. Production MIPS/uClibc BusyBox object, full firmware/TRX, live v4/v6 DHCP transitions and router behavior remain NOT VERIFIED. Other event-state semantics are still open.
- Selected the next cross-repository P2 investigation: Optware/storage lifecycle under BusyBox ash. Prior review notes K95 hooks are not ROMFS-installed/invoked by the in-tree S?? dispatcher, while built-in Aria2/Transmission use separate RC scripts. The next pass must prove active installation/call paths before treating K95 code as production behavior.
- Next task: inventory actual hook/RC/hotplug entrypoints; trace operation-lock ownership and descriptor lifetime, Optware startup/rollback/stop status, swapoff and /opt unmount ordering; add focused host regressions for the active path. Use BusyBox ash only when the shell implementation being tested is available and matched; do not substitute host POSIX shell claims for target ash evidence.
- Root dev_log.md: 106 complete records before this entry; 107 afterward. Below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap. Archive inventory remains dev-log=6, handoff=22, roadmap=18; no archive/index changes are due.
- Changed paths: SESSION_HANDOFF.md, REFACTOR_ROADMAP.md and this log only. This is a state-transition closeout and next-task handoff after exact-head verification, not a code-verification-only note.
- Resulting commit: THIS COMMIT.

---

## 2026-11-11 18:08 CST (UTC+8) — Preserve udhcpc event-script status and gate DHCP lease publication

- Base main SHA: a3ee4d0e628434b1e16171f457afa0791c84f7a3; parent 65119b857a7684974caf2ae0e13d8112a0e035bd; branch inventory: main only, no remote checkpoint.
- Closed the previous BusyBox crond detached-daemon lifecycle task on exact SHA a3ee4d0e628434b1e16171f457afa0791c84f7a3: Crond workflow [42999257797] and AI docs [42999257844] both SUCCESS. All 25 Actions runs on that SHA completed successfully; every job step and the relevant full logs were inspected. Crond GNU89 ASan/UBSan regression and RC/BusyBox source contracts passed.
- Selected the LAN DHCP client callback-status issue from the cross-repository governance ledger. The fix is in BusyBox udhcpc rather than the previously touched SkipDBv2, HTTPD or RC-only code.
- Root cause: BusyBox `spawn_and_wait()` returns `wait4pid()` status, but `udhcp_run_script()` discarded it. On DHCPACK, udhcpc then advanced to BOUND, published the new requested address and refreshed the lease timer even if the RC `bound`/`renew` callback returned an error while applying the address/routes/resolver.
- THIS COMMIT changes the event runner to return and log its script result. DHCPACK processing now gates address/BOUND/renewal-timer publication on callback success. A failed initial bound callback retries the same selected request after the configured retry interval. A failed renewal preserves the pre-existing lease deadline, charges callback runtime against that deadline and lets the normal expiry path run if the callback consumed the remaining lease.
- Added `udhcpc_script_policy.h` plus a five-case GNU89 host test for initial retry, renewal deadline preservation, callback overrun, exact expiry and an already expired lease. Added a source/caller checker confirming the actual `spawn_and_wait()` contract, ACK gate order, policy integration and symmetric path filters; added dedicated path-filtered Actions workflow.
- Local host policy reproduction: GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`, leak detection enabled; PASS for all five retry/deadline cases. The committed repository harness and source checker remain pending exact-head Actions.
- Changed paths: BusyBox `dhcpc.c`, new DHCP retry policy header/test, new source checker and workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log. No archive/index change is due.
- Root `dev_log.md`: 105 complete records before this entry; 106 afterward. The file remains below the 120-record / 384 KiB rotation triggers and the 512 KiB hard cap; archive inventory remains 6/22/18.
- Resulting commit: THIS COMMIT.
- Next: inspect all exact-head jobs and full logs for the dedicated udhcpc event-script status regression and AI handoff docs; also inspect any triggered WAN/HTTPD umbrellas. Require the sanitizer policy test, actual BusyBox script-result and DHCPACK source contracts, and docs validator to pass on this exact SHA. Production MIPS/uClibc BusyBox object, firmware/TRX, DHCPv6 and L4 remain NOT VERIFIED.

---

## 2026-11-11 17:55 CST (UTC+8) — Gate crond lifecycle on detached process readiness and confirmed stop

- Base main SHA: 65119b857a7684974caf2ae0e13d8112a0e035bd; parent 947ee69cfbda917ec1728cdde39a00b6a7726eeb; branch inventory: main only, no remote checkpoint.
- Closed the previous infosvr packet parsing task on exact SHA 65119b857a7684974caf2ae0e13d8112a0e035bd: packet workflow [42997959662] and AI handoff docs [42997959722] both SUCCESS. All steps and complete logs inspected; GNU89 ASan/UBSan packet policy test, UDP/source-boundary contract and docs archive/heading validator passed. Root log=104/119; archive counts 6/22/18.
- Selected BusyBox crond as the next P2 reliability issue from the broader RC/userland lifecycle inventory.
- Root cause: RC _eval() forks and waits for the BusyBox launcher process, but crond_main() daemonizes before child xchdir(), crontab rescan and pidfile publication. The launcher parent exits successfully immediately after forking, so RC could report success even if the detached scheduler failed during initialization.
- A second lifecycle defect discarded kill_services() status in stop_crond(); restart_crond() then launched another scheduler unconditionally. If the previous process remained after bounded TERM/KILL, duplicate cron jobs could run.
- Fix in THIS COMMIT: return the shared stop result, gate restart on confirmed process exit, and add an initial grace plus bounded checks for a live non-zombie pids("crond") process after the daemonizing parent exits. Preserve disabled-service return value 1. Propagate negative restart failures from RC notification dispatch so the in-flight completion marker remains for retry; log failure after initial crontab regeneration.
- Host regression: GNU89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer, run with leak detection; PASS, output crond daemon startup and restart policy: PASS. New source checker covers BusyBox fork/parent/child ordering, pid matching/zombie exclusion, lifecycle return propagation, all RC call sites and symmetric workflow filters.
- Changed paths: new crond workflow, policy header and test, source checker, services.c, rc.c, rc.h, this handoff, roadmap and this log. No archive/index rotation is due.
- Root dev_log.md: 104 complete records before this entry; 105 afterward, below the 120-entry/384 KiB rotation triggers and 512 KiB hard cap. Archives remain 6/22/18.
- Resulting commit: THIS COMMIT.
- Next: inspect all exact-head jobs/full logs for Crond lifecycle and AI docs, plus any path-triggered RC umbrellas. Require GNU89 sanitizer regression, source/caller contract and docs validator to pass on the same SHA. Production MIPS/uClibc, full firmware/TRX, live cron execution and L4 remain NOT VERIFIED.

---

## 2026-11-11 17:38 CST (UTC+8) — Restore required verification-boundary heading and guard handoff contract

- Base main SHA: `947ee69cfbda917ec1728cdde39a00b6a7726eeb`; parent `8ec8198c4564e57cd3956ea675e343bc35098e72`.
- Infosvr code/source-contract workflow [42997353568] completed SUCCESS on exact base SHA `947ee69cfbda917ec1728cdde39a00b6a7726eeb`. Full logs confirmed `infosvr packet initialization and length policy: PASS` and `infosvr packet receive and bounds source contracts: PASS`.
- AI docs run [42997352476] failed on that same base because the live handoff lacked the validator-mandated `## Verification boundary` heading. This was a documentation-structure defect, not a C regression or production failure.
- THIS COMMIT restores the required section with explicit local/CI/target evidence boundaries, updates the roadmap and next-step state, and adds checker assertions for all four mandatory handoff headings. The infosvr workflow now path-filters `SESSION_HANDOFF.md` on push and pull_request because the source contract reads it.
- No application source changed in this correction; the prior commit contains the packet fix and local test. This correction repairs the mandatory validator contract and makes that contract executable in the infosvr checker. Exact-head reruns remain pending; do not describe [42997352476] as green.
- Root `dev_log.md`: 103 records before this entry; 104 afterward. Below the 120-record / 384 KiB rotation triggers and 512 KiB hard cap; archive shards/indexes remain untouched.
- Resulting commit: THIS COMMIT.
- Next: inspect both infosvr and AI docs workflows, every step and full logs, on the exact resulting main SHA. Require sanitizer test, packet source/ABI checks, required handoff sections and archive validator to pass; then select another user-space service. MIPS/uClibc, firmware/TRX and L4 remain NOT VERIFIED.

---

## 2026-11-11 17:31 CST (UTC+8) — Harden infosvr UDP receive-buffer initialization and wire-prefix validation

- Base main SHA: `8ec8198c4564e57cd3956ea675e343bc35098e72`; parent `a73ca2221712f16dd5aa4bab450eed18a6cd3f92`; branch inventory was `main` only, no checkpoint.
- Closed networkmap signal-context work on exact base SHA `8ec8198c4564e57cd3956ea675e343bc35098e72`: workflow [42996232222] and AI docs [42996232115] both SUCCESS. Complete logs inspected; GNU89 ASan/UBSan test and signal/lifecycle/handler-registration source contract passed; docs reported root log=102/119 and archive inventory 6/22/18. The earlier [42995791241] docs failure on parent SHA was the missing heading, corrected in the current base and rerun green.
- New P2 reliability/security finding in the independent ASUS `infosvr` discovery daemon. `read_packet()` used `sizeof(*packet)` where `packet` is `void *`; GNU89's `sizeof(void)` extension is one byte, confirmed with a host compiler. Thus only the first byte of the 512-byte stack packet buffer was cleared. Short UDP datagrams left the remaining bytes indeterminate. In addition, `read_packet()` returned a boolean `0/1`, while main tested it for `< 0`, so incomplete datagrams continued into `process_packet()`. The parser then dereferenced base/extended wire fields without the actual received size.
- Fix in THIS COMMIT: zero the complete supplied receive capacity; return actual `recvfrom()` byte count/error; reject packets shorter than the base IBOX header; pass actual received length into `process_packet()`; validate the extended prefix up to `offsetof(IBOX_COMM_PKT_HDR_EX, Password)` before reading MAC; and fail closed when the NVRAM hardware address is missing/invalid rather than comparing an uninitialized local buffer.
- Regression: new GNU89 test covers all-byte zeroing including unused tail, short-packet deterministic padding, NULL/zero-size errors and base/extended prefix boundaries. New source checker verifies helper integration, actual-byte propagation, base/extended guards, safe MAC conversion and symmetric workflow push/PR filters.
- Local compile/run: GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`; ASan leak detection enabled. Output: `infosvr packet initialization and length policy: PASS`.
- Exact-head infosvr workflow and AI docs are pending. No production MIPS/uClibc infosvr object, complete firmware/TRX or L4 packet interoperability test is claimed. Separate follow-up: infosvr SIGTERM handler still does cleanup/exit in signal context; this packet parsing change intentionally does not combine that lifecycle fix.
- Changed paths: `infosvr.c`, `asus.c`, new packet policy header/test, new source checker/workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root `dev_log.md`: 102 complete entries before this record; 103 afterward. Below 120-entry / 384 KiB rotation triggers and 512 KiB hard limit; no archive or index update is due.
- Resulting commit: THIS COMMIT.
- Next: verify the exact resulting main SHA and all job steps/full logs for infosvr packet regression and AI docs. Require sanitizer test, receive/parser/ABI contract and docs validator to pass on that exact SHA; then select another under-covered user-space module.

---

## 2026-11-11 17:21 CST (UTC+8) — Repair handoff contract and extend networkmap signal lifecycle coverage

- Base main SHA: `a73ca2221712f16dd5aa4bab450eed18a6cd3f92`; parent `ce7efad34dbd81097d923f9568f0ae9946694b58`.
- Closed the previous NVRAM HDR2 atomic-save task before opening this slice: exact-head HTTPD security [42992756073] and AI docs [42992755926] both SUCCESS on `ce7efad34dbd81097d923f9568f0ae9946694b58`. All 39 HTTPD steps passed, including the new profile atomic writer, original NVRAM profile/CLI policy tests, RC restore durability and HTTPD private export caller contract. Docs validator reported 100/119 records and archive counts 6/22/18.
- Networkmap signal fix from parent SHA `a73ca2221712f16dd5aa4bab450eed18a6cd3f92` passed its exact-head dedicated workflow [42995791190]: GNU89 `-Wall -Wextra -Werror` ASan/UBSan test printed `networkmap signal deferral policy: PASS`; source/lifecycle/build contract printed `networkmap signal deferral and lifecycle source contracts: PASS`.
- AI docs run [42995791241] on that same parent SHA failed at its first section assertion: the consolidated `SESSION_HANDOFF.md` had renamed the exact required heading `## Current repository state`. This is a handoff-validator contract failure, not a production or test failure. THIS COMMIT restores all exact required headings (`Current repository state`, `Active task`, `Verification boundary`, `Exact first action for the next GPT`) and keeps the checkpoint text intact.
- Additional checker coverage asserts SIGUSR1/SIGTERM handlers are installed before daemonization and before the event loop begins; this guards the lifecycle boundary where process signal handling becomes active. The changed checker path also re-triggers the networkmap source/test workflow alongside the docs workflow.
- Changed paths: `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`, `scripts/check_networkmap_signals.py`.
- Root dev_log.md: 101 complete entries before this record; 102 after. Below 120/384 KiB rotation triggers and 512 KiB hard limit; no archive or INDEX update is due.
- Resulting commit: THIS COMMIT.
- Next: inspect every step and full log for the networkmap and AI docs workflows on the exact resulting main SHA. Require the existing signal policy test, expanded signal-install ordering source contract, handoff required sections and archive validator all PASS; only then close the networkmap slice. L3/L4 remain NOT VERIFIED.

---

## 2026-11-11 17:16 CST (UTC+8) — Defer networkmap daemon signal handling and cleanup to main loop

- Base main SHA: `ce7efad34dbd81097d923f9568f0ae9946694b58`; parent: `534e4e38ed5e729af845d7ad69bb84d8b96a5c47`. Only remote branch was `main`; no checkpoint branch.
- Closed the previously pending NVRAM profile-save task on exact head `ce7efad34dbd81097d923f9568f0ae9946694b58`: HTTPD Authentication/Security [42992756073] and AI handoff docs [42992755926] both completed SUCCESS. Every one of the 39 HTTPD job steps succeeded. Full logs confirm `NVRAM profile policy: PASS`, `NVRAM profile atomic publication: PASS`, `NVRAM CLI assignment policy: PASS`, RC NVRAM persistence checks, private HTTPD export path, and the rest of the selected HTTPD/storage/wireless contracts. Docs validation reported root log=100/119 and archives dev-log=6, handoff=22, roadmap=18.
- New P2 reliability finding in `trunk/user/networkmap/src/networkmap.c`: SIGTERM called `clear_resources()` and `exit(0)` from the handler. The cleanup closes the ARP socket, frees the client list, writes NVRAM and removes the PID file, risking re-entry into libc/allocator/NVRAM state. SIGUSR1 also directly changed `networkmap_fullscan` and `scan_now` while the main loop used them. `function.c` declares `extern unsigned int scan_now`, so the previous volatile definition was inconsistent.
- Fix in THIS COMMIT: both handlers now expand a macro to direct `volatile sig_atomic_t` flag assignments only; no helper function or cleanup routine runs in signal context. The main loop temporarily blocks SIGUSR1 while snapshotting/clearing the refresh flag and restores the caller's previous mask, so a new pending signal is delivered on the next pass. Refresh flags/counters and all resource cleanup are applied only in normal process context. `scan_now` declaration now matches its non-volatile extern.
- Regression: new GNU89 ASan/UBSan test covers refresh/exit mapping, coalescing, ignored signals, exactly-once consumption, already-blocked mask preservation, pending SIGUSR1 delivery and NULL input. New source checker verifies handler purity, mask/snapshot/restore order, receive timeout, main-context cleanup, networkmap build/package wiring, RC lifecycle and symmetric push/PR path filters.
- Local test rerun using the final macro-based header: `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer`; run with `ASAN_OPTIONS=detect_leaks=1`: `networkmap signal deferral policy: PASS`.
- Exact-head networkmap and AI docs workflows are pending; do not claim them before inspecting their complete logs. No full networkmap production object, MIPS/uClibc image, firmware/TRX or router test is claimed.
- Changed paths: `trunk/user/networkmap/src/networkmap.c`, new policy header/test, new source checker/workflow, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and this log.
- Root `dev_log.md`: 100 complete records before this entry; 101 after. Below 120-record / 384 KiB rotation triggers and 512 KiB hard cap; no archive or index update is due.
- Resulting commit: THIS COMMIT.
- Next: inspect exact resulting main SHA and every job step/full log for the networkmap signal regression and AI docs. Require sanitizer test, signal/call-path/package/source contract and docs validator to pass on that same SHA; then rotate to an under-covered userland service.

---

