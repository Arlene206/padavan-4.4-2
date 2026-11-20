## 2026-11-19 — Fix missing-original rollback fixture cleanup

- Parent SHA: `740c82cee671e5bcc133289d9126f75afd654ec3`.
- Process lifecycle run `44654002485` on `740c82c` reached the new password transaction regression and failed only in `fixture_cleanup()`: the missing-original test reused a backup path that `fixture_init()` had already populated with a hard link, then the helper correctly cleared the path when the source file was absent. The fixture lost the pathname of its old backup link, leaving the temporary directory non-empty.
- The test now explicitly removes the fixture's original backup and allocates a fresh unused `XXXXXX` backup template before removing the source file. Production transaction code is unchanged.
- Exact rerun required for Process lifecycle and HTTPD checker failures discovered on `740c82c`; inspect HTTPD failure separately because it is outside the account transaction source path. XY-C1 Build/TRX/artifact on `740c82c` was still in progress.
- Host test result: not yet re-run locally; exact Actions regression is pending. No physical-router L4 evidence.

---

## 2026-11-19 — Roll back partial Unix account database publication

- Parent SHA: `4d97400bd600b8503082bc749d577e8beee10963`.
- Followed the pending passwd/group rename fault-injection task. `recreate_passwd_unix()` staged complete files but published them with sequential `rename()` calls; if group or shadow publication failed after passwd (or passwd+group) had been replaced, the live account database could be left mixed with no recovery copy.
- Added same-filesystem hard-link recovery points for the existing regular passwd/group/shadow files and a reusable publish transaction. A later rename failure restores already-published destinations in reverse order; when restoration itself fails, the specific recovery link is retained and logged for manual recovery. Missing originals are handled by unlinking newly created destinations during rollback.
- Expanded the GNU89 ASan/UBSan policy regression with successful multi-file publication, injected second-rename failure, injected rollback failure/recovery-link retention, and missing-original rollback. Extended the Process workflow source contract to require the production caller and all rollback cases.
- Exact-head Process/XY-C1 Actions are pending after this production-source change. The new header/test are already included in Process and XY-C1 path filters. No physical-router L4 evidence.

---

## 2026-11-19 — Isolate ZeroTier identity-staging test cleanup from lifecycle assertions

- Parent SHA: `4c90601aa7e367ce959f78cce712b73441357dce`.
- Exact ZeroTier run `44652711847` on `bfe6342` reached the existing failed-stop gate and failed because the new identity helper's `EXIT` cleanup uses the test's `rm` double, which intentionally logs `rm -rf` actions. The production stop path itself did not mutate firewall/routes/config; the fixture had not reset its action log after the newly added identity writer cases.
- The regression now clears that test-double log before entering the existing stop-failure scenarios. No production code or lifecycle assertions were changed.
- Exact-head ZeroTier rerun, AI handoff docs check and XY-C1 Build/TRX/artifact validation are pending. The earlier exact-head Process inventory and WAN route runs on `bfe6342` passed (`44652711855`, `44652711813`); the AI docs run failed only because a concurrent history-restore commit had not yet reached its workflow checkout, then `44652852687` passed on `4c90601`.
- No L3/L4 claim for the ZeroTier writer change until its exact-source XY-C1 Build, TRX verification and artifact upload complete.

---

## 2026-11-19 — Harden privileged ZeroTier identity and network join writers

- Parent SHA: `df94216f2418bfb3a099aa55f847d7438a9e1f09`.
- Confirmed root-owned path traversal in `add_join()`: NVRAM-sourced `zerotier_id` was expanded unquoted into `touch $config_path/networks.d/$1.conf`. Identity keys were also written directly to persistent paths without enforcing private permissions or refusing symlink destinations; confidentiality depended on inherited umask.
- Added strict optional 16-hex-digit network-ID validation, quoted/contained join-file creation and symlink-leaf refusal. Startup now prepares config and networks.d directories as mode 0700, rejects symlink/non-directory paths, and stages identity outputs in a private same-filesystem directory before chmod-0600 and atomic per-file publication. The launcher sets umask 077 and aborts on preparation failures.
- Extended the shell lifecycle regression for permissive umask, directory/file modes, symlinked config paths, path traversal, malformed IDs, symlink leaves, generated/stored identity, failed public-key derivation and preservation of existing identity files. The Python source contract now asserts the production writer invariants.
- Local execution of these host regressions was unavailable in this environment. Exact-head ZeroTier lifecycle and XY-C1 Build/TRX/artifact Actions are the authoritative pending validation. No L3/L4 claim is made here.
- Exact-head CI: PENDING at commit creation. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Secure Unix password staging and propagate boot provisioning failures

- Parent SHA: `ae7874833c6cb870162222f415124d1db4e63dda`.
- Confirmed `change_passwd_unix()` used predictable `/tmp/tmpchpw` with `fopen("w")`; a pre-created symlink under world-writable `/tmp` could redirect a privileged credential write. The helper also ignored temporary-file, write/close, `chpasswd`, unlink and shadow-permission failures.
- Replaced password staging with exclusive `mkstemp()` mode-0600 creation, account/password delimiter validation, checked write/flush/fsync/close, command status, unlink and shadow chmod.
- `recreate_passwd_unix()` now stages passwd/group/shadow contents before per-file atomic rename, keeps root shadow locked until `chpasswd` succeeds, and returns the first provisioning error. `init_router()` includes this status in the boot DEGRADED result.
- Added a GNU89 ASan/UBSan host regression for unique exclusive files, exact permission modes and invalid templates. Process source contract covers production helper and PID 1 caller. Local sanitizer test passed.
- Previous wireless slice: Process `44649240357` and AI docs `44649240209` PASS on `ae787483`; production-source XY-C1 `44648971016` PASS on `9f565b1`, including Build, TRX and artifact upload (L3 for that exact SHA). L4 remains NOT VERIFIED.
- New exact-head Process/AI docs/XY-C1 Actions pending. Paired passwd/group rename rollback remains a focused fault-injection follow-up.

---

## 2026-11-19 — Make wireless boot source contract whitespace-normalized

- Parent SHA: `9f565b124635a0c3d0686636b4dc0313a5f3a298`.
- Before the exact-head Process source-contract step completed, a manual Python syntax check caught the initial capture string's over-escaped tabs/newlines and quote sequence. The workflow assertion now checks a whitespace-normalized C body, avoiding brittle newline/tab escaping while still requiring each of the six generator callsites to be assigned and recorded.
- The policy regression binary had already passed on the parent. No production C or policy semantics changed in this follow-up.
- The parent Process run `44648970780` was still in progress at last poll; XY-C1 `44648971016` was also in progress with Build/TRX/artifact steps pending. Fresh exact-head runs are required for this checker correction.
- Production-source L3 evidence remains limited to `61cbc6b842fdaaafc3f2fcab269bb11292fa8514`; no physical-router L4 claim.

---

## 2026-11-19 — Preserve wireless configuration publication failures during boot

- Parent SHA: `bd36a1c9da40c6f58aa810f1f1d38f70b06cf50d`.
- Confirmed `init_bridge()` discarded six `gen_ralink_config_2g/5g()` return values. Those functions return `-1` when atomic config publication fails; wireless restart preflight already aborts on the same failure, but boot initialization had no result propagation.
- Captured and recorded both radio-band config preparation branches and both post-start config restores in the bridge first-error accumulator. Independent bridge setup continues, but `init_bridge()` now reports any config publication failure to PID 1.
- Extended `bridge_init_policy.c` with a failed wireless config generation case and added Process source-contract assertions for all callsites. Added the regression test to Process and XY-C1 workflow path filters.
- Local GNU89 / `-Wall -Wextra -Werror` ASan+UBSan bridge policy regression passed. Exact-head Process/AI docs/XY-C1 Actions pending after commit; XY-C1 must execute Build, TRX verification and artifact upload on this production-source SHA.
- L3 from `44644448761` is limited to the older production source `61cbc6b842fdaaafc3f2fcab269bb11292fa8514`; checker-only run `44646517924` skipped all production build steps. Physical router L4 remains NOT VERIFIED.

---

## 2026-11-19 — Pin each ZeroTier firewall deletion in the stop regression

- Historical ZeroTier lifecycle run `44518868454` on `b1b621` failed at the normal-stop assertion that counted exactly five firewall delete commands. The fake firewall-state empty assertion was already present, so this test-only follow-up now checks the exact three filter deletes and two table-qualified NAT deletes individually.
- The existing exact-five delete count and empty firewall-state checks remain in place; per-rule failure prints the full command trace and remaining state. No ZeroTier production source changed.
- Current parent SHA: `d246ddf7a04fa8a678c95cf08f7292968e467e8c`. Exact-head ZeroTier lifecycle, AI handoff docs and XY-C1 validation are pending for this test commit.

---

## 2026-11-19 — Correct storage policy-header newline parser

- Storage run `44646265074` on `49ed539` passed the full caller graph and then failed `check_storage_safe_remove.py` at the check for `storage_hotplug_completion_status()`. The helper header is formatted as `static inline int` followed by a real newline; the checker again searched for a literal backslash-n because its source string contained `\\n` rather than `\n`.
- Corrected the adjacent policy-header needle. The prior helper-function parser is already fixed; no production behavior or policy assertions are weakened.
- NVRAM three-state readiness audit `44645672301` PASS on `133ccb9`. Product-source XY-C1 `44644448761` PASS on exact SHA `61cbc6b842fdaaafc3f2fcab269bb11292fa8514`, including firmware build, TRX verification and artifact upload (L3 for that SHA).
- Parent SHA: `49ed53969e6e14055c7f278f9545c8661a70a7b7`.

