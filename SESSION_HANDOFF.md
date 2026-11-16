# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify main/source baseline, active task, exact-head Actions, validation boundary, blockers, frozen backup and first next action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action are already accurate, do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task. Continue one continuous engineering effort across GPT conversations: recover once, avoid repeating completed audits, do not wait on pending CI when independent work is available, and do not ask the user what to do next.

Every logical multi-file change must be one complete Git tree and one atomic main commit, parented to freshly fetched main and pushed with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical change into partial commits, create a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. Parent for this LAN IPv6 reload slice: `7c46a6055c60c2fc534610cedd9189cf30ace4d6`.
- Exact `7c46a60` Actions: Process regression `43979698102`, Firewall Startup `43979698600`, AI handoff `43979698471` and HTTPD security `43979698034` PASS. WAN route `43979698355` was still in progress. XY-C1 `43979698609` had passed host/source checks and was building; TRX verification and artifact upload remained pending. Current-source L3 is NOT VERIFIED until exact Build/TRX/artifact steps all pass.
- LAN IPv6 teardown commit `7c46a6055c60c2fc534610cedd9189cf30ace4d6` propagates the `clear_if_addr6(IFNAME_BR)` result through `clear_lan_addr6()` and `stop_lan()`; Process, Firewall and AI checks passed, WAN and XY-C1 remained pending.
- This slice makes `reload_lan_addr6()` return clear/add failures, gates address publication on successful clear, and replaces shell-interpolated `doSystem()` with argv-based `eval()`. Both `start_lan()` and `full_restart_ipv6()` propagate failures while continuing independent work. Added injectable GNU89/ASan/UBSan tests and Process/Firewall/WAN/XY source/path contracts.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Physical-router L4 remains **NOT VERIFIED**.

## Active task

Validate the LAN IPv6 address reload result-propagation fix in this commit. `reload_lan_addr6()` returns stale-clear/address-add errors; addition is gated on successful clear; the NVRAM value is one argv element passed through `eval()`; `start_lan()` and `full_restart_ipv6()` aggregate the result while continuing independent work. Inspect exact-SHA Process, Firewall, WAN route, AI docs and XY-C1 Actions. XY-C1 Build/TRX/artifact remains the product-source L3 gate.

## Verification boundary

- Exact `7c46a6055c60c2fc534610cedd9189cf30ace4d6` Process regression `43979698102`, Firewall Startup `43979698600`, AI handoff `43979698471` and HTTPD security `43979698034` PASS.
- Exact `7c46a6055c60c2fc534610cedd9189cf30ace4d6` WAN route `43979698355` was in progress; XY-C1 `43979698609` was building after host/source regressions, with TRX/artifact pending. No L3 claim until all three pass on one SHA.
- This reload slice requires its own exact-SHA verification. Physical-router L4 remains **NOT VERIFIED**.

## Exact first action for the next GPT

1. Fetch live `main`; inspect this commit's exact-head Process, Firewall, WAN route, AI docs and XY-C1 runs. Diagnose first causal failures without waiting on unrelated pending runs.
2. Confirm XY-C1 Build, TRX verification and artifact upload on one exact SHA. Current-source L3 remains unverified until then; physical-router L4 remains NOT VERIFIED.
3. Continue the IPv6 security audit in `net_wan6.c`, especially shell-formatted `doSystem("ip -6 addr add ...")` call sites and input provenance. Preserve frozen backup and use expected-SHA non-forced main updates.
