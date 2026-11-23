## 2026-11-23 UTC — Align source contracts with the DDNS publisher and PPP response regression

- The exact-head WAN run `45171234395` exposed a brittle test-name assertion: it looked for `function();`, while the test correctly checks each function's return with `CHECK(function() == 0)`. Relaxed the source contract to require the actual function call.
- The exact-head Process lifecycle run `45171234444` exposed a stale boot contract that still expected inline `copyscripts_seed_status=1` in the DDNS call site. The shared `seed_storage_file()` helper owns that error aggregation now; the checker explicitly requires the DDNS helper invocation and rejects the old direct `cp -f` path. The preceding test step confirms the symlink and external sentinel behavior.
- Both were checker mismatches revealed by CI; no production source change in this follow-up. New exact-head WAN, Process and XY-C1 runs must verify the revised contracts and RP-L2TP host test.

---

## 2026-11-23 UTC — Propagate RP-L2TP control-socket command failures

- `start_rpl2tp()` treats a zero exit from `/usr/sbin/l2tp-control start-session` as a successful session. The bundled controller previously returned zero on every clean socket EOF, even when `cmd.so` had replied `ERR Unknown peer` or another command error; it also ignored partial/failed `writev()` and could block indefinitely waiting for a reply.
- The controller now parses the daemon's `OK`/`ERR` response token before returning success, rejects empty/truncated/malformed replies, writes the command with an EINTR/partial-write loop, and applies five-second send/receive socket timeouts. Added GNU89 ASan/UBSan response-policy regression and checker/workflow path coverage; XY-C1 firmware validation is triggered for the production handler source. Exact-head Actions/build pending.

---

## 2026-11-23 20:02:52 Beijing — Prevent premature AP relay lock release

- Fixed a process-state bug in the generated AP relay monitor: its cleanup gate checked `ap_script_num` twice, so it removed `/tmp/apc.lock` when `ap_script.sh` had exited even if `sh_ezscript.sh` was still performing the relay scan. This allowed a second scan/reconfiguration to overlap the first.
- The gate now requires both independently measured counts to be zero. Added a shell regression that extracts and executes the exact production fragment under mocked process tables for script-only, helper-only, both-running and neither-running cases. Wired the test and AP script into Process CI push/PR filters and XY-C1 firmware input filtering.
- Exact Process/XY-C1 validation for this SHA is pending; no local shell execution or firmware result is claimed.

---
## 2026-11-23 UTC — Correct the DDNS storage regression fixture

- Pre-run review found the new DDNS publication test's `printf` format had a doubled backslash and would write a literal `\\n`, so the preservation assertion would fail for a fixture-format reason.
- Changed the fixture to pass the string separately to `printf '%s\\n'`. Production code is unchanged; exact-head rerun is required.

---

## 2026-11-23 UTC — Publish the DDNS factory script through the safe storage writer

- The boot-time DDNS default path checked for a symlink and then used `cp -f` on the destination. That check/write sequence was not atomic: a concurrent replacement could redirect the privileged copy through a symlink, and the separate chmod also left a partial publication window.
- Reused the existing `seed_storage_file()` transaction, which validates the storage root/parent/leaf, stages beside the destination, applies mode 0755 before publication, and links without replacing a concurrent destination. Existing operator-managed regular files remain untouched; symlink and non-regular targets fail closed and join the boot status.
- Extended the script host regression and AliDDNS source contract to require DDNS default seeding through the shared no-follow publisher. The workflow already triggers for the source, test and checker paths. Exact-head AliDDNS, Process/XY-C1 and firmware checks are required; no local test or target build is claimed.

---

## 2026-11-23 19:48:37 Beijing — Reject malformed Ethernet source MACs in gateway ARP replies

