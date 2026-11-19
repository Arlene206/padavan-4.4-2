# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Live main at checkpoint parent: `ef421978331ade679241c5bc1c10dbb6350d4004` (`fix(wireless): fail closed on guest isolation setup errors`).
- WAN route checker correction is closed: WAN route `44586115401` and AI handoff documentation `44586115612` completed SUCCESS on `981857e60def0d4237253a3c0b061b85111ebbf5`.
- The guest ebtables fail-closed production change is on `ef421978331ade679241c5bc1c10dbb6350d4004`. Its exact-head Wireless radio Action `44592415803` compiled and passed the three GNU89 policy binaries, including the guest-isolation ASan/UBSan regression, then failed in `scripts/check_guest_lan_isolation_status.py` at line 161. The checker still expected the old `if (status != 0) return status` path in the down callback; the new implementation intentionally attempts interface-down even after an UNKNOWN flags probe and verifies the result separately. The correction in this follow-up distinguishes the new down-callback contract from the unchanged verification-callback UNKNOWN gate.
- On exact SHA `ef421978331ade679241c5bc1c10dbb6350d4004`, AI docs `44592415781`, watchdog `44592415729`, iNIC signal `44592415628`, and Process inventory `44592415914` completed SUCCESS; Process lifecycle regression `44592415657`, NVRAM `44592415639`, HTTPD security `44592415698`, and XY-C1 `44592415620` were still in progress at the last poll. Recheck exact-head outcomes.
- Earlier product SHA `29fa24741613095593a03f4e602f1d77636a21e0` passed XY-C1 `44585450325` through input detector, Build XY-C1, TRX and artifact upload, but the current wireless product-source change requires its own exact-head L3 validation.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never touch it. PR #1 remains closed and unmerged; do not revive or re-audit.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical radio/ebtables L4 remains NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — repair the stale UNKNOWN guest-rollback checker assertion

- Main parent SHA: `ef421978331ade679241c5bc1c10dbb6350d4004`.
- The production fail-closed fix is already committed: AP-mode ebtables intent is preserved when link-state inspection is UNKNOWN; module/flush/chain/rule setup failures take every configured ebtables guest AP offline, detach its bridge port and verify DOWN, including pre-existing guests. The rollback down callback now attempts down on UNKNOWN rather than returning before the safety action.
- Exact Wireless run `44592415803` on this source SHA compiled and passed the radio policy, bridge-membership policy, and guest-isolation GNU89/Werror/ASan/UBSan binaries. It failed only because the Python source checker still required `if (status != 0)` inside both rollback callbacks; that assumption is valid for the postcondition verifier but no longer for the down-request callback. This follow-up removes that stale assertion and verifies each callback's distinct contract.
- Exact-head AI docs `44592415781`, watchdog `44592415729`, iNIC signal `44592415628`, and Process inventory `44592415914` passed on `ef421978331ade679241c5bc1c10dbb6350d4004`. Recheck the remaining exact-head runs and rerun Wireless after the checker correction.
- XY-C1 run `44592415620` was still in progress on production source `ef421978331ade679241c5bc1c10dbb6350d4004`; confirm detector, Build, TRX and artifact for L3. A checker-only descendant does not change production firmware inputs.

## Verification boundary

- L1: source review confirms the pre-existing-guest exposure on ebtables setup failure and the new down/detach/verify remediation.
- L2: all three C policy binaries passed on `ef421978331ade679241c5bc1c10dbb6350d4004`; the first exact-head Python source checker failed on a stale callback assertion at line 161. Checker correction is committed in this follow-up; exact Wireless rerun pending.
- L3: XY-C1 `44592415620` on `ef421978331ade679241c5bc1c10dbb6350d4004` was in progress at last poll; exact production-input detector, Build XY-C1, TRX and artifact outcome pending.
- L4: AP/STA radio transitions, ebtables behavior, RT3352 MII/iNIC VLAN variants and physical-router runtime remain NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-head Actions for the checker-correction SHA; confirm Wireless radio regression, AI handoff docs, Process lifecycle if triggered, and XY-C1 Firmware Validation by exact `head_sha`.
2. If the Wireless checker still fails, fix its stale assertion without reverting the UNKNOWN-state down request or weakening the fail-closed AP down/detach/verify behavior. Diagnose an XY-C1 error from the actual failed build step.
3. After L2/L3 close, continue with a separate P1 stability/security defect outside the guest-wireless and prior WAN/USB slices.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

