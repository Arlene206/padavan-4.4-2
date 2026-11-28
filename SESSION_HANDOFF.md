## 2026-11-28 continuation checkpoint

- Repository Arlene206/padavan-4.4-2; main-only development. Parent for this DHCP router-normalization follow-up: 3d499eedc1e71050bd10d3c51db1aa11c2a72a51.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807 and is immutable. PR #1 remains closed/unmerged. Startup/protocol recovery completed once; do not repeat.
- AliyunDrive PID generation source 19d51bab853ed2b904e44333e3f6ce0bd72059a4 has L2 PASS and XY-C1 #46135763583 L3 PASS (Build, TRX, artifact upload). Descendant commits only affect checker/docs. Physical L4 not tested; final procfs-read-to-kill(pid) race remains documented.
- NTP bounded-input source correction c052506cbe9463f2719494d237e5323fe1c261c8: Watchdog NTP #46137016886, AI docs #46137016917, Process #46137016902, NVRAM #46137016925 all PASS; XY-C1 #46137016928 was pending (Build in progress, TRX/artifact pending at last poll). Poll it; no L3 claim yet.
- DHCP helper memory-safety guard landed in 3d499eedc1e71050bd10d3c51db1aa11c2a72a51: dhcp_lease_router_is_changed() returns conservatively for NULL/zero-capacity output storage instead of writing next_gateway[0] after parser failure. Its exact-SHA Actions were pending at last poll.
- Follow-up defect: malformed Router options were treated as normal empty/withdrawn gateway by the same helper. When no gateway was stored, invalid option could be deemed unchanged, skipping full bound validation. THIS follow-up returns changed on parse error, forcing full validation; valid empty/absent Router option remains a legitimate withdrawal.
- REFACTOR_ROADMAP.md remains near its hard size cap; avoid status-only edits.

## Current repository state

- Latest parent: 3d499eedc1e71050bd10d3c51db1aa11c2a72a51.
- DHCP source/test/checker: trunk/user/rc/src/dhcp_lease_policy.h, trunk/user/rc/src/tests/dhcp_lease_policy.c, scripts/check_lan_dhcp_deconfig.py.
- WAN DHCP workflow path filters include these paths and run GNU89/Werror/ASan/UBSan tests.
- No local checkout/host run is available; L2 is determined by exact-SHA Actions.

## Active task

- Make malformed Router option parsing fail closed in the DHCP renewal optimization. dhcp_lease_ipv4_list_first() returns 0 for absent/empty withdrawal, but negative for malformed data.
- Return 1 on parse error so udhcpc_renew() must run full udhcpc_bound() validation. Do not conflate malformed input with valid withdrawal.
- Regression covers malformed Router with an existing gateway and with no stored gateway; both must be considered changed. Previous zero-capacity/NULL output guards remain.

## Exact first action for the next GPT

1. Fetch live main and confirm this follow-up SHA.
2. Inspect exact-SHA WAN DHCP callback stability, AI docs, Process, NVRAM and XY-C1 runs. Parent run IDs #46137804678 WAN DHCP, #46137804690 NVRAM, #46137804692 AI docs, #46137804688 Process, and #46137804717 XY-C1 were pending; these apply only to 3d499...
3. Poll NTP #46137016928 on c052...; L3 only if Build, TRX and artifact upload pass.
4. Repair only actual failure steps from logs; PENDING permits independent work.
5. New DHCP source SHA requires its own Build/TRX/artifact verification for L3. Physical L4 remains NOT VERIFIED.

## Verification boundary

- L1: malformed Router input could be collapsed to empty and considered unchanged when no prior gateway existed. Parse errors now force the full bound path; legitimate absent/empty options remain withdrawals.
- L2: pending exact-head WAN DHCP ASan/UBSan regression and source-contract Actions; no local compile.
- L3: Aliyun source L3 PASS (19d51...); NTP L3 pending (c052...); current DHCP source requires its own exact-SHA Build/TRX/artifact pass.
- L4: NOT VERIFIED; no physical-router test.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Before ending, inspect live main, exact-head Actions, active task, validation boundary and first next action. Update only when facts materially change; do not create a no-op commit.

# Cross-session execution mandate

- One logical multi-file task, one atomic main commit: create all blobs, one tree and one commit, then update main with a non-forced expected-SHA compare-and-swap. Never force-push or publish per-file intermediate commits.
- Before context exhaustion, finish the current safe slice, publish it, inspect exact-head Actions and leave the next GPT a precise first action and truthful L1-L4 validation boundary.
- Recover once, then execute; do not repeat completed audits or ask what to do next.
- PASS permits the next issue, FAIL requires causal repair, PENDING permits independent work.
