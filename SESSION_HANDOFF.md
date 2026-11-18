# 2026-11-18 continuation checkpoint — align HTTPD AP DHCP retry contract

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS HTTPD contract correction: `816146abeff90634d307069edb91ea338b274f2f`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact parent `d7f4bc9f`: WAN DHCP callback `44203603261` PASS; Process inventory `44203603303` PASS; infosvr `44203603262` PASS; AliyunDrive `44203603304` PASS; AliDDNS `44203603381` PASS; AdGuardHome `44203603460` PASS. AI documentation action `44203603286` failed on missing exact handoff wording, then exact AI rerun `44203929395` on `62f315b1` PASSed. NVRAM `44203929348` on `62f315b1` PASSed.
- XY-C1 run `44202831040` on source SHA `509409f8` has now completed SUCCESS. It is the same production-source tree as the DHCP AP retry implementation (later changes were workflow/docs only); verify job steps for Build XY-C1, TRX and artifact upload before claiming L3.
- HTTPD Security still had three stale embedded assertions: direct `create_hosts_lan()` inside `udhcpc_lan_bound()`, error log inside that callback, and direct return from `update_hosts_ap()`. THIS updates the script to inspect the extracted helper, the callback's wrapper call, and success-only marker clear. Push filter already included callback policy files; pull_request filter is aligned too.
- No local MIPS toolchain or router; physical hardware L4 NOT VERIFIED.

## Active task
- Production AP DHCP host retry fix remains in `0af358f3`; standalone source-check whitespace fix `509409f8`; WAN integration contract fix `d7f4bc9f`.
- THIS commit changes only HTTPD's inline regression assertions/path filters and handoff files. It verifies retry marker-before-notification ordering and success-only clearing after `update_hosts_ap()`.
- No production C source changes.

## Verification boundary
- Source SHA `509409f8`: XY-C1 workflow `44202831040` completed SUCCESS; full Build/TRX/artifact steps to be confirmed from job detail before claiming L3.
- SHA `d7f4bc9f`: WAN DHCP `44203603261`, Process `44203603303`, infosvr `44203603262`, AliyunDrive `44203603304`, AliDDNS `44203603381`, AdGuardHome `44203603460` PASS.
- SHA `62f315b1`: AI handoff `44203929395` PASS and NVRAM `44203929348` PASS.
- THIS HTTPD contract change and its matching AI checkpoint will trigger exact-head HTTPD/AI verification. No hardware L4 claim.

## Exact first action for the next GPT
1. Inspect this commit's exact-head HTTPD Security and AI handoff runs; require both PASS, including DHCP source assertions for helper extraction, retry flag/notification order and success-only clear.
2. Inspect XY-C1 run `44202831040` job steps: require Build XY-C1, Verify TRX output and artifact upload on the same source SHA `509409f8`.
3. Continue the independent WAN-up notification status defect identified in `net_wan.c`, or another P1 userland issue, while new Actions are pending.
4. Keep the backup unchanged; no physical-router L4 claim.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect the live repository state, exact-SHA Actions, current handoff/roadmap facts, validation boundary and exact first actionable task before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and atomically commit; do not wait idly on pending CI or ask what to do next.

Every logical multi-file change must create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
