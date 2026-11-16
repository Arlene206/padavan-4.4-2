# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`. Parent before THIS COMMIT: `bb0c693125f0ddfd4e845d6c9d4f8fb58f68d2e2`; current product-source route-cache commit: `493f981e4db110923ca8d16328e29aec541a3734`.
- Conntrack teardown result propagation remains L3 VERIFIED at `cffc149` (exact XY-C1 run `43973861873`, Build/TRX/artifact pass). LAN IPv4 route teardown code is in `759c18d`; route-cache API/source change is `493f981`.
- Exact 493f WAN route `43978255126` PASS; Process inventory `43978254677`, Process lifecycle `43978254742`, WAN DHCP `43978255203` and the other successful jobs are recorded in the log. The 493f Process regression `43978254976`, Firewall Startup `43978255243`, XY-C1 `43978254941` and HTTPD security `43978255197` failed only at stale route-cache source assertions: the checkers still expected the old direct per-interface flush plus separate cache call, not the new combined helper.
- AI handoff `43978255204` failed because 493f reduced root `dev_log.md` to one entry. `bb0c693` restored 22 preexisting entries plus the new 493f record; exact bb0c AI handoff `43978638161` PASS confirms 23 complete entries.
- THIS COMMIT updates the common modem-WAN checker and HTTPD security checker to verify `rc_wan_route_flush_and_cache_apply()`, the cache callback, and the error gate before switch reconstruction. Exact reruns are required; XY-C1 has not yet reached firmware Build because the common checker stopped it.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Validate the route-cache checker correction on the next exact main SHA: Process regression, Firewall Startup, HTTPD security, WAN route and XY-C1. The prior failures all originated in outdated assertions for the newly composed route/interface + global-cache helper. Resolve actual first causal failures if anything else remains.

## Verification boundary

- Exact 493f WAN route `43978255126` PASS; exact bb0c AI handoff `43978638161` PASS.
- Exact 493f Process regression `43978254976`, Firewall Startup `43978255243`, HTTPD security `43978255197` and XY-C1 `43978254941` did not pass because their old route-cache API assertions failed. XY-C1 stopped before Build/TRX/artifact; route-cache L3 is NOT VERIFIED.
- cffc149 L3 remains verified; 759c18d LAN route and 493f route-cache source require exact-source firmware build, TRX verification and artifact upload. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-head Process, Firewall Startup, HTTPD security, WAN route, AI docs and XY-C1 results for THIS COMMIT. The shared checker path is used by Process/Firewall/XY; HTTPD has a second inline route assertion.
2. After these pass, implement the identified LAN IPv6 teardown defect: change `clear_lan_addr6()` to return `clear_if_addr6(IFNAME_BR)`, update its prototype, capture/log/aggregate it in `stop_lan()`, keep bridge-down attempted, and add a focused GNU89/ASan/UBSan regression plus CI source/path checks.
3. Preserve exact-SHA evidence, atomic main-only commits, expected-SHA non-forced ref updates, frozen backup immutability and L4 NOT VERIFIED unless physical hardware is tested.