---

## 2026-11-19 — Fix storage checker newline parsing and close exact-SHA evidence

- Storage run `44645869851` on `479516c` passed the storage caller-graph assertions, then exposed a checker parsing typo: `check_storage_safe_remove.py` searched for the two-character sequence `\\n` instead of a real newline in the signature of `queue_storage_hotplug_notification()`. The production helper and queue-status semantics are correct; fixed only the checker signature.
- NVRAM audit `44645672301` on `133ccb9` PASS. The PID1 prerequisite gate and three-state READY/DEGRADED contract now have exact source coverage.
- XY-C1 run `44644448761` on production source SHA `61cbc6b842fdaaafc3f2fcab269bb11292fa8514` PASS: input detector, Build XY-C1, TRX verification and artifact upload all completed successfully. This establishes L3 for this exact source SHA; no physical-router L4 claim.
- Current parent: `479516c632f00bd737deec633c40b582202e7370`.

---

## 2026-11-19 — Assert PID 1 storage notification queue integration

- Storage hotplug rerun `44645368071` on `5ebdf79` passed the policy binary tests and operation-error assertions, then failed one remaining assertion requiring `notify_rc("on_hotplug_mass_storage")` in each mdev function.
- The current caller deliberately uses `queue_storage_hotplug_notification(device_name, "on_hotplug_mass_storage")` and passes its returned status into `storage_hotplug_completion_status()`; this preserves failure propagation if PID 1 cannot accept the event. Updated the workflow source contract to match that real queue API rather than weakening the assertion.
- NVRAM audit `44645672301` on `133ccb9` is IN_PROGRESS after updating the three-state PID 1 prerequisite/readiness contract; AI docs `44645672267` PASS. XY-C1 `44644448761` on product-source SHA `61cbc6b` remained in the Build XY-C1 step at last poll.
- No production behavior changed in this workflow-only correction. Parent SHA: `133ccb9ed03ebd2812ba76b3cb5e0eb175582d56`.

---

## 2026-11-19 — Align NVRAM fail-closed audit with three-state PID 1 readiness

- Exact-head NVRAM audit `44644448560` on `61cbc6b` passed the NVRAM lock, snapshot, ioctl and device-node policy tests, then failed its source assertion because it still required `router_ready = dev_init_status == 0 && nvram_node_result == 0 && init_router() == 0`.
- The production three-state design deliberately calls `init_router()` only when both mandatory prerequisites pass, and separately accepts READY/DEGRADED from `init_router_status_is_ready()`; the mandatory `nvram_node_result == 0` condition still gates readiness. The checker now asserts the explicit call guard, FATAL initial state, and final READY/DEGRADED classifier.
- This is a checker-only correction; no NVRAM or PID 1 production code changed. Exact-head NVRAM rerun is pending.
- Parent SHA: `5ebdf7977d3abf53ca5f3514b76f8a59b64af294`.

---

## 2026-11-19 — Repair three stale startup/storage CI contracts

- Exact-head Process run `44644448788` on `61cbc6b` passed the PID 1 policy binary, `dev_init` shell regression, and new `copyscripts_ca_status.sh` regression; its source checker then failed because it compared space-normalized C to an unnormalized function-call expression. Updated the assertion to remove whitespace from both sides.
- Exact-head AliDDNS run `44644448746` passed the HTTPS, IPv4/IPv6, lifecycle, CA archive and HMAC tests, then failed an obsolete contract requiring `if (system("copyscripts.sh") != 0)`. PID 1 now captures and aggregates `copyscripts_status`; the checker now asserts this exact boot-result path.
- Exact-head Storage run `44644448682` passed the GNU89 sanitizer policy binaries, including notification publication. The embedded source contract expected the former inline return, while SD/MMC now use the separately tested `storage_hotplug_completion_status()`; updated the contract to retain assertions for operation errors and notification failures.
- No production behavior changed in this checker follow-up. XY-C1 `44644448761` and NVRAM `44644448560` were IN_PROGRESS at the last poll; exact-head reruns are required for the three checker jobs and source SHA.
- Current parent/source SHA: `61cbc6b842fdaaafc3f2fcab269bb11292fa8514`.

---

## 2026-11-19 — Include late boot preparation and autostart failures in PID 1 status

- Found a caller-side status loss after the earlier FATAL/READY/DEGRADED change: `init_router()` logged but discarded `copyscripts.sh`'s return status even though the script explicitly returns failure when CA trust-store installation fails. The wait statuses for `/etc/storage/started_script.sh &` and `/usr/bin/autostart.sh &` were also log-only, and the boot aggregate summary ran before those calls.
- The source follow-up adds all three statuses to `boot_services_status` using the first-error aggregator and moves the final aggregate log after the launch attempts. The two background command results represent shell launch/wait status, not successful child readiness; this distinction is preserved.
- Added `copyscripts_ca_status.sh`: valid CA archive succeeds; invalid archive, missing CA bundle and uncreatable certificate directory fail; static checks ensure `copyscripts.sh` exits with its CA status. Local `sh -n` and shell regression passed (expected failure diagnostics are printed for negative cases).
- Added the new test/script paths to Process push/PR triggers and the product script to XY-C1 inputs. Exact-head Process/HTTPD/AI docs and XY-C1 for the new source SHA remain pending.
- Parent SHA `e8fb566f3a122f75c4f648543c2a7aa516b00316` has exact Process `44643316833`, HTTPD `44643316983`, AI docs `44643316847` PASS. XY-C1 `44643316929` was still in progress when recorded.

---

## 2026-11-19 — Align PID 1 degraded-startup CI contracts

- Exact-head Process run `44642485690` on `9ad1570`: the new GNU89/Werror/ASan/UBSan result-policy binary passed and the existing `dev_init.sh` shell regression passed; the Python integration checker then failed because its generic C parser still used the `init` buffer for brace scanning and return slicing when asked to parse `rc.c`. Corrected every parser read to use the supplied source buffer.
- Exact-head HTTPD run `44642485613` exposed two stale assertions from the former binary startup contract: it required `return -1;` in `init_router()` and `router_ready = ... init_router() == 0` in PID 1. Updated the contract to require four FATAL exits, the boot-service status mapping, the READY classifier and the degraded event-handling branch.
- Added `init_router_result_policy.h` to HTTPD push/PR path filters and XY-C1 push inputs so header-only changes trigger the relevant source and firmware checks. No product-source behavior changed in this CI follow-up.
- AI docs run `44642485702` PASS on `9ad1570`. XY-C1 run `44642485538` for `9ad1570` was still at Build XY-C1 when last checked; exact-head re-query is required for L3.

---

## 2026-11-19 — Preserve PID 1 readiness while propagating degraded boot status

- `init_router()` aggregated module/service failures in `boot_services_status` but returned success unconditionally, hiding partial startup failure from PID 1. Added distinct FATAL/READY/DEGRADED outcomes: the four mandatory NVRAM/configuration aborts remain fatal; accumulated boot-service errors now return DEGRADED.
- `init_main_loop()` keeps router events and deferred hotplug handling enabled for both READY and DEGRADED, and logs the degraded state. `dev_init.sh` and NVRAM node prerequisites still gate `init_router()`; fatal results retain console recovery behavior.
- Added a GNU89/Werror policy regression for success, positive/negative error statuses, degraded readiness, fatal rejection and unknown-state rejection. Local command `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I/tmp /tmp/init_router_result_policy.c -o /tmp/init-router-result-policy && ASAN_OPTIONS=detect_leaks=1 /tmp/init-router-result-policy` PASS. Process workflow also compiles/runs the regression and checks the production caller graph.
- Parent exact-head VPN `44640556013` and AI docs `44640556098` PASS on `b35b6a7aaa2b5018c8c5dac85b2aa81bb6074c35`. Earlier rp-L2TP product SHA `2b646cccdcb24da5d447586893a500fdcf3cfe48` XY-C1 run `44639411601` passed Build, TRX verification and artifact upload (artifact `14584429568`). The current RC/PID 1 change still requires its own exact-head CI and firmware run.
- This slice does not claim physical-router L4 verification.

---

## 2026-11-19 — Scope VPN path-filter contract to pull_request.paths

- Exact-head VPN run `44640170448` on `40eb3ad8` compiled and passed the VPN lifecycle lock, callback identity, xl2tpd start gate, rp-L2TP ASan/UBSan filesystem regression, shared lease lock and restart-status tests. It failed only in `check_vpn_server_lifecycle.py`: the checker counted the rp-L2TP regression source once in the PR trigger and again in the later compiler command.
- The PR path list itself contains the new header and test exactly once. Updated the checker to isolate the YAML `pull_request.paths` section before asserting the count, preserving strict duplicate detection in both push and PR path filters.
- HTTPD exact-head run `44640170448` PASS after the previous checker correction. The original rp-L2TP product source SHA `2b646cc` had a pending XY-C1 firmware run `44639411601` at last verified checkpoint.
- This is a checker-only fix; no product-source behavior or regression semantics changed. No local MIPS build or physical-router test is claimed.

