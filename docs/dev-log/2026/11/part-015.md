## 2026-11-17 — AliDDNS WAN IPv6 source correction

- Implementation commit: `40fb7dfe716f1003c96b4b61453017daef5495d3`.
- Corrected the stock IPv6 discovery path from LAN bridge `br0` to the configured `wan0_ifname6` interface maintained by RC's WAN6 lifecycle (native PPP and `sit0` tunnel cases).
- Rejects unsafe/empty/overlength interface names, non-public address scopes, ambiguous multiple global addresses, and failed interface queries. Does not fall back to a LAN address.
- Added `aliddns_ipv6_lookup.sh` and wired it into `check_aliddns_api_transport.py` and `aliddns-api-transport.yml`.
- Workflow indentation follow-up: `29f9b24d3b7ad8fc5f0dd4e856e0d3d06687599a`. The earlier malformed run on the implementation commit did not execute jobs; exact-HEAD status is recorded in `SESSION_HANDOFF.md`.
- Main at this log entry: `07aecfc4c03368c7e05efecb5977916953b854d8`; backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
## 2026-11-17 — Preserve LAN restart bridge, PHY and WAN restart results

- Parent main SHA: `a0a5c7b5330d601032e83f1800fe7873612cc013`; result: THIS COMMIT.
- Review confirmed the boot path already aggregates `init_bridge()`, `start_lan()` and PID 1 status. The remaining concrete gap was in `full_restart_lan()`: it discarded status-returning STP, bridge forward-delay, LAN PHY power-down/up, STP/forward-delay restoration and nested `full_restart_wan()` calls.
- Routed each result through `lan_restart_record_status()`, preserving first-error semantics while still attempting independent recovery operations and VPN startup.
- Expanded `service_start_status.c` to cover a failure through the complete restart recovery sequence and verify every later operation is still attempted. Strengthened the Process workflow semantic contract to verify all production result captures and call ordering.
- Parent exact-head validations all passed: WAN route `44051968487`, WAN DHCP `44051968728`, Process inventory `44051968717`, Process fail-closed `44051968786`, NVRAM `44051968689`, and XY-C1 `44051968495`; XY-C1 Build, TRX and artifact upload succeeded. This commit's exact-head checks are pending.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains NOT VERIFIED.

---



## 2026-11-17 — Scope LAN restart order assertion to the nested WAN restart

- Parent main SHA: `0b2168e17833d5ae8e4223f34fcecd0e7a5df82f`; result: THIS COMMIT.
- The production fix and expanded GNU89/ASan/UBSan LAN restart regression passed in Process run `44054425453`; the final source-contract step alone failed because a raw `restart.index("start_vpn_server()")` matched an earlier DNSMasq-lock recovery path.
- Scope the VPN start index to the position after the nested `full_restart_wan()` call, preserving the intended order check without changing production code.
- On parent `0b2168e1`, WAN route `44054425489`, WAN DHCP `44054425743`, Process inventory `44054425531`, AI docs `44054425761`, AdGuardHome `44054425537`, infosvr `44054425744`, detect_link `44054425584`, and detect_internet `44054425871` passed; XY-C1 `44054425508` and NVRAM `44054425712` remained in progress at checkpoint. Fresh exact-head validation is required.
- Frozen backup unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Verify LAN DHCP event-handler symlink before launching udhcpc

- Parent main SHA: `89aff3132fd80f3853f38315112811ca5d72fb67`; result: THIS COMMIT.
- `start_lan()` previously ignored the result of `symlink("/sbin/rc", SCRIPT_UDHCPC_LAN)` and launched `udhcpc` regardless. A failed link creation or an unrelated pre-existing path could leave bound/renew/deconfig events without the intended RC dispatcher; an attacker-controlled script at the fixed /tmp name could also be invoked as root.
- Added an injected helper that creates the link or accepts `EEXIST` only when `readlink()` returns the exact `/sbin/rc` target. Wrong-target symlinks, non-symlink collisions, permission errors and readback errors are returned as positive errno-style status. The existing path is never unlinked or overwritten.
- AP-mode DHCP launch is now gated on successful link verification. A failure is logged and merged into `start_lan()`'s first-error status while later independent LAN configuration continues. Correct existing links remain idempotent across LAN restarts.
- Added GNU89/Werror/ASan/UBSan policy tests for link creation, exact-target idempotency, wrong target, non-symlink collision, creation/readback failure, invalid arguments and overlong targets. The Process workflow also asserts production gating/order.
- Traced the neighboring AP hostname path: initial `create_hosts_lan()` failure is retried by `prepare_dns_dhcpd()` via `update_hosts_ap()`; DNSMasq publication/start is refused on failure, and the AP DHCPD notification also gates restart. No defect was confirmed there.
- Local GNU89/Werror/ASan/UBSan symlink-policy regression PASS. Exact d0 Process lifecycle, AI handoff, NVRAM and XY-C1 Build/TRX/artifact all passed; the new commit's Actions are pending. Physical-router L4 NOT VERIFIED; frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Propagate guest Wi-Fi isolation and bridge-operation failures

- Parent main SHA: `d0ea67338241d47223dbf2ae434411dfeb53f242`; result: THIS COMMIT.
- Guest Wi-Fi control previously ignored `br_add_del_if()`, multicast-to-unicast bridge-port setup, `brport_set_param_int()`, ebtables flush/delete/rule installation, and ebtables module load/unload results. A guest interface could therefore be reported enabled despite an incomplete LAN isolation policy; PID-1 also ignored all six guest-control notification results.
- Added first-error/failure-count policy and host regression. The AP ebtables rule builder and cleanup now record each command; filter modules are retained if flush/delete fails. Bridge-port writes are attempted only for interfaces currently up, avoiding failures caused by disabled guest interfaces.
- `restart_guest_lan_isolation()` now returns a negative failure status. Initial bridge setup records it; full radio restarts return it; guest-control APIs preserve the existing 0/1 unchanged/changed success convention but return negative on bridge/multicast/isolation failure; PID-1 notification handlers retain those failures.
- Parent d0 exact-head Actions were 15/15 SUCCESS: Process lifecycle `44056030149`, NVRAM `44056029909`, AI handoff `44056030069`, and XY-C1 `44056030091` all passed. XY-C1 completed Build, TRX verification and artifact upload. New exact-head checks are pending after push.
- Host unit test and source integration checker are included. This does not yet fail-close an already enabled guest interface after isolation failure; next task is safe interface rollback. Target MIPS/uClibc, DBDC, RT3352 MII/iNIC and physical-router L4 remain NOT VERIFIED.

---

## 2026-11-17 — Align wireless-radio regression with fail-closed guest-control status flow

- Parent main SHA: `d3be26d0590ae05c20422db624d23868809c67b7`; result: THIS COMMIT.
- Exact-d3 Process lifecycle `44060466306` passed, including the guest-isolation GNU89/Werror/ASan/UBSan policy unit and the integration checker. AI handoff `44060466402` passed.
- Exact-d3 wireless-radio run `44060466245` failed at its inline source assertion, not at compilation or the existing radio/bridge unit regressions. The stale assertion required `return 0;` after an ifconfig failure. Guest-control now records the failure and jumps to its cleanup/isolation label before returning a negative status; update the checker to verify that ordering and status record instead.
- XY-C1 exact-d3 run `44060466380` was still building at checkpoint; do not claim its TRX/artifact checks yet.
- Next: implement rollback of a newly enabled guest AP if bridge, M2U or isolation policy setup fails, without taking down a guest interface that was already up. Handle RT3352 MII/iNIC ownership separately.

---

## 2026-11-17 — Update HTTPD security source contract for wireless restart failure propagation

- Parent main SHA: `56fb66fb68ed46bd8389c2dbdd4009243023f52c`; result: THIS COMMIT.
- Exact-d3 wireless-radio regression was repaired by updating its stale interface-failure assertion; exact follow-up run `44061356170` passed. Exact-56fb Process lifecycle run `44061356114` passed, including guest-isolation unit and source-contract checks.
- Exact-d3 HTTPD security run `44060465977` failed in its final static source checker: it still required `return 0;` from `restart_wifi_wl/rt`. The guest-isolation change correctly returns `isolation_status` to propagate policy errors; update the HTTPD checker to require that result instead. Prior HTTPD C and Ralink atomic-publication tests passed.
- Exact-d3 XY-C1 Build was still running at the last checkpoint; no TRX/artifact result recorded here. This commit's exact-head Actions pending.
- Next guest-Wi-Fi task: implement rollback of only newly enabled guest AP interfaces when bridge, multicast or isolation configuration fails; preserve pre-existing interfaces and account for RT3352 MII/iNIC VLAN bridge membership.

---

## 2026-11-17 — Fail closed when guest AP setup or isolation rollback fails

- Parent main SHA: `66b3354334d2d1a1e8542a2fd6564b67655d4260`; result: THIS COMMIT.
- Guest AP control previously returned an error for bridge, multicast-to-unicast or isolation failures but could leave a newly enabled interface active. Guest-disable also skipped bridge detach when ifconfig-down failed.
- Added a callback-driven rollback policy that always attempts interface-down, bridge detach and final down-state verification while preserving the first failure. Regression cases prove all cleanup operations run even when an earlier rollback stage fails.
- Guest-control rollback is limited to interfaces confirmed down before this activation attempt; an already-up guest interface is preserved. Rollback detaches the correct bridge port, including RT3352 MII router mode's iNIC guest VLAN rather than the wireless VIF. Failed guest-disable still attempts bridge detach.
- Full radio restart paths also roll back guest interfaces newly recreated by that restart when guest isolation reapply fails. DBDC peer rollback is limited to a peer radio restarted by the same request.
- Process and wireless-radio workflows compile the GNU89/Werror/ASan/UBSan rollback tests and check production rollback ordering. Exact-head Actions for THIS COMMIT are pending; no MIPS/uClibc target build or physical-router L4 result is claimed.
- Parent exact-head Process `44061735204` and HTTPD `44061735243` passed; XY-C1 `44061735229` and NVRAM `44061735223` were still running. Wireless-radio regression `44061356170` passed on source parent `56fb66fb68ed46bd8389c2dbdd4009243023f52c`.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Fix wireless-radio regression workflow shell continuation

- Parent main SHA: `04b98092eb68bc0c363462859a687fa235027042`; result: THIS COMMIT.
- Exact-head Wireless radio state run `44064716798` passed the existing wireless and bridge policy unit tests, then failed before compiling the guest rollback unit because the inserted three-line compiler command had two literal trailing backslashes. The shell treated a standalone backslash as an input file (`ld: cannot find \\\\ `).
- Corrected the three compiler continuations to one literal backslash; no production source change is required for this CI failure.
- On exact `04b98092`, Process lifecycle `44064716995`, HTTPD `44064716837`, AdGuardHome `44064716880`, AliDDNS `44064716827`, infosvr `44064716822`, iNIC `44064716795`, AliyunDrive `44064716760`, AI handoff `44064716914`, and watchdog `44064716964` passed. XY-C1 `44064716886` and NVRAM `44064716848` remained in progress at last poll.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Propagate guest Wi-Fi CLI failures to the caller

- Parent main SHA: `5d7461e85ade47a6305b998a14657f12dd15cd63`; result: THIS COMMIT.
- Source audit found that the four `radio2_guest_enable/disable` and `radio5_guest_enable/disable` CLI branches discarded the return from `manual_change_guest_*`. Those functions already return a negative value on guest AP/isolation failure, but `main()` initialized `ret = 0` and therefore silently reported success to scripts and shell callers.
- Each branch now maps negative control status to `-1`, while successful unchanged (`0`) and changed (`1`) results both map to CLI success. Added integration assertions for all four branches and the final `return ret;`.
- Exact-head on parent `aec5cc95`: Wireless radio state `44065308208`, Process lifecycle `44065308432`, AI handoff `44065307813`, AdGuardHome `44065308334`, infosvr `44065308038`, AliyunDrive `44065308080`, AliDDNS `44065307886` passed; NVRAM `44065307934` was still running at checkpoint. XY-C1 on source `04b98092` was still in progress.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Aggregate multicast-to-unicast errors during AP startup

- Parent main SHA: `f765dc8cc777916e1edff8713db6b40adc979623`; result: THIS COMMIT.
- Startup-path review found all four `wif_control_m2u()` calls in `start_wifi_ap_wl/rt` ignored their return values. A failed multicast-to-unicast bridge-port setting could therefore let AP startup return success even though the control path correctly treats the same failure as fatal.
- Aggregate M2U success/failure with `all_started` for 5GHz main/guest and generic 2.4GHz main/guest APs. The existing restart transaction then executes its stop/rollback path on any M2U failure. RT3352 MII/iNIC path is unchanged because it does not configure M2U here.
- Added source integration assertions for each of the four startup paths.
- The exact Process and Wireless regression runs on the shell-fix ancestor `aec5cc95` passed (`44065308432`, `44065308208`). Exact-head Actions for THIS COMMIT are pending; no MIPS/uClibc target build is claimed yet.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Repair M2U startup checker after first CI failure

