## 2026-11-18 20:30 CST — Align lifecycle checker with status-preserving rollback regression

- Parent main `2f8fb1d05a505afb5e9bde72f3d5b2b4a2ac6769` Actions: AI handoff `44305279681` PASS; Shadowsocks `44305279761` passed syntax, startup status, process lifecycle and Adblock tests, then failed in Python checker line 211 because it still searched for the superseded assertion label `dnsmasq restart failure triggers rollback`.
- The active regression expects status 19 with label `dnsmasq restart failure preserves original status`, proving the transaction returns the original failed-start status after rollback. Update only the static checker string; runtime assertions already passed.
- Changed: `scripts/check_shadowsocks_lifecycle.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- Validation: exact-head Action log identifies the only failing source-contract assertion after runtime and Adblock. Rerun required. Checker/docs only; no XY-C1 source changes.
- Next: exact-directive DNSMasq cleanup in Shadowsocks `oversea`/`ssp_close()` and `ss-rules`, with unrelated configuration preservation tests.

---

## 2026-11-18 20:25 CST — Remove duplicated Shadowsocks lifecycle test tail

- Parent main SHA `77ba640f2f8af88f8cb2856e5af62dd770ea765a`: AI handoff `44304776554`, Process fail-closed `44304776568`, and Process inventory `44304778927` PASS. Shadowsocks `44304776719` passed shell syntax and startup status; the bounded process test ran through the first complete PASS sentinel, then attempted to execute `$dnsmasq_config` and failed with Permission denied. A second old rollback assertion (expected 1) and duplicate run_bin/watchcat/DNS proxy tests followed the first sentinel.
- The diagnostic line added on parent 77ba was inside the duplicate segment, so the failure does not prove a production ChinaDNS publication defect. Preserve the complete first suite and correct status-19 rollback test; truncate only the duplicate post-sentinel tail and leave one terminal PASS line.
- Changed: `trunk/user/shadowsocks/tests/process_lifecycle.sh`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- Validation: exact-head job log identifies the duplicate-tail error. New exact-head Shadowsocks rerun is required; parent AI, Process and inventory Actions PASS. No new XY-C1 expected for this test/docs-only commit; product source 6910911 previously passed Build/TRX/artifact. Target BusyBox/L4 NOT VERIFIED.
- Next: implement exact-directive DNSMasq cleanup across Shadowsocks `oversea`/`ssp_close()` and `ss-rules`, with unrelated-content preservation tests.

---

## 2026-11-18 20:15 CST — Diagnose missing ChinaDNS upstream after config publication

- Parent `1e75f56d758468e8cacd088af52920c261e9b48a`: exact-head AI handoff `44303526325`, Process fail-closed `44303526428`, and Process inventory `44303526333` PASS. Shadowsocks `44303526422` passes syntax, startup and the now-correct status-19 rollback case, then fails at the successful transaction assertion because `server=127.0.0.1#65353` is absent from the final fixture config.
- Add failure-only escaped-file diagnostics (`sed -n l`) around the exact-line assertion to capture the post-transaction config. This is necessary evidence before changing production publication logic; no speculative change to the expected directive is made.
- The Python lifecycle checker `.indexOf()` defects and original error-status assertion are already corrected on parent `1e75f56`; its source-contract stage has not yet executed because the upstream assertion still stops the workflow. No local shell execution or new firmware-source change is claimed. Target runtime and L4 remain NOT VERIFIED.
- Next: inspect the escaped config from the diagnostic run, fix the actual reason the ChinaDNS server line is missing, then resume exact-match DNSMasq include cleanup.

---

## 2026-11-18 20:10 CST — Preserve DNSMasq restart errors and repair Shadowsocks source checker

- Parent main SHA: `dde3301aca4056774cd31696fdeb9b98bb0a3773`. Exact-head Shadowsocks workflow `44301647472` passed shell syntax and startup-status regression, then process lifecycle failed because the rollback daemon fixture intentionally exits with status 19 on the first replacement launch while the test incorrectly expected 1. The production transaction returns the original status unchanged; update the test to expect 19 so it verifies first-error preservation.
- The same failure prevented the static checker from running. Review found four Python `str.indexOf()` calls introduced with the DNSMasq RC source contract; change to `str.index()` so the checker can execute under Python.
- Exact DDE Actions: AI handoff `44301647352` PASS; Process fail-closed `44301647315` and Process inventory `44301647284` PASS. Corrective exact-head Shadowsocks and AI checks must be inspected after commit. This is test/checker/docs-only; no new XY-C1 run is expected from firmware-source filters. No local shell run, target BusyBox runtime or L4 router test is claimed.
- Next: exact-match DNSMasq include cleanup in Shadowsocks `oversea`, `ssp_close()` and `ss-rules`, preserving comments, `conf-file`, domain `address`/`ipset` directives and unrelated paths that only contain `cdn`, `gfwlist`, or `dnsmasq.dom`.

---

## 2026-11-18 — Repair exact-head Shadowsocks shell syntax and restore handoff checkpoint contract

- Exact main source SHA `6910911ffdd65a7a04eeb97fa1088a1b8a74d886` CI investigation: Shadowsocks workflow `44299349261` failed in `sh -n` before runtime tests because the ChinaDNS grep assertion had an unterminated quote at EOF; repair the full grep expression and retain the successful-RC-restart call-count assertion. AI handoff workflow `44299349087` failed because the current handoff omitted the exact mandatory sentence `The checkpoint is mandatory; a document edit is conditional.`; restore it.
- Process inventory fail-closed run `44299349041` and umbrella `44299348949` passed on `6910911`; XY-C1 run `44299349040` was still pending at investigation time.
- The repaired shell syntax and documentation contract require exact-head reruns. Local shell execution was unavailable; CI is the runtime authority.
- Next: resolve exact-head Actions, then continue exact-match DNSMasq include cleanup as a separate bounded source/test change.

---

## 2026-11-18 19:50 CST — Route Shadowsocks DNSMasq restart through the RC lifecycle lock

