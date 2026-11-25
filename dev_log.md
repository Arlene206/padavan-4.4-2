## 2026-11-24 — Reject non-contiguous DHCPv4 subnet masks

- `dhcpc.c:mton()` counted every set bit after shifting the mask, but never checked that the one-bits were contiguous from the most-significant bit. A malformed DHCP option 1 such as 255.0.255.0 was therefore exported as `mask=16`, silently changing the mask's meaning and allowing a server response to misconfigure the client's interface.
- Added a strict prefix-length helper; invalid length or non-contiguous masks cause the DHCPACK to be deferred through the existing acquisition/renewal retry policy. `fill_envp()` exports the raw subnet and derived `mask` only when the option is exactly four bytes and contiguous. Tests cover /0, /1, /24, /32 and non-contiguous masks.
- The source checker was extended to enforce validation before script invocation and before publishing the derived prefix. Parent SHA 3c7ba07 passed udhcpc callback regression #45522455255, Process/Lifecycle #45522455412, storage hotplug #45522455153 and AI docs #45522455290; NVRAM #45522455453 and XY-C1 #45522455192 were still in progress on last observation. L3 remains unverified; no local compile claim.

## 2026-11-24 — Fix undefined behavior in udhcpc DHCP-option bitmap masks

- fill_envp() builds a bitmap for all 256 DHCP option codes. Its BMASK macro used the signed expression `1 << bit`; any option whose low 5 bits were 31 (for example valid option 31 or 255) invoked undefined behavior by shifting signed `int` into its sign bit. This can occur while handling a remote DHCP reply.
- Moved the mask calculation into a GNU89 policy helper using `1U` and the unsigned-int width, and changed BMASK to call that helper. Added regression boundaries for bits 30/31, word wrap at 32/63 and option 255; the host workflow compiles with UBSan/ASan, and the source checker enforces helper use. The workflow already path-gates the DHCP source, policy, test and checker.
- Parent SHA bccbe5355f4506b121740f47866b962468d0ea37: Process/Lifecycle #45521301186 PASS; Process inventory #45521300961 PASS; HTTPD #45521301271 PASS; WAN DHCP #45521300987 PASS; NVRAM transaction audit #45521301083 PASS; AI handoff docs #45521301108 PASS. XY-C1 #45521301044 still IN_PROGRESS at Build XY-C1; no L3 build/TRX/artifact result yet. No local compile claimed.

## 2026-11-25 — Reject embedded-NUL PPP session state bytes

- ppp_callback_state_file_read() passed a NUL-terminated buffer to the strict decimal parser without checking the original bytes read. A corrupt file such as 42\0ignored\n was therefore interpreted as generation 42 because parsing stopped at the embedded NUL, silently accepting a valid prefix.
- Reject embedded NUL within the exact read length before adding the parser terminator; return EINVAL and leave generation_out zero. The regression creates a binary state file containing a valid decimal prefix, NUL and trailing bytes, then verifies fail-closed rejection.
- Parent source SHA 0af78992a772036818dc4fd71b48ff74b066abeb had Process/Lifecycle #45520798937 PASS, HTTPD #45520798957 PASS, WAN DHCP #45520799105 PASS and AI docs #45520799045 PASS. XY-C1 #45520798962 was still IN_PROGRESS; no L3 result is claimed. The source/header/test path filters now include the modified state file and regression. No local compile is claimed.

## 2026-11-25 — Align PPP shared-state checks and XY-C1 triggers

- The PPP stable-state regression and Process/Lifecycle workflow passed on 4f969e4313a14fc4b185e9af4e1a7c6715e08e0c. HTTPD Security on that SHA failed only because its PPP contract still searched net_ppp.c for O_NOFOLLOW and same-inode locking; WAN DHCP callback run #45519973125 on parent edd52074de4ab7f630ce8c42ec9af7d0835bb51d had the same stale assertions, including fsync inline in the wrapper.
- Updated both static contracts to inspect ppp_callback_state_file.h for owner/regular-file/O_NOFOLLOW validation, stable F_WRLCK sidecar handling, fsync and atomic rename, while checking net_ppp.c for wrapper integration and separate-state publication.
- XY-C1, HTTPD and WAN DHCP path filters now include both PPP callback policy headers and the state-file regression. This ensures that changes to the directly included implementation helpers cannot silently bypass firmware or lifecycle validation.
- Exact-head HTTPD/WAN/XY reruns are pending on the next main SHA. No L3 Build/TRX/artifact or L4 hardware result is claimed.

---

## 2026-11-24 — Preserve PPP callback generations on failed state writes

- A concrete fault path existed in PPP WAN session fencing: ppp_wan_session_generation_change() used the same inode as its fcntl lock and executed lseek()+ftruncate(0) before writing the next decimal generation. A short write or I/O failure after truncation left the pre-existing state file empty/partial; ppp_callback_generation_initial_state_allowed() correctly refuses to reset a pre-existing empty file, so later callback entry and WAN teardown remained fail-closed but wedged.
- Split the persistent lock to /tmp/ppp/wan-session.lock and keep /tmp/ppp/wan-session.state as replaceable data. Readers acquire/validate the stable owner-only lock inode before reading state with O_NOFOLLOW; writers create an owner-only same-directory stage, write the complete token, fsync, close and atomically rename without truncating the prior generation.
- Added a GNU89 ASan/UBSan regression for missing vs existing-empty state, malformed/link/directory state rejection, lock inode stability over atomic replacement, cross-process lock exclusion, RLIMIT_FSIZE short-write failure retaining the previous token, and staged-file cleanup. Added workflow path triggers for net_ppp.c and the new policy/test, plus a source contract prohibiting in-place truncate in the generation-change path.
- Parent main SHA: 0a92a8422ecbc9d38eb48f52992b967a4210fb5b. Process/Lifecycle and XY-C1 exact-head validations for this slice are pending; do not claim local compilation or firmware/hardware verification.

---

## 2026-11-24 — Fail closed on malformed IPTV multicast altnet values

- The IPTV proxy reads the user-configured mr_altnet_x snapshot and passes it to igmpproxy_config_publish(). The config token check only allowed digits/dots/slashes, so values such as 192.0.2.256/24 and 192.0.2.1/33 were published as if they were valid networks.
- A second fallback issue widened malformed short values: start_igmpproxy_with_settings() silently replaced every value with length <=6 by 0.0.0.0/0, instead of defaulting only a missing/empty setting.
- Added strict four-octet decimal IPv4 plus /0..32 CIDR parsing, an empty-only default resolver, and changed the caller to pass every non-empty value through validation. GNU89 regression checks valid /0 and /32, out-of-range octets, bad prefixes, missing separators, extra components, malformed short inputs and preservation of the last-good config. Process-lifecycle workflow also statically checks the production caller uses empty-only fallback.
- Prior checks: HTTPD Security PASS on e02d1e3; AI docs PASS on 139d245; FRP lifecycle PASS on 8b93aa3. XY-C1 Build remained IN_PROGRESS on e02d1e3; no L3 or L4 claim.

---

## 2026-11-24 — Align FRP cross-workflow contract with shared HTTPD uploader

- FRP lifecycle #45517975418 ran every shell syntax check and asynchronous startup, crontab, private config, stop and lock regression successfully on e02d1e3a1c631b308885ba3279fd97247564a4ea. Its only failure was the final scripts/check_frp_lifecycle.py source assertion, which still required the old inline FRP-only mkstemp/chmod/rename logic.
- Updated the checker to inspect the shared HTTPD upload helpers and assert the same security invariants through the generalized implementation: private-key session gate, FRP file type/mode policy, rejection of non-regular targets, same-directory mkstemp 0600 staging, O_NOFOLLOW, fsync/close and atomic rename.
- AI handoff size correction was committed in 139d24533a8f106d07306091320f820f04912548. Exact AI run #45518436003 was still running; this checker correction needs a new exact-head AI and FRP validation. HTTPD Security passed on e02d1e3; XY-C1 remained pending.

---

## 2026-11-24 — Repair cross-workflow contracts after shared HTTPD uploader change