---

## 2026-11-19 — Correct follow-up rp-L2TP CI contracts

- The first source-contract repair was followed by exact-head failures on 09dfc4b: VPN expected mode == 0644 while the regression correctly rejects any mode != 0644; HTTPD's callback extractor searched for a literal backslash-n because the workflow heredoc line was over-escaped.
- Corrected the VPN test-contract token and the HTTPD Python newline escape. Production C, filesystem regression and the security behavior are unchanged by this checker-only follow-up.
- After commit, recheck exact-head VPN and HTTPD jobs. The RP-L2TP test binary already passed with GNU89, -Werror, ASan and UBSan on 2b646cc.

---
## 2026-11-19 — Repair rp-L2TP security source contracts

- Exact-head VPN lifecycle run 44639411367 compiled and passed the new GNU89 ASan/UBSan rp-L2TP configuration-file regression, then failed because its source checker expected the directory inode comparison with reversed operands. Corrected the checker to match the production path-to-fd comparison.
- Exact-head HTTPD security run 44639411314 failed only because its PPP credential source contract expected fprintf() to remain inline in start_rpl2tp(). Updated it to verify l2tp_config_file_publish() integration and the separate rpl2tp_config_write() callback, while requiring removal of the old 0777 mkdir/fopen path.
- These are CI contract corrections, not new production changes. The exact-head reruns after this commit must pass before the slice's L2 gate is closed.

---
## 2026-11-19 — Secure rp-L2TP configuration publication

- Confirmed a privileged file-write boundary defect in APP_RPL2TP: PID 1 explicitly sets umask(0000); start_rpl2tp() used unchecked mkdir_if_none("/etc/l2tp", "777") and fopen("/etc/l2tp/l2tp.conf", "w"). The directory helper follows an existing symlink directory and does not repair permissive existing modes; the leaf open follows symlinks. A world-writable /etc/l2tp therefore let an unprivileged process redirect a root write through l2tp.conf.
- Replaced this path with an effective-UID-owned, no-follow directory check and permission hardening. Config writes now use a unique same-directory temporary file, checked stream flush/fsync/close, mode 0644 and atomic rename. Existing destination symlinks are replaced rather than followed; staging failures preserve the last-good config.
- Added GNU89 filesystem coverage for permissive umask, 0777 directory repair, symlinked directory refusal, symlinked leaf replacement without touching its target, failed-writer rollback and temporary cleanup. Added the new helper/test to VPN exact-head Actions and XY-C1 production-input triggers.
- Exact-head VPN, AI-docs and XY-C1 results must be checked against the new main SHA after push. No target build or physical-router result is claimed.

---
## 2026-11-19 — Correct firewall netmask contract and restore handoff checkpoint heading

- Firewall Startup run `44638506108` on `098a9d4` failed at its embedded source assertion because it still expected `ipv4_netmask_is_contiguous()` to be defined in `firewall_filter_policy.h`. The helper was correctly extracted into `ipv4_netmask_policy.h`; the firewall header includes it and the existing GNU89 regression calls it. Updated the checker to verify that actual include/definition relationship.
- AI handoff docs run `44638506375` failed because the required `Before context exhaustion` heading was absent. Restored the full checkpoint section while preserving the exact conditional-checkpoint sentence.
- On `098a9d4`, MiniUPnP config `44638505959`, Process lifecycle `44638506449`, WAN DHCP `44638506004`, WAN route `44638505955`, Samba `44638505951`, DNSMasq `44638505950` and Inadyn `44638506051` passed. Process umbrella, HTTPD, FULLCONENAT, NVRAM and XY-C1 were still pending at checkpoint.
- No production source or physical-router behavior changed in this correction.

---

## 2026-11-19 — Share strict LAN netmask validation with miniupnpd

- Review found miniupnpd startup in `services_ex.c` accepted any syntactically valid dotted-IPv4 mask, then passed it to `ip2class()`. Non-contiguous masks such as `255.0.255.0` could still produce the wrong LAN prefix although firewall startup was already hardened.
- Extracted the nonzero contiguous IPv4 mask predicate into shared GNU89 header `ipv4_netmask_policy.h`. Firewall and miniupnpd now call the same helper; miniupnpd rejects an invalid mask before `ip2class()` and returns `EINVAL`.
- Existing Firewall GNU89 regression now compiles the shared helper and covers valid prefixes, zero, malformed values, non-contiguous masks and injected strings. Firewall and miniupnpd workflow contracts pin source ordering and push/pull_request path coverage.
- Parent `5e2c7a3fd8479bd1cdbdf455b34c18d20494e1b7`: MTD Process lifecycle regression `44637423543` and Process inventory `44637423511` PASS. AI handoff docs `44637423581` failed because the handoff lacked its mandated exact conditional-checkpoint sentence; this handoff update restores it. NVRAM `44637423514` and XY-C1 `44637423396` were IN_PROGRESS at last poll.
- No target MIPS build or physical-router result is claimed for this integration.

---

## 2026-11-19 — Preserve the MTD bounded-EINTR limit through the full read/write loops

- Source review after the checker-only correction found a production defect in the first MTD migration: `read_safe()` and `write_safe()` caught the terminal `EINTR` from the shared eight-retry wrapper and restarted it, resetting its internal counter. A persistent signal storm could therefore still loop forever.
- Removed the outer EINTR restart. The bounded wrapper's terminal error now propagates; positive short I/O continues to advance buffers, read EOF remains a normal short read, and zero-progress write still fails with `EIO`.
- Strengthened the Process source contract to forbid outer EINTR/continue loops and direct syscalls while asserting short-I/O progress, EOF and zero-write handling. The existing GNU89 `process_pidfile.c` regression covers the shared wrapper's eight retry successes and terminal ninth EINTR for reads/writes.
- Parent `dfe8eb6d95bd5297612c41e06412e17c1a9434fe`: Process inventory and AI docs had triggered and were IN_PROGRESS at last poll. No target MIPS build or hardware result is claimed.

---

## 2026-11-19 — Narrow Process raw-syscall negative assertions

- Exact d7 Process umbrella `44636788168` failed at the source-contract step because substring `read(fd, ...)` also appears inside `process_io_read(fd, ...)`; same issue for write. All earlier procfs/PID-file checks ran before that assertion.
- Narrowed negative checks to the exact assignment prefix `ssize_t ret = read(...)` / `write(...)`, while retaining positive assertions requiring the bounded process_io wrappers. No production source changed.
- On d7, Process inventory `44636788355`, RStats `44636788530`, AI docs `44636788278`, and HTTPD security `44636788170` PASS. NVRAM `44636788042` and XY-C1 `44636788140` were still in progress at last poll.
- This checker repair awaits exact-head Process/AI/NVRAM/XY-C1 results. No local compile or router L4 is claimed.

---

## 2026-11-19 — Bound EINTR retries in RStats persistence

- Source review found unbounded EINTR retry loops in RStats exact reads, atomic writes, fsync and history-file open. A signal storm could indefinitely hold the stats daemon inside persistence I/O.
- Added an eight-consecutive-interruption policy and injectable bounded read-only open. Successful read/write progress resets the interruption counter; EOF and prior non-EINTR errors keep their meanings. Failed atomic writes still remove only their temporary file and retain the last-good target.
- Added GNU89 callback regressions for open/read/write/fsync success at the retry limit, persistent EINTR exhaustion, counter reset after progress, EOF and EIO propagation. Updated the RStats source checker to require bounded open/read integration.
- Parent `5c557db717ec173ef151fb6e473e44a676924234` follows the MTD flash-I/O retry fix; its Process umbrella, AI docs, NVRAM and XY-C1 runs were pending at checkpoint. Earlier parent `342631b` Process, firewall, FULLCONENAT, WAN route, NVRAM and AI docs checks passed; HTTPD and WAN DHCP failed only on source checker assertions, which the subsequent `8987975` commit corrects.
- This RStats slice awaits exact-head Actions; no local MIPS build or physical-router result is claimed.

---

## 2026-11-19 — Fix WAN restoration checker variable

- WAN DHCP callback run `44635983525` on source SHA `8987975` reached the new fail-closed restoration assertions but failed with `NameError: wan is not defined`; the workflow's source buffer is named `source`.
- Corrected the checker to parse `restore_wan_after_firewall_failure()` from `source`. The assertions still require delegation to the original physical-WAN restore and interface shutdown if firewall recovery fails.
- HTTPD security `44635983452` and AI docs `44635983515` already PASS on `8987975`. Exact rerun of WAN callback remains pending.
- Concurrent MTD flash fix `e5bd15b` has exact-head Process/AI/NVRAM/XY-C1 runs pending at last poll.

---

## 2026-11-19 — Bound EINTR retries in MTD flash I/O

- `flash_mtd.c` had unbounded retry loops around raw `read(2)` and `write(2)`. A persistent signal storm during flash partition I/O could spin indefinitely.
- Reused shared `process_io_read()/process_io_write()` wrappers with an eight-consecutive-EINTR cap. Positive short-I/O progress, short reads at EOF, and zero-progress write EIO behavior are unchanged.
- Process lifecycle source-contract pins both MTD callsites and tracks `flash_mtd.c` in push/PR; XY-C1 tracks the production file so its change triggers firmware validation.
- Parent `342631b` firewall-startup policy PASS; HTTPD/WAN workflow assertions were stale and repaired in `8987975`. Parent NVRAM/XY-C1 were pending at last poll. This MTD source SHA still needs exact-head L2/L3 validation.

