## 2026-11-23 14:45 UTC+8 — Validate VPN server subnet text before deriving NAT network

- Audited the new checked `vpns_vnet` snapshot in `ipt_nat_rules()`. Although backend errors, transaction-close errors and overlong strings already fail closed, any short malformed IPv4 token still reached legacy `ip2class()`, which calls `inet_addr()` without checking its parse result and can derive a bogus network from `INADDR_NONE`.
- Added `firewall_nvram_ipv4_address_is_valid()` using strict `inet_pton(AF_INET)`; invalid/missing VPN server pool text now returns EINVAL before IPv4 NAT staging. Existing bounded checked copy remains first.
- Extended the existing GNU89/ASan/UBSan firewall NVRAM policy test for valid dotted quads and NULL/empty/short/CIDR/out-of-range/leading-zero/trailing-junk inputs. Extended the Firewall Startup source contract to pin validation before both staged-file open and `ip2class()`.
- Prior Firewall mangle transaction regression exact run `45119408526` PASS, including all source and sanitizer tests. FTP mode-gating exact run `45119601925` PASS. Firefly credential safety `45119820496` PASS. The current pre-change d02 XY-C1 `45119820478` was still running; this production firewall source change requires an exact-head XY-C1 build/TRX/artifact and fresh Firewall Startup.
- No local checkout or local build claimed. Resulting commit: THIS COMMIT.

---

## 2026-11-23 14:35 UTC+8 — Avoid reading anonymous FTP rate when the active mode does not use it

- Reviewed concurrent vsftpd hardening from `b330a2f96cc15730b591c8fd5e49a7a63b4bb5a4`. The checked `st_ftp_anmr` read correctly prevented backend errors from becoming `anon_max_rate=0` (unlimited), but it was unconditional: modes 1/2 do not emit or consume `anon_max_rate`, yet any rate-key read fault prevented configuration publication.
- Added `ftp_transfer_rate_setting_required()` and moved normalized FTP-mode selection before the anonymous-rate snapshot. The checked rate read and clamp now run only for modes 3/4, which actually output `anon_max_rate`. Read faults in relevant modes still preserve the active config and fail closed.
- Extended the existing GNU89 ASan/UBSan policy regression to assert modes 3/4 require the setting while modes 1/2 and unsupported modes do not. Updated the workflow source contract to require mode normalization before the guarded rate read and registration of predicate tests.
- Firewall transaction fix `2c911df08141a3810f7d53cf8d0eaf1536e96a88` achieved exact L2 PASS on `1a99a9c543d237398294ef5738622f151c8d7f8e`, Firewall Startup run `45119408526`: source checker plus all compiled GNU89 sanitizer and integration checks succeeded. XY-C1 run `45119331092` on b330 had passed detection and entered Build; no TRX/artifact result was yet available. Exact new-SHA firmware proof still required.
- Concurrent FTP checked-rate fix's original vsftpd action `45119331064` PASS; this mode-gated refinement needs a new exact-SHA vsftpd rerun. No local checkout/test/build claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 14:26 UTC+8 — Fail closed when IPv4 mangle NVRAM snapshots cannot be read

- Source review found that `ipt_mangle_rules()` returned on errors reading `wan_ttl_fix`, `wan_ttl_value`, conditional `mr_enable_x` and conditional `viptv_ifname` without setting `firewall_core_restore_transaction.failed`. The caller could then continue to NAT/filter and finish with stale mangle rules mixed into the newly restored firewall.
- Added `firewall_mangle_snapshot_failure_status()` to set the transaction failure flag and normalize negative return values to EIO. A source helper logs the failed setting, preserves errno and gates all four preflight reads before side effects; the caller's existing transaction-abort path rolls prior table restores back.
- Added GNU89 sanitizer-policy regressions for success preserving transaction state, positive backend errors, negative status and invalid transaction pointers. The source contract checks all four gates and caller abort-before-NAT.
- Exact Firewall Startup run `45118876044` on 5191 failed only at a literal-whitespace assertion for the new VPN subnet checked-read gate; checker-only commit `4278a6905bc7c30c53690892ed0cb6f2a603bcbf` repaired that assertion and exact run `45119079720` passed. This commit carries that checker correction forward and adds the independent mangle transaction fix.
- Concurrent VPN server multicast failure propagation from `1c5bf5f02bc006484692c592ce5bdc6fd587f06c` is retained. Exact new-SHA Firewall Startup and XY-C1 Build/TRX/artifact are required.
- No local checkout; no local compile/build is claimed. Resulting commit: THIS COMMIT.

---

## 2026-11-23 14:12 UTC+8 — Add missing DHCPv6 source paths to XY-C1 firmware validation

- Exact WAN callback run `45118077270` on contract-correction commit `84110056136496c2f7bcce4034b8450dc4c121a1` completed SUCCESS. Dedicated DHCPv6 sanitizer/source-contract run `45117871967` on product source `80811fc53de0257bf44973fa0d703014981fcaf5` also completed SUCCESS.
- The XY-C1 workflow already included its own workflow file in push paths, but omitted production `dhcp6c.c`, `dhcp6c.h`, `dhcp6c_script.c` and the new retry policy header. Its only related existing path was `dhcp6c_common.c`; thus the DHCPv6 product change did not itself launch firmware validation.
- Added those four DHCPv6 client/source/header paths to the XY-C1 push filter. This commit changes CI trigger coverage only, not product C source.
- While preparing this change, main advanced with commit `3eba7fc820f0c3f9dc2dd7a80db313b39f63fff1` (`fix(firewall): check IPTV interface before TTL rules`). Preserved it as the new parent. Its exact XY-C1 run `45118311710` was in progress at the last lookup after prerequisites; this tree includes DHCPv6 source commit 80811, but no L3 claim is made until exact same-tree Build/TRX/artifact completion.
- Earlier e757, d2c and USB-printer slices have their own exact-SHA XY-C1 Build/TRX/artifact SUCCESS; those do not replace this DHCPv6 target validation. No local checkout or build.
- Resulting commit: THIS COMMIT.

---
## 2026-11-23 14:06 UTC+8 — Align WAN callback source contract with DHCPv6 event retry

