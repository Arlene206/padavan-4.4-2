# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Live main parent SHA: `94736c388977ca77fa9cf3a41833093707e9d618`.
- First Samba parent-directory fix `536f6f0e6c5a7835a38784d277c0d51eb692eca2` removed the WINS 0777 mkdir and added no-follow parent preparation. Dedicated Samba run `44593849014` SUCCESS. XY-C1 run `44593848957` failed only because its test compile omitted `-I trunk/user/rc/src`; workflow-only commit `287aa1cb8105de24dc6683485087ccdbcb12f062` corrected it. Exact XY-C1 run `44595726202` passed source/input detection; Build/TRX/artifact were skipped because it was workflow-only.
- Wireless rollback checker commit `8325b61ef1f4ee726e448eb81f5e4a41f44341e0`: exact Wireless `44592865932`, Process lifecycle `44592865823`, and AI docs `44592865722` PASS.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it. PR #1 remains closed and unmerged.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical router/runtime L4 remains NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — secure all Samba passdb and transaction entry points

- Parent main SHA: `94736c388977ca77fa9cf3a41833093707e9d618`.
- Review after the first guard found `run_samba()` still created `/etc/samba` as 0777 before path-based chmod, and public `write_smb_conf()` recovered the journal without securing its parent. All three paths touch the same passdb/config journal.
- This follow-up enforces exact effective-UID-owned 0700 via an opened `O_DIRECTORY|O_NOFOLLOW` fd and post-`fchmod` verification. Guards precede recovery in `start_wins()`, `run_samba()`, and `write_smb_conf()`; the legacy path-based helper and remaining 0777 mkdir are removed.
- Initial XY-C1 compile omitted `-I trunk/user/rc/src`; commit `287aa1c` corrected it and its detector passed (Build/TRX/artifact were skipped on workflow-only SHA). Initial Samba run `44593849014` PASS; exact all-entrypoint regression and product-source Build pending on this follow-up.
- Wireless/Process/AI runs `44592865932`, `44592865823`, `44592865722` PASS on `8325b61`.

## Verification boundary

- L1: all three transaction entry points identified; passdb/recovery parent must remain effective-UID-owned and exactly 0700.
- L2: GNU89/Werror/ASan/UBSan strict-mode regression and three-callsite source contracts pending on this follow-up SHA.
- L3: exact-head XY-C1 input detector, Build, TRX and artifact upload pending for this Samba production-source change.
- L4: live Samba authentication, WINS browse/registration and physical-router runtime remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect Actions by exact main `head_sha`: Samba share configuration regression, XY-C1 Build/TRX/artifact, Process inventory and AI docs.
2. Diagnose the first failing assertion/compiler output; maintain 0700 parent preparation before recovery in all three transaction entry points.
3. After L2/L3 pass, continue with a separate P1 stability/security defect outside Samba transaction publication.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

