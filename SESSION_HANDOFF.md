# Cross-session execution mandate

## Execution policy
Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Product-source baseline before this checker-contract correction: `68571af7f68f820569400543e80ee7f9e7a3f148` (WAN PPP recovery now uses checked `wanN_ifname_t` publication before firewall restoration).
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- On exact main SHA `68571af7f68f820569400543e80ee7f9e7a3f148`, the WAN checked-NVRAM policy binary passed in run `44674653374` step 7; `check_wan_ddns_status.py` passed step 25. Process lifecycle regression `44674653449`, Process inventory `44674653556`, AI handoff docs `44674653419`, Firewall Startup Stability `44674653529`, and NVRAM transaction audit `44674653621` passed.
- Three source-contract checks failed only because they still expected the old void setter after the production code switched to the private status-returning setter: WAN route `44674653374`, step 36/job `136083492939`; HTTPD security `44674653437`, step 19/job `136083493619`; miniupnpd config `44674653555`, step 5/job `136083493459`. Exact logs show stale `set_wan_unit_value(...)` matchers, not failed product tests. The focused WAN NVRAM regression itself passed.
- XY-C1 run `44674653396` remained IN_PROGRESS at last check (Build job `136083493218`). No current-source L3 PASS is claimed. Physical-router L4 remains NOT VERIFIED.

## Active task in this correction

Update the three affected workflow source contracts to verify the real fail-closed contract: `set_wan_unit_value_status(unit, "ifname_t", wan_ifname)` must be checked, `return nvram_status` must occur on failure, and `start_firewall_ex()` must be reached only after publication succeeds. Do not weaken or remove the checks. The legacy public void setter and unrelated callers remain unchanged.

## Verification boundary

- The production WAN fix on `68571af7f68f820569400543e80ee7f9e7a3f148` adds a private checked NVRAM writer that clears/captures errno, normalizes NVRAM `0/-1` outcomes (fallback EIO), rejects truncation, and prevents firewall rebuild on interface-publication failure. The outer recovery path attempts to keep the physical WAN interface down.
- The WAN NVRAM policy binary and DDNS semantic checker passed on `68571af7f68f820569400543e80ee7f9e7a3f148`; three other consumers had stale assertions and were red as enumerated above.
- THIS checker correction needs exact-head WAN route, HTTPD security, miniupnpd config, Process and AI docs reruns. XY-C1 must run its production-input detector, MIPS build, TRX verification and artifact upload on the same source SHA before L3 can be claimed. No physical hardware test was performed.

## Exact first action for the next GPT

1. Fetch live `main` and inspect Actions by exact `head_sha` for THIS checker-contract correction. Require WAN route, HTTPD security, miniupnpd config, Process regression/inventory and AI handoff docs to pass; inspect any first causal log if not.
2. Confirm XY-C1's detector, Build, TRX verification and artifact upload are complete on the same production source SHA. Do not reuse an older image result.
3. Resume the next P1 NVRAM publication audit: trace `set_wan_unit_value_int()`, `set_wan_unit_param()` and adjacent direct `nvram_set*_temp()` calls only where firewall/interface/route/daemon state later depends on the value. Fix the next proven failure-sensitive callsite with an injected regression and exact source contract; do not bulk-convert telemetry setters or change legacy ABI.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
