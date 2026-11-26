# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development on `main` only.
- Parent `main` for the VPN WAN rollback source-order checker coordinate fix: `1a0ff27d90b1d6cb396fcc45d979bbde0ebeab50`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify/delete. PR #1 remains closed/unmerged.
- Bootstrap/protocol recovery was completed earlier; do not repeat it.

## Active task

- Completed parser-safety slices: the DHCPv6 outer parser rejects duplicate CLIENTID, SERVERID, RELAY_MSG and INTERFACE_ID vbuf options before allocation; malformed-option cleanup frees the first owned buffers. Reconfigure Accept (option 20) is rejected when its payload is nonzero. Policy tests and source contracts cover both changes.
- The graceful DHCPv6 shutdown now waits for both IA cleanup retries and outstanding `DHCP6S_RELEASE` transactions.
- Exact SHA `cfa384c7fe1ba6fdd87ca478a920012880d835c6`: DHCPv6 IA #45799187926 PASS and AI handoff #45799187883 PASS for the bounded NoBinding watchdog. Follow-up parent `cd3d1a6ec469937f3488d073bf11c445c0aa4414` (initial Request construction-failure retry) passed DHCPv6 IA #45799579046, AI handoff #45799579038, WAN DHCP callback #45799579064, process inventory #45799579239, NVRAM #45799579136 and USB #45799579137. XY-C1 #45799579141 remained in progress; no L3/L4 claim. Exact-SHA DHCPv6 IA run #45791657631 on `fb01f7a` PASSed all six GNU89/ASan/UBSan policy binaries and the full source checker after normalizing the wrapped two-argument exit gate. This prevents shutdown from discarding Release exchanges deliberately preserved by `free_resources(0)`.
- Current P1 parser slice: `D6_OPT_NTP_SERVER` (option 56) was incorrectly grouped with flat arrays of IPv6 addresses, although RFC 5908 defines it as a container of 4-byte suboption headers. The parser now validates each nested header/payload, handles address/FQDN forms, requires exactly one suboption, counts unknown future registry values as that source while skipping their data, and keeps RFC 4075 `D6_OPT_SNTP_SERVERS` as the separate flat-address option. Tests cover truncated headers, length overruns, payload lengths, duplicate source suboptions, and the RFC 5908 requirement that address suboption 1 be unicast while suboption 2 be multicast. Production/test change is commit `74a0dbcc`; the six C policy binaries passed. Checker fixes: stale test-output banner corrected in `5ed671f`; bounded option-case assertion corrected in `c713dbc`. The exact-SHA C regression and complete Python source contract passed in DHCPv6 IA run #45792959729 on `c713dbc`.
- Confirmed P1 SIGHUP reload lifetime hazard: `free_resources(0)` may leave an IA and retry timer alive after downstream-prefix deletion fails, but the old signal path immediately called `client6_startall(1)` → `read_config6()` → `clear_ifconf(0)` → `clear_iaconf()`, freeing objects still referenced by the retry callback. Reload is now deferred until IA queues and RELEASE events drain; new client starts are blocked, SIGTERM cancels pending restart without duplicate Release events, and `read_config6()` rejects unsafe reload with `EBUSY` before destructive cleanup.
- Added GNU89 regression cases and Python source-contract assertions for IA/event drain, config lifetime guard, signal order and the missing config-header PR filter.
- Confirmed P1 NVRAM flash lost-update race: `nvram_commit()` captured a complete image under `nvram_lock` before acquiring `nvram_sem`; an older commit could wait behind a newer one and then overwrite flash with its stale snapshot. This slice moves `nvram_sem` ahead of snapshot generation and pins lock/snapshot/read/write order in the source checker.
- Added a deterministic pthread regression showing the stale-snapshot interleaving and the corrected serialized result.
- Fixed the NoBinding retry-watchdog gap: Request has finite MRC=10, so IAS_REQUEST now carries a four-minute watchdog beyond its retransmission window; duplicate NoBinding is ignored while that Request is active, and the watchdog is deferred while a replayed multi-IA Reply is still being locally applied. Follow-up review found that initial Request construction failure could still return before the watchdog was armed. The correction now switches to IAS_REQUEST before preparation, reuses/creates the IA timer, and schedules a 5-second setup retry on failure or the four-minute watchdog on success.
- Found a DHCPv6 IA recovery hole: `ia_timo()` deleted the IA timer at T2 before allocating the Rebind event payload/auth state. A recoverable allocation failure discarded the event and left no retry owner. The fix retains the T2 timer until Renew setup succeeds, retries failed Rebind setup after 5 seconds, removes the timer only after a Rebind event is ready, and cancels any timer on invalid IA state.
- Rotated to the VPN shared-transport lifecycle and found a marker ownership hole: `launch_wan_pppd_locked()` sets `l2tp_wan_t=1` before calling `safe_start_xl2tpd_locked()`, but its common launch-failure path invalidates callback generation and returns without clearing the pre-published marker. Later VPN server stop/restart may therefore treat an unstarted WAN transport as active and make incorrect shared-daemon stop/restore decisions. The fix tracks successful marker publication and clears it on launch failure while preserving the original launch error.
- Adjacent WAN6 fixes already on main: empty caller-interface teardown guard; SIT owner-record IFNAMSIZ bound; strict SIT gateway literal validation; strict DHCP 6rd relay literal validation; static and DHCPv6 DNS literal validation; first-error aggregation during delegated-prefix cleanup.