---

## 2026-11-19 — Align HTTPD and WAN workflow contracts with firewall safety fixes

- Exact parent SHA `342631b4efb09b4877dd944d74f03d20da7b4164` passed firewall-startup-policy run `44635384398`.
- HTTPD security `44635384245` failed on a stale expected callsite count: firewall NAT input now has a second LAN IPv4 address guard, raising `firewall_ipv4_address_is_safe()` from 9 to 10. Updated the source contract to assert 10.
- WAN DHCP callback `44635384375` expected the old direct restore helper, while source now calls stronger `restore_wan_after_firewall_failure()`. Updated the check and explicitly pin its fallback behavior: delegate original restore, then bring physical WAN down if restoring firewall also fails.
- NVRAM `44635384340` and XY-C1 `44635384278` were in progress at last poll; this contract-only fix does not change firmware source.
- No local build or router L4 result is claimed.

---

## 2026-11-19 — Reject malformed LAN netmasks before firewall subnet derivation

- `ip2class()` counts set bits instead of validating an IPv4 netmask. A non-contiguous mask such as `255.0.255.0` was silently converted to an incorrect prefix, while malformed text could be interpreted as `/32`; NAT-loopback and later firewall rules could therefore target the wrong LAN subnet.
- Added strict dotted-IPv4 and contiguous-prefix validation. `start_firewall_ex_locked()` now refuses to rebuild before deriving `lan_net`; `ipt_nat_default()` also refuses to replace the baseline NAT table when an active LAN address has an invalid mask.
- Extended the existing GNU89 firewall filter-policy regression and firewall workflow source-order contract. The isolated policy logic passed local `-std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` tests for valid prefixes, zero, non-contiguous, malformed and injected values. Exact-head Actions and XY-C1 results for the integrated change are pending.
- Exact parent SHA: `39c3db7ba0c4c1ec396a00b619e2c4c69993cc56`. No target MIPS build or physical-router behavior is claimed.

---

## 2026-11-19 — Repair the service-wait EINTR regression fixture

- Exact job logs for `437c7b2` showed that NFS run `44633888395` and Samba run `44633888489` both stopped compiling `tests/service_wait_policy.c`: the seconds-sleep storm test referenced an undeclared `fixture_sleep` and incorrectly reused the nanosleep fixture type.
- Added a dedicated injectable seconds-sleep fixture/callback. The test now verifies that the same one-second remainder is requested nine times and that the ninth interrupted call returns `EINTR`; nanosleep test cases remain independent.
- The following source SHA `ba47b5f` passed Process umbrella `44634242619` (including PID-file read/write retry tests) and AI docs `44634242529`; NVRAM `44634242550` and XY-C1 `44634242726` were pending at checkpoint.
- This is a focused regression-only correction; no production source or physical-router result is claimed.

---

## 2026-11-19 — Bound shared PID-file and legacy procps I/O retries

- Extended the existing eight-consecutive-EINTR policy with injectable bounded read/write wrappers. PID-file reads/writes, the inline PID start-time reader and legacy procps `read_to_buf()` now return preserved `EINTR` rather than retry indefinitely under a signal storm.
- Added GNU89 callback regressions for read/write recovery, retry exhaustion, EOF, EIO propagation and invalid callbacks. Existing Process regression workflow compiles `tests/process_pidfile.c`; current Process/XY-C1 path filters already cover the touched sources and test.
- On the preceding source chain, Process umbrella `44632969368` and AI docs `44632968248` passed exact SHA `16b7556`; the latest service-wait hardening commit `437c7b2` was still under Actions validation at handoff.
- No local firmware build or physical-router result is claimed; this I/O change requires its own exact-head Actions validation.

---

## 2026-11-19 — Bound repeated EINTR in shared service waits

- Shared `service_wait_seconds_with()` and `service_wait_timespec_with()` could retry interruption indefinitely. Under repeated immediate signals, integer-second `sleep()` can return the same remainder and keep PID 1/lifecycle callers stuck.
- Added a common eight-consecutive-interruption limit to both wait wrappers. Exact remaining-time retries continue normally; the ninth interruption returns `-1/EINTR` rather than an unbounded loop.
- Extended the GNU89 wait-policy regression for repeated `sleep()` and `nanosleep()` interruptions while retaining remainder, malformed-time and errno tests. Existing Process workflow runs this test; XY-C1 watches the shared header.
- The procfs read fix SHA `53dda2c` passed Process inventory, detect-link, GPIO, DNSMasq, VPN, crond and NVRAM; AI docs/Process umbrella initially failed only on workflow filter parity and passed on `16b7556` follow-up. XY-C1 `44632416166` remained in progress at last poll.

---

## 2026-11-19 — Restore Process workflow push/PR path parity

- Exact source SHA `53dda2c844e18cfb879abc3bf4b96bb50d46fd2d` passed Process inventory `44632416173`, detect_link `44632416162`, GPIO `44632416043`, DNSMasq `44632416164`, VPN `44632416149` and crond `44632417092`.
- AI-doc check `44632416060` failed because the new shared process-I/O helper was watched by the Process workflow push filter but not its pull_request filter. Restored trigger parity in this workflow-only follow-up; no production source behavior changed.
- Process umbrella `44632416363`, NVRAM `44632416256` and XY-C1 `44632416166` were in progress at last query. XY-C1 L3 remains unclaimed pending Build/TRX/artifact exact-head success.

---

## 2026-11-19 — Bound EINTR retries in shared procfs reads

- `proc_default_read_file()` had unbounded `read(2)` EINTR loops in both the normal file read and the one-byte overflow probe. Repeated signal interruptions could keep process-inventory scans stuck indefinitely.
- Added `process_io_retry_policy.h` with an eight-consecutive-interruption cap. Successful/EOF reads reset the normal counter; overflow probing has a separate finite budget. Exhaustion preserves EINTR and fails closed.
- Added a GNU89 regression for retry success, exhausted EINTR and invalid arguments. Process lifecycle workflow now asserts that both production read loops consume the helper and tracks the new header; XY-C1 also tracks the shared header.
- The parent lock-wait commit `d57cce1` passed AI docs, HTTPD, Process inventory, Inadyn, DNSMasq, Process regression, VPN, MiniUPnP config and NVRAM. Its XY-C1 run `44631856549` was still in progress at last poll. This new source SHA's exact-head checks remain pending; no local compile or router L4 is claimed.

---

## 2026-11-19 — Preserve lifecycle lock contention waits across EINTR

- Inadyn lock acquire skipped its one-second contention wait on EINTR; DNSMasq and VPN server used sleep(1) without retrying the remaining interval. Repeated signals could prematurely exhaust their bounded attempt count.
- Replaced these waits with the existing service_wait_seconds(1) policy, which preserves unslept time across EINTR. Lock identity validation and shared eight-retry unlock cap remain unchanged.
- Extended the MiniUPnP semantic checker to require signal-safe waits in all three production acquire paths. Added the DNSMasq lock header to the XY-C1 production-input filter.
- Exact 213 L2 Actions are PASS: MiniUPnP config, DNSMasq, Inadyn, VPN server, Process lifecycle, Process inventory, HTTPD security, AI docs, and NVRAM audit. Its XY-C1 `44631039226` was still IN_PROGRESS at last poll. This source follow-up's exact-head Actions remain pending; no local compile or router L4 is claimed.

---

## 2026-11-19 — Share bounded EINTR retries across service lifecycle locks

- The earlier MiniUPnP fix exposed the same unbounded unlock loop in Inadyn, DNSMasq and VPN-server lifecycle lock release helpers. Repeated signals could keep the release paths from reaching descriptor close.
- Added `lifecycle_lock_retry_policy.h` with a common eight-retry ceiling. MiniUPnP acquisition/release and Inadyn, DNSMasq and VPN-server release now use the shared helper; existing EACCES/EAGAIN acquisition backoff remains unchanged.
- Reused the GNU89 MiniUPnP lock regression for retry-bound/exhaustion behavior and extended the semantic checker to pin all four release paths. XY-C1 path filtering includes the shared production helper. Per-service exact-head Actions and same-source XY-C1 remain pending.
- Prior MiniUPnP source SHA `e736dbf`: Process lifecycle, MiniUPnP config, Process inventory, NVRAM audit and XY-C1 Build/TRX/artifact all PASS. AI handoff docs failed on e736 only due the missing required phrase and passed on follow-up `a9f3c20` (`44629051861`). No physical-router result is claimed.

---

## 2026-11-19 — Restore required cross-session handoff rule

- Exact-head Process lifecycle `44628661363`, MiniUPnP atomic config `44628661018`, and Process inventory `44628661358` PASS on source commit `e736dbfcff830e6ac6baa32026fff7a6334787d1`; these include the new MiniUPnP EINTR-bound source and regression contract.
- AI handoff docs `44628661127` failed because `SESSION_HANDOFF.md` no longer contained the required literal phrase `Before context exhaustion`. Restored the cross-session execution rule; this is a documentation-contract repair only.
- NVRAM audit `44628661528` and XY-C1 firmware run `44628661156` were still in progress at checkpoint. XY-C1 Build/TRX/artifact must all pass on `e736dbfcff830e6ac6baa32026fff7a6334787d1`; no L3 claim yet. No local compile or physical-router result is claimed.

