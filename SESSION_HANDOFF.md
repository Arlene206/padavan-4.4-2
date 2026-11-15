# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is already accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for this slice: 6ad669a672f8274f523723df4315e523841de5d5.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- WAN source 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6 has direct IPoE static-address fail-closed handling, checked physical-interface bring-up, safe AP-client WISP migration, and separate required MAN gateway host/default-route status for static MAN PPTP/L2TP. Optional routes remain aggregate-only; DHCP/ZeroConf callbacks preserve aggregate status; PPPoE async/no-wait remains unchanged.
- Exact 401066 WAN callback #43756373267, WAN route #43756373287, NVRAM #43756373306, Process lifecycle #43756373317, Process inventory #43756373209, HTTPD #43756373180, AdGuardHome #43756373548, DNSMasq #43756373394, miniupnpd #43756373360, USB identity #43756373320, WAN teardown #43756373261, WAN ARP #43756373302, AliDDNS #43756373308, AliyunDrive #43756373280, VPN #43756373251, infosvr #43756373126, detect_link #43756373117, detect_internet #43756373238 and AI docs #43756373157 PASS.
- Exact XY-C1 #43756373315 Build/TRX/artifact all PASS (L3 only for source 401066).
- Transmission and Aria start wrappers were fixed in commit eddf9aac4d44377d518d947af9b25f58685a67be; exact storage workflow #43758894731 completed SUCCESS. Host shell syntax/start-status tests and the firmware BusyBox ash test both passed. NVRAM #43758894726 was running at the last poll.
- Aria setup and default-config write/rename fault injection passed exact storage run #43759497496 on source SHA 755e8c711f1becc9a33c369f156d7b66def904cb. A fresh live-source review found the temp expression was still `aria2.conf.tmp.# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is already accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main for this slice: 6ad669a672f8274f523723df4315e523841de5d5.
- Frozen backup: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- WAN source 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6 has direct IPoE static-address fail-closed handling, checked physical-interface bring-up, safe AP-client WISP migration, and separate required MAN gateway host/default-route status for static MAN PPTP/L2TP. Optional routes remain aggregate-only; DHCP/ZeroConf callbacks preserve aggregate status; PPPoE async/no-wait remains unchanged.
- Exact 401066 WAN callback #43756373267, WAN route #43756373287, NVRAM #43756373306, Process lifecycle #43756373317, Process inventory #43756373209, HTTPD #43756373180, AdGuardHome #43756373548, DNSMasq #43756373394, miniupnpd #43756373360, USB identity #43756373320, WAN teardown #43756373261, WAN ARP #43756373302, AliDDNS #43756373308, AliyunDrive #43756373280, VPN #43756373251, infosvr #43756373126, detect_link #43756373117, detect_internet #43756373238 and AI docs #43756373157 PASS.
- Exact XY-C1 #43756373315 Build/TRX/artifact all PASS (L3 only for source 401066).
- Transmission and Aria start wrappers were fixed in commit eddf9aac4d44377d518d947af9b25f58685a67be; exact storage workflow #43758894731 completed SUCCESS. Host shell syntax/start-status tests and the firmware BusyBox ash test both passed. NVRAM #43758894726 was running at the last poll.
 (a constant literal suffix), contrary to the previous log's claimed correction; the earlier test did not cover concurrent starts. THIS COMMIT changes it to a PID-qualified suffix and adds a barrier-controlled two-process regression that requires two distinct staged files before either publish.
- Frozen backup, docs/dev-log/INDEX.md and archive shards remain untouched. No L4 result claimed.

## Active task

### Verify Aria default-config staging and pre-launch setup failures

Aria now aborts if it cannot prepare the config directory or session list, create the download directory, set required permissions/ownership, write its default config, or atomically publish it. The default config is first written to a PID-qualified aria2.conf.tmp.$ path in the config directory; failures remove the temporary file and return before start-stop-daemon.

The storage regression injects cat/write and mv/rename failures and asserts that start-stop-daemon is never invoked and no final or temporary config is left behind. Host and BusyBox ash runs remain wired to the storage workflow.

## Verification boundary

- Parent main: 6ad669a672f8274f523723df4315e523841de5d5.
- Changed in THIS COMMIT: trunk/user/aria2/aria.sh and trunk/user/scripts/tests/storage_app_start_status.sh, plus this handoff/roadmap/log checkpoint. The storage-hotplug workflow already includes both source and test paths in push/pull filters.
- Exact 755e8c711f1becc9a33c369f156d7b66def904cb storage-hotplug run #43759497496 SUCCESS (all 24 jobs steps completed), including the existing host and firmware BusyBox ash checks; it did not include concurrent staging-name coverage. Exact 6ad docs #43759640516, Process inventory #43759640572 and the other triggered main push checks PASS.
- THIS COMMIT's new exact-head storage workflow is pending until GitHub dispatches it. No firmware C changes; L3 remains established only for exact source 401066. No L4 claim.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

## Exact first action for the next GPT

1. Inspect the exact-head storage-hotplug `push` run for THIS COMMIT; confirm the PID-suffixed concurrent-start regression passes under host shell and firmware BusyBox ash, and inspect logs if either fails.
2. If storage CI passes, audit Transmission default settings generation and fail-closed pre-launch setup (mkdir/chmod/chown, generator, sed and publish); if it fails, repair that causal regression first.
3. Preserve frozen backup, docs/dev-log/INDEX.md and archive shards; retain the L3 boundary at source 401066 and make no L4 claim.