- Exact DHCPv6 retry workflow run `45117871967` on `80811fc53de0257bf44973fa0d703014981fcaf5` completed SUCCESS: GNU89 ASan/UBSan policy test and the DHCPv6 source integration checker both passed.
- The same-SHA WAN DHCP callback workflow `45117872004` completed FAILURE only at its final inline Python source contract. Its legacy assertions still required the old DHCPv6 log strings `configuration script failed to execute` / `exited with status %d`; the source fix deliberately replaces them with explicit `retrying event` log messages and a retained-event retry path. All preceding WAN/IPv6/LAN policy binaries and source contracts in that job passed.
- Updated the WAN workflow's stale DHCPv6 callback assertions to require the retry-policy gate, both new diagnostics, retransmission reset and five-second event timer. This is a test-contract migration only; no product C source changed.
- No exact-head XY-C1 run appeared in the returned push-run inventory for SHA 80811 at the latest lookup. The dedicated DHCPv6 host workflow passed, but L3 is still NOT VERIFIED until the same production-source SHA gets XY-C1 MIPS/uClibc build, TRX and artifact upload.
- No local checkout; no local test/build claim. Resulting commit: THIS COMMIT.

---
## 2026-11-23 14:00 UTC+8 — Retry DHCPv6 Reply events when configuration scripts fail

- Confirmed active production path: `start_dhcp6c()` in `trunk/user/rc/src/net_wan6.c` launches `/sbin/dhcp6c`; BusyBox Kbuild includes `dhcp6c.o`, `dhcp6c_script.o` and `dhcp6c_ia.o` under CONFIG_DHCP6C. The legacy `d6_dhcpc.c` is not this router's DHCPv6 client and was not modified.
- Defect: `dhcp6_script()` in `dhcp6c_script.c` returns the real `spawn_and_wait()` result, but `client6_recvreply()` in `dhcp6c.c` only logged a failed callback and unconditionally removed the DHCP event. It also armed Information Refresh Time before the script callback completed. A transient callback failure could discard the DHCP retry event and delay restoration of IPv6 DNS/NVRAM state until a later exchange.
- Fix: initialize script status to success when no callback is configured; on a nonzero status retain the current DHCP event, reset its retransmission parameters using `dhcp6_set_timeoparam()`, schedule the event timer for 5 seconds and return before `dhcp6_remove_event()`. The Information Refresh Time timer is now armed only after callback success. The existing child environment cleanup path remains unchanged.
- Added a policy header and GNU89 host regression for success, positive child exit status, spawn failure and retry interval. Added a Python source contract verifying the callback result, event retention, retry timer and Information Refresh Time ordering. A dedicated workflow compiles/runs the policy under ASan/UBSan and executes the source contract.
- Previous source slices now have complete verification: USB printer unplug commit `20012786541a3789f0391793345e403eb96c77d8` passed storage-hotplug host/sanitizer/source checks in run `45116268933` and exact-SHA XY-C1 build/TRX/artifact in run `45116268960`; FRP source `d2c388a04f6125666043bf94672a8f4222655d4b` passed exact-SHA XY-C1 run `45115500761`; FRP checker correction `41285d396d7bd1f984f13c49699ed6dacbd27e92` passed run `45116127095`.
- No local checkout is available; no local compile or test is claimed. The dedicated DHCPv6 host workflow and exact-new-SHA XY-C1 MIPS/uClibc build, TRX and artifact upload are required before L2/L3 closure. L4 router validation remains unavailable.
- Source, policy, test, checker, workflow and this handoff/log update form one atomic slice. Resulting commit: THIS COMMIT.

---
## 2026-11-23 13:43 UTC+8 — Preserve USB printer unplug retries when spooler shutdown fails

- Source review found a concrete event-loss defect in on_deferred_hotplug_dev(): the usb_unplug_lp branch called stop_usb_printer_spoolers() but discarded its result, then passed action_status=0 to usb_hotplug_event_ack_status() and acknowledged the NVRAM event. services_usb.c confirms that stop_usb_printer_spoolers() aggregates u2ec/lpd/p910nd stop errors and returns a nonzero errno-style status; therefore an un-stopped daemon could be left behind with no unplug retry queued.
- The caller now initializes printer_unplug_status, captures the stop result, skips marker clearing when the stop failed, and passes the real action/marker statuses through usb_hotplug_event_ack_status(). Failed stop remains action-retryable; successful stop plus failed marker clear latches completion so retries only clear the marker and do not repeat the side effect. Existing usblp-in-use policy remains a successful no-op.
- Added a positive EBUSY regression to usb_hotplug_event_policy.c. Extended check_storage_safe_remove.py to require stop-status propagation, marker-clear gating, the action-latch call and retry scheduling. The existing storage-hotplug workflow already compiles the policy test under GNU89 ASan/UBSan and runs the source checker.
- At the latest check, e757 XY-C1 run 45114634020 completed SUCCESS (L3 build/TRX/artifact on that e757 production-source SHA). d2c XY-C1 run 45115500761 was still building on d2c; it does not validate the newer printer source. f932 Process lifecycle 45115896497 passed; f932 FRP 45115896479 exposed a second stale source assertion, which is fixed in parent commit 41285d396d7bd1f984f13c49699ed6dacbd27e92. Its exact-head FRP rerun 45116127095 was in progress at the last lookup; f932 handoff validator 45115896495 and the other previously noted jobs passed.
- No local checkout, so no local host test was run. Require exact new-SHA storage-hotplug regression and XY-C1 target build/TRX/artifact for this source slice. No L4 claim.
- Source, regression and source contract are one logical change with the handoff/log update. Resulting commit: THIS COMMIT.

---
## 2026-11-23 13:34 UTC+8 — Remove the remaining stale FRP inline-copy source assertion

- Exact-head Process lifecycle run 45115896497 on f9320f888b7ac284a4b657b1ae58feca40f0428f completed SUCCESS. This validates the updated scripts/check_dev_init_boot.py and all process lifecycle regression steps, including the FRP credential seeding shell cases.
- Exact-head FRP lifecycle run 45115896479 on the same SHA completed FAILURE in a second, later assertion in scripts/check_frp_lifecycle.py at line 285. The initial checker block had been fixed, but this second assertion still required the removed inline FRP chmod 700 and symlink/regular-file tests rather than the shared seed_storage_file() contract. Every FRP shell regression before that source contract passed.
- Updated the remaining assertion to require the FRP helper invocation, forbid the retired direct cp -f, and assert the shared optional private-mode enforcement branch. This is checker-only; production source is unchanged.
- f932 exact AliyunDrive WebDAV, HTTPD authentication/security and handoff validator runs 45115896556, 45115896513 and 45115896495 completed SUCCESS. The only known f932 failure is this stale FRP checker block; its exact-head rerun is required.
- d2c XY-C1 run 45115500761 and e757 XY-C1 run 45114634020 remain to be inspected for final Build/TRX/artifact conclusions. No local checkout; no local execution claim.
- Corrective checker commit: THIS COMMIT.