- Parent main SHA: `69e4ab000835efe5f5107fbfdce696c68cf95706`; result: THIS COMMIT.
- Exact-head Process lifecycle run `44066355293` failed in `Compile and run guest Wi-Fi isolation status regression`; log identified `TypeError: 'str' object is not callable` at the new M2U startup contract. An earlier loop had shadowed the name `body`, so the new code invoked that string instead of the existing `function_body()` parser.
- Corrected the call to `function_body()` and used the actual startup function signatures (the source has `int ` before the newline). No production source change was required for this regression failure.
- On exact `69e4ab00`, Wireless radio state `44066355414` and Process inventory `44066355561` passed; Process lifecycle `44066355293` failed as above; XY-C1 `44066355563` was still running.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Ensure guest Wi-Fi source checker runs on pushes

- Parent main SHA: `080b8c91cb636fa95a8511c83b552bd4e639ba15`; result: THIS COMMIT.
- Process exact-head run 44066722261 on e7e3f210 still failed at the corrected M2U source assertion because the helper expected a leading `int\n` not present in the source declaration. Commit 080b8c91 fixed the checker to use the actual function-name signature and function_body().
- The follow-up 080b8c91 changed only the checker and did not trigger Process CI: scripts/check_guest_lan_isolation_status.py was included in pull_request.paths but missing from push.paths. Add the path so the corrected integration contract is exercised on push.
- No passing Process rerun after checker correction exists yet; do not record the M2U checker as verified until this commit's exact-head Process run completes.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---
## 2026-11-17 — Keep guest AP rollback link-state inspection fail-closed

- Parent main SHA: `776bd2af8e84ec7c4b87b336a31158ab09c69ae3`; result: THIS COMMIT.
- The rollback callbacks used `is_interface_up()`, which returns 0 for both a confirmed-down interface and any `get_interface_flags()` socket/ioctl failure. That made a failed link-state query indistinguishable from down and could falsely skip interface-down and pass final rollback verification.
- Added `wireless_guest_isolation_link_state()`: negative flag reads return `-EIO`; only valid flag values can produce up/down; invalid output pointer returns `-EINVAL`. Both rollback bring-down and verify-down callbacks now use `get_interface_flags()` and propagate query failure.
- Added GNU89-compatible unit cases for flags-read error, confirmed down, confirmed up and null output. Extended the Process source contract to ensure both production callbacks use the checked helper and never fall back to `is_interface_up()`.
- Exact-head parent `776bd2af`: Process lifecycle `44067663998` PASS (includes guest isolation/M2U source checker and C regressions); NVRAM `44067664059` PASS; AdGuardHome `44067664276` PASS; AI handoff `44067664224` PASS; AliyunDrive `44067664006` PASS; infosvr `44067664305` PASS; AliDDNS `44067664325` PASS. XY-C1 source run `44066355563` for `69e4ab00` completed SUCCESS; Wireless radio run `44066355414` for `69e4ab00` completed SUCCESS. These builds validate the prior M2U source commit, not this new tri-state change.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---
## 2026-11-17 18:50:33 CST — Fail closed on direct LAN-bridge AP startup

- Parent main SHA: `d2b1d75c4dfa3dd9792f31c8ed55ced3de9c6262`; result: THIS COMMIT.
- Audit found that `init_bridge()` starts AP radios directly via `start_wifi_ap_rt/wl()`, bypassing `restart_wifi_rt/wl()` and their guest-isolation rollback. A startup or final isolation failure could leave a guest AP that this boot path had just brought up.
- `init_bridge()` now snapshots guest interface ownership before AP startup and checks both AP startup return values and the final guest-isolation result. The shared rollback runs only for a guest that was down before initialization and is up afterward, then reapplies isolation. Pre-existing UP guests are preserved.
- Added a pure startup rollback-candidate predicate and GNU89 unit coverage for new-UP, pre-existing-UP, still-down and down-transition cases. Extended the source contract and wired it into both Process and Wireless CI.
- RT3352 MII uses `IFNAME_INIC_GUEST` as the interface target and the explicit VLAN bridge endpoint; restart rollback now uses the same MII guest VIF identity.
- Host build/physical-router L4 were not run in this ChatGPT runtime; exact-head Actions are pending.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 18:53:50 CST — Correct Process source contract for direct AP-start status variables

- Parent main SHA: `89f232e6a5e1ed445129dc18a45356c148e71c14`; result: THIS COMMIT.
- Exact-head Process workflow run `44068630031` failed in `Verify LAN startup error propagation through PID 1 and LAN restart`. Guest isolation policy/source checks passed earlier in that job. The failing assertion expected direct `start_wifi_ap_rt/wl()` calls to assign the generic `operation_status`; the new direct-boot rollback records AP startup separately as `rt_ap_start_status` and `wl_ap_start_status` so it can distinguish AP startup failures from WDS/APCLI operations.
- Updated only the workflow's embedded source contract to accept dedicated AP status variables while retaining generic operation-status checks for WDS/APCLI. No production-source change was needed for this failure.
- Exact-head on `89f232e6`: Wireless radio `44068630167` PASS; AI docs `44068629955` PASS; Process inventory `44068630249` PASS; Process lifecycle `44068630031` FAILED at the stale assertion described above; XY-C1 `44068629924` was still IN_PROGRESS at this checkpoint.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Use checked interface state throughout guest controls

- Parent main SHA: `5c5300876827a62a99bd2d5ac82d98853b5a230d`; result: THIS COMMIT.
- Explicit guest enable/disable previously used `is_interface_up()`, which masks `get_interface_flags()` errors as down, to decide whether the main AP was active and whether the guest VIF was already up.
- Added a shared checked-state reader and used it for main/guest interface decisions in both guest controls. Guest activation refuses to proceed when required state cannot be read. Guest-disable records unknown state but still attempts ifconfig-down and bridge detach, then reapplies isolation.
- Rollback callbacks use the same checked reader. Process source contract checks the helper and confirms both controls avoid unchecked `is_interface_up()`.
- Exact-head parent `5c530087`: Process run `44069081348` was pending at last poll; AdGuardHome `44069081212` passed; remaining workflow runs pending. This commit requires fresh Process/Wireless results.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---
## 2026-11-17 — Propagate guest bridge interface-state inspection failures

- Parent main SHA: `801d1bbfaf66448ea98a4f6ef0371f53da9c8ac5`; source and source-contract correction committed atomically in THIS COMMIT.
- `restart_guest_lan_isolation()` used `is_interface_up()` for 5GHz guest, 2.4GHz guest-policy input and the actual 2.4GHz/iNIC bridge port. Failed interface-flags ioctls were treated as DOWN, silently skipping isolation configuration.
- Replaced all three reads with `wireless_guest_interface_state_checked()`; record inspection errors and skip only the operation whose state is unknown while continuing independent band/cleanup work.
- Updated the source checker: rollback callbacks already use the shared checked reader; prior exact-head Wireless run `44069490310` and Process run `44069490305` failed on the stale assertion at line 110, after compiled policy regression passed. New exact-head runs must verify the corrected contract.
- Parent XY-C1 `44069490610` and NVRAM `44069490286` were pending; recheck against this commit. No L3/L4 claim.

---
## 2026-11-17 19:04:15 CST — Restore append-only development log archive

- Parent main SHA: 1be4affc0b533639d604515b0d0b98545dcdb02f; result: THIS COMMIT.
- The exact-head AI handoff documentation check 44070330879 failed because the preceding source commit replaced the entire dev_log.md with only the newest record, leaving one entry instead of the required 20 complete newest entries.
- Restored the complete 117-entry archive from parent 9bb789d0, then prepended the current guest link-state/attempt-owned rollback milestone. This is an append-only history correction; no source code or test behavior changed.
- On source SHA 1be4affc, Wireless radio 44070331034 and Process lifecycle 44070330981 were still in progress, XY-C1 44070330756 was in progress, and AI docs 44070330879 failed solely on the truncated archive.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; physical-router L4 NOT VERIFIED.

---
## 2026-11-17 19:07:56 CST — Correct guest boot rollback checker formatting and snapshot-order contracts

- Parent main SHA: `85062ff1a9a0d8b633d8c11b1d90cb7d6a18c2a7`; result: THIS COMMIT.
- Exact-head Process run `44070663265` on source `1be4affc` compiled and passed the GNU89 ASan/UBSan wireless guest-isolation policy binary. The production checker then failed at line 147 because it expected a two-line C function call as one exact string; a second assertion still expected the previous no-argument link-state API.
- Updated checker assertions to validate the multiline RT/WL rollback arguments and the current checked status-returning snapshot calls, while preserving ordering checks through isolation and rollback. No production source changed in this checker-only repair.
- AI handoff documentation run `44070663411` passed on `85062ff1` after restoring the 117-entry archive; `dev_log.md` remains append-only and retains its complete prior history.
- Exact-head Process/Wireless reruns and XY-C1 current-source build need checking after this commit; physical router L4 remains NOT VERIFIED.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---
## 2026-11-17 19:12:43 CST — Propagate initial deferred Optware worker launch failures

