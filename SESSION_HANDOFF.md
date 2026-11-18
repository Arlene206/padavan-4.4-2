# 2026-11-19 continuation checkpoint — MentoHUST credential confidentiality

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development remains on `main`. Parent main SHA: `3561edf47144278a2fa386358930cda901206ccf`.
- The previous Redsocks source/checker slice is closed: exact-main Redsocks lifecycle run `44407053205` and AI handoff run `44407053381` both completed SUCCESS on `bb6b243951b1b72adaf349546c8caebf8cb492ed`.
- XY-C1 source SHA `5debbf416f70cb136a7ad2859b002033143b4357` run `44406037894` has now completed SUCCESS; this validates that exact earlier product source, not the current MentoHUST changes.
- The frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify it. PR #1 remains closed/unmerged and is not a development path.
- Physical-router L4 remains NOT VERIFIED. Distinguish host/Actions checks from target build/artifact and hardware evidence.
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

## Completed prerequisite — enforce explicitly configured SYN cookies at startup and runtime

`start_wan()` and runtime `restart_all_sysctl()` now snapshot the configured `fw_syn_cook` value once and pass that exact value into `set_tcp_syncookies()`. If the option is enabled but the precise sysctl write fails, initial WAN startup is rejected before setup side effects; a runtime `RCN_RESTART_SYSCTL` failure attempts the normal lifecycle-locked `stop_wan()` path and deliberately does not restart PPPoE passthrough. Independent ICMP writes are still attempted and their errors remain aggregated. If the setting is disabled, a failed write remains diagnostic/aggregate-only. The policy regression covers both enabled and disabled cases, and the XY-C1 kernel configuration has `CONFIG_SYN_COOKIES=y`.

### Exact-head validation following the disabled-IPv6 gate

- Process lifecycle `44361396656` and AI handoff docs `44361396778` both PASS on checker/documentation SHA `f0392353ce2d71a31e4efaaea4ebf1abf7903878`.
- XY-C1 run `44360411938` on product-source SHA `66bdedc162493cda29d36864d17afaa826e0516b` is SUCCESS, including Build XY-C1, TRX verification and artifact upload. L3 is VERIFIED for that exact product-source SHA; physical-router L4 remains NOT VERIFIED.
- Firewall Startup `44360411721` and HTTPD Security `44360411452` also PASS on `66bdedc`; no known failure remains on the disabled-IPv6 source/checker slice.
- The current SYN-cookie source change is a new firmware-source SHA; its WAN route, XY-C1 and all other path-triggered exact-head checks must be inspected before claiming verification.
## Completed source slice — reject non-global IPv6 special-purpose prefixes in stock AliDDNS WAN discovery

The stock `arIpAddress6()` accepted any syntactically valid-looking address in `2000::/3` after checking Linux `Scope:Global`. That is insufficient: this broad global-unicast allocation contains special-purpose prefixes that IANA does not classify as globally reachable, including `2001:db8::/32` and the newer `3fff::/20` documentation range, benchmarking `2001:2::/48`, and deprecated ORCHIDv1 `2001:10::/28`. The kernel scope label does not replace address-registry classification.

This slice adds an eight-hextet numeric classifier to stock WAN discovery, rejecting those prefixes while retaining ordinary global unicast. A follow-up review against the complete IANA allocation tables found that the 2001::/23 parent assignment contains a small set of more-specific globally reachable exceptions and that the 3f00::/8 allocation range is reserved. It also found the parser had left compressed zero hextets unset, which could miss compressed-prefix checks. This follow-up initializes omitted groups, rejects the unassigned 2001::/23 space, 2002::/16 non-universal 6to4, and all 3f00::/8, while preserving the explicit globally reachable IANA suballocations. Operator-supplied custom DDNS script semantics remain unchanged. Expanded tests exercise both rejected prefixes and the IANA allowlist; exact-head Actions for the resulting SHA remain pending.

### Exact-head result for the immediately preceding WAN security source

- Startup-gate SHA `865215612acd2fd5dce31d147eb6a60b809ab210`: 22/22 exact-head workflows SUCCESS.
- Runtime-gate SHA `adedc6dfef3cbdfbdee8ddcb633ab57c92bc9759`: 22/22 exact-head workflows SUCCESS; XY-C1 run `44368075437` completed Build XY-C1, TRX verification and artifact upload.
- The active AliDDNS IPv6 classification change is a new firmware/source SHA; no validation result from either previous WAN SHA transfers to it.

## Completed source slice — reject duplicate Clause 45 MDIO CLI options before ioctl

