## 2026-11-17 21:03 CST — Align HTTPD boot-directory contract with required_dir

- Parent main SHA: `ab22c03fb3a8acd37004d52b5b3bcfd55dd1e461`; result: THIS COMMIT.
- HTTPD umbrella run `44088845536` reached its notification queue contract and failed because it expected inline `mkdir -p -m 700` calls that had already been replaced by the checked `required_dir 700 ... || exit 1` helper. Updated the contract to validate the current required-directory order and checked `mdev -s` gate.
- Removed the duplicate unchecked `chmod 700 /tmp/rc_notification /tmp/rc_action_incomplete` command: the preceding `required_dir` calls already create each directory and fail closed if its permission change fails. The focused PID-1 boot checker now guards against reintroducing this redundant unchecked operation.
- Parent exact-head Process `44088845758`, NVRAM `44088845655`, USB storage `44088845567`, AI docs `44088845461`, AliDDNS `44088845627`, AdGuardHome `44088845523`, AliyunDrive `44088845392`, and infosvr `44088845694` passed. XY-C1 `44088845842` was still building at checkpoint. New exact-head HTTPD, Process, and XY-C1 Actions pending.
- This slice removes only a redundant chmod already performed by a checked helper and repairs the stale contract; no target or hardware result is claimed. Local host tests NOT RUN in this connector-only runtime; physical-router L4 NOT VERIFIED.

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

## 2026-11-17 19:56 CST — Fail closed when Inadyn cache preparation fails

- Parent main SHA: `8a77982a142c4d55dbad874fee8c92823dfbe3d7`; result: THIS COMMIT.
- Confirmed defect in `services_ex.c:start_ddns()`: the `rm -rf /tmp/inadyn` result for an explicit cache reset and the `mkdir()` result were both ignored, so stale cache could survive a failed reset and the daemon launch could continue without a usable cache directory. Both direct callers preserve the function result (`rc.c` applet returns `ret`; `notify_ddns_update()` returns `start_ddns(0)`).
- Added an injected cache preparation policy: a requested reset is mandatory; `system()` wait statuses are normalized (including exit status 256 -> EIO, avoiding an apparent success when narrowed by the process exit code); mkdir errors abort except EEXIST followed by `lstat()` proving an actual directory. Files and symlinks are rejected. Configuration publication and Inadyn launch occur only after successful cache preparation.
- Added GNU89/Werror/ASan/UBSan regression cases for reset success/failure, raw system wait-status normalization, mkdir errors, existing directory, regular-file/symlink collisions, lstat failure and invalid callbacks. Extended the existing Inadyn production wiring contract; its workflow already covers all changed source/test/checker paths.
- No local checkout/host compiler was available in this session; local L2 test NOT RUN. Exact-head Inadyn Actions are triggered by this commit and must be checked by exact `head_sha`; no target MIPS/uClibc or physical-router validation is claimed.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.

---

## 2026-11-17 19:14:54 CST — Normalize router source before checking Optware status ordering

- Parent main SHA: `448cb4e6ad854228204100d5fb06f8f4135fd918`; result: THIS COMMIT.
- Preflight of the new RWFS integration assertion caught a checker-only bug: it used a whitespace-normalized expected call string against the unnormalized multiline `router_init` body. Switched the call-order comparison to the existing `router_compact` source, preserving the actual ordering assertion without depending on tabs/newlines.
- No production code changed in this correction. The Optware initial worker launch status is still returned, aggregated into `boot_services_status`, and retry scheduling remains unchanged.
- Exact-head Process and XY-C1 for the Optware change remain pending/in progress at this checkpoint; the checker correction is included before declaring those results.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; physical-router L4 NOT VERIFIED.

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
## 2026-11-17 19:07:56 CST — Correct guest boot rollback checker formatting and snapshot-order contracts

