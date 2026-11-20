# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then execute the next verified engineering task; do not repeat completed audits or spend sessions on planning alone.

- Each logical change is one complete atomic commit on live main: create all changed blobs, one complete tree based on the current tree, one commit parented to the live SHA, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Match Actions to the exact head_sha. Diagnose failures from causal logs. A pending run is not a stop condition; work on independent tasks.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, ABI/config compatibility and meaningful regressions.
- Before handoff, finish the current safe slice, verify the exact SHA and leave one precise next action.

## Current repository state

- Parent main SHA for this checker-repair slice: 21e567affba4e992476aa7dc16c30be7fa6d4500.
- The source change under validation is fix(wan): report resolver snapshot clear failures on teardown. In wan_down_internal(), the global vpnc_dns_t and conditional indexed wanN_dns snapshot clears now use errno-aware setters, aggregate first-error status, and continue independent VPN stop, route/interface cleanup and resolver refresh.
- Earlier PPP ip-up source baseline b4ab85764446ab133873f9d931d411ef25e7761e publishes the five-field volatile ipaddr/netmask/ifname_t/gateway/dns tuple through checked NVRAM writes, restores the previous tuple on partial failure, removes the new interface address and blocks WAN route setup unless publication succeeds. Its stale WAN DHCP/HTTPD source contracts were repaired by b68158c3ee405bcdc3270db0e6adfd9de91828e5; those exact-head checks passed.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only. Branch inventory contains only main and this backup.
- Legacy PR #1 is CLOSED / NOT MERGED; its AliDDNS HMAC/CA bootstrap functionality is already present and evolved on main. Do not repeat the port audit.

## Active task

Repair exact-head test contracts for the checked WAN-down DNS snapshot clear:

1. WAN route run 44686157087 and XY-C1 run 44686156823 both failed in scripts/check_wan_firewall_status.py, after host policy binaries passed. Its assertion sliced wan_dns_block up to (but not including) the resolver refresh, then demanded that refresh inside the slice. The actual C code performs the resolver refresh unconditionally after the checked DNS clear.
2. Detect-internet run 44686156914 failed in its inline source contract because it still expected the removed nvram_set_temp("vpnc_dns_t", "") instead of set_nvram_value_status().
3. This checker repair updates both assertions only; production C is unchanged. Its single atomic descendant commit is the current handoff. Fetch live main to obtain the exact resulting SHA and verify reruns; do not infer PASS from this local text inspection.

## Verification boundary at parent 21e567affba4e992476aa7dc16c30be7fa6d4500

- PASS: Process inventory/lifecycle, watchdog signal, WAN DHCP callback, DNSMasq, HTTPD, miniupnpd, ZeroTier, wireless radio, firewall startup, detect-link, WAN gateway ARP, VPN server, WAN interface teardown, USB modem and AI handoff docs.
- PASS after initial polling: NVRAM transaction boundary run 44686157053.
- FAIL from stale checker contracts, now corrected in this commit: WAN route 44686157087; XY-C1 44686156823; detect-internet 44686156914.
- XY-C1 stopped at the source-contract step before the full MIPS build/TRX/artifact gate. Product L3 is NOT VERIFIED on this source. Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main, then inspect exact-SHA Actions triggered by this checker/handoff commit: WAN route, XY-C1, detect-internet and AI handoff docs. Check NVRAM only if the pending parent run has not yet concluded.
2. If any run fails, inspect its failed step log and correct the actual cause without weakening behavioral assertions. Claim L3 only after the current production-source SHA passes the detector, MIPS build, TRX verification and artifact upload on that same SHA.
3. When the WAN resolver slice is green, resume a separate high-impact cross-userland service/process/security issue. Do not repeat the WAN DNS snapshot or PPP ip-up work. Inspect current callsites, implement a production fix plus focused GNU89/Werror regression, and keep unrelated work moving while Actions are pending.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep host/source regressions, exact-head Actions, MIPS build/TRX and physical-device evidence separate. Only a same-SHA detector, build, TRX verification and artifact upload establish L3; host tests do not. Never infer L4 from CI.