- Parent main SHA: `e71d9e336f4c1ab03725fcbf5fb0da670098ec5a`. The preceding exact-head parent runs `44294135620` (Shadowsocks), `44294135638` (Process fail-closed), `44294135667` (Process inventory), `44294135726` (AI handoff) and `44294135701` (XY-C1) all PASS; the XY-C1 job completed Build, TRX verification and artifact upload.
- Confirmed the Shadowsocks DNSMasq publisher directly stopped `dnsmasq` via the shared name-based helper and launched `/user/sbin/dnsmasq`, bypassing the RC-wide `/tmp/.dnsmasq.lifecycle.lock`, NVRAM service policy and the RC's fail-closed stop/start postconditions. Remove that independent launcher. After staging/validating and atomically publishing the storage include, use `ss_restart_dhcpd()` -> `/sbin/restart_dhcpd`, which acquires the shared RC lock and regenerates/starts the authoritative service. If the service restart fails, atomically restore the old include, retry the RC service against that restored config, and return the original error.
- Extend `trunk/user/shadowsocks/tests/process_lifecycle.sh` with an injected RC restart model and explicit call-count checks for success, validation rejection (no restart), stop failure (no launch, previous file restored) and candidate-start failure followed by rollback restart. Update `scripts/check_shadowsocks_lifecycle.py` to reject direct dnsmasq kill/launch and require the serialized RC path.
- Changed: `trunk/user/shadowsocks/scripts/shadowsocks.sh`, `trunk/user/shadowsocks/tests/process_lifecycle.sh`, `scripts/check_shadowsocks_lifecycle.py`, `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`.
- Validation: local host shell execution NOT RUN (no repository checkout/network in this session); static edit sanity checks PASS. THIS COMMIT: exact-head Shadowsocks/AI/XY-C1 Actions PENDING after push; Process workflows only report when they actually trigger. Parent L3 is PASS; this source commit L3 and physical-router L4 are NOT VERIFIED.
- Next: verify the exact SHA; then replace broad substring DNSMasq include cleanup in oversea/ssp_close/ss-rules with exact directive matching and injected preservation tests.

---

## 2026-11-18 — Propagate DNSMasq restart failure from Shadowsocks shutdown

- Exact b118b841d5b292285c296288fd01229fcf63f28a Shadowsocks lifecycle Action 44292658234 PASS (shell syntax, startup status, process lifecycle including DNSMasq stop-timeout/rollback, Adblock and static ROMFS/source checks); AI handoff Action 44292658587 PASS. Parent source commit cb6d80f XY-C1 run 44291015307 completed successfully: Build XY-C1, TRX verification, artifact upload and the cross-build job all SUCCESS.
- Found a concrete lifecycle reporting defect in ssp_close(): it unconditionally returned 0 after /sbin/restart_dhcpd and /usr/bin/detect.sh, so stop/restart/reserver could report success or proceed to a new startup while DNSMasq had not restarted. Preserve the first encountered firewall/config/service/reconciliation error, log failures, continue independent cleanup, and return the status to callers. Keep the existing early gate: if proxy process stop is incomplete, do not touch firewall/config/cgroups.
- Added host regressions for healthy close, DNSMasq restart error propagation, first-error retention and aborting cleanup after proxy stop failure. Static source contract checks the new return path.
- This commit changes ROMFS shell source and needs a fresh exact-head Shadowsocks/Process/AI/XY-C1 run. No target BusyBox runtime or physical-router L4 is claimed.

---

## 2026-11-18 — Fix stale ChinaDNS normalized-argument contract

- Exact 8e2121694004a1fdd79150a6e8107f845f2eea80 Actions: Shadowsocks runtime syntax, required-startup, bounded-process and Adblock regressions all PASS; Process lifecycle inventory 44291689977 PASS; umbrella Process inventory 44291689958 PASS; AI handoff 44291690049 PASS.
- The Shadowsocks workflow failed only in scripts/check_shadowsocks_lifecycle.py: a final legacy assertion still expected the old raw NVRAM variable "$ss_chdns", while start_chinadns now normalizes it to "$use_chinadns" before passing the transaction helper. Update the remaining assertion to match the source's normalized contract.
- XY-C1 was not triggered by the test/docs-only 8e commit. The source-changing cb6d XY-C1 run 44291015307 was still IN_PROGRESS at the last poll; do not claim firmware validation until its Build/TRX/artifact steps conclude.
- No source/runtime code changed in this slice; shell runtime tests passed on 8e. No target BusyBox runtime or physical-router L4 is claimed.

---

## 2026-11-18 — Fix the stop-timeout fixture and handoff section contract

- Exact 3ec095 Actions revealed the fixture repair still used a double-escaped printf format, so the files contained backslash-n text and pidof correctly classified the PID inventory as UNKNOWN (2). Replace with echo, which writes a valid newline-delimited PID and leaves the fake process in the intended never-exit mode.
- The AI handoff validator then advanced to its next structural check and rejected the renamed “Active engineering slice” heading; restore the required exact heading “## Active task”. The “Before context exhaustion” phrase is present.
- Exact 3ec095 Process inventory umbrella 44291402816 PASS. Shadowsocks 44291402913 failed at the stop-failure fixture (status 2 vs expected 1); AI docs 44291404027 failed on the required section heading. Exact-head rerun is pending. No firmware Build/TRX/artifact result is claimed here.
- This slice changes only test fixtures and handoff structure; no local shell execution or physical-router L4 is claimed.

---

## 2026-11-18 — Correct dnsmasq stop-failure fixture and restore handoff contract

- Exact cb6d source checks found two regressions. The DNSMasq stop-failure test wrote literal backslash-n sequences into its state and PID fixtures, causing the strict PID parser to correctly classify the mock inventory as UNKNOWN (status 2) rather than exercise the intended RUNNING/never-exit process path. Write both fixtures using printf format arguments.
- AI handoff run 44291015253 failed because SESSION_HANDOFF omitted the required literal cross-session rule “Before context exhaustion”. Restore that explicit end-of-session instruction.
- Exact cb6d Process lifecycle inventory 44291015163 and umbrella Process inventory 44291015219 PASS. Shadowsocks 44291015279 failed at the stop-failure case; AI docs 44291015253 failed on the missing phrase. XY-C1 44291015307 was IN_PROGRESS at the last poll; no Build/TRX/artifact claim is made.
- This correction changes only the regression fixture and handoff content; exact-head validation is required again. No local shell execution or physical-router L4 is claimed.

---

## 2026-11-18 — Make dnsmasq restart fail closed before config publication

- The current e9 transaction's shell regressions passed, but its Shadowsocks exact-head run failed only in a stale static source contract. A follow-up added the standalone dnsmasq count+1 launcher to keep DHCP/DNS outside the optional Shadowsocks cgroup; exact 745 checks pass for Shadowsocks, Process inventory/umbrella and AI docs.
- Tighten the transaction boundary: validate first, confirm the old dnsmasq process is stopped before publishing the staged file, and leave the live config untouched on stop timeout/UNKNOWN. If publication fails, restart using the unchanged config. On replacement failure, stop the candidate before restoring the backup and do not launch another daemon if that stop is unverified.
- Delay chinadnsng_enable_flag until dnsmasq config and service readiness are both confirmed. Add a stop-failure regression alongside success, syntax failure and restart rollback. Exact-head Actions for this correction are pending; XY-C1 L3 and physical-router L4 are not claimed.

---

## 2026-11-18 — Keep dnsmasq outside the Shadowsocks cgroup and align source contracts