- Parent main SHA: `85062ff1a9a0d8b633d8c11b1d90cb7d6a18c2a7`; result: THIS COMMIT.
- Exact-head Process run `44070663265` on source `1be4affc` compiled and passed the GNU89 ASan/UBSan wireless guest-isolation policy binary. The production checker then failed at line 147 because it expected a two-line C function call as one exact string; a second assertion still expected the previous no-argument link-state API.
- Updated checker assertions to validate the multiline RT/WL rollback arguments and the current checked status-returning snapshot calls, while preserving ordering checks through isolation and rollback. No production source changed in this checker-only repair.
- AI handoff documentation run `44070663411` passed on `85062ff1` after restoring the 117-entry archive; `dev_log.md` remains append-only and retains its complete prior history.
- Exact-head Process/Wireless reruns and XY-C1 current-source build need checking after this commit; physical router L4 remains NOT VERIFIED.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---
## 2026-11-17 19:04:15 CST — Restore append-only development log archive

- Parent main SHA: 1be4affc0b533639d604515b0d0b98545dcdb02f; result: THIS COMMIT.
- The exact-head AI handoff documentation check 44070330879 failed because the preceding source commit replaced the entire dev_log.md with only the newest record, leaving one entry instead of the required 20 complete newest entries.
- Restored the complete 117-entry archive from parent 9bb789d0, then prepended the current guest link-state/attempt-owned rollback milestone. This is an append-only history correction; no source code or test behavior changed.
- On source SHA 1be4affc, Wireless radio 44070331034 and Process lifecycle 44070330981 were still in progress, XY-C1 44070330756 was in progress, and AI docs 44070330879 failed solely on the truncated archive.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807; physical-router L4 NOT VERIFIED.

---
## 2026-11-17 — Propagate guest bridge interface-state inspection failures

- Parent main SHA: `801d1bbfaf66448ea98a4f6ef0371f53da9c8ac5`; source and source-contract correction committed atomically in THIS COMMIT.
- `restart_guest_lan_isolation()` used `is_interface_up()` for 5GHz guest, 2.4GHz guest-policy input and the actual 2.4GHz/iNIC bridge port. Failed interface-flags ioctls were treated as DOWN, silently skipping isolation configuration.
- Replaced all three reads with `wireless_guest_interface_state_checked()`; record inspection errors and skip only the operation whose state is unknown while continuing independent band/cleanup work.
- Updated the source checker: rollback callbacks already use the shared checked reader; prior exact-head Wireless run `44069490310` and Process run `44069490305` failed on the stale assertion at line 110, after compiled policy regression passed. New exact-head runs must verify the corrected contract.
- Parent XY-C1 `44069490610` and NVRAM `44069490286` were pending; recheck against this commit. No L3/L4 claim.

---
## 2026-11-17 — Use checked interface state throughout guest controls

- Parent main SHA: `5c5300876827a62a99bd2d5ac82d98853b5a230d`; result: THIS COMMIT.
- Explicit guest enable/disable previously used `is_interface_up()`, which masks `get_interface_flags()` errors as down, to decide whether the main AP was active and whether the guest VIF was already up.
- Added a shared checked-state reader and used it for main/guest interface decisions in both guest controls. Guest activation refuses to proceed when required state cannot be read. Guest-disable records unknown state but still attempts ifconfig-down and bridge detach, then reapplies isolation.
- Rollback callbacks use the same checked reader. Process source contract checks the helper and confirms both controls avoid unchecked `is_interface_up()`.
- Exact-head parent `5c530087`: Process run `44069081348` was pending at last poll; AdGuardHome `44069081212` passed; remaining workflow runs pending. This commit requires fresh Process/Wireless results.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---
## 2026-11-17 18:53:50 CST — Correct Process source contract for direct AP-start status variables