---

## 2026-11-19 — Bound MiniUPnP lifecycle-lock EINTR retries

- Source review found lock acquisition and release both retry `fcntl(F_SETLK)` indefinitely on `EINTR`; the configured wait only limits EACCES/EAGAIN contention.
- Added an eight-retry cap. Acquisition resets the retry count after non-`EINTR` results so ordinary contention still uses its existing timeout; release closes the lock descriptor after bounded unlock attempts.
- Extended GNU89 lock regression coverage for retry success, exhausted EINTR and invalid arguments; updated the semantic checker to pin bounded handling in both paths. Added the production header to the XY-C1 push path filter so a header-only production change triggers the build.
- On b4, Wireless `44627098078` PASS; Process `44627098093`, Storage `44627098102`, HTTPD `44627098106` and AI docs `44627098108` failed stale contracts repaired in `9fd21db`; reruns on 9fd remain unverified. Product SHA `3488f12` passed XY-C1 Build/TRX/artifact (`44625809599`). No local compile or physical-router result is claimed.

---

## 2026-11-19 — Align storage, HTTPD, wireless and handoff CI contracts

- Exact b4b46 checker runs: Wireless radio `44627098078` PASS; Process lifecycle `44627098093` failed because the contract searched the first radio-status merge, which is the earlier APCLI WAN result, rather than the later IAPP wait-status merge. The source correctly merges IAPP failure after its own status check; the checker now searches from that failure branch.
- Storage `44627098102` and HTTPD `44627098106` failed because their embedded source contracts still searched for legacy `notify_rc()` calls after the SD/MMC source switched to checked `queue_storage_hotplug_notification()`. Updated both ordering/target assertions.
- AI handoff docs `44627098108` failed because the handoff's mandatory checkpoint statement was shortened during a prior edit; restored the exact governance statement required by the protocol checker.
- Product SHA `3488f129babe376bccf27e132776c45910614dd7` passed XY-C1 Build, TRX verification and artifact upload (`44625809599`). This follow-up contains checker/docs only; exact-head reruns pending.

---

## 2026-11-19 — Repair APCLI path variable and storage completion contracts

- Parent main SHA: `3488f129babe376bccf27e132776c45910614dd7`. Wireless `44625501907` and Process `44625501771` on `f2e8147` passed the compiled wireless/bridge policy regressions and APCLI source checks, then failed only because the checker read `rc / "tests/wireless_radio_policy.c"` while the script defines `RC`. Corrected the typo.
- Storage hotplug `44625809558` and HTTPD security `44625809482` on `3488f12` exposed two downstream checker drifts from the SD/MMC notification result change: storage asserted the absent `tune_status == 0` even though failed tuning is logged while mounting continues and final mdev status reports the error; HTTPD expected the old boolean completion expression instead of `storage_hotplug_completion_status(operation_status, notification_status)`. Updated both checks to pin the current intended behavior and the two-input failure policy.
- NVRAM audit `44625809632`, Process inventory `44625809765`, and AI docs `44625809759` PASS on `3488f12`. XY-C1 firmware `44625809599` was still in progress on exact product SHA `3488f12`; Build/TRX/artifact must all pass before L3. This commit changes checkers/docs only; no product source or local firmware build is claimed.

---
## 2026-11-19 — Propagate SD/MMC mass-storage notification queue failures

- `mdev_sd_main()` and `mdev_mmc_main()` used legacy void `notify_rc()` for whole-device unplug and successful automount. The shared notifier can fail to create secure PID 1 queue markers, but mdev discarded that failure and could return success while storage cleanup/startup was never scheduled.
- Replaced both SD/MMC add/remove calls with checked `notify_rc_status()`, log and preserve queue errno, and include notification status in applet completion. Existing ordering remains: process swapoff before unplug notification and queue mount notification only after successful automount.
- Added a GNU89/Werror/ASan/UBSan completion-policy regression and semantic checks for both applet paths, workflow invocation, and XY-C1 tracking. Local policy binary PASS. Exact-head storage Actions and XY-C1 Build/TRX/artifact pending; no physical-router L4 result claimed.

---

## 2026-11-19 — Tighten APCLI call-count contract without rejecting the captured result

- Parent main SHA: `ccf419d1dcff6aff70080d37c90be707dec9fe88`. Exact WAN route run `44625039862` PASS validates the ordinary PPP demand path now uses `restore_wan_after_firewall_failure()`; AI docs `44625039727` PASS. Wireless `44625039991` and Process `44625039798` passed the APCLI helper and two-band accumulator assertions, then failed because the checker's `... not in restart` test also matched the intended `int wan_status = check_apcli_wan(...);` declaration.
- Replaced that substring ban with an exact single-invocation assertion. The preceding expected contract still requires the result to be captured into `wan_status` and merged into the existing sticky error accumulator.
- This is test-only; no product source changed. Exact Wireless/Process/docs reruns pending. XY-C1 `44624238619` passed host contracts but skipped Build, TRX and artifact upload; L3 remains NOT VERIFIED.

---
## 2026-11-19 — Align APCLI and PPP caller contracts with production control flow

- Parent main SHA: `b50ce7d86e43e60f0885f1d849a4f0dd690cffc8`. The HTTPD PPP credential/security regression `44624238578` and AI docs `44624238537` PASS. WAN route `44624238559` then exposed one remaining inline demand-PPP assertion expecting the direct lower-level recovery call; it now requires the fail-closed wrapper.
- Wireless `44624238741` and Process lifecycle `44624238561` failed after the newline parsing fix because the APCLI checker expected `check_apcli_wan()` embedded in the accumulator call. Production intentionally captures `wan_status` first, then merges it with `wireless_radio_restart_status()` to preserve the first failure. The checker now verifies that real two-step flow for both bands.
- XY-C1 host-side contracts passed on `44624238619`, including the WAN firewall contract, but the workflow reported host-only validation and skipped Build XY-C1, TRX verification and artifact upload because this commit changed only checker files. L3 remains NOT VERIFIED. Current exact-head checker reruns are pending; no physical router test is claimed.

---
## 2026-11-19 — Propagate AP-client WAN recovery failures through radio restart

- Parent main SHA: `5878c9d11404d1bd4a329b59430a7c1279025dd4`. Static review found `check_apcli_wan()` discarded the return from `full_restart_wan()` when MAN changed and `try_wan_reconnect(1, 0)` after a WISP APCLI radio cycle. Its `void` signature hid both failures from `restart_wifi_wl()` and `restart_wifi_rt()`.
- This change returns/logs the WAN recovery status and merges it into each radio restart's existing result without replacing an earlier guest-isolation failure. Added GNU89 policy cases for both WAN recovery failures and first-error retention, plus the source contract for both band-specific callsites.
- Parent exact-head Process umbrella `44622196753`, Process inventory `44622196808` and AI docs `44622196858` PASS; NVRAM audit `44622196798` was pending at preparation time. Query new exact-head Wireless, Process, docs, NVRAM and XY-C1 checks after push. No local build or physical-router test is claimed.

---

## 2026-11-19 — Fail closed on incomplete PPP physical-WAN firewall recovery

- Parent main SHA: `52cd91c1c7f0d5c297b157a9ffb636844363f605`. Three normal PPP startup failures in `start_wan()` bypassed `restore_wan_after_firewall_failure()` and called the lower-level recovery helper directly. If miniupnpd did not stop or rebuilding the physical-WAN firewall failed, the paths could leave the physical interface administratively up.
- Routed PPTP/L2TP physical setup failure, pppd launch failure and demand-PPP readiness failure through the existing fail-closed wrapper. It forces the physical interface down when UPnP/firewall recovery fails and preserves the first recovery error. Successful recovery semantics are unchanged.
- Extended `scripts/check_wan_firewall_status.py` to pin all three caller paths and forbid bypassing the wrapper. The existing WAN route workflow includes both changed paths. Local brace-aware semantic assertions for all three caller blocks PASS; exact-head WAN route and XY-C1 Build/TRX/artifact checks are pending. Physical-router L4 is NOT VERIFIED.
- Prerequisite results: SCUT Process umbrella `44622196808` PASS, Process inventory `44622196753` PASS and AI docs `44622196858` PASS on `52cd91c...`; Crond `44621067912` PASS on `69d64f4...`.

---

## 2026-11-19 — Prevent EPERM errno from colliding with crontab CHANGED status

- Re-reviewed the new crontab status decoder and found a subtle sentinel collision: Linux `EPERM` is errno 1, the same value that `check_crontab.sh` intentionally uses for a successful append. If `system()` returned -1 with `errno=EPERM`, the positive errno mapping would have falsely treated the failed invocation as CHANGED and entered MTD persistence/crond restart.
- Changed `system()` invocation errors to negative errno values; exit statuses 0/1/2+ remain unchanged, abnormal waits map to EIO, and the first-error merge/persistence gate treats all negative values as failures. Added explicit `EPERM`, negative merge and persistence-gate C regression assertions, and updated the Python source contract.
- Exact Process umbrella `44621537460` and AI docs `44621537441` PASS on parent `c2fe4764d6864ee3ebab43eeb21ce74f91a2578c`. The Crond stale-checker repair `44621067912` and AI docs `44621067804` PASS on `69d64f4bd9b70a8f32083ac84e8ebca0c8a97d9f`. The XY-C1 Build/TRX/artifact run on SCUT source SHA `69430c...` was still running at the last poll; L3 not yet asserted.
- This atomic commit changes only the C status helper, its tests, the semantic checker and handoff/roadmap/log records. Exact-head Process, Crond, AI docs and XY-C1 verification remain pending.

