# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Live main parent SHA: 2e61e6b4608d30127692f5d26e939bbce92f52fc (fix(infosvr): normalize daemon launch failure for PID 1).
- Wireless guest-isolation production SHA ef421978331ade679241c5bc1c10dbb6350d4004 and checker correction 8325b61ef1f4ee726e448eb81f5e4a41f44341e0 validated: exact Wireless 44592865932, Process 44592865823, AI docs 44592865722 PASS; XY-C1 44592415620 passed Build/TRX/artifact on its exact source SHA.
- Samba parent-directory hardening 0e13734b95f14bdc2cd7aed926895b4933a286bf protects start_wins(), run_samba(), and write_smb_conf() with effective-UID-owned 0700 no-follow parent validation before recovery. Dedicated Samba regression 44597410202 PASS. Current exact-head XY-C1 run is 44600896324.
- Exact-head 2e61e6b: Process umbrella 44600895951, Process inventory 44600896622, PID 1 notification queue 44600896480, AI docs 44600896036, Crond 44600895984, HTTPD security 44600896461, HTTPD lifecycle 44600896067, USB hotplug 44600896597, WAN ARP 44600896033, DNSMasq 44600896722 and most other service regressions PASS.
- Infosvr packet/signal 44600896016 failed only in scripts/check_infosvr_lifecycle.py because its marker list still required the removed test_start_preserves_launcher_failure name. All three GNU89/Werror/ASan/UBSan policy binaries passed, as did the packet checker. THIS commit updates the expected marker; exact-head rerun pending.
- detect_internet signal 44600896372 failed in its source-contract Python at line 134 with ValueError: substring not found; WAN route status 44600896744 failed at inline source-contract line 253 with the same exception. Inspect both checker strings and correct stale contracts without weakening DHCP/monitor teardown behavior.
- XY-C1 44600896324, SkipDBv2 44600896667 and NVRAM audit 44600896559 were IN_PROGRESS at checkpoint. Query each by exact head before making L3 claims.
- Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify it. PR #1 remains closed and unmerged.
- No local checkout or MIPS toolchain is available in this connector-only environment. Physical router/runtime L4 remains NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — correct the stale Infosvr lifecycle test marker

- Parent main SHA: 2e61e6b4608d30127692f5d26e939bbce92f52fc.
- Production fix in this parent converts every nonzero infosvr launcher status to -1 and sets errno=EIO for positive encoded wait statuses, preserving the original value in the log. GNU89/Werror/ASan/UBSan packet, signal and daemon policy binaries all PASS; the dedicated Python checker failed only because its marker list still required test_start_preserves_launcher_failure.
- THIS commit changes that assertion to test_start_normalizes_launcher_failure_for_pid1. No product source changes. Exact-head Infosvr rerun pending.
- Other independent failures remain to diagnose: detect_internet Action 44600896372 (source-contract line 134, ValueError) and WAN route Action 44600896744 (inline source-contract line 253, ValueError). XY-C1 44600896324, SkipDBv2 44600896667 and NVRAM 44600896559 were still running on the parent.

## Verification boundary

- L1: positive infosvr launch wait statuses are normalized to the negative PID 1 failure convention; source and policy unit tests are present.
- L2: all three infosvr C policy binaries passed on 2e61e6b4608d30127692f5d26e939bbce92f52fc; dedicated source checker failed on a stale renamed-test marker, corrected in THIS commit; exact rerun pending. Process umbrella and PID 1 notification queue passed.
- L3: XY-C1 Build/TRX/artifact 44600896324 was in progress on the parent; current-source result pending.
- L4: live infosvr signal/packet behavior and physical-router interoperability remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect the exact-head Infosvr rerun and verify XY-C1 Build/TRX/artifact, SkipDBv2 and NVRAM audit.
2. Diagnose detect_internet line 134 and WAN route inline line 253 ValueErrors from their actual source-contract expectations, preserving DHCP/monitor cleanup and notification safety.
3. After these independent L2/L3 items close, continue a separate P1 stability/security source fix outside infosvr, WAN and Samba.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

