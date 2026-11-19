# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Live main parent SHA: `5c43773c08d50540c481bc8e0ca4b6204c63a7f5` (`test(httpd): align Samba trust checks with descriptor guard`).
- Wireless guest-isolation source `ef421978331ade679241c5bc1c10dbb6350d4004` and checker correction `8325b61ef1f4ee726e448eb81f5e4a41f44341e0` validated: exact Wireless `44592865932`, Process `44592865823`, AI docs `44592865722` PASS; XY-C1 `44592415620` passed Build/TRX/artifact on the unchanged product source.
- Samba parent-directory hardening `0e13734b95f14bdc2cd7aed926895b4933a286bf` protects `start_wins()`, `run_samba()`, and `write_smb_conf()` with effective-UID-owned 0700 no-follow parent validation before recovery. Dedicated Samba regression `44597410202` PASS. XY-C1 `44598421480` on the recent source SHA `9e51fc0...` was still IN_PROGRESS at last poll; it is the first build covering Samba production changes.
- Exact-head `9e51fc0e6939e020b6bf4cf16d0ace2282f13f4c`: Infosvr packet/signal `44598421247`, Process umbrella `44598421270`, Crond lifecycle `44598421431`, Process inventory `44598421241`, NVRAM audit `44598421188`, and other completed module Actions PASS. XY-C1 `44598421480` remained in progress. This commit changes production `services.c`, so a new XY-C1 exact-head firmware build is required.
- HTTPD Authentication/Security failure `44598421207` was a stale checker expecting the removed path-based Samba trust helper. Workflow-only commit `5c43773c08d50540c481bc8e0ca4b6204c63a7f5` corrected the descriptor-guard source contract; exact HTTPD security `44598733635` and AI docs `44598733483` PASS.
- This commit fixes an infosvr launch-result leak: positive `eval()` wait status could pass through `restart_infosvr()` although PID 1 treats only negative `action_result` as failure, clearing a failed notification marker. Launch failures now normalize to `-1`, with positive wait statuses mapped to `errno=EIO`.
- The infosvr disabled-service guard remains before process inventory, avoiding unnecessary procfs dependency when the optional daemon is disabled.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it. PR #1 remains closed and unmerged.
- No local checkout or MIPS toolchain is available in this connector-only environment. Physical router/runtime L4 remains NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — normalize infosvr launch failures for PID 1 notifications

- Parent main SHA: `5c43773c08d50540c481bc8e0ca4b6204c63a7f5`.
- Confirmed defect: PID 1 `handle_notifications()` retains incomplete notification markers only when `action_result < 0`. `start_infosvr()` returned a positive `eval("/usr/sbin/infosvr", IFNAME_BR)` wait status unchanged, and `infosvr_start_postcondition()` preserved it; a failed detached child could be acknowledged as successful and its restart marker cleared.
- This commit retains the original status in logs, maps positive encoded wait statuses to `errno=EIO`, and returns through `infosvr_start_postcondition()`. The helper maps all nonzero launch statuses to `-1`; optional disabled-service success `0` and its guard-before-inventory ordering are preserved.
- GNU89/Werror/ASan/UBSan regression covers positive wait statuses `7`/`256`, negative launcher failure, and RUNNING/ABSENT/UNKNOWN postconditions. Dedicated Infosvr and broad Process source contracts enforce PID 1's negative-only failure convention. Existing workflows compile the policy regression.
- Exact-head Infosvr, Process and XY-C1 Build/TRX/artifact for the new source SHA are pending.

## Verification boundary

- L1: source review confirms positive launch wait status violated PID 1's negative-only notification failure convention; source now returns `-1` and sets `errno=EIO` for positive statuses.
- L2: GNU89/Werror/ASan/UBSan policy tests and dedicated Infosvr / Process source contracts are committed; exact-head Actions pending.
- L3: same production-source SHA requires exact-head XY-C1 input detection, Build, TRX and artifact verification; pending.
- L4: live infosvr packet discovery, signal lifecycle and physical-router interoperability remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Infosvr packet/signal, Process integration, and XY-C1 input detector, Build, TRX and artifact for this commit.
2. Fix any real failing checker/compiler/build step without weakening PID 1's negative-status failure convention or optional-disabled no-op.
3. After L2/L3 pass, continue a separate P1 stability/security defect outside the infosvr/Samba work.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

