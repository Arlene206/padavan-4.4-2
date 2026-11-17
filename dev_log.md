## 2026-11-18 — Align MiniUPnP lifecycle integration contracts with lock wrappers

- Parent main SHA: `03a993e558b23f5f163ca1a904cbf7b54f6ef7e9`, which introduced stable-inode lifecycle serialization around MiniUPnP public start/stop/WAN/update/restart entry points.
- Exact-head Process run `44192259665` compiled and passed the process inventory, service lifecycle, PID-file, SSH and earlier C policy regressions, then failed in `scripts/check_modem_wan_start_status.py:671`: its WAN firewall contract still expected `rc_upnp_restart_run(stop_upnp, restart_firewall)`, but the now-locked implementation correctly uses `stop_upnp_locked` while holding the lifecycle lock. XY-C1 `44192259641` stopped at the same stale assertion before Build.
- Dedicated MiniUPnP run `44192259644` passed atomic config publication and GNU89 daemon policy tests, then failed its embedded source checker because it still extracted public wrappers as though they contained the internal start/stop/WAN/update operations. Update it to inspect the `*_locked` operation bodies and verify the public restart lock ordering.
- Exact 03a AI docs `44192259762`, Samba `44192259623`, Inadyn `44192259711`, DNSMasq `44192259636`, infosvr `44192259808`, AdGuardHome `44192259712`, AliDDNS `44192259810`, WAN route `44192259691` and HTTPD security `44192259847` passed. MiniUPnP-specific Process failure `44192259665` and MiniUPnP test-workflow failure `44192259644` are checker-only at the first causal failure; NVRAM `44192259875` and AliyunDrive `44192259658` were still running at last poll.
- THIS commit corrects source contracts and ensures the dedicated workflow watches/runs the new lock policy checker; no production behavior changes. Exact-head Process, MiniUPnP, AI and XY-C1 checks must rerun. XY-C1 Build/TRX/artifact on 03a was skipped after its pre-build checker failed; no L3 claim.

## 2026-11-18 — Serialize MiniUPnP lifecycle transitions across RC callers

- Parent main SHA: `7bd030fa1e515250e14b6c3a075901eb55d6bfa2`; source slice changes `trunk/user/rc/src/services_ex.c`, adds `miniupnpd_lifecycle_lock.h` and `tests/miniupnpd_lifecycle_lock.c`, adds `scripts/check_miniupnpd_lifecycle_lock.py`, and integrates the test in Process lifecycle workflow.
- Audit found public MiniUPnP start, stop, WAN-interface cleanup, lease reload and firewall restart entry points had no shared lifecycle lock. Start scanned ABSENT then published config before launch, allowing concurrent RC/WAN notifications to both pass preflight and interleave config/start/stop.
- One stable-inode private fcntl lock now serializes every public state-changing entry point. Internal `*_locked` helpers prevent recursive acquisition. Restart retains lock through firewall refresh; update covers inventory/decision/signal; WAN check covers scan and optional stop.
- Startup re-scans daemon presence after config publication and immediately before exec. The GNU89 ASan/UBSan regression covers cross-process contention, hardlink/symlink rejection, invalid paths and release; the Python checker validates wrapper coverage and ordering.
- Exact-head Process, AI handoff and XY-C1 Build/TRX/artifact checks pending. No hardware L4 claim.

## 2026-11-18 — Restore required cross-session handoff heading

- Parent main SHA: `0ab0849c47cb210a24264236721c18c7f4dc077c`.
- The AI handoff checker on exact `0ab0849c`, run `44191156095`, passed the restored 32-entry `dev_log.md` retention check but failed the next exact-text contract: `SESSION_HANDOFF.md` was missing the literal `# Cross-session execution mandate` heading. The prior top-checkpoint replacement had inadvertently dropped the heading while retaining its body rules. THIS correction restores the required heading immediately before those rules.
- Other exact `0ab0849c` checks passed: Process lifecycle `44191156121`, PID1 notification queue `44191156366`, infosvr `44191156248`, AliDDNS `44191156160`, AliyunDrive `44191156171`, and AdGuardHome `44191156036`. NVRAM `44191156088` remained in progress.
- The production-source XY-C1 run is on f9 source SHA `f9afd25e`, run `44190333512`; all pre-build gates passed but Build XY-C1 was still in progress, with TRX/artifact pending. The 0ab correction changes only CI assertions/docs, so XY-C1 is not retriggered by its path filter.
- This documentation-only commit corrects the exact handoff contract; the AI check must be rerun on this SHA. Hardware L4 remains NOT VERIFIED.

## 2026-11-18 — Repair PID 1 SSH lifecycle contract and restore rolling dev log

