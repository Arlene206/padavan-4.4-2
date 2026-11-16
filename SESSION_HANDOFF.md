# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Source commit under test: `176e93e3e20889bdddcdab8769f3a4e9665ef01e`.
- LAN IPv6 reload behavior is implemented: stale address flush gates add; clear/add statuses are returned and propagated to LAN startup / IPv6 restart; command invocation uses argv-based `eval()`. The new GNU89/ASan/UBSan regression passed on the exact source SHA.
- Exact `176e93e` Actions: WAN route `43982636461` PASS, including the new LAN IPv6 reload test and inline source contracts; AI handoff `43982635895` PASS. Process `43982636314`, Firewall Startup `43982636272`, and XY-C1 `43982636425` failed at the shared source checker assertion for compacted whitespace. Process log confirms the new C regression itself PASS before checker failure; Firewall and XY stopped at the same checker. XY firmware Build/TRX/artifact were skipped, so product-source L3 remains NOT VERIFIED.
- The checker fix in this commit changes only assertions to match `compact()` semantics (whitespace is collapsed to single spaces). All production behavior and C regression remain unchanged. Exact-SHA Process/Firewall/XY verification must be rerun.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Verify the checker whitespace-normalization correction in this commit. Then confirm Process, Firewall Startup, WAN route, AI handoff and XY-C1 on the exact head SHA. The new LAN IPv6 C policy test already passed in WAN and in the Process log before the checker assertion failed; XY-C1 product build remains blocked until the shared checker passes.

## Verification boundary

- Exact `176e93e3e20889bdddcdab8769f3a4e9665ef01e`: WAN route `43982636461` PASS; AI handoff `43982635895` PASS.
- Process `43982636314`, Firewall `43982636272`, and XY-C1 `43982636425` FAIL at one false-negative checker assertion: `compact()` normalizes whitespace to spaces, while the new assertions incorrectly expected whitespace removal. New C test itself PASS; WAN ran it and all inline assertions PASS.
- XY-C1 Build/TRX/artifact were skipped because the checker failed before build. Product-source L3 NOT VERIFIED; physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head Actions for the checker-fix commit. Confirm Process, Firewall, WAN route, AI handoff and XY-C1 all pass; inspect logs for any new assertion failure.
2. Confirm XY-C1 Build, TRX verification and artifact upload on one SHA. Do not claim L3 before all product steps pass.
3. Continue IPv6 audit in `net_wan6.c`, checking remaining dynamic command arguments and whether validated address/interface inputs are passed safely. Preserve backup immutability and use atomic expected-SHA updates.