---
## 2026-11-23 13:27 UTC+8 — Update FRP source contracts for shared private seeding helper

- Exact-head FRP lifecycle run 45115500757 on product/source SHA d2c388a04f6125666043bf94672a8f4222655d4b completed FAILURE only in scripts/check_frp_lifecycle.py:130: it still required the removed direct cp -f/chmod/-L/-f source block. All prior FRP shell regressions, including private runtime config, exact process stop, managed crontab and lifecycle lock, passed.
- Exact-head Process lifecycle run 45115500771 on the same SHA completed FAILURE only in scripts/check_dev_init_boot.py:315: it still required an inline copyscripts_seed_status=1 assignment inside the FRP branch. The actual shared helper sets the status for every failure; copyscripts_ca_status.sh passed, including the new FRP mode-0700 creation, existing-file mode repair, injected chmod failure and no-old-cp source contract.
- Updated both source contracts to require the common seed_storage_file() call with the enforce-existing-mode flag, two guarded chmod paths and the absence of the old direct FRP cp. This is a checker-only correction; production source remains unchanged.
- Exact XY-C1 run 45115500761 on d2c388a04f6125666043bf94672a8f4222655d4b had passed the firmware input detector and all pre-build host checks; Build XY-C1 was still in progress at the last lookup; TRX and artifact upload were pending. Exact handoff validator 45115500743, inventory 45115500720, Adbyby host config 45115500738 and AliDDNS API transport 45115500805 all passed on d2c.
- No local checkout. No local checker execution. Exact new-SHA FRP and Process Actions must pass before the checker migration is closed. No new L3/L4 claim.
- Corrective checker commit: THIS COMMIT.

---
## 2026-11-23 13:14 UTC+8 — Route FRP credential seeding through exclusive private publication

- Source review found that copyscripts.sh already used seed_storage_file() for ordinary defaults, which stages beside the destination and publishes with an exclusive hard link to avoid following a leaf or parent symlink. The credential-bearing FRP script was a security-sensitive exception: a separate symlink check followed by cp -f had a check/use gap and duplicated path/type/mode handling.
- Extended the shared helper with an optional enforce-existing-mode flag. FRP now uses the same staged/exclusive publication path at mode 0700; an existing operator-managed regular file keeps its content but has its private mode restored on each boot. A concurrently published regular file retains its content while still receiving the requested private mode. Invalid mode policy and chmod failures propagate through the existing boot-seed status.
- Expanded trunk/user/scripts/tests/copyscripts_ca_status.sh for private creation, existing-file content preservation plus mode repair, mode-repair failure, and source wiring that forbids the old direct FRP cp path. The Process lifecycle workflow already watches both changed paths and runs this test; no workflow filter change is needed.
- The preceding libdisk ACL lookup source SHA e757f86309b19f890797b2f36fb15f44f87a7e95 passed its descendant source-contract rerun 45114769360 on 5378fccc2201f4359ea3b0b37f1043d2b813a883. Its exact XY-C1 run 45114634020 is IN_PROGRESS; the NVRAM boundary run 45114633929 is COMPLETED.
- No local checkout is available, so the shell regression was not run locally. The exact-head Process lifecycle Action must validate it. No L3/L4 claim.
- Product source and shell regression changed together. Resulting commit: THIS COMMIT.

---
## 2026-11-23 12:07 UTC+8 — Repair Python syntax in PID1 mask-failure source checker

- Exact-head BusyBox job 137478257344 / run 45109466477 on SHA 81be0de7b68467468789d7a67c590b5eb737a449 compiled and ran both ASan/UBSan C binaries successfully, then failed in scripts/check_busybox_init_reap_queue.py at line 127: JavaScript const/indexOf syntax had been inserted in a Python assertion block.
- Replaced the malformed block with Python index/find/slicing assertions that require failed signal-mask restoration to requeue the batch and invoke pause_and_low_level_reboot(RB_HALT_SYSTEM), with no return to the main loop.
- AI handoff 45109466503 and Process inventory 45109466511 PASS on 81be. USB and NVRAM were still running; XY-C1 45109466474 was in Build XY-C1 with TRX/artifact pending. Exact XY 9de run 45108415640 and ca609 run 45109162782 also remained in Build at their last matching polls; neither is recorded as failed. The earlier curl error 56 log was from a different/unmatched job and is not attributed to them.
- Exact-child wait/reap L3 remains verified on 7c556be. Delayed-signal source L3 remains pending until a same-SHA Build/TRX/artifact result passes.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:02 UTC+8 — Fail closed if PID 1 cannot restore delayed signal mask; correct CI status attribution

- Source review found that check_delayed_sigs() requeued a snapshot when sigprocmask(SIG_SETMASK, &old_set, NULL) failed, but then returned to the main loop. The delayed signal set may still be blocked, leaving PID 1 deaf to reboot/shutdown requests. The failure branch now requeues the captured events and calls the existing NORETURN pause_and_low_level_reboot(RB_HALT_SYSTEM) rather than continue.
- Extended scripts/check_busybox_init_reap_queue.py to require the mask-restore failure branch to retain the pending events and invoke the fatal halt path, and to require the NORETURN declaration. The focused BusyBox source-contract workflow will exercise it on this commit.
- Corrected an earlier CI attribution error in SESSION_HANDOFF.md and REFACTOR_ROADMAP.md: exact XY run 45108415640 on 9de was still IN_PROGRESS at its last matching job lookup (job 137476531958 was at Build XY-C1); it was not confirmed failed. The curl error 56 log came from another unmatched job and should not be attributed to that run.
- Exact ca609 run 45109162782: detector, new atomic queue-header regression and production-input detection PASS; Build XY-C1 IN_PROGRESS, TRX/artifact PENDING at the last poll. Exact AI docs 45109162785 and Process inventory 45109162805 PASS. No L3 claim for delayed-signal source.
- The queue-header helper remains a defensive retry/atomic-publication improvement. Its regression PASS on the exact ca609 workflow; this change is not represented as a repair for a proven 9de failure.
- Resulting commit: THIS COMMIT.
---

