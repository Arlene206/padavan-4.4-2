# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is already accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for this slice: eddf9aac4d44377d518d947af9b25f58685a67be.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- WAN source 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6 has direct IPoE static-address fail-closed handling, checked physical-interface bring-up, safe AP-client WISP migration, and separate required MAN gateway host/default-route status for static MAN PPTP/L2TP. Optional routes remain aggregate-only; DHCP/ZeroConf callbacks preserve aggregate status; PPPoE async/no-wait remains unchanged.
- Exact 401066 WAN callback #43756373267, WAN route #43756373287, NVRAM #43756373306, Process lifecycle #43756373317, Process inventory #43756373209, HTTPD #43756373180, AdGuardHome #43756373548, DNSMasq #43756373394, miniupnpd #43756373360, USB identity #43756373320, WAN teardown #43756373261, WAN ARP #43756373302, AliDDNS #43756373308, AliyunDrive #43756373280, VPN #43756373251, infosvr #43756373126, detect_link #43756373117, detect_internet #43756373238 and AI docs #43756373157 PASS.
- Exact XY-C1 #43756373315 Build/TRX/artifact all PASS (L3 only for source 401066).
- Transmission and Aria start wrappers were fixed in commit eddf9aac4d44377d518d947af9b25f58685a67be; exact storage workflow #43758894731 completed SUCCESS. Host shell syntax/start-status tests and the firmware BusyBox ash test both passed. NVRAM #43758894726 was running at the last poll.
- Aria fail-closed preparation checks config/session/download-directory creation, permissions/ownership and same-directory temporary default-config write/rename. The temp name includes the shell PID, preventing concurrent startup collisions; failures clean up and abort before daemon launch.
- Frozen backup, docs/dev-log/INDEX.md and archive shards remain untouched. No L4 result claimed.

## Active task

### Verify Aria default-config staging and pre-launch setup failures

Aria now aborts if it cannot prepare the config directory or session list, create the download directory, set required permissions/ownership, write its default config, or atomically publish it. The default config is first written to a PID-qualified aria2.conf.tmp.$ path in the config directory; failures remove the temporary file and return before start-stop-daemon.

The storage regression injects cat/write and mv/rename failures and asserts that start-stop-daemon is never invoked and no final or temporary config is left behind. Host and BusyBox ash runs remain wired to the storage workflow.

## Verification boundary

- Parent main: eddf9aac4d44377d518d947af9b25f58685a67be.
- Changed: trunk/user/aria2/aria.sh and trunk/user/scripts/tests/storage_app_start_status.sh; previous eddf9aac changed Transmission/Aria start-status flow and storage workflow.
- Exact eddf9aac storage-hotplug workflow #43758894731 completed SUCCESS: host syntax and launch-status regression PASS, firmware BusyBox 1.24.2/ash stage and full storage regression PASS. Exact AI docs #43758894840 and Process inventory #43758894771 PASS; NVRAM #43758894726 was still running at last poll.
- Current Aria staging change needs a new exact-head storage workflow result; a follow-up corrected the temp filename to include the PID. No firmware C changes; L3 remains established only for exact source 401066. No L4 claim.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

## Exact first action for the next GPT

1. Inspect exact-head storage-hotplug run for the Aria staging change; fetch full logs if host/BusyBox config fault injection fails.
2. Verify config-write and config-rename injection tests under host shell and firmware BusyBox ash, especially that neither calls start-stop-daemon or leaves a temp/final config.
3. Separately audit Transmission default settings generation and remaining directory/permission/ownership failures before daemon launch.
4. Preserve frozen backup, archive index/shards and L3/L4 boundary.