- During a follow-on review of `detect_wan_arp_reply_matches()`, found it verified that Ethernet source and ARP sender-hardware fields matched each other but did not verify the sender MAC was a valid unicast station address. An attacker or malformed bridge frame could use a multicast, all-zero or broadcast sender address and still be treated as a live gateway reply, suppressing DHCP recovery.
- Added an explicit responder-MAC policy: unicast locally administered addresses remain valid, while multicast, all-zero and all-broadcast addresses are rejected before IP tuple matching. Added GNU89 regression cases for all four invalid forms and the missing sender-hardware pointer; the dedicated WAN ARP workflow now pins the new production check and test.
- Exact-head WAN ARP regression and XY-C1 Build/TRX/artifact are pending. No local sanitizer, firmware or physical-router result is claimed.

---
## 2026-11-23 19:44:36 Beijing — Repair OpenSSH lifecycle CI contracts

- Exact-main Process run `45167154533` exposed a stale integration fixture: `check_optional_app_start_status.py:check_sshd()` still expected the wrapper to call raw `killall`, but the new standalone stop correctly refused to proceed without its shared process-lifecycle helper (status 2). Added a test-local helper adapter that delegates to the existing mocked `killall`, and asserted the wrapper passes exactly `sshd` while propagating status 61.
- Exact-main AI documentation run `45167154428` found that the Process regression push allowlist included `trunk/user/openssh/Makefile`, while its pull-request allowlist did not. Added the same packaging input to PR filters; the lists now remain identical.
- These changes repair test/trigger contracts only. The OpenSSH source hardening remains in `e525274ee5a75490627bdc11593a5e3da9b53558`; exact-head Process and AI-doc reruns are pending, as is XY-C1 firmware validation. No local test, MIPS build or hardware result is claimed.

---
## 2026-11-23 19:38:16 Beijing — Fail closed on standalone OpenSSH stop

- Found a lifecycle gap in the standalone `/usr/bin/sshd.sh stop` entry point: `func_stop()` issued `killall -q sshd` without checking the initial process inventory or confirming shutdown. The main RC `stop_sshd()` path already uses the shared lifecycle transaction, but the script's direct CLI bypassed it.
- Replaced the raw signal with the shared `pl_stop_named` transaction (tri-state inventory, bounded TERM wait, fresh recheck before KILL, and confirmed-absence postcondition). The OpenSSH package now installs `/usr/lib/process_lifecycle.sh` itself; a missing, symlinked or unreadable helper fails closed rather than reverting to `killall`.
- Extended the existing OpenSSH shell regression for delegation, missing-helper refusal and helper error propagation; added a source contract and included the OpenSSH Makefile in Process and XY-C1 build triggers. Exact-head Process/XY-C1 validation pending; no local shell run or target image is claimed.

---
## 2026-11-23 UTC — Reject duplicate static VPN peer addresses

- `vpn_server_read_acl_snapshot()` validated every per-client address value individually but `start_vpn_server()` did not validate uniqueness across the snapshot. Two authenticated PPP clients could therefore receive the same configured static remote address in `chap-secrets`, creating a peer-address collision and ambiguous routing.
- Added a bounded policy check for the complete ACL snapshot before lease-inventory cleanup, private CHAP file creation or daemon startup. Repeated nonzero static offsets now fail with `EADDRINUSE`; zero remains the supported dynamic-assignment value and can be shared. GNU89 policy coverage includes distinct, duplicate, dynamic-only, null and count-overflow cases.
- Exact VPN Server regression rerun is required. No local MIPS/uClibc build, sanitizer run or hardware behavior is claimed in this record.

---
## 2026-11-23 UTC — Enforce removal of all unchecked URL MAC fallbacks

- After the IPv6 source fix, a fresh source scan confirms exactly two URL-MAC consumers (IPv4 and IPv6) call `firewall_read_url_filter_mac()` and there are no remaining `mac_conv("url_mac_x")` fallbacks anywhere in `firewall_ex.c`.
- Restored a whole-file negative assertion in the Firewall Startup contract now that the previously missed IPv6 branch has been converted. The per-protocol checked-read → abort → staged-file removal → return ordering assertions remain in place. The exact Firewall Startup rerun on the IPv6-fix SHA #45165224108 passed; this adds a regression guard for future accidental fallback reintroduction.