- Parent main SHA: `40dc687e25b1537febbd35a8b633a10067f6bc65`; result: THIS COMMIT.
- P1 startup-status inspection found `start_rwfs_optware()` returned void even though its asynchronous worker launch can fail during pipe setup, descriptor configuration, or fork. The function logged and scheduled a retry, but `init_router()` discarded that initial launch failure and did not include it in the existing `boot_services_status` aggregate.
- Changed the function/API and non-NAND stub to return the immediate launch result (`0` for not-applicable/already-pending; the worker's nonzero status when initial launch fails). Retry scheduling and the nonblocking alarm remain unchanged.
- `init_router()` now folds the initial Optware launch result into `boot_services_status` and emits a specific retry-scheduled diagnostic. The aggregate summary moved after this step so it covers the initial Optware launch too; it still does not make optional service failure fatal to PID 1 recovery readiness.
- Extended `scripts/check_rwfs_mount_integration.py` to verify the return contract, non-NAND stub, and caller aggregation/logging. Existing storage Optware policy tests cover child-result decoding and retry/backoff; exact-head CIs will validate the integration checker.
- Guest Process/Wireless source-contract fix and AI docs passed on descendant SHA `40dc687e`; XY-C1 Build/TRX/artifact for source `1be4affc` was still pending at last poll.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.

---
## 2026-11-17 19:14:54 CST — Normalize router source before checking Optware status ordering

- Parent main SHA: `448cb4e6ad854228204100d5fb06f8f4135fd918`; result: THIS COMMIT.
- Preflight of the new RWFS integration assertion caught a checker-only bug: it used a whitespace-normalized expected call string against the unnormalized multiline `router_init` body. Switched the call-order comparison to the existing `router_compact` source, preserving the actual ordering assertion without depending on tabs/newlines.
- No production code changed in this correction. The Optware initial worker launch status is still returned, aggregated into `boot_services_status`, and retry scheduling remains unchanged.
- Exact-head Process and XY-C1 for the Optware change remain pending/in progress at this checkpoint; the checker correction is included before declaring those results.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.

---
## 2026-11-17 19:56 CST — Fail closed when Inadyn cache preparation fails

- Parent main SHA: `8a77982a142c4d55dbad874fee8c92823dfbe3d7`; result: THIS COMMIT.
- Confirmed defect in `services_ex.c:start_ddns()`: the `rm -rf /tmp/inadyn` result for an explicit cache reset and the `mkdir()` result were both ignored, so stale cache could survive a failed reset and the daemon launch could continue without a usable cache directory. Both direct callers preserve the function result (`rc.c` applet returns `ret`; `notify_ddns_update()` returns `start_ddns(0)`).
- Added an injected cache preparation policy: a requested reset is mandatory; `system()` wait statuses are normalized (including exit status 256 -> EIO, avoiding an apparent success when narrowed by the process exit code); mkdir errors abort except EEXIST followed by `lstat()` proving an actual directory. Files and symlinks are rejected. Configuration publication and Inadyn launch occur only after successful cache preparation.
- Added GNU89/Werror/ASan/UBSan regression cases for reset success/failure, raw system wait-status normalization, mkdir errors, existing directory, regular-file/symlink collisions, lstat failure and invalid callbacks. Extended the existing Inadyn production wiring contract; its workflow already covers all changed source/test/checker paths.
- No local checkout/host compiler was available in this session; local L2 test NOT RUN. Exact-head Inadyn Actions are triggered by this commit and must be checked by exact `head_sha`; no target MIPS/uClibc or physical-router validation is claimed.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 20:03 CST — Gate PID 1 router startup on required boot environment

- Parent main SHA: `ac7787ccee5df8a6e52a16177ef094f7b0504ca7`; result: THIS COMMIT.
- Source review found `init_main_loop()` discarded the result of `system("dev_init.sh")`, while `dev_init.sh` ignored failures from essential proc/sys/dev/etc/tmp/var and devpts mounts, `mdev -s`, and private notification/HTTPD directory setup. PID 1 could therefore call `init_router()` and launch services against a broken boot filesystem/device environment.
- Added checked helpers for mandatory mounts and directory/mode preparation; a failed requirement exits the boot script before router startup. Debugfs, cgroups, storage seeding and the optional persistent user start script remain best effort; an explicit final success prevents an optional start script failure from masking the required setup result.
- PID 1 now records the shell status and gates `init_router()` on both successful boot-environment setup and NVRAM node readiness. If required setup fails, PID 1 remains alive but does not enter router-service startup; console availability is not guaranteed when `/dev` itself failed.
- Added command-mocked shell tests for mount, mkdir and chmod failures and a semantic integration checker; both are wired into the existing Process lifecycle push workflow. Local test execution NOT RUN in this connector-only runtime; exact-head Actions must validate them.
- Previous Inadyn cache-preparation fix: exact-main push workflow `44081035410` / job `134031372777` PASS (GNU89/Werror/ASan/UBSan compile, policy regression and production integration checker). Exact source SHA: `ac7787ccee5df8a6e52a16177ef094f7b0504ca7`.
- XY-C1 firmware workflow `44081035905` on `ac7787ccee5df8a6e52a16177ef094f7b0504ca7` was in progress at last check; no L3 or L4 claim for this init slice.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 20:53 CST — Repair exact-head boot integration contracts

- Parent main SHA: `10be37e50fbbd11c686593b0edd3863b31055b2e`; result: THIS COMMIT.
- The boot-environment source integration passed in exact-main Process lifecycle run `44084634310`: both the command-mocked required-mount/directory policy regression and `check_dev_init_boot.py` passed. AI handoff workflow `44084634333` also passed.
- Exact-head NVRAM `44084634391` and HTTPD security `44084634464` failed only because their static contracts still required the prior PID-1 expression `router_ready=(nvram_node_result==0&&init_router()==0)`. Update both to assert the stricter production gate, which also requires `dev_init_status==0`.
- Exact-head USB storage `44084634290` failed in its embedded source checker because it still searched for `void start_rwfs_optware(void)`; current `common_ex.c` and `rc.h` expose `int start_rwfs_optware(void)` and return the initial worker launch status. Correct the contract and assert the returned status.
- The XY-C1 workflow did not trigger for the boot source commit because its path filters omitted `init.c` and `dev_init.sh`. Add both paths; the workflow already watches its own file, so the corrective workflow change should trigger the current-source Build/TRX/artifact sequence.
- This is test/workflow integration repair only; product source is unchanged. Local host execution NOT RUN in this connector-only runtime. Corrective exact-head L2/L3 Actions are pending; do not claim firmware/TRX or hardware validation until those runs complete.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 21:03 CST — Align HTTPD boot-directory contract with required_dir

- Parent main SHA: `ab22c03fb3a8acd37004d52b5b3bcfd55dd1e461`; result: THIS COMMIT.
- HTTPD umbrella run `44088845536` reached its notification queue contract and failed because it expected inline `mkdir -p -m 700` calls that had already been replaced by the checked `required_dir 700 ... || exit 1` helper. Updated the contract to validate the current required-directory order and checked `mdev -s` gate.
- Removed the duplicate unchecked `chmod 700 /tmp/rc_notification /tmp/rc_action_incomplete` command: the preceding `required_dir` calls already create each directory and fail closed if its permission change fails. The focused PID-1 boot checker now guards against reintroducing this redundant unchecked operation.
- Parent exact-head Process `44088845758`, NVRAM `44088845655`, USB storage `44088845567`, AI docs `44088845461`, AliDDNS `44088845627`, AdGuardHome `44088845523`, AliyunDrive `44088845392`, and infosvr `44088845694` passed. XY-C1 `44088845842` was still building at checkpoint. New exact-head HTTPD, Process, and XY-C1 Actions pending.
- This slice removes only a redundant chmod already performed by a checked helper and repairs the stale contract; no target or hardware result is claimed. Local host tests NOT RUN in this connector-only runtime; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 21:15 CST — Align HTTPD storage restore boot-order contract

- Parent main SHA: `d638def602cc2de4d43ed06e9ef55b24b9439afe`; result: THIS COMMIT.
- Exact-head HTTPD run `44089834028` passed its notification queue contract, then failed at embedded Python line 713: its storage restore startup contract still expected the old unchecked `chmod 1777 /tmp` and `mkdir -p -m 700 /tmp/.httpd-private`.
- Production `dev_init.sh` now uses checked `required_dir 1777 /tmp || exit 1` and `required_dir 700 /tmp/.httpd-private || exit 1`; update the HTTPD checker to validate these exact required calls and their ordering before optional `mtd_storage.sh load`. This is checker-only.
- Exact `d638def` NVRAM `44089834067`, AdGuardHome `44089833973`, AliyunDrive `44089833865`, infosvr `44089833884`, AliDDNS `44089833907`, AI docs `44089833839`, Process lifecycle `44089833960`, and Process inventory `44089833968` PASS. XY-C1 `44089833795` was still in progress at last query. New exact-head HTTPD/docs/XY checks pending.
- Local host tests NOT RUN; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 21:30 CST — Roll back partial Ralink IAPP startup

- Parent main SHA: `4220420d7dd9f6440f525875802646899198befa`; result: THIS COMMIT.
- Source review found IAPP launcher/readiness/firewall failures returned an error but left a partially started daemon running. If TCP INPUT rule installation succeeded and UDP rule installation failed, the TCP rule also remained.
- `iappd.sh` now rolls back the daemon on launcher failure, failed bounded readiness postcondition, and TCP/UDP rule failure. A UDP append failure removes only the TCP rule successfully added by this invocation. Cleanup errors are logged while preserving the original startup failure.
- Extended the existing mock-based lifecycle regression for partial-child launcher failure, first/TCP rule failure, and second/UDP rule failure after successful TCP publication. Process CI already runs this test; XY-C1 path filter already includes the production shell script.
- The parent `4220420` exact-head checks must still be inspected; this source/test slice Actions pending. No local host execution, current-source firmware/TRX or physical-router L4 is claimed.

---

## 2026-11-17 21:44 CST — Preserve original IAPP startup error across rollback

- Parent main SHA: `2549e37eb27b8a8879f7ad4b0fad4161e71a1f39`; result: THIS COMMIT.
- Exact-head Process run `44091805839` failed in the IAPP regression. The helper contained a literal backslash before shell parameter expansion, producing an "Illegal number" test error; additionally, the call site returned global `rc` after `stop()` could overwrite it, turning a launcher failure into false success.
- Corrected both state checks to use variables initialized by `start()`. Each startup failure path now returns the status returned by `rollback_start_failure()`, preserving the original cause independently of stop-helper bookkeeping.
- Added a regression forcing partial-daemon cleanup to fail, asserting that the original launch code is still returned and the rollback failure is logged.
- Parent SHA `4220420` HTTPD `44091239897` PASS; NVRAM `44091239714`, AdGuardHome `44091239939`, AliDDNS `44091239940`, AI docs `44091239799`, infosvr `44091239833`, AliyunDrive `44091239881`, and Process inventory `44091239744` PASS. XY-C1 `44091239961` was still in progress at last poll. New exact-head Actions pending.
- Local host tests NOT RUN; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 21:52 CST — Fix IAPP mock ordinal in retained firewall failure case

- Parent main SHA: `a54c1b427d0080aa4563e4da9d35051b40dbcb57`; result: THIS COMMIT.
- Exact-head Process inventory/lifecycle run `44092580092` reached the IAPP regression and failed with `expected 42, got 0`. The mock was changed to fail only the Nth `iptables -A` operation, but a retained legacy test set only `MOCK_IPTABLES=42`; its zero `MOCK_IPTABLES_FAIL_AT` correctly disabled the failure.
- Fix the test fixture to set `MOCK_IPTABLES_FAIL_AT=1`. Production IAPP script and rollback behavior are unchanged.
- On parent `a54c1b`, Process lifecycle inventory `44092579805`, infosvr `44092579891`, AdGuardHome `44092580045`, AliyunDrive `44092580008`, AliDDNS `44092579920`, and AI docs `44092579809` PASS; XY-C1 `44092579822` and NVRAM `44092579905` were still in progress at last poll. New exact-head tests pending.
- Local host tests NOT RUN; L4 NOT VERIFIED.

---

## 2026-11-17 22:03 CST — Cover IAPP post-launch readiness rollback

- Parent main SHA: `da2808b4e7080072b7a77676ec3c0c2f7cba343b`; result: THIS COMMIT.
- Exact-head `da2808b` Process lifecycle/inventory run `44093110816` and Process inventory run `44093110839` PASS; Process job log confirms both IAPP RC shutdown registration and daemon lifecycle regressions PASS.
- Exact-head NVRAM `44093110807`, infosvr `44093110957`, AI handoff docs `44093110887`, AdGuardHome `44093111099`, AliDDNS `44093110904`, and AliyunDrive `44093110974` PASS.
- Added a missing readiness-failure case: launcher returns success but daemon vanishes before the bounded running postcondition. It must return failure, must not publish either firewall rule, and must log the timeout. Production script unchanged. Test fixture state now includes `MOCK_EXIT_AFTER_LAUNCH`.
- XY-C1 run `44092579822` for product source `a54c1b` was still IN_PROGRESS at last poll. The boot-environment-specific XY-C1 Build/TRX/artifact run `44091239961` on `4220420` completed SUCCESS. Do not generalize it to the later IAPP source. Test-only changes in this commit do not require a new firmware build.
- Local host tests NOT RUN; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Gate AliDDNS boot startup on CA readiness

- Parent main SHA: `55374c12e124711c0b789ffd8dc3f0767c3343cc`; result: THIS COMMIT.
- Source review found a dependency gap: `rc.c` installs the firmware CA trust archive before launching `autostart.sh`, and `aliddns.sh` refuses HTTPS requests without `/etc/ssl/certs/ca-certificates.crt`, but autostart launched the AliDDNS keeper unconditionally when enabled even after CA installation failed.
- `autostart.sh` now checks that exact CA bundle is readable before launching AliDDNS. Missing trust material logs a specific skip reason and does not block unrelated optional add-ons.
- Extended `autostart_optional_start_status.sh` with a private CA-path fixture, an absent-bundle case proving AliDDNS is skipped while DDNSTO/FRP still start, and a present-bundle case proving AliDDNS starts and its launcher error remains logged.
- The existing Process workflow already covers the autostart source/test paths; the AliDDNS transport workflow also watches `autostart.sh`. Exact-head runs for THIS COMMIT are pending.
- Parent `55374c` exact-head Process lifecycle, Process inventory, NVRAM, AdGuardHome, AI docs, infosvr, AliyunDrive, and AliDDNS runs all PASS. IAPP product source `a54c1b` XY-C1 Build/TRX/artifact run `44092579822` completed SUCCESS; this does not validate the new autostart source change.
- Local host tests NOT RUN in the connector-only runtime; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Correct AliDDNS CA-gate regression fixture

- Parent main SHA: `0c754be22ab4d4da7e78198f5ca043c521ec4dc2`; result: THIS COMMIT.
- Corrected a test-fixture typo in `autostart_optional_start_status.sh`: the new AliDDNS NVRAM case had a literal backslash-n before the existing FRP server case, rather than a real line break. The shell regression would fail parsing before exercising either CA branch.
- Production `autostart.sh` is unchanged; the correction is isolated to the test fixture and checkpoint metadata. Exact-head Process/AliDDNS Actions are pending for this correction.
- The prior parent `55374c` baseline Actions all passed; IAPP product source `a54c1b` completed XY-C1 Build/TRX/artifact run `44092579822`.
- Local host tests NOT RUN in connector-only runtime; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Align AliDDNS static contract with CA-gated boot startup

- Parent main SHA: `32e2eab9757261d458f7035a7d4215c5b5dc5e88`; result: THIS COMMIT.
- Exact-head AliDDNS workflow `44095983119` completed its shell syntax, signed API transport, IPv4 discovery, DNS lifecycle, custom script, keeper lifecycle, CA bootstrap success/failure, and HMAC privacy tests successfully. It failed only in `scripts/check_aliddns_api_transport.py` because the checker still expected the pre-change unquoted `aliddns_enable` condition and direct launcher invocation.
- Updated the static contract to assert the quoted enable check, readable `/etc/ssl/certs/ca-certificates.crt` gate, `run_optional_start` invocation, skip diagnostic, and execution before the independent DDNSTO block. Product shell/source behavior is unchanged.
- Parent `32e2eab` AdGuardHome `44095983135`, infosvr `44095983195`, AI docs `44095983011`, AliyunDrive `44095983148`, Process lifecycle `44095983136`, and Process inventory `44095983194` PASS; NVRAM `44095983154` was in progress at last poll. New exact-head AliDDNS/docs runs pending.
- Local host tests NOT RUN; current-source XY-C1 after AliDDNS source change and physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Trigger XY-C1 builds for packaged autostart changes

- Parent main SHA: `d6c0a1aa92d7f1615576811e653703364ae3b27a`; result: THIS COMMIT.
- Confirmed the XY-C1 firmware workflow's push path filter omitted `trunk/user/scripts/files/usr/bin/autostart.sh`, despite that file being packaged into ROMFS. As a result, the CA-readiness source change did not trigger a target build.
- Added the packaged autostart script to the workflow's path filter. This workflow-only commit itself triggers an exact-head XY-C1 build of the current main source.
- Exact-head AliDDNS transport run `44096633039` on parent `d6c0a1` passed after its semantic checker was updated for the CA gate. AI docs, infosvr, AliyunDrive, and AdGuardHome also passed; Process and NVRAM were pending at last poll.
- XY-C1 Build/TRX/artifact for THIS COMMIT pending. Local host tests NOT RUN; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Propagate WAN firewall notification enqueue failures

- Parent main SHA: `ec99ebf0f47393f712b62771a1f34e5af8f621bf`; result: THIS COMMIT.
- Confirmed defect: `notify_rc_internal()` logged secure-queue open/touch failures but returned `void`. Dynamic MAN-up, MAN-down and WAN-up ignored the result of queuing `restart_firewall_wan`, so a route transition could report success while the firewall refresh had not been accepted by PID 1.
- Added additive `notify_rc_status()`: 0 means both secure queue markers were accepted; -1 means queue publication failed. Existing void wrappers remain source/ABI compatible and production queue ownership remains uid 0. Failed wakeup delivery after successful marker creation is logged without falsely reporting queue failure.
- WAN/MAN callers merge enqueue failure into their existing first-error result.
- Added an isolated GNU89 integration test using the production notifier with compile-time test path/owner overrides and a stubbed `kill(1, SIGUSR1)`; cases cover missing directories, invalid names, valid/duplicate markers, unsafe queue mode and successful queueing despite wakeup failure.
- Extended notification/WAN contracts and path filters; XY-C1 also watches the changed `notify_rc.c`. Exact-head Actions pending; local C execution NOT RUN; physical-router L4 NOT VERIFIED.
- Parent `ec99ebf` AliDDNS `44096925473`, Process `44096925510`, NVRAM `44096925502`, AdGuardHome `44096925476`, infosvr `44096925399`, AI docs `44096925411`, and AliyunDrive `44096925225` PASS.

---

## 2026-11-17 — Align HTTPD queue security contract with owner abstraction

- Parent main SHA: `327164075c015004e17ef435fd06994ff4074090`; result: THIS COMMIT.
- Exact-head HTTPD run `44098885513` failed only in the static privileged-queue check after the notifier introduced a local owner variable. Its assertions still required `rc_notify_queue_open_dir(..., 0)`.
- The production default remains root-only: `#ifndef RC_NOTIFY_OWNER / #define RC_NOTIFY_OWNER ((uid_t)0)`; both queue open operations and marker touches use `uid_t owner = RC_NOTIFY_OWNER`. The test can override ownership for temporary directories.
- Updated HTTPD workflow assertions to verify the default root owner and the derived owner at both directory opens and marker touches. This is a CI contract correction; no production HTTPD or notifier source changed.
- Exact-head WAN route `44098885494` and PID 1 notification queue `44098885415` passed on `3271640`. XY-C1 `44098885378` remained in progress at last poll; this change triggers the HTTPD security workflow again.
- Local host execution NOT RUN in connector-only runtime; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Update MAN-down firewall notification source contract

- Parent main SHA: `353776121c419780fea73d1e43bb7d7f3dcc3820`; result: THIS COMMIT.
- After the root-owner assertion repair, HTTPD exact-head run `44099542600` passed the DHCP input policy and LAN DHCP bound/renew source contracts, then failed one later static assertion that still required the direct `notify_rc("restart_firewall_wan")` call in `man_down_internal()`.
- The production source now routes MAN-up/MAN-down/WAN-up through `queue_wan_firewall_restart(context)`, which uses the checked `notify_rc_status()` API and propagates enqueue errors. Updated the stale MAN-down contract to require the helper call and first-error aggregation. Unrelated legacy `notify_rc()` events are unchanged.
- The workflow itself is in its push path filter; exact-head HTTPD rerun is pending.
- Exact WAN route `44098885494` and notification queue `44098885415` PASS on `3271640`; XY-C1 `44099542554` remains IN_PROGRESS for source build/TRX/artifact.
- Local host execution NOT RUN in connector-only runtime; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Keep committed LAN DHCP lease aligned with BusyBox

- Parent main SHA: `d4f64127749f78e463bb82541e3e1b73fc5d4992`; result: THIS COMMIT.
- Found a post-commit status inconsistency in `udhcpc_lan_bound()`: the core transaction had changed interface address, routes, resolver and temporary NVRAM lease fields, but a later `create_hosts_lan()` failure was returned as overall DHCP script failure.
- BusyBox treats nonzero bound/renew script status as a rejected lease: initial acquisition retries without committing BOUND; renewal retains its previous lease deadline and charges callback runtime against it. This diverges from the router's already committed network state.
- Host-file publication remains best-effort and its failure remains logged. The callback now returns only the core lease transaction result; core errors still return before NVRAM publication.
- Added `lan_dhcp_callback_policy.h` and a GNU89 ASan/UBSan regression proving post-commit ancillary errors cannot reject a committed lease while core transaction errors remain fatal. Wired the regression and source-order/result assertions into the WAN DHCP callback workflow and both path filters.
- Exact parent `d4f6412` HTTPD, Process, NVRAM, AliDDNS, infosvr, AdGuardHome, AliyunDrive and AI docs passed; XY-C1 `44099813116` was IN_PROGRESS at last poll. Exact-head WAN DHCP/XY-C1 pending.
- Local host execution NOT RUN in connector-only runtime; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Align HTTPD DHCP contract with committed lease callback status

- Parent main SHA: `ee3cbaf4342d90e65978a9211bd279a6d21373ba`; result: THIS COMMIT.
- Exact-head HTTPD run `44101239302` failed at its stale source assertion requiring `return hosts_status;`. The production fix in `ee3cbaf` now returns `lan_dhcp_bound_callback_status(0, hosts_status)`, correctly keeping a committed DHCP lease successful when only ancillary hostname/hosts publication fails.
- Updated the HTTPD static contract to verify that post-commit ancillary failure is ignored, core transaction failure remains fatal, and the GNU89 sanitizer regression covers both outcomes. Added the helper/test files to both HTTPD workflow path filters.
- Exact-head WAN DHCP callback run `44101239234` on `ee3cbaf` passed, including the ASan/UBSan callback regression and source contract. HTTPD rerun and AI handoff docs are triggered by this workflow/documentation-only follow-up; XY-C1 run `44101239264` on the unchanged firmware source was still IN_PROGRESS at checkpoint.
- Local host execution NOT RUN in this connector-only runtime; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Restore static LAN hostname files on DHCP deconfig

- Parent main SHA: 23e4563fc9a610c420d5aca60d485372b6718da3; result: THIS COMMIT.
- LAN DHCP bound publishes the leased address into /etc/hostname and /etc/hosts, but deconfig restored the persistent static LAN address without restoring those host files. The old DHCP address could remain in the hostname mapping after NAK, release or lease expiry.
- Added a route/interface readiness predicate. Once the old route is absent and static LAN address is installed, deconfig restores hostname/hosts files using persistent lan_ipaddr and lan_domain, including when resolver cleanup needs retry.
- Host-file publication failure is returned, preserving lease identity for BusyBox DECONFIG_PENDING retry. On full success only, clear DHCP-only lan_dns_t, lan_wins_t and lan_lease_t after update_lan_status(0).
- Added readiness cases to the GNU89 DHCP policy regression and scripts/check_lan_dhcp_deconfig.py. Wired both into WAN DHCP callback CI and the XY-C1 source path.
- Exact-head WAN DHCP and XY-C1 Build/TRX/artifact validation for THIS COMMIT pending. Local host compile NOT RUN; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Prevent zero-delay DHCP callback retry loops

- Parent main SHA: da076cf8d2425b747a9ffda182144bcce49d5ab0; result: THIS COMMIT.
- BusyBox parses -A/--tryagain with A+, which explicitly accepts zero. On a failed initial bound/renew event, udhcpc_script_failure_prepare_retry() copied that zero directly to the event-loop timeout; a persistent callback failure could therefore retry the same DHCPREQUEST continuously without sleeping.
- Clamp the initial-acquisition callback retry delay to at least one second, matching the existing DECONFIG_PENDING retry floor. Add GNU89 test cases for both zero and negative policy inputs, and pin the public zero-accepting option contract in the source checker.
- Exact-head da076cf WAN DHCP callback, WAN route, DNSMasq, HTTPD, Process inventory, AdGuardHome, AliDDNS, AliyunDrive, infosvr, detect_link, detect_internet, wireless-radio and AI handoff push Actions passed. NVRAM and XY-C1 were still in progress when this change was prepared; exact-head udhcpc script regression pending.
- No local checkout/compiler in this connector-only runtime; host test execution awaits Actions. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Ensure udhcpc policy header changes trigger XY-C1 firmware validation

- Parent main SHA: d674d50bfe95863dbe0851e269180d0516868586; result: THIS COMMIT.
- The dedicated BusyBox udhcpc script-status workflow passed exact-head run 44102850811 on d674d50. Review found the XY-C1 path filter tracked dhcpc.c but omitted the included udhcpc_script_policy.h, so a header-only production fix would not trigger a same-SHA firmware build.
- Add the policy header and the XY-C1 workflow file itself to the push path filter, ensuring the current source receives MIPS firmware validation and future trigger-filter edits are exercised. Exact-head XY-C1 run for this follow-up pending; the da076cf build is still independent and in progress.
- Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Propagate failed kernel hostname/domain updates during LAN host publication

- Parent main SHA: 422e006dc861caefcb7f3b231f0f53842ec63eef; result: THIS COMMIT.
- Audit found create_hosts_lan() silently discarded both sethostname() and setdomainname() return values after publishing /etc/hostname and /etc/hosts. A kernel-name update failure was therefore reported as success; during DHCP deconfig BusyBox would leave DECONFIG_PENDING even though the running kernel hostname/domain could remain stale.
- Added an injectable GNU89 policy that attempts both kernel setters and preserves the first errno-style failure. Production adapters normalize syscall failures to errno/EIO, create_hosts_lan() reports setter failures only after releasing the hostname lock, and the existing deconfig callback now naturally triggers BusyBox retry. No false rollback-snapshot cleanup is attempted after the file pair has already committed.
- Added regression cases for success, hostname failure with domain update still attempted, domain-only failure, invalid callbacks and missing setters. WAN DHCP workflow compiles it with ASan/UBSan; XY-C1 path filter includes the production header and net_lan.c remains tracked.
- Exact-head Actions and XY-C1 Build/TRX pending. No local compiler in this connector-only runtime; physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Correct LAN kernel-name checker ordering after first CI run

- Parent main SHA: ff47cd7cfe079529edbd1f058603666bcc37774f; result: THIS COMMIT.
- Exact-head WAN DHCP run 44104199182 compiled and passed the existing DHCP lease regression, then failed in scripts/check_lan_dhcp_deconfig.py. The new assertion incorrectly compared lock release against the first `if (saved_errno != 0)`, which is intentionally the pre-release diagnostic log, not the post-release error return.
- Corrected the source contract to distinguish first-error logging from the final post-lock-release return. Production behavior remains unchanged; the dedicated kernel hostname/domain regression compile step was not reached in the failed run and is pending rerun.
- Exact-head XY-C1 run 44104199066 remains in progress; no build result inferred. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Clear withdrawn DHCPv6 automatic DNS servers

- Parent main SHA: 357347eaf5085869a80d83a7c075970affb9c073; result: THIS COMMIT.
- The DHCPv6 response environment only exports `new_domain_name_servers` when the Reply actually contains the DNS option. `store_wan_dns6()` previously returned without changing state for a missing or empty value, leaving DNS servers from an earlier lease in WAN NVRAM/resolver state. RFC 8415 §18.2.9 says a client SHOULD stop using previously received configuration information when it is omitted from a Renew, Rebind, or Information-request Reply.
- Treat an absent/empty automatic DNS option as an empty list and refresh resolver state when the list is withdrawn. Reject an overlong option rather than silently truncating it, and propagate that validation failure after releasing the DHCPv6 callback generation lock.
- Added GNU89 ASan/UBSan unit cases, a BusyBox response-environment/source contract, WAN DHCP push/PR path filters, and XY-C1 coverage for the new production policy header. Exact-head Actions for this commit pending. Prior parent 357 WAN DHCP regression [44104411947] PASS; parent XY-C1 [44104411880] was still building at last poll.
- No local checkout/compiler in this connector-only runtime. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Keep HTTPD DHCP contract aligned with kernel hostname error propagation

- Parent main SHA: 158bd53e800104da692ab41e87b12eca85f3ecb9; result: THIS COMMIT.
- Exact-head WAN DHCP callback run 44105702632 passed, including DHCPv6 DNS withdrawal and LAN kernel hostname/domain tests. HTTPD run 44105702667 exposed a stale inline assertion still requiring `sethostname(lan_hname)` directly inside `create_hosts_lan()` after that call was correctly moved behind the injectable `lan_kernel_names_apply()` policy.
- Updated the HTTPD source contract to verify the pair publish precedes the policy call, both syscall adapters are present, and regression cases cover each setter failure; added the helper/test to the HTTPD workflow path filters. Production C source is unchanged by this follow-up.
- Exact-head XY-C1 run 44105702953 remains in progress at the BusyBox cache setup step; Build/TRX/artifact results are pending. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Align WAN teardown regression with continued IPv4 cleanup

- Parent main SHA: 510b0756a834f6cc97d2a257a06c4c0558b612a2; result: THIS COMMIT.
- Exact-head WAN callback run 44107179918 failed in its inline static contract because it still required `wan_down_internal()` to immediately return the IPv6 teardown error. The source fix in 510b075 instead accumulates that error and deliberately continues independent IPv4 route, multicast, static-address, resolver and conntrack cleanup before returning the combined status.
- Updated the regression to require the IPv6 error to be accumulated before entering IPv4 protocol cleanup, require that no early `return ipv6_status` remains, and preserve the final accumulated return. The prior failure was a stale assertion, not a test or runtime failure in the new cleanup path.
- Exact-head process-lifecycle run 44107179477 and XY-C1 run 44107179452 were still in progress at checkpoint; exact-head WAN rerun pending. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Route legacy pids() queries through the tri-state procfs scanner

- Parent main SHA: 70cfa7881c64e25ce4109d9ded609c795600cb78; result: THIS COMMIT.
- Process-inventory regression already established a fail-closed tri-state scanner, but the exported legacy `pids()` Boolean wrapper still called `find_pid_by_name()`. That older BusyBox-derived scan collapses procfs read/enumeration and allocation failures into an empty PID list, so service/UI callers could treat an unknown process state as a confirmed stop.
- Added `pids_presence()` as an explicit tri-state API and routed `pids()` through it. For backward-compatible Boolean callers, only confirmed ABSENT maps to zero; UNKNOWN conservatively maps to one, avoiding false proof that a service is stopped. Empty/null process names retain their previous false result. The older `find_pid_by_name()` remains available for callers explicitly needing the PID list.
- Added GNU89 ASan/UBSan policy regression for RUNNING, ABSENT, UNKNOWN and invalid enum values; extended the process-lifecycle source contract and push/PR filters; XY-C1 filters now track the shared-library source and policy header.
- Exact-head Actions pending. No local checkout/compiler in this connector-only runtime; physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Remove second stale WAN IPv6 teardown early-return assertion

- Parent main SHA: 24ef91459ce1d3e7f17de5c3d843ce866d6aa675; result: THIS COMMIT.
- Exact-head WAN run 44107619068 passed the first updated static block but then failed in a second, duplicate inline contract at workflow line 705. That block still demanded `return ipv6_status;` before `del_dhcp_routes_by_prefix`, contradicting the intended 510b075 IPv4-continuation fix.
- Updated the second contract to require IPv6 status accumulation, continued IPv4 route cleanup, and a final combined `return status;`; no production code changed.
- Exact-head process inventory run 44107929004 and XY-C1 run 44107928829 were pending at checkpoint; WAN rerun pending. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Fix IPv6 aggregation checker search origin

- Parent main SHA: 95fc2873e0185135938057fa08f59a3c2fc496ab; result: THIS COMMIT.
- Exact-head WAN run 44108152600 failed at the updated second IPv6 teardown assertion. The accumulator is intentionally executed immediately after `wan6_down()` and before the `if (ipv6_status != 0)` diagnostic block; the checker incorrectly searched for it starting at the later diagnostic position, causing `ValueError: substring not found` despite the production source containing the required aggregation.
- Corrected the static contract to search from the `wan6_down()` call, then assert the actual order: call → accumulate status → log nonzero result → continue IPv4 route cleanup → return combined status. No production C change.
- Exact-head process-lifecycle run 44108152657 PASS, including legacy `pids()` UNKNOWN mapping. NVRAM audit run 44108152786 remained active; XY-C1 run 44107928829 still in `Build XY-C1` with TRX verification/artifact upload pending at checkpoint. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Validate fallback product hostname before writing /etc/hosts

- Parent main SHA: a400a3b7e5dd4835b5126479b87bf2c01894dc19; result: THIS COMMIT.
- Security review found `get_our_hostname()` validates the configured `computer_name` but falls back directly to the NVRAM `productid` when the configured value is empty or invalid. `create_hosts_lan()` only checked that the returned pointer was non-NULL, then wrote it into `/etc/hostname`, `/etc/hosts`, and the kernel hostname. A malformed fallback containing control characters could therefore inject extra lines into the hosts file despite the getter's apparent validation.
- Validate the final resolved hostname again at the file-publication trust boundary with the existing `is_valid_hostname()` policy; invalid fallback values now fail before staging or publishing either file. Extended the LAN DHCP static regression contract. WAN/HTTPD/XY-C1 exact-head validation pending.
- This is defense in depth; normal configured `computer_name` values were already validated. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Require WAN and HTTPD CI to exercise the final LAN hostname guard

- Parent main SHA: f680e4f6366b37f76d2cc552da9e77158b54226a; result: THIS COMMIT.
- Added the final-hostname validation contract to both WAN DHCP and HTTPD security regression workflows. The production fix changes `net_lan.c`; this explicitly verifies that the already-resolved hostname is checked before either host file is staged/published, including the `productid` fallback path.
- Normalized the WAN workflow's `net_lan.c` filter quoting and touched both workflow files so their exact-head push runs execute the new static assertions; XY-C1 already tracks `net_lan.c` and independently validates the firmware compile.
- Exact-head Actions for this CI-contract change pending. Physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Cross-chat development checkpoint

- Parent main SHA: 9926124272593182c7e3f8fdf6c0836d25d331a3; result: THIS COMMIT.
- Captured exact main/backup refs, recent commits, and exact-head workflow status in SESSION_HANDOFF.md and REFACTOR_ROADMAP.md. The checkpoint records the DHCPv4/v6, LAN hostname publication, WAN teardown, and tri-state process fixes completed during this continuation.
- No local compiler/checkout was available; no local test result is claimed. Physical router L4 remains NOT VERIFIED.
- Exact-head Actions status is recorded in both handoff files; the handoff-doc workflow for this documentation commit must be checked after push.

---

## 2026-11-17 — Remove stale exact-call-count assertion from storage inventory CI contract

- Parent main SHA: d64d2e3ffd61a0e6fe00dc728988d9e57574dfd4; result: THIS COMMIT.
- Triaged HTTPD security run 44110556946 on exact SHA 9926124272593182c7e3f8fdf6c0836d25d331a3. All preceding checks passed; the storage mount-inventory contract failed only because it hard-coded exactly four `count_stor_mountpoint()` call sites, while the physical hot-unplug fix correctly added a fifth call in `umount_ejected_locked()`. The surrounding checks already verify the meaningful call-site semantics.
- Removed the brittle count-only assertion rather than changing production behavior. No local checkout/compiler is available; the updated workflow must validate this correction on its exact pushed SHA. Physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Align HTTPD storage regression with the parameterized eject helper

- Parent main SHA: 81329ebbb04a80e11da0f13169355523bd1fe00b; result: THIS COMMIT.
- Exact-head HTTPD run 44111149377 on 81329ebbb04a80e11da0f13169355523bd1fe00b passed the earlier inventory assertions after the call-count fix, then failed in the safe-removal source-contract step because its parser still searched for the pre-refactor `umount_ejected_locked(void)` signature. The production helper now correctly takes the stop/restart mode parameters; the separate storage workflow already checks that contract.
- Updated the stale inline source locator to the live function signature. No product source behavior changed. Exact-head rerun required; no local checkout/compiler available and physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Gate targeted safe-remove restart on complete unmount success

- Parent main SHA: d023872d38c866cb9427db2c29f19813f9e59d21; result: THIS COMMIT.
- Found a second unsafe restart path in `safe_remove_stor_device()`: after the requested disk's staged unmount or shared ejected-mount cleanup returned an error, it still restarted storage-backed daemons whenever any mount remained. A positive count can include a stale mount from the device being removed.
- Added a shared policy gate requiring both unmount stages to succeed and a fresh positive mount inventory before `start_stor_apps()`. Either cleanup error or unknown inventory now leaves dependent services stopped; this path deliberately does not queue the ordinary app-start retry, because that retry could restart against stale/unknown mounts. Clean-empty NFS teardown is unchanged.
- Added GNU89 policy cases for successful restart, either cleanup failure, empty inventory and unknown inventory. Updated the standalone integration checker and storage/HTTPD workflow source contracts. Exact-head Actions must pass before claiming L2/L3. Physical-router L4 remains NOT VERIFIED; no local checkout/compiler was available.

---

## 2026-11-17 — Align storage safe-remove checker with fail-closed retry semantics

- Parent main SHA: 776907e05a23638a6bf5c9cd7ab1bbef35bb4571; result: THIS COMMIT.
- Exact-head storage run 44114215851 on 776907e05a23638a6bf5c9cd7ab1bbef35bb4571 passed the full caller-graph and lock-order checks, then failed only in `scripts/check_storage_safe_remove.py`: a legacy assertion still required an app-start retry after unknown mount inventory.
- The new production behavior intentionally suppresses that retry when either unmount stage failed or mount inventory is unknown; a positive stale count can refer to the physically removed disk. The normal five-second retry remains required after cleanup succeeded but `start_stor_apps()` itself failed. Updated the checker to encode these two distinct cases rather than weakening production safety.
- Parent exact-head SHA 776907e05a23638a6bf5c9cd7ab1bbef35bb4571: 14 Actions succeeded, storage checker failed, and XY-C1 firmware validation was still running. The checker correction must be revalidated on this commit's exact SHA. Physical-router L4 remains NOT VERIFIED.

---

## 2026-11-17 — Correct normalized helper-call assertion in storage checker

- Parent main SHA: e1a21c6867df9391d4fef938fda93e24ef05a07f; result: THIS COMMIT.
- The second exact-head storage run `44114861725` again passed the full storage caller-graph and lock-order checks, then the standalone checker failed on the newly added helper-wiring assertion. The checker expected a space after the opening parenthesis even though the production call is on one line. This is a test-source formatting mismatch, not a production failure.
- Corrected the checker to match the exact normalized helper invocation. The preceding assertion that checks retry only after successful cleanup remains; the obsolete unknown-inventory retry expectation remains forbidden.
- Exact parent SHA e1a21c6867df9391d4fef938fda93e24ef05a07f: AliDDNS, AliyunDrive, infosvr and AI-handoff checks PASS; NVRAM, process, AdGuardHome and storage are in progress or pending at last poll, and parent XY-C1 remains in progress from SHA 776907e. Re-run storage on THIS COMMIT before claiming L2. Physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Reject non-global IPv4 responses from AliDDNS WAN discovery

- Parent main SHA: 55ffdc1fd6591c4bf2b3aedd76a8532ab3f47540; result: THIS COMMIT.
- Audit found that `aliddns-ipv4.sh` enforced HTTPS, CA verification, HTTP 200, syntax and octet bounds, but treated any syntactically valid dotted-quad as a public WAN address. A captive portal, misconfigured HTTPS proxy or poisoned endpoint response could therefore feed private/loopback/link-local/CGNAT/documentation/benchmark/multicast/reserved space into a public DDNS A record.
- Hardened only the external WAN-discovery helper; the general `aliddns_ipv4_valid()` syntax validator remains unchanged for operator-supplied scripts and legitimate private-network use. The helper now rejects special-use ranges while preserving globally reachable IETF anycast exceptions 192.0.0.9 and 192.0.0.10.
- Expanded the mocked HTTPS regression across RFC1918, CGNAT, loopback, link-local, documentation, 6to4 relay, benchmarking, multicast and reserved address ranges; added acceptance coverage for 192.0.0.9. Extended the source-contract checker. The AliDDNS workflow already filters the helper, test and checker paths.
- Parent AliDDNS run 44115309701 on 55ffdc1fd6591c4bf2b3aedd76a8532ab3f47540 was queued at last poll; the earlier signed-API transport regression passed on 776907e. Revalidate this new address policy on this commit's exact SHA. No local test/compiler is claimed; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Fail closed on incomplete AliDDNS keeper inventory

- Parent main SHA: `097705a6cbd46d1291dc22c281863c68cc2380e8`; result: THIS COMMIT.
- Lifecycle review found that `aliddns_find_keeper_pids()` treated an unreadable `/proc/<pid>/cmdline` as though that process were absent. If the PID directory still existed, this was an incomplete scan that could miss a running keeper and allow a duplicate start.
- The finder now distinguishes the normal PID-disappeared procfs race from a still-present PID whose command line cannot be read. The latter clears partial results and returns failure, preserving the stop-before-start gate.
- Added a regression with a still-present PID directory and mode-000 cmdline; it proves incomplete discovery fails and does not retain a partial PID list. The static checker pins the production fail-closed branch.
- Extended the XY-C1 push path filter for packaged AliDDNS sources; the changed production `aliddns.sh` must trigger a target build for the current source SHA.
- Parent exact-head AliDDNS run `44115712513` on `097705a6` passed the WAN IPv4 hardening slice. New exact-head AliDDNS and XY-C1 Actions are pending; no local execution is claimed. Physical-router L4 NOT VERIFIED.

---

## 2026-11-17 — Bound AliDDNS polling interval to prevent a keeper busy loop

- Parent main SHA: `29f9b24d3b7ad8fc5f0dd4e856e0d3d06687599a`; result: THIS COMMIT.
- Source/UI review confirmed `aliddns_interval` is a client-side numeric field with a five-character limit, but NVRAM is not itself constrained by the browser keypress handler. The keeper previously ran unquoted `sleep $aliddns_interval`; zero could spin continuously, while multiword or malformed values made sleep fail immediately on each loop.
- Added a POSIX-shell normalizer: accept only decimal digits up to five characters, strip leading zeros, clamp zero to one second, and fall back to 600 seconds for missing/malformed/overlong input. Persist the canonical safe value when it changes; the keeper now calls quoted `sleep "$aliddns_interval"`.
- Extended the production-linked lifecycle regression for default, valid, zero, leading-zero, maximum UI width, overlong, multiword, malformed, metacharacter, negative and fractional values. Static checker pins both the normalization policy and quoted sleep argument.
- Parent `29f9b24d` AliDDNS workflow `44117236166` passed the prior signed API, WAN IPv4/IPv6, DNS and keeper lifecycle tests; this new exact-head interval regression is pending. XY-C1 on parent `40fb7dfe` run `44116901861` was still building; current source needs a new exact-head target build after this commit. No local execution or physical-router L4 is claimed.

---

## 2026-11-17 — Correct AliDDNS interval normalizer output escaping

- Parent main SHA: `b2692cec47bfd7b838fd4fdfda1368a16a088cc2`; result: THIS COMMIT.
- Immediate source review caught that the new POSIX-shell interval normalizer used double-escaped printf format strings, which would emit literal backslash-n text instead of a valid numeric value. This would be rejected by the new lifecycle regression; it was corrected before claiming test success.
- The helper now emits ordinary newline-terminated decimal values. The static source contract also pins the intended one-backslash format and rejects the two-backslash form. Exact-head AliDDNS regression and target build pending.
- No local test/compiler or physical-router L4 is claimed.

---

## 2026-11-17 — Bound PPPoE-MWAN lock contention after stale-lock risk review

- Parent main SHA: `f02edd2564903e26e678e327bfeadcd66cb8823a`; result: THIS COMMIT.
- The detector acquired the shared `/tmp/lock` atomically and removed it during normal EXIT. But SIGKILL or process crash bypasses EXIT cleanup and can leave a stale regular file; later WAN transitions previously slept in an unbounded loop forever.
- Added a 30-iteration one-second bound. On expiry the script returns a visible nonzero status (propagated by the WAN-up/down callers) and never removes or rewrites a lock it did not acquire. This fails closed rather than making an unsafe stale-lock ownership guess.
- Mocked-sleep host regression checks exactly 30 waits, timeout status 1, no firewall/route commands without lock ownership, and byte-preservation of a foreign lock. The WAN source contract pins the timeout and regression marker. Exact-head WAN Actions pending; no local test/firmware build or physical-router L4 is claimed.

---

## 2026-11-17 — Propagate NDIS USB modem firewall rebuild failures

- Parent main SHA: `14d9907941caa59b8515a05b0c8275c3e29d4047`; result: THIS COMMIT.
- Source review found `launch_wan_usbnet()` discarded `start_firewall_ex()` status after bringing up an NDIS modem interface. The normal and RAS WAN startup paths already retain first-stage firewall errors, but NDIS modem startup could return success after a failed firewall rebuild.
- Capture and log the firewall result, retain it as the first modem startup error, and preserve it through later control-session and DHCP setup. Startup continues independent work, but `start_wan()` now receives the failure instead of a false success.
- Added a GNU89 regression for first-error retention and a source-contract check for result capture/order/returns. The USB modem workflow runs that policy binary with ASan/UBSan and executes the WAN startup/firewall source checker.
- Exact-head USB modem Actions and XY-C1 Build/TRX/artifact are pending for THIS COMMIT. No local compiler/build or physical-router L4 is claimed.

---

## 2026-11-17 — Secure PPPoE-MWAN session options and roll back partial launch

- Parent main SHA: `6791d98b116c60a8097ada361d350746b2efa269`; result: THIS COMMIT.
- `mwan.sh` previously interpolated NVRAM username/password into a raw PPP options line, reused `options.wan0` for every PPP session and ignored intermediate macvlan/PPPD failures.
- Replaced it with checked POSIX-shell flow: validate count/VLAN and credentials, escape apostrophes/backslashes like the RC C writer, reject control/newline/oversized values, check interface creation/MAC/up, atomically publish separate 0600 `options.wanN` files and retain the first command error.
- When a later PPPD fails after earlier sessions started, `net_ppp.c` invalidates callback generation and stops the partial PPPD set under the already-held VPN lifecycle lock. Cleanup failure is logged without overwriting the original launch error.
- Added a 12-case host harness and wired it into WAN-route and XY-C1 workflows, with static contracts for the C rollback ordering and credential file policy.
- Isolated draft host harness passed all 12 cases. Full repository Actions/current-source XY-C1 build remain pending; no local C/MIPS compile or physical-router L4 is claimed.

---

## 2026-11-17 — Correct PPPoE-MWAN credential escaping regression expectation

- Parent main SHA: `ab02de4517cd3cd5f4a978964dc0255a9be31adc`; result: THIS COMMIT.
- Exact WAN Actions `44122651031` reached the launcher test after shell syntax passed and private per-session option files were created with correct mode. The test then failed because the expected string represented the credential's original backslash once, but the PPP options format correctly escapes it twice.
- Corrected username and password expected strings to assert the exact escaped output. The isolated launcher host fixture now passes all 12 scenarios (including file modes, quotes/backslashes, newline/control rejection, failure propagation, interface gating, NVRAM and VLAN validation, sync identity and symlink rejection).
- Rerun exact-head WAN-route and XY-C1 regression/source contracts; target Build/TRX/artifact remains unverified. No local MIPS build or router L4 claimed.

---

## 2026-11-17 — Correct PPPoE-MWAN temporary PID suffix source contract

- Parent main SHA: def277350a862c452c2324fd86876aab9bdbaf21; result: THIS COMMIT.
- After the credential expectation correction, exact-head WAN-route run 44123334960 passed all 12 PPPoE-MWAN shell scenarios, then the Python source contract failed because it expected the temporary path to end in one dollar sign.
- Production uses `.$$` to append the current process ID and isolate per-process staging files. Updated the checker to require the exact PID suffix. No production source changed.
- Exact-head WAN-route rerun and XY-C1 Build/TRX/artifact verification are pending.

---

## 2026-11-17 — Preserve BusyBox pending deconfig during DHCP client stop

- Parent main SHA: bae6e8715fa5d1ba583f8fba5601de46ee6aa7d1; result: THIS COMMIT.
- BusyBox udhcpc retries a failed deconfig in DECONFIG_PENDING using its default 20-second -A interval. SIGTERM routes through ret0, which retries pending deconfig; RC stop callers were using a 3-second wait followed by SIGKILL, able to terminate the client before its callback retry.
- Changed all four WAN/AP-LAN udhcpc stop call sites to a bounded 30-second wait with forcekill=0. WAN releases both DHCP clients and stops them separately from ordinary daemons; LAN AP shutdown separates DHCP from detect_wan. A timeout or unknown PID identity aborts interface teardown.
- Extended scripts/check_lan_dhcp_deconfig.py to assert all four paths, no-force-kill settings, stop ordering and BusyBox pending-deconfig retry semantics. Existing WAN DHCP callback and XY-C1 workflows execute the checker and trigger on the changed sources.
- Exact-head WAN-DHCP, WAN-route and XY-C1 Actions are pending; no local build or physical-router L4 is claimed.

---

## 2026-11-17 — Align DHCP stop-order workflow contracts with fail-closed client cleanup

- Parent main SHA: 735847725be8fa488bc54f575ce71514e910d617; result: THIS COMMIT.
- The new udhcpc no-force stop behavior was already covered by scripts/check_lan_dhcp_deconfig.py. Exact WAN-DHCP run 44124204917 and WAN teardown run 44124205356 then failed in older embedded source contracts that still required the previous combined 3-second SIGKILL transaction.
- Updated both workflow contracts to require SIGUSR2 release of both DHCP clients, bounded 30-second no-force udhcpc stop, then the ordinary WAN daemon stop with its existing 3-second TERM/KILL policy, before USB modem stop/interface teardown.
- No production source changed in this commit. WAN-route on source 7358477 passed (44124204908); exact-head WAN-DHCP, teardown and XY-C1 reruns pending. XY parent run 44124205199 was still in its Build/TRX/artifact stage at last poll.

---

## 2026-11-17 — Align Process lifecycle contract with DHCP no-force stop

- Parent main SHA: `246d17ccd13189f267a98db85e6534aad743aa59`; result: THIS COMMIT.
- Exact Process run `44124766115` on 246d17c completed process inventory, PPPD/udhcpc PID identity, service lifecycle, ZeroTier and storage tests successfully, then failed at line 675 because the final checker still expected the removed 3-second force-kill call in AP-mode LAN stop.
- The AP stop path now pauses internet detection, waits up to 30 seconds for the PID-file-bound udhcpc process with `forcekill=0`, then terminates `detect_wan`. On timeout/unknown identity it resumes detection and returns before bridge teardown. Updated the checker to pin this order and failure abort boundary; its stop helper assertion now also verifies the 30-second no-force policy.
- Exact parent DHCP callback `44124766309` and WAN teardown `44124766186` already PASS after the workflow-contract update; related AliDDNS, AliyunDrive, NVRAM, infosvr, AdGuardHome and AI-handoff parent checks PASS. Exact-head Process rerun pending.
- XY-C1 run `44124205199` on product-source SHA `7358477` was IN_PROGRESS at last poll. No product source changed in THIS COMMIT; no MIPS build or physical-router L4 claim.

---

## 2026-11-17 — Release AP LAN DHCP lease before stopping udhcpc

- Parent main SHA: bb64fc406b1f9a20dc2099c7d512c2588d1000c5; result: THIS COMMIT.
- AP-mode LAN udhcpc starts without -R. BusyBox SIGTERM therefore exits a BOUND client without invoking deconfig unless a previous failure left it in DECONFIG_PENDING. The AP stop path had not sent SIGUSR2, so it could leave the lease address/routes configured.
- Centralized release and stop in stop_udhcpc_lan(): validate PID/start-time identity and send SIGUSR2 first, accept only ENOENT/ESRCH when no matching client exists, allow 300ms for release/deconfig, then apply bounded 30-second no-force termination. Unknown PID identity fails closed.
- stop_lan(AP) now calls that helper before stopping detect_wan. Updated LAN DHCP and broad Process source contracts to verify release-before-stop, timeout/forcekill behavior and teardown ordering.
- Exact-head WAN-DHCP, Process, WAN teardown, WAN-route and XY-C1 runs pending. No local build or physical-router L4 claimed.

---

## 2026-11-17 — Correct DHCP stop-source contract after LAN helper consolidation

- Parent main SHA: e64ab93290e20935c0a11ac990612f8dc29fa363; result: THIS COMMIT.
- Exact e64 WAN DHCP run 44125814011 passed its GNU89/ASan/UBSan DHCP lease policy binary, then failed the source checker because it expected four textual `kill_services_with_pidfiles(..., "udhcpc", 0, 30, 0)` statements.
- AP LAN shutdown had been centralized into one stop_udhcpc_lan() helper, so the source now has three unique no-force stop call statements: WAN/VIPTV batch, USB WAN pidfile helper and AP LAN helper. This still covers four logical stop contexts/clients. Updated the count from 4 to 3; production source unchanged.
- Exact WAN-route run 44125814206 SUCCESS. Exact Process and XY-C1 runs were still in progress at last poll; current-source Build/TRX/artifact pending.

---

## 2026-11-17 — Fix Process lifecycle helper extraction test

- Parent main SHA: 30428280cde13499dc0703bb0eb6606398773e12; result: THIS COMMIT.
- Exact Process workflow `44125814201` exercised process inventory, PID-file identity, service lifecycle, ZeroTier, RWFS/storage and prior RC integrations successfully, then failed in the new LAN DHCP helper assertion because the Python source literal searched for a double-escaped `\\n` rather than the function's newline delimiter.
- Replaced the brittle prefix extraction with `split("stop_udhcpc_lan(void)")` and a real newline closing-brace delimiter. No production code changed.
- Exact WAN-DHCP count correction `3042828` run `44126128791` was pending at last poll. Current-source XY-C1 run `44125814296` was still in build/TRX/artifact stage.

---

## 2026-11-17 — Align Process WAN stop contract with DHCP-first no-force teardown

- Parent main SHA: 4bb27f9690a8c16f2a8753fa708727694b33140a; result: THIS COMMIT.
- Process run `44126339259` passed the LAN DHCP release-before-stop, 30-second no-force, and abort-before-bridge-down contracts, then exposed another stale source assertion at line 735 that expected `kill_services_with_pidfiles(svcs_wan, udhcpc_pidfiles, "udhcpc", 0, 3, 1)`.
- Updated Process source contract to require the actual WAN order: SIGUSR2 to WAN and VIPTV, 30-second no-force stop for the udhcpc PIDfile set, then separate normal TERM/KILL stop for other WAN daemons.
- No production source changed. WAN DHCP corrected-count run `44126128791` SUCCESS; WAN-route on e64 `44125814206` SUCCESS; current-source XY-C1 e64 `44125814296` still in Build/TRX/artifact stage.

---

## 2026-11-17 — Align AP DHCP release and WAN service-batch source contracts

- Parent main SHA: d7775129120a04b94dd8f10b5de272bccc46afaf; result: THIS COMMIT.
- Exact e64 HTTPD run 44125813980 failed at an assertion counting only one `kill_services()` batch in `stop_wan_locked()`; the production function has two separately checked batches, for PPP transport children and ordinary WAN daemons. Updated the contract to require both.
- Exact e64 detect_internet run 44125814192 failed because its AP-LAN contract expected the PID-file stop inline in `stop_lan()`. Production now calls `stop_udhcpc_lan()`; the revised contract checks PID-identity SIGUSR2, ENOENT/ESRCH-only absence handling, the 300 ms release window, bounded no-force termination, and abort/resume before bridge ifdown.
- No production source changed. Both workflows include their own workflow path in push filters and are retriggered by this contract commit. WAN-DHCP 44126128791 on 3042828 passed; Process 44126737621 and XY-C1 44126128987 were pending at checkpoint.
- No local MIPS/uClibc compile or physical-router L4 claim.

---

## 2026-11-17 — Repair HTTPD workflow block indentation after contract refresh

- Parent main SHA: e2b5cc184a67b0b25d5ca3dfa8c6f93576adc49c; result: THIS COMMIT.
- The first HTTPD contract refresh inserted two unindented lines inside a YAML literal block. GitHub created run 44127046365 on e2b5cc1 but no job; the embedded Python contract therefore did not execute. Removed the explanatory lines and restored the assertion's block indentation so the workflow remains a valid literal command.
- The sibling detect_internet workflow run 44127050720 on exact e2b5cc1 passed, including the new AP DHCP helper identity/release/no-force and caller abort-order contracts.
- Other e2b5cc1 results: infosvr, AdGuardHome, AI handoff, AliyunDrive and AliDDNS passed; Process, NVRAM and XY-C1 were pending. No production source changed.
- No local MIPS/uClibc build or physical-router L4 claim.

---

## 2026-11-17 — Correct Process and HTTPD DHCP teardown integration contracts

- Parent main SHA: 88d2439710316c70429535fc057dfc9a98f18085; result: THIS COMMIT.
- Exact Process run 44127050578 on e2b5cc1 passed process inventory, PPPD/udhcpc PID identity, lifecycle, ZeroTier, storage, USB modem and the first LAN DHCP helper contract, then failed at a second stale source assertion (line 804) expecting the removed direct three-second force-kill stop call in stop_lan(). Replaced it with checks for SIGUSR2 identity-bound release, ENOENT/ESRCH-only exception, 300 ms wait, 30-second no-force stop and abort/resume before ifdown.
- Exact HTTPD run 44127395058 on 88d2439 passed YAML parsing and earlier lifecycle checks, then failed because its WAN stop contract still expected a combined DHCP/daemon force-kill call. Updated it to require WAN and VIPTV SIGUSR2 release, PID-file-bound no-force DHCP termination, ordinary WAN daemon stop and interface teardown in that order.
- No production source changed. The Process script and HTTPD workflow file are in their push path filters; both exact-head workflows are retriggered.
- No local MIPS/uClibc build or physical-router L4 claim.

---

## 2026-11-17 — Serialize HTTPD lifecycle across PID 1 and watchdog

- Parent main SHA: 1a984b3d889cda4accdb6d6dbc03a283de5b4e6f; result: THIS COMMIT.
- Review found a cross-process race: PID 1 handles RCN_RESTART_HTTPD while standalone watchdog used split stop_httpd()/start_httpd() calls. Concurrent operations could interleave around the process-inventory ABSENT preflight.
- Added stable root-private /tmp/.httpd.lifecycle.lock (O_NOFOLLOW, regular-file/owner/single-link/private-mode validation, bounded F_SETLK wait). Public start and stop serialize complete operations; restart holds one lock across both private helpers, avoiding nested process-associated record-lock unlock.
- Watchdog now calls restart_httpd() once. Added fork-based contention/path-integrity regression and source checks for public wrappers, restart scope and watchdog call path.
- Exact 1a984b3 HTTPD, Process, NVRAM, AliDDNS, AliyunDrive, infosvr, AdGuardHome and AI handoff checks passed; XY-C1 was still building at checkpoint. This production change needs exact-head target Build/TRX verification.
- No physical-router L4 claim.

---

## 2026-11-17 — Update WAN status checker for serialized HTTPD entrypoint

- Parent main SHA: 4c9793229270232501ce3a21cc6f46198bccb6cb; result: THIS COMMIT.
- The exact-head Process regression `44132790766` and XY-C1 host validation `44132791794` stopped at `scripts/check_modem_wan_start_status.py:606`. The checker still extracted `start_httpd()` to assert firewall result propagation, but the cross-process lock change moved the operation body to `start_httpd_locked()`.
- Updated the checker to assert firewall status in the private operation and separately require the public wrapper to acquire the lifecycle lock, call the private helper and release the lock.
- The actual HTTPD security workflow on `4c979` passed, including the new lock test. The exact current-source MIPS build/TRX and Process rerun are pending; no target build result is inferred.
- No production source changed in this follow-up.

---

## 2026-11-18 — Align Process lifecycle contract with serialized HTTPD restart

- Beijing time: 2026-11-18 01:12 (UTC+8). Parent main SHA: 7dac0a147c9c96524b97f9890a6fea21088ec81c; result: THIS COMMIT.
- Exact-head Process lifecycle run 44133242864 on 7dac0a1 passed 45 integration stages, then failed because its watchdog HTTPD assertion still expected separate public stop/start calls. The source now calls the serialized `restart_httpd()` transaction introduced in 4c979.
- Updated the Process integration checker to require the watchdog restart call, nonzero-result diagnostic and the private helper's lock-acquire → stop → start-if-safe → lock-finish ordering. No production source changed.
- Exact-head AliyunDrive lifecycle 44133243141 separately failed in the special-domain scenario and remains under diagnosis; XY-C1 run 44133243104 was pending.
- Local execution is unavailable because this environment has no repository checkout; no target build or physical-router L4 claim.

---

## 2026-11-18 — Fix HTTPD lifecycle workflow function extraction

- Parent main SHA: `f0c5aee9d5980055cb1b409a1c598fd5fdb0bea9`; result: THIS COMMIT.
- Exact-head HTTPD workflow `44136425461` failed at embedded source-contract line 57 after the GNU89/ASan/UBSan daemon policy regression passed. Its generic `body()` lookup matched the forward declaration `static int stop_httpd_locked(void);` and extracted the following unrelated function, so the stop-order assertion never inspected the real helper.
- Anchored extraction on the actual definition signature `static int\\nstop_httpd_locked(void)`, preserving the production stop-order assertions without changing firmware source.
- On the parent SHA, AI handoff workflow `44136425520` PASS; Process lifecycle `44136425549` IN_PROGRESS at checkpoint. Exact-head Actions for THIS COMMIT are pending.
- AliyunDrive special-domain lifecycle run `44133243141` remains under diagnosis. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No target build or physical-router L4 claim.

---

## 2026-11-18 — Bound switch table-engine and search polling

- Parent main SHA: 7ebc2c006799f2fa3b6f0139ad535affa13aa8b0; result: THIS COMMIT.
- Fixed three switch CLI failure modes: VLAN/MAC table-engine busy timeouts previously returned to callers as if successful; DIP/SIP dumps could poll search status forever; MAC table dump silently advanced after its 20-poll budget expired.
- Added shared GNU89 wait policy: VTCR/MAC busy masks clear within 20 × 1 ms; DIP/SIP/MAC search accepts ready/end only after busy clears and within 20 × 5 ms. Timeout/read error logs register and last value, closes the switch socket and exits nonzero before later table operations.
- Host regression covers immediate/delayed idle, busy timeout, read-error propagation, busy+stale-ready, ready/end markers and invalid inputs. Local GNU89 -Wall -Wextra -Werror ASan/UBSan compile and execution PASS.
- Switch CLI CI compiles/tests the policy and checks production wiring; XY-C1 path filter triggers firmware validation for these source files. Exact-head CI pending at commit creation; no MIPS firmware or physical-router L4 claim.

---

## 2026-11-17 — Fail closed on XUPnPD storage provisioning errors

- Parent main SHA: 8307f6abbc7dacade3f4744b277ae404b8096b73; result: THIS COMMIT.
- Found ignored `mkdir()` failures for required XUPnPD config/playlists directories and ignored `doSystem("cp -rf ...")` / `symlink()` results for plugins/profiles. The daemon could launch after required runtime content was not provisioned.
- Added checked directory creation, strict directory validation and exact-source symlink validation for low-storage mode. Copy/link/validation failures are logged and returned before launching XUPnPD; a wrong pre-existing symlink or non-directory is rejected.
- Extended GNU89 host tests for missing-directory creation, symlink/non-directory rejection, expected/wrong source links and missing paths; added a production wiring checker. WAN CI compiles with ASan/UBSan and runs both regressions.
- Parent Switch CLI, Process lifecycle and AI handoff Actions PASS; exact-head WAN/XY-C1 for this commit PENDING. No local MIPS firmware or physical-router L4 claim.

---

## 2026-11-17 — Close remaining XUPnPD copy and cleanup failure paths

- Parent main SHA: 17916d64e66ae90786f8e88838fff459bcfafdf4; result: THIS COMMIT.
- WAN run 44140465411 showed the XUPnPD host regression compiled and passed, but its source checker falsely rejected a checked `doSystem("cp -rf ...")` call. Updated the checker to reject only unassigned command invocations.
- Extended startup fail-closed behavior: validate plugin/profile source dirs; propagate top-level `xupnpd.lua` copy failure and remove partial output; reject pre-existing symlink/non-regular runtime Lua paths; check stale PID and generated Lua override unlink errors except ENOENT.
- Switch CLI, NVRAM, AI handoff and process inventory Actions PASS on 17916d6. Process lifecycle separately failed at a shared-library integration contract; XY-C1 remained in progress. Exact-head WAN retry pending.
- No MIPS firmware or physical-router L4 claim.

---

## 2026-11-17 — Correct Process integration contract for XUPnPD directory setup

- Parent main SHA: 7b1a1ba2e493d9603932322c772d44a27ebc4538; result: THIS COMMIT.
- Exact-head Process lifecycle run 44141514694 failed at one stale source assertion: it expected `xupnpd_config_directory_validate(dir_dst)`, while `start_xupnpd()` correctly uses checked `xupnpd_config_ensure_directory(dir_dst, 0755)` before creating config/playlists and separately validates plugin/profile source and destination paths.
- Updated the integration checker to assert the actual ensure-directory call, its ordering before required subdirectories, and the subdirectory-validation path. No production source or test execution was weakened.
- WAN source/helper workflow run 44141514745 passed on the same SHA, including XUPnPD directory, copy, cleanup and source contract checks. Process exact-head rerun pending; XY-C1 run 44141514685 was still in progress.
- No local MIPS firmware or physical-router L4 claim.

---

## 2026-11-18 — Gate switch table clear completion on bounded status

- Parent main SHA: `558901c1ca847ba491caf68c95f812c95a47b13e`; result: THIS COMMIT.
- Found three remaining fixed-delay paths: `table_clear()`, `dip_clear()`, and `sip_clear()` wrote the ATC clear command, slept 5ms and printed the register without requiring the busy bit to clear.
- Replaced each fixed sleep with the existing 20-poll `wait_mac_atc()` gate. Stuck-busy or register-read errors now follow the existing nonzero/fail-visible CLI path before reporting completion.
- Extended Switch CLI CI source contracts to require write → bounded wait → result read and forbid the fixed 5ms sleep in each clear routine. Existing GNU89 sanitizer policy tests cover timeout/read-failure behavior.
- Parent XUPnPD WAN run `44141514745` PASS; parent Process checker alignment run `44141984432` pending. Parent XY-C1 `44141514685` remained in build.
- No MIPS firmware or physical-router L4 claim.

---

## 2026-11-18 — Restore switch clear diagnostic line endings

- Parent main SHA: 642e7a20e44a654f32170e7c1d23fbd0bdaf2943; result: THIS COMMIT.
- Review of `642e7a20` found three clear-operation diagnostic strings accidentally contained double backslashes, which would emit literal `\\n\\r` rather than the prior line endings. Corrected each to a single C escape sequence.
- Extended the Switch CLI source contract to assert the exact output format in `table_clear()`, `dip_clear()`, and `sip_clear()`; no polling behavior changed.
- Parent Switch CLI, Process lifecycle/inventory, XUPnPD WAN route and NVRAM Actions PASS. Parent XY-C1 Build remained IN_PROGRESS; current rebuild must be checked separately.
- No local MIPS or physical-router L4 claim.

---

## 2026-11-17 — Bound and validate MiniUPnP presentation URL

- Parent main SHA: `895384877b709383abd9e1d3b09d0b689ad157f8`; result: THIS COMMIT.
- `start_upnp()` previously passed unvalidated NVRAM `lan_ipaddr`/`lan_netmask` to `ip2class()`, then used unbounded `sprintf` into `char var[100]` for a custom HTTP port presentation URL.
- Added LAN IPv4 validation and `miniupnpd_format_lan_url()`: GNU89-compatible `snprintf`, explicit port range, truncation/invalid-argument errors, and clearing of partial output on failure. Startup now fails before config publication/daemon launch if these checks fail.
- Added ASan/UBSan tests for default/custom/max ports, oversized/truncated input, null input/output and invalid port; added integration source contract plus Process/XY-C1 path coverage.
- Parent Switch CLI `44143066255` and Process lifecycle `44143066195` PASS; parent XY-C1 `44143066291` remained IN_PROGRESS. New exact-head Actions pending.
- No local target build or physical-router L4 claim.

---

## 2026-11-18 02:20 CST — Gate shutdown MTD persistence on LAN teardown

- `shutdown_router()` previously discarded `stop_lan()`'s result and invoked it only after the MTD persistence decision. In AP mode, this stop can fail closed when the udhcpc PID/start-time identity or deconfig completion is UNKNOWN; in router mode it aggregates conntrack, route, IPv6 and bridge-down errors. Those failures were not part of the persistence precondition.
- Move the single LAN stop into the pre-persistence shutdown sequence, preserve and log its status, and require success before publishing the MTD storage snapshot. Shutdown still proceeds on failure, but persistence is skipped.
- Extend `scripts/check_process_lifecycle_integration.py` to assert one status-capturing stop, error logging, ordering before RWFS/persistence, and fail-closed gate inclusion. Existing Process and XY-C1 workflows already cover the touched paths.
- Exact-head Process/XY-C1 validation: THIS COMMIT / PENDING. No local RC cross-build or physical-router validation is claimed.

## 2026-11-18 02:26 CST — Stop VPN client and server before firmware flash

- `flash_firmware()` held the shared VPN lifecycle lock but only ran the broad L2TP/XL2TP/PPPD stop transaction. Since `stop_services(0)` intentionally skips its `stopall`-only VPN-server hook and does not register the VPN-client hook, OpenVPN client/server daemons and their tunnel-interface cleanup could be missed.
- Add `stop_vpn_server_under_lifecycle_lock()`, matching the existing client entry point, and invoke both lock-owned teardown paths before the residual L2TP/PPP scan. Aggregate the first failure, log per-endpoint results, release the lock, and abort firmware flashing on any incomplete stop.
- Extend the Process source-integration contract to verify both endpoint stops, no nested public lock wrappers, ordering inside the held lock, status propagation, and header registration. XY-C1 and Process workflows include the modified sources.
- Exact-head Process/XY-C1 validation: THIS COMMIT / PENDING. No local MIPS/uClibc firmware build or physical VPN test is claimed.

## 2026-11-18 02:34 CST — Report optional boot launcher invocation failures

- PID 1 previously launched the persistent user startup hook and firmware optional-service autostart with unchecked `system(... &)` results. When the shell could not be started (for example resource exhaustion or a missing/non-executable image), boot continued silently and no diagnostics identified which launcher failed.
- The persistent `/etc/storage/started_script.sh` is optional, so now invoke it only when present and record its background-shell launch status. The ROMFS `/usr/bin/autostart.sh` remains mandatory and now logs launch failure explicitly. Existing per-add-on diagnostics remain unchanged.
- Extended the Process integration contract to pin optional-file handling, both status checks/logs, and CA trust preparation → user hook → firmware autostart ordering. Process and XY-C1 workflows already track `rc.c` and the integration checker.
- Exact-head validation: THIS COMMIT / PENDING. No firmware cross-build or on-device boot test is claimed.

## 2026-11-18 02:41 CST — Propagate IAPP restart failures from wireless restart paths

- Both `restart_wifi_wl()` and `restart_wifi_rt()` previously invoked `system("/usr/bin/iappd.sh restart")` without checking the wait status. The boot path already records this command's failure, but a wireless restart could report success while the Ralink IAPP helper had failed.
- Capture and log the wait status on each band. Preserve any earlier guest-isolation error; otherwise normalize a nonzero `system()` result to `-1` so the existing PID 1 notification handler records the restart as incomplete (its caller checks negative statuses).
- Extend the Process source-integration contract to require both status checks, diagnostics, failure propagation and absence of the old unchecked calls. Process and XY-C1 workflows already track `net_wifi.c` and the checker.
- Exact-head Process/XY-C1 validation: THIS COMMIT / PENDING. No target wireless runtime or physical-router test is claimed.

## 2026-11-18 02:49 CST — Unit-test wireless restart status merging and repair Process checker

- Follow up to the two-band IAPP restart failure propagation: factor the result merge into `wireless_radio_restart_status()`. It maps both `system()` fork errors and positive encoded child wait statuses to a negative RC error, while preserving an earlier guest-isolation failure.
- Extend the existing GNU89 wireless-radio policy test for success, `-1`, positive exit/wait values and first-error preservation. The Process source checker pins use of the helper in both band paths.
- CI exposed a malformed Python regular-expression assertion added with the prior IAPP checker. Exact-head Process run `44150969175` failed at Python line 1116 with `SyntaxError: unterminated string literal`; the C wireless regression, Process inventory, AI handoff and NVRAM tests passed. Replace that fragile regex with an exact command-count assertion here.
- The dedicated `wireless-radio-state` workflow already watches `net_wifi.c`, `wireless_radio_policy.h` and its test; Process and XY-C1 rerun the source checker. Exact-head validation for THIS COMMIT pending.

## 2026-11-17 — Remove orphaned Process checker continuation

- The exact-head Process umbrella `44151820845` on `83909383adb5c2ac3cf9b5fae2760c6b02426088` failed only at its final Python source-integration step; all 46 preceding steps passed. The job log reports `SyntaxError: unmatched ')'` at line 1953.
- Inspection found one detached line `, block)` immediately after the IPv6 boot-status source assertions and before the shutdown source block. It was the orphaned suffix of a previously removed multi-line assertion. Remove that line only; preserve the adjacent IPv6 and shutdown contracts.
- This is a checker-only correction. Exact-head Process rerun is pending; parent wireless, AI docs, Process inventory, iNIC, watchdog, infosvr, HTTPD, AliDDNS, AliyunDrive and AdGuardHome passed. Parent XY-C1 and NVRAM remained in progress at last poll.
## 2026-11-17 — Serialize notification-driven Samba restart with storage eject

- PID 1 routes both `RCN_RESTART_NMBD` and `RCN_RESTART_WINS` through `restart_nmbd()`. The running-smbd branch rewrites `smb.conf`, restarts nmbd and SIGHUP-reloads the captured smbd processes; the no-smbd branch stops nmbd and starts WINS. Unlike `restart_smbd()`, this notification route lacked the common storage-operation lock.
- A notification could race safe eject: storage teardown holds the lock while stopping Samba and unmounting shares, but unguarded `restart_nmbd()` could start/reload daemons against mounts being removed. Take the same cross-process eject barrier before first process inventory and retain it across config publication, stop/absence checks, launch/readiness, smbd identity-bound reload and WINS startup. Lock failure now aborts fail-closed.
- Add Samba workflow source assertions for lock acquisition preceding inventory, no process/config side effect before lock success, lock retention through transition calls and exactly one unlock. Workflow already includes `services_ex.c` in path filters.
- Checker-repair parent `85c02c4` had Process exact-head run `44152539523` still in progress at last poll; all of its tracked source checks were not yet conclusively green. THIS COMMIT exact-head Samba/Process/XY-C1 checks pending. No MIPS/uClibc or router/L4 validation is claimed.
## 2026-11-17 — Block physical-unplug unmounts when swapoff is not confirmed

- Safe removal already performs a full swapoff preflight and refuses all unmounts if a selected swap is active but cannot be disabled or `/proc/swaps` is UNKNOWN. The physical hotplug cleanup `umount_ejected_locked()` only compared `/proc/mounts` with `/proc/partitions`; it could unmount stale paths after the mdev swapoff attempt had failed.
- `report_device_swapoff()` previously returned void, so `mdev_sd_main()`/`mdev_mmc_main()` always reported success even when `try_device_swapoff()` returned failure. It now returns the checked status; both handlers still queue the root-device unplug notification but return failure to their caller when swapoff was not confirmed.
- The deferred physical-eject worker now rechecks every stale mounted partition's swap state before stopping services and before any unmount. Any failed/UNKNOWN swapoff blocks the whole unmount batch, leaves storage daemons stopped, returns failure and uses the existing `usb_unplug_ms` cleanup retry path.
- Existing GNU89 `storage_ejected_may_unmount_device()` test covers swapoff failure; extend storage workflow source contracts to pin mdev propagation and swap-preflight → stop → gate → unmount order. Exact-head CI pending; no target firmware or L4 result claimed.
## 2026-11-17 — Propagate SD/MMC automount helper failures

- `mdev_sd_main()` and `mdev_mmc_main()` previously discarded the failure result from `_eval("/sbin/automount.sh", ...)`: they merely skipped the hotplug notification and still returned 0, making a failed mount appear to be a successful device event.
- Capture the helper status for both buses, log nonzero results and return a failing mdev status when either the automount command or removal-time swapoff check failed. `on_hotplug_mass_storage` remains emitted only after successful automount.
- Update the existing storage workflow source-integration contract to require captured `_eval` result, success-only notification and failure propagation. Exact-head CI for THIS COMMIT pending; parent USB/Process/XY validation had not completed at last poll.
## 2026-11-17 — Propagate unknown partition inventory from whole-device hotplug

- Both `mdev_sd_main()` and `mdev_mmc_main()` treated any nonzero `check_root_partition()` result as the same no-op. The helper returns 1 when partitions exist, but -1 when `/proc/partitions` cannot be read or validated; the latter was being reported as successful device handling.
- Preserve the intended partitioned-disk no-op, mount as partition 1 only when the result is 0, and log/return mdev failure when the inventory result is negative. Include the topology result alongside swapoff and automount statuses.
- Extend storage workflow source contracts to require explicit negative and positive branches and the combined final event status. Exact-head storage CI pending; no target firmware/runtime claim.
## 2026-11-17 — Align HTTPD's duplicate storage check with tri-state hotplug topology

- Exact-head HTTPD run `44155697386` on `e90549004908d388123e1807abf0ddeb4a6a11fd` passed its preceding authentication, NVRAM transaction, wireless publication, USB modem metadata and other security checks, then failed at `Verify block-storage whole-disk partition gate`.
- Its duplicated source assertion still required `if (check_root_partition(...))`, which was superseded in `hotplug_stor.c` by explicit tri-state handling. The dedicated USB storage workflow on the same SHA already passed the new check.
- Replace the stale HTTPD contract with assertions for SD/MMC result capture, negative UNKNOWN rejection, positive partitioned no-op, topology failure aggregation and absence of the old ambiguous check. Workflow/docs-only correction; exact-head rerun pending.
