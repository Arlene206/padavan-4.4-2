# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create every source/test/workflow/doc blob, one complete Git tree and one commit parented to freshly fetched main; update main with that exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Keep the frozen backup and archived log shards/index unchanged. Before context exhaustion, finish a safe slice and leave this file accurate.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session must record exact main/backup SHAs, source changes, exact-head Action IDs/statuses, validation boundary and the first next action before ending. Do not create a no-op commit when the checkpoint is already accurate.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Main at start of AP-client WISP cleanup slice: `7ebeaa9e02e6249ac787497429defab2f726e5a0`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only.
- Product source `20f1bbd6eb138ec87b451cd2d7f424337db798bb` adds direct IPoE static IPv4 fail-closed handling: capture `ifconfig()` result, set `wan_err`, log the status, attempt partial-address clearing and prevent `wan_up()`/UPnP via the existing error gate; IPv6 remains independent.
- Product source `7ebeaa9e02e6249ac787497429defab2f726e5a0` replaces the unchecked shell-based initial WAN bring-up with checked `set_interface_mtu()` plus direct `ifconfig()`. MTU error is diagnostic/nonfatal; failed physical interface bring-up attempts to leave the interface down and skips the WAN unit before ApCli reconnect/protocol launch.
- Exact 7ebe Actions: WAN callback #43754281208 PASS; AI docs #43754281251 PASS; Process inventory #43754281242 PASS; WAN route status #43754281323 PASS; DNSMasq #43754281246 PASS; USB modem identity #43754281329 PASS; detect_link #43754281306 PASS; detect_internet #43754281305 PASS; WAN interface teardown #43754281294 PASS; AliDDNS #43754281292 PASS; infosvr #43754281282 PASS; VPN lifecycle #43754281279 PASS; WAN ARP probe #43754281264 PASS; AdGuardHome #43754281227 PASS; AliyunDrive #43754281226 PASS; HTTPD #43754281200 PASS; Process lifecycle #43754281205 PASS. NVRAM #43754281347 was still auditing raw ioctl paths at last poll.
- Exact XY-C1 #43754281438 for 7ebe was in Build XY-C1 at last poll; TRX verification and artifact upload pending. Earlier source SHA `524601f` has a successful XY-C1 Build/TRX/artifact run, but cannot be reused for the later C changes.
- New confirmed status gap in this slice: `config_apcli_wisp()` ignored failure clearing the previous MAN interface address before switching the active WAN to WISP. The current slice logs the clear status, attempts to shut down the previous interface on failure (or checks/retries the existing single-MAC shutdown), and refuses to publish the WISP interface if the old interface cannot be ensured down.
- Frozen backup, `docs/dev-log/INDEX.md` and archived shards remain untouched. No physical-router/L4 result is claimed.

## Active task

### Verify AP-client WISP migration cleanup is fail-safe

`config_apcli_wisp()` now checks clearing the old MAN interface address. For non-single-MAC platforms, address-clear failure triggers a down-only fallback; if that fails, the function returns before changing the active WAN interface name. On single-MAC platforms, the pre-existing down operation is checked and, if it fails, retried without the address argument; a second failure refuses the WISP migration. The successful path preserves the original behavior.

The workflow checks the status capture, diagnostic, both hardware-specific fallback paths and that the WISP interface name is published only after the cleanup gate.

## Verification boundary

- Parent main: `7ebeaa9e02e6249ac787497429defab2f726e5a0`; source change: `trunk/user/rc/src/net_wan.c`; regression contract: `.github/workflows/wan-dhcp-callback-stability.yml`.
- Exact 7ebe host source contract #43754281208 PASS; exact AI docs #43754281251 PASS; Process inventory #43754281242 PASS; WAN route #43754281323 PASS; other independent Actions listed above PASS.
- Exact XY-C1 #43754281438 for source 7ebe was still building; TRX/artifact pending. Newest AP-client WISP source slice requires its own exact-head current-source Action results.
- NVRAM audit #43754281347 was in progress at the last poll.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; no L4 claim.

## Exact first action for the next GPT

1. Fetch main and inspect exact-head WAN callback, AI docs, Process inventory, NVRAM and XY-C1 results for the WISP cleanup change. Inspect complete logs if a contract fails.
2. Require latest-source WAN host tests and XY-C1 Build/TRX/artifact success before claiming L3.
3. Reassess ignored status returns in WAN startup and classify static MAN route failures at `man_up_internal()` into critical gateway/default-route errors versus optional user-configured routes before any fatal propagation.
4. Preserve the direct IPoE static address fail-closed change, checked initial physical-interface bring-up, PPPoE async/no-wait and independent IPv6 behavior.
5. Preserve frozen backup and archived log index/shards and the mandatory checkpoint phrase.