- Exact-head HTTPD Security #45517975390 passed on e02d1e3a1c631b308885ba3279fd97247564a4ea, including the updated GNU89 policy regression and source contract.
- AI handoff #45517975544 failed because REFACTOR_ROADMAP.md measured 131566 bytes against the 131072-byte validator limit. Compressed two verbose P1 bullets without deleting completed findings; the resulting roadmap must be rechecked on the correction SHA.
- FRP Lifecycle #45517975418 passed every shell syntax and lifecycle regression, then failed only at scripts/check_frp_lifecycle.py: its HTTPD contract still expected the former inline FRP-only staging path. The new shared writer preserves FRP's 0700 mode and strict access policy but stages every file class through httpd_upload_create_staging/httpd_upload_publish_staged. Update the checker to verify this shared contract rather than restore the unsafe legacy path.
- XY-C1 #45517975662 was IN_PROGRESS; NVRAM audit #45517975469 had reached its final raw-ioctl audit. No L3 build/TRX/artifact result is claimed for this SHA.

---

## 2026-11-24 — Atomically publish HTTPD storage text files

- Security review found that write_textarea_to_file() protected only FRP private scripts with same-directory staging; normal dnsmasq, crontab, OpenVPN, certificate and authorized_keys updates still opened the final pathname with fopen(..., "w"), followed a destination symlink and truncated the previous file before the full replacement had been written.
- The replacement path now rejects symlink/non-regular destinations, stages on the destination filesystem, compares content using no-follow regular-file descriptors, applies the intended mode to the staged inode, and publishes only after checked flush/fsync/close through atomic rename. Existing empty credential deletion, private-script permission gating, unchanged-content restart behavior and authorized_keys side effects are retained.
- Extended the GNU89 HTTPD upload regression for regular/missing versus symlink/FIFO/directory targets, secure no-follow comparison, missing-target detection and same-directory stage paths. Updated the HTTPD inline source contract to require the atomic writer and forbid the old direct final-path write.
- Parent main SHA: 83f323b86625c210c67c84f1bcde1e3f1eda6f16. Exact-head HTTPD Security, AI handoff and XY-C1 Build/TRX/artifact checks must be verified on the new commit. No local compile, L3 firmware or L4 hardware validation is claimed.

---

## 2026-11-24 — Remove duplicate Process path-filter entries

- AI handoff documentation run #45510549832 on SHA cc6e707136447a5f14fff465e664f8c52cd81bc5 failed because rc.h, net_lan.c and net_wan.c had been appended even though each was already listed in both Process lifecycle push/pull_request path allowlists. Removed only the new duplicate lines; the added helper header, Ralink source and regression test remain in both lists.
- Parent main SHA: cc6e707136447a5f14fff465e664f8c52cd81bc5. Process run #45510550051 and XY-C1 run #45510550061 were still pending/in progress at the time of correction; exact-head recheck required.

---
## 2026-11-24 — Read private credentials before applying UID/GID changes

- The new `lanauth -f` reader validates the credential file against `geteuid()`. A review found that the existing option parser immediately called `setuid(atoi(optarg))` / `setgid(atoi(optarg))`, before loading `-f`; a command containing `-u` could therefore drop privileges and then fail to read the intended root-owned 0600 file. In the old parser, `-u` before `-g` could also make the later setgid fail.
- UID/GID options are now recorded during getopt parsing. lanauth consumes the private password file first, then applies GID before UID independent of argument order; both operations' failures abort startup. A host injectable policy regression covers correct ordering, each failure boundary, no-op and missing callbacks. The file-reader error diagnostic newline was also corrected.
- Exact-head Process/AI and source-SHA XY-C1 Build/TRX/artifact checks pending. Parent main SHA: a30043d18ebf28e4c0dbef2205af2819073aa92f.

---

## 2026-11-24 — Align inline Process workflow with private lanauth credentials

- Process lifecycle #45508464483 on SHA 76c951349c99e3f96ea3226de15438004ce4f756 compiled and ran the LAN authentication restart policy regression successfully, then failed the inline YAML Python contract because that duplicate block still expected the removed `-p password` call. The external `scripts/check_process_lifecycle_integration.py` had already been updated; this correction brings the inline check into sync, verifies normalized -f argv for both gateway cases, asserts the managed writer's mode/write/fsync behavior, and checks daemon loader no-follow/mode/owner/link/inode/unlink protections.
- The following private credential-file C regression had been skipped only because the preceding inline step failed; the next Process run must execute it.
- Parent main SHA: 76c951349c99e3f96ea3226de15438004ce4f756. XY-C1 production-source validation remains tracked on exact source SHA 4e9d0c25f02e0c83c8657215f7cfa6b7b6ac0120.

---

## 2026-11-24 — Normalize wrapped LAN authentication argv contract

- Process lifecycle #45508199161 on source SHA 259f9bc5f5574e07bf820c0a685ceefaa72af9cb passed all preceding integration checks and failed on a formatting-sensitive assertion: the gateway variant of `eval(... "-f", credential_path)` is split across two C source lines. Updated the contract to collapse whitespace before checking the exact argument vector; this does not loosen the no-password-in-argv requirement.
- Parent main SHA: 259f9bc5f5574e07bf820c0a685ceefaa72af9cb. The next run must continue through the credential-file ASan/UBSan test.

---

## 2026-11-24 — Remove final legacy-password expectation from managed LAN auth contract

- Process lifecycle run #45507892712 on source SHA 76e6f03167f6265e0ed89d5d86430df2881c24d7 passed earlier inventory/boot gates and reached the RC integration checker, then failed because one old assertion still expected `eval(..., "-s", gateway, "-p", password)`. The managed caller has intentionally moved both branches to private `-f` file input; removed the obsolete assertion while retaining checks for both `-f` branches and absence of password argv.
- Parent main SHA: 76e6f03167f6265e0ed89d5d86430df2881c24d7. Exact Process rerun must reach the credential-file compile/run step.

---

## 2026-11-24 — Restore EAPoL source variable in process integration checker

- Process lifecycle run #45507075779 on source SHA 3e31c744db7c4fd27665ccadc7325058c3c60a83 failed after the checked-read stop assertion was corrected: the checker still referenced `auth_start_eapol`, but the prior replacement had removed its declaration. Restored the declaration without changing EAPoL production assertions.
- The private lanauth credential transport is on main at 4e9d0c25f02e0c83c8657215f7cfa6b7b6ac0120; exact Process rerun, new AI docs, XY-C1 and NVRAM audit remain to be verified.
- Parent main SHA: 4e9d0c25f02e0c83c8657215f7cfa6b7b6ac0120.

---

## 2026-11-24 — Keep LAN authentication password out of managed daemon argv

- lanauth previously accepted only `-p <password>`; the RC service passed the NVRAM secret in argv before the daemon's later optarg overwrite. Added a one-shot credential file transport: RC uses `mkstemp`, exact mode 0600, complete write and `fsync`, then passes only the random pathname with `-f`. lanauth opens with `O_NOFOLLOW|O_NONBLOCK`, validates regular-file type, effective owner, single link, exact mode and inode identity, reads at most the legacy 20-byte password, then unlinks the file before daemonizing. The old `-p` interface remains for manual compatibility; managed router starts now use `-f`.
- Added GNU89 ASan/UBSan tests for valid one-shot consumption, empty/oversized/insecure/missing inputs and symlink rejection, plus source-contract assertions. Process/XY-C1 path filters include lanauth sources. Exact-head checks pending.
- Parent main SHA: dc1d0654022a1cfec415d589f2f4c05fa352f541. No firmware or physical-router result claimed.

---

## 2026-11-24 — Repair LAN authentication integration contract and handoff checkpoint