- Parent main SHA: `89f232e6a5e1ed445129dc18a45356c148e71c14`; result: THIS COMMIT.
- Exact-head Process workflow run `44068630031` failed in `Verify LAN startup error propagation through PID 1 and LAN restart`. Guest isolation policy/source checks passed earlier in that job. The failing assertion expected direct `start_wifi_ap_rt/wl()` calls to assign the generic `operation_status`; the new direct-boot rollback records AP startup separately as `rt_ap_start_status` and `wl_ap_start_status` so it can distinguish AP startup failures from WDS/APCLI operations.
- Updated only the workflow's embedded source contract to accept dedicated AP status variables while retaining generic operation-status checks for WDS/APCLI. No production-source change was needed for this failure.
- Exact-head on `89f232e6`: Wireless radio `44068630167` PASS; AI docs `44068629955` PASS; Process inventory `44068630249` PASS; Process lifecycle `44068630031` FAILED at the stale assertion described above; XY-C1 `44068629924` was still IN_PROGRESS at this checkpoint.
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

## 2026-11-17 — Keep guest AP rollback link-state inspection fail-closed

- Parent main SHA: `776bd2af8e84ec7c4b87b336a31158ab09c69ae3`; result: THIS COMMIT.
- The rollback callbacks used `is_interface_up()`, which returns 0 for both a confirmed-down interface and any `get_interface_flags()` socket/ioctl failure. That made a failed link-state query indistinguishable from down and could falsely skip interface-down and pass final rollback verification.
- Added `wireless_guest_isolation_link_state()`: negative flag reads return `-EIO`; only valid flag values can produce up/down; invalid output pointer returns `-EINVAL`. Both rollback bring-down and verify-down callbacks now use `get_interface_flags()` and propagate query failure.
- Added GNU89-compatible unit cases for flags-read error, confirmed down, confirmed up and null output. Extended the Process source contract to ensure both production callbacks use the checked helper and never fall back to `is_interface_up()`.
- Exact-head parent `776bd2af`: Process lifecycle `44067663998` PASS (includes guest isolation/M2U source checker and C regressions); NVRAM `44067664059` PASS; AdGuardHome `44067664276` PASS; AI handoff `44067664224` PASS; AliyunDrive `44067664006` PASS; infosvr `44067664305` PASS; AliDDNS `44067664325` PASS. XY-C1 source run `44066355563` for `69e4ab00` completed SUCCESS; Wireless radio run `44066355414` for `69e4ab00` completed SUCCESS. These builds validate the prior M2U source commit, not this new tri-state change.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---
## 2026-11-17 — Ensure guest Wi-Fi source checker runs on pushes

- Parent main SHA: `080b8c91cb636fa95a8511c83b552bd4e639ba15`; result: THIS COMMIT.
- Process exact-head run 44066722261 on e7e3f210 still failed at the corrected M2U source assertion because the helper expected a leading `int\n` not present in the source declaration. Commit 080b8c91 fixed the checker to use the actual function-name signature and function_body().
- The follow-up 080b8c91 changed only the checker and did not trigger Process CI: scripts/check_guest_lan_isolation_status.py was included in pull_request.paths but missing from push.paths. Add the path so the corrected integration contract is exercised on push.
- No passing Process rerun after checker correction exists yet; do not record the M2U checker as verified until this commit's exact-head Process run completes.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---
## 2026-11-17 — Repair M2U startup checker after first CI failure

- Parent main SHA: `69e4ab000835efe5f5107fbfdce696c68cf95706`; result: THIS COMMIT.
- Exact-head Process lifecycle run `44066355293` failed in `Compile and run guest Wi-Fi isolation status regression`; log identified `TypeError: 'str' object is not callable` at the new M2U startup contract. An earlier loop had shadowed the name `body`, so the new code invoked that string instead of the existing `function_body()` parser.
- Corrected the call to `function_body()` and used the actual startup function signatures (the source has `int ` before the newline). No production source change was required for this regression failure.
- On exact `69e4ab00`, Wireless radio state `44066355414` and Process inventory `44066355561` passed; Process lifecycle `44066355293` failed as above; XY-C1 `44066355563` was still running.
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

## 2026-11-17 — Propagate guest Wi-Fi CLI failures to the caller