## 2026-11-26 implementation continuation — DHCPv6 cleanup shutdown

- New confirmed P1 defect fixed in the next commit (parent `f55521e7bc74691d18d4a6f62a6f9e326730b970`): on SIGTERM/SIGQUIT, `dhcp6c` previously called forced `check_exit(1)`; `free_resources(1)` discarded the 5-second retry timers that `remove_iaprefix()` schedules after a delegated downstream-address deletion fails. The WAN caller also used a 3-second stop with SIGKILL escalation, so a transient kernel delete error could strand a delegated prefix without a retry owner.
- The client now enters a shutdown-drain state, runs `free_resources(0)`, keeps the event loop alive until every configured IA queue is empty, checks readiness after timer callbacks and received packets, and suppresses new Solicit/Information-Request events (including the `client6_start()` triggered after successful cleanup). SIGHUP cannot restart a client already shutting down.
- Normal and failed-start stop paths use a 16-second non-forced wait. Persistent cleanup errors therefore cause a fail-closed timeout instead of SIGKILL; normal stop does not advance callback generation on timeout.
- Added policy cases and Python source assertions for queue drain, shutdown/restart ordering and non-forced stop; DHCPv6 IA workflow filters include `dhcp6c_config.h`. Exact-SHA run #45789833785 compiled and passed all six GNU89/ASan/UBSan C regressions; its Python checker first exposed a checker-only variable-order issue, corrected in #8d2bf19. The same `a0fa423` WAN DHCP workflow then exposed two inline source-contract assertions still expecting the former 3-second forced stop. This commit aligns those assertions with the intentional 16-second non-forced behavior; rerun exact-SHA WAN/IA checks before claiming PASS. The #8d2bf19 rerun then caught one remaining legacy `3, 1` expectation in the IA checker’s existing callback-generation fencing assertion; this commit updates that final assertion to `16, 0`.
- Exact parent `9ac2adf871a85e91a2a05518e47aba18c99df9be`: DHCPv6 IA #45788196340 PASS; WAN DHCP #45788196312 PASS; AI #45788196513 PASS; process inventory #45788196355 PASS; XY-C1 #45788196523 was still building; NVRAM #45788196283 and USB storage #45788196181 were cancelled.
- Intermediate parent `f55521e7bc74691d18d4a6f62a6f9e326730b970`: DHCPv6 IA #45788409998 PASS; DHCPv6 script retry #45788410243 PASS; HTTPD #45788410121 PASS; AI #45788410109 PASS; Process #45788410216 PASS; NVRAM #45788410050 PASS; USB storage #45788410075 PASS. XY-C1 #45788410074 is still in progress with Build XY-C1 running; TRX and artifact steps are pending.
- These results are parent-SHA facts only. Hardware remains untested; exact-SHA CI must validate the shutdown commit.

## 2026-11-26 validation update — shutdown drain source contract PASS

- Exact SHA `78205c10fb71cfb962917e65261fa499c0548573`: DHCPv6 IA workflow #45790398561 PASS. The GNU89/`-Werror`/ASan/UBSan compile-and-run step and the complete `scripts/check_dhcp6c_ia_publication.py` source contract both passed. AI handoff workflow #45790398455 PASS.
- Exact SHA `48bec7b6ac28ea93394467d6814023884eadfa1c`: WAN DHCP callback workflow #45790261953 PASS after inline stop-order checks were updated for the 16-second non-forced shutdown contract.
- Product firmware source SHA `a0fa423d528aee7d36bf06f79b0a28dc1c736fa5`: XY-C1 run #45789833788 remains in progress at `Build XY-C1`; TRX verification and artifact upload are pending. Later commits adjusted only checker/workflow/handoff material, not product firmware source. L3 is not yet verified.
- Hardware L4 remains NOT VERIFIED.