---
## 2026-11-23 UTC — Close IPv6 URL-filter MAC fail-open gap

- Source review after the IPv4 URL-MAC checker repairs found the matching IPv6 `ip6t_filter_rules()` branch still called `mac_conv("url_mac_x")`. Unlike the updated IPv4 branch, it still collapsed an NVRAM backend failure into an empty qualifier and could publish an IPv6 URL-filter rule without its per-client MAC restriction. This was a real fail-open gap, not a checker-only issue.
- Replaced the IPv6 fallback with the shared presence-aware checked reader. Backend or malformed-value errors now abort core restore, remove the staged rule file, preserve errno and return before appending the URL-filter rule, matching IPv4 behavior. The destination is sized to the exact 18-byte formatted-MAC requirement.
- Existing Firewall Startup source contract iterates both `ipt_filter_rules()` and `ip6t_filter_rules()` and verifies checked-reader → abort → staged-file removal → error return precede the MAC rule. Exact-head Firewall Startup rerun required; no local build or sanitizer result is claimed.

---
## 2026-11-23 UTC — Scope URL MAC legacy-call assertion to URL builders

- Firewall Startup #45164776940 on `d9796188e245b2cd2ae752b865b140b4f0959a8b` passed the checked-helper and policy fixture assertions but failed a global source assertion that prohibited every `mac_conv("url_mac_x")` call in `firewall_ex.c`. The remaining legacy call is in an unrelated filter-generation path outside the IPv4/IPv6 URL-rule builders; the URL builders themselves already have explicit scoped assertions forbidding that fallback.
- Removed the over-broad global assertion while retaining both per-builder checks that each URL filter uses the checked helper and cannot emit a rule after a read failure. No runtime source was changed. Exact Firewall Startup rerun required.

---
## 2026-11-23 UTC — Repair Firewall Startup checker variable reference

- The exact-head Firewall Startup rerun #45164580872 progressed past the prior multiline-source assertion and then raised `NameError`: the checker referred to `firewall_policy`, but its existing fixture variable is named `filter_policy`. HTTPD policy regression #45164581007 passed, including the corrected 17-byte undersized-buffer test.
- Updated the Firewall Startup contract to use the already-loaded `filter_policy` variable. This is a checker-only fix; rerun Firewall Startup on the next main SHA.

---
## 2026-11-23 UTC — Correct URL MAC regression test boundaries

- Exact-head run #45164372502 stopped in the Firewall Startup source checker because the new source assertion expected a checked-getter call on one line while production intentionally wrapped its arguments across lines. HTTPD regression #45164372525 compiled the new firewall policy test and exposed a bad test boundary: a 23-byte buffer was expected to fail even though the formatter contract requires only 18 bytes.
- Fixed the source contract to validate the normalized multiline call and changed the negative buffer test to pass 17 bytes. Production firewall code is unchanged; rerun exact-SHA Firewall Startup and HTTPD checks before accepting the fix.

---
## 2026-11-23 UTC — Fail closed on URL-filter MAC qualifier reads

- Found that both IPv4 and IPv6 URL-filter builders called the generic `mac_conv("url_mac_x")`, which uses error-collapsing `nvram_safe_get()` and only checked stored string length. An NVRAM backend error could silently remove a configured MAC qualifier, and a 12-byte non-hex value could become an untrusted bare `--mac-source` token in the staged iptables-restore file.
- Added a checked, presence-aware URL MAC reader. Confirmed missing/empty/zero/broadcast values retain the historical optional-no-qualifier behavior; valid 12-hex-digit values are formatted through the existing strict MAC policy; backend errors and malformed present values abort and remove the unpublished IPv4/IPv6 rule file before core restore publication.
- Extended the GNU89 firewall policy test with missing/present/read-error/invalid-input cases and the Firewall Startup source contract with both-family abort-before-rule assertions. Exact-SHA Firewall Startup, HTTPD and XY-C1 runs are required; no local compile, MIPS build or hardware result is claimed.

