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

## Active task — preserve NDIS interface-down failures during USB removal

- Parent main SHA for this source slice: `f0406397927cc648c7fd70c1961fb3aba1fd1243`. Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it.
- Previous guest ebtables module-state fix: exact-head Wireless radio regression/source contract `44576171024` PASS; AI docs `44576170956`, NVRAM `44576171017`, Process fail-closed `44576171294`, Process inventory `44576170689`, HTTPD security `44576170606`, iNIC signal `44576171003`, and watchdog signal `44576171120` all PASS on exact source SHA `f0406397927cc648c7fd70c1961fb3aba1fd1243`. XY-C1 run `44576170960` remained IN_PROGRESS at last poll. The preceding iNIC source run `44573883318` later completed PASS (Build XY-C1, TRX verification and artifact upload) on its exact product SHA `3d86aebbef26086a4383489efc4470e35e91b9df`.
- Source gap: `mdev_net_main()` discarded `ifconfig(device_name, 0, "0.0.0.0", NULL)` on USB NDIS removal. The physical interface may already have disappeared (expected ENODEV), but a failed down operation on an extant interface or an UNKNOWN interface-state probe must not be reported as successful removal.
- THIS COMMIT records the `ifconfig()` result, re-probes interface flags only on failure, treats a confirmed ENODEV as the normal physical-unplug race, preserves the ifconfig failure when the interface still exists, and returns failure when the fresh state probe is UNKNOWN. It still notifies the WAN owner after best-effort cleanup failure. GNU89 regression covers existing, removed, and UNKNOWN states; the USB-modem workflow source contract pins operation order. Exact-head USB L2 and XY-C1 L3 are pending.
- No local checkout/MIPS toolchain or physical modem/router is available here. Hardware L4 is NOT VERIFIED.

## Verification boundary

- L1 source review: USB NDIS remove no longer drops an extant-interface shutdown failure or an uninspectable link-state error; expected physical removal remains a no-op.
- L2 host regression: pending exact-head USB modem identity workflow, including GNU89 `-Wall -Wextra -Werror`/ASan/UBSan modem WAN policy binary and hotplug source integration contract. Previous ebtables Wireless L2 passed on exact SHA `f0406397927cc648c7fd70c1961fb3aba1fd1243`.
- L3 target build: guest ebtables product-source SHA `f0406397927cc648c7fd70c1961fb3aba1fd1243` still awaits XY-C1 run `44576170960`; this new `hotplug_usb.c` product source change requires its own exact-head detector, Build XY-C1, TRX verification and artifact upload.
- L4 live USB NDIS unplug/replug and WAN recovery on target hardware: NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head USB modem identity and AI docs Actions for this NDIS unplug status commit. Confirm the new unplug-state GNU89 regression and source ordering contract, while preserving the WDM/TTY/NDIS identity tests.
2. Poll XY-C1 run `44576170960` for the guest ebtables product-source SHA `f0406397927cc648c7fd70c1961fb3aba1fd1243`, then inspect the new exact-head XY-C1 workflow for detector, Build XY-C1, TRX and artifact.
3. On failure, inspect the first causal log and fix without treating UNKNOWN as absence or suppressing physical unplug notification. After closing this slice, continue with the next independent P1 item.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

