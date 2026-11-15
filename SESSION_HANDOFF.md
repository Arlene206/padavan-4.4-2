# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is already accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Main at start of this storage-script slice: 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- WAN source 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6 has direct IPoE static-address fail-closed handling, checked physical-interface bring-up, safe AP-client WISP migration and separate required MAN gateway host/default-route status for static MAN PPTP/L2TP. Optional routes remain aggregate-only; DHCP/ZeroConf callbacks preserve aggregate status; PPPoE async/no-wait remains unchanged.
- Exact 401066 WAN callback #43756373267 PASS; WAN route #43756373287 PASS; NVRAM #43756373306 PASS; Process lifecycle #43756373317 PASS; Process inventory #43756373209 PASS; HTTPD #43756373180 PASS; AdGuardHome #43756373548 PASS; DNSMasq #43756373394 PASS; miniupnpd #43756373360 PASS; USB identity #43756373320 PASS; WAN teardown #43756373261 PASS; WAN ARP #43756373302 PASS; AliDDNS #43756373308 PASS; AliyunDrive #43756373280 PASS; VPN #43756373251 PASS; infosvr #43756373126 PASS; detect_link #43756373117 PASS; detect_internet #43756373238 PASS; AI docs #43756373157 PASS.
- Exact XY-C1 #43756373315 SUCCESS: Build XY-C1, TRX verification and artifact upload all PASS; L3 verified for source 401066.
- Independent storage defect: Transmission and Aria launchers used the previous command status; a failed daemon start was followed by echo (zero), while a successful daemon start was followed by logger (which could fail).
- This slice tests start-stop-daemon directly and explicitly returns success/failure regardless of logger status. Added temporary-directory shell regression with stubbed process lookup, launch and logging; host and firmware BusyBox ash execution are wired into storage workflow.
- Frozen backup, docs/dev-log/INDEX.md and archive shards remain untouched. No L4 result claimed.

## Active task

### Verify Transmission and Aria startup exit-status propagation

Both launchers preserve the actual start-stop-daemon result. Successful launch returns 0 even if logger fails; failed launch returns 1 even if logging succeeds or fails. The new storage_app_start_status.sh test stubs pidof/NVRAM/start-stop-daemon/logger/chmod/chown and tests both branches under host shell and firmware BusyBox ash.

## Verification boundary

- Parent main: 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6.
- Changed: trunk/user/transmission/transmission.sh; trunk/user/aria2/aria.sh; new trunk/user/scripts/tests/storage_app_start_status.sh; .github/workflows/storage-hotplug-stability.yml.
- Exact parent 401066 WAN callback #43756373267, WAN route #43756373287, NVRAM #43756373306, Process lifecycle #43756373317, Process inventory #43756373209, HTTPD #43756373180, AdGuardHome #43756373548, DNSMasq #43756373394, miniupnpd #43756373360, USB identity #43756373320, WAN teardown #43756373261, WAN ARP #43756373302, AliDDNS #43756373308, AliyunDrive #43756373280, VPN #43756373251, infosvr #43756373126, detect_link #43756373117, detect_internet #43756373238 and AI docs #43756373157 all PASS.
- Exact parent XY-C1 #43756373315 Build/TRX/artifact all PASS (L3 for 401066); firmware C source unchanged in this shell-only slice.
- New exact-head storage regression pending; no physical-router/L4 result.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

## Exact first action for the next GPT

1. Inspect exact-head storage workflow result and complete logs if failed.
2. Confirm regression runs under host shell and BusyBox ash.
3. Separately audit directory creation and config-write/chmod/chown errors before daemon launch.
4. Preserve exact 401066 L3 boundary, frozen backup and archive index/shards; make no L4 claim.
