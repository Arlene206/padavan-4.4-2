# Session Handoff — 2026-11-26

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development remains on `main` only.
- Parent `main` for the current 6rd relay validation slice: `763015839432f2fb6e4e3673c9c4745c2186bf46`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify/delete. PR #1 remains closed and unmerged.
- Bootstrap/protocol recovery was already performed once. Do not repeat it or re-audit completed work.

## Active task

- P1 WAN IPv6 strict-input, teardown and route-ownership hardening is advancing in four connected slices:
  1. SIT route ownership records reject interface names of 16+ bytes; a 15-byte name round-trips.
  2. `wan6_up()` rejects ambiguous `inet_aton()` gateway aliases during preflight, before service/route mutation; empty and canonical `0.0.0.0` preserve no-gateway semantics.
  3. DHCP 6rd relay-list parsing previously used `is_valid_ipv4()` (based on `inet_aton()`) although later SIT setup requires strict `inet_pton()` literals. Production now validates with `format_ipv4_literal()`; abbreviated/hex aliases are rejected while valid multi-relay lists remain accepted.
  4. `stop_wan_locked()` intentionally permits an empty optional `man_ifname` but still calls `wan6_down()`. The empty name had reached per-device sysctl writes and returned `EINVAL` before the selected, saved live IPv6 interface was cleaned. Those writes are now skipped for empty caller names; native selection returns no fallback when no saved live owner exists. Regression covers empty/NULL input and the saved-live-owner case.
  5. Current slice: `remove_iaprefix()` previously overwrote its first downstream `ifaddrconf(IFADDRCONF_REMOVE)` failure with later failures. It now aggregates through `dhcp6c_ia_first_error()`; the regression proves three failures are all attempted while the first errno remains authoritative.
- Focused tests: `trunk/user/rc/src/tests/wan6_identity_policy.c`, `wan6_sixrd_option_policy.c` and `wan6_teardown_policy.c`. The WAN DHCP callback workflow now compiles/runs the teardown policy test and pins the guarded sysctl-write source contract.
- Local focused validation: the nine-case `wan6_teardown_policy.c` binary compiled and passed under GNU89/`-Werror`/ASan/UBSan with leak detection. No local full firmware tree/build or hardware environment is available.

## Verification boundary

- Exact source SHA `21d71c59e186de568492b0f31e4518396ed63b35`: WAN DHCP callback #45787666271 PASS (including the new empty-interface test and source contract), WAN route #45787666333 PASS, 6rd #45787666495 PASS, IPv6 service #45787666373 PASS, DHCPv6 IA #45787666291 PASS, WAN DNS #45787666335 PASS, AI #45787666397 PASS, Process #45787666517 PASS, HTTPD #45787666332 PASS. NVRAM #45787666341 and XY-C1 #45787666437 were still in progress at the last poll; same-SHA Build/TRX/artifact steps remain to be checked.
- On the preceding source SHA `032e71f1eee03ed2f3d4d6683c2485872bea16e1`, WAN DHCP, WAN route and AI handoff all passed; XY-C1 Build was still running at its last poll.
- Product L3 baseline remains `dd2885fc9941d3a57fd9c81cf01ed86488b0183d` with same-SHA Build XY-C1, TRX verification and artifact upload PASS (artifact #15098382009). No newer L3 claim until all three pass on one product-source SHA. Physical router L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head DHCPv6 IA workflow for the first-error aggregation regression; ensure the new C case and Python source contract pass.
2. Inspect XY-C1 #45787666437 and NVRAM #45787666341; for firmware, require same-SHA `Build XY-C1`, `Verify TRX output` and artifact upload all PASS.
3. Investigate the remaining shutdown boundary in `dhcp6c.c`: SIGTERM currently enters `check_exit(1)`, whose forced cleanup can discard the 5-second retry timers that `remove_iaprefix()` deliberately creates after failed delegated-prefix removal. Confirm whether the process-stop timeout permits a safe graceful-drain solution before changing this lifecycle.

## Verification boundary details

- Parent `5bec4f7`: WAN DHCP callback passed with the new IPTV NVRAM/IGMP C regressions and Python contract; its XY workflow was host-only and skipped Build/TRX/artifact. Initial AI handoff failed on missing checkpoint text, corrected; current AI checks passed on subsequent commits.
- Parent `032e71f`: WAN callback and WAN route passed; AI handoff passed after adding the complete cross-session checkpoint contract; exact XY Build was still pending at its last poll.
- Parent `7630158`: the three SIT policy changes (interface-length, strict gateway, strict 6rd relay) had not yet all landed together; its relevant test suite passed before the current 6rd relay tightening.
- No hardware/device test is available in this environment.

## Mandatory handoff checkpoint

**Mandatory handoff checkpoint:** Every session MUST compare the live repository and relevant exact-head Actions with this file before ending. Verify engineering baseline, active task, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all facts are accurate, leave this file untouched and create no no-op commit.

# Cross-session execution mandate

Continue as the same repository maintainer across conversations. Every independently reviewable source/test milestone must be published as one atomic main commit: create all changed blobs, one complete Git tree and one commit, then update `main` with `force:false` and the expected parent SHA. Never publish partial-file commits or force-push.
Before context exhaustion, finish a safe bounded slice, inspect exact-SHA Actions, and leave this handoff with the precise next task and validation boundary.

## Execution invariants

- Main only; one complete tree/commit per logical task; expected-SHA non-forced ref update; never force-push.
- Preserve frozen backup; do not reopen PR #1.
- PENDING is not a gate on independent safe work.
- Product L3 requires same-source-SHA Build XY-C1, TRX verification and artifact upload all PASS; physical L4 requires actual target-device testing.
- Update dev_log for completed engineering slices, handoff for material resumable changes, roadmap only for material module/status changes.