- Parent main SHA: `f9afd25e8957de96c5661f9c0d00c29cf5d22da3`; source change `fix(sshd): verify daemon exit with identity-bound stop` routes C-controlled SSH stop through `kill_services()`, with Dropbear-first/OpenSSH fallback selection.
- Exact f9 Process lifecycle `44190334132` and Process inventory/lifecycle regression `44190334289` passed, including SSH policy compile and source checker. PID 1 notification workflow `44190333392` compiled and ran all policy binaries, then failed its Python source contract because it still expected the pre-refactor `start_sshd()` to directly return `eval()`; update the contract to require captured launcher status, bounded RUNNING postcondition, and identity-bound `stop_sshd()`.
- AI handoff docs run `44190333813` failed because the parent commit replaced the 31-entry rolling `dev_log.md` with one entry. Restore the complete 31-entry file from parent `07b8266e` and prepend this checkpoint, retaining the >=20-entry contract.
- Exact 6a5 source/test SHA completed all gates including XY-C1 `44188951477` Build, TRX verification and artifact upload. On f9, XY-C1 `44190333512` remained at Build at last poll; AliyunDrive rerun `44190333972` completed success; AdGuardHome `44190334184` and NVRAM `44190334351` were in progress.
- This correction changes only PID1 checker expectations and restores development history/document accuracy. Exact-head PID1/AI/XY checks must pass on this correction. Physical-router L4 remains NOT VERIFIED.

## 2026-11-18 - Scope HTTPD checker ordering to the firewall rollback block

- Parent main SHA: 6a5f1477771ecb1f4aa612e59a12472937051e0f; changed scripts/check_modem_wan_start_status.py, SESSION_HANDOFF.md and REFACTOR_ROADMAP.md.
- The first checker adjustment 773642be removed the obsolete unconditional firewall-error return requirement, but its new order assertion accidentally selected the initial pre-launch httpd_started=0 assignment. Exact USB modem WDM/NDIS, Firewall Startup and XY-C1 jobs exposed this as line 635.
- Scope the expected order to the block beginning at firewall_status = restart_firewall();. The checker now ensures error capture -> stop gate -> locked stop -> fresh process scan -> ABSENT/uncertain state publication -> original errno/status return.
- Exact 6a5f1477 Firewall Startup 44188951449 and AI docs 44188951370 passed. Process 44188951409, USB modem WDM/NDIS 44188951386 and XY-C1 44188951477 were pending/in progress at last check.
- Frozen backup 72f4f157a084f8184b9597ca5bf334b8080e5807 unchanged; no XY-C1 L3 or physical-router L4 claim.

## 2026-11-18 — Verify SSH detached startup and fail-closed restart preflight

- Parent main SHA: `f1f3edd6961e2ea7f20b63da76f2304e0ab72db3`. On the parent HTTPD slice, exact-head HTTPD security `44186935262`, HTTPD lifecycle `44186935549`, and AI handoff `44186935541` passed. Process lifecycle `44186935557`, WAN route `44186935517`, NVRAM `44186935696`, and XY-C1 `44186935585` were still running at last inspection.
- `start_sshd()` previously trusted the detached OpenSSH/Dropbear wrapper status. It now uses the shared bounded startup policy (one-second initial wait, up to four process-inventory probes); only a confirmed RUNNING child returns success. UNKNOWN preserves the inventory error, continued ABSENT returns `ETIMEDOUT`, and launcher failures remain visible.
- `restart_sshd()` previously invoked the broad name-based stop script even when its initial process inventory was UNKNOWN. It now aborts before stop on UNKNOWN, skips unnecessary stop when already ABSENT, and only stops after a confirmed RUNNING snapshot; the post-stop scan must still confirm ABSENT before start.
- Extend the existing GNU89/ASan/UBSan SSH policy test and `check_sshd_firewall_status.py` source-order contract. Existing Process workflow compiles/runs the policy binary; XY-C1 tracks changed production `services.c`.
- Exact-head Process and XY-C1 checks for THIS source commit are pending. Parent CI is not a substitute for this SHA's firmware validation. No MIPS/uClibc L3 or physical-router L4 claim.

## 2026-11-18 — Withdraw HTTPD listener when firewall refresh fails

