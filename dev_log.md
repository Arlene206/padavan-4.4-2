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

