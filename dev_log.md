## 2026-11-23 — Reject whitespace-corrupted DNS upstreams before proxy teardown

- The DNS preflight accepted whitespace-only values (for example `tunnel_forward='   #53'`) and values containing embedded whitespace. They passed the old non-empty/`nil` check, so `ss_restart_dns_proxy()` could stop the healthy local DNS daemon before the replacement was guaranteed to be usable.
- `ss_dns_proxy_config_read()` now rejects any whitespace in the upstream token before publishing the cached snapshot. The production-linked shell test covers whitespace-only and embedded-whitespace values and asserts that neither the stop nor start function is called. The Python source contract requires the whitespace gate and both negative cases.
- Parent `7fdf985` exact Shadowsocks run #45233294870 passed the complete DNS preflight, startup-status and process-lifecycle suites after the previous dispatcher-contract fix. This new validation slice is not yet verified; exact-head Actions are pending.

---
## 2026-11-23 — Update DNS readiness contract for preflight/launcher split

- Exact 7a48 Shadowsocks #45232308218: shell syntax and all host regressions passed, including the repaired direct process-lifecycle fixtures. The Python source checker then failed because its existing readiness assertion sliced only `start_dns_proxy()` but still expected the executable `command -v` check there; that check now correctly lives in the preceding `ss_dns_proxy_config_read()` preflight.
- Corrected the source-contract slice to cover the ordered preflight → launcher → restart wrapper block. The existing ordered assertions still require executable validation before the backend launch, bounded readiness observation, cleanup on launch failure, and propagation through the stop/start wrapper. No production code changed in this correction.
- Exact 7a48 AI handoff and Process inventory workflows passed. XY-C1 validation remained a separate pending run; rerun the exact-head Shadowsocks and Process Actions for this checker correction.

---
## 2026-11-23 — Align direct DNS process lifecycle fixtures with preflight snapshots

- Exact DC8 Shadowsocks run #45231946878: shell syntax, startup status regression, DNSMasq cleanup regression and the new DNS proxy preflight regression all PASS. The subsequent existing process-lifecycle test FAILED because it extracted `start_dns_proxy()` and invoked it directly without first sourcing/calling the new `ss_dns_proxy_config_read()` helper; under `set -u`, the missing snapshot correctly failed closed (`SS_DNS_PROXY_PROCESS: parameter not set`).
- Updated that production-linked lifecycle harness to extract the preflight helper and validate both dnsproxy/dns2tcp fixtures before launch. Empty-upstream and unsupported-backend negative cases now assert the preflight function rejects configuration before any launcher invocation. Added a source-contract check to prevent direct-launch tests bypassing the snapshot.
- Rerun the complete exact-head Shadowsocks workflow after this correction; the initial DC8 AI handoff contract and Process inventory workflows passed. XY-C1 #45231946608 remained in progress during the failure diagnosis.

---
## 2026-11-23 — Preflight DNS proxy replacement before stopping the live service

- `start_dns()` stopped `dnsproxy`/`dns2tcp` before `start_dns_proxy()` read `pdnsd_enable`, `tunnel_forward` and checked the selected executable. A transient NVRAM read failure, empty upstream, invalid selector or missing binary could therefore take down a working local DNS upstream before the replacement was rejected.
- Added a validated configuration snapshot for selector, upstream and executable, then routed router/gfw restarts through preflight → stop → start. The launcher uses the validated snapshot, so validation failures return without touching the current daemon and a stale NVRAM re-read cannot change the launch target after teardown.
- Added production-linked `/bin/sh` regression cases for selector/upstream read errors, nil upstream, invalid selector, successful dns2tcp selection, and stop/start error propagation, plus Python source-order assertions. Exact-head Shadowsocks and XY-C1 checks remain the acceptance gate; no live-router behavior is claimed.

---
## 2026-11-23 — Bind AliyunDrive process state to its executable inode

- BusyBox 1.24 pidof accepts a process when its comm or basename of argv[0] matches the requested name. A different executable can therefore spoof aliyundrive-webdav; the previous tri-state wrapper treated that result as RUNNING and could send TERM/KILL to the wrong process during stop.
- ald_process_state() now resolves the configured daemon and every /proc/PID/exe candidate with stat -L -c '%d:%i'. A missing, malformed or mismatched identity returns UNKNOWN (2), which blocks start/stop signalling; only candidates whose device/inode matches the packaged executable are RUNNING. The existing no-cached-PID and bounded stop policy is unchanged.
- The lifecycle fixture mocks only these executable-identity stat calls and retains real stat behavior for lock/workdir checks. Added mismatch, inaccessible and malformed identity cases, including no-signal and no-duplicate-start assertions; the Python source contract pins the inode gate. Parent main: 53c6f6623e5531deeb889762e81bf1943c1abf50. Exact-head AliyunDrive lifecycle and AI handoff Actions are pending for this source slice. No local shell execution or firmware validation is claimed.