- e9a69e67 passed shell syntax, startup status, process lifecycle runtime and Adblock regressions, plus Process inventory `44289343898`, umbrella `44289344240` and AI handoff `44289343965`. The Shadowsocks workflow `44289343841` failed only in the final source checker, which still expected the previous inline ChinaDNS `sed` lines.
- Update the ordering contract to require the transactional dnsmasq publisher after ChinaDNS daemon readiness. Review also caught that `run_bin()` attaches its process to Shadowsocks CPU/memory cgroups; dnsmasq must remain outside those cgroups. Add a standalone validated PID-count launcher with bounded count+1 wait and enable the cgroup option in its host regression.
- e9 XY-C1 `44289343944` was IN_PROGRESS at last poll. This correction modifies ROMFS source and needs its own exact-SHA Build/TRX/artifact result. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-18 — Stage dnsmasq configuration before verified restart and rollback

- ChinaDNS setup previously performed multiple live-file `sed -i`/append writes, followed by unchecked `killall dnsmasq` and detached launch. Partial writes or failed startup could strand the router without the previous working DNS configuration.
- Add same-directory staging and rollback copies, preserve metadata with `cp -p`, apply the intended ChinaDNS/cache directives to the staged copy, validate with `dnsmasq --test -C`, then atomically publish. Stop with the bounded lifecycle helper and require a new daemon PID via `run_bin`; on any stop/start failure, restore the previous config and attempt a verified old-service restart while preserving the original status. Keep the backup if restoration fails.
- Regression covers successful publication/restart, invalid staged config preserving live file and service, and replacement-start failure restoring old config and recovering dnsmasq. Static checker pins transaction order and propagation.
- Parent 4b956 Shadowsocks `44286192332`, Process inventory `44286192381`, umbrella Process `44286192617` PASS; XY-C1 `44286192538` Build/TRX/artifact PASS; docs-only 2528 AI handoff `44288452305` PASS. This source change needs its own exact-SHA Actions and XY-C1.

---

## 2026-11-18 — Gate ChinaDNS-ng startup on confirmed daemon readiness

- `start_chinadns()` previously set its enabled flag and launched `chinadns-ng` in the background without checking its upstream configuration, executable or child survival; router/GFW branches could mask startup errors with a later DHCP restart result.
- Add a validated launcher that rejects empty upstream DNS, fails closed on UNKNOWN pidof inventory, skips an existing instance and uses the common bounded count+1 `run_bin` postcondition for a new ChinaDNS-ng daemon. Set `chinadnsng_enable_flag` only after readiness and propagate errors in both router/GFW branches.
- Production-linked regression covers empty upstream, fresh daemon, duplicate, immediate exit and UNKNOWN inventory; static source checker pins executable lookup, daemon identity and caller error propagation.
- Parent e412 exact-head Shadowsocks lifecycle `44285579546` PASS and AI docs `44285579386` PASS. The base source has XY-C1 run `44283616981` SUCCESS with Build/TRX/artifact all PASS on cfbf source SHA; this ChinaDNS source change needs its own exact-SHA XY-C1 run before L3 is claimed.
- No physical-router L4 is claimed. Atomic dnsmasq config publication and verified restart rollback remain the next independent DNS task.

---

## 2026-11-18 — Fix Python checker newline boundaries for process helper ranges

- Exact-head fa45 Shadowsocks workflow 44285172102 passed shell syntax, required startup, process lifecycle and Adblock runtime regressions, then the Python checker raised ValueError while locating the next helper function.
- The two range delimiters were written as double-escaped \\n sequences, which search for literal backslash-n text. Change them to Python newline escapes so the named-wait and PID-count sections are sliced correctly.
- AI handoff 44285172079 PASS. cfbf XY-C1 run 44283616981 remains the only known firmware build for this production change and was IN_PROGRESS at last check; Build/TRX/artifact require exact-SHA confirmation.
- No local full test or physical-router L4 is claimed. New exact-head Actions required.

---
## 2026-11-18 — Update DNS readiness contract after count-wait helper refactor

- Exact-head `9204e82aa06efb3ef3a5fa774c1df4753328a315` Shadowsocks lifecycle workflow `44284758611`: shell syntax, startup status, production-linked process lifecycle and adblock publication regressions all PASS. The Python source checker failed because it still expected the old inline `ss_start_state` switch in `ss_wait_named_process_started()`, which now delegates to `ss_wait_named_process_count()`.
- Update the source contract to assert the delegated bounded wait and the tri-state PID-count behavior directly: status 1 with empty output maps to zero; malformed output/other statuses remain UNKNOWN (2).
- Exact-head AI handoff workflow `44284758515` PASS. The corrected checker has not yet been run; current XY-C1 run `44283616981` remains subject to exact Build/TRX/artifact verification.
- No local full shell run, current firmware L3 or router L4 is claimed.

---

## 2026-11-18 — Correct the bounded wait checker to match four actual loops

- Parent 2f3b still failed the final Shadowsocks source contract because the checker expected seven `sleep 1 || return 2` occurrences. Direct helper inspection confirms exactly four wait loops: named/pattern shutdown and named/pattern PID-count startup. The checker already asserts shutdown wait bounds and both startup count-loop bounds individually.
- Correct the aggregate count from seven to four. This is a test-only alignment; all production shell regression stages had already passed on cfbf/2f3b.
- AI handoff on 2f3b passed run `44284415671`. The cfbf XY-C1 run `44283616981` remained IN_PROGRESS at last check, so current-source L3 is not yet confirmed.

---

## 2026-11-18 — Align Shadowsocks process checker with bounded count-readiness helpers

- Exact-head cfbf 44283617060 passed shell syntax, startup status, bounded process lifecycle and adblock publication regressions, then failed in the static checker because it still expected five sleep guards from the old helper inventory.
- The ABSENT-under-errexit correction added two independent, bounded PID-count wait loops (named and pgrep-pattern), correctly increasing the fail-closed sleep-guard count to seven. Update the checker to assert seven guards and both new loop bounds rather than weakening production behavior.
- cfbf process regression 44283617129, process inventory 44283616977 and AI docs 44283617094 PASS. XY-C1 44283616981 was IN_PROGRESS for exact product-source SHA cfbf; verify Build/TRX/artifact separately.
- This checker-only correction needs a new exact-head Shadowsocks run. No local shell execution or physical-router L4 is claimed.

---
## 2026-11-18 — Preserve confirmed ABSENT PID inventory under set -e

- The exact-head e20 Shadowsocks lifecycle run `44282353324` failed at Lua-generated `ipt2socks` readiness even after the fixture sleep was widened to 100 ms. A POSIX `/bin/sh` reproduction isolated the issue: direct assignment of `pidof` output with exit status 1 is treated as errexit before the helper's ABSENT branch can normalize it.
- Change both named-process helpers and both pgrep pattern helpers to capture lookup status in conditional assignments, preserving RUNNING/ABSENT/UNKNOWN. Extend host regressions to assert ABSENT named and pattern counts are zero under `set -eu`; add checker contracts for all guarded lookups.
- Exact e20 Actions: Shadowsocks lifecycle `44282353324` FAIL at the Lua assertion; AI handoff `44282353220`, Process inventory `44282353174`, and umbrella fail-closed regression `44282353250` PASS. XY-C1 `44282353140` targets e20 and was still IN_PROGRESS; no L3 result for this fix is claimed.
- No full repository shell test or physical-router runtime is claimed. This helper change needs exact-head Shadowsocks and XY-C1 Build/TRX/artifact verification; L4 remains NOT VERIFIED.

