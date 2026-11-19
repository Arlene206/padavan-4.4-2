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

## Active task — propagate modem hotplug notification queue failures

- Parent main SHA for this source slice: 8fea3095d59b3278a9318d401eadad0606c193b7. Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify it.
- Completed prior NDIS unplug shutdown slice: exact-head USB modem identity run 44578459451, Process inventory 44578460016, Process fail-closed 44578459789, AI docs 44578459995, NVRAM audit 44578460023, and HTTPD security 44578459942 all PASS on exact SHA 8fea3095d59b3278a9318d401eadad0606c193b7. USB regression compiled and ran SCSI generic readiness, SCSI CD eject/unload, modem identity/metadata, module alternative readiness and modem WAN status policies; hotplug WDM/NDIS source contract passed. XY-C1 44578459810 remained IN_PROGRESS at last poll; L3 for 8fea is not yet verified.
- Newly confirmed adjacent defect: notify_rc.h explicitly offers notify_rc_status() to report secure queue marker publication failures, while notify_rc() discards them. mdev_net_main() and mdev_tty_main() used the void wrapper for modem NDIS/TTY hotplug events and could therefore return success even though PID 1 had not accepted the plug/unplug notification.
- THIS COMMIT replaces the four modem event call sites with a checked notifier, preserves/logs the notification errno and folds queue failures into the handler's existing metadata result. Unplug cleanup remains best-effort and notification is not skipped after an earlier interface-down or metadata error. Added policy regression for queue success/failure and first-error preservation; USB workflow source contracts pin both NDIS/TTY plug/unplug callers and prohibit void notifier use in those handlers. Exact-head USB L2/XY-C1 L3 pending.
- No local checkout/MIPS toolchain or physical USB modem/router is available here. L4 remains NOT VERIFIED.

## Verification boundary

- L1 source review: modem hotplug event outcomes now reflect whether secure queue markers were accepted; a failed init wake-up signal remains non-fatal after durable queue acceptance, per notify_rc.h.
- L2 host regression: pending exact-head USB modem identity workflow; it includes GNU89 -Wall -Wextra -Werror/ASan/UBSan policy binaries and source contracts for WDM/NDIS identity, modem WAN status, and hotplug queue propagation. Prior NDIS unplug-down commit reached USB L2 PASS on 44578459451.
- L3 target build: previous product source SHA 8fea3095d59b3278a9318d401eadad0606c193b7 awaits XY-C1 run 44578459810; this new hotplug_usb.c change requires its own exact-head detector, Build XY-C1, TRX verification and artifact upload.
- L4 live USB modem plug/unplug, PID-1 queue failure and WAN reconvergence on target hardware: NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head USB modem identity, Process, AI docs and XY-C1 Actions for this checked-notification commit. Confirm modem queue regression, source contracts and firmware Build/TRX/artifact.
2. If any contract fails, inspect the first causal log and correct the source/checker without reverting to the void notify_rc() wrapper or suppressing notification after cleanup failure.
3. After closing this modem queue slice, continue with the next independent P1 reliability/security issue in a different subsystem.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

