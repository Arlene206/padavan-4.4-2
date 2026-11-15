# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current main parent for whole-tree lifecycle inventory integration: `4997afa4c480230727aa095f55b89b04f1e4fa73`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- All eight exact-head 4997 Actions PASS: infosvr #43775696102, AdGuardHome #43775696141, Process inventory/lifecycle #43775696137, NVRAM #43775696126, SkipDBv2 #43775696130, AliyunDrive #43775696121, AI handoff docs #43775695951 and AliDDNS #43775696124.
- Exact 4997 SkipDB job #133001359689 passed the reducer-wrapped GPIO preflight, Configure, all SkipDBv2 targets and regression tests.
- Exact aebfd65 USB storage #43775390744 PASS including Transmission/Aria host + firmware BusyBox 1.24.2 ash PID-state regressions; Process inventory #43775390816, Process lifecycle #43775390764, AI docs #43775390851, infosvr #43775390910, AdGuardHome #43775390873, AliDDNS #43775390953, AliyunDrive #43775390850 and NVRAM #43775390849 PASS.
- Exact aebfd65 XY-C1 #43775390773 SUCCESS: Build XY-C1, TRX verification and artifact upload all passed on this source SHA. Commits after aebfd65 through 4997 changed only CI contracts/docs, not firmware product files; L3 is VERIFIED for current product-source tree aebfd65. No physical-router/L4 validation.
- Current source investigation: residual legacy process APIs and direct name-wide commands across `trunk/user/**`. `services_ex.c:is_dns_dhcpd_run()` still calls legacy `pids("dnsmasq")`; `dnsmasq_process_status()` already provides RUNNING/ABSENT/UNKNOWN nearby, but external callsites must be located/classified before changing legacy semantics.
- This commit integrates the whole tracked-user-tree lifecycle lexical inventory into Process CI, prints `pids`, DNSMasq status APIs and `killall` candidates, and runs on every `trunk/user/**` change.
## Active task

### Produce a complete tracked-user-tree process lifecycle inventory

The previous audit identified `services_ex.c:is_dns_dhcpd_run()` as a remaining legacy Boolean consumer, but external callers are not fully classified. The shared tri-state scanner exists and `dnsmasq_process_status()` already exports RUNNING/ABSENT/UNKNOWN; do not change `pids()` or exported Boolean semantics until every caller is identified. The Process workflow now runs `scripts/audit_process_lifecycle.py` against the exact checked-out tree on every `trunk/user/**` change and prints all `pids()`, DNSMasq API and `killall` candidates. Use that exact-head report to select the smallest safe source migration, add regression coverage and verify before broadening.
## Verification boundary

- All eight exact-head 4997 Actions PASS: infosvr #43775696102, AdGuardHome #43775696141, Process inventory/lifecycle #43775696137, NVRAM #43775696126, SkipDBv2 #43775696130, AliyunDrive #43775696121, AI docs #43775695951 and AliDDNS #43775696124.
- SkipDB job #133001359689 passed source integration, Configure, all targets and regression tests; the obsolete GPIO source-contract block is closed.
- Exact aebfd65 USB storage #43775390744 and NVRAM #43775390849 PASS. Exact aebfd65 XY-C1 #43775390773 passed Build XY-C1, TRX verification and artifact upload. Product sources are unchanged between aebfd65 and 4997; current-source L3 verified for that product tree.
- This inventory commit modifies only the Process workflow, lexical audit script and checkpoint docs. The new exact-head Process run must prove the whole-tree scan works and emit consumer lists; no source migration result is claimed yet.
- No physical router/L4 validation is claimed.

## Exact first action for the next GPT

1. Inspect exact-head Process inventory/lifecycle run for this commit. Confirm scanner self-test and whole-tree inventory pass, then capture the complete legacy `pids()`, DNSMasq exported API and name-wide `killall` candidate lists from the job log.
2. Trace every callsite of `is_dns_dhcpd_run()` before changing its RUNNING/ABSENT/UNKNOWN contract. Migrate only a bounded consumer group with explicit UNKNOWN handling and regression coverage; do not modify legacy `pids()` globally.
3. Continue broad lifecycle review on remaining direct name-wide signals and shutdown hooks; preserve frozen backup and archived log inventory. No L4 without hardware.
