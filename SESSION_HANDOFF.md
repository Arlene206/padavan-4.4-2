# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Live main at checkpoint parent: `981857e60def0d4237253a3c0b061b85111ebbf5` (`test(wan): normalize route source contract whitespace`).
- The WAN off-subnet gateway /32 lifecycle correction is closed for this slice. Exact-head WAN route Action `44586115401` and AI handoff documentation Action `44586115612` both completed SUCCESS on `981857e60def0d4237253a3c0b061b85111ebbf5`; the whitespace-only source checker correction passed without changing production C.
- The product-source SHA `29fa24741613095593a03f4e602f1d77636a21e0` passed XY-C1 run `44585450325`: input detector, production-input check, Build XY-C1, TRX verification and artifact upload all completed SUCCESS. The descendant `981857e60def0d4237253a3c0b061b85111ebbf5` changed only workflow assertions and documentation, not product source.
- The independent Wireless radio baseline on `29fa...` passed Action `44585450126`; this session adds a new production wireless fail-closed correction, so exact-head Wireless and XY-C1 validation must be rerun on the resulting commit.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never touch it. PR #1 remains closed and unmerged; do not revive or re-audit.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical router / radio L4 remains NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — fail closed on incomplete guest ebtables isolation

- Main parent SHA: `981857e60def0d4237253a3c0b061b85111ebbf5`.
- Confirmed defect: `restart_guest_lan_isolation()` recorded missing/unknown `ebtable_filter`, failed `ebtables -F/-X`, and individual rule-add failures, but returned an error while leaving already-running, bridged AP guest interfaces active. `control_guest_wl/rt()` only rolled back a guest AP created by the current request (`rollback_candidate`), so a pre-existing guest AP could remain connected without its configured LAN isolation.
- This commit makes AP-mode ebtables intent independent of a potentially UNKNOWN interface-flags probe, tracks new failures during module/flush/chain/rule setup, and on any such failure brings down, detaches and independently verifies every configured ebtables-isolated guest AP, including pre-existing guests. RT3352 MII uses the existing iNIC interface/VLAN mapping. Rollback now still issues a down request after an UNKNOWN initial flags probe and relies on its separate postcondition check.
- New GNU89 policy cases cover UNKNOWN-triggered down requests and the rule that only a newly failed setup with an enabled guest-isolation mask requires AP rollback. The Wireless workflow already covers the source, policy test and checker; exact-head L2 and XY-C1 L3 are pending.
- The preceding WAN route checker task is closed: exact-head WAN route `44586115401` and AI docs `44586115612` both PASS on `981857e60def0d4237253a3c0b061b85111ebbf5`. XY-C1 Build/TRX/artifact completed SUCCESS for the unmodified product source `29fa...` in run `44585450325`; do not repeat that earlier run as evidence for this new wireless source change.
## Verification boundary

- L1: source review confirmed a fail-open case for pre-existing AP guest interfaces on ebtables setup failure; the current slice now attempts AP down + bridge detach + separate down-state verification.
- L2: GNU89/Werror/ASan/UBSan policy regression and production-source checker are wired into the existing Wireless radio workflow; exact-head execution pending.
- L3: current wireless production-source SHA requires exact-head XY-C1 detector, Build XY-C1, TRX and artifact verification; pending.
- L4: AP/STA radio transitions, ebtables behavior, RT3352 MII/iNIC VLAN variants and physical-router runtime remain NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-head Actions for this commit by exact `head_sha`: Wireless radio regression, AI handoff docs, Process lifecycle if triggered, and XY-C1 Firmware Validation.
2. On a Wireless failure, fix the first failing source/test contract without weakening the fail-closed AP down/detach/verify behavior; if XY-C1 fails, diagnose the actual compile/build step before claiming L3.
3. After the slice validates, continue with a separate P1 stability/security defect outside this guest-wireless and prior WAN/USB work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