---
## 2026-11-23 — Repair stale DNSMasq split-DNS test contract

- Historical exact DNSMasq run #45110039566 on `d36ba24` compiled and executed the GNU89/ASan/UBSan NVRAM policy regression successfully, then failed in `check_dnsmasq_integer_options.py` because it required the helper `dnsmasq_split_dns_snapshot_is_zero` to also appear as a standalone `helper();` invocation. The test suite intentionally defines this helper to validate public snapshot fields without relying on struct padding, and its error-path tests call it with `&snapshot`.
- Corrected the checker to extract the helper body and require the field-zero predicate plus an actual `assert(dnsmasq_split_dns_snapshot_is_zero(&snapshot));` call. This removes the false negative without weakening the failure-path test contract or changing production code. Exact-head DNSMasq and XY-C1 Actions are required for this current checkpoint.

---
## 2026-11-23 — Repair the backup-server (`reserver`) startup path

- `ressp()` called `start_redir`, which is not defined in `shadowsocks.sh`; the actual TCP proxy was therefore not launched. Its next `start_rules $BACKUP_SERVER` passed an ignored argument to a helper that reads `GLOBAL_SERVER`, leaving firewall/config generation on the primary node. Stage errors were ignored and the function always printed “backup server started successfully”.
- The backup path now rejects unreadable/empty/`nil` selectors, switches process-local `GLOBAL_SERVER` to the selected backup, and follows the primary startup sequence: firewall, cgroup setup, TCP, optional UDP, DNS, configured local SOCKS5 and watchcat. It preserves the first required error while keeping cron best-effort; the `reserver` CLI propagates failure.
- Extended the production-linked startup fixture and Python source contract for backup node identity, `same` UDP/local mapping, invalid/unreadable configuration, TCP/UDP dependency gates and optional cron failure. Exact-head Actions are required before calling this slice verified; no router failover is claimed.

---
## 2026-11-23 — Revalidate process identity across PID reuse

- `proc_read_identity()` previously read `/proc/PID/stat` and `cmdline` once each. If the process exited and its PID was recycled between those reads, the scanner could combine the old process's matching `comm` with the replacement's unrelated command line, incorrectly report RUNNING and suppress a daemon restart.
- Candidate matches now re-read and parse stat before returning RUNNING. Disappearance/zombie is ABSENT; malformed or inaccessible stat remains UNKNOWN; changed start time/comm/truncation is fail-closed UNKNOWN/EAGAIN. The extra read occurs only after a candidate match.
- Added `test_pid_reuse_between_stat_and_cmdline_is_unknown()`: the fake procfs reader replaces PID 123 between reads, and the test requires UNKNOWN/EAGAIN plus an empty snapshot. The Process regression path already tracks both source and test files; XY-C1 tracks the shared source.
- Parent main: `8aa5ef84403044d2f81160d3fc259384c67c4c60`. Exact-SHA GNU89 host regression, process source audit and XY-C1 build/TRX/artifact checks are pending. No local build is claimed.

---

## 2026-11-23 — Anchor p910nd launch failure contract and restore handoff rule

- Exact e52 USB modem #45225619186 PASS, including the p910nd GNU89/Werror/ASan/UBSan mapping fixture and USB NVRAM/source contract. Exact e52 Storage #45225619727 PASS.
- Exact e52 Process fail-closed #45225619509 FAIL: `launch_error_gate` was found with an unanchored `index()`, so the assertion selected the earlier invalid-device input error branch, not the launcher failure branch. The Process workflow now searches for the error gate starting at the `eval()` launch.
- Exact e52 AI handoff #45225619139 FAIL because `SESSION_HANDOFF.md` did not include the exact required phrase `one atomic main commit`; restored it while retaining one full Git tree and expected-SHA non-forced update.
- A documentation-only continuation commit `0e9eb13c56c6f83fe75fbd8261eedaaef6ff154c` added the previous cross-session task summary but left those two exact failures unresolved. This atomic checkpoint corrects them. XY-C1 #45225109549 on e6a6 was still building; TRX/artifact pending. No L3 or hardware result claimed.
## 2026-11-23 — Normalize p910nd rollback contracts and test invocation