---
## 2026-11-23 UTC — Match watchdog timezone source filters for pull requests

- AI handoff check #45163084187 on `da6c721111852b8ca888d2797717a9b29b4713fb` exposed two path-filter asymmetries in `.github/workflows/process-lifecycle-regression.yml`: the watchdog-timezone policy header and its GNU89 regression test were added to `push.paths` by the timezone scheduler fix but omitted from `pull_request.paths`.
- Added both exact paths to the pull-request filter so PR validation now runs for the same source/test changes as main pushes. This repairs the governance failure without relaxing the parity check. Exact-head AI docs rerun required.

---
## 2026-11-23 UTC — Update HTTPD umbrella contract for checked firewall schedule calls

- HTTPD authentication/security run #45162166214 on code SHA 444674117db122af04538cff330777f5421dd0e4 passed all HTTPD auth/upload/route contracts, then failed in its shared firewall source audit at inline Python line 831. That umbrella checker still required two direct `if (timematch_conv(...))` calls which were deliberately replaced in ad562 by explicit `url_schedule_status = timematch_conv(...)` handling so NVRAM errors can abort the staged IPv4/IPv6 restore.
- Updated the umbrella contract to require both checked call sites and both negative-error gates. The dedicated Firewall Startup source-order contract remains stronger and still verifies abort/cleanup/return before rule publication. No production code changed in this checker-only repair; exact HTTPD rerun required.

---
## 2026-11-23 UTC — Keep scheduled actions on the last valid timezone

- Found an unchecked NVRAM access in `watchdog.c:timecheck_reboot()`: unlike the checked timezone setup used by PID 1, this scheduler still called `nvram_safe_get("time_zone_x")` and overwrote process `TZ` with an empty value on a backend error. The resulting local-time comparison could trigger a configured reboot/update at the wrong wall-clock time. The same function also ignored `time()` failure and could evaluate an uninitialized timestamp.
- Reused the existing `timezone_env_policy.h` checked-read formatter. The scheduler now validates/copies the timezone before changing `TZ`, preserves the prior environment and skips the scheduled action on read/validation/setenv errors, rate-limits repeated error logs, and checks the clock result before calling `localtime()`.
- Added the existing timezone GNU89/Werror/ASan/UBSan policy test to the Process lifecycle workflow and source-order assertions for checked read → validate → apply → clock. Process and XY-C1 exact-head Actions are pending; no local compile or MIPS result is claimed.

---
## 2026-11-23 UTC — Retry LAN networkmap when completion state cannot be inspected

- Found a tri-state recovery bug in `udhcpc_lan_bound()`: a missing or inaccessible PID 1 incomplete-action directory made `lan_dhcp_networkmap_retry_pending()` return a negative error, but the caller converted that UNKNOWN state to zero. An unchanged DHCP RENEW then skipped retrying a previous failed networkmap refresh.
- Added `lan_dhcp_networkmap_retry_required()`: only a confirmed absent marker (0) may suppress a retry; a present marker (1) and any inspection error (negative) both conservatively requeue the idempotent networkmap request. The DHCP lease remains independent of this ancillary refresh.
- Added GNU89 policy regression cases for absent, present, EIO and ENOENT probe results, and strengthened the production source-order contract to prevent UNKNOWN from being normalized to “no retry”. Exact-head WAN DHCP callback and XY-C1 Actions are pending; no local sanitizer, MIPS build or hardware result is claimed.

---
## 2026-11-23 UTC — Fail closed on URL-filter schedule NVRAM reads