## 2026-11-26 validation update — RFC 5908 NTP parser checker correction

- Exact SHA `74a0dbccda2028b87f4fe7cd57dd0513608f8f39`: DHCPv6 IA run #45792454948 FAILED only in the Python source checker. All six GNU89/`-Werror`/ASan/UBSan C policy binaries passed, including the new NTP bounds/suboption policy test. The checker expected the previous output string `DHCPv6 option and UDP datagram bounds policy: PASS`; the test correctly prints `DHCPv6 option bounds and NTP suboption policy: PASS`. The expected string is updated in this commit.
- Same SHA AI handoff #45792454964 PASS, DHCPv6 script retry #45792455152 PASS, Process inventory #45792454913 PASS. HTTPD #45792454943, NVRAM #45792454938 and USB #45792455009 were still running at last poll.
- XY-C1 #45792454987 on exact product-source SHA `74a0dbcc` was still in progress; Build/TRX/artifact conclusions were not yet available. L3 is not verified; hardware L4 NOT VERIFIED.

## 2026-11-26 validation update — NTP parser source checker PASS

- Exact SHA `c713dbc59c566254f736d21ff1e8b06de6f2ab43`: DHCPv6 IA #45792959729 PASS. All six GNU89/`-Werror`/ASan/UBSan policy binaries and the complete Python parser/source checker passed after the checker was scoped to the full NTP option case and normalized whitespace. AI handoff #45792959822 PASS.
- The NTP decoder from `74a0dbcc` is therefore regression-verified on that source baseline. This follow-up adds RFC 5908 unicast/multicast address-class checks and must receive its own exact-SHA Actions result.
- XY-C1 #45792454987 on product-source SHA `74a0dbcc` was still in progress at the last poll; Build/TRX/artifact result not verified. Hardware L4 NOT VERIFIED.

## 2026-11-26 validation update — NTP address-class hardening PASS

- Exact SHA `734e9a86960d5f9158904a3e73280fbbae3b3842`: DHCPv6 IA #45793204920 PASS (all six GNU89/`-Werror`/ASan/UBSan policy binaries and full source checker), DHCPv6 script retry #45793204973 PASS, AI #45793204728 PASS, HTTPD #45793204829 PASS, Process #45793204808 PASS, USB #45793204756 PASS, NVRAM #45793204909 PASS.
- XY-C1 #45793204934 remained in progress at the last poll; same-SHA Build/TRX/artifact result not yet available. L3 not verified; hardware L4 NOT VERIFIED.

## 2026-11-26 — DHCPv6 SIGHUP reload lifetime fencing

- Confirmed P1 lifetime defect: SIGHUP previously called `free_resources(0)` and immediately reloaded configuration. If delegated-prefix cleanup failed, `remove_ia()` retained the IA and retry timer in `iac->iadata`, but `read_config6()` then called `clear_ifconf(0)`/`clear_iaconf()` and freed IA/config/PIF objects still referenced by the timer callback.
- SIGHUP now records `restart_pending`, suppresses new client exchanges, and waits for both old IA cleanup and outstanding `DHCP6S_RELEASE` transactions to drain before reloading. SIGTERM cancels a pending restart without queuing duplicate Release events. `read_config6()` independently rejects unsafe reload with `EBUSY` before cleanup if any IA remains.
- Added GNU89 policy coverage and Python source-contract checks; corrected the DHCPv6 IA pull-request path filter to include the changed `dhcp6c_config.h`. Local compile/test not run without checkout; exact-SHA Actions are the validation gate.
- Parent main: `6dbcf822bed1213baabef7639aa3f1ae68288f3f`; resulting commit: THIS COMMIT.

## 2026-11-26 — Serialize NVRAM snapshot generation with flash commits