---

## 2026-11-18 — Give the Lua-generated Shadowsocks readiness fixture a scheduler-safe polling window

- Exact-head e20 lifecycle run `44282353324` failed at `Lua-generated ipt2socks readiness`: the detached Lua mock was not observable before the test's 5-poll timeout. The e20 production watchcat change itself was not identified as the cause.
- The mock `sleep 1` yielded only 10 ms, a total window of at most 50 ms. Increase the test-only yield to 100 ms per poll (bounded maximum 500 ms) so the spawned fixture has a reliable scheduling opportunity; no production timeout or runtime behavior is changed.
- e20 AI handoff `44282353220`, Process inventory `44282353174` and full fail-closed inventory `44282353250` PASS. XY-C1 run `44282353140` was still in progress for product-source SHA e20; verify Build/TRX/artifact on that exact SHA.
- The test-only correction and refreshed handoff/roadmap require exact-head Actions. No local shell run or physical-router L4 is claimed.

---

## 2026-11-18 — Gate Shadowsocks watchcat launch on validated detached-process readiness

- The previous `start_watchcat()` fire-and-forgot `/usr/bin/ss-monitor` and returned success without observing the monitor. The new code performs a fail-closed `pgrep -f` inventory, skips duplicate launches when a monitor is already running, and waits for a bounded PID count increase after scheduling.
- Add validated process-pattern count/readiness helpers and production-linked test coverage for successful startup, timeout, scheduling failure, UNKNOWN inventory, duplicate prevention, disabled watchcat and no active proxy services.
- Parent `361d4b33ef8801d94cd13d949f693974b46af1d3` fixed the regression harness scheduler yield and exact handoff phrase. Exact-head Actions were: Process inventory `44281846459` PASS; full inventory `44281846403` PASS; AI handoff docs `44281846410` FAIL because the handoff omitted the exact heading `Mandatory handoff checkpoint`. The heading is corrected in this handoff.
- No local shell run or XY-C1 build is claimed. This watchcat slice needs exact-head Actions before it can be marked PASS.

---

## 2026-11-18 — Make detached proxy readiness regression deterministic and restore exact handoff contract text

- The parent SHA `8ad80886fbdd7623b118f3798af031a02cc6e897` re-ran the core process-lifecycle and AI-doc workflows. The latter still failed because the handoff used the literal `one atomic `main` commit` instead of the exact contract phrase `one atomic main commit`.
- The detached daemon regression's mock `sleep` returned immediately, so the bounded polling loop could exhaust all observations before the asynchronous fixture process had a scheduling opportunity. The fixture now yields for 10 ms on each poll, and successful detached launch calls use `expect_status 0` for clear diagnostics.
- Parent SHA `8ad808...`: Process inventory `44279976854` and full inventory `44279976810` PASS; Shadowsocks lifecycle `44279976835` and AI docs `44279976680` FAIL. This corrective commit fixes the test scheduling race and exact docs phrase; inspect the corresponding reruns before declaring them resolved.
- No local shell run or XY-C1 build was executed. Exact-head Actions and Build/TRX/artifact verification remain pending.

---

## 2026-11-18 — Repair detached-launch regression fixture and restore handoff contract headings

- Exact-head Actions for `47b599812981423af1cd03fd8928ff7b7cef7f4b` identified two failures. The process-lifecycle script set `RUN_BIN_MODE` but did not export it to its detached mock children; the AI documentation checker required three exact handoff headings and an explicit conditional-checkpoint sentence.
- The correction exports the fixture mode before the success/failure/UNKNOWN cases and restores the required handoff structure/wording.
- Exact-head Process inventory and lifecycle regression `44279554758` PASS; full inventory `44279554805` PASS. XY-C1 run `44279555018` was IN_PROGRESS at last poll. Recheck the corrective SHA before declaring either failed workflow resolved.

---

## 2026-11-18 — Verify every Shadowsocks detached proxy launch before reporting startup success

- `run_bin()` previously returned after scheduling a background subshell, so TCP, UDP and local-SOCKS callers could log success when the daemon exited immediately. In multi-thread mode, an old sibling could also satisfy a presence-only check.
- Capture the validated pre-launch PID count, replace the wrapper with the requested executable via `exec`, and require a bounded tri-state observation of count + 1. The Lua generator is checked against its actual `ipt2socks` child, not the short-lived Lua wrapper. Propagate failures through TCP, UDP (including Trojan UDP) and local startup; apply the same postcondition to watchcat replacement launches.
- Extend the production-linked host regression for sibling count, the Lua-spawned daemon, immediate exit and UNKNOWN inventory, and pin the launch paths in the source contract.
- No local shell test, host build, MIPS/uClibc build, TRX inspection, artifact upload or router test was run in this connector-only session. Exact-head push Actions must be checked after publication; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-18 — Verify Shadowsocks DNS proxy startup readiness

- Confirmed startup status gap: the dnsproxy/dns2tcp child was backgrounded and start_dns_proxy() returned success without checking that the required daemon appeared. stop_dns_proxy() also used unchecked pgrep/kill pipelines.
- Add a bounded pidof readiness wait that distinguishes confirmed absence from UNKNOWN/error. DNS startup validates backend, upstream and executable, waits for the selected daemon, and attempts cleanup on failure without signalling when inventory is unknown. Teardown uses the existing validated process helper; router/GFW startup propagates cleanup and launch status.
- Extend the host lifecycle regression for both backends, immediate child exit, unknown inventory (no signal), empty upstream and unsupported backend. Static source contract pins the caller propagation and cleanup behavior.
- Exact-SHA Actions pending after commit. No local shell execution is claimed. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-18 — Preserve Shadowsocks cgroup setup failure statuses

- Parent exact SHA: `5ed498112535ff34a3d7d7092530d0339524f6c5`. Confirmed defect: `cgroups_init()` ignored failed cgroup directory creation, CPU/memory quota writes and memory-limit readback; the trailing `export` could mask an earlier error.
- Added testable `ss_cgroup_mkdir`, `ss_cgroup_write_value` and `ss_cgroup_read_value` helpers. The initializer now captures each failing child status and returns it. Existing disabled-cgroup behavior remains a no-op.
- Expanded `tests/start_status.sh` to extract the actual production cgroup functions and inject directory, CPU write, memory write and readback errors. Extended the lifecycle checker to pin every stage and failure scenario.
- Parent rules-status follow-up: exact Shadowsocks run `44268519415` PASS on `5ed498112535ff34a3d7d7092530d0339524f6c5`; AI docs `44268393651` and Process inventory `44268393793` PASS on `eace3578`. Product-input XY-C1 `44268393871` on `eace3578` remained in progress; no L3 claim.
- This new slice's exact-SHA Actions are pending after push. No local execution or physical-router validation is claimed; L4 remains NOT VERIFIED.