- Exact-head Process lifecycle run #45506666241 on 3e31c744db7c4fd27665ccadc7325058c3c60a83 reached the aggregate integration check, then failed on a stale assertion requiring `ret = stop_auth_kabinet();` inside `start_auth_kabinet()`. The source now delegates read→stop→start ordering to `auth_kabinet_restart_with_ops()`; the checker now validates checked NVRAM reads, read-before-stop ordering, the launch adapter and secret-buffer clearing.
- Exact-head AI handoff run #45506666102 failed because the added roadmap note raised `REFACTOR_ROADMAP.md` to 131503 bytes, above the 131072-byte hard limit. Removed that redundant bullet and reconstructed `SESSION_HANDOFF.md` from the full parent version so prior completed-source records are retained.
- Parent main SHA: 3e31c744db7c4fd27665ccadc7325058c3c60a83. Exact correction SHA and AI/Process/XY-C1 Actions must be checked after the atomic commit. Next security task remains removing the LAN authentication password from `lanauth` argv.

---

## 2026-11-24 — Preserve LAN authentication when NVRAM reads fail

- start_auth_kabinet() previously used nvram_safe_get() for wan_auth_host / wan_auth_pass and stopped the running lanauth daemon before it could distinguish a valid empty password from a failed NVRAM read. A transient backend failure could be converted into an empty value and interrupt a known-good authentication session.
- Both settings are now copied through nvram_get_checked() and the bounded rc_wan_nvram_copy_value() helper before any stop operation. Read errors and truncation return without touching the active daemon; a successfully read empty password preserves the legacy stop/no-start behavior. The injected policy regression covers each read failure, empty password, stop failure, read→stop→start ordering, start failure and clearing of temporary buffers.
- Process Lifecycle, XY-C1 and AI handoff exact-head checks are wired to the production source, policy header and test. Their exact run results must be checked on the resulting commit. Parent main SHA: b2afc079f262a13aa05ef0d926455b328c480dd6. No firmware/device result is claimed.
- Next security item: start_auth_kabinet() still places its password in the lanauth command-line arguments. Inspect the daemon parser and replace that secret transport with a private file or inherited descriptor in a separate tested slice.

---
## 2026-11-24 — Read FTP credentials under one NVRAM transaction

- `asus_getpwnam()` used separate single-key `nvram_get_int()` / `nvram_safe_get()` calls for account count, username and password. Each getter took and released the NVRAM lock independently, while add/delete/rename/password writers update the same multi-key inventory under `nvram_transaction_begin()`. A login concurrent with a writer could combine a username from one generation with a password from another (or briefly observe a newly published account with a stale tail-slot password).
- Added a coherent credential snapshot under one exclusive NVRAM transaction. The reader checks the bounded account count, requires every active username slot to be present/nonempty, rejects duplicate matches, copies the matching nonempty password before releasing the shared cache, and fails closed if begin/get/end fails. All non-success paths clear the output password.
- Added GNU89 ASan/UBSan regressions for the lock lifetime (including a simulated writer attempting to change the password between username and password reads), absent user, incomplete/ambiguous inventory, missing/empty password, malformed/overflowing count and transaction begin/end errors. The VSFTPD workflow path filters and exact-source checker cover the new header/test and the production NSS callback.
- Parent main SHA: `7d589c2ea0a3f46356e8bc361d212f8a9b81776f`. Exact-head VSFTPD and AI handoff checks must be verified after the atomic commit. This source-only auth change makes no XY-C1 or physical-router validation claim.

---

## 2026-11-24 — Align FTP share listing visibility with read permissions

- `asus_check_permission_at(..., PERM_READ, ...)` already treated share-right 2 as write-only and denied directory access, but `asus_check_file_visible()` accepted every value >=1. A write-only user could see protected share directory names in parent listings even though CWD/read authorization denied access.
- Added one fail-closed `ftp_acl_share_rights_allow_read()` rule and use it for both PERM_READ authorization and share-entry visibility. Only rights 1 (read) and 3 (read/write) are visible; 0, 2 (write-only), negative/error and unknown values are denied. Added GNU89 tests for 0, 1, 2, 3, -1 and 4, plus exact-source assertions for both production callsites.
- Parent main SHA: `07e7c3453ab02d25704c2d6770a1c30d06d771f3`. VSFTPD Stability #45505371869, AI handoff #45505371734, process inventory #45505371860 and NVRAM audit #45505371707 all PASS on source SHA 7d589c2. XY-C1 #45505371723 was still running at the last poll; no target firmware build or physical-router validation is claimed for this FTP-only slice.

---

## 2026-11-24 — Reject unsupported SSH daemon mode values

- `sshd_nvram_mode_status()` accepted any in-range integer, but `start_sshd_with_mode()` implements only modes 0 (disabled), 1 (Dropbear) and 2 (OpenSSH). A corrupt value such as 3 could therefore pass configuration validation and return success without starting a listener; negative modes were silently treated as disabled.
- Reject values outside 0..2 with `EINVAL` without modifying the caller's output. Added GNU89 sentinel-preservation tests for -1, 3, INT_MIN and INT_MAX, and exact-source assertions for the range guard and all boundary cases. The Process Lifecycle Regression workflow already filters these source/test/checker paths and runs the policy test under ASan/UBSan.
- Parent main SHA: `05356ed622d1762e5e251c6adeb5fcd7be7713b6`. Exact-head Process Lifecycle #45504233875, PID 1 SSH policy regression #45504233986, AI handoff #45504233800, process inventory #45504234314 and NVRAM audit #45504234041 all PASS. The two preceding XY-C1 runs #45502092732 (d096) and #45503440356 (6d) also completed Build, TRX and artifact upload successfully; neither run is the 07e7 source SHA. No target MIPS/uClibc runtime on the 07e7 source or physical-router validation is claimed.

---

## 2026-11-24 — Restore the required handoff checkpoint contract wording

- AI handoff documentation #45503784498 on SHA `2be0e5125bb29f01fc3a9a15fe8938a554387452` failed only because the rewritten handoff omitted the validator's exact sentence `The checkpoint is mandatory; a document edit is conditional.`.
- Restored that mandatory no-churn checkpoint statement. This corrects a handoff contract omission; no production source is changed.
- Parent main SHA: `2be0e5125bb29f01fc3a9a15fe8938a554387452`. AI docs rerun is pending after this commit; firewall VTS source checker correction and GNU89 sanitizer regression are running separately on 2be0.

---

## 2026-11-24 — Align VTS source checker with wrapped checked-read call

- Firewall Startup Stability #45503440459 on product SHA `6d5d5376e425587e6f116d09f88cbcc7ac6a05c1` failed before compiling the new policy test: the Python source checker searched for the checked NVRAM call as one line while the C invocation is intentionally wrapped.
- The checker now normalizes source whitespace before validating read/classify/failure/return/use ordering, while retaining checks for both fail-closed ruleset diagnostics and removal of the old lossy `portrange_conv(vts_lport_x)` path.
- The standalone GNU89 `-Wall -Wextra -Werror` ASan/UBSan VTS policy test compiled and passed locally. Firewall Startup Stability #45503784466 PASS on checker-fix SHA `2be0e5125bb29f01fc3a9a15fe8938a554387452`, including the test compile/run and both exact-source checks. XY-C1 #45503440356 for source SHA 6d was still building at last poll.
- Parent main SHA: `6d5d5376e425587e6f116d09f88cbcc7ac6a05c1`. No production source changed in this checker-only correction.

---

## 2026-11-24 — Fail closed when optional virtual-server translation-port NVRAM reads fail

- `include_vts_filter()` and `include_vts_nat()` used `portrange_conv("vts_lport_x", i)` but treated a NULL result as an absent/empty optional translation port. If `nvram_get_checked()` failed while reading a configured internal port, the firewall silently fell back to the external port and could publish a different filter/DNAT mapping (for example, WAN 8080 -> LAN:8080 instead of LAN:80).
- Both VTS builders now use the checked indexed-string reader with presence information. Missing/empty values preserve the legacy external-port fallback; any backend/malformed read result aborts the incomplete filter/NAT staging so the caller retains the previously published ruleset.
- Added GNU89 ASan/UBSan regression coverage for missing, empty, configured, backend-error and malformed reader results, and extended the exact-source checker plus firewall Actions trigger/test step. Parent main SHA: `d096f168bfe4188f5223bf76806b4ea012742580`.
- VPN lifecycle #45502092649, AI docs #45502092664, NVRAM boundary audit #45502092721, process inventory #45502093101, IPv6 service read #45502092750, HTTPD security #45502092764 and OpenVPN credential staging #45502092820 PASS on product SHA `d096f168bfe4188f5223bf76806b4ea012742580`. XY-C1 #45502092732 is still PENDING on that SHA (Build active; TRX/artifact not started at last poll). New firewall, XY-C1 and AI docs exact-head validation are pending after this commit.
- No target runtime or physical-router validation is claimed.

