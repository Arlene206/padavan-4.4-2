## 2026-11-09 12:10 CST — Stage storage eject inventories and propagate cleanup failures

- **Base main SHA:** `692b7b2372dd4ccabab879a53ddcd78aad69e751`.
- **Implementation:** THIS COMMIT — stage one complete `/proc/partitions` snapshot before any ejected-device unmount; propagate `umount_ejected()` failure into PID 1 notification completion.
- **Finding:** `umount_ejected()` already staged all of `/proc/mounts`, but reopened and reparsed `/proc/partitions` for each mount. A later partition-inventory read/parse/close failure could therefore be discovered only after earlier mountpoints had already been unmounted. The `on_unplug_mass_storage` handler also discarded the cleanup return value, causing the queue to mark a failed operation as completed.
- **Change:** parse and validate every partition row once, including the procfs heading, exact field count, numeric major/minor/block fields, newline termination, read/close status and allocation outcomes. On any preflight failure, leave all staged mountpoints untouched. Compare the staged names during cleanup instead of reopening procfs. PID 1 now retains the incomplete notification marker when safe-eject cleanup fails; the failure log is generic for all notification types.
- **Regression:** extend `storage_ejected_inventory_policy.c` for valid/header/blank/malformed/truncated/extra-field partition records; add `storage-hotplug-stability.yml` to compile/run the GNU89 host test and assert preflight ordering, single partition scan and notification error propagation. Exact-head execution is pending.
- **Validation boundary:** L1 source/caller review PASS. L2 host regression / Actions PENDING on THIS COMMIT. No production MIPS/uClibc RC build, full firmware image or physical USB/filesystem test is claimed.
- **Next:** verify the storage-hotplug workflow and AI documentation contract on the exact resulting `main` SHA, then continue the caller/concurrency audit across `mdev_sd_main()`, `mdev_mmc_main()`, `safe_remove_usb_device()`, eject commands and per-device lock identity. USB/storage remains PARTIAL / AUDITING.

---

## 2026-11-09 12:05 CST (UTC+8) — Reconcile withdrawn DHCP router options during renewal

- **Base main SHA:** `d96f8df72fe36afcd7875d30ca70228151085a83`; **implementation commit:** `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`.
- **Repository:** maintained fork `Arlene206/padavan-4.4-2`.
- **Finding:** BusyBox udhcpc exports only DHCP options present in the ACK. A server that withdraws option 3 (Router) on renewal leaves `getenv("router") == NULL`. The old WAN and IPTV renew callbacks guarded router comparison with `if (value)`, so neither reconciled a withdrawn gateway and the old default route could survive.
- **Fix:** add `dhcp_lease_router_is_changed()` to `dhcp_lease_policy.h`, normalizing missing/empty router options to an empty gateway; invoke it unconditionally from WAN and IPTV renew callbacks in `net_wan.c`. A changed/withdrawn router enters the existing bound path, which removes old lease routes and clears the gateway. Add host cases for absent, empty, unchanged, changed and invalid options; extend workflow source contracts for both callers.
- **Exact-head verification:** [AI handoff contract 42559613261](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613261), [WAN teardown 42559613131](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613131), [HTTPD/DHCP regression 42559613119](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559613119) and [WAN route status 42559612983](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559612983) all SUCCESS on exact implementation SHA `4e9614b5b40ff602423da3488d1aa6dfe859d4d4`. The HTTPD job's DHCP policy test and normalization/renew source-contract step both reported OK.
- **Verification boundary:** L1/L2 PASS for this scoped path. MIPS/uClibc RC/full firmware (L3) and router DHCP withdrawal/route behavior (L4) NOT VERIFIED.
- **Next:** verify the AI handoff contract on the new governance/archive commit's exact SHA, then rotate to USB/storage hotplug caller/concurrency audit. Keep DHCP LAN and deconfig/leasefail/NAK/release coverage open.

---

## 2026-11-09 12:04 CST (UTC+8) — Restore canonical roadmap archive shard and repair archive inventory

- **Repository:** `Arlene206/padavan-4.4-2`.
- **Recovery:** the prior handoff referenced `docs/roadmap-history/2026/11/part-008.md`, but that canonical path was absent. Recovered the exact Git blob `622163bfb030d1d7fbdf7e4bc844a14ace7df1d0` from its misplaced nested copy and restored the standard indexed path in commit `8bcde43b62bd8c2116a2023ee48c3cdc04ee61a3`.
- **Correction:** the misplaced nested duplicate was not indexed and caused the follow-up archive inventory check to fail. Removed only `docs/roadmap-history/archive/2026/11/part-008.md` in commit `d96f8df72fe36afcd7875d30ca70228151085a83`; the canonical standard-path archive remained intact and byte-identical.
- **Verification:** [AI handoff contract 42559250321](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42559250321) SUCCESS on exact correction SHA `d96f8df72fe36afcd7875d30ca70228151085a83`; log confirms `root log=20 entries; archive shards: dev-log=5, handoff=9, roadmap=8`.
- **Next:** preserve the archive/index invariant during this checkpoint rotation; always compare archive inventory with index rows, not only blob contents.

---

## 2026-11-09 11:40 CST (UTC+8) — Restore required handoff verification heading after contract failure

- **Base main SHA:** `0810b15526a7c6c7b7ba5980679c62fd522f481a`; resulting documentation-only commit: THIS COMMIT.
- **Wireless code regression:** [42557455460](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455460) SUCCESS on exact code SHA `0810b15526a7c6c7b7ba5980679c62fd522f481a`; the workflow compiled and ran the GNU89 policy test and passed the wireless source contracts for iNIC recovery, interface-down aggregation, MII teardown and LED publication. Dependent HTTPD regression [42557455207](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455207) also SUCCESS on that exact SHA.
- **Documentation contract finding:** [42557455144](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42557455144) FAILED on `0810b15526a7c6c7b7ba5980679c62fd522f481a` because `SESSION_HANDOFF.md` used `## Validation boundary`; `scripts/check_ai_handoff_docs.py` requires the literal heading `## Verification boundary`. No archive/hash/count error was reached in that run.
- **Correction:** restore the required heading without changing the substantive checkpoint; preserve the superseded handoff and roadmap byte-for-byte, update indexes, and rotate the oldest complete root log record to the active shard.
- **Validation:** wireless L2 Actions PASS on exact code SHA; docs L2 correction is PENDING until a fresh exact-head contract run. No target RC/MIPS firmware or hardware check.
- **Remaining:** current docs run must pass on the resulting main SHA. Wireless driver/switch consumers and board-specific runtime remain open; FULLCONENAT mapping namespace scope remains a distinct P2 audit.
- **Next:** verify the fresh AI handoff documentation contract on exact live main. Then resume DNS/DHCP client lifecycle error handling.
- **Result:** THIS COMMIT.