---

## 2026-11-18 — Preserve exact Shadowsocks firewall-rule failure status

- Parent SHA `efe00be7f58c93feff92ea7d1f9102c2b2829582`; this commit fixes the remaining startup result-loss gap after the broader `ssp_start()` aggregation.
- `rules()` previously converted every nonzero `start_rules` / `ss-rules` status to 1. It now saves the failing child status immediately and returns it unchanged.
- The host test now extracts the real `rules()` function from `shadowsocks.sh`, stubs only its `start_rules` dependency, and verifies an injected status 12 reaches `ssp_start()`. The Python source contract requires the production capture/return sequence.
- Parent exact-head Shadowsocks lifecycle `44267823337` PASS on `efe00be7f58c93feff92ea7d1f9102c2b2829582`. Product-source XY-C1 `44267333941` on `62ecae88a449deccfb697b8e490590c0c514d6c1` was still building at last poll; its Build/TRX/artifact status is not yet known. This commit's exact-SHA Actions remain pending after push.
- No local execution is claimed. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-18 — Keep exact-SHA XY-C1 firmware builds alive across pushes

- XY-C1 used a shared per-branch concurrency group with `cancel-in-progress: true`. The `ffa7f6d0` production-source run `44264231022` was cancelled by later pushes before Build/TRX/artifact; the subsequent docs/test-only commit `0ecab3a6` correctly skipped the build based on its own diff, leaving the source tree without a current L3 check.
- Change the concurrency key to include `github.sha` and set cancellation false so every push's exact-SHA run remains independent. Add a unittest in `scripts/tests/test_xy_c1_firmware_inputs.py` to prevent regressions.
- Exact-head AI docs run `44265518065` PASS on `f46d2ed`; exact-head WAN route `44265049894` PASS on `0ecab3a6`. XY `44265049545` on `0ecab3a6` was host-only; Build/TRX/artifact skipped. Next product-source commit must show all three firmware steps on its own SHA.
- Next P1 module after this workflow fix: Shadowsocks startup required-stage error propagation. Frozen backup untouched; physical L4 NOT VERIFIED.

---

## 2026-11-18 — Record exact-head WAN pass and repair remaining handoff contract

- Exact-head WAN route run `44265049894` PASS on `0ecab3a6f91e6c835c24c92230d1c11ad7ad28de`, including deferred watchdog status propagation and the PPPoE-MWAN detector and caller regressions.
- AI handoff docs run `44265049584` failed on the same SHA at the cross-session mandate check: the live handoff did not state `one atomic main commit` and lacked the four standard checkpoint headings. This commit restores both while preserving the no-op documentation gate.
- XY-C1 run `44265049545` completed with Build, TRX and artifact upload skipped; no L3 claim. Investigate why the firmware input detector did not schedule the build needed for recent production-source changes before relying on this host-only result.
- Next P1 module task remains Shadowsocks `ssp_start()` required-stage error propagation. Frozen backup untouched; physical L4 NOT VERIFIED.

---

## 2026-11-18 — Correct PPP and deferred-watchdog CI contracts after exact-head failures

- Parent main SHA: `73ba562aecb420e23d0e210b01ab9c79b2406f46`; result: THIS COMMIT.
- Exact-head `73ba562aecb420e23d0e210b01ab9c79b2406f46` exposed three test-contract defects, not new firmware build failures: WAN route run `44264508036` rejected the valid double-spaced `int  start_watchdog(void);` declaration; AI handoff docs run `44264508172` rejected the missing exact end-of-session checkpoint rule; XY-C1 run `44264508112` stopped at the PPP source checker because its expected newline/tab pattern was double-escaped.
- Correct the declaration parser by splitting whitespace tokens, restore the explicit mandatory checkpoint wording in SESSION_HANDOFF, and change the PPP checker to match the actual tab/newline layout. Re-run all three workflows on THIS commit before claiming PASS.
- Firmware Build/TRX/artifact never started in failed XY-C1 `44264508112`; no L3 claim. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-18 — Close PPPoE-MWAN existing-interface probe failure propagation

- Source slice: `66dbe404` introduced sysfs-aware PPP probing; test cleanup `4bc9d87c` repaired the regression block; `ba893707` moved exact `ifconfig` status capture into the failing `else` branch.
- Existing ppp0 with failing ifconfig preserves status 44 while ppp1 remains independently reconcilable; an absent ppp1 sysfs entry skips ifconfig and is not reported as an error.
- Exact-head WAN route `44263552770`, Process inventory `44263552790`, Process fail-closed `44263552757` PASS on `ba893707c6f5cb939980cf4eecb5cd2d5dd14871`. XY-C1 `44263552808` was still IN_PROGRESS (Build active; TRX/artifact pending). Current descendant `ffa7f6d07522d8a51b53a9c1fede22c42d3588b2` has new independent WAN watchdog status propagation; its WAN/Process jobs were running and XY-C1 pending at last poll.
- This commit strengthens the PPPoE source checker to require error-status capture inside `else` and records the exact cross-session state. No physical-router test; L4 NOT VERIFIED.

---

## 2026-11-18 — Rotate the development log before the 120-record limit

- Exact-head AI handoff workflow `44260635743` failed because root `dev_log.md` had reached 120 complete records. The validator requires archiving the oldest 100 complete records verbatim and retaining the newest 20 before adding a further record.
- Preserved the 100 evicted records oldest-first in `docs/dev-log/2026/11/part-016.md`; archive size 105054 bytes, Git blob `633b273fa028cb3c197fc24e1d810896c1409810`. Updated the canonical index with the exact record count, date span, byte count and Git blob SHA. The 20 newest existing records remain unchanged in root, with this record prepended.
- Product-source SHA `a0cb9aad4844f7d044ce79efad6c417857c95972` passed VPN, Firewall, DNSMasq, HTTPD, Process inventory, Process lifecycle, NVRAM boundary and XY-C1 Actions. XY-C1 `44260005493` completed Build, TRX verification and artifact upload successfully. The docs-only parent SHA `6330828fca8a4d6f091e07e468732f40991e0909` needs a fresh AI handoff run after this rotation.
- Next source investigation: PPPoE-MWAN `detect.sh` currently treats every failed `ifconfig pppN` invocation as an absent interface. Distinguish a missing sysfs netdev (normal WAN reconvergence) from an `ifconfig` error on an existing interface; retain first error, continue independent PPP interfaces, and cover both paths in `detect_status.sh`.