Reviewed the production MT7621 Clause 45 utility `trunk/user/utils/src/switch/mii_mgr_cl45.c` and its host regression. The parser validated ranges and required all fields, but accepted duplicate `-p`, `-d`, `-r`, and `-v` flags with silent last-value-wins semantics; it also accepted repeated `-g`/ `-s` mode flags. This is inconsistent with the Clause 22 utility, which rejects duplicate required fields, and makes an invocation ambiguous when it changes PHY/DEVAD/register/value selection. The parser runs before socket creation and hardware ioctl, so duplicate inputs should fail before either operation.

The parser now rejects every duplicate mode or field flag and the host regression covers duplicate GET/SET modes, port, device address, register address and write value. Exact-head Switch CLI (`44375203978`), Process inventory (`44375203938`), AI docs (`44375203886`) and NVRAM audit (`44375203974`) all PASS on `f57161cc56f82f1b091093a8e93eac012413ce15`. No XY-C1 run was triggered by this utility-only change; live PHY/hardware L4 remains unverified.

### IANA IPv6 filter validation boundary

- AliDDNS, Process inventory, AI docs and HTTPD security runs on `f3efd5e4a7101b930e699c54704fcf25a5ae341d` passed: AliDDNS `44373890929`, Process `44373890954`, AI docs `44373891052`, HTTPD security `44373891002`.
- XY-C1 run `44373890927` remains IN_PROGRESS at last check: `Build XY-C1` active; TRX/artifact steps pending. Its result is independent of the Clause 45 host parser change and remains an exact-source build boundary.

## Active task

Close MentoHUST credential confidentiality and configuration-publication hardening.

- `trunk/user/rc/src/init.c` deliberately sets PID 1 umask to `0000`; the prior `trunk/user/mentohust/scripts/mentohust.sh` wrote plaintext username/password with ordinary redirections and left config files at inherited permissions. Its sed-based value interpolation also allowed line-breaks to inject additional config directives.
- THIS COMMIT sets a restrictive script umask, generates a same-directory mode-0600 temporary config, writes fixed key/value names with literal `printf`, rejects CR/LF-bearing NVRAM values and symlink/directory destinations, verifies ownership against the executing UID, atomically renames the generated file, and restricts existing regular configs before launching the daemon.
- Regression coverage is extended in `scripts/tests/legacy_wrapper_pidfile_safety.sh`; `scripts/check_legacy_wrapper_pidfile_safety.py` now enforces the production security contract and correctly detects `kill -9` tokens. The existing `.github/workflows/legacy-wrapper-pidfile-safety.yml` already watches all changed paths, and the XY-C1 production-input workflow already watches the MentoHUST script.
- Preserve the current script's lifecycle semantics: duplicate-start preflight, bounded RUNNING verification, identity-safe stop helper, and restart stop-before-start behavior.

## Verification boundary

- L1 source review: PID 1 umask, plaintext NVRAM credential flow, old file creation modes, existing-config reuse, shell interpolation, and firmware script packaging were inspected. The code, regression and handoff updates are part of THIS COMMIT.
- L2: exact-head legacy wrapper source-contract/runtime Actions and AI handoff documentation Actions must be inspected after push. No local checkout is available in this connector-only environment; do not claim local shell execution.
- L3: current MentoHUST production script change requires exact-head XY-C1 Build, TRX verification and artifact upload. Earlier source SHA `5debbf4` run `44406037894` is successful but does not cover this change.
- L4 physical router: NOT VERIFIED.

## Exact first action for the next GPT

1. Check Actions for THIS COMMIT's exact SHA: the `Legacy wrapper stale pidfile safety` workflow must pass shell syntax, runtime regressions and source contracts; the AI handoff docs check must pass; and XY-C1 must complete Build, TRX verification and artifact upload on this same production-source SHA.
2. Triage any exact-head failure from its first failing log and repair the actual defect without weakening coverage. If XY-C1 or another independent check is pending, continue a separate safe P1 task rather than waiting.
3. Then resume the P1 whole-userland service lifecycle/start-result queue in `REFACTOR_ROADMAP.md`; rotate beyond already-verified Redsocks, Shadowsocks, WireGuard and WAN callback work. Keep the archival backup immutable.
## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. Compare the source baseline, active task, relevant exact-SHA workflow states, validation boundary, blockers and exact first action with this handoff. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Build the complete Git tree from the exact parent and update `main` only with a non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Do not ask the user what to do next. Keep the archival backup immutable.

## Before context exhaustion

Finish the current safe slice and commit it atomically. Record the exact commit, tests, exact-SHA Actions and unverified boundaries. Leave the next GPT one concrete first action; do not claim local tests, builds, firmware artifacts or hardware checks that were not performed.
