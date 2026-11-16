# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active ref: `main`.
- Parent main for this XY-C1 path-filter repair: `f16167cf2ea3666810341f1edd65bfcafeda28dc`; write protocol is complete-tree atomic commits with non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Startup recovery is complete; do not repeat.
- Root dev log has 38 records before this entry; part-012 retains the oldest 100.
- PPP ip-up validates canonical local/peer IPCP IPv4 addresses and a contiguous netmask before interface configuration or lease publication; invalid callback values return EINVAL without side effects.
- WAN IPv6 teardown preserves the last live runtime `ifname6` through RA/autoconfiguration and route/address cleanup; DHCPv6 generation fencing must remain intact.
- Optional WAN MTU ioctl failures remain diagnostic-only and do not set DHCP or route failure state.

## Active task

The PPP ip-up strict-address policy changed `dhcp_lease_policy.h`, `ppp_wan_ipup_policy.h`, and `tests/ppp_wan_ipup_policy.c`, but the XY-C1 firmware workflow watched only `net_ppp.c`; it did not trigger a same-source-SHA build for header and regression changes. Added those three dependencies to the workflow push path filter. The workflow already watches its own file, so this update should start XY-C1 validation for the exact current product tree.

## Verification boundary

Exact-head Actions for parent `f16167cf2ea3666810341f1edd65bfcafeda28dc`:
- WAN route `43874341443`: PASS, including the optional-MTU diagnostic-only source contract.
- AI handoff docs `43874341634`: PASS.
- Process inventory/lifecycle `43874341728`: PASS.
- AliDDNS `43874341573`, AdGuardHome `43874341534`, AliyunDrive `43874341457`, infosvr `43874341851`: PASS.
- NVRAM transaction boundary `43874341561`: IN_PROGRESS at last poll.
- No XY-C1 run was triggered for the PPP policy header/test changes because they were outside the previous path filter. This commit adds the paths; verify Build, TRX verification and artifact upload on its exact SHA.
- Firmware L3 and physical-router L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Confirm XY-C1 firmware validation is triggered for this commit and inspect Build, TRX verification and artifact upload on one identical SHA.
2. Recheck NVRAM run `43874341561` for parent `f16167cf2ea3666810341f1edd65bfcafeda28dc`; proceed with independent work if it remains pending.
3. Continue concrete WAN/PPP reliability or security work only; preserve PPP/DHCPv6 generation and session-lock fencing.
4. Claim firmware L3 only after Build, TRX and artifact upload all pass on the same SHA. Physical-router L4 remains unverified.
