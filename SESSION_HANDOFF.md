# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at start of this compile-safety follow-up: `b1b8991aaba12684542e90929daad429f6e57d9f`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Current WAN production source covers direct IPoE static-address error fail-closed handling, checked initial physical interface bring-up, AP-client WISP old-interface cleanup and abort-before-IPTV-client behavior, plus separate critical MAN gateway host/default route status for static MAN PPTP/L2TP. Optional configured static/DHCP/DNS routes remain aggregate-only; DHCP/ZeroConf callbacks keep aggregate return semantics; PPPoE async/no-wait is unchanged.
- The required MAN status output helper is defined below `launch_wanx()`, but `launch_wanx()` calls it first. Added a file-scope forward declaration using unnamed parameter types so GNU89 compilation has the correct static prototype.
- Exact parent b1b899 WAN callback #43756151245 PASS; WAN route status #43756151031 PASS; AI docs #43756151101 PASS; Process inventory #43756151205 PASS; HTTPD security #43756151156 PASS; infosvr #43756151158 PASS; AliyunDrive #43756151160 PASS; AdGuardHome #43756151126 PASS; DNSMasq #43756151038 PASS; WAN teardown #43756151129 PASS; WAN ARP #43756151066 PASS; miniupnpd #43756151110 PASS; VPN #43756151125 PASS; USB identity #43756151191 PASS; detect_link #43756151181 PASS; detect_internet #43756151047 PASS. Process lifecycle #43756151162, NVRAM #43756151222 and AliDDNS #43756151138 were still running at last poll.
- Exact XY-C1 #43756151170 for b1b899 was still in Build XY-C1; TRX verification and artifact upload pending. This prototype source change needs a new exact-head build; no current-source L3 claim.
- Frozen backup, `docs/dev-log/INDEX.md` and archived shards remain untouched. No physical-router/L4 result.

## Active task

### Validate the static MAN helper declaration

A forward declaration now precedes the earlier `launch_wanx()` call to `man_up_internal()`. This removes implicit-function/static redeclaration risk in GNU89. The WAN callback workflow asserts the declaration exists; the route and WAN source contracts already pass on parent b1b899.

## Verification boundary

- Parent main: `b1b8991aaba12684542e90929daad429f6e57d9f`; source change: `trunk/user/rc/src/net_wan.c`; regression contract: `.github/workflows/wan-dhcp-callback-stability.yml`.
- Exact parent b1b899 WAN callback #43756151245 and WAN route #43756151031 PASS.
- Parent b1b899 exact module Actions listed above were PASS; Process lifecycle #43756151162, NVRAM #43756151222 and AliDDNS #43756151138 were still running at last check.
- Exact parent XY-C1 #43756151170 was in Build XY-C1; TRX/artifact pending. New source SHA requires its own build.
- Backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no current-source L3 or physical-router/L4 claim.

## Exact first action for the next GPT

1. Fetch current main and inspect exact-head WAN callback, WAN route, Process, NVRAM, AliDDNS and XY-C1 workflows; if build compilation fails, inspect the full RC compiler log.
2. Require latest-source Build XY-C1, TRX verification and artifact upload before L3 claim.
3. After the declaration is verified, move to another module's stability/security ledger rather than extending the WAN audit without a concrete defect.
4. Preserve PPPoE async/no-wait, independent IPv6, the frozen backup and archived log index/shards.