- `timematch_conv()` previously used `nvram_get()` and substituted the all-days/all-day legacy defaults whenever a lookup returned NULL. Because that path could not distinguish an absent key from an NVRAM backend failure, a read fault could silently broaden the URL-filter activation schedule. It now uses `nvram_get_checked()` via the presence-aware string reader, copies each value before the next shared-cache read, preserves defaults only for genuinely missing keys, and returns an error for backend failures.
- Both IPv4 and IPv6 URL-filter rule builders now abort the staged filter restore, remove the unpublished rule file and preserve the read errno when either schedule lookup fails. Present-empty and oversized values remain inactive, matching existing schedule validation semantics. The policy helper and GNU89 ASan/UBSan test cover missing defaults, present-empty, backend EIO, oversized values and invalid destinations.
- Product-source commit `ad56220d066e771be499426708b6c2f744937467` (`fix(firewall): fail closed on URL schedule read errors`) had Firewall Startup run #45161165818 fail only at a stale test-name assertion after the preceding checked-NVRAM, lifecycle, startup, enable and static-route checks passed. Checker-only follow-up `db60eb4deb95f7702c63f50563b8cc9b23d350e9` corrected the assertion; exact Firewall Startup #45161387113 PASS and AI docs #45161387099 PASS on db60. WAN DHCP callback #45161165802 also PASS on ad562. XY-C1 #45161165974 on the product-source SHA ad562 remained in progress at Build XY-C1, with TRX and artifact pending at last poll.
- The prior LAN networkmap-retry product slice `edeb1e8354a97b312d8f3f5dd8787af030abfc6d` is L2-verified by WAN DHCP callback #45160239807 on descendant checker SHA cbec. Exact XY-C1 #45160010405 on edeb completed Build, TRX verification and artifact upload; artifact `XY-C1-firmware-edeb1e8354a97b312d8f3f5dd8787af030abfc6d` (12,759,150 bytes, ID 14811585463). This confirms L3 through edeb, not the later firewall URL-schedule source.
- No local MIPS/uClibc build, local sanitizer run or physical-router test is claimed; sanitizer and policy results above are GitHub Actions results only.

---
## 2026-11-23 UTC — Fix firewall URL schedule regression contract

- Firewall Startup run #45161165818 on source SHA ad56220d066e771be499426708b6c2f744937467 passed the firewall IP checked-read contract and lifecycle/startup/enable/static-route GNU89 ASan/UBSan binaries, then failed at inline Python line 634. The checker referred to a nonexistent standalone test function name; the tests are assertions inside the existing `main()` and directly exercise `firewall_schedule_copy_checked_value()`.
- Corrected the contract to assert the helper is exercised by `firewall_filter_policy.c`. No runtime C or test code changed in this follow-up. Exact Firewall Startup rerun is required. The initial exact WAN DHCP callback #45161165802 completed PASS on ad56220. XY-C1 #45161165974, Process inventory #45161166010, AI docs #45161165835 and NVRAM audit #45161165871 were in progress at last poll; no result is inferred.

---
## 2026-11-23 UTC — Normalize notification macro source-contract whitespace

- WAN DHCP callback run #45160010474 on product/checker SHA edeb1e8354a97b312d8f3f5dd8787af030abfc6d compiled and passed the initial USB-modem, WAN interface-name, DHCP lease policy and WAN NVRAM ASan/UBSan binaries, then stopped at the new source-contract assertion for `RCN_RESTART_NETWORKMAP`. The checker expected one literal space while the shared header aligns macro definitions with tabs; this was a formatting-only checker false negative.
- The assertion now removes source whitespace before comparing the macro name/value. Runtime code, queue policy and test semantics are unchanged. Exact-head WAN DHCP callback rerun required. Networkmap signal lifecycle #45160010227, PID 1 notification queue #45160010473, Wireless #45160010276 and AI docs #45160010425 already PASS on edeb1e8. XY-C1 #45160010405, Process umbrella #45160010508 and Process inventory #45160010184 were still in progress at last poll. No local MIPS build or hardware result is claimed.

---
## 2026-11-23 UTC — Reject non-unicast Ethernet source addresses in WAN ARP