---

## 2026-11-19 — Update Process umbrella contract for SCUT crontab tri-state

- Process lifecycle umbrella `44620322615` on `69430c238e26c0de0b9210a5d090512bb4570f50` passed the scanner, identity, WAN, startup, notification, SCUT policy, shell and service regressions through step 54, then `scripts/check_process_lifecycle_integration.py` raised IndexError because it still split `init_router()` at the removed `if (init_crontab())` callsite.
- Updated that aggregate source contract to inspect the new status 0/1/error branches, require persistence only on a confirmed append, ensure MTD failure is aggregated before independent crond restart, require crond error aggregation, and ensure crontab failures reach boot status. No production source changed in this checker-only slice.
- The separate crond lifecycle assertion correction is already PASS at exact SHA `69d64f4bd9b70a8f32083ac84e8ebca0c8a97d9f` (`44621067912`); AI handoff contract `44621067804` also PASS. Process umbrella rerun after this commit remains pending. Root log remains within its 20–119 entry window.

---

## 2026-11-19 — Align crond lifecycle contract with SCUT tri-state boot update

- Exact-head Crond lifecycle run `44620322749` at source SHA `69430c238e26c0de0b9210a5d090512bb4570f50` passed its GNU89/Werror/ASan/UBSan C daemon policy test, then failed in `scripts/check_crond_lifecycle.py`: the checker still split `init_router()` at the removed `if (init_crontab())` callsite and raised IndexError.
- The production change deliberately replaced the boolean-like caller with explicit status 0=unchanged, 1=appended, 2+=error. Updated the crond source contract to assert persistence gating, MTD-error aggregation before independent crond activation, crond restart error aggregation and the separate crontab failure branch. Production crond behavior and SCUT implementation are unchanged by this checker-only correction.
- On `69430c...`, AI handoff docs `44620322445`, Process inventory `44620322214`, Firewall `44620322286`, vsftpd `44620322183`, PID-1 queue `44620322244`, ZeroTier `44620322876`, detect_internet `44620322573`, rstats `44620322742`, DNSMasq `44620322174`, detect_link `44620322410`, Wireless `44620322537`, NFS `44620322374` and iNIC `44620322401` PASS. XY-C1 `44620322487` and the Process umbrella were still pending at last query.
- Re-run Crond and inspect exact-head Process + XY-C1 on the correction SHA. No physical-router L4 result claimed.

---

## 2026-11-19 — Propagate SCUT crontab update failures into boot status

- `check_crontab.sh` uses exit 1 for a successful new entry and 0 for an existing entry, but append failure silently fell through to exit 0 and grep I/O errors were treated as “not found”. `init_crontab()` ORed the raw encoded `system()` wait status; `init_router()` consequently treated every nonzero error as “changed”. It also logged but discarded MTD persistence failure.
- Added the explicit 0=unchanged, 1=added, 2+=error contract, symlink/non-regular path rejection, checked grep/append/directory results and restrictive creation umask. RC now decodes wait status, preserves `system()` errno, persists only after a real append, and aggregates script/MTD failures while still attempting crond activation.
- Added GNU89/Werror/ASan/UBSan policy tests and isolated shell regressions for changed/no-op, bad arguments, missing storage, grep I/O, append target errors and symlink paths. Local C policy binary, shell syntax and shell regression all PASS. Exact-head Process lifecycle, AI docs and XY-C1 Build/TRX/artifact remain pending.
- Parent main: `930dd7ed850ef59275cc340bc7c5a36bc7ebedc9`; its AI handoff documentation check `44617795116` PASS. No local MIPS build or physical-router L4 result is claimed.

---

## 2026-11-20 05:09 CST (UTC+8) — Restore the AI handoff contract and catch up root log rotation

- Parent main SHA: `f4fbd093e934fda5304aa796cba30d59eb29b3a4`. Exact-head OpenVPN client credential staging run `44613650285` PASS; VPN server lifecycle run `44613173654` PASS; XY-C1 run `44613173721` completed Build, TRX verification and artifact upload on the OpenVPN source SHA `e01f70a766c4e628593e272192723f6add59ffca` (L3).
- AI handoff documentation run `44613650462` failed because SESSION_HANDOFF.md lacked the required literal `## Active task` section. Restored the active-task section without changing product source or weakening the validator.
- The root log contained 121 dated records. Archived the 101 oldest complete records verbatim, oldest-first, in `docs/dev-log/2026/11/part-018.md`; retained the newest 20 and prepended this milestone (21 root records). Updated the archive index with exact count, byte size and Git blob SHA.
- No product source changed. No local validator was run in the connector-only environment. Exact-head AI documentation validation for THIS COMMIT is pending; L4 physical-router/client-import behavior remains NOT VERIFIED.

---

## 2026-11-19 — Correct OpenVPN export contract formatting check

- The OpenVPN credential workflow at exact SHA e01f70a766c4e628593e272192723f6add59ffca completed its four GNU89/ASan/UBSan policy tests successfully, then failed the new source contract because the TLS-Crypt-v2 key call spans two source lines while the assertion expected a single line. The VPN server lifecycle workflow passed. This is a test-checker formatting mismatch; no source failure was indicated.
- Corrected the contract to check the second-line key argument in order after chmod and before cleanup. Kept all source security checks unchanged.
- The exact-head AI docs, XY-C1, HTTPD, process and NVRAM runs were still in progress at the last query.

---

## 2026-11-19 — Harden OpenVPN exported client credentials

- ovpn_server_expcli_main() previously wrote embedded client private-key material to predictable /tmp/client.ovpn via fopen("w+"), ignored fclose() and chmod(0600) results, and returned success even if certificate/key generation or PEM input reads failed. A pre-existing symlink could be followed and a failed permission change could leave private credentials exposed.
- Switched the output to the existing private staged/atomic OpenVPN config publisher (fsync, checked close, atomic rename, final 0600). Generated keys reside in a unique mkdtemp() 0700 directory. Key input now uses no-follow/nonblocking descriptors, regular-file ownership/link checks, restrictive modes for embedded secret sections, PEM BEGIN/END validation and read/close error checks. Certificate and TLS-Crypt-v2 key generation and temporary cleanup are checked. Removed the post-publish unix2dos/chmod rewrite; OpenVPN accepts LF.
- Expanded scripts/check_openvpn_client_secret.py with exact source contracts. Existing openvpn-client-secret CI runs this checker and the ASan/UBSan atomic config publication regression; VPN server lifecycle and XY-C1 workflows also trigger from vpn_openvpn.c.
- Parent main SHA 4148ac7315d4835b9818c555956d99b195bf4947 had all 13 Actions PASS, including XY-C1 44608598285 and AI docs 44608598296. New exact-head OpenVPN, VPN lifecycle, AI docs and XY-C1 runs remain pending until checked.
- No physical-router or VPN client import result is claimed.

---

## 2026-11-19 — Propagate create_file close failures and satisfy exact handoff contract

- Exact-head DNSMasq lifecycle regression `44606578028`, HTTPD security `44606578001`, WAN DHCP `44606577590`, Process lifecycle inventory `44606578189`, WAN route `44606578101`, Inadyn `44606577502`, Samba `44606577607`, miniupnpd `44606578008` and Process inventory `44606578137` passed on `b1835ef0936517df7a39ef06dc242168251dd4fe`. XY-C1 `44606578070` and NVRAM `44606578018` were still running.
- AI handoff check `44606578020` failed because SESSION_HANDOFF omitted the explicit conditional-document-edit checkpoint statement and the Verification boundary heading. Restore both exact governance requirements; no source failure was reported by that job.
- Continued the resolver-file reliability slice after reviewing shared `create_file()`: it returned success even when `close(fd)` failed. The new injectable policy returns failure with the real close errno (or EIO fallback), and GNU89 ASan/UBSan tests cover open success/failure, close failure, errno preservation and invalid inputs. `common_ex.c` now routes through the tested helper.
- Exact-head DNSMasq, AI docs, and XY-C1 for the next commit are pending; no physical-router L4 result is claimed.

---

## 2026-11-19 — Fail closed when DNSMasq resolver-file creation fails

- Confirmed a concrete RC service-preparation gap in `prepare_dns_dhcpd()`: router mode called `create_file(DNS_RESOLV_CONF)` but discarded its nonzero result, then continued to publish a DNSMasq configuration that references the file and launch the daemon. A permission/filesystem failure could therefore leave resolver forwarding unavailable while the launch path treated the service as started.
- This slice preserves the create errno (falling back to EIO), logs the failure, and aborts before DNSMasq config publication/launch. AP mode is unchanged. Added a GNU89/Werror/ASan/UBSan policy test for success, errno preservation and missing-errno failure, plus exact source-order checks and XY-C1 header path coverage.
- Parent main SHA: `dec2079fcbf9d45731dff1ce03b2765a23bd3a31`; preceding exact-head detect-internet `44603838266`, WAN route `44603838310` and AI docs `44603838305` PASS. New DNSMasq regression, AI docs and XY-C1 Build/TRX/artifact are pending for this source commit until its exact-head runs complete.
- No local firmware/toolchain result or physical-router L4 result is claimed.

