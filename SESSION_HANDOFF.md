# 2026-11-18 continuation checkpoint — report deferred VPN stop enqueue failures

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS WAN teardown slice: `fda7c018f1cb3ca4d0588f1f3e30baaba13f28c0`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- AP DHCP retry production source is committed in `0af358f3`; standalone checker correction `509409f8`; WAN DHCP integration assertion fix `d7f4bc9f`; HTTPD contract fix `4a9b812b`.
- Exact `4a9b812b` HTTPD Security `44204487769` PASS; AI handoff `44204487682` PASS; Process inventory `44204487756` PASS; infosvr `44204487704` PASS; AdGuardHome `44204487714` PASS; AliyunDrive `44204487708` PASS; AliDDNS `44204487832` PASS. NVRAM `44204487741` and XY-C1 `44204488447` were IN_PROGRESS at last poll.
- Exact `d7f4bc9f` WAN DHCP callback `44203603261` PASS.
- The prior WAN start-side deferred VPN enqueue gap is already fixed in commit `816146ab`: `notify_rc_status("start_vpn_client")` is aggregated into WAN-up status. Found the symmetric teardown gap: `wan_down_internal()` used fire-and-forget `notify_rc("stop_vpn_client")`, discarded enqueue failure, and could return success while VPN-client shutdown was never queued. THIS slice uses `notify_rc_status`, logs and aggregates the error while continuing independent route/interface cleanup.
- XY-C1 run `44202831040` on source SHA `509409f8` completed SUCCESS; Build XY-C1, Verify TRX output and artifact upload steps all reported SUCCESS. Later changes are workflow/docs or this WAN teardown source change; this WAN change needs its own exact-head firmware build before L3 can be claimed.
- Physical-router L4 NOT VERIFIED.

## Active task
- Added focused route-status regression that simulates failed VPN-stop queueing followed by successful teardown operations and asserts the original failure remains visible while all cleanup steps execute.
- Extended `.github/workflows/wan-route-status.yml` to assert the production ordering, status aggregation, log and absence of the old unchecked `notify_rc()` call.
- THIS slice changes WAN teardown production source, one C regression file, the WAN route workflow and handoff docs.

## Verification boundary
- Exact parent `4a9b812b`: HTTPD, AI, Process, infosvr, AdGuardHome, AliyunDrive and AliDDNS PASS; NVRAM and XY-C1 pending at last check.
- Exact `509409f8` XY-C1 run `44202831040`: completed SUCCESS for Build XY-C1, TRX verification and artifact upload; this is prior production-source SHA for the AP DHCP change, not this later WAN stop-notification patch.
- THIS WAN source slice requires exact-head WAN route regression and XY-C1 Build/TRX/artifact. No hardware L4 claim.

## Exact first action for the next GPT
1. Inspect this slice's exact-head WAN route workflow and require its GNU89/ASan/UBSan test and embedded VPN-stop source contract to PASS.
2. Inspect exact-head XY-C1; require Build, TRX validation and artifact upload on the same SHA containing this WAN stop-notification change.
3. Recheck the exact-parent `4a9b812b` pending NVRAM `44204487741` and XY-C1 `44204488447` results.
4. Continue independent P1 stability/security work while CI is pending. Frozen backup unchanged; hardware L4 NOT VERIFIED.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect the live repository state, exact-SHA Actions, current handoff/roadmap facts, validation boundary and exact first actionable task before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and atomically commit; do not wait idly on pending CI or ask what to do next.

Every logical multi-file change must create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