---

## 2026-11-18 — Restore shared xl2tpd after incomplete VPN client teardown

- `restart_vpn_client_locked()` force-stops the VPN client and its shared `xl2tpd` before its second identity-aware stop/cleanup. If that follow-on stop failed, it returned immediately and skipped the shared-daemon restoration already present on the successful restart path, potentially stranding a configured WAN-L2TP client or L2TP VPN server.
- Restore the shared daemon on this specific failure path when the force-stop succeeded and WAN-L2TP or L2TP-server configuration requires it. Log restoration failure but preserve the original teardown status and errno. A failed initial force-stop still fails closed and does not trigger another start attempt.
- Add a GNU89 policy matrix regression and semantic source-contract ordering/errno checks; include the new policy header in VPN stability and XY-C1 path filters.
- Exact SHA `a0cb9aad4844f7d044ce79efad6c417857c95972`: VPN stability `44260005390`, Firewall `44260005246`, DNSMasq `44260005238`, HTTPD security `44260005241`, Process inventory `44260005245`, Process lifecycle `44260005256`, and NVRAM audit `44260005237` PASS. XY-C1 `44260005493` was still building; TRX and artifact were pending. Isolated local GNU89/Werror/ASan/UBSan decision-policy test PASS. No L3/L4 claim for this SHA.
- The previous Internet-monitor production SHA `8c272644dfc86a047eed50dd48aee42400607b72` passed its XY-C1 Build/TRX/artifact run `44258150922`; detect-internet static contract and handoff checks passed on checker SHA `764d7c7aeae4577f287cc6b5462d36c7e86aaa0c` (`44259182501`, `44259181122`).

---

## 2026-11-18 — Normalize detect Internet SIGUSR1/SIGUSR2 contract formatting

- Exact parent SHA `0e721be7339c59a18c9e94eff96c4454335c387f` repaired the process lifecycle checker and passed Process lifecycle run `44258799982` and AI handoff run `44258799964`.
- Detect Internet signal stability run `44258799968` still failed only at its inline source-contract check: the SIGUSR2 call used multiline C formatting with a different indent from the brittle expected literal. The GNU89/ASan/UBSan policy executable passed. Normalize both SIGUSR1 and SIGUSR2 call expressions after stripping whitespace.
- This repair changes CI validation/documentation only, not production behavior. Prior Internet-monitor source SHA `8c272644dfc86a047eed50dd48aee42400607b72` still needs final XY-C1 Build/TRX/artifact confirmation from run `44258150922`. No local shell/build or physical-router test is claimed; L4 remains NOT VERIFIED.

---

## 2026-11-18 — Report Internet-monitor launch and PID-file signal failures

- `notify_run_detect_internet()` discarded the `start_detect_internet()` status for a new monitor and the `process_pidfile_signal(SIGUSR1)` result for an existing monitor. `notify_pause_detect_internet()` also discarded SIGUSR2 errors, hiding launch, stale-PID and process-identity failures.
- The source now normalizes status/errno pairs and logs failed launch/resume/pause operations; ENOENT/ESRCH on pause remains a benign no-monitor/race condition. The GNU89 host policy regression passed on exact SHA `8c272644dfc86a047eed50dd48aee42400607b72`; its source contract initially failed on a multiline helper-call assertion.
- Exact 8c Actions also exposed a stale Process lifecycle checker expecting one-line PID-file signal calls. This follow-up makes both source contracts whitespace-insensitive. Process inventory, AI handoff docs and NVRAM transaction audits passed. XY-C1 `44258150922` was still building; TRX/artifact remained pending at last poll.
- The earlier SCUT/MentoHUST source SHA `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb` now has XY-C1 Build, TRX and artifact upload all PASS on run `44256159232`. No local shell execution or physical-router test is claimed.


---

## 2026-11-18 — Require SCUT and MentoHUST child readiness after background launch

- Parent main: `aab4a9a3eccc2ceb25bd584a8b2fb16952656534`. SCUT and MentoHUST used `start-stop-daemon -S -b` and treated a zero launcher status as a running daemon, although a detached child could exit immediately before entering the process inventory.
- Add shared POSIX `pl_wait_started()`: a one-second initial delay followed by four tri-state probes, one second apart. Only confirmed RUNNING succeeds; confirmed ABSENT after bounded probes returns timeout; UNKNOWN inventory, invalid arguments or sleep failure fail closed. Preserve launcher errors and existing duplicate/stop semantics.
- Require this postcondition in both scripts; extend shared and optional-app runtime regressions for absent, delayed and UNKNOWN child states; include the ROMFS-installed helper in XY-C1 build triggers and firmware-input tests.
- Exact-head repair SHA `4989579534f712f54126629b2a3323ef50436ad9`: Process lifecycle run `44256754447` and AI handoff docs run `44256754667` PASS. For implementation SHA `777d291bfad084cb7d6c7a3a28e8ed5d609c5acb`, Process inventory `44256159035` and Legacy wrapper pidfile `44256158977` PASS; XY-C1 run `44256159232` remained in progress, with Build running and TRX/artifact pending. No L3 is claimed until same-SHA Build/TRX/artifact are all successful. No local shell execution or cross-build is claimed; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-18 — Propagate WAN conntrack, TCP security and IGMP/MLD sysctl failures

- The WAN startup and system-control refresh paths called `set_nf_conntrack()`, `set_tcp_syncookies()` and `set_igmp_mld_version()` as void operations. These functions ignored failed procfs writes, so configured connection-table limits, SYN-cookie/ICMP protections or multicast compatibility values could silently remain unapplied while callers reported success.
- Return status from each setting group, attempt every independent TCP/ICMP and IGMP/MLD write, preserve the first error, and aggregate it in both `start_wan()` and `restart_all_sysctl()` without skipping unrelated WAN setup. Add a GNU89 host policy regression and source-contract checks; wire the WAN Actions and XY-C1 source filter.
- CI trigger review: the three declarations initially changed shared `rc.h`, causing unrelated RC service workflows to run. Move this narrow API to `network_sysctl.h`; only the three consumers include it, keeping future signature changes out of the global RC header.
- The first exact SHA `49f1ab1d4c921306ed67a34c45cadd84a21f17ae` workflow-file run (`44254413936`) was rejected before jobs because two checker steps were malformed YAML. The correction is committed on `180ac810ea6d9c4551512ce7aa2d5bc599d60935`; exact-head WAN route run `44254662647` and AI handoff run `44254662816` PASS. XY-C1 build for SHA 49f1 was still in progress; TRX and artifact steps were pending.
- No physical-router sysctl fault-injection or L4 result is claimed; exact-SHA Actions and XY-C1 build are the verification gates.

---

## 2026-11-18 — Fail closed when AP-client WISP association times out

