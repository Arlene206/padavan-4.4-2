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

## Active task — close USB hotplug notification regression contract

- Product-source baseline: modem queue-failure propagation is committed on main as `0f719780b4d1b1d38b0300855b8424d74d033e20`; parent was `8fea3095d59b3278a9318d401eadad0606c193b7`. The checked notifier now reports `notify_rc_status()` failure, preserves/logs errno with EIO fallback, and merges the result into NDIS/TTY handler status without suppressing unplug notification after cleanup errors.
- Exact-head Actions on source SHA `0f719780b4d1b1d38b0300855b8424d74d033e20`: USB workflow `44580744698` FAILED in the WDM/NDIS source-contract step after all five GNU89/ASan/UBSan policy binaries passed; the checker still searched for the removed legacy `notify_rc("on_unplug_usb_modem")` call. This is a stale checker assertion, not evidence of a production-source failure. Process inventory `44580744598`, Process fail-closed regression `44580744806`, AI handoff docs `44580744704`, and HTTPD security `44580744667` PASS. NVRAM transaction audit `44580744629` and XY-C1 `44580744748` were IN_PROGRESS at recovery.
- THIS COMMIT changes only the USB workflow contract: require the checked `notify_modem_hotplug_event("on_unplug_usb_modem", device_name)` call to follow the existing metadata/cleanup status merge. Existing NDIS/TTY assertions still require checked plug/unplug notifications and prohibit the void `notify_rc()` wrapper.
- No local checkout or MIPS toolchain is available in this connector-only session. Physical USB modem/router L4 remains NOT VERIFIED.

## Verification boundary

- L1 source review: modem hotplug event outcomes now reflect whether secure queue markers were accepted; a failed init wake-up signal remains non-fatal after durable queue acceptance, per notify_rc.h.
- L2 host regression: pending exact-head USB modem identity workflow; it includes GNU89 -Wall -Wextra -Werror/ASan/UBSan policy binaries and source contracts for WDM/NDIS identity, modem WAN status, and hotplug queue propagation. Prior NDIS unplug-down commit reached USB L2 PASS on 44578459451.
- L3 target build: previous product source SHA 8fea3095d59b3278a9318d401eadad0606c193b7 awaits XY-C1 run 44578459810; this new hotplug_usb.c change requires its own exact-head detector, Build XY-C1, TRX verification and artifact upload.
- L4 live USB modem plug/unplug, PID-1 queue failure and WAN reconvergence on target hardware: NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the push-triggered USB modem workflow for the resulting main SHA. Confirm the corrected NDIS unplug ordering assertion passes and all five policy binaries plus the remaining WDM/NDIS and TTY contracts execute.
2. Check exact-head XY-C1 and NVRAM runs on that same SHA. A checker-only commit should remain host-only under the fail-safe production-input detector; do not claim L3 unless Build XY-C1, TRX verification, and artifact upload actually complete on a product-source SHA.
3. If USB L2 is green and no exact-head failure remains, close this source-contract slice and continue with the next independent P1 reliability issue from the roadmap in a different user-space module.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