---

## 2026-11-19 — Align monitor-pause teardown source contracts

- Exact-source detect-internet Action 44600896372 failed in its inline Python contract at line 134; WAN route Action 44600896744 failed at inline line 253. Both expected obsolete `(void)notify_pause_detect_internet()` calls and advisory-only behavior.
- Source commit a7eae5a115dae74685e9130ff6d339b8202e5725 deliberately changed MAN/WAN teardown to aggregate the monitor pause result into the first error, emit a diagnostic, continue route/interface cleanup, and return the accumulated status. MAN only clears lease state when all cleanup succeeds. The workflow contracts now assert this ordering and status preservation; production code is unchanged.
- Infosvr correction commit 2cdd33f556b925f130d5a05348bb6774fc88dea0 passed exact-head Infosvr packet/signal 44602137230 and AI handoff docs 44602137197. XY-C1, SkipDBv2 and NVRAM passed on product source 2e61e6b; Samba regression, detector, Build XY-C1, TRX and artifact all passed in 44600896324.
- Exact reruns for detect-internet and WAN route are pending. No physical-router L4 result claimed.

---
## 2026-11-19 — Repair the Infosvr checker after launcher-status normalization

- Exact-head Infosvr Action 44600896016 on production SHA 2e61e6b4608d30127692f5d26e939bbce92f52fc passed packet, signal and daemon policy binaries under GNU89/Werror/ASan/UBSan and passed the packet/signal source checker.
- The lifecycle checker failed only at line 132 because its marker tuple still named test_start_preserves_launcher_failure, intentionally replaced by test_start_normalizes_launcher_failure_for_pid1. Updated the expected marker; no product source changes.
- On the same SHA, Process umbrella 44600895951, Process inventory 44600896622, PID 1 notification queue 44600896480 and AI docs 44600896036 passed. XY-C1 44600896324, SkipDBv2 44600896667 and NVRAM audit 44600896559 were still running.
- Also tracked two separate checker failures: detect_internet 44600896372 ended in Python ValueError at line 134; WAN route 44600896744 ended in Python ValueError at inline line 253. Preserve the DHCP/monitor cleanup assertions when correcting them.
- No physical-router L4 result claimed.

---
## 2026-11-19 — Normalize infosvr launch wait status for PID 1 notifications

- PID 1's `handle_notifications()` retains an incomplete notification marker only when `action_result < 0`. `start_infosvr()` returned a positive `eval("/usr/sbin/infosvr", IFNAME_BR)` wait status unchanged, and `infosvr_start_postcondition()` preserved it; a failed detached child could therefore be acknowledged as success and its restart marker cleared.
- The launch error path now preserves the original status in its diagnostic log, maps positive encoded wait statuses to `errno=EIO`, and returns through `infosvr_start_postcondition()`. The helper maps every nonzero launch status to `-1`; disabled optional-service success `0` and the guard-before-inventory order remain unchanged.
- Regression tests cover positive wait statuses 7/256, negative launcher failure, and RUNNING/ABSENT/UNKNOWN postconditions. Dedicated Infosvr and broader Process source checkers now enforce the negative-only PID 1 failure convention. Existing workflows compile the GNU89/Werror/ASan/UBSan policy test.
- Parent `5c43773c08d50540c481bc8e0ca4b6204c63a7f5` inherited exact-head Infosvr `44598421247`, Process umbrella `44598421270`, Crond `44598421431`, Process inventory `44598421241`, and NVRAM `44598421188` PASS on `9e51fc0...`; XY-C1 `44598421480` was still IN_PROGRESS. Samba L2 `44597410202` PASS on `0e137...`; HTTPD security/AI docs `44598733635`/`44598733483` PASS after descriptor-check correction. New exact-head L2/L3 pending. No physical-router L4 result claimed.

---

## 2026-11-19 — Secure every Samba transaction entry point and enforce private 0700 state

- Review after `536f6f0e` found two additional ways into the same state journal/passdb: `run_samba()` and public `write_smb_conf()` in `services_stor.c`. The former still created `/etc/samba` with mode 0777 before a path-based chmod; the latter recovered the journal without securing its parent. Guarding only `start_wins()` was incomplete.
- The parent-directory helper now enforces exact effective-UID-owned 0700 (not merely no group/other write), using an opened `O_DIRECTORY|O_NOFOLLOW` fd and post-`fchmod` verification. All three transaction entry points — `start_wins()`, `run_samba()`, and `write_smb_conf()` — prepare the parent before recovery. Removed the redundant path-based `samba_secure_private_samba_dir()` check and the remaining permissive `mkdir_if_none(..., "777")`.
- The first commit's XY-C1 run `44593848957` failed only because its regression compile omitted `-I trunk/user/rc/src`; workflow-only commit `287aa1cb8105de24dc6683485087ccdbcb12f062` corrected it. Its exact XY-C1 run `44595726202` passed source/input detection; Build/TRX/artifact were skipped because the SHA contained workflow-only changes. Initial dedicated Samba run `44593849014` passed. New all-entrypoint checks and firmware build for this production-source change are pending.
- No local build or physical-router result is claimed.

---

## 2026-11-19 — Harden Samba passdb and recovery parent-directory permissions

- Source review found `start_wins()` used `mkdir_if_none("/etc/samba", "777")`. PID 1 runs with a permissive umask, so a missing directory could become world-writable. The helper also accepts an existing path without validating its type, owner or mode. This directory parents `smbpasswd` and the root-only `.config-transaction` journal; group/other write access lets local users replace or remove passdb/recovery entries despite the child transaction directory's 0700 mode.
- Replaced the blind mkdir with descriptor-based parent preparation: create as 0755 even under umask 0000; open with `O_DIRECTORY|O_NOFOLLOW`; require effective-UID ownership; remove group/other write bits while preserving safe read/execute access; revalidate the opened directory after permission repair. Symlinks, non-directories and foreign-owned paths fail closed before WINS recovery/publication.
- Added GNU89 ASan/UBSan regression for permissive umask, legacy 0777 repair, preservation of restrictive 0711 mode, and file/symlink rejection. XY-C1 workflow watches the header/test and runs this regression plus a production caller-order contract. Exact-head regression and XY-C1 Build/TRX/artifact are pending; no local test or device result is claimed.

---

## 2026-11-19 — Correct the UNKNOWN guest rollback source contract

- Exact Wireless radio run `44592415803` on production source `ef421978331ade679241c5bc1c10dbb6350d4004` compiled and passed the radio-state policy, bridge-membership policy, and guest-isolation policy binary with GNU89/Werror/ASan/UBSan. The Python checker then failed at line 161 because it required the old early-return-on-UNKNOWN pattern in both rollback callbacks.
- The new fail-closed down callback intentionally requests interface-down after an UNKNOWN flags probe, then relies on a separate probe in `wireless_guest_rollback_verify_down()`; only that verifier should keep the old UNKNOWN early-return assertion. Updated the source checker to distinguish the two contracts. Exact-head rerun pending.
- On the same source SHA, AI docs `44592415781`, watchdog `44592415729`, iNIC signal `44592415628`, and Process inventory `44592415914` passed; Process, NVRAM, HTTPD and XY-C1 remained in progress at the last poll.
- This commit changes only checker and handoff/roadmap/log documentation; no production C. No L4 router verification is claimed.

---

## 2026-11-19 — Fail closed when guest ebtables isolation setup is incomplete

- Source review found that `restart_guest_lan_isolation()` propagated module, flush and rule-add failures but left pre-existing guest AP interfaces up and bridged. The guest control callers only rolled back interfaces started by the current request, so toggling/rebuilding isolation could leave a configured guest AP connected without its required LAN isolation.
- AP-mode ebtables policy bits are now retained even when the interface-flags probe is UNKNOWN; the code still records the probe error rather than treating the interface as up/down. A failure-count checkpoint isolates errors from module confirmation, `ebtables -F/-X`, and individual rule additions. Any new setup error now triggers down, bridge detach and independent DOWN verification for every configured guest AP using ebtables.
- Guest rollback no longer treats an UNKNOWN initial flags probe as a reason to skip the down request: it attempts down and lets the independent postcondition probe determine completion. RT3352 MII iNIC interface/VLAN mapping is preserved.
- Added GNU89 policy cases for UNKNOWN-triggered rollback requests and ebtables setup failure/mask semantics; extended the existing Wireless production-source contract. Exact-head Wireless L2, AI docs and XY-C1 Build/TRX/artifact for this product-source change are pending.
- The parent `981857e60def0d4237253a3c0b061b85111ebbf5` WAN route and AI handoff Actions `44586115401` / `44586115612` passed; prior product SHA `29fa...` XY-C1 run `44585450325` passed detector, Build XY-C1, TRX and artifact, but does not validate this new wireless source. No local compile or physical-router L4 is claimed.

---

## 2026-11-19 — Fix WAN route source-contract whitespace matching

- Exact-head WAN route run `44585450279` on production source `29fa24741613095593a03f4e602f1d77636a21e0` compiled and ran `route_status_policy.c` with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan successfully. The subsequent Python contract failed at line 275 because its joined string retained a space after the multiline helper's opening parenthesis while the expected token did not.
- Updated both WAN-up and WAN-down helper-call checks to remove all source whitespace before matching. This preserves the semantic assertion while allowing normal multiline formatting. Exact-head rerun pending.
- XY-C1 run `44585450325` was still in progress at checkpoint; all other listed 29fa stability and security Actions completed PASS. No product build result is inferred from the route-policy host test.