---

## 2026-11-24 — Reject malformed OpenVPN client certificate lifetime arguments

- `ovpn_server_expcli_main()` parsed the optional `days_valid` argument twice with `atoi()`. Numeric-prefix values such as `30junk` silently became 30 days, while values outside the `int` range had undefined conversion behavior before being passed to certificate generation.
- Added a strict GNU89-compatible parser: preserves the historical 365-day default and whitespace/leading-plus compatibility, accepts 1–36,500 days, and rejects negatives, zero, suffix junk, fractions, out-of-policy values and integer overflow before configuration/key-generation side effects.
- Added ASan/UBSan regression coverage and exact-source lifecycle contract; wired the header/test into both VPN workflow path filters. XY-C1 run #45499228265 completed SUCCESS on product source SHA `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a`, with Build, TRX and artifact upload all PASS. PID 1 #45500981047 and AI docs #45500980887 passed on `8ebf9dfc1aadd4dc45aeaa10514086aa97ed74ef`.
- Parent main SHA: `8ebf9dfc1aadd4dc45aeaa10514086aa97ed74ef`. New VPN lifecycle regression, exact-source contract, XY-C1 and AI docs validation are pending on this commit. No physical-router L4 validation is claimed.

---

## 2026-11-24 — Repair PID 1 queue workflow's stale modem restart assertions

