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
- The 8f550 inventory commit integrated whole tracked-user-tree lifecycle scanning into Process CI, prints `pids`, DNSMasq status APIs and `killall` candidates, and runs on every `trunk/user/**` change; exact-head inventory passed and uploaded artifact #14186597970.
## Active task

### Fail closed when AiDisk disables Samba or FTP

The exact 8f550 whole-tree report established there are no callers of `is_dns_dhcpd_run()` in the tracked user tree; its only hits are the `rc.h` declaration and `services_ex.c` definition. It also identified 27 real `pids()` callsites. In `httpd/src/aidisk.c:ej_set_AiDisk_status()`, both disable branches used `if (!pids(...)) goto success` before invoking the RC stop handler. Since legacy `pids()` returns 0 for a NULL list, an UNKNOWN procfs scan could be misreported as ABSENT, skipping shutdown and returning UI success after only changing NVRAM.

THIS COMMIT removes those two prechecks: disabling SMB/FTP always invokes `/sbin/stop_samba` or `/sbin/stop_ftp`. Both RC stop handlers already use tri-state process inventory, check the stop postcondition and return failure for UNKNOWN; the UI propagates nonzero status. A source-integration regression rejects `pids()` in this callback and requires both commands. XY-C1 path filters now include `aidisk.c`.


## Verification boundary

- The complete inventory is verified on 8f550: Process inventory #43776729995 PASS, scanner self-test PASS, 6,255 text files / 9,501 tree entries, no unread candidates or read errors; artifact #14186597970.
- THIS COMMIT changes the AiDisk CGI source, Process source-integration checker, XY-C1 path filter and canonical dev log / handoff / roadmap. Exact-head Process, AI docs and XY-C1 Actions must validate this fix; L3 only after Build XY-C1, TRX verification and artifact upload all pass on the same SHA.
- aebfd65 remains the verified product-source L3 baseline (Build/TRX/artifact #43775390773 PASS); this commit changes HTTPD product source, so that earlier L3 does not cover the new AiDisk code. No physical-router/L4 validation is claimed.

## Exact first action for the next GPT

1. Inspect exact-head Process lifecycle, AI handoff-doc and XY-C1 Actions for THIS COMMIT. Confirm the AiDisk source contract passes and the firmware workflow completes Build XY-C1, TRX verification and artifact upload on the exact same SHA.
2. Continue the remaining 25 real legacy `pids()` callsites from the 8f550 inventory, prioritizing consequential state-changing consumers; do not globally alter `pids()`. The `is_dns_dhcpd_run()` wrapper has no in-tree callers and is not an active migration target.
3. Continue auditing direct name-wide signals/shutdown hooks from the inventory artifact; preserve frozen backup and archived log inventory. No L4 without hardware.