- Found a persistence lost-update race in `nvram_commit()`: it generated `bufw` under `nvram_lock` before taking `nvram_sem`. Concurrent commits could snapshot revisions A and B, write B first, and then let older A overwrite the newer flash image.
- Move `mutex_lock(&nvram_sem)` before snapshot generation and retain it through MTD compare/write. Generation failure now releases the mutex before buffer cleanup.
- Added a deterministic pthread regression modeling both the broken lock-after-snapshot interleaving and the fixed lock-before-snapshot ordering, plus source-contract assertions for mutex/snapshot/read/write/unlock order.
- Parent main: `d5ef589529bf19971f996265cffb92b9906c5635`; resulting commit: THIS COMMIT.

## 2026-11-26 — Tighten NVRAM commit-order checker

- Follow-up static review of the new commit-order assertion found its regex was double-escaped and would not match the actual mutex-unlock tokens. Replaced the regex with a literal-token scan so the checker counts both error-path and normal-path unlocks exactly.
- This is a checker-only correction; the production lock-order fix and pthread interleaving regression remain unchanged.
- Parent main: `02c27d988c9c3cdb7be180bb468da6a465633990`; resulting commit: THIS COMMIT.

## 2026-11-26 implementation continuation — DHCPv6 timer callback iterator safety

- Confirmed P1 use-after-free: `dhcp6_timer_check()` captured `tm_next` before running an expiry callback. A successful delegated-prefix retry can call `remove_ia()` → `pref_cleanup()` and delete timers belonging to sibling prefixes, invalidating that saved pointer.
- Timer traversal now stamps visited timers with a per-pass generation and records timer-list mutations. It restarts from the current list head after any callback that frees a timer or changes the list, defers newly inserted timers until the next sweep, handles generation wrap, and recomputes the expiry sentinel after callbacks.
- Added a standalone GNU89/ASan/UBSan regression that actually frees the saved-next node during callback, plus a semantic source checker and both push/pull_request workflow path filters. Local host regression compile/run PASS; exact-SHA GitHub Actions pending at commit creation. No L3/L4 claim.
- Exact SHA `a856ba60a4695276b5c484f7e8388fdaeaf1ced0`: DHCPv6 IA run #45795811813 FAILED only when launching the new regression compile command; the six existing C policy binaries had passed. The YAML used doubled line-ending backslashes, so the linker received a literal backslash argument. This follow-up uses one shell command line and requires exact-SHA rerun. On the same SHA, AI #45795811557, DHCPv6 script retry #45795811708, HTTPD #45795811471 and process inventory #45795811538 PASS; NVRAM #45795811645, USB #45795811700 and XY-C1 #45795811655 were still running at last poll.
- Exact SHA `c241c6a150f2c0bf8b6f2a89177780cb9766ad9f`: DHCPv6 IA run #45796127157 PASS, including the new timer traversal ASan/UBSan regression and complete Python source contract; AI handoff #45796127128 PASS. The workflow-only fix does not change product-source inputs. On source SHA `a856ba60a4695276b5c484f7e8388fdaeaf1ced0`, USB #45795811700 PASS; HTTPD #45795811471, DHCPv6 script retry #45795811708 and process inventory #45795811538 PASS. NVRAM #45795811645 is still running. XY-C1 #45795811655 is still at `Build XY-C1`; Verify TRX and artifact upload remain pending, so L3 is not verified.
- Parent main: `5b03226e811dbba4cba5d9219c43086b7962d6e7`; resulting commit: THIS COMMIT.

---

## 2026-11-26 — RFC 5908 FQDN suboption validation

- Parent `main`: `26842e306af329188b8aefd999cbbd9d94024ecd`.
- The NTP FQDN suboption carries exactly one FQDN, but the existing generic `dhcp6_get_domain()` decoder accepts a domain-name list. Validate the wire data before decoding: exactly one terminal root label, no compression/extended labels, ASCII LDH labels only, no IDNA A-labels, and RFC label/name length bounds.
- Added policy regression for valid single/multi-label names, concatenated names, compression, missing terminator, non-ASCII and IDNA labels, hyphen placement, label/name overflow and NULL input. Python source checker pins the production call.
- Exact SHA `8a484891c96894a5452710101a05145fab0fc7e6`: DHCPv6 IA #45796494343, NVRAM #45796494293 and XY-C1 #45796494552 PASS; XY-C1 completed Build, Verify TRX output and artifact upload on that SHA only. DHCPv6 script retry #45796494093, HTTPD #45796494090, USB #45796494100 and Process inventory #45796494041 also PASS. AI handoff #45796494225 failed because root `dev_log.md` reached 120 records; lossless rotation is recorded in the next log item. These earlier results do not validate later DHCPv6 source changes. Hardware L4 remains NOT VERIFIED.