- Extended the new ARP source-MAC parser beyond syntax: a sender address must be unicast, not all-zero and not broadcast. A syntactically valid multicast/group MAC (for example 01:00:5e:...) or reserved zero/broadcast value is not a valid station source address for the router's Ethernet/ARP probe, even though every nibble is hexadecimal.
- Kept locally administered unicast addresses accepted (02:...) and added explicit globally administered unicast coverage (00:11:22:...). Added transactional rejection cases for zero, broadcast, IPv4 multicast and IPv6 multicast source MACs; the output buffer remains unchanged on failure.
- The check remains local to ARP frame construction and does not change the historical WAN override formatter or NVRAM values. Exact-head regression and XY-C1 Build/TRX/artifact checks for this source refinement are pending.

---

## 2026-11-23 UTC — Validate WAN ARP source MAC before packet construction

- Confirmed a malformed-input boundary in arpping_gateway(): the WAN/LAN MAC is read through checked NVRAM APIs, but each octet was decoded with strtol(tmp, NULL, 16). The end pointer was ignored, so non-hex legacy override text silently became zero or a partially parsed byte and was transmitted as the Ethernet/ARP source address.
- Added an ARP-boundary-only parser for exactly six colon-separated hexadecimal octets. It accepts uppercase/lowercase hex and locally administered addresses, rejects malformed lengths/separators/nibbles/null pointers, and leaves output untouched on failure. The legacy WAN override formatter is deliberately unchanged for configuration compatibility.
- Replaced the permissive strtol loop in the live ARP probe. An invalid source identity now produces EINVAL/probe error before socket creation or packet construction, so it cannot be mistaken for a gateway no-reply or trigger DHCP renewal.
- Added GNU89 -Wall -Wextra -Werror ASan/UBSan regression cases for valid mixed-case parsing and transactional rejection of malformed/missing values. Extended the existing ARP source contract to require strict parser use, removal of the permissive decoder, and the parse/error gate before socket creation. Exact-head WAN ARP and XY-C1 Build/TRX/artifact checks are pending; no local or hardware result is claimed.

---

## 2026-11-23 UTC — Complete compact DBDC return-order assertions

- Exact Wireless #45157966807 on SHA 280e959c passed the wireless policy executable, Ralink config ASan/UBSan, bridge membership, guest isolation and the first DBDC token-presence checks, then failed with `ValueError: substring not found` at inline Python line 656. The second DBDC ordering lookup still searched for `return -<peer_status>;` in a whitespace-stripped function string. The prior correction fixed the token-presence assertion but missed this second occurrence; this checkpoint aligns the remaining ordering lookup with the same compact representation. Production wireless code is unchanged.
- AI handoff documentation #45157966954 passed on 280e959c after restoring the validator-required heading `## Exact first action for the next GPT`.
- XY-C1 #45157966804 on 280e959c was IN_PROGRESS at last poll, with the build step not yet completed; TRX and artifact outcomes remain unverified. The source product baseline remains 0e4976cf, whose #45156939135 build was also still in progress at last poll. Checker-only descendant runs with skipped product steps do not establish L3.

---

## 2026-11-23 UTC — Normalize DBDC error-return contract and restore handoff heading

- Exact Wireless run #45157423895 on source SHA d9a5f32 passed the GNU89 policy binaries, Ralink config/SKU ASan/UBSan, bridge membership and guest-isolation tests, then failed at the DBDC peer-band source contract (inline Python line 649). The checker compacts all whitespace from the function body but expected `return -<status>;` with a space; the production source correctly returns `-rt_radio_status` / `-wl_radio_status`, which becomes `return-<status>;` in the compact representation. Updated the assertion to use the compact form; no runtime source change.
- Exact AI handoff run #45157423846 failed because the handoff used heading `## Exact first action for the next session`, while the validator requires `## Exact first action for the next GPT`. Restored the protocol/validator-required heading and refreshed the checkpoint facts.
- XY-C1 run #45157423889 on d9a5f32 completed workflow SUCCESS but Build XY-C1, TRX verification and artifact upload were all SKIPPED because this was checker/docs-only. Product-source XY-C1 #45156939135 on 0e4976cf remained IN_PROGRESS at Build; TRX/artifact pending. The latest confirmed full Build/TRX/artifact PASS remains #45154946558 on descendant product SHA eac5eeee. No L3 is claimed for 0e4976cf.

---