- Parent main SHA: `5d7461e85ade47a6305b998a14657f12dd15cd63`; result: THIS COMMIT.
- Source audit found that the four `radio2_guest_enable/disable` and `radio5_guest_enable/disable` CLI branches discarded the return from `manual_change_guest_*`. Those functions already return a negative value on guest AP/isolation failure, but `main()` initialized `ret = 0` and therefore silently reported success to scripts and shell callers.
- Each branch now maps negative control status to `-1`, while successful unchanged (`0`) and changed (`1`) results both map to CLI success. Added integration assertions for all four branches and the final `return ret;`.
- Exact-head on parent `aec5cc95`: Wireless radio state `44065308208`, Process lifecycle `44065308432`, AI handoff `44065307813`, AdGuardHome `44065308334`, infosvr `44065308038`, AliyunDrive `44065308080`, AliDDNS `44065307886` passed; NVRAM `44065307934` was still running at checkpoint. XY-C1 on source `04b98092` was still in progress.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Fix wireless-radio regression workflow shell continuation

- Parent main SHA: `04b98092eb68bc0c363462859a687fa235027042`; result: THIS COMMIT.
- Exact-head Wireless radio state run `44064716798` passed the existing wireless and bridge policy unit tests, then failed before compiling the guest rollback unit because the inserted three-line compiler command had two literal trailing backslashes. The shell treated a standalone backslash as an input file (`ld: cannot find \\\\ `).
- Corrected the three compiler continuations to one literal backslash; no production source change is required for this CI failure.
- On exact `04b98092`, Process lifecycle `44064716995`, HTTPD `44064716837`, AdGuardHome `44064716880`, AliDDNS `44064716827`, infosvr `44064716822`, iNIC `44064716795`, AliyunDrive `44064716760`, AI handoff `44064716914`, and watchdog `44064716964` passed. XY-C1 `44064716886` and NVRAM `44064716848` remained in progress at last poll.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

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

## 2026-11-17 — Update HTTPD security source contract for wireless restart failure propagation

- Parent main SHA: `56fb66fb68ed46bd8389c2dbdd4009243023f52c`; result: THIS COMMIT.
- Exact-d3 wireless-radio regression was repaired by updating its stale interface-failure assertion; exact follow-up run `44061356170` passed. Exact-56fb Process lifecycle run `44061356114` passed, including guest-isolation unit and source-contract checks.
- Exact-d3 HTTPD security run `44060465977` failed in its final static source checker: it still required `return 0;` from `restart_wifi_wl/rt`. The guest-isolation change correctly returns `isolation_status` to propagate policy errors; update the HTTPD checker to require that result instead. Prior HTTPD C and Ralink atomic-publication tests passed.
- Exact-d3 XY-C1 Build was still running at the last checkpoint; no TRX/artifact result recorded here. This commit's exact-head Actions pending.
- Next guest-Wi-Fi task: implement rollback of only newly enabled guest AP interfaces when bridge, multicast or isolation configuration fails; preserve pre-existing interfaces and account for RT3352 MII/iNIC VLAN bridge membership.

---

## 2026-11-17 — Align wireless-radio regression with fail-closed guest-control status flow

- Parent main SHA: `d3be26d0590ae05c20422db624d23868809c67b7`; result: THIS COMMIT.
- Exact-d3 Process lifecycle `44060466306` passed, including the guest-isolation GNU89/Werror/ASan/UBSan policy unit and the integration checker. AI handoff `44060466402` passed.
- Exact-d3 wireless-radio run `44060466245` failed at its inline source assertion, not at compilation or the existing radio/bridge unit regressions. The stale assertion required `return 0;` after an ifconfig failure. Guest-control now records the failure and jumps to its cleanup/isolation label before returning a negative status; update the checker to verify that ordering and status record instead.
- XY-C1 exact-d3 run `44060466380` was still building at checkpoint; do not claim its TRX/artifact checks yet.
- Next: implement rollback of a newly enabled guest AP if bridge, M2U or isolation policy setup fails, without taking down a guest interface that was already up. Handle RT3352 MII/iNIC ownership separately.