- Exact-head PID 1 notification queue run #45499228171 on source SHA `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` passed the bounded-batch, secure-marker, enqueue-status, optional-daemon and SSH GNU89 policy tests, then failed in the final inline source contract. The modem assertions still expected the removed `stop_failed` flag after the USB retry fix had correctly replaced it with `transition_failed` and guarded cache publication.
- Updated this workflow to verify both modem rule/type reload paths, cache-publication and failure ordering, and the final WAN-restart gate. This is a test-contract repair; production source is unchanged.
- Exact-head PID 1 rerun and AI handoff validation are pending on the resulting commit. XY-C1 validation remains in progress on product-source SHA `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` (#45499228265); Build was active and TRX/artifact steps pending at the last poll.
- Parent main SHA: `d2fa77e69aa8017db457d5e8cfd4f9744cebe5a4`. USB modem source fix `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a` and the dedicated caller-contract follow-up `d2fa77e69aa8017db457d5e8cfd4f9744cebe5a4` are already validated by USB #45499733421 PASS and AI docs #45499733544 PASS on d2fa.
- No target MIPS/uClibc runtime or physical-router validation is claimed.

---

## 2026-11-24 — Align USB modem caller contract with retry-safe cache publication

- The exact-head USB modem workflow #45499228398 reached its inline WDM/NDIS caller-graph contract, which still searched for the old `stop_failed` flag. The source transition now uses `transition_failed` so module-readiness failure also blocks WAN restart; the stale contract stopped the workflow before the dedicated modem module checker could run.
- Updated the inline contract to verify both rule/type reloads pass through guarded cache publication, each failure marks the transition failed, and WAN restart occurs only after both failure gates.
- Follow-up parent source SHA: `8a2fbd592e1f0ddbd7a014d97bd00a8e08fb949a`. The previous exact-head run failed only at this stale workflow assertion; the C policy and GNU89 sanitizer tests passed. The rerun must confirm the newly updated caller contract and dedicated checker; XY-C1 build/TRX/artifact remain a separate exact-SHA gate.
- No target runtime or physical-router validation is claimed.

---

## 2026-11-24 — Keep USB modem restart retries effective after failed driver reload

- `RCN_RESTART_MODEM` retained its in-flight marker when driver readiness failed, but it had already advanced the in-memory `modem_rule`/`modem_type` cache and allowed WAN restart to continue. A retry therefore saw the requested values as current, skipped the failed module reload, and could clear the incomplete marker without restoring the selected modem driver.
- Cache publication now occurs only after `reload_modem_modules()` confirms an NDIS/RAS alternative. A failed readiness check keeps the previous baseline, marks the transition failed, and prevents WAN restart; the retained notification can retry the module transition.
- Added GNU89 regression cases for failed/successful cache publication and strengthened the exact-source contract. Exact-head USB modem, Process, AI docs and XY-C1 validation are pending after this commit.
- Parent main SHA: `786c504c8247fe42d5d1d5f2dc3ad28757fafc80`. No target runtime or physical-router validation is claimed.

---

## 2026-11-24 — Reject malformed persisted WAN traffic baselines

- `wanlink_hook()` in `web_ex.c` read persisted `bytes_rx`/`bytes_tx` baselines with `strtoull(..., NULL, 10)`. That accepts numeric prefixes such as `"40junk"` as 40, yielding a plausible but false WAN traffic delta rather than reporting invalid persisted state.
- Added `net_ifstats_counter_delta()` and `get_ifstats_counter_delta_status()`. Empty baselines retain the legacy zero default; signs, trailing junk and overflow are rejected. Counter regression/reset still yields zero. The HTTPD handler treats RX/TX as an atomic sample: if either baseline is invalid, both deltas are cleared and byte status is unavailable.
- Added GNU89 regression cases for valid subtraction, counter reset, empty baseline, numeric-prefix junk, signed/overflowing values, NULL and output initialization. Updated WAN-counter and XY-C1 path filters.
- Parent main SHA: `36ca1aea683ab148d348c15e79a1ddd619498686`. WAN-counter and XY-C1 exact-head workflows remain to be checked; no target firmware runtime or physical-router L4 claim.

---

## 2026-11-24 — Fix HTTPD workflow's deconfig-scope assertion

- On 8f69bb2cbbfe296d3913d3c4b55eb65eb92a2ca4, AI handoff #45497016593 PASS. HTTPD Authentication and Security #45497016612 failed at inline Python line 110 with `NameError: name 'cleanup' is not defined`.
- The production LAN route fix was unchanged and WAN DHCP callback #45496718762 had already passed on 1572f3e5c91ec78de42382e02f8cc40d0e7fd18c. The prior checker correction mistakenly referenced a variable from another workflow. This follow-up scopes `lan_down_auto()` directly from `net_lan.c` and asserts the idempotent delete wrapper inside that function.
- XY-C1 Firmware Validation #45496718759 remained IN_PROGRESS at Build XY-C1 on product source 1572; TRX and artifact upload still needed confirmation. No L3 claim.
- No MIPS/uClibc runtime or physical-router L4 validation.

---

## 2026-11-24 — Sync HTTPD duplicate contract with idempotent LAN DHCP route cleanup

- Exact-head WAN DHCP callback state stability #45496718762 PASS on 1572f3e5c91ec78de42382e02f8cc40d0e7fd18c; the LAN deconfig source checker and existing route-policy regression passed.
- HTTPD Authentication and Security #45496718781 failed at inline assertion 108 because it still required `return route_del(context->lan_ifname...)`, while the LAN BOUND/RENEW callback now wraps that operation with `route_status_idempotent_delete()`. The production fix is intentional; the duplicate static contract is updated here to require the idempotent wrapper and nested route syscall, plus the deconfig wrapper.
- AI handoff #45496718769 and WAN route status #45496719195 PASS on this SHA. XY-C1 #45496718759 remained in ccache setup at last poll; Build/TRX/artifact not yet confirmed.
- No MIPS/uClibc runtime or physical-router L4 validation.

---

## 2026-11-24 — Make LAN DHCP route deletion safe across DECONFIG_PENDING retries

- `lan_down_auto()` removed the saved DHCP gateway with raw `route_del()`. If route deletion succeeded but a later cleanup step failed, BusyBox retried `deconfig`; Linux then returned `ESRCH` for the already-absent route, and the callback aborted before re-running static address/resolver/hosts cleanup. This could strand DHCP teardown in an endless `DECONFIG_PENDING` loop.
- The same raw delete appeared when BOUND/RENEW transaction logic removed an old gateway before applying a replacement. Both call sites now use `route_status_idempotent_delete()`, accepting only `ESRCH` as already complete while preserving `EIO`, `EPERM` and other real errors.
- Extended `scripts/check_lan_dhcp_deconfig.py` to require the helper at both call sites. Existing `route_status_policy.c` exercises the idempotent-delete mapping and confirms other errno values remain failures. WAN DHCP callback CI is the exact-head regression gate.
- Parent main SHA: `1708fe61dbed08802f8c5b4ea29126c4ad6b6ca4`. No target MIPS/uClibc or physical-router L4 validation is claimed.

---

## 2026-11-24 — Close stale HTTPD lease-parser contract; refresh exact validation status

- Commit 7c77568b8148c31604b107972c2d8427e6b04d5d updates the duplicate HTTPD static checker to the current canonical LAN DHCP lease callback. The old assertion demanded a raw 0..INT_MAX parser that was intentionally removed; the checker now requires `lan_dhcp_effective_lease_seconds()`, canonical parsed assignment, the shared one-hour default and the matching regression test, and forbids the obsolete expression.
- Exact-head HTTPD Authentication and Security #45495732037: PASS on 7c77568b8148c31604b107972c2d8427e6b04d5d. AI handoff documentation #45495731574: PASS on the same SHA.
- Parent product-source SHA 402fc1cc455490c9c678be28966bf30f5597e1e3: BusyBox udhcpc event-script stability #45495287614 PASS; WAN DHCP callback state stability #45495287778 PASS; AI handoff #45495287791 PASS. HTTPD #45495287685 failed due the stale duplicated source assertion and was superseded by the corrected passing run above.
- XY-C1 Firmware Validation #45495287536 remains IN_PROGRESS at Build XY-C1 on 402fc; TRX verification and artifact upload have not been confirmed. Do not claim L3 until all three steps PASS on this one source SHA.
- No MIPS/uClibc runtime or physical-router L4 validation is claimed.

---

## 2026-11-24 — Keep LAN NVRAM lease duration identical to the udhcpc timer

- `udhcpc_lan_bound()` previously stored `lease=0` when the option was absent, even though BusyBox armed a 3600-second lease. It also persisted the unnormalized raw option although the client clamps values below 16 seconds and caps values at/above `0x10000000` to `0x0fffffff`. Non-four-byte DHCP lease options are treated as absent by `udhcp_get_option32()`, so the script environment must use the same validity test rather than formatting the raw option.
- Added `udhcpc_lease_time_effective()` as the single fallback/clamp policy used by both ACK timer setup and the `lease=` event-script export. The script receives one canonical numeric duration; missing/invalid DHCP option lengths produce the same one-hour fallback as the client.
- LAN callback now defaults a missing environment value to 3600 and accepts only canonical 16..268435455 values before committing `lan_lease_t`. Noncanonical, multi-value, negative and overflowing values fail the callback rather than publishing a mismatched duration.
- Added GNU89 policy regression cases for no option, short values, both upper-bound edges, `UINT32_MAX`, malformed script values and multi-value formatting. Extended both exact-source checkers and XY-C1 path filters. Parent main SHA: `2a54ebbc7030fb69d2d60ac1443a9fbc76a043f22`. Exact-head udhcpc, WAN DHCP callback, AI handoff and XY-C1 Build/TRX/artifact gates remain pending.
- SCUT watchdog auth-marker/stat correction #45494687100 and AI handoff #45494687184 PASS on this parent SHA; Process lifecycle inventory #45494687146 and XY-C1 #45494687091 were still pending/in progress at last poll.
- No MIPS/uClibc device runtime or physical-router L4 validation is claimed.

---

## 2026-11-24 — Pin BusyBox stat format support for watchdog marker checks

- Confirmed the watchdog's `stat -c` metadata checks depend on both BusyBox STAT and FEATURE_STAT_FORMAT; the board configuration enables each by default. Added an explicit source comment and workflow contract for FEATURE_STAT_FORMAT so an applet configuration regression cannot silently break marker validation.
- Exact parent SHA 0d9b3b51d174ef281a5a379c12c59b29811b9734: AI handoff #45494472680 PASS. SCUT workflow was not retriggered by the checker-only push; the preceding SCUT run #45494178433 had runtime tests PASS and only its static source assertion failed on missing stderr-redirection text. The checker fix is now present; trigger and verify the complete workflow on this source-path commit.
- XY-C1 #45494178359 remained in cross-build at step 38; no TRX/artifact completion yet. No L3 or physical L4 claim.

---

## 2026-11-24 — Correct SCUT watchdog auth-marker source contract

- The extended exact-head SCUT watchdog run #45494178433 reached all runtime tests successfully, including symlink, hard-link and writable-mode auth-marker spoof rejection, but failed the final Python static contract at assertion 58. The checker expected the pidfile stat command without its intentional 2>/dev/null redirection; this was a stale assertion, not a production behavior failure.
- Updated the source-contract literal to include the existing stderr redirection. No C or shell runtime code changed. Re-run the exact-head workflow on the new commit before closing the security slice.
- Parent SHA 495da8f95041bee67025fe52ede7cba526b133c9: AI handoff #45494178404 PASS; Process inventory #45494178432 PASS. XY-C1 #45494178359 had not reached its product-input detector at last poll, so L3 remains unverified.
- No physical-router L4 validation.

---

## 2026-11-24 — Validate SCUT watchdog auth marker inode before restart

- The watchdog still trusted /tmp/scutclient_status with -f plus cat. A user-created symlink or hard link containing "1" could make the privileged watchdog run the SCUTClient restart path even though the default writer had been changed to atomic publication.
- Added watchcat_auth_flag_enabled(): reject symlinks, non-regular files, non-current-service-owner files, multi-link inodes, non-two-byte values and group/other-writable modes. It uses BusyBox stat -c (STAT and FEATURE_STAT_FORMAT default enabled) and compares marker UID with the watchdog-owned PID-file UID. Since /tmp is sticky, an unprivileged user cannot replace that validated root-owned inode. Root-owned custom hooks producing mode 0644 remain compatible.
- Added an injectable ping executable for deterministic tests and regression cases for symlink, hard-link and writable-mode spoofing. The SCUT watchdog workflow runs the expanded shell test and statically checks the new trust boundary.
- Parent exact SHA 64a2e4b7147f01a85f0068189b94e4ec552ca22b: SCUT watchdog status writer/PID suite #45493427133 PASS; AI handoff #45493427136 PASS; Legacy wrapper #45493427241 PASS; Process inventory #45493427332/#45493427337 PASS. XY-C1 #45493427214 was still building at step 38; TRX/artifact pending. Do not claim L3.
- No physical-router L4 validation.

---

## 2026-11-24 — Publish SCUTClient auth status without following /tmp symlinks

- Default SCUTClient -E/-Q event hooks previously executed shell redirection to /tmp/scutclient_status. Since the daemon is launched by the privileged service wrapper, a local user could pre-create a symlink at that shared /tmp path and redirect the privileged write into an unrelated file.
- Added scutclient_status_file.sh: validate status 0/1 and an absolute destination; stage in the same directory under umask 077 with mktemp; enforce mode 0600; atomically rename into place. Existing file symlinks are replaced as directory entries rather than followed; directory targets are rejected. Explicit user-supplied NVRAM hooks remain unchanged.
- Updated default success/failure hooks to call /usr/bin/scutclient_status_file.sh 0/1. The SCUTClient Makefile already installs scripts/ wholesale into /usr/bin.
- Added a shell regression for normal publication, 0600 mode/single link, symlink-target preservation, hard-link inode preservation, directory destinations, invalid arguments and temporary cleanup. The SCUT watchdog safety workflow now runs shell syntax, the fixture test and exact-source contract checks.
- Parent SHA 347628c7535656c9d80bb466f8271037e1e24a55: QMI reader workflow #45491981121 PASS; AI handoff #45491981194 PASS; bounded NVRAM audit #45491981203 PASS; Process inventory #45491981328/#45491981119 PASS; WAN teardown #45491981147, WAN DHCP callback #45491981235, detect_link #45491981246 and HTTPD auth/security #45491981225 PASS. XY-C1 workflow #45491981249 still had Build XY-C1 step 38 in progress; TRX and artifact upload were pending.
- Wireless correction SHA c3d9f29fc94e81e1828c4c854f46658d2111ec3c: exact-head Wireless #45491054451 and AI handoff #45491054663 PASS. No product inputs changed on c3d9, so XY-C1 host-only detector skipped Build/TRX/artifact.
- No physical-router L4 validation is claimed.

---

## 2026-11-24 — Protect USB modem QMI status reads from /tmp precreation

- get_qmi_handle() previously used fopen() on /tmp/qmi-client-id and /tmp/qmi-handle, which follows symlinks and can block indefinitely on a pre-created FIFO. A forged PDH marker can make WAN startup believe a QMI session already exists; a forged WDS CID can affect later teardown.
- Added modem_qmi_status_file_read(): O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC open, fstat validation for a regular euid-owned 0600 single-link file, bounded binary-safe reads, embedded-NUL and oversize rejection, and the existing strict numeric parser. The output handle resets to -1 on every failure.
- Added GNU89 regression coverage for metadata ownership/mode/type/link-count policy, valid and malformed numbers, symlinks, nonblocking FIFO rejection, hard links, embedded NUL, oversized data, relative/missing paths and directories. The USB modem workflow compiles this test with -Wall -Wextra -Werror and ASan/UBSan and checks the source contract.
- Exact-head USB modem and XY-C1 Actions for this source commit are pending immediately after push; do not claim CI or firmware build completion until each reports PASS on the same SHA.
- The preceding Wireless checker correction on c3d9 normalized declaration whitespace only. Its run #45491054451 was still in progress when this QMI task started. NVRAM transaction audit #45490305501 completed PASS on 046. No physical-router L4 claim.

---

## 2026-11-24 — Harden wireless radio status reads and notification outcomes

- PID-1 radio notification and manual-toggle paths used legacy `is_radio_on_wl/rt()` readers that call lossy `get_mlme_radio_wl/rt()` on IWPRIV/RT3352-MII boards. An NVRAM read error becomes zero, so explicit OFF may look like an already-satisfied no-op and be reported as success.
- Added checked `wireless_radio_power_status()`, negative-status propagation from radio control, checked `wl_radio_x`/`rt_radio_x` enable reads for manual toggles, and a notification-result helper that preserves state and operation errors. Added GNU89 policy regressions for failed state snapshots and operations when the old/fallback state equals the requested state. Updated Wireless workflow source contracts.
- Exact-head follow-up: Wireless run #45490305618 reached the source-contract block after its GNU89/ASan/UBSan regressions passed, then failed because the checker expected one space in the `rc.h` return-type alignment while the valid declaration uses two. The workflow assertion now compacts whitespace before comparing the signature; production code is unchanged by this checker correction. Re-run the new exact-head job before treating the slice as closed.
- Exact 717 SHA: Wireless #45489316780 PASS; NVRAM audit #45489317079 PASS; WAN DHCP #45489318671 PASS; VSFTPD #45489317086 PASS; udhcpc #45489316871 PASS; Process inventory #45489318666 PASS. XY-C1 #45489316847 had steps 1–37 PASS with Build step 38 IN_PROGRESS and TRX/artifact pending at last poll; no L3 claim.
- Exact cdf documentation child AI handoff #45489793605 failed because the handoff lacked the exact phrase `one atomic main commit`; the updated SESSION_HANDOFF now includes that phrase and the checkpoint section. Verify the next exact-head documentation run.
- The NAK tree-recovery issue is closed: ff9's incomplete tree was followed by 717, which restored the root tree and retained the four intended DHCP changes. Compare 054..717 shows only those four paths modified. No history rewrite or backup modification.
- No physical-router L4 test is claimed.

---

## 2026-11-24 — Accept authoritative DHCPNAK replies during REBINDING

- `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c` now passes the current state to the server-ID filter. DHCPNAKs during REBINDING may come from any server reached by the broadcast DHCPREQUEST; a missing server-ID is still rejected. SELECTING/RENEWING retain strict original-server matching, and the legacy no-known-server behavior remains unchanged.
- Added GNU89 policy cases for alternate-server acceptance in REBINDING and missing/mismatched IDs outside that state; updated the source-contract checker. Exact-head udhcpc regression #45489316871 on `717f983f07434cd78a4ad00ebdb944b8f6d30fa5` PASS (compile/run with `-Wall -Wextra -Werror` + ASan/UBSan and source-order contracts).
- Current exact-head checks: Process inventory #45489318666 PASS; Router runtime NVRAM reset #45489318744 PASS; MiniDLNA #45489318793 PASS; DNSMasq #45489318703 PASS; EAPoL #45489318738 PASS. XY-C1 #45489316847 was at production-input detection (step 37), with Build/TRX/artifact pending; WAN DHCP callback #45489318671 queued; VSFTPD #45489317086 in progress. AI handoff #45489318581 failed because the current handoff lacked the mandatory end-of-session checkpoint section; this entry and handoff repair are in the consolidated documentation checkpoint.
- Git object recovery note: source commit `ff9cb0ec0f9143cd398d8de1d9b4eb13dca87be5` was created with an empty tree base and therefore omitted unrelated paths. This was caught by inspecting its GitHub commit statistics before claiming validation. Non-forced recovery commit `717f983f07434cd78a4ad00ebdb944b8f6d30fa5` rebuilt the tree from the actual parent tree; REST comparison from `054ff22bf517a45bf29f2da21963a4206f8b3b83` to `717f983f07434cd78a4ad00ebdb944b8f6d30fa5` confirms exactly four modified files (the DHCP source, policy header, regression test and checker), with no other paths changed. Current main has the full 63,997-entry repository tree and the DHCP CI above passes. No force update was used; the frozen backup is unchanged.
- The earlier exact source baseline `7e700e86e191bfbf11d563fce7f9cfbbcd539516` has XY-C1 #45486634245 PASS through Build, TRX and artifact upload, and NVRAM audit #45486634206 PASS (bounded userland/kernel caller inventory; no global atomicity claim). That L3 result belongs to the 7e source SHA only. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-24 — Verify VSFTPD quarantine confinement on exact main SHA

- Exact main SHA `7e700e86e191bfbf11d563fce7f9cfbbcd539516`: VSFTPD regression #45486633987 PASS. The GNU89 `-Wall -Wextra -Werror` policy binaries and embedded Python source contracts passed, including the symlinked-quarantine alias regression for write-parent, rename-entry and directory resolution.
- AI handoff #45486633983 PASS; Process lifecycle inventory #45486634249 PASS.
- NVRAM audit #45486634206 remained PENDING at step 13 (raw ioctl and kernel API callers); steps 3–12 had passed. XY-C1 #45486634245 remained PENDING at step 38 (Build XY-C1); steps 1–37 passed, with TRX verification and artifact upload still outstanding. These results are bound to the exact 7e SHA only; no L3 or L4 claim.
- Replaced stale handoff claims from earlier 77/d01 failures with the current exact verification boundary and next actions. Frozen backup unchanged.

---

## 2026-11-24 — Close the rename-destination quarantine alias gap

- Exact SHA `77e281ea0d165520fd5782e04550fcfd7a7293dd` VSFTPD #45486387054 compiled and ran the existing C policies. The new symlink-alias regression first passed the write-parent fence, then exposed that `ftp_acl_path_resolve_rename_entry()` still returned success because the canonical-parent check had not landed in that function.
- Added the missing `ftp_acl_path_is_internal_metadata(resolved_parent)` check in the rename resolver itself, returning EACCES before any caller can open or rename through a symlinked quarantine parent. The regression already asserts this exact behavior.
- VSFTPD #45486387054 therefore remains FAIL; AI handoff #45486386998 PASS. Exact-head VSFTPD rerun and same-SHA XY-C1 Build/TRX/artifact validation remain pending. No L3/hardware claim.
 
---

## 2026-11-24 — Block symlink aliases into the VSFTPD cleanup quarantine

- Security review found a canonicalization bypass in the new hidden quarantine: lexical checks blocked direct `.__vsftpd_cleanup_*` paths, but a symlinked parent with a missing final leaf passed through `ftp_acl_path_resolve_parent()` without rechecking the canonical parent. New-file uploads and rename destinations could therefore resolve into the internal quarantine through an alias.
- Added canonical reserved-metadata checks after parent resolution in `ftp_acl_path_resolve_parent()` and `ftp_acl_path_resolve_rename_entry()`, and after canonical directory resolution in `ftp_acl_path_resolve_directory()`. Existing-final-target checks were already canonical and remain unchanged.
- Added a GNU89 regression that creates a symlink alias to the owned 0700 cleanup directory and requires write-parent, rename-entry and directory resolution to fail with EACCES. The VSFTPD workflow now pins all three canonical-path fences and regression markers.
- At the previous exact heads, VSFTPD #45485681381 and AI handoff #45485681367 passed on `ebb67734c1eae61e1835806eea689187801c1a2d`; Process inventory #45485311380 and NVRAM #45485311362 passed on `4ee6d7bc1b55c81bab7022e160397c7f7da297d6`. XY-C1 #45485311337 for 4ee was still in progress at last poll. This security slice now needs fresh exact-head tests and product build.
- No local compiler, MIPS build or router test is claimed. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-24 — Make VSFTPD quarantine source contract whitespace-safe

- Exact source SHA `4ee6d7bc1b55c81bab7022e160397c7f7da297d6` VSFTPD run #45485311376 passed GNU89 compilation and the C ACL-path regression, then failed at the embedded Python contract. Traceback line 594 mapped to a formatting-sensitive assertion requiring `fstatat(cleanup_fd, quarantine_leaf, &quarantined, AT_SYMLINK_NOFOLLOW)` on one source line; the production call is split over two lines.
- Updated the checker to collapse whitespace in the extracted cleanup function before matching the complete call. This preserves the full semantic check and does not relax the quarantine/inode verification requirement.
- Exact-head VSFTPD rerun is required. On 4ee, AI handoff #45485311277 and Process lifecycle inventory #45485311380 passed; XY-C1 #45485311337 and NVRAM #45485311362 were still running at last poll. No L3 claim.
 
---

## 2026-11-25 01:34 CST — Fix duplicate declaration left by quarantine-helper replacement

- Exact parent `d01e302d2d22365bf1f828b4708dd6fb22ac5234`, VSFTPD run #45485007449 failed during GNU89 `-Wall -Wextra -Werror` compilation before executing the ACL regression. The cause was mechanical: replacing functions starting at the function name left the original preceding `static int` declaration in place, producing duplicate declaration specifiers for the metadata predicate and the new quarantine-directory helper. No runtime assertion was reached.
- Removed the redundant declaration and obsolete best-effort TOCTOU comment. The quarantine implementation remains unchanged; this is a compile-only correction, and a new exact-head workflow must verify the actual binary and Python contracts.
- Parent a35ae98 process inventory / XY-C1 workflows were pending at last poll; AI handoff #45484687791 passed. Earlier f5b01d9 storage-hotplug #45484032665 and XY-C1 #45484032783 passed for that exact SHA.
- No local compile or device test is claimed. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-25 01:28 CST — Close the VSFTPD failed-upload unlink race with quarantine rename

- Parent main SHA: `a35ae98adefb4d90e6deba8ade947c90e348c5d9`. The previous `ftp_acl_path_unlink_opened_entry()` compared the upload descriptor to `fstatat(parent_fd, leaf)` and then unlinked that public name; a concurrent replacement between those syscalls could make cleanup delete another writer's file.
- Replaced the check/delete pair with atomic `renameat2(RENAME_NOREPLACE)` into a hidden per-effective-UID quarantine directory. The directory is created or validated as an owned 0700 directory, and path policy blocks the reserved component anywhere in client paths. The moved inode is checked through the pinned quarantine directory FD; only a matching regular file is unlinked there.
- A mismatch or unlink error attempts a no-replace restore. If a concurrent writer has occupied the public leaf, the quarantined item is retained instead of deleting either object. If renameat2 is unavailable, cleanup fails closed; there is no racy fallback.
- Extended GNU89 ACL path tests for private directory ownership/mode, path exclusion, normal partial-upload deletion and replacement restoration. Updated the embedded VSFTPD Python contract to forbid public-leaf fstatat/unlinkat in the cleanup helper.
- Current parent `a35ae98adefb4d90e6deba8ade947c90e348c5d9`: AI handoff #45484687791 PASS; Process lifecycle inventory #45484687868, XY-C1 #45484687835, NVRAM #45484687920, DNSMasq #45484687914, button timing #45484687913, crond #45484687872, Process inventory/lifecycle combined #45484688144 and link detection #45484687806 were in progress at last poll. Previous f5b01d9 storage-hotplug #45484032665 and XY-C1 #45484032783 both PASS; this is exact evidence for f5b01d9 only.
- No local compilation, MIPS build or router test is claimed. Exact-head VSFTPD regression is required, with same-SHA XY-C1 Build/TRX/artifact for L3. Hardware L4 remains NOT VERIFIED.

---

## 2026-11-24 — Preserve exact 15-byte comm names in process inventory

- L1 review found an off-by-one in `proc_parse_stat()`: a `comm` value exactly 15 bytes long fits the 16-byte buffer including its terminator, but the old >= comparison marked it truncated. This could cause the scanner to miss a valid maximum-length process name when cmdline is empty or differs.
- Changed the truncation boundary to > 15 and added an injected GNU89 regression for a 15-byte process name with an empty cmdline. The Process workflow source contract now requires the corrected boundary and the regression case.
- No local compiler run or MIPS build is claimed. On parent f5b01d9, storage-hotplug #45484032665 and XY-C1 #45484032783 were still in progress; AI handoff #45484032630 passed. Frozen backup remains read-only.

---

## 2026-11-25 — Align storage safe-remove FTP lifecycle checks

- Exact SHA `d91dd33195eb32b29e2250c9cec4cd1604e50b77` storage-hotplug #45483595415 passed all compiled storage/FTP regressions and the full embedded caller-graph check. Its final standalone `scripts/check_storage_safe_remove.py` step failed at line 578 due to two duplicate contracts still calling `body(storage, "stop_ftp_locked(void)")`.
- THIS checker-only commit updates both duplicate checks to the new `stop_ftp_locked(int *final_presence)` signature and requires the final tri-state process result to be exported separately while the stop postcondition remains strict. No production source or C behavior changed.
- Exact-head storage-hotplug rerun pending; b3 NVRAM #45482945421 and Process lifecycle #45482945410 PASS; b3 XY-C1 #45482945636 was still running at last observation. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-25 — Align storage-wide FTP caller graph with final process state

- Exact source b3f165ef4e08b585cc5a77ccdfb791165ba60496 VSFTPD run #45482945754 failed source assertions while its GNU89 C policy binary passed. Checker-only descendant 155a8be964e1ceb1ce82583df241be2248d1ec9d VSFTPD #45483093422 and AI docs #45483093417 both PASS.
- The same b3 SHA storage-hotplug run #45482945464 passed compiled policy/storage regressions, including FTP tri-state presence and stop-postcondition tests, but failed the Python whole-caller-graph step at a stale no-argument stop_ftp_locked signature. THIS commit updates the locked-definition matcher and asserts final-presence propagation plus firewall ordering in grouped stop and FTP restart. No product source changed.
- Exact-head storage-hotplug rerun and handoff Action pending. Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

---

## 2026-11-25 — Align VSFTPD source checker with final-presence stop output

- On exact source SHA `b3f165ef4e08b585cc5a77ccdfb791165ba60496`, VSFTPD workflow #45482945754 was triggered. A static pass over its embedded Python contract found three assertions still looking for the old no-argument `stop_ftp_locked()` and one ordering assertion still matching `storage_service_stop_complete(status, is_ftp_run())`.
- THIS checker-only commit updates those expectations to the final-presence output parameter and the new stop observation order. No product source or C regression was changed.
- Exact-head VSFTPD Actions for THIS correction are pending. The b3 source commit's triggered dependent workflows were also pending at last observation; inspect each by exact SHA and do not transfer PASS status between SHAs.

---

## 2026-11-25 — Reconcile FTP firewall after confirmed absence despite stop-command error

- Confirmed source-control defect: `stop_ftp_locked()` can return failure when `kill_services()` reports an error even though the final procfs inventory proves `vsftpd` absent. Standalone stop/restart previously gated firewall refresh on status==0, skipping FTP/UPnP cleanup after that partial stop failure.
- The locked stop now exports the final tri-state process state separately. Standalone stop, restart (including its early stop-failure branch) and FTP/Samba group stop reconcile only when final presence is PROCESS_PRESENCE_ABSENT. RUNNING/UNKNOWN/invalid never authorize cleanup; the original stop error remains the first returned failure, so failed restart/eject does not proceed.
- Added GNU89 policy regression for ABSENT/RUNNING/UNKNOWN/invalid and source contracts for the failed-stop branch, successful stop and grouped stop. No local compile/MIPS build is claimed; exact-head VSFTPD Actions pending.
- Prior DHCP L3 closure: source SHA `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e` XY-C1 #45480041423 Build/TRX/artifact PASS; Process #45480041644/#45480041481, USB #45480041346, NVRAM #45480041721 and AI handoff #45480041430 PASS. Descendant udhcpc gate `b2dab6b05fb2cf07abbdae01b1acff2cc134df31` #45480533291 PASS; docs descendant `f43b856869a0ad94bae5fd15a1e14daff1f8ec1f` #45480803456 PASS. Frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807` remains unchanged.

---

## 2026-11-25 04:32 CST — Repair blocking handoff contract literal

- Exact SHA `b2dab6b05fb2cf07abbdae01b1acff2cc134df31`: udhcpc #45480533291 PASS (GNU89 ASan/UBSan regression and Python source contract). AI handoff #45480533158 failed because the live handoff expressed the atomic-tree/commit rule but omitted the validator's required literal phrase `one atomic main commit`.
- THIS docs-only commit adds that exact rule to the checkpoint. The change is necessary to restore the required handoff workflow; no product source changed. It also records the udhcpc gate's PASS and preserves the exact pending verification boundary.
- Prior source SHA `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e`: Process inventory #45480041644, Process lifecycle #45480041481, USB storage #45480041346 and NVRAM #45480041721 PASS. XY-C1 #45480041423 was still IN_PROGRESS at last observation. Product-source L3 baseline remains `36a10c0e5844daf31b3cc483c20d9e25a798ef6b`; hardware L4 NOT VERIFIED.

---

## 2026-11-25 04:29 CST — Normalize conditional branch token in DHCP checker

- Exact main SHA `c522dc20b504eac7a7c701d341034752ac3f0ec1`, udhcpc run #45480285457: GNU89 `-Werror` ASan/UBSan policy binary PASS; Python checker failed at line 276 because the code compacts whitespace but the expected token still contained the space in `else if`. The actual compact spelling is `elseif(state==DECLINE_PENDING)`.
- Corrected the source-contract token only. The test still requires the DECLINE_PENDING resume branch and the following non-discovery path to close the listener and use `INT_MAX`; no production C or regression behavior changed.
- Exact SHA `c522dc20` AI handoff run #45480285572 was IN_PROGRESS at last observation. On parent `fd59df3`, AI handoff #45480041430, Process inventory #45480041644, Process regression #45480041481 and USB storage #45480041346 PASS; NVRAM #45480041721 and XY-C1 #45480041423 were still in progress. New exact-SHA udhcpc/handoff runs are required for THIS checker correction.
- Prior L3 source baseline remains `36a10c0e5844daf31b3cc483c20d9e25a798ef6b` (XY-C1 #45476128451 Build/TRX/artifact PASS); the `fd59df3` DHCP product changes need same-SHA target build evidence. Hardware L4 NOT VERIFIED.

---

## 2026-11-25 — Align DHCP retry scheduler checker with tested plan

- Exact main SHA `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e`, udhcpc run #45480041494: GNU89 -Werror ASan/UBSan policy regression PASS; Python source checker failed at line 238 because it still expected `udhcpc_retry_delay(retry_timeout)` to appear directly in the scheduler after the call moved into `udhcpc_deconfig_retry_plan_init()`.
- THIS COMMIT updates the stale source assertion to match the new scheduler/plan integration. The independent source-order checks still pin plan initialization, LISTEN_NONE, state publication and timer/counter publication; no DHCP product C code or test semantics changed.
- Parent AI handoff #45480041430 PASS. Exact-head udhcpc rerun pending; prior-SHA target-build and integration runs remain separate. L3 baseline `80f1bfccd255536a0a2682c7d046f0216038a922`, hardware L4 unverified.

---

## 2026-11-25 — Make DHCP deconfig retry state and timer planning testable

- Extends the active WAN DHCP transition slice by extracting the pending-cleanup resume tuple into a pure GNU89 policy plan: resume state, release intent, retry timeout (including the one-second floor), packet count and elapsed-wait counter. The event loop prepares the tuple before closing its listener, then publishes the saved transition and timer only after switching to LISTEN_NONE.
- Added regression cases for normal retry, zero/negative retry intervals and release-intent normalization. Expanded source contracts for the pending transition publication order, INIT_SELECTING versus non-discovery resume socket/timer setup, lease-expiry gating, SIGUSR2 release-on-pending and -R exit cleanup. Wrong-server DHCPNAK filtering remains required.
- Parent `98168ad16689cf47e1efeb2ee3999374790cbc41`; THIS COMMIT changes only DHCP production/header/test/checker and the active handoff/log. No local checkout is available, so no local compiler run is claimed. Exact-head GNU89 ASan/UBSan, Python contracts and XY-C1 Build/TRX/artifact remain pending; L3 baseline remains `80f1bfccd255536a0a2682c7d046f0216038a922`, L4 unverified.

---

## 2026-11-25 — Normalize the udhcpc retry-helper return contract

- Exact main SHA `0db869b438d344406375dde71ee8b1764b047272`, udhcpc run #45476513525: the GNU89 -Werror ASan/UBSan policy binary compiled and passed; the Python source contract failed at its retry-helper return assertion. The checker compares `policy_compact`, which removes all whitespace, but the expected token still contained a space after `return`.
- THIS COMMIT changes that one checker expectation to the compact spelling `returnudhcpc_retry_delay(retry_timeout);`. It does not change DHCP production behavior or relax the state/order assertions.
- Parent AI handoff run #45476513417 PASS. Exact-head udhcpc and any triggered source/firmware checks for THIS COMMIT are pending; no local test was run because this session has no repository checkout. L3 remains limited to the earlier verified production SHA `80f1bfccd255536a0a2682c7d046f0216038a922`; hardware L4 is not verified.

---

## 2026-11-25 — Correct shared retry-delay checker whitespace normalization

- Exact 36a10c0e5844daf31b3cc483c20d9e25a798ef6b udhcpc workflow #45476128312 compiled and ran the GNU89 ASan/UBSan C regression successfully, then failed its Python source checker on a typo in the new compact-source assertion: whitespace normalization removes the space after the C return keyword.
- Corrected the assertion to match normalized C source. This was a checker-only failure; no C test or product code failure was reported.
- Parent da2 exact workflow state at correction time: udhcpc #45475643463 COMPLETED (SUCCESS); AI docs #45475643556 COMPLETED (SUCCESS). Exact retry on this checker fix is required.

---

