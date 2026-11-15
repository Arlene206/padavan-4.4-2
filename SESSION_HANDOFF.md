# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main before this direct IPoE DHCP status slice: `485d7c782c57b3fc0cb9cf4e42dcf0a79ba6665a`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Exact-main WAN DHCP callback #43749781222 PASS, AI handoff docs #43749781375 PASS, Process inventory/lifecycle #43749781215 PASS and Process inventory artifact #43749781176 PASS on `485d7c782c57b3fc0cb9cf4e42dcf0a79ba6665a`. XY-C1 #43749781299 was IN_PROGRESS at Build XY-C1 at last poll; TRX and artifact steps remain pending.
- Exact XY-C1 #43748599786 on product-source commit `5bbaaba493f63b0e58bac035e750907304fd9939` completed successfully: Build XY-C1, TRX verification and artifact upload all PASS. The later `ab9ae` and `97ff` commits only adjusted test contracts/docs; firmware product source remained identical to 5bbaa, so L3 is VERIFIED for that source tree. No physical-router/L4 claim is made.
- Dynamic IPTV client status propagation is committed at `5bbaaba493f63b0e58bac035e750907304fd9939`; exact regression contract now passes on `97ff`.
- Root `dev_log.md` contained 92 entries before this slice; append in-place, no rotation. Keep `docs/dev-log/INDEX.md` and archived shards unchanged.

## Active task

### Propagate direct IPoE DHCP launch failures into WAN error state

The earlier WAN slice returns the launch status from `launch_wanx()` for USB/PPP branches while preserving PPPoE asynchronous startup and PPTP/L2TP wait behavior. A separate direct IPoE DHCP path in `start_wan()` still discarded `start_udhcpc_wan(wan_ifname, unit, 0)` even though the function returns an execution status. A failed DHCP client spawn therefore left the persisted WAN `err` field clear.

This source change captures `dhcp_status`, sets `wan_err = 1` and logs interface/status on failure, then persists `wan_err` for both static and DHCP IPoE paths. It does not abort independent IPv6 startup: IPv6 is still attempted and reports its own status. The WAN workflow asserts result assignment, failure log/state, persisted error field and ordering before IPv6.


## Verification boundary

- Parent main `485d7c782c57b3fc0cb9cf4e42dcf0a79ba6665a`: exact WAN DHCP callback #43749781222 PASS; AI handoff docs #43749781375 PASS; Process lifecycle #43749781215 PASS; Process inventory artifact #43749781176 PASS.
- Exact-parent XY-C1 firmware run #43749781299 was IN_PROGRESS at Build XY-C1 at last poll. TRX verification and artifact upload are pending; no L3 claim for this source.
- Earlier XY-C1 #43748599786 completed Build/TRX/artifact for the earlier IPTV source tree only; do not reuse it for these later WAN source changes.
- Other exact-parent Actions: WAN teardown #43749781180, WAN route #43749781190, DNSMasq #43749781280, AdGuardHome #43749781197, AliDDNS #43749781279, AliyunDrive #43749781297, HTTPD #43749781268, NVRAM #43749781237, USB modem identity #43749781376, WAN ARP probe #43749781212, detect_internet #43749781302, VPN server #43749781304 and infosvr #43749781328 all PASS.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no physical-router/L4 evidence is claimed.

## Exact first action for the next GPT

1. Poll this follow-up's exact WAN callback, AI docs and Process checks; inspect any failure log.
2. Poll the matching XY-C1 run through Build, TRX verification and artifact upload. Claim L3 only if all three pass for the exact product-source SHA.
3. Continue auditing the remaining WAN startup callers without changing PPPoE asynchronous startup or PPTP/L2TP wait semantics.
4. Keep the FTP `fstatat()`/`unlinkat()` replacement race blocked until a no-data-loss Linux 4.4-compatible solution is proven.