## Verification boundary

- Exact SHA `d5ef589529bf19971f996265cffb92b9906c5635`: DHCPv6 IA #45794325763 PASS; AI handoff #45794325767 PASS; NVRAM transaction boundary #45794325755 PASS; USB/storage hotplug #45794325967 PASS; Process inventory #45794325764 PASS; DHCPv6 Reply-script retry #45794326034 and WAN DHCP callback #45794325993 PASS. XY-C1 #45794325773 had passed all pre-build checks; Build XY-C1, Verify TRX output and artifact upload remained pending at last poll.


- Parent exact SHA `6dbcf822bed1213baabef7639aa3f1ae68288f3f`: all DHCPv6/CI runs were in progress at handoff refresh; no result carried over from prior SHA.


- Parent `78205c10fb71cfb962917e65261fa499c0548573`: DHCPv6 IA #45790398561 PASS, including the GNU89/`-Werror`/ASan/UBSan C policy binaries and complete Python source contract; AI handoff #45790398455 PASS.
- Parent `48bec7b6ac28ea93394467d6814023884eadfa1c`: WAN DHCP callback #45790261953 PASS with the corrected 16-second non-forced stop source contract.
- Handoff-only parent `d739b7f409e57e9b7bbdfa85b7abfacbd8e5804d`: AI handoff #45790741509 was in progress at the last poll. The Reconfigure Accept parser edge added in this commit requires exact-head DHCPv6 IA revalidation.
- Latest fully verified product-source L3 baseline remains `89b80f1505508bb8ae96466cfe5d0c7587a6be67`: same-SHA Build XY-C1, TRX verification and artifact upload all PASS (artifact #15101485510, 12,823,157 bytes). Later production source changes are not yet shown to pass all three on one SHA. Physical router L4 NOT VERIFIED.
- No local repository checkout/full firmware build is available because the shell environment cannot resolve GitHub. Do not claim local full-suite or hardware verification.

## Exact first action for the next GPT

1. Exact SHA `1a0ff27d90b1d6cb396fcc45d979bbde0ebeab50`: AI handoff #45800802088 PASS; VPN lifecycle #45800802057 FAILed only in the Python source contract after all 14 GNU89 policy regressions passed. Root cause: checker offset comparisons mixed raw and whitespace-compacted source indices. THIS commit normalizes every compared position and errno slice; run exact-head VPN lifecycle and AI handoff Actions.
2. The older source SHA `3c53eb5a5fd339d77bc650fc2fefc7d8b60e81b4` has Process #45800460310, WAN route #45800460260, WAN DHCP callback #45800460321, HTTPD #45800460223, process inventory #45800460215 and NVRAM #45800460212 PASS. XY-C1 #45800460210 was still in progress at last poll; it is an older product-source SHA and cannot validate later source changes. Require Build XY-C1, Verify TRX and artifact upload all PASS on one exact current product-source SHA before L3. Hardware L4 remains NOT VERIFIED.
3. After the corrected exact-head VPN contract passes, continue auditing shared L2TP/PPP rollback/error paths, then rotate to another userland stability/security issue.

## Mandatory handoff checkpoint

**Mandatory handoff checkpoint:** Every session MUST compare the live repository and exact-head Actions with this file before ending. Verify source baseline, active task/progress, test/CI boundary, blockers and exact next action. Do not repeat completed recovery or audits. The checkpoint is mandatory; a document edit is conditional. If facts remain accurate, do not create a no-op commit.

# Cross-session execution mandate

Continue as the same maintainer across conversations. Every reviewable source/test milestone must be one atomic main commit: create all changed blobs, a complete Git tree and one commit, then update `main` non-forced with the expected parent SHA. Never publish partial-file commits or force-push.
Before context exhaustion, complete a safe bounded slice, inspect exact-SHA Actions and leave this handoff with the precise next task and validation boundary.

## Execution invariants

- Main only; full tree/atomic commit; expected-SHA non-forced update; never force-push.
- Preserve the frozen backup and do not reopen PR #1.
- PENDING never blocks independent safe work.
- Product L3 requires same-product-source-SHA Build XY-C1, TRX verification and artifact upload all PASS. Physical L4 requires actual device testing.
- Update dev_log for completed engineering milestones, handoff for material progress/validation changes, and roadmap only for material module/status changes.