## 2026-11-23 11:53 UTC+8 — Make XY-C1 sys/queue.h fetch retryable and atomic

- Exact XY-C1 run 45108415640 on main SHA 9de5f37d550244590158b302b6ac9935bff765b3 failed before firmware source compilation: the root Makefile sys/queue.h curl returned error 56, "Recv failure: Connection reset by peer", and Makefile exited from prepare-headers. Toolchain download succeeded; this is a transient prerequisite transport failure, not a compiler diagnosis.
- Added scripts/install_musl_queue_header.sh and routed the root Makefile through it. Curl retries all errors (five retries with bounded timeouts); output goes to a unique temporary file in the same directory, empty responses are rejected, mode is normalized to 0644, and a complete file is atomically renamed into place. Failed/truncated transfers cannot replace an existing valid header; -s makes an empty stale file trigger a fresh fetch.
- Added scripts/tests/test_install_musl_queue_header.py. It starts a local HTTP server that returns a truncated first response and a valid second response, verifies retry and exact final content, tests partial/empty failed downloads preserve an existing header and leave no temporary files, and checks Makefile/workflow wiring.
- Added root Makefile, helper and regression paths to XY-C1 push filters and a dedicated test step. The root Makefile change is a firmware production input, so this exact commit should retry Build XY-C1, TRX and artifact upload for the delayed-signal PID1 source.
- Exact 9de checks: BusyBox PID1 45108415639 PASS; AI docs 45108415662 PASS; Process inventory 45108415598 PASS; USB 45108415607 PASS; NVRAM 45108415605 PASS. XY-C1 45108415640 is the sole known failure and failed at header download, before compilation.
- Local installer regression passed with curl 8.10.1 using a real truncated HTTP response and subsequent retry. New exact-head Actions and L3 for the signal accumulator remain pending.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 11:49 UTC+8 — Correct PID1 delayed-signal local declaration scope

- Immediate source review after commit 352c47fef554d64ad8c536d1ed8227a3af261d96 found signal_error declared inside the conditional DEBUG_SEGV_HANDLER block, while the ordinary init signal setup uses it. This would leave the default production build with an undeclared identifier.
- Moved the declaration into the ordinary signal setup block and removed it from the debug-only block. The code is checked for same-block declaration/use before creating this commit.
- The standalone new signal accumulator tests passed under GNU89 with -Wall -Wextra -Werror and ASan/UBSan. This policy test does not compile the full BusyBox init.c; exact-head BusyBox and XY-C1 Actions must verify that translation unit and target build.
- The exact-child wait/reap baseline already has L3 evidence: XY-C1 run 45106983817 on 7c556be passed Build, TRX and artifact upload. The signal accumulator change itself is not yet L2/L3 verified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 11:47 UTC+8 — Preserve concurrent PID1 delayed-signal requests

- Source review confirmed a lost-event race in check_delayed_sigs(): the main loop snapshots the single bb_got_signal value and then clears it, so a different delayed handler can overwrite that global between those operations. The source comment already acknowledged SIGHUP/Ctrl-Alt-Del collision; a shutdown request could also be overwritten by another delayed event.
- Added init_signal_pending.h with independent event bits and a retained shutdown signal; init.c installs PID1-only delayed handlers with a full signal mask, snapshots and clears pending events while the delayed signal set is blocked, processes shutdown before restart/service actions, and requeues a taken batch if restoring the prior mask fails. The general libbb bb_got_signal path is retained for SIGCONT in stop_handler().
- Added GNU89 ASan/UBSan tests for distinct events, coalescing, shutdown mode, snapshot restore and invalid inputs; extended the source-contract script and BusyBox workflow to compile/run them. A local GNU89 -Wall -Wextra -Werror ASan/UBSan compile and execution of the standalone policy test passed.
- The earlier exact-child source change already has L3 evidence: XY-C1 run 45106983817 on 7c556be passed Build XY-C1, TRX verification and artifact upload; compare 1f9d39bc..7c556be contains only workflow, test, script and log paths after the production source.
- Handoff CI is being repaired alongside this source slice: run 45107256439 first found the missing mandatory checkpoint sentence; exact-head run 45107711807 on 056edb68 then found the additional required phrases one atomic main commit and Before context exhaustion. Added the exact rules without changing the validator. Exact Actions for this source/test commit remain pending.
- Resulting commit: THIS COMMIT. New signal-fix L2/L3 and hardware L4 are not yet claimed.

---

## 2026-11-23 11:42 UTC+8 — Restore the explicit end-of-session handoff checkpoint contract

- The exact-head AI handoff documentation job 45107256439 on 8331df0c690aafa00a7d46263dacea8ab8bfbb01 failed at its validator with: current handoff does not require an end-of-session accuracy checkpoint. The log and roadmap were not the cause.
- Root cause: the refreshed SESSION_HANDOFF.md explained the checkpoint process but omitted the exact required sentence, “The checkpoint is mandatory; a document edit is conditional.” Added that statement and the explicit end-of-session live-state comparison rule. The validator is unchanged; this is a genuine handoff contract correction.
- On the same 8331 SHA, BusyBox PID1 regression 45107256450, Process inventory 45107256374, USB hotplug 45107256428 and NVRAM audit 45107256356 all PASS. XY-C1 host/detector run 45107256442 PASS and deliberately skipped Build/TRX/artifact because the commit changed only tests/docs.
- Exact XY-C1 workflow-only run 45106983817 on 7c556be was still in Build XY-C1 at the latest check; TRX and artifact upload remained pending. L3 is not claimed; L4 is not verified.
- This standalone docs-only governance fix repairs the exact-head mandatory documentation contract failure. Product source is unchanged. Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:32 UTC+8 — Run XY-C1 build after repairing firmware input detector tests

