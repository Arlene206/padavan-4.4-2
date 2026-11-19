# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Live main parent SHA: `8325b61ef1f4ee726e448eb81f5e4a41f44341e0`.
- Wireless isolation checker correction commit `8325b61ef1f4ee726e448eb81f5e4a41f44341e0` fixed the stale assertion for UNKNOWN-state guest shutdown. Exact-head Wireless run `44592865932`, Process lifecycle regression `44592865823`, and AI docs `44592865722` all PASS.
- Parent guest-isolation production source `ef421978331ade679241c5bc1c10dbb6350d4004`: Process inventory `44592415914`, AI docs `44592415781`, watchdog `44592415729`, iNIC `44592415628`, HTTPD security `44592415698` passed. NVRAM transaction audit `44592415639` passed. XY-C1 build `44592415620` was still IN_PROGRESS at last check; check it only for completion, do not repeat prior audits.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it. PR #1 remains closed and unmerged.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical router/runtime L4 remains NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — harden Samba state-directory permissions before WINS recovery

- Parent main SHA: `8325b61ef1f4ee726e448eb81f5e4a41f44341e0`.
- `start_wins()` used `mkdir_if_none("/etc/samba", "777")`. PID 1 has a permissive umask, so a missing directory could be created world-writable; existing paths were accepted without type/owner/mode validation. This directory parents `smbpasswd` and the root-only `.config-transaction` journal, allowing local users to replace/remove passdb or recovery entries despite the child directory's 0700 mode.
- The source fix adds descriptor-based preparation before recovery: `mkdir(...,0755)`, `O_DIRECTORY|O_NOFOLLOW`, effective-UID ownership validation, removal of group/other write bits on legacy directories, and post-`fchmod` validation. Invalid symlink/non-directory/foreign-owner paths fail closed.
- GNU89 ASan/UBSan tests cover umask 0000 creation, repair from 0777 to 0755, preservation of 0711, and file/symlink rejection. XY-C1 workflow runs the regression and a production call-order contract. Exact-head tests and firmware Build/TRX/artifact are pending.
- Prior wireless checker failure on `ef421` was corrected in `8325b61`; exact Wireless, Process and AI docs reruns PASS on `8325b61`.

## Verification boundary

- L1: source review confirms the insecure 0777 parent affected both the passdb and Samba transaction journal.
- L2: new GNU89/Werror/ASan/UBSan permission regression and production WINS call-order contract pending on exact source SHA.
- L3: exact-head XY-C1 input detector, Build, TRX verification and artifact upload pending for this Samba production-source change.
- L4: live Samba authentication, WINS browse/registration and physical-router runtime remain NOT VERIFIED.

## Exact first action for the next GPT

1. Query Actions by the exact current main `head_sha`; inspect the Samba permission regression and XY-C1 detector, Build, TRX and artifact results.
2. Diagnose any first failing assertion or compiler error without weakening owner, no-follow or group/other write restrictions.
3. After L2/L3 pass, continue with a separate P1 stability/security defect outside this Samba directory fix.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

