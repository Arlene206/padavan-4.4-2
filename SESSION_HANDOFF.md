# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Main parent for this firmware link fix: `89a43d9afab380b167b8b47a80b4b3837976e33f`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`, unchanged/read-only.
- Exact-head 09ea XY-C1 `43947164750` reached the MIPS link step but failed with undefined references to `rc_first_service_start_status` from `firewall_ex.o`. The helper is `static inline` in `service_start_status.h`; `firewall_ex.c` used it without including that header. This commit includes the header and adds a source-check assertion to prevent recurrence.
- Exact-head 89a43d9 Firewall Startup `43948306770`, Process regression `43948306885`, HTTPD `43948306816`, AI handoff `43948306828`, USB modem `43948306849`, infosvr `43948306784`, AliDDNS `43948306867`, AdGuardHome `43948306841`, and AliyunDrive `43948306780` passed. NVRAM `43948306748` was pending at last poll; XY-C1 `43948306746` was actively cross-building.
- Exact-head 39190 NAT source SHA failed only stale validation contracts at that stage: Firewall Startup/Process shared the policy-variable mismatch; HTTPD expected the old HW NAT helper signature. Those were corrected in 89a43d9. New-SHA firmware and NAT unit verification is required after this fix.

## Active task

The Shadowsocks, persistent post-firewall and Optware hook paths now propagate non-ENOENT staging failures, command failures and staged-file cleanup errors to `start_firewall_ex()`, while continuing independent hooks. Missing optional scripts remain no-op; a successful built-in firewall stays enabled even when an extension fails. Staged-path removal retains its path when unlink fails.

The implementation is present, but the integration suites exposed two stale checker assumptions: a doubly escaped whitespace regex and an outdated `static void` signature. This slice fixes those validation contracts only.

Next P1: audit NAT helper and HW NAT/SFE module load/unload postconditions in `net.c`. Verify required ALG failures remain visible and optional accelerator failures do not block ordinary WAN/firewall service. Add injectable unload postcondition tests.

## Verification boundary

- Exact-head 09ea XY-C1 `43947164750` failed at link: `firewall_ex.o` referenced `rc_first_service_start_status` without including its header definition. The inline function resides in `service_start_status.h`; this commit adds the missing include.
- 89a43d9 Firewall Startup, Process regression, HTTPD, AI handoff, USB modem, infosvr, AliDDNS, AdGuardHome and AliyunDrive passed. NVRAM was pending and XY-C1 was still building at the last poll.
- 39190 NAT policy and test code has not yet completed green exact-SHA CI; its initial source-checker failures are fixed by 89a43d9, and the current firmware link repair must be verified on a fresh SHA.
- No XY-C1 Build/TRX/artifact success is claimed for the new SHA until the corresponding exact-SHA job passes. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect new-SHA Firewall Startup, Process regression, HTTPD and XY-C1 cross-build. Verify the `service_start_status.h` inline helper now resolves at firmware link.
2. Verify the NAT load/unload policy unit test under ASan/UBSan and the HW NAT/SFE path assertions; diagnose any new source/compile issue.
3. Keep commits main-only and atomic with non-forced expected-SHA CAS. Frozen backup read-only; physical-router L4 NOT VERIFIED.
