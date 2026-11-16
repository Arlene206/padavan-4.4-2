# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active ref: `main`.
- Parent main for this DHCP IPTV MTU declaration fix: `17df1d524e5f0ac72951381d56cf238c8699c1e9`; write protocol is complete-tree atomic commits with non-forced expected-SHA compare-and-swap.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. Startup recovery is complete; do not repeat.
- Root dev log has 39 records before this entry; part-012 retains the oldest 100.
- PPP ip-up validates canonical local/peer IPCP IPv4 addresses and a contiguous netmask before interface configuration or lease publication; invalid callback values return EINVAL without side effects.
- WAN IPv6 teardown preserves the last live runtime `ifname6` through RA/autoconfiguration and route/address cleanup; DHCPv6 generation fencing must remain intact.
- Optional WAN MTU ioctl failures remain diagnostic-only and do not set DHCP or route failure state.

## Active task

The XY-C1 build on `17df1d524e5f0ac72951381d56cf238c8699c1e9` exposed a compile regression in the optional-MTU diagnostic slice: `udhcpc_viptv_bound()` assigned `mtu_status` but its local declaration omitted the variable. Build XY-C1 failed at `net_wan.c:4344` with `'mtu_status' undeclared`; preceding RC policy/regression steps 9–16 passed, while TRX verification and artifact upload were skipped. This correction adds the missing GNU89 local declaration and asserts it in both the WAN DHCP workflow source contract and reusable modem-WAN checker.

## Verification boundary

Exact-head Actions for parent `17df1d524e5f0ac72951381d56cf238c8699c1e9`:
- XY-C1 Firmware Validation `43874973723`: FAIL in Build XY-C1 at `trunk/user/rc/src/net_wan.c:4344`, where `udhcpc_viptv_bound()` used undeclared `mtu_status`. Pre-build policy/regression steps 9–16 passed. TRX verification and artifact upload were skipped. This correction adds the missing local declaration and regression assertions.
- NVRAM transaction boundary `43874973747`: PASS.
- Process inventory/lifecycle `43874973647`, AI docs `43874973791`, AdGuardHome `43874973840`, AliDDNS `43874973707`, AliyunDrive `43874973891`, infosvr `43874973811`: PASS.
- Prior WAN route `43874341443` on `f16167cf2ea3666810341f1edd65bfcafeda28dc`: PASS, including the optional-MTU route contract.
- Firmware L3 and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Inspect exact-SHA XY-C1 Build, TRX verification and artifact upload after the IPTV `mtu_status` declaration correction; fix only the first causal failure.
2. Inspect exact-head WAN DHCP callback and route workflows; the declaration assertion must pass and optional MTU errors must remain diagnostic-only.
3. Continue the roadmap P1 RC startup-result propagation audit across modules, tracing actual callers before changing legacy void APIs.
4. Claim firmware L3 only after Build, TRX and artifact upload all pass on one identical SHA. Physical-router L4 remains unverified.
