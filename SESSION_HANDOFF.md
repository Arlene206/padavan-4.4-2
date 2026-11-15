# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at start of physical-interface bring-up slice: `20f1bbd6eb138ec87b451cd2d7f424337db798bb`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Product source `20f1bbd6eb138ec87b451cd2d7f424337db798bb` adds direct IPoE static IPv4 fail-closed handling: capture `ifconfig()` result, set `wan_err`, log the status, attempt partial-address clearing and prevent `wan_up()`/UPnP via the existing error gate; IPv6 remains independent.
- Exact parent-20f1bbd source checks: WAN callback #43753689812 PASS, AI docs #43753689582 PASS, Process inventory #43753689669 PASS, WAN route status #43753689651 PASS. DNSMasq lifecycle #43753689763, AliDDNS #43753689742, WAN interface teardown #43753689732, AdGuardHome #43753689712, AliyunDrive #43753689699, HTTPD #43753689687, detect_link #43753689643, miniupnpd config #43753689638, WAN ARP probe #43753689573, infosvr #43753689570, Process lifecycle #43753689558, VPN lifecycle #43753689546 and detect_internet #43753689531 all PASS.
- Exact parent-20f XY-C1 #43753689746 was in Build XY-C1 at last poll; TRX verification and artifact upload remained pending. NVRAM audit #43753689581 was still running.
- New issue in this slice: initial WAN bring-up used unchecked `doSystem("ifconfig ... mtu 1500 up 0.0.0.0")`. For PPPoE without MAN preseed this was the only physical-interface setup before pppd launch, so a failed command could be silently followed by PPP startup.
- The current slice replaces that shell command with checked `set_interface_mtu()` plus direct `ifconfig()`; MTU failure is diagnostic/nonfatal, but failed interface bring-up now attempts to keep the interface down and skips the WAN unit before ApCli reconnect or protocol launch.
- Frozen backup, `docs/dev-log/INDEX.md` and archived shards remain untouched. No physical-router/L4 result is claimed.

## Active task

### Verify physical interface bring-up fail-closed behavior

`start_wan()` now performs the initial MTU operation through `set_interface_mtu()` and brings the interface up / clears its previous IPv4 address through the checked `ifconfig()` helper instead of an unchecked shell command. A MTU failure is logged but is not fatal because the interface can still function with its current MTU. A failed physical interface setup is fatal for that WAN unit: log it, attempt to keep the interface down, and `continue` before ApCli reconnect or any WAN protocol startup.

The workflow asserts both statuses, the cleanup attempt and abort ordering. The direct IPoE static address status/cleanup fix from parent `20f1bbd` remains in the same source tree.

## Verification boundary

- Parent main: `20f1bbd6eb138ec87b451cd2d7f424337db798bb`; current implementation modifies `trunk/user/rc/src/net_wan.c` and its WAN source-contract workflow.
- Exact parent 20f1bbd: WAN callback #43753689812 PASS; AI docs #43753689582 PASS; Process inventory #43753689669 PASS; WAN route status #43753689651 PASS. Several independent module workflows listed above also PASS.
- Exact parent 20f1bbd XY-C1 #43753689746 was at Build XY-C1; TRX/artifact pending. NVRAM #43753689581 was still running.
- Exact current-head Actions for this slice are pending and a new XY-C1 run is expected. No L3 for the new physical-interface bring-up code until Build, TRX verification and artifact upload all PASS.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no L4 claim.

## Exact first action for the next GPT

1. Fetch main and inspect exact-head WAN callback, AI docs, Process inventory, NVRAM and XY-C1 Actions for the checked physical-interface startup change. Read the complete failing job log if any source contract fails.
2. Require current-source WAN host compile/run/source contracts and XY-C1 Build/TRX/artifact to pass before claiming L3.
3. Reassess remaining ignored status returns in WAN startup, including `config_apcli_wisp()`, without widening behavior to unrelated USB/PPPoE paths.
4. Continue route error classification at `man_up_internal()`: distinguish critical default/gateway routes from optional configured static/DNS routes before any fatal propagation.
5. Preserve PPPoE async/no-wait, independent IPv6, frozen backup and archived log shards/index; retain the mandatory checkpoint phrase.