---

## 2026-11-09 11:21 CST (UTC+8) — Fix wireless interface-down result handling and LED state publication

- **Base main SHA:** `91b3bb41efe0f6ad11d18f8a19d9df3592d7c7b6`; resulting source/documentation commit: THIS COMMIT.
- **Module / priority:** wireless management and RT3352 iNIC lifecycle, P2 stability. Whole-module audit remains open.
- **Findings:** `check_inic_mii_rebooted()` discarded the return from the periodic iNIC guest-interface-up command, so a failed recovery was indistinguishable from success. `inicd.c` invokes the check from its SIGALRM timer handler, making this an opportunity for an explicit retry on the next health-check tick. `stop_wifi_all_wl()` and `stop_wifi_all_rt()` ignored every interface-down result and switched the radio LED off even if a netdevice remained up. The RT3352 MII startup path brings up `IFNAME_INIC_MAIN` and optionally `IFNAME_INIC_GUEST`, but the old stop path attempted the non-MII 2.4GHz APCLI/WDS/AP names instead of tearing down those iNIC interfaces.
- **Fix:** route the health-check guest-up through `wif_control_success()` and explicitly defer recovery to the next periodic check on failure. Add a sticky shutdown-result policy; every supported interface-down is attempted and checked, a later success cannot erase an earlier failure, and the LED is switched off only if all required shutdown operations succeeded. The MII path now stops the actual iNIC guest/main interfaces, removes guest VLAN bridge membership, and reports isolate/RGMII-disable failures without short-circuiting cleanup.
- **Changed paths:** `trunk/user/rc/src/net_wifi.c`; `trunk/user/rc/src/wireless_radio_policy.h`; `trunk/user/rc/src/tests/wireless_radio_policy.c`; `.github/workflows/wireless-radio-state.yml`; compact root handoff/roadmap/log plus lossless archives/indexes.
- **Regression:** extended GNU89 host policy test with sticky shutdown-failure aggregation; added source contracts for periodic iNIC recovery status, every stop attempt, MII teardown and LED-off ordering. Exact-head wireless and documentation Actions are PENDING at commit creation.
- **Validation boundary:** L1 source/caller review PASS for the scoped health-check and shutdown paths. L2 host compile/test and Actions PENDING until the new exact-head run completes. No MIPS/uClibc RC build, complete firmware/TRX, live radio restart or hardware test claimed.
- **Remaining scope:** driver/switch consumers, other board-conditional lifecycle paths and concurrent restart/stop behavior remain open. Firewall/NAT remains AUDITING; FULLCONENAT mapping hashes and the dying-tuple queue are still module-global and not namespace-keyed.
- **Next:** reconcile wireless and AI handoff documentation workflows against exact live main SHA. Once green, rotate to DNS/DHCP client lifecycle error handling; retain FULLCONENAT mapping namespace isolation as a separate P2 audit.
- **Result:** THIS COMMIT.

---

## 2026-11-09 11:17 CST (UTC+8) — Confirm FULLCONENAT per-net notifier regression and handoff archive CI

- **Base main SHA:** `91cae3d19bb72a9e0cebcad509db256aded52467`; this documentation-only follow-up records the exact-head results for the code commit.
- **FULLCONENAT lifecycle regression:** [run 42555493213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555493213), SUCCESS on exact source commit `91cae3d19bb72a9e0cebcad509db256aded52467`. Its updated step, “Verify FULLCONENAT per-net notifier ownership and cleanup”, passed the per-net registry/refcount, both notifier API layouts, registration/unwind, owning-net unregister, RCU grace period, IPv4/IPv6 rule destructor/module reference order and two-namespace lifecycle assertions.
- **AI handoff documentation contract:** [run 42555491302](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42555491302), SUCCESS on exact source commit `91cae3d19bb72a9e0cebcad509db256aded52467`. The executed `scripts/check_ai_handoff_docs.py` reported: `PASS (root log=20 entries; archive shards: dev-log=4, handoff=6, roadmap=5)`.
- **Code commit:** `91cae3d19bb72a9e0cebcad509db256aded52467` — `fix(netfilter): scope FULLCONENAT notifiers per netns`.
- **Validation boundary:** L1 source review PASS; L2 focused source-contract and documentation-archive Actions PASS on the exact code commit. These workflows do not compile the kernel module or execute conntrack/NAT in a live namespace. No local kernel object, MIPS/uClibc, full firmware/TRX or hardware test is claimed.
- **Open risks:** Firewall/NAT stays AUDITING. FULLCONENAT IPv4/IPv6 mapping tables and its dying-tuple queue remain module-global and are not netns-keyed; IPv4/IPv6 writer parity, rollback/runtime semantics and complete module closure remain open.
- **Actions on this follow-up:** the documentation-only commit will trigger a fresh AI handoff contract run. The next handoff must reconcile that run against the resulting live `main` SHA; the FULLCONENAT code regression above remains tied to the unchanged source commit `91cae3d19bb72a9e0cebcad509db256aded52467`.
- **Next:** after the new documentation contract passes on exact live main, rotate to Wireless: inspect `check_inic_mii_rebooted()` guest-interface-up handling, then `stop_wifi_all_wl/rt()` interface-down and LED publication gates. Keep FULLCONENAT mapping namespace scope as an explicit later audit.

---

## 2026-11-09 11:11 CST (UTC+8) — Scope FULLCONENAT conntrack notifiers to their owning network namespace

