# 2026-11-19 continuation checkpoint — repair exact-SHA startup contracts and retain fail-closed WAN state

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main` only.
- WAN VLAN source commit: `1fa05ac1901a1f34fd3ddc6d0a6f69defd4d968f`. First checker follow-up is current main `117c52c925cb96478802d8c4bd2e829e4bd9c133`; fetch live main at the next recovery rather than assuming a remembered SHA.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not switch to, update or delete it.
- PR #1 remains closed/unmerged; do not revive it.
- No local checkout, MIPS cross-build or physical-router validation was available in this session.

## Active task

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

## Verification boundary

- WAN VLAN source SHA `1fa05ac1901a1f34fd3ddc6d0a6f69defd4d968f`: Firewall policy C regression passed; exact-source XY-C1 run `44351420211` is now SUCCESS, including every host regression, Build XY-C1, TRX verification and artifact upload. L3 is VERIFIED for this exact firmware source SHA; physical-router L4 remains NOT VERIFIED.
- Checker/handoff commit `117c52c925cb96478802d8c4bd2e829e4bd9c133`: Firewall Startup run `44353199552`, HTTPD Security run `44353199583`, and AI handoff docs run `44353199773` all SUCCESS.
- Process lifecycle run `44353199473` and Wireless radio run `44353199497` both FAIL only at the shared guest-LAN source checker. The current C policy binaries passed; the stale checker compares a multi-line C `if` assignment against a single-line literal. The checker repair in this continuation normalizes function-body whitespace and additionally asserts the VLAN capture occurs after `switch_config_vlan(1)` and before the bridge-status record.
- The prior exact-head failures on source SHA `1fa05ac` are historical; the three successful follow-up workflows above resolved firewall, HTTPD and AI handoff contracts. Do not attribute their original failures to the new main SHA.
- No local test, local firmware build, firmware artifact or hardware result is claimed beyond the recorded GitHub Actions evidence.

## Exact first action for the next GPT

1. Inspect exact-head `push` Actions for the new guest-checker repair commit: Process lifecycle and Wireless radio must pass; recheck Firewall Startup, HTTPD Security and AI handoff documentation rather than assuming results carry forward. On checker commit `117c52c925cb96478802d8c4bd2e829e4bd9c133`, Firewall `44353199552`, HTTPD `44353199583` and AI docs `44353199773` are SUCCESS; Process `44353199473` and Wireless `44353199497` are FAIL at the whitespace-sensitive checker assertion now repaired. XY-C1 source run `44351420211` on product SHA `1fa05ac1901a1f34fd3ddc6d0a6f69defd4d968f` is SUCCESS through Build, TRX and artifact upload (L3 for that SHA).
2. After the new Process/Wireless exact-head checks pass, finish the next P1 IPv6 safety slice in `trunk/user/rc/src/rc.c`: retain the result of `init_ipv6()` beyond its current local block and block WAN only when IPv6 is configured disabled but the global disable sysctls did not complete. This prevents an unverified IPv6 disable from leaving an unfiltered path open. Extend `firewall_startup_policy.h` and its C regression for positive/negative failures; preserve current behavior for enabled IPv6, where active IPv6 firewall filter installation remains the separate gate. Re-attempt global IPv6 forwarding disable on the blocked path.
3. Do not turn early `init_sysctl()` memory/TCP performance tuning failures into PID-1 boot blockers. Those writes are best-effort; classify actual required network/firewall state separately. Continue cross-userland stability/security afterward.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and exact-head Actions before ending. Compare the source baseline, active task, relevant exact-SHA workflow states, validation boundary, blockers and exact first action with this handoff. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Cross-session execution mandate

One logical slice, one atomic main commit. Build the complete Git tree from the exact parent and update `main` only with a non-forced expected-SHA compare-and-swap. Continue independent work while Actions are pending. Do not ask the user what to do next. Keep the archival backup immutable.

## Before context exhaustion

Finish the current safe slice and commit it atomically. Record the exact commit, tests, exact-SHA Actions and unverified boundaries. Leave the next GPT one concrete first action; do not claim local tests, builds, firmware artifacts or hardware checks that were not performed.
