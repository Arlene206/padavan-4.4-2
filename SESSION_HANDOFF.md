# 2026-11-19 continuation checkpoint — require configured SYN-cookie protection before WAN exposure

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main` only.
- Main before this source slice: `f0392353ce2d71a31e4efaaea4ebf1abf7903878`. Product-source baseline: `66bdedc162493cda29d36864d17afaa826e0516b`; fetch live main at every continuation rather than assuming a remembered SHA.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not switch to, update or delete it.
- PR #1 remains closed/unmerged; do not revive it.
- No local checkout, MIPS cross-build or physical-router validation was available in this session.

## Completed prerequisite — switch VLAN WAN gate

The previous source slice closed a P1 WAN-startup safety gap. Router-mode `init_bridge()` formerly recorded `switch_config_vlan(1)` failure in its aggregate bridge status, but WAN admission did not consume that result. `init_bridge(int is_ap_mode, int *switch_vlan_status)` now exports only the switch VLAN isolation result; `init_router()` passes it into `firewall_startup_policy_allows_wan()`. Any nonzero result blocks WAN even when firewall and NAT are disabled. Unrelated radio/guest bridge errors are not promoted to the WAN prerequisite. The existing IPv4/IPv6 forwarding-disable cleanup remains in place.

Atomic source commit: `1fa05ac1901a1f34fd3ddc6d0a6f69defd4d968f` (parent `26f0638d76b94a076278f14cf976a6a015d310dc`). The C policy regression compiled and passed on its exact-head firewall run; subsequent source-integration contracts exposed several stale checkers, not a failed production C regression.

### Exact-SHA CI failures identified for repair

- Firewall startup run `44351420600`: the GNU89 ASan/UBSan C regression passed. The later inline source checker failed because it still expected the old six-argument policy call and used a brace scanner that ignored comments/quotes. Replace its nested startup extraction with the existing lexical C function-body helper and verify the seventh switch-VLAN argument.
- HTTPD authentication/security run `44351420392`: all preceding security checks passed; the inline WAN-policy contract failed on the same outdated six-argument expectations. Update its policy test call expectations and assert the switch-VLAN fail-closed gate.
- Process lifecycle regression `44351420304` and wireless-radio run `44351420162`: both fail only because `scripts/check_guest_lan_isolation_status.py` still searches for the old `init_bridge(int is_ap_mode)` signature. Update the checker to the new signature and assert exact switch-VLAN status capture. The script is already present in both workflows' push/PR path allowlists.
- AI handoff run `44351420055`: failed because the prior handoff rewrite dropped the mandatory checkpoint section. Restore that section and retain all required handoff headings; the checkpoint is mandatory, but a document edit is conditional.
- Process lifecycle inventory run `44351420273`: SUCCESS.
- XY-C1 firmware run `44351420211`: at last poll, host regressions, source contracts and the production-input detector passed; `Build XY-C1` remained IN_PROGRESS; TRX verification and artifact upload were PENDING. Recheck this exact run before claiming L3.
- The other independent exact-head runs observed for `1fa05ac` completed successfully. Do not collapse the five failures above into a blanket CI failure or misstate the pending firmware job.

### Process workflow follow-up at main 68154ac14fad3330f99b2310f796cbcf66960e2e

- Wireless radio run `44354158097` and AI handoff run `44354157678` are SUCCESS. Process lifecycle run `44354157953` still FAILS in its separate embedded LAN startup checker: that workflow itself searches for the old `init_bridge(int is_ap_mode)` signature/prototype/call after the shared guest-isolation checker was repaired. This repair updates the Process workflow's local signature, prototype and `init_router()` caller assertions to the `switch_vlan_status` output parameter, and verifies the VLAN result is captured between `switch_config_vlan(1)` and its bridge-status record. The workflow already watches its own file on both push and pull_request.

## Completed prerequisite — fail closed when IPv6 disable cannot be verified

IPv6 configured as disabled is a security boundary: if global `disable_ipv6`, RA-suppression or forwarding sysctl writes fail, WAN must not start merely because IPv6 firewall filtering was considered unnecessary. `init_router()` now retains the exact `init_ipv6()` status and passes a dedicated disabled-IPv6 prerequisite into the WAN policy. The new gate applies only to configured-disabled IPv6; enabled IPv6 continues to rely on the existing active firewall-filter requirement. On any rejected WAN startup, PID 1 retries the global IPv6 forwarding-disable write and records a failure rather than conditioning that safety cleanup on the configured IPv6 mode.

### Exact-head IPv6 WAN-gate validation at 66bdedc162493cda29d36864d17afaa826e0516b

- Firewall Startup `44360411721`: SUCCESS; HTTPD Security `44360411452`: SUCCESS; AI handoff docs `44360411581`: SUCCESS.
- Process lifecycle `44360411889`: FAIL in `scripts/check_process_lifecycle_integration.py`, not in a C regression. All earlier checks in the job passed; the checker contains two identical assertions expecting the old block-local `int ipv6_init_status = init_ipv6();`. PID 1 now declares the status at `init_router()` scope and assigns it after calculating whether configured-disabled IPv6 must be gated. Both duplicated contracts are updated to validate the retained variables and assignment.
- XY-C1 exact-source run `44360411938` was IN_PROGRESS at last poll. Do not claim L3 until Build XY-C1, TRX verification and artifact upload complete successfully.

## PID 1 early-sysctl review

Reviewed `trunk/user/rc/src/init.c::init_sysctl()` against its callers and later network setup. Kernel panic policy, socket buffer sizes, VM tuning and TCP timeout/retry values are best-effort tuning; they must not become generic PID-1 boot blockers. The early IPv6-disable operation is checked again by `init_router()::init_ipv6()`, whose status is retained and gates WAN when IPv6 is configured disabled. The early global `rp_filter` write is a baseline; the firewall separately verifies per-WAN-interface rp_filter changes when multicast policy requires them. No additional early-init change is justified without evidence that a specific global write is a mandatory boot prerequisite.

## Active task — enforce explicitly configured SYN cookies before WAN exposure

`start_wan()` already captured and returned the aggregate `set_tcp_syncookies()` result, but it still launched WAN protocols after the actual `tcp_syncookies` write failed—even when NVRAM explicitly requested SYN cookies. The two ICMP hardening writes share that aggregate but are independent of the explicit SYN-cookie prerequisite. This slice exposes the precise status of the first write, adds positive/negative error policy coverage, and evaluates the gate before callback links, VLAN migration or WAN client launch. When the feature is disabled, its write failure remains diagnostic/aggregate-only; when enabled, WAN startup fails closed. The XY-C1 kernel configuration has `CONFIG_SYN_COOKIES=y`.

### Exact-head validation following the disabled-IPv6 gate

- Process lifecycle `44361396656` and AI handoff docs `44361396778` both PASS on checker/documentation SHA `f0392353ce2d71a31e4efaaea4ebf1abf7903878`.
- XY-C1 run `44360411938` on product-source SHA `66bdedc162493cda29d36864d17afaa826e0516b` is SUCCESS, including Build XY-C1, TRX verification and artifact upload. L3 is VERIFIED for that exact product-source SHA; physical-router L4 remains NOT VERIFIED.
- Firewall Startup `44360411721` and HTTPD Security `44360411452` also PASS on `66bdedc`; no known failure remains on the disabled-IPv6 source/checker slice.
- The current SYN-cookie source change is a new firmware-source SHA; its WAN route, XY-C1 and all other path-triggered exact-head checks must be inspected before claiming verification.
## Verification boundary

- WAN VLAN source SHA `1fa05ac1901a1f34fd3ddc6d0a6f69defd4d968f`: Firewall policy C regression passed; exact-source XY-C1 run `44351420211` is now SUCCESS, including every host regression, Build XY-C1, TRX verification and artifact upload. L3 is VERIFIED for this exact firmware source SHA; physical-router L4 remains NOT VERIFIED.
- Checker/handoff commit `117c52c925cb96478802d8c4bd2e829e4bd9c133`: Firewall Startup run `44353199552`, HTTPD Security run `44353199583`, and AI handoff docs run `44353199773` all SUCCESS.
- Process lifecycle run `44353199473` and Wireless radio run `44353199497` both FAIL only at the shared guest-LAN source checker. The current C policy binaries passed; the stale checker compares a multi-line C `if` assignment against a single-line literal. The checker repair in this continuation normalizes function-body whitespace and additionally asserts the VLAN capture occurs after `switch_config_vlan(1)` and before the bridge-status record.
- The prior exact-head failures on source SHA `1fa05ac` are historical; the three successful follow-up workflows above resolved firewall, HTTPD and AI handoff contracts. Do not attribute their original failures to the new main SHA.
- An isolated GNU89 policy helper test was compiled and run locally with ASan/UBSan; it is not a full repository integration build. No local MIPS firmware build or physical-router L4 result is claimed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect every exact-head Action for this SYN-cookie gate commit, especially WAN route status, Process lifecycle/inventory, AI handoff docs and XY-C1. The preceding disabled-IPv6 source SHA `66bdedc162493cda29d36864d17afaa826e0516b` is L3 verified; do not transfer that build result to the new source SHA.
2. If this exact SHA passes, inspect runtime `RCN_RESTART_SYSCTL` behavior when `fw_syn_cook` is changed while WAN is already active. Confirm whether a failed required write keeps the old setting, reports a failed action, or leaves a live interface outside the requested security policy; do not assume the boot-only gate solves runtime reconfiguration.
3. Continue cross-userland reliability/security fixes without weakening the switch-VLAN or disabled-IPv6 WAN gates. Physical-router L4 remains a separate, unverified boundary.
## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. Compare the source baseline, active task, relevant exact-SHA workflow states, validation boundary, blockers and exact first action with this handoff. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Build the complete Git tree from the exact parent and update `main` only with a non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Do not ask the user what to do next. Keep the archival backup immutable.

## Before context exhaustion

Finish the current safe slice and commit it atomically. Record the exact commit, tests, exact-SHA Actions and unverified boundaries. Leave the next GPT one concrete first action; do not claim local tests, builds, firmware artifacts or hardware checks that were not performed.
