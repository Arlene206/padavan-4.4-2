# 2026-11-18 continuation checkpoint — repair stale LAN DHCP workflow assertions

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS workflow-contract correction: `355a5c2565b2e6d001a721289e1e690fda997931`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Exact `355a5c25` Actions: WAN route `44207255511` PASS; AliDDNS `44207255497` PASS; AliyunDrive `44207257596` PASS; Process inventory/fail-closed `44207255376` PASS; Process inventory `44207255263` PASS; infosvr `44207255297` PASS; AdGuardHome `44207255252` PASS; Wireless `44207255217` PASS; detect_link `44207255099` PASS; DNSMasq `44207255096` PASS; detect_internet `44207255062` PASS; NVRAM audit `44207255182` PASS.
- Exact `355a5c25` WAN DHCP `44207255220` FAILed only in its final embedded source-contract assertion: it still expected the previous test function name `test_successful_lease_commit_ignores_ancillary_hosts_failure`, while the actual compiled policy test was renamed `test_successful_lease_commit_ignores_ancillary_refresh_failure`. Earlier GNU89/ASan/UBSan and LAN DHCP source checker steps passed.
- Exact `355a5c25` HTTPD Security `44207255662` FAILed at an embedded assertion expecting a literal `lan_dhcp_bound_callback_status(0, EIO) == 0`; the actual policy regression uses a table of `EIO`, `ENOSPC`, `ETIMEDOUT`, `ECHILD` values and loops over them. THIS corrects the stale check to assert the real multi-error loop.
- Exact `355a5c25` AI handoff `44207255510` FAILed because the handoff lacked the checker-required literal phrase `one atomic main commit`; THIS restores that exact governance phrase.
- XY-C1 `44207255537` is IN_PROGRESS at Build XY-C1; Verify TRX output and artifact upload are still pending. No product-source L3 claim for this SHA yet.
- The production LAN callback now captures and logs IAPP restart and networkmap refresh failures after lease state is committed, while keeping callback status success so BusyBox does not retain a stale internal lease. No production C changed in THIS correction.
- Roadmap dashboard remains intact; it is about 55 KiB and retains `## Whole-repository module status` / `AUDITING`.

## Active task
- THIS commit fixes two stale embedded assertion assumptions and the AI handoff wording. It does not modify production code:
  - WAN DHCP workflow now references the actual `ancillary_refresh_failure` test function.
  - HTTPD workflow validates the current array/loop that tests multiple ancillary failure codes rather than requiring one literal EIO call.
  - Handoff includes the required atomic-main-commit wording.
- The underlying 355a source change is unchanged.

## Verification boundary
- Exact parent `355a5c25`: WAN route PASS; AliDDNS PASS; AliyunDrive PASS; Process, infosvr, AdGuardHome, Wireless, detect_link, DNSMasq, detect_internet and NVRAM PASS.
- WAN DHCP FAIL was an obsolete C test-name assertion after the C regression step had passed.
- HTTPD Security FAIL was an obsolete literal-argument assertion; the current policy test exercises the same and additional failure cases through an array.
- AI FAIL was missing exact wording, not a source/build defect.
- XY-C1 `44207255537` remains in Build; TRX/artifact have not run. No L3 yet for this exact source SHA.
- Physical-router L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head WAN DHCP, HTTPD Security and AI handoff runs for THIS correction; require all three PASS, including GNU89/ASan/UBSan tests and source contracts.
2. Inspect XY-C1 `44207255537` and the new exact-head run: require Build XY-C1, Verify TRX output and artifact upload all PASS on one SHA containing the 355a LAN ancillary-status source change.
3. Continue another independent P1 userland reliability/security defect while any Actions remain pending; do not repeat the initial repo audit.
4. Keep frozen backup unchanged and do not claim physical-router L4.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect live main, exact-SHA Actions, current handoff/roadmap, validation boundary and the first actionable task before ending. If those facts and the first action remain accurate, do not create a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical multi-file change must create all blobs, one tree and **one atomic main commit**, parented to freshly fetched main and published with a non-forced expected-SHA compare-and-swap. Never force-push, create dev branches/PRs, or modify the frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first action.