- **Base main SHA:** `93277fe13c6c53e49fd94d3badfd4a1a4def0241`.
- **Module / priority:** Linux Netfilter `xt_FULLCONENAT`; P2 conntrack event lifecycle and namespace-cleanup reliability. Firewall/NAT remains **AUDITING**.
- **Recovered task:** resumed the exact handoff's remaining audit of the module-global notifier/reference count against Linux 4.4's per-`struct net` conntrack notifier API. Previous FULLCONENAT GC workqueue/registration-error handling was not repeated.
- **Confirmed defect:** `include/net/netns/conntrack.h` stores `nf_conntrack_event_cb` / `nf_conntrack_chain` inside each namespace. A single global `tg_refer_count` registered the callback only in the first namespace; a last-rule teardown in a second namespace then unregistered against the wrong `par->net`. In the legacy event API this can trigger `BUG_ON(notify != new)`; in chain-event builds it targets the wrong notifier chain. Destroy events for other namespaces were also not delivered to FULLCONENAT's GC.
- **Fix:** `xt_FULLCONENAT.c` now maintains one notifier object and reference count per namespace. Each namespace registers independently; same-net rules share the node; the last destructor unregisters from the exact stored owner. Allocation, registration and overflow failures release the `nf_ct_netns_get()` reference; legacy callback storage is freed only after `synchronize_rcu()`. Added explicit slab/RCU headers and kept declarations GNU89-compatible.
- **Teardown review:** IPv4/IPv6 iptables cleanup passes its owning `struct net` into the target destructor and performs `module_put()` only after the destructor returns. The regression contract was extended to cover this ordering, both notifier API variants, registration/unwind, per-net refcounts and independent namespace lifecycles.
- **Changed paths:** `trunk/linux-4.4.x/net/netfilter/xt_FULLCONENAT.c`; `.github/workflows/fullconenat-lifecycle.yml`; the workflow also reads `include/net/netns/conntrack.h`, IPv4 `ip_tables.c`, IPv6 `ip6_tables.c`, conntrack event provider and existing IPv6 restore rollback sources.
- **Validation:** L1 source review PASS for notifier API ownership, error unwind, target teardown and RCU lifetime. The expanded workflow is a source-contract/model regression only; exact-head Actions are **PENDING**. No local kernel object/module build or standalone host executable was run.
- **Actions:** FULLCONENAT lifecycle regression and AI handoff documentation contract are PENDING until their run IDs, conclusions and `head_sha` are reconciled against the resulting main commit.
- **Remaining risks:** FULLCONENAT mapping hashes and the dying-tuple queue remain module-global and are not keyed by namespace; this round fixes notifier ownership only. IPv4/IPv6 writer parity, complete NAT rollback/runtime semantics, MIPS/uClibc, full firmware/TRX and hardware are not verified.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify both Actions workflows on exact live main. If green, rotate to Wireless residuals at `check_inic_mii_rebooted()` guest-interface-up handling and `stop_wifi_all_wl/rt()` interface-down/LED publication; retain FULLCONENAT mapping namespace scope as an explicit open audit.

---

## 2026-11-09 11:08 CST (UTC+8) — Confirm exact-head route, teardown, HTTPD and handoff CI

- **Base main SHA:** `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`.
- **Route code CI:** WAN route operation status regression [42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) SUCCESS on exact implementation SHA `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; sanitizer helper regression and route source/caller contract passed.
- **WAN teardown CI:** [42552840447](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840447) SUCCESS on that same implementation SHA.
- **Documentation at route implementation SHA:** [42552840438](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840438) SUCCESS.
- **Cross-workflow stale assertion:** HTTPD run 42552840406 first failed because an existing DHCP lease source contract still expected a void route-delete wrapper and an `if (is_add)` branch. Its DHCP lease C regression passed. The contract was updated in `.github/workflows/httpd-security.yml`; corrected HTTPD run [42553153266](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153266) and documentation run [42553153260](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42553153260) both SUCCESS on exact SHA `3e1be8adf8ac531581fa6e505da36c0e5e1e5966`; all HTTPD workflow steps passed.
- **Handoff:** root status records both the route implementation and the cross-workflow contract correction; the prior handoff/dashboard are archived byte-identically. New root log is still the newest 20 records.
- **Validation boundary:** L1 source review PASS; L2 helper sanitizer, route workflow, WAN teardown, corrected HTTPD and pre-final docs workflow PASS. The final documentation-only commit's handoff-contract run must be reconciled against its exact HEAD. Production RC object, MIPS/uClibc, complete firmware/TRX, live route failure injection and hardware remain NOT VERIFIED.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify the final handoff-contract Actions run against exact main, then start the bounded FULLCONENAT namespace/notifier ownership audit. Do not repeat the route status fix.

---

## 2026-11-09 10:47 CST (UTC+8) — Reconcile WAN route Actions and correct dependent HTTPD contract

- **Base main SHA:** `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **Route implementation CI:** [WAN route operation status regression 42552840579](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42552840579) completed SUCCESS on exact `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`; sanitizer regression and source/caller contract both passed.
- **Additional exact-head checks:** WAN interface teardown run 42552840447 and AI handoff documentation run 42552840438 both completed SUCCESS on exact `6f4b55e88a553e8bf6ae6f3b41c59963335eb787`.
- **Recovered cross-workflow failure:** HTTPD run 42552840406 failed in its existing DHCP lease source-contract Python step after the route API changed from `void` to `int` and from a literal `if (is_add)` branch to the equivalent checked ternary. The DHCP lease C policy regression itself passed. This was a stale workflow expectation, not a production compile or runtime finding.
- **Correction:** `.github/workflows/httpd-security.yml` now checks the int-returning DHCP delete wrapper/prototype and ternary add/delete selection. The corrected workflow is included in this follow-up commit; rerun on its exact HEAD is PENDING.
- **Documentation:** handoff/roadmap now record the exact run outcomes and next Firewall/NAT task. Previous root checkpoint/dashboard are preserved byte-identically in indexed archives; root log remains a 20-entry window.
- **Validation boundary:** route host sanitizer regression and exact-head route Actions PASS; exact-head WAN teardown and documentation-contract Actions PASS. Corrected HTTPD Actions PENDING. No production RC object, MIPS/uClibc, firmware/TRX or hardware test.
- **Resulting commit:** THIS COMMIT.
- **Next:** verify corrected HTTPD and AI handoff workflows against exact main HEAD, then begin FULLCONENAT per-net notifier/reference-count lifecycle audit.