---
## 2026-11-19 — Remove the orphaned off-subnet WAN gateway host route

- `wan_up_internal()` installed a metric-1 `/32` gateway host route for valid off-subnet gateways on non-PPP interfaces. `wan_down_internal()` deleted the default route plus DHCP/static route inventories, but did not delete that independently-created host route. It could survive WAN DHCP lease loss or replacement.
- A shared route-policy predicate now guards both setup and teardown using valid-gateway, non-PPP and off-subnet conditions. Teardown attempts the host-route delete after the default-route delete, treats only ESRCH as idempotent absence, accumulates other errors, and continues ordinary route cleanup.
- Added GNU89 policy regression for PPP/on-subnet/invalid exclusions, absent route handling and first-error preservation; extended WAN route workflow contracts for setup/delete symmetry and ordering. Exact-head WAN route L2 and XY-C1 L3 are pending. No local cross-build or physical WAN test is claimed.

---
## 2026-11-19 — Correct NDIS local scope and check all USB queue notifications

- Follow-up exact-head XY-C1 runs 44578459810 and 44580744748 failed at Build XY-C1 because interface_down_status, interface_down_errno, interface_flags, interface_error and interface_cleanup_status had been declared in mdev_wdm_main() rather than mdev_net_main(). Moved the locals to the NDIS handler; the first compiler diagnostics are recorded in SESSION_HANDOFF.md.
- The USB workflow checker-only repair 4c331818077d8b9ddfdb5998e35e9c8e0b0add6f passed exact-head USB run 44582391137 and AI docs 44582391240. This commit carries that corrected contract forward and adds the missing printer-handler checks.
- Replaced the two remaining void notify_rc() printer calls with the same errno-logging checked queue helper used by modem NDIS/TTY. The printer hotplug command now returns nonzero if the queue markers were not accepted. All USB hotplug events are checked; exact-head USB and XY-C1 reruns pending.
- No local build or physical USB device validation is claimed.

---

## 2026-11-20 — Align USB hotplug checker with checked PID-1 notifier

- Base main SHA: `0f719780b4d1b1d38b0300855b8424d74d033e20`. Exact-head USB push run [44580744698](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44580744698) failed in the WDM/NDIS source-contract step after all five GNU89, -Wall/-Wextra/-Werror, ASan/UBSan policy binaries passed. The assertion still required the removed `notify_rc("on_unplug_usb_modem")` call even though the production handler now uses the checked `notify_modem_hotplug_event(...)` helper.
- Updated the cleanup-before-notification contract to check the checked unplug notifier call. The existing assertions continue to require checked NDIS/TTY plug/unplug events, preserve status precedence, and reject legacy void notifier use. No production source changed in this checker correction.
- On the same source SHA, Process inventory `44580744598`, Process fail-closed `44580744806`, AI docs `44580744704`, and HTTPD security `44580744667` passed; NVRAM `44580744629` and XY-C1 `44580744748` were in progress. THIS COMMIT's exact-head USB, NVRAM, and XY-C1 results must be checked after push. No local compile, MIPS build, or physical-router validation is claimed; L4 remains NOT VERIFIED.

---

## 2026-11-19 — Propagate modem hotplug queue publication errors

- notify_rc.h exposes notify_rc_status(), whose contract returns failure when secure incomplete/action queue markers cannot be accepted; legacy notify_rc() intentionally discards the result. NDIS and serial TTY hotplug handlers used the void wrapper for modem plug/unplug events, so they could report success while PID 1 had no queued transition to process.
- Replaced the four modem NDIS/TTY plug/unplug notifier calls with a checked helper that preserves/logs errno and merges queue failure into each handler's returned metadata status. An unplug notification is still attempted after interface-down or metadata cleanup failure; first nonzero status remains authoritative.
- Added GNU89 policy coverage for queue success/failure and status precedence. Extended the USB modem identity workflow contract to pin NDIS and TTY plug/unplug notification wiring and reject legacy void calls in those handlers. Exact-head USB L2/XY-C1 L3 pending; no local compile or physical modem test is claimed.
- The preceding NDIS unplug status commit 8fea3095d59b3278a9318d401eadad0606c193b7 passed USB workflow 44578459451, Process inventory 44578460016, Process fail-closed 44578459789, AI docs 44578459995, NVRAM 44578460023, and HTTPD 44578459942; XY-C1 44578459810 remained in progress at checkpoint.

---

## 2026-11-19 — Preserve NDIS ifconfig-down failures on USB removal

- `mdev_net_main()` discarded the result of `ifconfig(device_name, 0, "0.0.0.0", NULL)` on NDIS removal. A vanished USB interface is an expected ENODEV race, but a failed down operation on an interface that still exists was being reported as success; state-probe failures were also hidden.
- The remove path now captures ifconfig status and probes interface flags only on failure. Confirmed ENODEV is treated as already removed; an extant interface preserves the original ifconfig failure, while UNKNOWN preserves the probe error and fails closed. WAN-owner notification is still attempted after cleanup failure.
- Added GNU89 policy cases for successful down, extant-interface failure, physical-removal ENODEV, and UNKNOWN probes. Extended the USB modem identity workflow contract to verify status handling precedes but does not suppress WAN notification. Exact-head USB L2/XY-C1 L3 pending; no local compile or hardware test is claimed.
- The preceding guest ebtables source SHA `f0406397927cc648c7fd70c1961fb3aba1fd1243` passed Wireless `44576171024`, AI docs `44576170956`, NVRAM `44576171017`, Process fail-closed `44576171294`, inventory `44576170689`, HTTPD `44576170606`, iNIC signal `44576171003`, and watchdog signal `44576171120`; XY-C1 `44576170960` remained in progress at checkpoint.

---

## 2026-11-19 — Preserve guest ebtables state when module probes are unknown

- Selected a separate Wireless stability defect after the iNIC force-disable fix. `is_module_loaded()` converts the tri-state module probe's UNKNOWN/error result to false, so guest-isolation disable could skip stale ebtables cleanup yet return success. The enable path also trusted `module_smart_load()` even though its zero return means either already-loaded or load failure.
- Guest-isolation enable now probes `ebtable_filter` after the load attempt and refuses to flush/install rules unless the module is confirmed present. Disable maps module state to three explicit actions: loaded = cleanup then unload; absent = no-op; unknown = record the probe error and attempt `-F`/`-X` cleanup without unloading modules.
- Added GNU89 unit coverage for loaded/absent/negative-errno enable states and all three disable actions. Extended the production source checker to pin tri-state probe, error-recording, cleanup, and unload ordering. Exact-head Wireless/AI docs and XY-C1 validation are pending; no local compile or physical-router result is claimed.
- Preceding iNIC force-disable exact-head Wireless radio run `44574370283` and AI docs run `44574368349` passed; Process lifecycle regression `44573883051` and NVRAM transaction audit `44573883023` passed. XY-C1 `44573883318` was still in progress on iNIC product-source SHA `3d86aebbef26086a4383489efc4470e35e91b9df` at the last poll.

---

## 2026-11-19 — Correct iNIC disable source-contract matcher

- Exact-head Wireless Action `44573883138` for source commit `3d86aebbef26086a4383489efc4470e35e91b9df` passed the GNU89 `-Wall -Wextra -Werror` policy regression, bridge policy regression, and guest isolation policy regression. Its Python production-source contract then failed because the newly added expected multiline call literal was over-escaped and therefore did not match the actual C statement.
- Fixed the checker to assert the stable call prefix and argument token independently, retaining the separate assertion that the call precedes bridge detach, PHY isolation/RGMII disable, GPIO reset, and the final status return. No production code or test behavior was weakened.
- Exact-head AI docs, Process inventory, iNIC signal, watchdog signal and HTTPD checks passed on `3d86aeb`; Process regression, NVRAM and XY-C1 were still pending at the last poll. This checker follow-up's exact-head Wireless and XY-C1 runs must be checked after commit.

---

## 2026-11-19 — Preserve iNIC force-disable interface failure in returned status

- Selected a separate RC wireless stability defect after closing WAN gateway ARP identity. In `start_inic_mii()`, the explicit `inic_disable` branch discarded the result of `wif_control_success(ifname_inic, 0)`; it still continued bridge detach, PHY isolation, RGMII disable and reset, but the returned status could be successful when the initial interface-down failed and later cleanup succeeded.
- The force-disable branch now merges the interface-down boolean into the sticky `all_started` result before all later safety operations. It does not short-circuit or skip bridge/hardware/reset cleanup on failure.
- Added `test_inic_force_disable_interface_failure_is_sticky` to model the failed ifconfig operation followed by successful detach/PHY cleanup, and added an exact production/source regression assertion in `.github/workflows/wireless-radio-state.yml`.
- Prior WAN ARP product SHA `fe286b8cabd4334664c8270c5212d26a15ce6393` reached exact-head L2 and XY-C1 L3 PASS; the corrected AI handoff contract `44573162136` passed on `f3f39d7d6e7acde293982a51e9dcab76994b3c8e`. This new wireless slice is pending exact-head validation. No local toolchain or physical iNIC test is available.

---

