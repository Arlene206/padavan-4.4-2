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
  4. Current slice: `stop_wan_locked()` intentionally permits an empty optional `man_ifname` but still calls `wan6_down()`. The empty name had reached per-device sysctl writes and returned `EINVAL` before the selected, saved live IPv6 interface was cleaned. Those writes are now skipped for empty caller names; native selection returns no fallback for empty/NULL input when no saved live owner exists. Regression covers both the empty-input and saved-owner cases.
- Focused tests: `trunk/user/rc/src/tests/wan6_identity_policy.c`, `wan6_sixrd_option_policy.c` and `wan6_teardown_policy.c`. The WAN DHCP callback workflow now compiles/runs the teardown policy test and pins the guarded sysctl-write source contract.
- Local focused validation: the nine-case `wan6_teardown_policy.c` binary compiled and passed under GNU89/`-Werror`/ASan/UBSan with leak detection. No local full firmware tree/build or hardware environment is available.

## Verification boundary

- Exact source SHA `89b80f1505508bb8ae96466cfe5d0c7587a6be67`: 10 of 11 push-triggered workflows concluded PASS, including 6rd #45785983604, AI #45785983636, DHCPv6 IA #45785983610, WAN DHCP #45785983731, IPv6 service type #45785983716, WAN DNS #45785983910, Process #45785983770, HTTPD #45785983585, WAN route #45785983765 and NVRAM #45785983758. XY-C1 #45785983682 was still in progress at the last poll; same-SHA Build/TRX/artifact conclusions were not yet verified.
- On the preceding source SHA `032e71f1eee03ed2f3d4d6683c2485872bea16e1`, WAN DHCP, WAN route and AI handoff all passed; XY-C1 Build was still running at its last poll.
- Product L3 baseline remains `dd2885fc9941d3a57fd9c81cf01ed86488b0183d` with same-SHA Build XY-C1, TRX verification and artifact upload PASS (artifact #15098382009). No newer L3 claim until all three pass on one product-source SHA. Physical router L4 NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch the new live `main` SHA, then inspect exact-head WAN DHCP callback and WAN route runs for this teardown fix; confirm the new empty-interface regression and inline source contract passed.
2. Poll XY-C1 #45785983682 and inspect same-SHA Build XY-C1, TRX verification and artifact-upload steps. Do not count skipped or pending steps as firmware verification.
3. Continue the remaining BusyBox DHCPv6 IA/callback live-transition audit in `trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcp6c.c` and `dhcp6c_ia.c`; only change behavior on a confirmed defect with a focused regression.

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