- Exact USB modem run #45225109809 on source SHA `e6a6a3868a0e409a009ede9b01d01c355c372b15` passed the WAN/modem/firewall contracts, then failed in `check_usb_printer_nvram.py` because the checker expected `marker();` while the GNU89 test invokes each case through `CHECK(test_name() == 0)`. The production test was correctly wired; the checker now verifies that actual invocation form.
- The same checker and Process/Storage workflows previously encoded callback formatting or broad character-window assumptions. They now compact whitespace and explicitly assert `usb_printer_daemon_start_failure_recover(process_name,start_status)`, ensuring the port-specific identity is passed without relying on indentation.
- The e6a6 source commit fixes the substantive nonzero-port rollback bug by passing the derived `process_name` rather than literal `p9100d`. USB modem #45225109809 is the checker failure described above; Process #45225109772, Storage #45225109612, process inventory #45225109604, NVRAM #45225109574 and XY-C1 #45225109549 were in progress at the last poll. AI docs #45225109551 PASS. Exact reruns of these checker corrections are pending.
- No local test, MIPS firmware build, TRX, artifact or hardware printer result is claimed.

---

## 2026-11-23 — Keep p910nd readiness rollback scoped to the selected printer

- The first exact-SHA USB workflow caught a source-contract ordering assertion error; review of the same source also found that the new per-port readiness path still passed the old literal `p9100d` to the rollback helper. That could stop the wrong daemon when a non-zero port failed readiness.
- Changed rollback to use the same validated `process_name` used by readiness. Corrected the checker ordering to reflect that port parsing/name construction precedes the port-specific launch. Process workflow now explicitly guards the per-port rollback target.
- Initial p910nd test compile and full exact-head XY-C1/Process/Storage results are being checked on successor SHA.

## 2026-11-23 — Correct p910nd per-printer port and process identity

- Fixed a concrete multi-printer defect: the hotplug loop scans `/dev/usb/lp0` through `lp9`, but every p910nd launch passed port `0`. p910nd uses that digit for its TCP port, lock/PID filename, and rewritten process identity (`p9100d` through `p9109d`), so multiple printers collided and readiness for later devices could be falsely satisfied by the first daemon.
- Startup now strictly parses the canonical `/dev/usb/lpN` path, passes that same digit to p910nd, derives the matching rewritten process name, and performs readiness plus failure rollback against that specific instance. Stop now targets the launcher name and all ten rewritten daemon identities.
- Added a GNU89 ASan/UBSan policy regression for the complete 0–9 map, malformed paths, invalid ports, and non-mutating error paths. USB, Process and Storage workflow source contracts and path triggers are updated. Exact-head Actions pending; hardware multi-printer runtime remains unverified.

## 2026-11-23 — Roll back late p910nd children after readiness failure

- `start_p910nd()` previously returned a readiness timeout or process-inventory error after `p910nd` had already detached its child. The caller observed only the launcher parent, so an incompletely initialized listener could remain running and be retried later.
- The readiness failure path now invokes the shared identity-validated rollback with process name `p9100d` (the child’s rewritten argv[0]), then re-scans process state while preserving the original failure status. This is limited to post-launch/readiness failures; a confirmed launcher failure still returns directly.
- Extended the USB modem, Process and Storage source contracts to require the `p9100d` rollback after the readiness gate. The existing GNU89 detached-daemon policy regression covers cleanup outcomes and primary-error preservation; exact-head Actions are required before treating this slice as verified.

## 2026-11-23 — Align Storage workflow with USB printer rollback

- Storage hotplug run #45221949175 exposed a stale source assertion left over from the pre-rollback launcher: it required a direct `return EIO;` after a partial exec-status write. The implementation now invokes the shared failed-start rollback policy for that uncertain state.
- Updated the Storage source contract to check the rollback hand-off and the dedicated stop/re-scan callbacks rather than the removed direct return. This was a test-contract mismatch, not a runtime storage failure.

## 2026-11-23 — Correct Process workflow rollback callback scope

- Process run #45221949004 failed only because the new source-contract snippet searched for `kill_services()` inside the recovery function, although the call intentionally lives in its separate stop callback. The implementation and USB modem workflow checker both passed.
- Updated the workflow to inspect the stop callback and recovery function independently. No production code changed; rerun on the successor SHA is required.

## 2026-11-23 — Roll back late USB printer daemons after failed readiness

