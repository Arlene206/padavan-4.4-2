# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main SHA for this source slice: `f33ecd5a495f78e140acb40567fccd2c6ff714b5`. Prior WAN gateway ARP reply identity source SHA `fe286b8cabd4334664c8270c5212d26a15ce6393` reached exact L2/L3 PASS; the corrected AI docs contract passed on `f3f39d7d6e7acde293982a51e9dcab76994b3c8e` and the current checker-only handoff repair passed on `f33ecd5a495f78e140acb40567fccd2c6ff714b5`.
- The preceding DHCPv6 startup errno-precedence fix is closed on exact SHA f50387e29be43e9de4a1b55344390742a4252b53. WAN DHCP callback run [44563154100](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154100), WAN route [44563153894](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153894), HTTPD security [44563153840](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153840), NVRAM transaction [44563154066](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154066), Process inventory [44563154013](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154013), and AI handoff docs [44563154047](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154047) all PASS on that exact push SHA.
- XY-C1 run [44563153885](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153885) passed its production-input detector, Build XY-C1, TRX verification and artifact upload on f50387e29be43e9de4a1b55344390742a4252b53 (L3 for that SHA). This evidence does not cover the new ARP changes below.
- Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; do not touch it.
- PR #1 remains closed and unmerged. Its prior valid CA bootstrap/HMAC/package/ROMFS work was already ported to main and verified; do not revive or re-audit it.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical-router L4 is NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — complete checked USB hotplug events and repair product build

- Parent main SHA for this repair slice: 4c331818077d8b9ddfdb5998e35e9c8e0b0add6f. Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify it.
- USB checked-notification source commit 0f719780b4d1b1d38b0300855b8424d74d033e20: policy test binaries passed but USB source contract failed on a stale assertion requiring the removed void notify_rc call. Checker-only follow-up 4c331818077d8b9ddfdb5998e35e9c8e0b0add6f fixed the assertion; exact-head USB workflow 44582391137 and AI docs 44582391240 both PASS on 4c3318.
- XY-C1 run 44578459810 on source 8fea3095d59b3278a9318d401eadad0606c193b7 failed at Build XY-C1 because five interface_down_* locals were accidentally declared in mdev_wdm_main() rather than mdev_net_main(). Later XY run 44580744748 on 0f719 also failed at the same build stage; its first causal compiler diagnostics reported all five locals undeclared in the NDIS handler. This repair moves them to mdev_net_main().
- Completes the original queue-failure scope for every USB event: checked notifier used for NDIS and TTY modem plug/unplug and now also for both printer plug/unplug events. The printer handler now returns nonzero if PID 1 did not accept queue markers. The workflow checker pins these calls and rejects void notify_rc in the printer handler. Exact-head USB and XY-C1 verification for this source repair pending.
- No local checkout/MIPS toolchain or physical printer/modem/router is available here. Do not claim L3/L4 until exact CI/build and physical tests finish.

## Verification boundary

- L1: all USB printer and modem hotplug event handlers report queue-marker acceptance; modem unplug notification is still attempted after best-effort interface/metadata cleanup failures.
- L2: modem queue policy binaries passed on 0f719; checker-only exact-head USB workflow 44582391137 passed on 4c3318. This source repair adds printer assertions; exact-head rerun pending.
- L3: XY-C1 runs 44578459810 and 44580744748 failed at Build XY-C1 due to undeclared NDIS locals. This commit moves those locals into mdev_net_main; exact-head detector, Build XY-C1, TRX and artifact pending.
- L4: physical USB printer/modem attach/detach and PID-1 queue rejection simulation NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head USB modem identity and XY-C1 Actions for this source repair. Confirm mdev_net_main compiles and all printer/modem queue source contracts pass.
2. If build still fails, inspect the first causal diagnostic and fix without weakening WDM/NDIS/TTY identity or checked-notification contracts.
3. After USB L2 and XY-C1 L3 are green, continue with an independent P1 issue outside USB.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

