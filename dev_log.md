## 2026-11-25 — Pin atomic DHCPv6 environment rollback in the source contract

- The BusyBox DHCPv6 callback retry regression on exact source SHA `898f67081429468f9e41f2603645b73112d71324` completed PASS (#45715277126). The cross-workflow WAN callback contract was then updated and completed PASS on `b839cd32fc594e3ac86a9c480232bda4746cda76` (#45715551138); AI handoff docs also passed on that SHA (#45715551148).
- Strengthened `scripts/check_dhcp6c_script_retry.py` to pin the publication helper's reverse-order rollback, original-errno preservation, and environment-vector cleanup, so future edits cannot silently weaken the new failure path.
- XY-C1 firmware build on source SHA `898f67081429468f9e41f2603645b73112d71324` remained in progress when polled; TRX verification and artifact upload had not started. No firmware completion is claimed.
- Changed paths: `scripts/check_dhcp6c_script_retry.py`, `dev_log.md`.

---

## 2026-11-25 — Align WAN callback contract with atomic DHCPv6 environment publication

- The exact DHCPv6 script-retry workflow for `898f67081429468f9e41f2603645b73112d71324` passed. The broader WAN DHCP callback workflow failed only because its independent inline source contract still required the removed unchecked `putenv(*curr++)` implementation.
- Updated the cross-workflow contract to require the atomic `dhcp6c_script_env_publish()` gate, reject the old unchecked publication pattern, and ensure `dhcp6_script()` refuses to spawn with a NULL environment. Added the retry policy/test files to this workflow's push and pull-request path filters so future changes re-run the cross-contract check.
- No WAN DHCP product behavior changed in this follow-up. Exact-head WAN callback and XY-C1 Actions pending; DHCPv6 retry L2 test already passed on the parent source commit.
- Changed paths: `.github/workflows/wan-dhcp-callback-stability.yml`, `dev_log.md`.

---

## 2026-11-25 — Make DHCPv6 callback environment publication fail atomically

- `fill_envp_client6()` previously ignored every `putenv()` result. Under memory pressure, one option variable could fail to enter the parent environment while the function still launched the configuration script and treated its exit status as success. A partial DNS/SIP/NTP option set could therefore be committed without retry.
- The builder now constructs the complete environment first, checks each publication, and removes already-published entries in reverse order if any publication fails. It frees the full temporary vector and returns NULL with the original error (or ENOMEM fallback); `dhcp6_script()` refuses to execute when environment construction failed, allowing the existing Reply retry path to retry the whole callback.
- Added GNU89 regression cases for mid-vector failure/rollback order, missing errno normalization and successful publication, plus exact-source integration assertions. Exact-head DHCPv6 retry and XY-C1 Actions pending at commit creation; no firmware or router runtime result is claimed.
- Changed paths: `dhcp6c_script.c`, `dhcp6c_script_retry_policy.h`, `tests/dhcp6c_script_retry_policy.c`, `scripts/check_dhcp6c_script_retry.py`.

---

## 2026-11-25 — Filter WAN IPv6 candidates through the public-GUA policy before preference

- Review found a mixed-address edge in the stable/temporary selector: Linux reports ULA and other reserved IPv6 addresses with `scope global`. Counting candidates before the public-GUA validation meant a single stable ULA could mask an otherwise usable public temporary GUA (or create false ambiguity alongside a public stable address).
- Changed discovery to collect global, non-tentative, non-deprecated candidates with stable/temporary tags, validate each candidate with the existing strict `aliddns_ipv6_public_valid()` policy, then prefer exactly one valid stable GUA or fall back to exactly one valid temporary GUA. Invalid ULA/reserved candidates no longer affect the valid-candidate cardinality.
- Added fixtures for ULA + public stable and ULA + public temporary address combinations; existing stable preference, temporary-only and ambiguity cases remain.
- Local AWK and shell regression checks are pending this commit; exact-head AliDDNS and XY-C1 workflows are required. No firmware or physical-router result is claimed.
- Changed paths: `trunk/user/aliddns/ddns_script.sh`, `trunk/user/aliddns/tests/aliddns_ipv6_lookup.sh`, `scripts/check_aliddns_api_transport.py`, `dev_log.md`.

---

## 2026-11-25 — Align DDNSTO tri-state source contract with procfs absence helper

- Exact DDNSTO run #45713996599 on `230cf171` passed all lifecycle shell regressions, including lock serialization, exact-PID TERM/KILL, process identity, and serialized restart. The final Python source checker then failed because it still required a literal `return 1` inside `ddnsto_process_state()`.
- Production correctly delegates the ABSENT proof to `ddnsto_confirm_absent()` and returns its status unchanged; this preserves UNKNOWN (2) instead of collapsing all failures into ABSENT. Updated the source contract to require the helper call and `return $?`, while retaining explicit RUNNING (0) and UNKNOWN (2) checks.
- Shell lifecycle tests passed on #45713996599; exact-head DDNSTO workflow for this checker-only correction is pending. No firmware or hardware result is claimed.
- Changed paths: `scripts/check_ddnsto_lifecycle.py`, `dev_log.md`.

---

## 2026-11-25 — Remove stale single-address-count requirement from AliDDNS checker

- Exact AliDDNS run #45713773321 on `6a8d91b6` passed every shell regression, including the new WAN IPv6 selector (stable preference and sole-temporary fallback), but the Python source-contract checker still required the old `count != 1` test from the ifconfig implementation.
- Removed that obsolete checker predicate. The new stable/secondary counters and their explicit one-candidate branches remain required; the contract message now reflects both supported selection outcomes.
- The full shell regression and AWK selector tests passed on #45713773321 before the stale contract failed. Exact-head AliDDNS CI for this checker correction is pending; no firmware or hardware result is claimed.
- Changed paths: `scripts/check_aliddns_api_transport.py`, `dev_log.md`.

---

## 2026-11-25 — Rebuild the DDNSTO lifecycle test tail without duplicated fragments

- Exact DDNSTO run #45713687122 on `8cf6160f` still failed after the force-stop fixture because earlier string replacements had only partially repaired the quoted grep expression. Inspection of the full 891-line test also exposed a pre-existing duplicated concurrency/restart block and orphan command lines after the first restart PASS, which could invoke the signal log as a command.
- Replaced the damaged numeric-PID assertion as complete lines and removed the orphan plus redundant duplicate tail, retaining exactly one concurrency serialization test and one RC stop/start transaction test. Production DDNSTO source remains unchanged.
- Local `sh -n` and exact TERM/KILL log assertion smoke checks PASS. Exact-head DDNSTO Actions for this repair are pending; no firmware or hardware result is claimed.
- Changed paths: `trunk/user/ddnsto/tests/ddnsto_lifecycle.sh`, `dev_log.md`.

---

## 2026-11-25 — Close the AliDDNS IPv6-selection AWK block

- Exact-head AliDDNS run #45713498685 on `31078270` passed the API and IPv4 regressions but the WAN IPv6 fixture exposed a syntax error in the new selector: the final `else` was closed, but the surrounding AWK `END` action was not.
- Added the missing closing brace before the AWK program terminator. The defect was isolated to the IPv6 selection script; no public-address policy or fallback semantics changed.
- Local shell syntax and AWK selector smoke checks PASS. Exact-head AliDDNS and XY-C1 checks for this follow-up are pending; no firmware or hardware result is claimed.
- Changed paths: `trunk/user/aliddns/ddns_script.sh`, `dev_log.md`.

---

## 2026-11-25 — Repair the complete DDNSTO TERM/KILL test assertion

- DDNSTO exact-head run #45712559118 on `373b4a12` failed at the original unterminated ERE. The first test-only correction was incomplete: a replacement boundary left the following PASS message concatenated into the grep expression. Exact-head run #45713236953 on `a9dd0b06` therefore failed again with `grep: Unmatched (`.
- Replaced the entire damaged region up to the next stable section heading, restoring a complete ERE that checks exactly two positive numeric TERM/KILL PID records and a separate PASS message. Production DDNSTO code is unchanged.
- Local shell syntax and numeric-PID assertion smoke checks PASS. Exact-head DDNSTO lifecycle run for this commit is pending; no firmware or hardware result is claimed.
- Changed paths: `trunk/user/ddnsto/tests/ddnsto_lifecycle.sh`, `dev_log.md`.

---

## 2026-11-25 — Preserve temporary-only WAN IPv6 fallback for AliDDNS

- Review of the new stable-address preference found a compatibility edge: the first implementation rejected a sole temporary/secondary global address, whereas the prior single-address behavior accepted it. A privacy-enabled WAN can legitimately need that fallback when no stable preferred address remains.
- Selection now prefers exactly one usable stable global address when present; if none exists, exactly one usable temporary/secondary global address is accepted. Tentative/deprecated addresses are excluded, and ambiguous stable or temporary-only sets still fail closed.
- Regression now checks stable-over-temporary preference, temporary-only compatibility, multiple stable/temporary ambiguity, and tentative/deprecated exclusion. The Python source contract pins both selection branches.
- Local smoke verification is being repeated for this compatibility refinement; exact-head AliDDNS and XY-C1 checks will be required. No firmware or physical-router result is claimed.
- Changed paths: `trunk/user/aliddns/ddns_script.sh`, `trunk/user/aliddns/tests/aliddns_ipv6_lookup.sh`, `scripts/check_aliddns_api_transport.py`, `dev_log.md`.

---

## 2026-11-25 — Repair DDNSTO lifecycle regression's truncated grep expression

- Exact DDNSTO workflow #45712559118 on `373b4a12` failed after the process-identity and procfs cases passed. The signal-record assertion at test line 559 had an unterminated single-quoted ERE; its quote captured a later AWK program and grep reported `Unmatched (`.
- Restored the intended positive numeric PID validation as a bounded count of TERM/KILL signal records. This is test-only; no daemon behavior changed.
- Local signal-log assertion smoke test PASS. Exact-head DDNSTO lifecycle run for the follow-up SHA is pending at commit creation; no firmware or hardware result is claimed.
- Changed paths: `trunk/user/ddnsto/tests/ddnsto_lifecycle.sh`, `dev_log.md`.

---

## 2026-11-25 — Align AliDDNS source contract with stable IPv6 address selection

- The privacy-mode source and shell regression were committed as `5a855d59c5b1313cb4459609ee8444e3589a110d`. Reviewing the full CI path revealed the Python source-contract checker still required the retired `ifconfig` parser, so it would reject the intended implementation.
- Updated the exact-source contract to require the BusyBox `ip -6 addr` lookup and explicit exclusion of secondary/temporary, tentative and deprecated addresses; it also requires the new privacy-mode and non-usable-address fixtures.
- The local address-selection smoke test and `sh -n` checks passed before the first commit. The exact-head AliDDNS workflow has not yet been observed for that SHA; this atomic checker follow-up restores its source-contract consistency.
- Changed paths: `scripts/check_aliddns_api_transport.py`, `dev_log.md`.

---

## 2026-11-25 — Select a stable WAN IPv6 address for AliDDNS privacy mode

- AliDDNS WAN IPv6 discovery previously required exactly one global address from `ifconfig`. With IPv6 privacy extensions enabled, Linux may expose a temporary/secondary global address alongside the stable address; the legitimate pair was classified as ambiguous and the DDNS update failed.
- Switched discovery to BusyBox `ip -6 addr show dev`. It now accepts exactly one usable global address, excludes temporary/secondary, tentative and deprecated entries, and retains the strict public-GUA validator. Multiple stable addresses and a temporary-only interface still fail closed.
- Extended the shell regression with temporary+stable selection, temporary-only, deprecated-only, tentative-only and ambiguous-stable cases; existing NVRAM, interface-boundary, public-range and command-failure cases remain.
- Local stable-address selection smoke test and shell syntax checks PASS. Exact-head AliDDNS Actions are pending at commit creation; no firmware or physical-router result is claimed.
- Changed paths: `trunk/user/aliddns/ddns_script.sh`, `trunk/user/aliddns/tests/aliddns_ipv6_lookup.sh`.

---

## 2026-11-25 — Fix DDNSTO UNKNOWN-start fixture and duplicate XY-C1 path

- DDNSTO exact-head #45711617256 on `4abf56ad` passed the executable/UID rejection and procfs-confirmation cases, then failed at UNKNOWN-start. Initial stop does a direct pidof lookup and the shared procfs fallback does a second; the fixture had injected UNKNOWN at call two before launch. Changed injection to call three (first post-launch probe), still independent of daemon scheduling.
- XY-C1 exact-head #45711617341 failed before Build because `trunk/user/shared/Makefile` appeared twice in its push path filter. Removed the duplicate and retained one existing package dependency path.
- Parent Process inventory #45711617474, AI docs #45711617453 and Process fail-closed #45711617537 PASS. This atomic correction needs fresh exact-SHA DDNSTO/XY-C1 checks; no firmware success is claimed.

---

## 2026-11-25 — Bind DDNSTO stop signals to verified process identity

- DDNSTO's numeric-only `pidof` check and name-wide TERM/KILL could signal an unrelated process which merely adopted the `ddnsto` comm name. It also treated `pidof` status 1 as absence without verifying procfs.
- Candidate PIDs must now match the installed `/usr/bin/ddnsto` device/inode and service UID. A `pidof` miss is confirmed ABSENT only by the shared procfs scanner; TERM and KILL use fresh validated inventory snapshots and exact numeric PIDs, with UNKNOWN blocking escalation.
- Regression extends lifecycle fixtures for spoofed executable, foreign UID, unreadable `/proc/PID/exe`, pidof/procfs disagreement, unreadable procfs and numeric PID signals. DDNSTO CI follows the shared procfs helper/package; XY-C1 follows the shared Makefile packaging dependency.
- Parent exact-SHA WAN DNS #45710242187, HTTPD #45710242155, WAN route #45710242121, Process #45710242116, NVRAM #45710242112, WAN DHCP callback #45710242082, WAN 6rd #45710242043 and IPv6 service-type #45710241898 PASS on `c0926c37`; XY-C1 #45710242062 remained IN_PROGRESS. AI docs #45711063239 PASS on `199c9f04`. Current-slice Actions pending; no hardware result claimed.
- Changed paths: `ddnsto.sh`, `tests/ddnsto_lifecycle.sh`, `check_ddnsto_lifecycle.py`, `ddnsto-lifecycle.yml`, `xy-c1-firmware-validation.yml`, `SESSION_HANDOFF.md`, `dev_log.md`.

---

## 2026-11-26 06:05 CST (UTC+8) — Validate static and DHCPv6 WAN DNS server literals

- Static WAN DNS fields `ip6_dns1..3` were combined into the resolver-facing value without validating each slot. DHCPv6 `new_domain_name_servers` was copied into the `dns6` NVRAM value without validating its individual addresses. Malformed text or CR/LF could therefore enter resolver state.
- Added `wan6_dns_validate_static_server()` and `wan6_dns_validate_server_list()`. Static slots accept valid IPv6 literals or empty optional slots; dynamic lists accept space/tab-separated IPv6 literals or an empty withdrawal. NULL, overlong, malformed, hostname, IPv4, prefix and control-character input is rejected before NVRAM read/publication, preserving the previous setting.
- Regression additions cover valid IPv6, optional empty slots, dynamic withdrawal, multiple servers, malformed later tokens, IPv4, prefixes, hostname, malformed IPv6 and CR/LF injection. `WAN IPv6 static DNS literal validation` #45710242187 PASS on exact SHA `c0926c379f6f9774d4992ef550353a98a347a690`. The workflow compiles with GNU89, `-Wall -Wextra -Werror` and ASan/UBSan, then checks production call ordering.
- Same-SHA WAN route #45710242121, WAN DHCP callback #45710242082, WAN 6rd #45710242043, IPv6 service-type #45710241898, HTTPD #45710242155, Process inventory #45710242116 and NVRAM #45710242112 PASS. XY-C1 #45710242062 remained IN_PROGRESS at Build XY-C1; TRX and artifact upload were pending. No current-source firmware L3 or physical L4 result is claimed.
- Changed paths: `net_wan6.c`, `wan6_dns_policy.h`, `tests/wan6_dns_policy.c`, `scripts/check_wan6_dns_policy.py`, `.github/workflows/wan6-dns-policy-stability.yml`.

---

## 2026-11-26 03:24 CST (UTC+8) — Normalize ZeroTier /32 route display during cleanup

- `zero_route_entry_matches()` compared the configured NVRAM destination to the first `ip route show` token literally. iproute2 may render a host route without its explicit `/32`, so a configured `192.0.2.7/32` could be classified as already absent when the kernel inventory returned `192.0.2.7`. Stop could then remove persistent ZeroTier configuration while leaving that static route attached to the interface.
- Added an AWK route-key normalizer that equates a bare IPv4 host destination with the same explicit `/32`, without collapsing any other prefix. Existing gateway/interface and multipath matching remain strict.
- Added a lifecycle regression with a configured `/32` destination and route output lacking the suffix; it requires a delete and verifies the route disappears before persistent configuration is removed. The Python source contract pins the normalization and regression.
- Parent exact-head USB modem #45708933844, Process #45708933888, HTTPD #45708933932 and AI #45708933958 PASS; NVRAM #45708933862 and XY-C1 #45708934022 were still IN_PROGRESS at last poll. Current ZeroTier/WAN/Process/AI/XY exact-head checks pending.
- Changed paths: `zerotier.sh`, `tests/zerotier_lifecycle.sh`, `check_zerotier_lifecycle.py`, `SESSION_HANDOFF.md`, `dev_log.md`.

---

## 2026-11-26 03:08 CST (UTC+8) — Fail closed on USB SCSI-generic modem_zcd reads

- `mdev_sg_main()` selected between loading `sr_mod` and executing the privileged `/sbin/zerocd` helper using unchecked `nvram_get_int("modem_zcd")`. A backend read failure was therefore indistinguishable from the legacy missing-key/default value and could take the wrong device-mode transition.
- Replaced the lossy getter with `nvram_get_checked()` and a dedicated bounded decimal policy. A missing key preserves the historical zero default; a backend failure, impossible API status, missing returned value, overflow or trailing junk aborts before either side effect. The policy output is initialized to fail closed.
- Added GNU89 regression cases for absent/default, configured 0/1/negative nonzero, backend errors, malformed values, overflow, NULL value and impossible API status. The USB modem workflow now pins checked-read-before-branch ordering and the regression cases.
- Exact parent `38fe0fadc4eca4dd35431b72bad079725fe56887` WAN6 #45708135345, NVRAM #45708135328, Process #45708135338 and AI docs #45708135299 PASS. Exact-head USB modem/Process/NVRAM/AI checks for this slice are pending; no firmware or physical USB test is claimed.
- Changed paths: `hotplug_usb.c`, `modem_sg_module_policy.h`, `tests/modem_sg_module_policy.c`, `usb-modem-identity-stability.yml`, `SESSION_HANDOFF.md`, `dev_log.md`.

---

## 2026-11-26 02:52 CST (UTC+8) — Enforce the RFC 5969 6rd relay-count ceiling

- The DHCP 6rd binary-option validator limits the payload to its one-byte length and requires the 18-byte fixed header plus complete four-byte relay addresses, so a valid option can contain at most 59 relays. The downstream text policy documented that limit but accepted arbitrarily more addresses when called with a synthetic or future non-wire input.
- Added one shared 59-relay ceiling and fail-closed rejection before accepting relay 60. Regression keeps the exact 59-address maximum valid and confirms a 60-address list fails with the output buffer cleared. The Python source contract now pins the policy guard and over-limit case.
- Local GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` compile/run with leak detection — PASS. Parent WAN 6rd workflow #45706623877 passed on `360ccb32e57ffa4689992b3c8efda20cde5d5262`; exact-head validation for THIS COMMIT is pending. No MIPS firmware or physical-router result is claimed.
- Base main SHA: `a2979282ae6bd462289bf55016fe7dca4b1c9149`. Changed paths: `wan6_sixrd_option_policy.h`, `tests/wan6_sixrd_option_policy.c`, `scripts/check_wan6_sixrd_relay.py`, `SESSION_HANDOFF.md`, `dev_log.md`.

---

## 2026-11-25 — VSFTPD HTTP status correctness and cross-userland hardening

- VSFTPD HTTP GET previously emitted `HTTP/1.1 200 OK` before the shared RETR handler checked ACL access, securely opened the requested path, confirmed a regular file and enforced anonymous-world-readable policy. Missing/denied paths could therefore produce an HTTP 200 response followed by an FTP 550 reply. Commit `6534ff95d5a6368bc7f6f438a3427ea6048fb6df` delays success headers until all pre-transfer checks pass and maps invalid REST, permission, missing-file and non-regular-file failures to HTTP 400/403/404 with an empty body. The VSFTPD workflow source contract and GNU89 policy suite #45707415023 PASS; Process inventory #45707415097 and AI docs #45707414961 PASS on the same SHA. NVRAM #45707414901 and XY-C1 #45707417102 were IN_PROGRESS at checkpoint; XY Build was running and TRX/artifact pending. No live HTTP socket test or firmware L3 result is claimed.
- Earlier source commits in this session: `1b8ea87b` / `0e8f4dac` / `22b2a943` reject empty, non-regular and symlinked AliDDNS CA bundles (actual tar and startup regressions; AliDDNS #45705446010 PASS on 22b2a943); `70b5733c` fixes BusyBox DHCP 6RD type confusion and bounds relay formatting; `b4d727d1` / `8522ac96` cap VSFTPD STOU candidate probes at 128 and avoid an outer retry rescan; `360ccb32` validates every DHCPv6 6rd relay-list token and supports the first relay under the existing single-relay NVRAM model (WAN 6rd #45706623877 PASS).
- Exact current-main verification remains source-specific: `6534ff95` VSFTPD #45707415023, Process inventory #45707415097, AI docs #45707414961 PASS; NVRAM #45707414901 and XY-C1 #45707417102 IN_PROGRESS at last poll. Earlier 360ccb32 WAN/IPv6/NVRAM/HTTPD/Process/6RD checks passed; its XY-C1 build had not completed at last poll. Do not infer firmware L3 or hardware L4 from host Actions.
- Changed source paths across these slices include BusyBox `dhcpc.c` and 6RD policy/tests, WAN IPv6 `net_wan6.c` and relay policy/tests, AliDDNS CA bootstrap/autostart scripts and regressions, and VSFTPD `postlogin.c` plus workflow source contracts.

---

## 2026-11-26 — Keep detect_wan ARP receive deadline across EINTR

- The gateway ARP probe had a finite one-second socket timeout and a five-second elapsed-time bound for EAGAIN and normal packet wakeups, but its EINTR branch retried unconditionally. Repeated unrelated signal interruptions could therefore keep a single gateway probe alive beyond its advertised deadline and stall monitoring/recovery.
- Added `detect_wan_arp_wait_remaining()` and use it after EINTR, EAGAIN/EWOULDBLOCK and received packets. The receive loop now terminates at the same five-second deadline regardless of why recvfrom returned. A negative elapsed time also fails closed.
- Extended the existing GNU89/Werror/ASan/UBSan ARP policy fixture with before/exact/after deadline and invalid-argument cases, and the WAN ARP workflow source contract requires the helper in all three wakeup paths.
- Parent 274 RStats #45702809332, Process inventory #45702809053 and AI docs #45702809111 PASS; Process fail-closed #45702809178, NVRAM #45702809234, HTTPD #45702809077 and XY-C1 #45702809103 were IN_PROGRESS at last poll. Exact WAN ARP / shared checks for this follow-up are pending at commit creation. No local, MIPS or hardware result is claimed.
- Changed paths: `detect_wan.c`, `detect_wan_arp_result_policy.h`, `tests/detect_wan_arp_result_policy.c`, `detect-wan-arp-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-26 — Size the RStats row buffer for full-width counters

- The transactional `/proc/net/dev` parser rejects truncated line input. A valid row containing sixteen 20-digit 64-bit counters plus labels and separators can exceed the prior 256-byte `fgets()` buffer, falsely classifying a valid snapshot as corrupt.
- Increased the bounded line buffer to 512 bytes and added a regression that builds and parses a full 16-counter row with every field at `UINT64_MAX`; the test asserts the encoded row exceeds 255 bytes but fits the configured buffer.
- RStats parent #45702409711, Process #45702409582, HTTPD #45702409529 and AI docs #45702409433 PASS; Process fail-closed #45702409483, NVRAM #45702409511 and XY-C1 #45702409540 were IN_PROGRESS. Exact-head checks for this follow-up pending. No firmware result claimed.
- Changed paths: `rstats.c`, `rstats_persistence_policy.h`, `tests/rstats_policy_test.c`, `rstats-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-26 — Preserve RStats history when /proc/net/dev snapshot is incomplete

- `process_rstats()` previously updated interfaces inline while reading `/proc/net/dev`, ignored malformed or missing rows and stream errors, then advanced the shared ring. A truncated read could partially update some interfaces, synthesize zero-speed gaps for others and age out their history; an open/header failure still advanced the baseline without any valid sample.
- Added a strict shared parser for complete Linux 16-counter records and a snapshot result policy. Production now stages all selected interface samples, validates both header lines, every row, stream error and close result, and only commits samples / advances history after the entire snapshot succeeds. Errors preserve the last valid counters and retry on the next poll.
- GNU89 ASan/UBSan regression covers valid RX/TX extraction, `UINT64_MAX`, signed/missing/extra/overflow counters, overlong interface names, NULL arguments, and read/close status precedence. RStats workflow source contract pins full read/close before the first counter mutation.
- Previous WDM exact-head USB modem #45701307930, Process #45701307914, AI docs #45701307827 and HTTPD #45701307732 PASS; NVRAM #45701307926 and XY-C1 #45701307950 were IN_PROGRESS. Parent AdGuardHome exact-head security #45701455808, Process #45701455295 and AI docs #45701455540 PASS. New exact-head RStats / Process / AI / NVRAM / XY-C1 checks are pending at commit creation. No local, MIPS or hardware result is claimed.
- Changed paths: `rstats.c`, `rstats_persistence_policy.h`, `tests/rstats_policy_test.c`, `rstats-stability.yml`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-26 — Remove eval from AdGuardHome daemon launch

- `start_adg()` interpolated the environment-overridable `ADG_CONFIG_FILE` into an `eval` string. Whitespace split the path and shell metacharacters such as `; touch <marker> #` were parsed as a second command in the privileged launch context.
- The launcher now directly invokes `AdGuardHome -c "$adg_file" -w /tmp/AdGuardHome -v` with no second shell parse. The lock FD is still closed in the child before daemon exec.
- Added an adversarial regression with a path containing spaces, semicolon, a marker command and comment delimiter. The test records every argv entry and requires one exact config argument plus absence of the injected marker; Python source contracts forbid eval.
- Static source/test/checker contract assertions PASS. Exact-head AdGuardHome security workflow and shared Process/NVRAM/AI/XY-C1 checks pending at commit creation; no MIPS/hardware result claimed.
- Changed paths: `adguardhome.sh`, `tests/adguardhome_config_security.sh`, `scripts/check_adguardhome_config.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-26 05:30 CST (UTC+8) — Fail closed when WDM hotplug cannot clear the QMI client ID marker

- \`mdev_wdm_main()\` previously ignored \`unlink(QMI_CLIENT_ID)\` and could publish a new WDM physical-identity record while a global WDS client-ID marker remained. If /tmp cleanup failed, that stale marker could be reused against a different physical modem.
- WDM hotplug now passes unlink result/errno through \`modem_qmi_handle_file_unlink_status()\`: missing markers remain benign; EACCES/EROFS/other errors stop new identity publication and trigger best-effort removal of stale per-device metadata. Physical unplug still removes per-device metadata even if the global marker cleanup failed, while returning a failure status for either incomplete cleanup.
- Extended the USB modem workflow's exact-source contract to pin marker unlink → status normalization → add-side publication gate and remove-side metadata cleanup. Reuses existing GNU89 ASan/UBSan policy cases for success, ENOENT, EACCES, EROFS and missing-errno normalization.
- Parent exact-head DHCPv6 IA #45700615752, USB storage #45700615482, WAN callback #45700615454, Process inventory #45700615636 and AI docs #45700615554 PASS. Parent NVRAM #45700615438 and XY-C1 #45700615486 were IN_PROGRESS. New exact-head USB modem / Process / AI / NVRAM / XY-C1 Actions are pending at commit creation. No local run, firmware or physical-router result is claimed.
- Changed paths: \`hotplug_usb.c\`, \`usb-modem-identity-stability.yml\`, \`SESSION_HANDOFF.md\`, \`REFACTOR_ROADMAP.md\`, \`dev_log.md\`.

---

## 2026-11-26 05:24 CST (UTC+8) — Propagate DHCPv6 NoBinding recovery failures

- During Renew/Rebind, a NoBinding Status Code triggers `reestablish_ia()`, which removes the old IA timer before preparing the replacement Request. Its error return was ignored; if IA option preparation or authentication parameter copying failed, the event was acknowledged even though the IA had no new timer/request to continue recovery.
- The branch now clears stale errno, captures the result, normalizes a missing errno to EIO, stores the first IA error and returns it through `client6_recvreply()`'s existing bounded event retry.
- Added error-normalization regression cases and source-checker assertions proving the NoBinding result is captured and recorded before the branch exits.
- Focused GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` policy compile/run — PASS. Parent DHCPv6 IA workflow #45700172634 PASS; exact-head Actions for THIS COMMIT pending at commit creation. No MIPS or hardware result claimed.
- Changed paths: `dhcp6c_ia.c`, `dhcp6c_ia_policy.h`, `tests/dhcp6c_ia_publication_policy.c`, `scripts/check_dhcp6c_ia_publication.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-26 05:10 CST (UTC+8) — Prevent DHCPv6 IA T1/T2 ratio overflow

- `update_ia()` computed default T2 with `duration * 4 / 5` and adjusted T1 with `t2 * 5 / 8`. Both operands are 32-bit unsigned DHCP lifetime fields; multiplication wrapped before division for large but valid finite leases, scheduling T2/T1 much earlier than the intended ratios.
- Added exact 4/5 and 5/8 quotient/remainder helpers to `dhcp6c_ia_policy.h` and replaced all three production arithmetic sites. The helper avoids both overflow and target-side 64-bit division.
- Existing DHCPv6 IA publication regression now tests zero/small/default, two-billion-second and maximum finite inputs; the source checker rejects both legacy expressions and requires both policy helpers.
- Local focused GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` compile/run — PASS. Exact-head DHCPv6 IA, Process, NVRAM, AI docs and XY-C1 Actions remain pending at commit creation; no MIPS or physical-router result claimed.
- Changed paths: `dhcp6c_ia.c`, `dhcp6c_ia_policy.h`, `tests/dhcp6c_ia_publication_policy.c`, `scripts/check_dhcp6c_ia_publication.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-26 04:56 CST (UTC+8) — Frame and drain each queued IAPP driver signal separately

- The new nonblocking writer made each driver signal one atomic `PIPE_BUF`-bounded write, but the consumer still requested up to `IAPP_MAX_RCV_PKT_SIZE` in one `read()`. Pipe reads may combine several atomic writes; `IAPP_SIG_Process()` handles only one structure, so a coalesced read could silently discard later queued events.
- Each write now carries a native `size_t` payload-length prefix, and the runtime atomic-write check includes that prefix. The reader consumes exactly one prefix and exactly its declared payload (retrying interrupted/short reads) before dispatch, leaving subsequent records queued for the next select iteration.
- Extended the GNU89 ASan/UBSan policy fixture with two queued records, one-byte fragmented read delivery, and oversized-length rejection. The same test job covers nonblocking pipe saturation, EINTR, short-write rejection and EAGAIN.
- Local helper regression: GNU89 compile with `-Wall -Wextra -Werror -fsanitize=address,undefined` and leak detection — PASS. Parent IAPP host workflow #45698920147 PASS; exact-head checks for THIS COMMIT pending at commit creation. No MIPS build or hardware result is claimed.
- Changed paths: `rtmpiapp.c`, `iapp_signal_pipe_policy.h`, `iapp_signal_pipe_policy.c`, `iappd_lifecycle.sh`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.

---

## 2026-11-26 04:45 CST (UTC+8) — Make the IAPP driver-signal pipe nonblocking and record-atomic

- `IAPP_USR2Handle()` previously wrote a driver signal context to a blocking pipe, while the pipe reader runs in the same IAPP event-loop thread. A full pipe could therefore block the callback and prevent the reader that would free capacity from running.
- `IAPP_SocketOpen()` now queries the pipe writer's runtime `_PC_PIPE_BUF`, rejects a limit smaller than `sizeof(RT_SIGNAL_STRUC)`, and sets only the write endpoint to `O_NONBLOCK` before registering SIGUSR2. The new policy helper validates the complete record against that limit, retries EINTR, rejects any short write as EIO, and passes EAGAIN through immediately rather than blocking or splitting a record. Pipe teardown clears the cached limit.
- Added GNU89 ASan/UBSan tests for invalid/zero/oversized records, EINTR retry, short-write rejection, EAGAIN propagation and a saturated real nonblocking pipe. Integrated the fixture and its header/source paths into both push and pull_request IAPP lifecycle coverage.
- Local helper regression: GNU89 compile with `-Wall -Wextra -Werror -fsanitize=address,undefined` and leak detection — PASS. Exact-head IAPP/Process/AI/XY-C1 Actions for THIS COMMIT pending at commit creation; no full MIPS or hardware result is claimed.
- Changed paths: `rtmpiapp.c`, `iapp_signal_pipe_policy.h`, `iapp_signal_pipe_policy.c`, `iappd_lifecycle.sh`, `iappd-lifecycle.yml`, canonical handoff/roadmap/log and indexed log archive.

---

## 2026-11-25 — Keep the TCP write failure label portable

- Source review of the bounded write helper caught a preprocessor portability defect before its exact-SHA CI completed: `goto write_fail` is part of the shared sender, but the label had been compiled only under `IAPP_OS_LINUX`. Non-Linux/VxWorks builds would therefore fail with an undefined label. Moved the label outside the platform conditional while keeping only the `fcntl()` restore conditional.
- Added a lifecycle source contract that keeps the shared failure label present independently of Linux-specific flag restoration.

---

## 2026-11-25 — Bound TCP frame write backpressure

- `IAPP_TCP_WriteFrame()` handled partial writes but used a blocking socket. If the peer stopped reading, a send could wait beyond the connect/read deadlines and stall the IAPP event loop.
- On Linux the sender now saves current flags, sets O_NONBLOCK, waits for writability with the same five-second absolute deadline, retries EINTR/EAGAIN/EWOULDBLOCK, and restores the original flags after completion or error. `MSG_NOSIGNAL` remains used when available. Non-Linux targets retain the legacy blocking send loop for compatibility.
- Lifecycle source contracts assert separate nonblocking connect/write setup and the write-side deadline/would-block retry path.

---

## 2026-11-25 — Bound outbound IAPP TCP connect latency

- All four outbound TCP paths called blocking `connect()` directly. A peer that drops packets or never completes a handshake could stall the daemon's event loop even though receive already has an absolute deadline.
- Added `IAPP_TCP_Connect()` for Linux: save file flags, set O_NONBLOCK, initiate connect, wait for writability using the existing five-second absolute-deadline helper, check SO_ERROR, then restore original socket flags before sending. Timeout/error restores flags and fails closed. Non-Linux/VxWorks retains legacy blocking connect for compatibility.
- Routed all four outbound TCP paths through the helper. The lifecycle source contract pins nonblocking mode, deadline wait, SO_ERROR validation and flag restoration.

---

## 2026-11-25 — Handle SIGPIPE for the raw-driver notification pipe

- Follow-up to TCP `MSG_NOSIGNAL`: `IAPP_USR2Handle()` also writes driver signal contexts into `PipeRawDrv`. If the pipe reader has closed, `write()` can deliver process-wide SIGPIPE before the existing short-write/error check runs, terminating the daemon.
- On Linux, the daemon now ignores SIGPIPE before registering the driver's SIGUSR2 handler or publishing its PID to the driver. Pipe writes therefore return EPIPE and use the existing error path; this does not alter VxWorks signal behavior. The IAPP lifecycle source contract pins the Linux guard and registration.

---

## 2026-11-25 — Prevent SIGPIPE from terminating IAPP TCP sender

- `IAPP_TCP_WriteFrame()` now uses `MSG_NOSIGNAL` when the target socket API exposes it. A remote close/reset during `send()` therefore becomes a normal send error handled by the helper instead of delivering SIGPIPE and potentially terminating the daemon. The existing zero/error handling remains fail-closed; platforms without this flag retain their native fallback.
- Added a lifecycle source assertion for the conditional no-SIGPIPE send flag.

---

## 2026-11-25 — Keep receive-length validation active without FT encryption

- Follow-up review found TCP/UDP `ReceivedLength` was recorded unconditionally, but its corresponding decrypted-length guard sat inside `#ifdef FT_KDP_FUNC_PKT_ENCRYPT`. In builds without that feature macro, this left a set-but-unused local and skipped the generic returned-length/capacity invariant. Moved both guards immediately after the optional decrypt block so raw and decrypted paths always require a nonnegative length no larger than the bytes received.

---

## 2026-11-25 — Validate every IAPP encryption ioctl result

- Audit found transmit encryption IOCTLs (except the UDP call's failure branch) ignored failure and trusted the mutable returned length before TCP/UDP/raw-RRB sends. A failed ioctl could leak plaintext; a negative or over-capacity output length could transmit stale bytes beyond the reserved packet tail.
- Added `IAPP_EncryptFrame()`: it holds the driver output in a signed `INT32`, rejects ioctl failure/negative lengths, and requires the encrypted output to fit the caller's reserved `IAPP_SECURITY_EXTEND_LEN` tail before updating the send length. Routed UDP, generic TCP, FT security-block send/ACK, AP-information response, and raw RRB encryption through it.
- Added GNU89 sanitizer policy cases for negative, exact-capacity and over-capacity driver lengths. Lifecycle source contract requires all six transmit encryption paths to use the wrapper.

---

## 2026-11-25 — Bound driver-returned receive and signal-context lengths

- TCP/UDP decrypt ioctls update the in/out `SizeRcvMsg`. The dispatcher then trusted that returned value as if it were still the number of bytes actually received, so a driver-reported length larger than the input frame could bypass all subsequent parser bounds. Both paths now retain the pre-ioctl receive length and reject negative or larger decrypted lengths before reading the IAPP header.
- The non-raw-socket `IAPP_USR2Handle()` allocated exactly one `RT_SIGNAL_STRUC`, ignored ioctl failure, then used the returned `DataLen` to read from that allocation into a pipe. It now rejects ioctl failure and negative/over-capacity returned lengths, and reports short pipe writes.
- Lifecycle source gates pin the TCP/UDP in-place decrypt output limits and signal-context allocation bound.

---

## 2026-11-25 — Guard TCP select descriptors before FD_SET

- The absolute-deadline wait helper passed its socket descriptor to `FD_SET()` without checking `FD_SETSIZE`. `FD_SET` operates on a fixed-size `fd_set`; an unexpectedly high descriptor can write beyond that object. The helper now rejects negative or out-of-range descriptors before touching the sets, and the lifecycle source gate pins the check.

---

## 2026-11-25 — Validate the raw RRB decryptor's returned length

- The raw RRB path now checks the rounded encrypted input span before ioctl, but then trusted the in/out `SizeRcvMsg` returned by the driver decrypt ioctl and added the fixed header before allocating/copying it. A negative or unexpectedly large returned length could make the later copy exceed the received frame buffer.
- After a successful decrypt, reject negative lengths and require the plaintext length to fit the actual received frame bytes after `FT_RRB_HEADER_SIZE` before adding the header or copying into the OID request.
- Lifecycle source contract now pins both pre-decrypt encrypted-span and post-decrypt plaintext-span checks.

---

## 2026-11-25 — Bound TCP frame receive time against slow peers

- Review of the new read-until-EOF loop found an availability regression: a peer could connect, send a prefix, and keep the stream open indefinitely, blocking the IAPP event loop. Added an absolute five-second frame deadline with `select()` before each read (including the overflow probe); EINTR is retried without extending the deadline.
- The frame reader still accumulates fragmented reads, rejects data beyond `IAPP_MAX_RCV_PKT_SIZE`, and treats EOF as the end of this one-frame connection. The lifecycle contract pins the deadline constant and wait-before-read path.

---

## 2026-11-25 — Require exact IAPP protocol frame lengths

- TCP and UDP dispatchers previously checked only the fixed IAPP header minimum and trusted its 16-bit `Length` field independently of actual post-decrypt bytes. This allowed packets with truncated or trailing data to reach command handlers when the command-specific payload check did not reject them.
- Added shared equality policy and require `SWAP_16(RT_IAPP_HEADER.Length) == SizeRcvMsg` in both dispatchers before command dispatch. The optional IAPP_EVENT_LOG security monitor now requires the full `RT_IAPP_SECURITY_MONITOR` struct before reading its event table.
- Added exact/short/long header-length unit cases and lifecycle source contracts for TCP, UDP and security-monitor checks.

---

## 2026-11-25 — Complete partial TCP writes for IAPP frames

- Four IAPP TCP paths used a single `send()` and treated any nonnegative return as a complete frame. A short write can leave the peer with a truncated encrypted IAPP packet, while the sender closes the one-frame connection immediately afterward.
- Added `IAPP_TCP_WriteFrame()` to loop until the entire frame is sent, retry EINTR, reject zero/error writes and bound each append. Replaced single-send behavior in the general TCP packet sender, FT security-block send, FT security-block ACK and AP-information response sender. Raw Ethernet/datagram sends remain single-message sends.
- The shared append-capacity policy test covers partial accumulation/exact-capacity/overflow, and lifecycle source contracts require all four TCP send paths to use the helper.

---

## 2026-11-25 — Read complete IAPP TCP frames across partial reads

- `IAPP_RcvHandlerTcp()` used a single `read()` on a TCP stream and then closed the connection. TCP may return only a prefix even when the peer issued one `send()`, so valid frames could be dropped or parsed incomplete.
- Added `IAPP_TCP_ReadFrame()`: collect until EOF (the repository's TCP sender creates one connection per frame and closes after its send), retry EINTR, enforce receive-buffer capacity and probe one extra byte to reject over-capacity frames rather than silently parse a truncated prefix.
- Added overflow-safe append-capacity policy and GNU89 ASan/UBSan tests for fragmented accumulation, exact capacity, over-capacity and SIZE_MAX. Lifecycle shell pins EOF/retry/capacity behavior.

---

## 2026-11-25 — Reject truncated fixed IAPP ADD/MOVE notifications

- UDP ADD_NOTIFY checked only the protocol-declared IAPP Length before reading AddressLen/MAC/Rsvd; MOVE_NOTIFY did the same and had no actual receive-length input. A short datagram with a forged fixed-length header could trigger reads beyond received bytes.
- TCP and UDP MOVE_NOTIFY dispatches now pass actual post-decrypt receive length. Its handler requires exact `sizeof(RT_IAPP_MOVE_NOTIFY)`, matching IAPP Length, AddressLen == ETH_ALEN, and zero context length before any field read that drives state changes. UDP ADD_NOTIFY applies equivalent actual-size, declared-size and AddressLen checks before reading its MAC/Rsvd.
- IAPP lifecycle source contracts pin both actual-frame-size guards. Exact-head Actions pending.

---

## 2026-11-25 — Bound raw driver signal event payloads

- `IAPP_RcvHandlerRawDrv()` copied the destination MAC from any positive-length raw frame and formed a signal pointer after the 14-byte Ethernet prefix; `IAPP_SIG_Process()` then read `RT_SIGNAL_STRUC` fields and an 8-byte `FT_KDP_EVT_HEADER` without using its `Len` parameter. EventLen subsequently drives key/action/neighbor data access, and debug-trace signals read an additional 32-bit value.
- Raw driver now rejects frames shorter than Ethernet + the fixed target signal prefix + event header before MAC copy/pointer dispatch. It passes only the post-Ethernet signal byte count. `IAPP_SIG_Process()` validates the fixed signal/event prefix, ensures EventLen fits actual received bytes using the subtraction-based policy helper, and requires a full INT32 for debug-level events.
- Added GNU89 ASan/UBSan policy boundary cases for empty/exact/truncated signal event payloads. Exact-head CI pending.

---

## 2026-11-25 — Validate raw IAPP RRB frame and encrypted payload lengths

- `IAPP_RcvHandlerRawRRB()` read the Ethernet DA and `FTActionLength` from any positive-length datagram, then rounded the declared action length for the encryption ioctl without checking that the padded encrypted span existed in the received frame. A truncated packet could therefore cause out-of-bounds reads in daemon/driver decrypt handling.
- The handler now requires the complete fixed `FT_RRB_FRAME` header before any field access, saves the physical receive length, preserves the documented Ethernet minimum-frame zero padding behavior, and verifies the rounded encrypted payload fits in the bytes actually received after the RRB prefix before issuing the decrypt ioctl.
- Added regression boundaries for a 60-byte minimum Ethernet frame (24-byte RRB header + 36-byte available payload) and a 64-byte frame. Exact-head IAPP and XY-C1 Actions pending.

---

## 2026-11-25 — Bound IAPP AP-information payload copies

- `IAPP_RcvHandlerApInfor()` trusted its 16-bit Information Block Length and copied that many bytes from the received packet without knowing the packet's actual size. This could read beyond a short TCP/UDP receive buffer. TCP INFO_RESPONSE and UDP INFO_BROADCAST/INFO_REQUEST now pass the post-decrypt receive length; the handler verifies its fixed prefix and validates the declared payload span before command-buffer sizing or copying.
- The shared helper uses subtraction after checking the fixed prefix, preventing `size_t` addition wrap. The IAPP lifecycle source contract asserts the AP-info length guard precedes its payload copy. Exact-head IAPP/firmware checks pending.

---

## 2026-11-25 — Repair GNU89 receive-length test dependency

- Exact IAPP lifecycle run #45691011094 on d444b0e8c2347bf70f5cafec9b05aee08a6e6ffd FAILED before running the policy tests: the new SIZE_MAX boundary assertions lacked their required <stdint.h> include. This was a test-fixture compile error; production receive guards were not exercised by that job. Add the direct standard header include so the test does not depend on transitive definitions.
- d444 exact Actions at correction time: AI handoff #45691011016 PASS; XY-C1 #45691011111, Process lifecycle #45691011010, process lifecycle regression #45691011103 and NVRAM #45691010984 pending. The previous IAPP failure is corrected here; rerun its workflow on this descendant.