- The workflow-trigger change at `11090e80b126fa1665758026d2163fd9e2f09e6a` correctly queued XY-C1 validation, but its detector unit test failed because the first fixture version used double-escaped Git delimiters. The corrected fixture commit `a71253cc463befcab865afb8a90407afa7975e33` passed the full XY-C1 workflow, including the detector tests; Build XY-C1/TRX/artifact steps were skipped because that commit changed only host tests and dev log.
- Trigger the corrected validation again by clarifying the workflow's own comment that BusyBox init is production code and requires XY-C1 rebuilds. The path filter already includes `trunk/user/busybox/busybox-1.24.x/init/**`, and the detector now treats edits to this workflow as build inputs. This next exact-SHA run should therefore pass tests, detect the workflow change, and run the full build/TRX/artifact stages against the latest BusyBox PID1 source.
- Parent main SHA: `a71253cc463befcab865afb8a90407afa7975e33`. BusyBox PID1 regression, USB storage hotplug and process-lifecycle inventory have each passed exact-head host validation on the current source tree; NVRAM audit passed on the previous exact head `84060ba4f52823b396487b24cdfe66254baff0d0`.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:22 UTC+8 — Correct NUL and newline fixtures in XY-C1 workflow-only detector test

- The exact-head XY-C1 workflow trigger correction at parent `11090e80b126fa1665758026d2163fd9e2f09e6a` started run `45106741184`; code review of the new detector regression found its mocked Git output encoded backslash characters rather than actual newline/NUL delimiters. That would have exercised the fail-safe invalid-SHA path instead of proving the targeted workflow-only diff behavior.
- Corrected the test fixture literals so `rev-parse` returns a normal newline-terminated SHA and `git diff -z` returns a NUL-terminated path. Production detector and workflow configuration are unchanged; the current workflow run is still used to validate its full firmware build path, while this successor will assert the unit-test fixture accurately.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:18 UTC+8 — Trigger XY-C1 firmware validation for BusyBox PID1 production changes

- Exact-head BusyBox PID1 regression run `45106412054` on parent `84060ba4f52823b396487b24cdfe66254baff0d0` passed the GNU89 ASan/UBSan test binary and full source-contract checker. The USB storage hotplug suite `45106411983` also passed on the same SHA; NVRAM audit `45106412065` was still in progress at this checkpoint.
- The successful BusyBox fix had not started an XY-C1 build: the firmware workflow trigger list omitted `trunk/user/busybox/busybox-1.24.x/init/**`, even though it already tracked BusyBox UDHCP sources. Added the init subtree to the path filter. Also classified edits to the XY-C1 workflow itself as build-triggering inputs; otherwise workflow edits are discarded by the detector's blanket `.github/**` host-only rule. Added tests that distinguish production `init.c`/policy header from host-only init tests and assert a workflow-only push triggers validation. Kept ordinary documentation-only pushes build-free.
- Parent BusyBox change SHA `1f9d39bc59f9882b9f0d0efae2f09e908110a1ac` is covered by exact-head regression PASS on follow-up `84060ba4f52823b396487b24cdfe66254baff0d0`; this workflow correction is intended to cause the next exact-head commit to run Build XY-C1, TRX verification and artifact upload. The current commit's L3 state remains pending until those steps complete.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:06 UTC+8 — Match PID1 wait regression declarations to the source-contract style

- Exact-head BusyBox PID1 run `45106301790` on `1f9d39bc59f9882b9f0d0efae2f09e908110a1ac` compiled and ran the GNU89 ASan/UBSan test binary successfully, then the Python contract failed because its test-presence assertion expects the repository's existing single-line `static void test_name(void)` declaration form. The five new functions were formatted with the return type and name split across lines.
- Normalized those five test declarations to the existing test-file style. This is a checker-compatibility-only correction; helper and production behavior are unchanged. Re-running all exact-head checks on the successor commit.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 13:02 UTC+8 — Wait for the exact BusyBox PID1 action child

- In BusyBox `init.c`, `waitfor(pid)` used `kill(pid, 0)` followed by unqualified `wait(NULL)`. The PID existence probe is vulnerable to PID reuse between probe and wait; if that reused PID belongs to an unrelated live process, init can block on that process instead of its completed action child. The unqualified wait also consumed arbitrary children while waiting for the target.
- Added `init_wait_for_child()` to target `waitpid(pid, NULL, 0)` specifically, retry EINTR, treat ECHILD as the signal-handler-already-reaped case (the queued PID is drained immediately afterward), and preserve other failures. `waitfor()` now marks the exact reaped child, opportunistically drains only already-exited peers with `waitpid(-1, ..., WNOHANG)`, and never blocks on an unrelated child. Added five GNU89 callback regressions for exact PID/options, EINTR retry, queued ECHILD, error propagation and invalid arguments; source contracts reject the old kill/wait sequence.
- Parent main SHA: `e3d9935d58d255d089392a1c8861779a2b6b1be9`. No local full BusyBox toolchain or target init binary was available; host regression and source contract are pending the exact-head Actions run. This change does not claim router PID1 boot validation or hardware behavior.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:39 UTC+8 — Update the final delete-folder contract for extracted transaction policy

- Exact-head libdisk run `45105379349` on `f7ab8fb2f2ccce29e11687f8b5cd5d8134c499e0` passed both GNU89 ASan/UBSan test binaries, including all five newly-added function-level delete transaction tests, then failed a final legacy assertion that still required `disk_share_permission_batch_apply()`, rollback, and `rmdir(staging_path)` to appear directly inside `del_folder()`.
- The new implementation intentionally delegates these steps to `disk_share_folder_delete_batch()`; the stronger source contract already pins its move → refresh → ACL apply → staged remove → ACL rollback → directory restore → snapshot refresh ordering and all tests. Replaced the obsolete wrapper-level assertion with a check for preflight, staging-path creation, injected callbacks, and safe staging-parent cleanup. No product C or regression behavior changed.
- Resulting commit: THIS COMMIT. Root `dev_log.md` now has 119 dated records (within the 20–119 protocol window); do not append another record without rotating/archive-preserving first.

---

## 2026-11-23 12:34 UTC+8 — Exercise the complete folder deletion transaction with fault injection

- Parent main SHA: `b08863f2dc42c06abf1b9765b8ca42f69025fd7b`. The rename path already had injected callback tests, but `del_folder()` still embedded directory staging, snapshot refresh, ACL publication, staged-directory removal and rollback directly in its public wrapper. That left the delete path's rollback combinations untested at policy level.
- Extracted `disk_share_folder_delete_batch()` with injected move/remove/refresh/file-writer callbacks, then routed `del_folder()` through it. The helper preserves the staging directory when a failed rename-back leaves data there, always attempts to refresh the visible snapshot after compensation, and returns the original operation errno unless compensation is incomplete (then EIO). Added five GNU89 state-machine tests: successful delete; initial snapshot failure; partial ACL write failure; staged-directory removal failure; and failed directory compensation with explicit staged-data retention. Updated source contracts to pin operation/rollback ordering.
- Parent exact-head libdisk run `45105209634` is in progress at this update; earlier `e16fc458` exact-head run `45105008310` passed both sanitizer binaries but failed the first source contract because its normalizer preserved spaces. The next exact-head run will validate the corrected contract and these new tests. No L3/L4 claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:25 UTC+8 — Update mod_folder source contract for no-follow directory checks

