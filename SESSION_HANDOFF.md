# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Live main parent SHA: 2cdd33f556b925f130d5a05348bb6774fc88dea0 (test(infosvr): update normalized launcher regression marker).
- Wireless guest-isolation production SHA ef421978331ade679241c5bc1c10dbb6350d4004 and checker correction 8325b61ef1f4ee726e448eb81f5e4a41f44341e0 validated: Wireless 44592865932, Process 44592865823, AI docs 44592865722 PASS; XY-C1 44592415620 passed Build/TRX/artifact.
- Samba parent-directory hardening 0e13734b95f14bdc2cd7aed926895b4933a286bf protects start_wins(), run_samba() and write_smb_conf() with effective-UID-owned 0700 no-follow parent validation before recovery; dedicated Samba regression 44597410202 PASS.
- Product-source SHA 2e61e6b4608d30127692f5d26e939bbce92f52fc passed XY-C1 run 44600896324: Samba shared-header staging, Samba ASan/UBSan regression, production-input detector, Build XY-C1, TRX verification and artifact upload all SUCCESS. SkipDBv2 44600896667 and NVRAM 44600896559 also PASS on that exact SHA.
- Infosvr packet/signal and AI docs Actions 44602137230 / 44602137197 PASS on exact checker-only SHA 2cdd33f556b925f130d5a05348bb6774fc88dea0 after replacing the stale renamed-test marker. Process umbrella 44600895951, Process inventory 44600896622, PID 1 notification queue 44600896480, Crond 44600895984, HTTPD security/lifecycle 44600896461 / 44600896067, USB hotplug 44600896597, WAN ARP 44600896033, DNSMasq 44600896722, and other completed module checks PASS on preceding product SHA 2e61.
- Two exact-source checker failures remain being corrected in THIS commit: detect-internet signal 44600896372 failed at inline Python line 134 because it expected advisory monitor pause; WAN route status 44600896744 failed at inline line 253 for the same obsolete expectation. WAN source commit a7eae5a115dae74685e9130ff6d339b8202e5725 intentionally records and aggregates pause errors during MAN/WAN teardown while still completing cleanup. The checker updates now assert that first-error contract and log ordering.
- Current source changes have no local checkout or MIPS toolchain available in this connector-only environment. Physical router/runtime L4 remains NOT VERIFIED.
- Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify it. PR #1 remains closed and unmerged.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — align WAN monitor-pause source contracts with first-error teardown

- Parent main SHA: 2cdd33f556b925f130d5a05348bb6774fc88dea0.
- WAN source commit a7eae5a115dae74685e9130ff6d339b8202e5725 changed MAN/WAN down paths to capture notify_pause_detect_internet(), aggregate it into the existing first-error status, log failures, and still finish route/interface cleanup. MAN retains lease/NVRAM inventory unless all cleanup succeeds. This behavior is intentional and must not be reverted to advisory pause.
- The detect-internet workflow 44600896372 and WAN route workflow 44600896744 failed only because old source-contract code still searched for (void)notify_pause_detect_internet() and asserted no status aggregation. THIS commit updates both checkers to verify pause -> aggregate -> log -> cleanup/state-mutation order, return status, and no discarded pause result. No production source changes.
- Exact reruns for both workflows are pending. After they pass, select a separate source defect rather than further repeating completed module audits.
## Verification boundary

- L1: MAN/WAN down source intentionally aggregates monitor pause errors as the first teardown status and continues idempotent cleanup; the source fix is in a7eae5a.
- L2: earlier detect-internet and WAN route C policy suites had reached source-contract checks but failed on obsolete advisory-pause assumptions; checker corrections are committed in THIS slice, exact reruns pending.
- L3: XY-C1 Build/TRX/artifact passed on exact production SHA 2e61e6b (run 44600896324); this commit changes workflows/docs only and does not change firmware inputs.
- L4: live WAN DHCP/PPP transitions and physical-router route behavior remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head detect-internet signal and WAN route status Actions; confirm that the new MAN/WAN monitor-pause assertions pass.
2. If another assertion fails, fix only the stale source contract while preserving first-error status, lease retention, and complete teardown ordering.
3. After both L2 checks close, continue with a separate P1 production stability/security defect outside the just-completed WAN/DHCP and infosvr slices.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