- Parent main SHA: `0a297f48326221209bdbdf57173db719cd3a79b8`; production path `trunk/user/rc/src/services.c`, policy `httpd_daemon_policy.h`; regression: GNU89 HTTPD daemon policy and existing HTTPD lifecycle source contract.
- `start_httpd_locked()` could publish `httpd_started=1`, then return a required public HTTP/HTTPS firewall refresh error while leaving the listener alive under potentially stale or incomplete rules.
- On refresh failure, invoke `stop_httpd_locked()` under the existing lifecycle lock, rescan process inventory, publish `httpd_started=0` only for confirmed PROCESS_PRESENCE_ABSENT, otherwise retain `1` and log uncertainty. Preserve original firewall errno and status.
- Add `httpd_firewall_failure_requires_stop()` and six GNU89 unit cases across zero/error and ABSENT/RUNNING/UNKNOWN/invalid states. Extend workflow assertions to require stop → inventory postcondition → state publication before returning the firewall error.
- Parent SSH SHA `36b2b171`: Process `44185018675` and WAN route `44185018710` passed. XY-C1 `44185018862` was still building at last observation. THIS source SHA requires fresh HTTPD/Process/XY-C1 verification; router hardware L4 remains NOT VERIFIED; frozen backup is `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## 2026-11-18 — Withdraw SSH listener when public firewall refresh fails

- Parent main SHA: `7ffa87073cb7a18fadd5c7dc0fdbdd794b272f20`; production paths `trunk/user/rc/src/services.c` and `sshd_daemon_policy.h`; regression: GNU89 SSH policy test plus `scripts/check_sshd_firewall_status.py`.
- `restart_sshd()` could start a listener after a process-state transition, call `restart_firewall()` for enabled public SSH, then return the refresh error without stopping the newly active listener. That could leave SSH reachable under an old or incomplete ruleset.
- Add `sshd_firewall_failure_requires_stop()`: nonzero refresh requires shutdown unless state is confirmed ABSENT. On failure invoke `stop_sshd()`, rescan the daemon, and treat only PROCESS_PRESENCE_ABSENT as confirmed withdrawn. RUNNING/UNKNOWN is logged as unresolved; original refresh errno is preserved.
- Add GNU89 `-Wall -Wextra -Werror` ASan/UBSan coverage for zero/error and ABSENT/RUNNING/UNKNOWN/invalid states; add a source-order contract; wire into Process CI and PR path filters.
- Prior 319fd181 passed exact-head XY-C1 Build/TRX/artifact. PPPoE-MWAN follow-ups through `7ffa8707` require exact-head validation. THIS SSH source commit needs fresh exact-head Process/XY-C1 checks; router L4 remains NOT VERIFIED; backup frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## 2026-11-18 — Correct PPPoE-MWAN source-checker newline delimiters

- Parent main SHA: c8b8ba8c107bebb2dd73f3390f2e0fa53f68a86e; path scripts/check_pppoe_mwan_status.py.
- The first detector gate attempt (ac2b1f6) failed its baseline route assertion because the new helper incorrectly set MWAN_FLUSH_FAILED on successful iptables deletions. c8b8ba8 now only poisons the phase for nonzero statuses.
- While preparing the exact rerun, inspect the Python contract source: the generated strings contained two backslashes, so Python searched for literal \n text rather than actual newline characters. Use chr(10) in both the helper-body and pre-policy boundary lookups.
- Parent 319fd181 XY-C1 run 44180704552 passed pre-build regressions, Build XY-C1, TRX verification and artifact upload. Current PPPoE detector changes still await new-SHA verification.

## 2026-11-18 — Only poison PPPoE-MWAN cleanup on nonzero status

- Parent main SHA: ac2b1f630e5bb0a51fd75aa6a7c090d2d7865c58; changed detect.sh and its Python source contract plus checkpoint docs.
- Exact ac2b1f6 WAN-route run 44183697784 and XY-C1 run 44183697726 both failed first at the pre-existing normal multipath default-route test. Root cause was the new cleanup recorder setting MWAN_FLUSH_FAILED unconditionally even for successful iptables -D (status 0).
- Guard the cleanup poison assignment with a nonzero status check; successful deletions leave cleanup state healthy, while snapshot, parse and deletion failures still block new rules/routes. Extend source checker to require this zero-status guard.
- The four new failure-injection branches were not reached in the failed run and remain unverified until exact-head rerun.
- Parent 319fd181 XY-C1 run 44180704552 did pass all pre-build checks, Build XY-C1, TRX verification and artifact upload; target L3 proven at that SHA. Router hardware L4 not run.

## 2026-11-18 — Prevent PPPoE-MWAN policy rebuild after failed cleanup

- Parent main SHA: 319fd18123df5e47929cbe1c27f2989fd129bef4; changed detect.sh, detect_status.sh, check_pppoe_mwan_status.py and XY-C1 workflow atomically with handoff/roadmap/log.
- The detector previously recorded iptables-save, malformed snapshot parsing and iptables -D failures but continued adding new mangle/NAT rules and policy/default routes. This could layer a new multi-WAN policy over stale or partially removed rules.
- A cleanup failure now poisons the previous-policy phase while all three independent cleanup tables are still attempted. The detector flushes route cache, preserves the first error, removes its owned lock via EXIT trap, and exits before any new mangle/NAT/policy/default route writes.
- Added shell failure-injection cases for failed mangle snapshot, failed NAT snapshot, rule-deletion failure and malformed saved rule; each asserts no new policy or route mutation and cache cleanup. XY-C1 now runs this detector suite before Build in addition to WAN-route CI.
- On parent 319fd181, WAN DHCP, Firewall Startup, Process, USB modem, NVRAM, AI docs, AdGuardHome, AliDDNS, AliyunDrive and infosvr exact-SHA Actions passed. Its XY-C1 run 44180704552 had pre-build regressions green and Build XY-C1 in progress; this new SHA requires its own WAN/XY verification. No hardware L4 claim.

## 2026-11-18 — Correct RAS startup aggregation count and WAN DHCP static-IPoE boundary

- Parent main SHA: `62fbefe3d562c332c9929b22f6d2d8ea078e1ade`; only regression contracts and handoff/log changed.
- Exact-head `659c0720` runs Process `44179526342`, Firewall Startup `44179526456`, USB modem `44179526516`, and XY-C1 `44179526334` advanced through the earlier source-boundary fixes and stopped at `scripts/check_modem_wan_start_status.py:315`: the checker still expected four `modem_start_status` aggregates. The current `start_wan()` path has five distinct results, including USB-net startup, RAS interface migration, RAS firewall rebuild, firewall-failure physical-WAN restoration, and RAS launch. The checker now expects five and explicitly verifies RAS restoration is followed by result aggregation.
- Exact-head `62fbefe3` WAN DHCP callback `44179778384` passed the APCLI call-boundary slice but failed later at embedded Python line 852 because its static-IPoE block still ended at the removed legacy iptables comment. That inline checker now ends at the actual `vlan_result = start_firewall_ex();` gate after static address configuration.
- On `659c0720`, AI handoff `44179526396`, AdGuardHome `44179526443`, infosvr `44179526331`, AliyunDrive `44179526451`, and AliDDNS `44179526391` passed. Process/Firewall/USB/XY failures were at the shared checker count, not production compile failures; XY-C1 Build/TRX/artifact were skipped.
- Exact `62fbefe3` AI handoff `44179778471` and AliyunDrive `44179778466` passed; WAN DHCP `44179778384` failed only at the second stale inline source boundary. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No target L3 or router L4 result claimed.

## 2026-11-18 — Replace the final stale AP-client boundary in WAN DHCP CI

- Parent main SHA: `659c0720a1d0baccdd5ec3644a9d2177b199bf69`; workflow source-contract correction only, no production C changes.
- After the common modem checker was advanced to the current IPoE firewall gate in `659c0720`, one duplicated stale AP-client comment remained in `.github/workflows/wan-dhcp-callback-stability.yml`. Its physical-interface setup slice now ends at the real `vlan_result = reconnect_apcli(wan_ifname, 0);` caller.
- The obsolete marker was previously exposed by the exact `8e9ea213` WAN DHCP callback run `44177798012`; this workflow was not triggered by `659c0720`, so no result on the newer source SHA is claimed yet.
- This commit must trigger and validate WAN DHCP callback CI. Other exact-head `659c0720` workflows were still in progress at handoff; inspect their corresponding run IDs before recording results. No MIPS build or router L4 claim. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## 2026-11-18 05:32 CST — Align modem checker with the new IPoE firewall gate boundary

- Parent main SHA: `ece8011f4cd14ed37e24ab6a95d2b847196d6d0a`; path: `scripts/check_modem_wan_start_status.py`.
- Exact-head ece8011 run failures: Firewall Startup `44178773543`, Process `44178773567`, and XY-C1 `44178773562`. Their GNU89 service status binary and earlier firewall-specific regression passed; all three stopped in the later modem checker at line 304 because it searched for the removed `/* re-build iptables rules (final stage for IPoE static) */` marker.
- Replace the static-address validation end marker with the current `/* Rebuild required firewall policy before EAPOL, DHCP or route activation. */` comment. This preserves the checker’s intended scope (address configuration up to firewall stage) rather than weakening assertions.
- Exact-head ece8011: WAN route `44178774262`, Wireless radio `44178774053`, AI handoff `44178773961`, AdGuardHome `44178773571`, AliyunDrive `44178773589`, infosvr `44178773674`, and NVRAM `44178774246` PASS. XY-C1 Build/TRX/artifact remained skipped before this correction; no L3 claim.

## 2026-11-18 05:16 CST — Repair exact-head WAN and APCLI source-contract drift

- Parent main SHA: `8e9ea2131d7be59833756c15d03cf58ba288085c`; paths: `scripts/check_modem_wan_start_status.py`, `scripts/check_wan_initial_mtu_status.py`, and `.github/workflows/wireless-radio-state.yml`.
- Exact-head Firewall Startup `44177798054` failed after all firewall policy binaries and its firewall caller checker passed; the later modem checker still searched for the old `/* perform ApCli reconnect */` comment. Process `44177797717` compiled and ran the updated GNU89 service-start policy regression successfully, then stopped at the same checker boundary.
- WAN route `44177797840` passed its earlier firewall result propagation contract, then its initial MTU checker searched for the former IPoE firewall comment. Update both checkers to current source boundaries.
- Wireless radio `44177798027` passed its C/policy, bridge and guest-isolation tests but its APCLI source checker counted both command and log strings. Count the four actual `doSystem(...ApCliAutoConnect...)` invocations. Also add the missing `net_wan.c` path to the PR filter and checker list.
- AI handoff `44177797925` PASS; XY-C1 `44177798165` was in progress at last poll. No code-level failure was observed in the preceding firewall or service-status regression steps. Exact-head reruns pending; MIPS build/hardware L4 not yet claimed.

## 2026-11-18 04:48 CST — Fail closed WAN protocol startup on firewall rebuild errors

- Parent main SHA: `be0f5b56efa203fc802334e08b846381e9a3f04a`; source paths: `trunk/user/rc/src/net_wan.c`, `trunk/user/rc/src/service_start_status.h`, `trunk/user/rc/src/tests/service_start_status.c`, and `scripts/check_wan_firewall_status.py`.
- `start_wan()` previously accumulated `start_firewall_ex()` errors but continued to launch USB RAS/PPP clients and IPoE EAPOL/DHCP/route setup. The firewall API reports nonzero when the requested ruleset or configured extension hooks/snapshot cleanup are incomplete; reporting the error without gating protocol launch could expose WAN traffic under stale or partial policy.
- Added `rc_wan_firewall_start_allows_protocol()` and regression cases for zero, negative, EIO, EBUSY and EACCES results. The firewall source checker now asserts all three gate-before-launch paths and recovery/cleanup ordering.
- USB RAS and PPP launch are skipped on firewall failure; physical-WAN policy restoration is retried, with interface-down fallback if restoration fails. IPoE EAPOL/DHCP/route/IPv6 setup is skipped; static address cleanup is attempted and interface-down is the fallback if that clear fails. First-error aggregation is preserved.
- Standalone GNU89 policy harness with `-Wall -Wextra -Werror -fsanitize=address,undefined`: PASS. Full repository C regression binary, full Python source checker and MIPS XY-C1 build were not run locally. Exact-head WAN-route/XY-C1/AI handoff Actions remain pending/not verified; no target L3 or L4 claim. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## 2026-11-17 — Propagate AP-client reconnect driver failures into WAN startup

- `reconnect_apcli()` in `net_wifi.c` was `void` and discarded all `doSystem()` results for APCLI `ApCliAutoConnect` and forced `ApCliEnable=0/1`. Failed driver commands could leave WISP unassociated while `start_wan()` still returned success.
- Return normalized status using `wireless_radio_restart_status()`; retain the first failure while still attempting forced re-enable after disable error. `start_wan()` aggregates the error and continues other WAN setup; manual WISP reassociation logs nonzero status. No-op cases remain successful.
- Extend GNU89 wireless policy regression and exact source contract across `net_wifi.c`, `rc.h`, and `net_wan.c`; path filters now cover the WAN caller. Exact-head Wireless radio Actions pending; local host compile not available in this runtime.
- Preceding verifications: Samba `44173796606`, Process `44174094871`, NVRAM `44174094746`, infosvr `44174094855`, AI docs `44174094903`, AliDDNS `44174094743`, AdGuardHome `44174094915`, AliyunDrive `44174094822` PASS.

## 2026-11-17 — Correct Process lifecycle syslogd/klogd source-boundary escapes

- Process exact `222cbfba37ef2e4c17ae13bd9275ed648c6ab407` run `44173365883` reached the syslogd/klogd source-contract section after prior C/RC integration checks passed. Its Python checker split on `"int\\\\nstart_syslogd(void)"` and `"\\\\n}"`, which searched literal backslash-n rather than C newlines, causing `IndexError`.
- Correct both syslogd and klogd delimiters to Python newline escapes. No production code or test binary changes in this correction. Current-head Process rerun `44173796903` was in progress at last poll.
- Samba exact run `44173796606` on current ancestor `6174c78` passed after the preceding whitespace-insensitive checker correction. Current exact NVRAM `44173796346` remained pending.

## 2026-11-18 — Normalize Samba WINS restart source-contract spacing

- The exact Samba run `44172985076` on `2e6974211a9e3a487913076b84dfe89eb9817b0d` passed its GNU89/Werror/ASan/UBSan C binaries, including the WINS restoration decision matrix, but the final integration checker failed at a one-line spelling assertion for a deliberately wrapped helper call.
- Update the checker to remove whitespace from the extracted `restart_smbd()` body before matching the complete argument sequence. This addresses the checker false negative; no production C change is needed.
- Exact rerun of the Samba workflow is pending. NFS `44172985066`, AI docs `44172984806`, and AdGuardHome `44172984782` passed on that source SHA.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-18 — Verify detached syslogd and klogd children after launcher success

- Parent main SHA: `2e6974211a9e3a487913076b84dfe89eb9817b0d`; THIS COMMIT modifies RC startup, Process source integration contracts/PR filters and the live handoff/roadmap.
- BusyBox syslogd forks by default unless `-n` is supplied; RC's argv has no `-n`. The parent returns after `bb_daemonize_or_rexec()`; the detached child writes its PID file and runs `do_syslogd()`. RC `_eval()` waits only for the launcher.
- BusyBox klogd also daemonizes by default unless foreground mode is requested; its detached child opens kernel log and enters the read loop while RC `eval()` only sees the launcher.
- Add optional-daemon tri-state preflight and bounded child checks. Already-running is success; UNKNOWN fails closed; nonzero launcher status is preserved; only RUNNING after a 1s initial delay and four probes returns success; UNKNOWN post-scan preserves errno; ABSENT reports `ETIMEDOUT`.
- Reuse GNU89-tested `process_presence_start_postcondition()`. Source checker verifies BusyBox daemonization plus RC launch/preflight/poll/result order; Process workflow push/PR filters cover the BusyBox sources.
- Parent `db362b5` Process fail-closed `44171722700` PASS; NVRAM `44171722663` pending. Parent `9be5d29` HTTPD `44171043495` PASS; XY-C1 `44171043411` pending. Exact-head Process/XY-C1 for THIS slice pending; no hardware L4 claim.

## 2026-11-18 — Restore WINS after restarting disabled Samba shares

- Review found that `restart_smbd()` force-stops `nmbd`, then selects `run_samba()` solely from mounted-storage presence. `run_samba()` returns success immediately if `enable_samba=0` or `st_samba_mode=0`; with mounted storage and `wins_enable=1`, the restart silently left the independent WINS daemon down.
- Added a fail-closed restart-action policy that selects Samba only when shares are enabled and storage is mounted; otherwise it restores WINS via `start_wins()` (which independently respects `wins_enable`). Invalid enablement or mount-state inputs abort.
- Extended `samba_process_policy.c` with GNU89 cases covering Samba enabled/disabled × mounted/empty, plus negative/invalid inventory and invalid enablement. Updated the Samba workflow’s embedded production source contract to enforce the new branch and unit-test wiring.
- Local full Samba process-policy test compiled with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan: PASS. Local targeted decision-policy matrix also PASS.
- Exact-head Samba stability and XY-C1 runs for this source commit are pending; no target build or physical-router L4 result is claimed yet. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Fail closed on IPTV DHCP NAK/expiry/release cleanup

- The first IPTV DHCP deconfig implementation restored the old address when route deletion failed. Review showed this could resurrect a lease already revoked by NAK, expiry or release. The policy now never restores the address and attempts route cleanup even when address clear fails; empty NVRAM address publishes only when both complete. BusyBox retries the retained lease/route identity after an error.
- Ensure igmpproxy teardown still runs after either cleanup failure and aggregate the first interface/route/multicast error. Extend GNU89 regression with simultaneous failure and route-retry cases; update both WAN DHCP and HTTPD workflow contracts for the transactional dispatch and error handling.
- Exact product-source WAN DHCP run 44170820583 PASS, including GNU89 -Wall -Wextra -Werror with ASan/UBSan and semantic source contracts. HTTPD checker run 44171043495 on 9be5d29 PASS after fixing the escaped-newline endpoint. Exact current-source XY-C1 44171043411 remains IN_PROGRESS; Build/TRX/artifact not yet verified.
- Concurrent rstats/LLTD source `92d95b8` Process run 44169139271 PASS; XY-C1 44171043411 pending. Backup branch remains frozen.

## 2026-11-18 — Verify detached rstats and LLTD startup children

- Parent main SHA: 9e6d19f6ca53cbfeeda33e109abf28b6251f4000; this commit modifies RC startup, Process integration contracts/workflow filters and handoff/roadmap.
- rstats_main() daemonizes before alloc_rstats() and process_pidfile_write(RSTATS_PID_FILE, "rstats"); the child can exit on either failure while the parent launcher has already returned zero.
- LLTD main() calls osl_become_daemon(); that implementation double-forks and calls setsid(). The detached child writes its interface PID file, drops privileges and enters event_mainloop() only after the parent exits.
- start_rstats() and start_lltd() now retain their disabled sentinel, preflight tri-state presence, treat already-running as success, fail closed on UNKNOWN, and verify post-launch RUNNING with the shared 1s + four-probe bounded policy. ABSENT after attempts reports ETIMEDOUT; launcher errors are preserved.
- Reuse the GNU89-tested process_presence_start_postcondition() policy. The Process checker validates startup ordering and daemonization source; its PR path filters include all referenced LLTD/rstats files.
- Exact parent 6b5f551 DDNSTO, Process, NVRAM, AliyunDrive, infosvr, AliDDNS, AdGuardHome and AI-handoff runs passed. Telnetd product-source 989409e XY-C1 Build/TRX/artifact 44164998281 passed. Latest IPTV DHCP source 32622d8 checker follow-up 9e6d19f had WAN DHCP run 44168515283 in progress at last observation. THIS COMMIT exact-head Process/XY-C1 pending; no hardware L4 claimed.

## 2026-11-18 04:32 CST — Fix whitespace-sensitive IPTV DHCP checker assertion

- Exact source commit 32622d8549a0b5c022bfb765b189e7c3214a801c WAN DHCP run 44168062187 compiled and ran dhcp_lease_policy.c under GNU89, -Wall -Wextra -Werror, AddressSanitizer and UBSan; all five new IPTV deconfig transaction cases passed. Earlier DHCP policy steps also passed.
- Embedded Python caller contract then failed at line 171: it compared a multiline snprintf() call against a single-line string. Source formatting is valid; normalize whitespace before checking both old-address snapshots. No production or C-test change required for this correction.
- Exact 32622 WAN route run 44168062198 and AI docs run 44168062208 passed. XY-C1 44168062063, Firewall 44168062335 and NVRAM 44168062259 were still in progress at observation. Telnetd source 989409e XY-C1 44164998281 completed Build/TRX/artifact SUCCESS.

## 2026-11-18 04:25 CST — Clear stale IPTV DHCP address on lease deconfig

- Exact 6b5f5514f412e9a9ba173cc2148b5945ea082df3 DDNSTO 44166279131 passed shell syntax, full lifecycle regression and Python source contract. Process 44166279162, Process inventory 44166279188, AliDDNS 44166279025, AI docs 44166279079, AdGuardHome 44166279220, NVRAM 44166278930, infosvr 44166279207, and AliyunDrive 44166279170 also passed.
- Found an IPTV DHCP deconfig defect: BusyBox invokes the script for NAK with an active lease, lease expiry and explicit release, but udhcpc_viptv_main(deconfig) called only manv_route_deconfig(). Routes/gateway state were removed without clearing the interface address or manv_ipaddr.
- Implement failure-aware clear-address → route-removal transaction. Restore the previous address if either step fails; publish manv_ipaddr=0.0.0.0 only after both succeed; then stop igmpproxy and propagate its status. Extend dhcp_lease_policy.h, GNU89 fault-injection tests and WAN-DHCP workflow source assertions. BusyBox deconfig retry behavior is unchanged.
- Local host compile/run was NOT performed. Exact-head WAN-DHCP callback policy/source contract and XY-C1 are pending after this commit.

## 2026-11-18 04:05 CST — Make the DDNSTO UNKNOWN-startup regression deterministic

- Exact-head DDNSTO push run `44163173235` on `34c02687139afc6db6484c2eb888cca2f8f01612` passed shell syntax, then failed at `UNKNOWN startup inventory was incorrectly retried`; the semantic source-contract step was skipped. The background mock could update its state after the first post-launch `pidof`, allowing a legitimate ABSENT observation to cause a retry before UNKNOWN was published.
- Make only the test fixture deterministic: count pidof calls for this case, let the pre-launch stop inventory observe ABSENT, and force the first post-launch inventory to return UNKNOWN. Wait boundedly for the detached mock to publish its state; continue asserting that the production wait path did not sleep/retry UNKNOWN.
- No production DDNSTO source changed. Local host shell execution was NOT RUN in this environment; exact-head lifecycle + Python source-contract rerun is pending. On current parent `b00ff490`, Process lifecycle `44165568972`, infosvr `44165568767`, AliDDNS `44165568791`, AdGuardHome `44165569096`, and AI docs `44165568764` passed. XY-C1 `44164998281` for product-source `989409e` was still IN_PROGRESS; no current-source L3/L4 claim.

## 2026-11-18 — Fix stale Process regression output marker

- Parent main SHA: `989409e1a02a92ea5018f3550d52d75c33b028f8`.
- Exact Process lifecycle run `44164998210` compiled and ran the expanded GNU89 process-presence policy binary successfully. Its shared integration checker passed prior contracts, then failed only because its legacy assertion at line 1015 expected `Process presence start/notify tri-state policy: PASS`, while the test now prints `Process presence start/notify and detached-daemon postcondition policy: PASS`.
- Correct the exact expected test output. This follow-up changes checker/handoff/roadmap only; no production source changes. Re-run the Process checker on the exact SHA.

## 2026-11-18 — Verify BusyBox Telnetd after detached launch

- Parent main SHA: `34c02687139afc6db6484c2eb888cca2f8f01612`; THIS COMMIT updates RC startup, shared process-presence policy/test, the Process integration checker/workflow path filter, and the handoff/roadmap.
- BusyBox `telnetd_main()` calls `bb_daemonize_or_rexec()` unless `-F` is specified. RC calls `eval("telnetd")` without foreground mode; the detached child binds/listens only after daemonization. A port-in-use or post-fork setup failure can leave the parent launcher successful while Telnetd has exited.
- Add bounded child-presence verification after launch: one-second initial wait, four probes, one-second retry. Only RUNNING succeeds; UNKNOWN fails closed with the procfs scan errno; ABSENT after the probes reports `ETIMEDOUT`; launcher failures are preserved.
- Extend the GNU89 process-presence test for RUNNING/ABSENT/UNKNOWN/invalid and launcher-status cases. Update both Telnetd checker copies for BusyBox daemonize-before-bind and the launch/wait/inventory/postcondition sequence. Add BusyBox `telnetd.c` to the Process workflow push/PR filters.
- Parent exact-head Networkmap `44163174212`, Process `44163173313`, HTTPD daemon `44163173359`, infosvr `44163173874`, SkipDBv2 `44163173539`, and USB storage `44163176401` passed on `34c0268`; XY-C1 `44163173638` and NVRAM `44163173644` were pending. THIS COMMIT's validation is pending; no L4 hardware result is claimed.

## 2026-11-17 — Verify the detached networkmap child after daemonizing launch

- `trunk/user/networkmap/src/networkmap.c::main()` binds the ARP socket, installs signal handlers, calls `daemon(0, 0)`, writes its pidfile and starts the scan loop (or waits five seconds for `-w`). `start_networkmap()` previously returned the direct `eval()` result without confirming that the detached child survived.
- Add a bounded process-inventory postcondition after a zero launcher status: one-second initial delay, up to four probes, one-second retry spacing. Report success only after `networkmap` is RUNNING; UNKNOWN inventory fails closed with its scan error and a child that never appears returns `ETIMEDOUT`. Existing pre-launch tri-state absence gate, already-running no-op and first-call `-w` behavior are preserved.
- Add GNU89 policy tests for RUNNING/ABSENT/UNKNOWN/invalid states and preservation of nonzero launch status; update the networkmap source checker to enforce launch → wait → post-launch inventory order. The existing networkmap workflow tracks every changed source/test/checker path.
- Parent `9f9439ed` infosvr, process inventory/lifecycle, AI, networkmap signal, and 20 other current-head contracts passed. Parent storage, NVRAM, SkipDB and XY-C1 were still running at last poll; new exact-head networkmap check pending.
## 2026-11-17 — Verify the detached infosvr child after daemonizing launch

- Repository inspection confirmed `trunk/user/infosvr/src/infosvr.c::main()` binds the UDP socket and installs SIGTERM handling, then calls `daemon(0, 0)`, writes `/var/run/infosvr.pid` and enters its long-lived packet loop. `eval("/usr/sbin/infosvr", IFNAME_BR)` only reports the launcher/daemonizing parent status.
- `start_infosvr()` previously returned success as soon as `eval()` returned zero; a detached child that exited before the first packet loop could therefore be reported as successfully started. Add bounded process-inventory verification (initial 1-second wait, up to four probes) and report success only after `infosvr` is confirmed RUNNING. UNKNOWN inventory fails closed with the actual scan error; a missing child returns `ETIMEDOUT`.
- Preserve the existing pre-launch tri-state gate and disabled-service no-op. Add GNU89 policy tests for RUNNING/ABSENT/UNKNOWN/invalid state and launcher error preservation; extend the infosvr source checker for launch → wait → inventory ordering. Existing workflow already compiles the policy test and runs the checker.
- Parent exact-head storage, Process lifecycle/inventory, AliDDNS, AliyunDrive, infosvr, AI, AdGuardHome and NVRAM checks all passed on `dec7924b`. New exact-head infosvr CI pending. XY-C1 L3 Build/TRX/artifact is green for firmware source `fb99c2a`; no hardware L4 claim.
## 2026-11-17 — Make the swap staging source contract whitespace-tolerant

- Exact-head storage run `44159705744` on `9cf66b7bec1a7befd0408ef8a9c2aec9b44563b3` completed shell/ash, MTD, storage C policy tests including the new `/proc/swaps` parser cases, and earlier lifecycle tests successfully. It then failed in the final production caller graph at the source assertion for `storage_ejected_inventory_complete(read_error, close_error, parse_error)`.
- The production call is line-wrapped after the second argument; the checker compared the full call as one raw substring. Normalize whitespace in `swap_stage` before matching. This changes only the test harness, not production C code.
- Parent HTTPD, AI handoff, Process lifecycle/inventory, AdGuardHome, AliDDNS, AliyunDrive and infosvr passed. Parent NVRAM was still in progress at last poll.
## 2026-11-17 — Fix newline escapes in raw swap inventory tests

- Exact-head HTTPD run `44159082928` on `fb99c2a7dbf4c1576e4d863d5d451f18633fefd3` passed the previous HTTPD/NVRAM/wireless/storage source checks and then failed at the newly added `storage_ejected_parse_swaps_header()` policy assertion.
- Inspection of the actual C source showed the `/proc/swaps` header and row test literals contained two backslashes before `n`; the compiled string therefore ended in literal `\n` rather than a newline and correctly failed `storage_ejected_line_is_complete()`. Normalize only the new swap fixtures to single C newline escapes.
- Move the test's PASS message from before the new assertions to after them so the output cannot report success before all cases run. No production source changed in this test repair.
- Parent exact-head AliDDNS, AliyunDrive, Firewall, vsftpd, AI docs, USB modem, NFS, Samba, infosvr and AdGuardHome passed; storage, Process, NVRAM and XY-C1 were still in progress at last poll.
## 2026-11-17 — Retry raw swap-only devices during physical eject

- `umount_ejected_locked()` previously returned success immediately if the complete `/proc/mounts` inventory contained no removable storage mounts. Its swapoff retry loop only visited stale mounted partitions. A raw SD/MMC swap partition can exist without a filesystem mount; if the mdev partition-remove callback's first swapoff attempt failed, the subsequent whole-device notification had no stale mount to drive a deferred retry.
- Stage and validate `/proc/swaps` independently from `/proc/mounts`, then compare raw removable swap device names against a complete `/proc/partitions` snapshot. Retry swapoff for each missing `/dev/sd*`/`/dev/mmcblk*` device. A failed/UNKNOWN attempt returns failure even with no mountpoints, so `umount_ejected()` schedules the existing `usb_unplug_ms` retry. Non-removable swap files are ignored by the raw-device path parser and continue through the existing Optware mount teardown hook.
- Add GNU89 tests for complete five-column swap headers, exact five-field rows, truncated/extra-field rejection, SD/MMC whole-disk and partition identity, non-removable swapfiles and malformed paths. Extend storage source contracts for inventory-before-no-work and swapoff-before-stop/unmount.
- Parent `12f4ba4a` AI docs, AdGuardHome, infosvr, Process inventory, AliDDNS and AliyunDrive checks passed; NVRAM was still in progress. No MIPS/uClibc or physical unplug result is claimed.
## 2026-11-17 — Restore the live module dashboard after accidental roadmap truncation

- Exact-head AI handoff run `44156954920` on `5bdf59f0bcceb7802bd94309bd0bdb5b467381b7` failed because `REFACTOR_ROADMAP.md` lacked the required `## Whole-repository module status` dashboard. The job log identified this single invariant; other document checks passed.
- The live roadmap had been reduced to a 996-byte checkpoint-only note, losing the 51,338-byte module dashboard, priority queue and verification matrix. Restore the full version from Git history at `e90549004908d388123e1807abf0ddeb4a6a11fd`, update the current checkpoint, and keep the historical dashboard content intact.
- Parent WAN teardown `44156954967`, HTTPD `44156954859`, Process lifecycle/inventory `44156955420`/`44156955350`, and NVRAM `44156954872` passed. Exact parent XY-C1 `44156955220` was still building; TRX/artifact pending.
## 2026-11-17 — Stop MSD Lite during WAN teardown

- `start_udpxy()` can launch the optional `msd_lite` proxy, and the shared proxy stop list already includes it. The independent `stop_wan_locked()` inventory omitted `msd_lite`, so a WAN-down/reconnect teardown could leave the daemon alive with stale WAN routing and interface state.
- Add `msd_lite` to the WAN daemon stop inventory and extend the WAN teardown source contract to require all four WAN proxy daemons, including MSD Lite.
- Add `hotplug_stor.c` to the XY-C1 workflow path filters. The tri-state SD/MMC source change had only storage/HTTPD coverage; adding the source filter ensures future hotplug edits also trigger firmware validation. This commit changes `net_wan.c`, so XY-C1 is also triggered on this exact SHA.
- Exact-head WAN, XY-C1 Build/TRX/artifact, Process, AI handoff and NVRAM checks: pending. No local MIPS/uClibc build or hardware result is claimed.

