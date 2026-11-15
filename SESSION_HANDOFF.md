# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at start of this slice: `3f6ca487db9729987cdc6b3a49a2facf479bae7e`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Current WAN source includes direct IPoE static-address error gating, checked physical-interface bring-up and AP-client WISP address cleanup; latest route slice separates critical gateway host/default route status from optional configured static/DHCP/DNS routes and propagates required failures into static MAN PPTP/L2TP startup gating. DHCP/ZeroConf callbacks keep aggregate status; PPPoE async/no-wait is preserved.
- Exact 3f6 Actions: WAN callback #43755792972 FAIL (C policy executables passed; Python source contract failed with `NameError: wanx is not defined`, and contains a stale static MAN call assertion); WAN route status #43755792869 PASS; AI docs #43755792956 PASS; Process inventory #43755793007 PASS; infosvr #43755792908 PASS; AdGuardHome #43755793061 PASS; AliyunDrive #43755792904 PASS; AliDDNS #43755792868 PASS. The 41ea module runs remained PASS: Process lifecycle #43755418852; Process inventory #43755418919; WAN teardown #43755418845; DNSMasq #43755418936; HTTPD #43755418812; VPN #43755418832; detect_internet #43755418808; detect_link #43755418966; USB modem #43755418970; infosvr #43755418948; WAN ARP #43755418839; AdGuardHome #43755418976; miniupnpd #43755418910; AliDDNS #43755418826; AliyunDrive #43755418815. NVRAM #43755793079 remained in ioctl audit at last check.
- Static MAN route classification source is already in parent 41ea; 3f6 repaired the exact WAN route regression and its Action passed. The WAN callback checker still lacked a local `wanx` function binding before route-specific assertions, and still expected the old `man_up()` call.
- The WISP cleanup helper still returned `void` while its caller ignored the unrecoverable failure. This slice makes the helper return a status and aborts caller startup before any IPTV DHCP/ZeroConf client is launched.
- Backup and archive index/shards remain unchanged. No physical-router/L4 validation is claimed.

## Active task

### Complete WISP caller failure propagation and WAN callback source contract

`config_apcli_wisp()` now returns zero for no-op/success and returns the final interface-down status when the previous MAN interface cannot be made safe. `start_wan()` checks this status, removes callback links and aborts before launching IPTV DHCP/ZeroConf clients, NAT refresh or WAN protocol startup. WISP interface publication remains after the old interface cleanup gate.

The WAN callback workflow now verifies the WISP return/caller abort and correct order, defines the `wanx` function body before testing MAN required-route propagation, and checks the updated `man_up_internal(..., &gateway_route_status)` call.

This source, regression workflow and checkpoint docs are one atomic commit.

## Verification boundary

- Parent main: `3f6ca487db9729987cdc6b3a49a2facf479bae7e`; frozen backup: `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact parent WAN callback #43755792972 FAIL only at the workflow Python source contract after all compiled policy tests passed; immediate cause: `wanx` local variable was never initialized before use at the required-MAN route assertions. The static MAN assertion also still looked for the old `man_up()` call and must be updated as part of this source contract.
- Exact parent WAN route #43755792869 PASS; AI docs #43755792956 PASS; Process inventory #43755793007 PASS; infosvr #43755792908 PASS; AdGuardHome #43755793061 PASS; AliyunDrive #43755792904 PASS; AliDDNS #43755792868 PASS.
- Exact parent source 41ea module Actions listed above completed PASS; NVRAM #43755793079 remained in progress.
- No local target compile is claimed. Exact new-head WAN callback and XY-C1 Build/TRX/artifact Actions must be checked; no current-source L3 until they pass and no L4 claim.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head WAN callback and XY-C1 Actions for this source slice; inspect full logs if a new assertion fails.
2. Require WAN callback contract PASS and latest-source XY-C1 Build, TRX verification and artifact upload before L3 claim.
3. After the caller failure path is validated, resume the broader user-space stability/security ledger; do not reopen the now-covered MAN route classification without new evidence.
4. Preserve PPPoE async/no-wait, IPv6, backup immutability and archived log shards/index.