- Exact-head libdisk run `45105123378` on `e94bdc2ddb2defee0fe5e5edb3d2d298a9b7f7bb` passed both GNU89 ASan/UBSan regression binaries, then failed the source contract at a second stale assertion: it still expected `test_if_dir(full_path)` in `mod_folder()`. The symlink hardening commit correctly replaced that check with `lstat()` plus `disk_share_folder_entry_status()`.
- Updated the ordering contract to require the no-follow check before the cached folder lookup and stale-snapshot refresh. The earlier fresh-enumeration contract also now matches the checker's space-normalized source representation. This is test-only; no product C code changed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:22 UTC+8 — Correct the lstat source-contract whitespace locator

- Exact-head libdisk run `45105008310` on `e16fc458f434cdb097d27e17bda7daaf9f245dec` passed both GNU89 ASan/UBSan C test binaries, including the new temporary-directory symlink fixture, then failed the Python source contract. The source code itself used `lstat(testdir, &entry_status)`; the checker compared against `lstat(testdir,&entry_status)` even though `compact_all_folders` joins tokens with spaces.
- Corrected the source-contract token to match its normalized representation. No product source or test behavior changed in this checker-only correction; the full exact-head libdisk workflow must pass on the successor SHA before this slice is considered L2-verified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:19 UTC+8 — Reject symlinked directories from the libdisk share inventory

- Audited Samba's private-share builder: it consumes `get_folder_list_in_mount_path()` and constructs `path = mount_point/folder_name`. Both the fresh scanner and persisted folder-list cache previously accepted directory symlinks (the scanner used `opendir()`, and the cache trusted stored names), which could expose a path outside the intended removable-volume subtree. `mod_folder()` also used a symlink-following directory check before rename.
- Added a shared `lstat()` status policy. Fresh scans now accept only real directories; cached names are revalidated and compacted before publication, skipping missing/stale/symlink/non-directory entries while failing closed on other stat errors. `mod_folder()` now applies the same no-follow gate before renaming. The GNU89 regression builds a real directory and symlink and checks real-dir accept, symlink reject, ENOENT skip, and EACCES/unknown-error fail-closed semantics. Source contracts pin all three call paths.
- Parent main SHA `a835089b13114527c9acafe5edab3fddaa046725`. Parent libdisk run `45104677566` on `cdc137f84a2d5c704198184d5b014d18b3ffe937` passed both ASan/UBSan C regression binaries but failed its final checker on a stale direct-rename locator; checker-only fix `a835089b13114527c9acafe5edab3fddaa046725` updated the transaction helper assertion. This code change triggers a fresh exact-head libdisk run. NVRAM audit `45104677474` and XY-C1 `45104677571` on `cdc137f` were still in progress at last poll. No L3/L4 result claimed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:14 UTC+8 — Align libdisk source contract with folder rename transaction helper

- Exact-head libdisk run `45104677566` on parent `cdc137f84a2d5c704198184d5b014d18b3ffe937` passed both GNU89 ASan/UBSan regression binaries, including the new four-case function-level folder-rename state-machine tests. Its final Python source contract failed with `ValueError: substring not found` because an older assertion still searched `mod_folder()` for the direct `rename(full_path,new_full_path)` call after commit `cdc137f84a2d5c704198184d5b014d18b3ffe937` delegated that operation to `disk_share_folder_rename_batch()`.
- Updated the stale ordering assertion to locate the transaction-helper call after stale-cache refresh and reject the removed direct-rename form. This is a checker-only correction; no product C source changed. Parent NVRAM and XY-C1 exact-head runs remain in progress at this update; no L3/L4 claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 12:05 UTC+8 — Restore required handoff section heading after documentation CI failure

- Current parent main SHA: `768fc390acf8ada74c935a9096c67308b15937b0`; backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact-head watchdog run `45104441871` completed PASS after the C whitespace-normalized source dispatch assertions were introduced. All runtime policy sanitizer tests and source wiring checks passed.
- The documentation contract run `45104441878` failed because the prior handoff rewrite renamed a required heading from `## Active task` to `## Active work`. The section content is intact; this commit restores the protocol-required heading without changing the active engineering record.
- XY-C1 run `45104213876`: all host checks PASS, Build XY-C1 still IN_PROGRESS, TRX and artifact upload PENDING. NVRAM audit `45104213886`: all transaction/locking regressions PASS, raw ioctl/kernel caller audit IN_PROGRESS. Process inventory `45104213913` PASS. HTTPD authentication `45104213964` PASS. DNSMasq lifecycle `45104213901` PASS; HTTPD daemon lifecycle `45104213888` PASS.
- No firmware L3 or hardware L4 claim. dev_log count after this entry must remain below 120.

---

## 2026-11-23 11:45 UTC+8 — Fix watchdog signal lifecycle source checker whitespace matching