- Follow-up reliability defect: when U2EC/LPRng `execv()` succeeded but the bounded RUNNING postcondition timed out, was interrupted, or encountered an unknown process inventory, the caller returned failure without stopping a daemon that could appear late. A pipe read/partial-status error also left launch state uncertain.
- The printer launcher now uses the shared `detached_daemon_start_rollback_with_ops()` policy on readiness and post-fork status-channel failures. It sends an identity-validated stop through `kill_services()`, takes a fresh process snapshot, logs confirmed absence versus incomplete rollback, and preserves the primary startup errno. A definitive full exec-error record still returns directly because no daemon was started.
- The existing GNU89 rollback-policy regression already covers successful no-op, stop, final ABSENT/RUNNING/UNKNOWN and primary-error preservation; USB-printer and Process source contracts now pin this integration. Exact-head Actions pending.

## 2026-11-24 01:00 CST (UTC+8) — Restore mandatory handoff accuracy checkpoint after AI docs failure

- Exact AI documentation run #45221089290 on `e4d34fd7f13baa203bbbae90130b0782a9b22e5c` failed with: “current handoff does not require an end-of-session accuracy checkpoint”. The handoff had been compacted but omitted the validator-required `Mandatory handoff checkpoint` section and exact conditional-edit sentence, along with its required cross-session heading and section names.
- THIS commit restores the mandatory checkpoint contract, including “The checkpoint is mandatory; a document edit is conditional.”, live-state inspection, materiality/no-op guidance, cross-session atomic-main rules, and the four required handoff section headings. No product source changed.
- USB modem checker PASS remains #45220509149 on f947; Process, NVRAM and Storage PASS remain #45220191260/#45220191315/#45220191366 on ecff. Exact AI docs rerun on THIS handoff correction is pending; XY-C1 runs #45220033426 (e6dc), #45220191233 (ecff), and #45221089272 (e4d34) were still in progress at last observation.

---

## 2026-11-24 00:53 CST (UTC+8) — Trigger XY-C1 build on WAN ARP policy header changes

- `detect_wan.c` directly includes `detect_wan_arp_socket_policy.h`, but the XY-C1 push path allowlist listed only the C file. A header-only production change could bypass firmware cross-build/TRX validation.
- Added the production policy header beside `detect_wan.c` in `.github/workflows/xy-c1-firmware-validation.yml`. The workflow remains main-only/push-only; its event model was not changed.
- USB printer NVRAM checker correction landed in `f9475bf081db66e795df8c25332253ef7c6c80a2`; exact USB modem run #45220509149 PASS, including the previously failing source-contract step. Process fail-closed #45220191260, NVRAM audit #45220191315 and USB storage #45220191366 PASS on `ecff475abb782b32e0a8f4f02026acc709ea6844`.
- XY-C1 #45220033426 (`e6dc19c`) and #45220191233 (`ecff475`) were still building at last observation, with TRX and artifact steps pending. No L3 claim is made. This workflow-only path correction does not retroactively retrigger those source builds.

---

## 2026-11-23 — Align USB printer NVRAM checker with idempotent launcher

- The first Process/Storage source review caught one stale checker assumption: `check_usb_printer_nvram.py` still expected U2EC/LPD to unlink PID files in the individual wrappers. Cleanup now belongs to the shared launcher after a tri-state ABSENT preflight, so the checker has been updated to verify the checked-setting gates, three-argument paths, fail-closed UNKNOWN handling, RUNNING no-op and cleanup-before-fork ordering.
- This is a regression-check correction, not a production-code change. The initial USB modem workflow failed only this stale assertion; exact successor Actions are pending.

## 2026-11-23 — Normalize invalid process-inventory error values

- Tightened the shared process-presence start policy: an UNKNOWN scan now maps zero or negative/non-errno error values to `EIO`, preserving only positive errno values. This prevents a negative internal result from escaping as a negative startup status or being confused with a launcher convention.
- Added a GNU89 regression for negative inventory status and a source-contract assertion. Exact-SHA Process, Storage and XY-C1 Actions are tracked on the successor commit.

## 2026-11-23 — Make USB printer daemon launch idempotent across hotplug retries

- Fixed a retry-path duplication defect in U2EC/LPRng startup. A previous launch can leave the daemon running even when the subsequent NVRAM/hotplug acknowledgement fails; the retry path previously unlinked that live daemon's PID file and forked another instance.
- `start_usb_printer_daemon()` now takes ownership of the daemon PID-file path, performs a tri-state process inventory before any cleanup or fork, treats RUNNING as an idempotent success, and fails closed on UNKNOWN. A stale PID file is removed only after a complete ABSENT scan, and unlink errors are propagated instead of ignored.
- U2EC and LPRng no longer unlink PID files before the shared preflight. Existing process-presence policy regression and the Process/Storage source contracts are extended to assert RUNNING/UNKNOWN gates precede PID cleanup and fork. Exact-head Process/Storage/XY-C1 validation pending; hardware USB printer behavior remains unverified.