---

## 2026-11-09 10:34 CST (UTC+8) — Propagate static and DHCP route operation failures

- **Base main SHA:** \`347ab8b0392a3c74d85a230e199c2faae27f646c\`.
- **Finding:** \`control_static_routes()\` discarded every \`route_add()\` / \`route_del()\` status and returned success; DHCP route setup/removal used a void helper and discarded legacy and RFC3442/Microsoft route failures. DHCP-list allocation failures were also hidden.
- **Fix:** static and DHCP helpers now return the first nonzero operation status while continuing to attempt every remaining valid route. Both DHCP option lists are handled independently, so one allocation failure does not suppress processing of the other. Public DHCP wrappers now return status; every failed route and allocation is logged. Route deletion remains available when dynamic-route addition is disabled, and WAN/MAN cleanup ordering is preserved.
- **Changed paths:** \`trunk/user/rc/src/net.c\`, \`net_wan.c\`, \`rc.h\`; new \`route_status_policy.h\`, \`tests/route_status_policy.c\`, and \`.github/workflows/wan-route-status.yml\`.
- **Validation:** local GNU89 \`-Wall -Wextra -Werror\` regression with ASan/UBSan and leak detection: **PASS**. The regression verifies first-error preservation and that later route operations are still attempted. Workflow source contracts cover static/DHCP wiring, allocation handling, public return types and WAN/MAN delete ordering.
- **Actions:** exact-head WAN route workflow is **PENDING** at commit creation; reconcile its run ID, conclusion and head SHA during the next recovery. The existing documentation-contract run [42550725545](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42550725545) passed on the pre-change main SHA \`347ab8b0392a3c74d85a230e199c2faae27f646c\`.
- **Validation boundary:** L1 selected helper/caller review PASS; L2 focused host policy regression PASS. Production RC translation-unit/object, MIPS/uClibc target build, complete firmware/TRX, live route failure injection and physical-router validation are **NOT VERIFIED**.
- **Resulting commit:** THIS COMMIT.
- **Next:** reconcile exact-head Actions, then rotate away from WAN to Firewall/NAT lifecycle ownership; do not mark WAN/PPP audited or target-verified.

---

## 2026-11-09 10:23 CST — Establish durable cross-conversation handoff and bounded project records

- **Base main SHA:** `71de75dd39b2ebc87fa56b1c74ff798cc0e57fdf`.
- **First priority:** make GitHub sufficient for a fresh GPT session to recover the exact active task, whole-repository progress and verification boundary without relying on chat memory.
- **Governance:** bootstrap and protocol now enforce one recovery sequence, compact current-state root files, size limits, lossless archives and resumption of the open task before selecting new work.
- **History preservation:** the old `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md` and all 663 original `dev_log.md` entries are retained verbatim in indexed archives. This root log contains this entry plus the 19 newest prior entries.
- **Automation:** add `scripts/check_ai_handoff_docs.py` and the `AI handoff documentation contract` workflow. Its exact-HEAD result is PENDING until retrieved.
- **Scope:** documentation/governance only; no firmware source, MIPS build, firmware image or hardware test is changed or claimed.
- **Resulting commit:** THIS COMMIT; fetch live `main` during the next recovery to obtain the exact SHA.

## 2026-11-09 09:58 CST — Include the active USB NDIS interface in WAN teardown verification

- **Base main SHA:** `d324ce8e26f6f730aa36d311c8d51c701879edcc`.
- **Follow-up finding:** a final caller-path review found that `stop_wan_usbnet()` can operate on a distinct NDIS interface stored in `wan0_ifname_t`. The first teardown-policy version checked only the management interface and `IFNAME_WAN`; because the USB helper itself returns void, a distinct `wwan0` / `weth0` down failure could still escape the checked result.
- **Fix:** code commit `8a02953fb60cced248254df86feca85e8bc42d10` passes the runtime `ifname_t` as an optional third interface only when a USB modem is active and `modem_type == 3`. The shared policy now attempts all non-empty distinct management, physical-WAN and USB-NDIS names, deduplicates equal names, and aggregates failures without skipping later cleanup. The adapter's existing `ENODEV` handling still treats a disappeared interface as unavailable, while other errors keep the final state-clear gate closed.
- **Regression expansion:** `tests/wan_teardown_policy.c` now covers optional NDIS shutdown, first/second/third-interface failures, continued attempts after a failure, duplicates across the three names, wired-only operation and invalid required inputs. The workflow source contract verifies the active NDIS selection and that `ifname_t` reaches the common checked teardown.
- **Workflow compatibility:** exact-head WAN regression [run 42548284186](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42548284186) completed **SUCCESS** on `8a02953fb60cced248254df86feca85e8bc42d10`. The broader HTTPD/service lifecycle regression [run 42548284184](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42548284184) also completed **SUCCESS** on the same head. This followed two corrected source-contract mismatches: the initial WAN workflow's global return search was narrowed in `395af4db6d5f10e9356b9709d9072aa4673be0f8`, and the existing HTTPD workflow's expected `stop_wan()` failure-return count was updated from three to four, with an explicit interface-state gate, in `d63ddae44e104d3fd781a2d19b30a86d2bd7d083`. Both workflows subsequently passed on the final code head.
- **Additional diagnostic precision:** `d324ce8e26f6f730aa36d311c8d51c701879edcc` changes the failure log to say that the *final* WAN state clear is skipped; this avoids implying that protocol-specific `wan_down()` bookkeeping performed earlier has been rolled back.
- **Validation:** L1 control-flow review PASS. Local GNU89 host regression compiled with `-Wall -Wextra -Werror -fsanitize=address,undefined` and ran with ASan/UBSan plus leak detection: **PASS**. Exact-head Actions above passed both the focused policy regression and the existing service-stop/WAN contracts.
- **Validation boundary:** no production RC translation-unit/object build, MIPS/uClibc, firmware/TRX, runtime NDIS hot-unplug/failure injection or physical-router testing. The static/DHCP route result-propagation finding remains separate and open; WAN/PPP, Wireless and Firewall/NAT are not closed.

---

## 2026-11-09 09:49 CST — Preserve WAN runtime state when interface shutdown fails

- **Base main SHA:** `54689c55e57094f95fda7566afd2933ecf40fbf9`.
- **Implementation commit:** `26c3c67d27e1b18fba4da14bd607f80fa70afe3b` — add a small host-testable interface teardown policy and make `stop_wan()` check the final management and physical-WAN `ifconfig(..., down, 0.0.0.0)` operations after USB-modem shutdown. Both distinct interface names are attempted even if the first fails; duplicate names are attempted once. `ENODEV` is treated as an interface already removed/unavailable; other failures are logged and cause `stop_wan()` to return `-1` after callback-link cleanup but before `clear_wan_state()` and final WAN LED-down publication.
- **Finding:** `ifconfig()` returns `-1` for socket/name-copy failures and positive errno values for ioctl failures. The previous `stop_wan()` discarded the return from both interface-down calls and still cleared WAN runtime state and returned success. A failed interface-down could therefore be reported as a completed disconnect/reconnect precondition while an interface remained active.
- **SIGUSR2 investigation:** the suspected ignored `killall -SIGUSR2 udhcpc` status was traced rather than patched speculatively. It is a best-effort DHCP RELEASE signal; the subsequent `kill_services(svcs_wan, 3, 1)` uses the process inventory, TERM/KILL escalation and final survivor check, and `stop_wan()` returns before interface teardown if a WAN worker remains. No unsafe teardown ordering was demonstrated for the ignored RELEASE result or fixed 300ms grace delay. Keep its status/retry policy as an explicit review item only if new evidence shows a defect.
- **Route audit evidence:** `route_add()` / `route_del()` return `route_manip()` errors, but `net.c::control_static_routes()` discards per-route results and returns success, while `net_wan.c::control_dhcp_routes()` is void and discards dynamic-route add/delete failures. This is a separate unimplemented follow-up, not included in this bounded interface teardown change.
- **Changed paths:** `trunk/user/rc/src/net_wan.c`; new `trunk/user/rc/src/wan_teardown_policy.h`; new `trunk/user/rc/src/tests/wan_teardown_policy.c`; new `.github/workflows/wan-teardown-stability.yml`.
- **Regression / workflow correction:** the initial workflow commit's global `return -1` ordering assertion matched an earlier PPP/service failure return and failed on run [42547441349](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42547441349). Workflow-only correction commit `395af4db6d5f10e9356b9709d9072aa4673be0f8` scopes the assertion to the interface-failure block; no production-code change was needed for this test correction.
- **Validation:** L1 source review confirmed the `ifconfig()` errno contract, the preceding PPP/WAN service-exit fences, the dual-interface shutdown ordering and the state-clear gate. L2 local host regression compiled with `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` and ran with ASan/UBSan plus leak detection: **PASS**. Exact workflow-head Actions [run 42547527340](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42547527340) completed **SUCCESS** on `395af4db6d5f10e9356b9709d9072aa4673be0f8`; compile/run and Python source-contract steps passed.
- **Validation boundary / remaining risks:** no production RC translation-unit/object build, MIPS/uClibc build, complete firmware/TRX, live DHCP client/interface failure injection, USB modem runtime or physical-router validation. Static and DHCP route add/delete result propagation remains open; Wireless still has the separate iNIC reboot/stop residuals; Firewall/NAT and the wider WAN/PPP audit remain open.

---

## 2026-11-09 09:30 CST — Fail safe when RT3352 iNIC interface startup fails

- **Base main SHA:** `12e4053c07b7f08d3f7b91baa8b5bf37d06df097`.
- **Code commit:** `159c396bbd208d55abee3108b9e1ea4d79ea1394` — `start_inic_mii()` now checks the `rai0` interface-up result through `wif_control_success()`. On failure it removes any stale bridge membership, attempts switch-port isolation and RGMII disable, asserts the iNIC reset GPIO, logs the failure, and returns before iNIC parameter updates, RadioOn commands, or bridge-add publication.
- **Shutdown behavior:** the explicit `inic_disable` branch checks/logs `rai0` interface-down failure but deliberately does not return early. It still attempts bridge removal, iNIC-port isolation, RGMII disable and reset assertion in that order; isolation/disable failures are logged without skipping the reset step.
- **Finding / risk addressed:** the previous RT3352 MII startup discarded `ifconfig rai0 up` status and continued through iNIC configuration, de-isolation/RadioOn selection and `br0` attachment. A failed interface transition could therefore leave switch/bridge state published for an unusable interface. The disable branch also discarded the interface-down result without diagnostics.
- **Changed paths:** `trunk/user/rc/src/net_wifi.c`; `.github/workflows/wireless-radio-state.yml`.
- **Regression/workflow commit:** `4bed634294b80416ca5c17adbcaca026ee0d6d94` adds source-order contracts for the failed-start gate, bridge removal/isolation/RGMII-disable/reset recovery order, suppression of later parameter/RadioOn/bridge-add effects, and the fail-safe disable sequence after a failed interface-down.
- **Validation:** L1 fetched-source review and source-order preflight PASS. Exact-head GitHub Actions [run 42545914009](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42545914009) completed **SUCCESS** on `4bed634294b80416ca5c17adbcaca026ee0d6d94`; job compiled `tests/wireless_radio_policy.c` with `cc -std=gnu89 -Wall -Wextra -Werror`, ran it, and passed the expanded Python source-contract checks. Code-only head [run 42545882789](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42545882789) also completed SUCCESS on `159c396bbd208d55abee3108b9e1ea4d79ea1394`.
- **Validation boundary / remaining risks:** no production RC translation-unit/object build, MIPS/uClibc build, complete firmware/TRX, live RT3352 MII transition, switch failure injection, or physical-router validation. This run did not execute ASan/UBSan. `check_inic_mii_rebooted()` still ignores its guest-interface up result; `stop_wifi_all_wl/rt()` still ignore down results and unconditionally switch the band LED off. These are separate open follow-ups; Wireless remains **AUDITING**. Firewall/NAT remains **AUDITING**, including FULLCONENAT notifier/reference-count versus per-net namespace ownership.
- **Next rotation:** move to the P1 WAN/PPP lifecycle ledger item in `trunk/user/rc/src/net_wan.c::stop_wan()`: trace the ignored `killall -SIGUSR2 udhcpc` result and the subsequent 300ms wait/service teardown, then check interface/static-route command-status propagation. Keep the wireless residuals queued; do not let this module monopolize development.

---

## 2026-11-09 09:23 CST — Gate WDS bridge attachment on successful interface-up commands

- **Base main SHA:** `771430b44511920b63a9091c264ebc44b52db054`.
- **Code commit:** `05c6472da8f4105dbc99c6c082d376ce5bbbff84` — gate all four 5GHz WDS and all four 2.4GHz WDS interface-up paths through `wif_control_success()`; a failed interface-up no longer leads to bridge attachment.
- **Regression/workflow commit:** `4cded21c21a4a4108970fad055f45a7d9d6650f1` — add eight WDS source-order contracts, including the requirement that one failed port must not short-circuit attempts for the other configured WDS ports.
- **Finding:** `start_wifi_wds_wl()` and `start_wifi_wds_rt()` discarded `wif_control()` results and unconditionally attached each requested WDS interface to `br0`. A failed `ifconfig up` could therefore publish stale bridge membership for an interface that did not transition successfully.
- **Implementation details:** 5GHz bridge attachment is now a single statement guarded by the matching successful status check. 2.4GHz gates wrap the bridge-side conditional so that RT3352 MII builds retain their existing no-direct-bridge behavior while still logging interface failures. Each WDS port is checked independently; no early return prevents other enabled WDS ports from being attempted. Stop/mode-disable behavior and WDS port-count selection are unchanged.
- **Changed paths:** `trunk/user/rc/src/net_wifi.c`; `.github/workflows/wireless-radio-state.yml`. Existing `wireless_radio_policy.c` GNU89 host test is reused; no new test source was added.
- **Validation:** L1 fetched-source preflight confirmed all eight calls are guarded and their corresponding bridge calls remain downstream. L2 local host test compiled with `cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined` and ASan/UBSan + leak detection; executable PASS. Exact workflow-head GitHub Actions run [42545325747](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42545325747) completed SUCCESS on `4cded21c21a4a4108970fad055f45a7d9d6650f1`; the compile/policy executable and Python source-contract step both passed. Code-only head `05c6472da8f4105dbc99c6c082d376ce5bbbff84` also triggered run 42545322482; the final exact test/workflow-head result is the run cited above.
- **Validation boundary / remaining risks:** no production RC translation-unit/object build, MIPS/uClibc build, complete firmware/TRX, live WDS transition, RT3352 MII behavior or physical-router validation. Wireless remains **AUDITING**. iNIC main boot/disable ordering, `check_inic_mii_rebooted()`, stop-path status handling, association/driver/switch consumers and external consumers remain open. Firewall/NAT also remains AUDITING, including the separate FULLCONENAT global notifier/reference-count versus per-net namespace question.
- **Next:** re-fetch main and audit `start_inic_mii()` first, specifically failed `rai0` up/down command handling before iNIC update, isolation, RadioOn and bridge operations; then inspect `stop_wifi_all_wl/rt()` and `check_inic_mii_rebooted()`. Close only evidenced defects, then rotate away from Wireless toward DNS/DHCP or another high-risk unaudited user-space module.

---

## 2026-11-09 09:18 CST — Gate AP and APCLI bridge publication on successful interface-up commands

- **Base main SHA:** `398aaf17125ee87321c683c1e3b9d951178e04ed`.
- **Code commit:** `df7c97c7fbba3ed30f003a6f7ecb5127b6fc91be` — make the 2.4/5GHz AP main/guest start paths, the RT3352 iNIC guest path, and both AP-client start paths return immediately when `wif_control()` fails. The checked `wif_control_success()` wrapper now gates bridge attachment, M2U configuration and AP-client auto-connect publication.
- **Regression/workflow commit:** `abe6bfc750dda173e4cc25866b3e6f43ad12b21b` — extend `.github/workflows/wireless-radio-state.yml` to assert all seven AP/APCLI failure gates occur before their dependent bridge/M2U/AutoConnect side effects. Existing push and pull_request path filters remain enabled.
- **Finding:** AP and APCLI startup ignored the return from `ifconfig up`. When it failed, code still attempted bridge attachment and M2U, and APCLI also attempted `ApCliAutoConnect`, leaving software bridge/configuration state inconsistent with the actual interface transition.
- **Validation:** L1 source/call-order preflight PASS for all seven updated gates. L2 host policy regression compiled with GNU89 `-Wall -Wextra -Werror -fsanitize=address,undefined` and leak detection; executable PASS. New exact-head Actions on `abe6bfc750dda173e4cc25866b3e6f43ad12b21b` is **NOT VERIFIED**: combined-status returned no rows and the available workflow-run query exposes pull_request-triggered runs only, not push-triggered runs. Do not treat Actions as passed until inspected on GitHub.
- **Validation boundary / remaining risks:** no production RC translation-unit/object build, MIPS/uClibc build, complete firmware/TRX, live AP/APCLI/WDS transition or hardware validation. Wireless remains **AUDITING**. WDS start callers, iNIC main boot/disable sequencing, stop callers and residual driver/association/switch consumers remain open.

---

## 2026-11-09 09:13 CST — Keep guest AP scheduler and bridge state behind successful ifconfig

- **Base main SHA:** `74ee602e37e5596927023360a375cd882cf7ad11`.
- **Implementation/test/workflow commit:** `c22eae28906a7a5ab7a76d5c4a3c4431b97aecfa` — add a checked `wif_control_success()` wrapper and stop guest AP control from publishing ON/OFF, bridge membership, M2U or isolation changes after a failed interface up/down command.
- **Regression correction:** `c10eb07e13d3d48a68943b272253cc84d0d419b5` — correct the test `main()` call after renaming the interface-failure regression function.
- **Changed paths:** `trunk/user/rc/src/net_wifi.c`, `trunk/user/rc/src/tests/wireless_radio_policy.c`, `.github/workflows/wireless-radio-state.yml`.
- **Finding:** `control_guest_wl/rt()` ignored the ifconfig status returned by `wif_control()`; failed up/down commands were followed by bridge/M2U updates and a false `is_ap_changed` transition, including isolation restart and scheduler logging.
- **Validation:** GNU89 `-Wall -Wextra -Werror` policy regression under ASan/UBSan + leak detection PASS. Exact-head wireless Actions [42544153924](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42544153924) SUCCESS on `c10eb07e13d3d48a68943b272253cc84d0d419b5`; HTTPD/authentication Actions [42544128087](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42544128087) SUCCESS on `c22eae28906a7a5ab7a76d5c4a3c4431b97aecfa`. Initial wireless attempt [42544128073](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42544128073) FAILED because the renamed test's `main()` still referenced the old function; this test-only defect was corrected by `c10eb07`, then exact-head CI passed.
- **Remaining risks:** no full RC production object, MIPS/uClibc, firmware/TRX, live radio or hardware validation. Remaining `wif_control()` startup/stop consumers in AP/WDS/APCLI/iNIC lifecycle need review; Wireless remains AUDITING. Firewall/NAT and FULLCONENAT multi-netns notifier ownership remain open.
- **Next:** inspect `start_wifi_ap_wl/rt()` and `start_wifi_apcli_wl/rt()` for bridge/state publication after failed ifconfig; review WDS/iNIC/stop callers afterward, then rotate to another user-space module.

---

## 2026-11-09 09:04 CST — Gate wireless radio-state publication on successful driver/restart operations

- **Module / priority:** RC Wireless radio control; P2 state-consistency and restart-reliability. Keep Wireless **AUDITING**; this is one bounded control-path fix, not subsystem closure.
- **Recovered task:** resumed the latest handoff's requested Wireless/driver state-publication audit after reconciling the FULLCONENAT exact-head result. Re-read net_wifi.c, ralink.c, the rc.h declarations and rc.c scheduler/toggle consumers. The previous ralink.c::wl_ioctl() socket/ioctl/close fix and scan-parser fixes were not repeated.
- **Finding:** mlme_radio_wl() / mlme_radio_rt() discarded the iwpriv RadioOn command result and still wrote mlme_radio_* NVRAM state plus LED state. On MLME-controlled boards, is_radio_on_*() reads this persisted state together with interface state; a failed driver operation could therefore be reported as a completed radio transition and suppress a later scheduler retry. Separately, control_radio_wl/rt() unconditionally set is_radio_changed = 1 after invoking a void MLME function, and ignored the explicit restart_wifi_wl/rt() return value even though restart preflight can return -1 before stopping any services.
- **Fix:** 0c57013b56d709ab2be87b43e1c48e7259078300 adds status-returning private MLME apply helpers. They preserve the existing public void API, but only publish NVRAM/LED state after a successful RadioOn command. control_radio_wl/rt() now derives its changed result from the MLME helper or the restart function's zero-success status; failed operations remain retryable and are not logged as successful scheduler transitions. The existing replacement-config preflight remains before radio-service shutdown.
- **Changed paths:** trunk/user/rc/src/net_wifi.c; new trunk/user/rc/src/wireless_radio_policy.h and trunk/user/rc/src/tests/wireless_radio_policy.c; new .github/workflows/wireless-radio-state.yml.
- **Test/workflow commits:** helper 9d55d855f4dd32dc07ff98c9146380ad7033955d; GNU89 policy regression 645a4eabb2eec476fe708bde0d5326a31dcb1c48; workflow ead6032950b357806242d7a5380f5372ef838b0b; workflow assertion corrections c260aecce2907fb92c576546030a27f4dec27b30 (path-count assumption) and 27d2e373224d2dc6bdb37e8937f53cd4a4e0a946 (independent symmetric push/PR filter checks). The first local mirror of the workflow contract exposed the two incorrect path-count assumptions; both were corrected before recording the final preflight.
- **Validation:** local cc -std=gnu89 -Wall -Wextra -Werror compile and execution of wireless_radio_policy.c PASS. Fetched-source structural checks for both MLME command-before-state gates, both control-radio success assignments, restart preflight-before-stop ordering, and symmetric push/PR path filters PASS. These are L1/L2 helper checks only; the Python workflow contract has not yet been observed executing on Actions.
- **Exact code/test/workflow HEAD:** 27d2e373224d2dc6bdb37e8937f53cd4a4e0a946. The GitHub connector returned no commit status rows and its workflow-run listing is limited to PR-triggered runs, so the push-triggered wireless-radio-state Actions result/run ID is **NOT VERIFIED**; inspect it before starting unrelated work.
- **Validation boundary / residuals:** no full production net_wifi.c RC object, MIPS/uClibc build, firmware/TRX, live AP/STA/WDS transition or physical-router test was run. The auxiliary RT3352 TxPower command remains best-effort as before. wif_control() callers still need systematic review for ignored ifconfig failures; switch link/speed and other Wireless/driver consumers remain open. Firewall/NAT remains **AUDITING**, including the separate FULLCONENAT global notifier/reference-count versus per-net namespace ownership question.
- **Next:** first resolve exact-head Actions for 27d2e373224d2dc6bdb37e8937f53cd4a4e0a946 using the wireless-radio-state.yml workflow. If green, continue the Wireless lifecycle rotation at wif_control() and its interface start/stop callers, proving whether ignored interface-up/down failures can be published as active radio/bridge state; otherwise fix the workflow/source failure first.

---

## 2026-11-09 08:56 CST — Require working FULLCONENAT garbage collection before accepting rules

- **Module / priority:** Linux Netfilter xt_FULLCONENAT; P2 connection-lifecycle and kernel-memory stability. Firewall/NAT and Netfilter extension coverage remain **AUDITING**.
- **Recovered IPv6 audit result:** traced ip6_tables.c table-provider autoload and error return, ip6table_nat.c per-net registration/four-hook setup and unwind, xt_check_target() negative checkentry propagation, and the ULA FULLCONENAT rule through firewall_ex.c. The IPv6 NAT writer is void and does not return its local apply result, but the shared restore transaction records command failure; start_firewall_ex() checks transaction status immediately after ip6t_nat_rules() and aborts/rolls back. No defect was proven in that IPv6 table/restore propagation path, so it was left unchanged.
- **Confirmed defect:** fullconenat_tg_init() logged create_singlethread_workqueue() failure but still registered IPv4/IPv6 targets. ct_event_cb() allocates and links a tuple_list item for each destroyed UDP connection, but schedules reclamation only when wq != NULL; without the worker those queued allocations had no normal-path consumer and could accumulate until module unload. Separately, fullconenat_tg_check() ignored nf_ct_netns_get() and conntrack notifier registration errors (including the legacy notifier's -EBUSY), accepted a target even when destroy events could not be observed, and left mapping cleanup unavailable. Module init also leaked its newly created workqueue if xt_register_targets() failed.
- **Implementation:** 11cc2d984254c0dd8534ef90a3bd8fd2bed1ea2a refuses to register targets unless the GC workqueue exists; if target registration fails, it destroys the workqueue, clears wq, and returns the original error. fullconenat_tg_check() now propagates netns acquisition and notifier-registration errors, releases the netns reference on notifier failure, and increments tg_refer_count only after successful notifier registration.
- **Changed paths:** trunk/linux-4.4.x/net/netfilter/xt_FULLCONENAT.c; new .github/workflows/fullconenat-lifecycle.yml.
- **Regression workflow corrections:** 27e25ffc7c243b5e9712da24339c389352f761b5 corrected initial regex escaping and source locators; 6d09cd03d4f1b8ff147748890bbee35583ad22f0, a1e4fc862b503fa343068e355839b447b75f2ab0, and b63d863b8afeac8c9fa8012403becb9b312c8607 corrected source-contract locators for the worker definition, legacy notifier implementation and multiline xtables registration definition. The first four Actions attempts failed only in this newly added static test's assertion/locator code, not in a kernel build. The corrected exact-head workflow passed.
- **Exact-head Actions:** https://github.com/Arlene206/padavan-4.4-2/actions/runs/42542694771 completed **SUCCESS** on exact code/test/workflow SHA b63d863b8afeac8c9fa8012403becb9b312c8607; checkout and the FULLCONENAT/IPv6 restore source-contract step passed.
- **Validation boundary:** L1 source and error-path review PASS; L2 exact-head GitHub Actions source-contract PASS. No Linux 4.4 kernel object/module build, MIPS/uClibc build, full firmware/TRX, live conntrack/NAT traffic, namespace concurrency or hardware test was run. A separate residual remains to audit: the module-global tg_refer_count/notifier state versus the per-network-namespace notifier API; this bounded fix does not claim multi-netns correctness.
- **Next rotation:** leave Firewall/NAT AUDITING, not closed. Rotate to remaining Wireless/driver call-site coverage across net_wifi.c, ralink.c and switch link/speed consumers; do not repeat the fixed ralink.c::wl_ioctl() wrapper or the completed iwlib.c/iwlist.c scan-parser bounds. Trace returned lengths, ioctl error propagation and state-publication ordering, prove one new defect before patching, and keep expanding the wider userland inventory.

---

## 2026-11-09 08:24 CST — Stage one complete mount inventory across safe storage removal

- **Base main SHA:** `7d10984e2bd89d0adaa63c59b345c8977977bee7`.
- **Implementation commit:** `d7fb1e12867b3678878daba3fb36e94bcd22cbaf` — stage a complete `/proc/mounts` inventory once for the full selected storage operation; validate all six fields, read/close status and allocations before side effects; preserve the public `umount_stor_path()` signature.
- **Workflow contract correction:** `97e91979aa731e26962c1f3a1a8b949ab6dfdb43` — migrate the older safe-eject source contract away from the removed per-device `umount_dev()` loop to the range-wide inventory and swapoff preflight.
- **Regression correction:** `968437c118f373bfc4ef1313ffeb259ace788ff1` — correct a negative MMC match assertion that mistakenly expected a device path not to match itself.
- **Finding:** `umount_stor_path()` called `umount_dev()` once per partition, and `safe_remove_stor_device()` repeated the path helper once per port. Each inner helper staged the complete mount inventory for only that invocation. A later inventory read/parse/close/allocation error could therefore be discovered after an earlier partition/port had already been unmounted.
- **Behavioral change:** one operation-wide mount snapshot is parsed and staged first. Any incomplete inventory aborts before swapoff, optional unmount hooks, `umount()` or spindown. The code then attempts all selected swapoffs before any selected unmount; any swapoff failure blocks all selected unmounts and spindown. Staged device entries are matched against exact SD/MMC whole-disk/partition names, preventing a textual prefix such as `sda-extra` from matching `sda`. Multi-port safe removal invokes one range operation rather than a per-port loop.
- **Changed paths:** `trunk/user/rc/src/services_stor.c`; `trunk/user/rc/src/storage_ejected_inventory_policy.h`; `trunk/user/rc/src/tests/storage_ejected_inventory_policy.c`; `.github/workflows/httpd-security.yml`. Existing push/pull_request path filters already covered the modified source/helper/test paths.
- **Regression history:** initial exact-head run [42539757133](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42539757133) failed because a pre-existing workflow assertion still required `umount_dev()`. After the workflow contract update, [42539851945](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42539851945) passed the migrated safe-eject source contract but the new C test exposed the invalid same-disk negative assertion. After correcting the fixture, exact-head run [42539924139](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42539924139) completed **SUCCESS** on `968437c118f373bfc4ef1313ffeb259ace788ff1`, 41/41 steps, including the GNU89 `-Wall -Wextra -Werror` compile/run and new operation-wide inventory/swapoff-order contracts.
- **Validation boundary:** L1 source/control-flow review PASS; L2 focused GNU89 host policy regression and exact-head Actions PASS. No full production `services_stor.c` object, MIPS/uClibc, complete firmware/TRX, concurrent mount-churn/runtime storage or physical router validation was run.
- **Next:** rotate to the open IPv6 Firewall/NAT audit. Trace `firewall_ex.c` IPv6 restore transactions, `ip6table_nat` registration and extension-hook failures against the IPv4 path; implement only an evidenced return/rollback defect. Keep Firewall/NAT and USB/storage **AUDITING**.

---