- Exact-head watchdog run `45104213963` on `5892a4229e9683979bc1e97fca71b2e8528d2413` passed GNU89 ASan/UBSan signal deferral, NTP/service lifecycle, and CPU-watchdog postcondition regressions, then failed the final Python source-wiring step with `ValueError: substring not found`.
- Root cause: the embedded Python assertions searched for literal escaped newline/tab sequences in `watchdog_main()`, although the C source has ordinary formatting whitespace. The same ordering assertions now operate on `"".join(main.split())` and compare all offsets in that normalized view.
- The adjacent libdisk follow-up `1ccfaed9ba04feaa5c1d6802e6708d788e16f953` fixed a live-directory rollback bug in `add_folder()`; exact-head libdisk run `45104177783` passed.
- Parent SHA `5892a4229e9683979bc1e97fca71b2e8528d2413`: XY-C1 run `45104213876` host stages PASS / Build IN_PROGRESS / TRX and artifact upload PENDING; NVRAM audit `45104213886` policy and lock regressions PASS / raw caller audit IN_PROGRESS; HTTPD auth `45104213964` PASS; Process inventory `45104213913` IN_PROGRESS; DNSMasq lifecycle `45104213901`, HTTPD daemon lifecycle `45104213888`, and handoff contract `45104213904` PASS.
- This correction changes only the workflow's source-contract assertion and the handoff/log status, not watchdog runtime code. No L3 or hardware L4 validation claimed.
- dev_log now has ${(log.match(/^## \d{4}-\d{2}-\d{2}/gm)||[]).length+1} dated records; it remains below the 120-record rotation threshold.

---

## 2026-11-23 11:35 UTC+8 — Verify PID1 exact-child waiting against real child reaping

- Parent main SHA: 7c556be9eb1caa343d720bcecd3df74c5461262a. The existing PID1 callback tests exercised injected waitpid results but did not prove the OS behavior that motivated the fix: a blocking wait for one action child must not consume an unrelated exited child.
- Added test_wait_for_exact_child_does_not_reap_peer() to trunk/user/busybox/busybox-1.24.x/init/tests/init_reap_queue.c. It creates a peer child and waits for its exited-but-unreaped state with waitid(WNOWAIT), then starts a delayed target child and calls init_wait_for_child(target_pid, waitpid, ...). The test confirms the peer remains waitable and the target has been reaped. Registered the case in the source contract.
- An isolated GNU89 -Wall -Wextra -Werror ASan/UBSan harness for the real fork/waitid scenario passed 10/10 locally. The full repository binary and source contract await this commit's exact-head BusyBox Actions.
- Exact predecessor evidence: BusyBox regression 45106412054, Process inventory 45106412151, AI docs 45106411973, USB hotplug 45106411983, and NVRAM audit 45106412065 PASS on 84060ba4f52823b396487b24cdfe66254baff0d0. XY workflow unit regression 45106798326 and AI docs 45106798279 PASS on a71253cc; its target build was correctly skipped because that SHA only changed tests/docs.
- The workflow-only XY-C1 run 45106983817 on 7c556be is still building; its terminal result is required before the PID1 L3 state can be claimed. No L4 claim.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 11:08 UTC+8 — Preserve ACLs when failed folder creation leaves a live directory

- Parent main SHA: `256d736d8483227b5df3e26c074394ae4071786d`. In `add_folder()`, failure cleanup called `rmdir(full_path)`; if another writer populated the directory before rollback, removal failed but the old `else if (permission_init_started)` branch still deleted that folder's permission records from all accounts. The directory remained live while its ACL metadata was removed.
- Added `disk_share_folder_rollback_can_remove_permissions()`: ACL-row compensation is allowed only when the directory was removed or already absent (`ENOENT`). If it survives (for example `ENOTEMPTY`, `EBUSY` or `EACCES`), preserve its ACL rows, report incomplete rollback, and refresh the visible folder snapshot. Added policy regressions for success, ENOENT, ENOTEMPTY, EBUSY and EACCES, and pinned guard ordering in the source contract.
- Parent exact-head libdisk run `45103728364` on `7bd561845032ed68084e53c609da8c0a22655a6a`: PASS (GNU89 ASan/UBSan host regressions and source contract). NVRAM audit `45103585419` on product SHA `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e`: PASS. XY-C1 run `45103585439` on that product SHA remains IN_PROGRESS at Build XY-C1; TRX and artifact upload are PENDING. This commit's exact-head Actions are pending. No local compile, L3 build or L4 hardware result is claimed.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 11:20 UTC+8 — Make folder permission changes compensating and staging creation exclusive

- `0e83450aa686d745f80547604bd4f08479a43538` added a reusable per-file permission batch with original-content snapshots, reverse-order rollback and retryable compensation. `del_folder()` now stages its directory and only removes it after all permission updates succeed; failures restore the directory and folder snapshot. Added success, later-write failure and rollback-failure/retry policy regressions.
- `f00ed9b4ba735b1943887ccd95b4dcda75255e88` reused the batch for `mod_folder()`, precomputing old-to-new ACL contents before the directory rename and restoring exact original file images on failure.
- `b5458732188ea2ac1b6ab27ca8f1c73a87e3c06e` removed the staging-path check/use race by reserving a hidden parent through exclusive `mkdir()` semantics before moving the folder. It retries only `EEXIST`; tests cover collision retry, hard creation errors and path truncation.
- Checker-only descendants `44e3a626bebf255b8a08eb212a62c5fb07ebe74b` and `7bd561845032ed68084e53c609da8c0a22655a6a` repaired stale ACL-pruner/source locators. Exact current-head libdisk run `45103728364`: PASS (both GNU89 sanitizer binaries and source contracts).
- Product-source XY-C1 run `45103585439`: host checks PASS; Build XY-C1 IN_PROGRESS; TRX and artifact upload PENDING. NVRAM boundary audit `45103585419`: policy/locking regressions PASS, raw ioctl/kernel caller audit IN_PROGRESS. HTTPD `45103585429` and Process inventory `45103585424`: PASS.
- The root log had 109 dated records before this entry and now has 110, below the 120-record rotation threshold. No shard rotation performed.
- Next: finish exact product-source build/TRX/upload and NVRAM audit status; then add function-level filesystem failure injection for folder delete/rename compensation.

---

## 2026-11-23 09:13 UTC+8 — Make PID 1 recovery-shell respawn throttling independent of pre-exec latency

- Confirmed the old timer in `init_main_loop()` started before `run_shell(0, 1)` and stopped only after the child exited. This measured fork + signal reset + console attach + timezone setup + shell runtime, not shell runtime itself. Slow pre-exec setup could incorrectly make a repeatedly failing shell appear to have lived at least two seconds, removing the throttle and allowing a rapid restart loop.
- Source commit `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c` removes the misleading uptime timestamp/threshold. Every reaped recovery-shell child now gets a one-second throttle; the existing wait predicate still cancels the delay for a pending shutdown. Updated the policy regression, exact source contract, and its file list for the console policy header/test.
- Exact same-SHA PID 1 mdev/security `45093300454` PASS (including Build/run PID 1 console recovery retry regression and source contract); Process lifecycle `45093300665` PASS; AI docs `45093300644` PASS. PID 1 notification queue `45093300520`, DNSMasq `45093300505`, DNS forwarder `45093300535`, HTTPD `45093300612`, process inventory `45093300456` PASS.
- XY-C1 `45093300583` host checks PASS and Build XY-C1 IN_PROGRESS; TRX verification and artifact upload not yet run. NVRAM audit `45093300474` and USB hotplug `45093300506` still in progress at the last poll. Do not claim L3/L4.
- Resulting source commit: `3d0917068e0311d38f6e4a01c5ec0c9c3bab3d0c`.

---

## 2026-11-23 09:04 UTC+8 — Restore the 20–119 record development-log root window

- Parent main SHA `1b4d7138730431ecfc7d317adc4d8f4efa027ddd` had only one dated record in root `dev_log.md`, violating the validator's rolling-window contract. The prior 100-record root snapshot remains at parent `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`; restored it verbatim and added eight subsequent dated records. Root now contains 108 complete records and stays below the 120-record rotation threshold. No archived shard was changed.
- Parent `a4e7170d` MiniUPnP regression `45092211012` PASS and Process lifecycle regression `45092211001` PASS. Parent `1b4d7138` AI docs run `45092323908` failed because root log count was one; re-run exact docs contract on this repaired window.
- XY-C1 `45091972678` remains IN_PROGRESS at Build for product SHA `4ecc2877`; TRX/artifact not yet verified.
- Resulting commit: THIS COMMIT.

---

## 2026-11-23 09:03 UTC+8 — Restore the cross-session execution mandate

- Parent `a4e7170d` AI docs run `45092211047` exposed the missing literal `# Cross-session execution mandate` section. Restored the required continuous-work loop, one atomic main commit per logical slice, non-forced compare-and-swap, no repeated user authorization and a first next action.
- The new AI handoff run `45092323908` passed the cross-session section check and exposed the independent root log retention defect (one entry vs. minimum 20). The log-window repair is in the next record.
- Parent MiniUPnP `45092211012` and Process lifecycle `45092211001` both PASS.
- Resulting commit: `1b4d7138730431ecfc7d317adc4d8f4efa027ddd`.

---

## 2026-11-23 09:01 UTC+8 — Normalize C call whitespace in MiniUPnP and Process contracts

- Parent `70d487002e68cb169b4faae75e9ff902235c2699` still failed the MiniUPnP inline contract because whitespace-joining left a space after the multiline call opening parenthesis; Process failed the equivalent check in `check_process_lifecycle_integration.py`.
- Removed all whitespace before comparing the LAN address/netmask/MAC checked-copy calls and their argument order. Exact-head MiniUPnP `45092211012` PASS and Process lifecycle `45092211001` PASS on `a4e7170d826cf6b77cea4393122dcdfa1d7efdd9`.
- No product source changed in this checker-only commit; the earlier failures were source-contract string matching, not GNU89/ASan regression failures.
- Resulting commit: `a4e7170d826cf6b77cea4393122dcdfa1d7efdd9`.

---

## 2026-11-23 09:00 UTC+8 — Align MiniUPnP source contract with checked LAN snapshots

- Parent product SHA `4ecc28778b3605743262514dbce53b7ffbebda2d` changed `start_upnp_locked()` to use checked owned LAN snapshots. The inline MiniUPnP workflow still searched for removed direct `nvram_safe_get()` assignments, producing `ValueError: substring not found`.
- Replaced the stale source-order assertions with checked-get/copy helper checks, ordered address/netmask/MAC snapshots, validation, subnet derivation and config publication ordering, plus explicit rejection of unsafe raw getters. A first run then exposed a remaining whitespace mismatch, corrected by the following `a4e7170d` commit.
- Resulting commit: `70d487002e68cb169b4faae75e9ff902235c2699`.

---

## 2026-11-23 09:00 UTC+8 — Restore the mandatory handoff accuracy checkpoint

- Parent `4ecc28778b3605743262514dbce53b7ffbebda2d` required an accurate end-of-session checkpoint; added the mandatory heading and exact conditional-edit rule to `SESSION_HANDOFF.md`.
- Exact AI docs run `45092102706` then failed at the next required literal section, `# Cross-session execution mandate`; the next docs correction restores it.
- Resulting commit: `030c39e578e83ee690f156f6549c3cf77618c791`.

---

## 2026-11-23 08:59 UTC+8 — Snapshot checked LAN identity before MiniUPnP config publication

- Commit `4ecc28778b3605743262514dbce53b7ffbebda2d` replaces rotating-cache pointers for `lan_ipaddr` and `lan_netmask`, plus an unchecked `lan_hwaddr` read, with checked owned copies before IP validation, subnet derivation and config publication. Any read/close/overflow error aborts startup before publishing configuration.
- Added WAN NVRAM policy regression cases for cache-slot overwrite and failure at each of the three LAN identity reads. Exact Samba, WAN, DNSMasq, HTTPD and NVRAM host checks on this source SHA passed; initial MiniUPnP/Process failures were stale/whitespace-sensitive checker contracts, repaired later. XY-C1 remained in progress at the last poll.
- Resulting source commit: `4ecc28778b3605743262514dbce53b7ffbebda2d`.

---

## 2026-11-23 08:56 UTC+8 — Watch MTK ESW kernel inputs in the XY-C1 workflow

- Commit `7dd05bffdb09a6aecb68480131303d00e857bee2` adds the MTK ESW ioctl driver, policy header and test paths to the XY-C1 workflow push filter and source contract. Switch CLI host regression `45091738973` PASS; XY-C1 `45091739058` completed with workflow SUCCESS but Build/TRX/artifact steps SKIPPED because this commit only changed workflow/docs. Same-SHA kernel Build/TRX/artifact remains NOT VERIFIED.
- Exact AI handoff `45091738990` exposed missing mandatory handoff checkpoint wording; restored in `030c39e5`.
- Resulting commit: `7dd05bffdb09a6aecb68480131303d00e857bee2`.

---

## 2026-11-23 08:55 UTC+8 — Preserve checked Samba workgroup and WINS values

- Commit `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee` changes `write_smb_conf_header()` to copy `st_samba_workgroup`, hostname and conditional `dhcp_wins_x` into owned buffers through checked NVRAM reads. This prevents later NVRAM lookups from overwriting cache-backed string pointers and aborts staged smb.conf publication on read/close/length failures.
- Added regression coverage for sequential workgroup/WINS reads and preservation on WINS read failure. Exact Samba host regression `45091634196` PASS; Process umbrella `45091634176` initially failed only on a multiline whitespace-sensitive Samba source assertion, fixed in `b803d7a5`.
- Resulting source commit: `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`.

---