- Parent main: `4388a1cca473a89255ba0ab938f19db450ee1ba5`. Two WAN startup paths called wait_apcli_connected() but discarded its result. The helper returns 0 when the AP-client interface never associates within its bounded wait, yet launch_wanx() and the IPoE path continued into DHCP/ZeroConf or static-address activation.
- Convert an unsuccessful wait into ETIMEDOUT. Abort physical WAN client setup in launch_wanx(); in IPoE startup, preserve the first WAN error, mark the unit failed, and skip address/lease activation. Add GNU89/ASan/UBSan policy coverage plus source-contract checks for both callers and both workflow path filters.
- Exact SHA `d486aba51fc1614a4e63836f1d192e2c7899ca24`: all relevant host suites passed except the Process lifecycle source-contract checker, which still expected the pre-readiness XUPnPD log. The WAN route suite first failed a whitespace-sensitive APCLI assertion; follow-up `aff2b0b5bd098ba4c775e84e583b8ad0cf18b149` normalized it and WAN route plus AI handoff workflows passed exactly on aff2. The checker update that matches XUPnPD's new launch/readiness status is now being applied.

---

## 2026-11-18 — Require XUPnPD child readiness after detached startup

- Parent main: f49ed8ab87a76ff4174408d49eb991649203760a. start_xupnpd() accepted the success of system("/usr/bin/xupnpd &") or the daemonizing launcher as proof of service startup, even though the child could exit immediately and no process presence was checked. A successful background shell exit could therefore produce a false RUNNING result to callers and allow IPTV startup to continue with no XUPnPD service.
- Add a fail-closed xupnpd process inventory gate before PID/config mutation, repeat the ABSENT check immediately before launch, and require a bounded post-launch RUNNING observation for both daemon and background launch modes. UNKNOWN inventory, duplicate presence, interrupted wait and timeout all return errors.
- Add injectable scan/wait policy and GNU89 ASan/UBSan regression cases for launch failure, immediate/delayed RUNNING, ABSENT timeout, UNKNOWN/malformed state and interrupted waits; wire source checks, WAN workflow, and XY-C1 path filters.
- Exact-head Actions for this task are pending after the atomic commit. No local compile, MIPS build on this new source SHA, or physical-router L4 is claimed.

---

## 2026-11-18 — Escape NVRAM values in generated Inadyn configuration

- Parent main: \`ffd0a5466fbe0c67e512e9a5c0284138a1b8eadc\`. \`write_inadyn_conf()\` interpolated NVRAM-sourced credentials, hostnames and custom server/path values directly inside libConfuse quoted strings. Embedded quotes/backslashes could change the parsed value, while raw line/control bytes could inject or break directives despite the existing private atomic file publisher.
- Add a quoted-value writer that escapes the libConfuse delimiters and line-control sequences, rejects unsupported control bytes, and routes every primary/secondary credential, hostname and custom endpoint through checked serialization. Any rejected/failed value discards the staged file and preserves the last-known-good config.
- Extend the GNU89 config-file regression with delimiter escaping and control-byte rejection, add the ASan/UBSan test step and trigger paths to the Inadyn lifecycle workflow, and require the production integration in its source checker.
- Exact SHA c69fe60d2f3c7f5beb4a94081ad24552f4c38df4: Inadyn config escaping GNU89/ASan/UBSan tests passed; initial Inadyn/DNSMasq contract failures were repaired in 98b6bc0, e6d7ce1 and fb6f183. Exact-head fb6f183 Inadyn [44249536374], DNSMasq [44249536311] and AI handoff [44249536380] all PASS. A further partial-stage rollback test first failed on 60a1f9d because it ran after fixture teardown; corrected before cleanup in f49ed8a. Exact-head f49 Inadyn [44250475415], DNSMasq [44250475599], AI docs [44250475535], Process [44250475515, 44250475692] and NVRAM audit [44250475565] all PASS. XY-C1 exact product-source SHA c69fe60 build [44249015863] completed with Build XY-C1, TRX verification and artifact upload all SUCCESS. Host firmware validation is L3 evidence only; physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Align Adbyby source contract with shared publisher

- Parent main: `d67ca20b78785b0d501829c12db6dfd2fc4f0fae`. The first exact-head Actions run passed the Process lifecycle regression, but the Adbyby configuration workflow's source-contract checker still required `hosts_ads` to appear directly in both callers after publication was correctly moved into the shared helper.
- Update the checker to assert that start and rule-update invoke the shared publisher, that the helper retains DNS/firewall/hosts/dnsmasq/optional-cron ordering, and that first-error preservation and rollback remain wired to both callers. This is a checker synchronization fix, not a relaxation of production behavior.
- Exact SHA `d67ca20`: Process lifecycle, process inventory, AI documentation and HTTPD regressions passed; the Adbyby contract failure was this stale assertion; XY-C1 firmware validation remained in progress. The rerun on the checker-fix SHA is required. No local shell execution is claimed.

---

## 2026-11-17 — Roll back Adbyby when post-launch publication fails

- Parent main: `8b92f532974fc5363115aa1e9bd80ea791bb68d4`. After detached-daemon RUNNING verification, both Adbyby startup paths still ignored DNS, firewall, hosts, dnsmasq-restart and cron errors, so partially configured services could be reported as started.
- Route start and rule-update through one checked publisher. Preserve the first failing status/stage, stop and clean the partial service best-effort, and keep the update path from changing cron.
- Harden DNS fragment/config writes and cron-file/schedule updates so those functions return real write/validation failures. Extend the existing Process regression with per-stage injected failures, rollback assertions, short-circuit ordering and update-path coverage.
- Local shell execution was unavailable in this connector-only session; no local test, firmware build, Actions result or physical-router validation is claimed. Exact-head push Actions must be inspected after publication.

---

## 2026-11-17 — Verify Adbyby before publishing DNS and redirect state

- Parent main: `f50583d26acad4904271fcd0d968425c2cd906fc`. adbyby_start() and adbyby_uprules() both backgrounded the daemon and immediately continued into DNS/firewall/hosts setup without confirming the child survived. The restart action also discarded adbyby_start() failures and returned zero.
- Add a shared bounded startup postcondition: only a validated RUNNING inventory succeeds; ABSENT is retried for five probes, UNKNOWN/malformed pidof and sleep errors fail closed. Both initial start and rule-update launch paths must pass it before publishing DNS/redirect state; restart preserves the original start error.
- Add shell regressions for RUNNING, delayed appearance, ABSENT timeout, UNKNOWN/malformed inventory, wait failure, start/update publication gates and restart status propagation. Wire the regression into the Process lifecycle workflow; the existing XY-C1 push filter already includes the production Adbyby script.
- The shell regression was not executable in this connector-only editing environment before publication; its exact-head Actions result must be checked. No MIPS/uClibc target build, firmware artifact or physical-router L4 result is claimed.

---

## 2026-11-17 — Correct AdGuardHome checker indentation after static-test failure

- Parent main: `90768054381f8756fe0771de6a02ff0fcb078509`. The shell lifecycle regression passed delayed RUNNING, confirmed-ABSENT rollback, UNKNOWN preservation and wait-failure cases. The following source-checker run failed before evaluation because the second rollback-position variable had one accidental leading space at module scope. Correct the indentation; no production code or shell fixtures change.
- Exact-head AdGuardHome config/security and AI documentation checks are required again.

---
## 2026-11-17 — Anchor AdGuardHome timeout rollback ordering after launch

- Parent main: `f02fc8b380da5fc202779bbe0a9862b41841a6ef`. The detached-start shell regression passed all new cases on `f02fc8b3`, but the source checker initially searched for the first `clear_iptable()` call in `start_adg()`; that earlier call belongs to the pre-launch resolver-failure rollback, not the new post-launch timeout rollback. Anchor the second rollback check after the post-launch `del_dns()` call so it verifies the intended reverse-order cleanup without confusing the existing pre-launch path.
- Production code and lifecycle fixtures are unchanged. Exact-head AdGuardHome config/security, Process inventory/umbrella and AI handoff checks are required again.

---
## 2026-11-17 — Verify AdGuardHome after detached daemon launch

- Parent main: `b3b856dd6a78c4ee62333f288c64b867f7599a21`. `start_adg()` backgrounds AdGuardHome, then returned success immediately. Unlike DDNSTO and AliDDNS, this launcher had no post-fork process check; an immediate bind/config/runtime exit could leave resolver configuration and DNS redirect rules pointing at a daemon that never stayed alive.
- Add a five-attempt tri-state process wait. Only RUNNING reports success; confirmed ABSENT timeout rolls back resolver configuration before firewall redirects; UNKNOWN (including a failed wait primitive) fails closed and preserves both traffic states because the daemon may be alive. Extend the existing shell regression with delayed RUNNING, timeout rollback, UNKNOWN and sleep-failure cases, and strengthen the Python source contract.
- Exact-head AdGuardHome config/security and Process lifecycle/inventory checks are required. This shell-only package change is not in the XY-C1 firmware path filter; no firmware rebuild or router runtime result is claimed.

---
## 2026-11-17 — Keep Infosvr packet regression scoped to Infosvr behavior

- Parent main: `10c232113e404cee71248e268485575e0b2a63c4`. The Infosvr packet/signal regression script also asserted root `SESSION_HANDOFF.md` section headings. That cross-project dependency caused an unrelated Infosvr workflow failure whenever a valid handoff used a changed heading, duplicating the dedicated AI handoff documentation contract.
- Remove only the handoff read/assertions from `check_infosvr_packets.py`; retain all packet bounds, signal lifecycle, and workflow contracts. The canonical AI handoff workflow remains the owner of handoff structure validation.
- Parent Infosvr run `44237364896` failed solely on the now-corrected handoff heading; the next Infosvr exact-SHA run must confirm this checker is scoped and green.

---

## 2026-11-17 — Restore the exact handoff heading required by the live contract

- Parent main: `9df55d47899e5b84b491f70c00c224b14998aa77`. The new ttyd handoff content was correct, but the AI documentation contract requires the exact section heading `## Exact first action for the next GPT`; this checkpoint initially used a plural variant and the exact-head AI handoff run `44237365114` failed on that literal.
- Restore the canonical heading without changing the task or verification claims. Re-run the AI handoff documentation contract on this corrective SHA.

