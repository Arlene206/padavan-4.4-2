# 2026-11-18 continuation checkpoint — repair WAN DHCP retry source contract

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS checker repair: `509409f8a9d72127bf40641ca779f0a43c8d5783`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- AP LAN DHCP retry source is in `0af358f3`; standalone source-checker whitespace normalization landed in `509409f8`. Exact 509 WAN DHCP workflow `44202830940` failed in the embedded callback contract at Python line 906 because it still expected `udhcpc_lan_bound()` to call `create_hosts_lan()` directly and contain that helper's failure message. Production code correctly delegates publication to `lan_dhcp_publish_hosts_and_schedule_retry()`; the extracted helper owns the hosts write, failure diagnostic, AP retry marker and DHCPD recovery notification. The standalone `scripts/check_lan_dhcp_deconfig.py` already validates this helper and retry ordering.
- THIS commit changes only the embedded workflow contract: it asserts lease NVRAM publication precedes the wrapper invocation, then checks that the wrapper itself calls `create_hosts_lan()` and emits the failure diagnostic. Production source unchanged.
- Exact 509 AI docs `44202830922`, AdGuardHome `44202830932`, Process inventory/lifecycle `44202830930`, infosvr `44202831074`, AliDDNS `44202830944`, AliyunDrive `44202830935`, and NVRAM audit `44202831128` are SUCCESS. XY-C1 `44202831040` remains IN_PROGRESS at last check; same-SHA Build/TRX/artifact outcome still pending.
- The failed WAN DHCP run passed its earlier lease-input C policy check; callback policy binary was not reached because the stale embedded assertion stopped the step.
- Static-IPoE teardown work: WAN teardown `44201908894` PASS; WAN route source-contract correction `d7eea9c0` followed by exact route run `44202284751` PASS. Earlier XY run `44201909002` was pending when last recorded. Physical-router L4 NOT VERIFIED.

## Active task
- THIS commit updates `.github/workflows/wan-dhcp-callback-stability.yml` only. The embedded contract previously looked for the hosts write and failure log inside `udhcpc_lan_bound()`; the implementation correctly extracts them into `lan_dhcp_publish_hosts_and_schedule_retry()`. The revised assertions follow the actual wrapper/helper boundary, while checking lease-state publication occurs before ancillary hosts publication.
- The separate `scripts/check_lan_dhcp_deconfig.py` also checks `lan_dhcp_publish_hosts_and_schedule_retry()`, AP-only retry marker and immediate DHCPD restart notification. No production runtime changes in THIS commit.

## Verification boundary
- Parent 509 WAN DHCP workflow `44202830940`: FAIL at stale embedded helper-location assertion after the preceding lease-input policy checks passed. Callback policy binary was not reached.
- Exact 509 AI, AdGuardHome, Process, infosvr, AliDDNS, AliyunDrive and NVRAM runs listed above: PASS.
- Exact 509 XY-C1 `44202831040`: IN_PROGRESS at last inspection. Require Build XY-C1, TRX verification and artifact upload on the same SHA before claiming L3.
- Exact-head WAN DHCP rerun and its callback policy/source contracts: PENDING.
- No local MIPS toolchain/build. Physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect the exact-head WAN DHCP callback run for THIS correction; require the standalone DHCP checker, GNU89/ASan/UBSan lease-input and callback policy binaries, and embedded source contract all PASS.
2. Inspect exact 509 XY-C1 `44202831040` completion. Check Build, TRX verification and artifact upload separately; record L3 only if all pass.
3. Continue a new independent P1 stability/security slice while Actions are pending; do not repeat the AP DHCP retry audit.
4. Keep frozen backup unchanged; no local target compile or hardware L4 claim.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect the live repository state, relevant exact-head Actions, the current handoff/roadmap facts, the validation boundary and the exact first actionable task before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit; do not create a no-op commit.

At session end, inspect live main, relevant exact-SHA Actions, the handoff/roadmap and the first actionable task. The checkpoint is mandatory; documentation-only updates are conditional. Record PASS/FAIL/PENDING accurately.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and atomically commit; do not wait idly on pending CI or ask what to do next.

Every logical multi-file change must create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
