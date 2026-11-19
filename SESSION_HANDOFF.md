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

## Active task — fail closed on guest ebtables module-state uncertainty

- Parent main SHA for this source slice: `f33ecd5a495f78e140acb40567fccd2c6ff714b5`. Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it.
- Closed preceding iNIC force-disable status fix: product source `3d86aebbef26086a4383489efc4470e35e91b9df` now includes the failed `rai0` down result in the sticky return status while continuing bridge detach, PHY isolation, RGMII disable and reset. Exact-head Wireless radio regression/source contract `44574370283` PASS on checker-follow-up SHA `f33ecd5a495f78e140acb40567fccd2c6ff714b5`; Process lifecycle regression `44573883051` and NVRAM audit `44573883023` PASS on product SHA `3d86aebbef26086a4383489efc4470e35e91b9df`; AI handoff docs `44574368349` PASS on `f33ecd5a495f78e140acb40567fccd2c6ff714b5`. XY-C1 run `44573883318` remains IN_PROGRESS on product SHA `3d86aebbef26086a4383489efc4470e35e91b9df`; detector and all pre-build host gates passed, while Build/TRX/artifact were still running at the last poll.
- Source gap: `restart_guest_lan_isolation()` uses `is_module_loaded("ebtables")`, collapsing the tri-state module probe's UNKNOWN/error result into false and silently skipping disable cleanup; enable also records `module_smart_load("ebtable_filter")` as successful even though that API returns zero both when already-loaded and when loading fails.
- THIS COMMIT requires a confirmed post-load module state before flushing or installing guest-isolation rules. During disable, a confirmed loaded state permits cleanup and subsequent unload; confirmed absence is a no-op; UNKNOWN records the probe error, still attempts best-effort `-F`/`-X` cleanup, but never unloads modules whose state cannot be trusted. New host policy tests cover module presence errno mapping and all three disable actions; the production checker verifies that probing, error recording, cleanup and unload gates remain ordered. Exact-head Wireless L2 and XY-C1 L3 are pending for THIS COMMIT.
- No local repository checkout/MIPS toolchain or physical router is available in this connector-only environment. L4 physical behavior remains NOT VERIFIED.

## Verification boundary

- L1 source review: guest ebtables enable/disable transitions now preserve module UNKNOWN and never publish requested isolation rules without confirming `ebtable_filter` is loaded.
- L2 host regression: pending exact-head Wireless radio workflow, including GNU89 `-Wall -Wextra -Werror`, ASan/UBSan guest policy binary and `scripts/check_guest_lan_isolation_status.py` source contract. The prior iNIC Wireless source contract passed on `44574370283`.
- L3 target build: prior iNIC source SHA `3d86aebbef26086a4383489efc4470e35e91b9df` still awaits the in-progress XY-C1 Build/TRX/artifact run `44573883318`; this new `net_wifi.c` product-source change requires its own exact-head detector, Build XY-C1, TRX and artifact verification. Parent firmware evidence is not carried forward.
- L4 live guest AP/RT3352 MII hardware behavior: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Wireless radio and AI handoff Actions for the guest ebtables tri-state source commit. Confirm all GNU89 policy binaries, sanitizer run and production source checker.
2. Poll XY-C1 run `44573883318` for the iNIC source SHA `3d86aebbef26086a4383489efc4470e35e91b9df`, then inspect the new exact-head XY-C1 run separately: production-input detector, Build XY-C1, TRX verification and artifact upload.
3. If any relevant check fails, inspect the first causal job/log and repair it without weakening fail-closed UNKNOWN handling, best-effort cleanup, or the no-unload-on-UNKNOWN rule. After closing this slice, pick the next high-priority unfinished P1 from a different subsystem.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