---

## 2026-11-17 — Verify the detached ttyd child after launcher success

- Parent main: `56eff0e0e7c27fd1c632eb41e96df733febd72aa`. `ttyd.sh` starts the service with `start-stop-daemon -S -b`, so `eval()` only proves that the background launch was requested. `start_ttyd()` previously returned success without checking whether the detached child survived a bind/setup failure or immediate exit.
- Add the same bounded RUNNING postcondition used by other detached RC daemons: initial wait, four process-inventory probes, immediate fail-closed UNKNOWN handling, ETIMEDOUT when the child remains ABSENT, and unchanged launcher-error propagation. Preserve disabled and already-running no-op behavior. Extend both Process integration source contracts; the existing GNU89 process-presence policy test already covers RUNNING/ABSENT/UNKNOWN/invalid states and launch-status preservation.
- Exact-head Process lifecycle and XY-C1 Build/TRX/artifact checks are required on the new source SHA. No local production build or physical-router L4 test is claimed.

---

## 2026-11-18 — Align WAN restart regression contracts with propagated teardown statuses

- Parent main: `5ddc031903da062cdd55784c8f9c31fe2c1c0d9e`. Exact-head CI for `5ddc0319` verified the new full-restart source behavior but exposed three stale callers' tests: HTTPD still required `if (stop_wan() != 0)`; WAN VLAN still required an inline `config_soft_bridge_lan() != 0`; WAN teardown searched for both old inline bridge gates and generic `return -1`.
- Update those contracts to require status capture, nonzero gate, operation-specific diagnostic, unchanged `return status;`, and prevention of later reset/start steps. No product source behavior is changed in this test-only follow-up.
- Exact-head HTTPD, WAN-route and WAN-teardown Actions must pass again; 5ddc0319 XY-C1 was in progress at observation and its Build/TRX/artifact state remains separately pending.

---
## 2026-11-17 — Preserve concrete WAN restart teardown errors

- Parent main: `059de22e8e646f238606d543fd8ccbda3cce7fa2`. `full_restart_wan()` discarded three distinct lower-level failures: `stop_wan()`, `disassembly_bridges_wan()` and `config_soft_bridge_lan()` were each converted to generic `-1`, and the log omitted the actual status. These helpers already return checked error codes, including the first bridge teardown failure. Capture each result, log it and return it unchanged before proceeding to route/switch/restart work.
- Add source-contract regression assertions that each result is captured, gated, logged and returned unchanged. This does not alter successful restart ordering. Exact-head WAN/Firewall, Process, USB and XY-C1 checks are required; L3 only counts if XY-C1 builds/TRX/artifact on the exact source SHA.

---
## 2026-11-17 — Keep the common WAN checker out of redundant XY-C1 trigger fan-out

- Parent main: `0ab1c2001cd807db65c949ce34fd219d1511f778`. The XY-C1 workflow already executes `scripts/check_modem_wan_start_status.py` during host validation, but editing that checker alone cannot change firmware inputs and already triggers its owning Process/Firewall/USB regression workflows. Its extra push-filter entry created a host-only XY run with no target build (the exact `2b4c7aea` run reported `Firmware production inputs changed: False`). Remove only the checker from the XY push trigger; retain execution inside XY-C1 whenever genuine firmware-input changes trigger the workflow.
- This preserves firmware-source coverage while eliminating the redundant Actions run for checker-only edits. Exact-head XY and AI docs checks validate the trigger change.

---