---

## 2026-11-17 — Propagate guest Wi-Fi isolation and bridge-operation failures

- Parent main SHA: `d0ea67338241d47223dbf2ae434411dfeb53f242`; result: THIS COMMIT.
- Guest Wi-Fi control previously ignored `br_add_del_if()`, multicast-to-unicast bridge-port setup, `brport_set_param_int()`, ebtables flush/delete/rule installation, and ebtables module load/unload results. A guest interface could therefore be reported enabled despite an incomplete LAN isolation policy; PID-1 also ignored all six guest-control notification results.
- Added first-error/failure-count policy and host regression. The AP ebtables rule builder and cleanup now record each command; filter modules are retained if flush/delete fails. Bridge-port writes are attempted only for interfaces currently up, avoiding failures caused by disabled guest interfaces.
- `restart_guest_lan_isolation()` now returns a negative failure status. Initial bridge setup records it; full radio restarts return it; guest-control APIs preserve the existing 0/1 unchanged/changed success convention but return negative on bridge/multicast/isolation failure; PID-1 notification handlers retain those failures.
- Parent d0 exact-head Actions were 15/15 SUCCESS: Process lifecycle `44056030149`, NVRAM `44056029909`, AI handoff `44056030069`, and XY-C1 `44056030091` all passed. XY-C1 completed Build, TRX verification and artifact upload. New exact-head checks are pending after push.
- Host unit test and source integration checker are included. This does not yet fail-close an already enabled guest interface after isolation failure; next task is safe interface rollback. Target MIPS/uClibc, DBDC, RT3352 MII/iNIC and physical-router L4 remain NOT VERIFIED.

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

## 2026-11-17 — Scope LAN restart order assertion to the nested WAN restart

- Parent main SHA: `0b2168e17833d5ae8e4223f34fcecd0e7a5df82f`; result: THIS COMMIT.
- The production fix and expanded GNU89/ASan/UBSan LAN restart regression passed in Process run `44054425453`; the final source-contract step alone failed because a raw `restart.index("start_vpn_server()")` matched an earlier DNSMasq-lock recovery path.
- Scope the VPN start index to the position after the nested `full_restart_wan()` call, preserving the intended order check without changing production code.
- On parent `0b2168e1`, WAN route `44054425489`, WAN DHCP `44054425743`, Process inventory `44054425531`, AI docs `44054425761`, AdGuardHome `44054425537`, infosvr `44054425744`, detect_link `44054425584`, and detect_internet `44054425871` passed; XY-C1 `44054425508` and NVRAM `44054425712` remained in progress at checkpoint. Fresh exact-head validation is required.
- Frozen backup unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Preserve LAN restart bridge, PHY and WAN restart results

- Parent main SHA: `a0a5c7b5330d601032e83f1800fe7873612cc013`; result: THIS COMMIT.
- Review confirmed the boot path already aggregates `init_bridge()`, `start_lan()` and PID 1 status. The remaining concrete gap was in `full_restart_lan()`: it discarded status-returning STP, bridge forward-delay, LAN PHY power-down/up, STP/forward-delay restoration and nested `full_restart_wan()` calls.
- Routed each result through `lan_restart_record_status()`, preserving first-error semantics while still attempting independent recovery operations and VPN startup.
- Expanded `service_start_status.c` to cover a failure through the complete restart recovery sequence and verify every later operation is still attempted. Strengthened the Process workflow semantic contract to verify all production result captures and call ordering.
- Parent exact-head validations all passed: WAN route `44051968487`, WAN DHCP `44051968728`, Process inventory `44051968717`, Process fail-closed `44051968786`, NVRAM `44051968689`, and XY-C1 `44051968495`; XY-C1 Build, TRX and artifact upload succeeded. This commit's exact-head checks are pending.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains NOT VERIFIED.

---

