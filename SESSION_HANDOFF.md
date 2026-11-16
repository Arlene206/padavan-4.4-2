# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; only active development branch is `main`.
- Main at the start of this validation repair: `acf73fd0fa217560ed670beb2d92bdd0d0a4a924`. Refetch before each write and use non-forced expected-SHA update.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only. Startup/backup recovery was completed once; do not repeat it.
- Root log rotation is complete: `part-012.md` preserves the oldest 100 records; root retains 22 recent records before this update.
- PR #1 CA/HMAC features are already integrated in main; the closed unmerged historical PR has no unique remaining change to cherry-pick.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII startup errors propagate; optional hw_nat/LED operations remain best-effort.
- WAN startup now reports physical IFUP and required IPv6-control failures; static WAN route, tunnel and WAN teardown failures are preserved.
- `launch_wanx()` now accumulates first-error status across MAN static address, complete static MAN route, gateway routes and configured physical IPv6. The current regression failures are stale source assertions still expecting the old gateway-status overwrite assignment.
- This repair only changes two workflow expectations; it does not revert or alter the correct first-error product-source implementation.

## Active task

Update `.github/workflows/wan-route-status.yml` and `.github/workflows/wan-dhcp-callback-stability.yml` to require `route_status_accumulate()` for both MAN route and gateway route status instead of the superseded `launch_status = gateway_route_status` assignment. Exact-head parent runs `43866344779` and `43866344884` failed only at the obsolete assertion.

## Verification boundary

- Parent `acf73fd0fa217560ed670beb2d92bdd0d0a4a924`: WAN route `43866344779` failed at Python source-contract assertion line148; WAN DHCP callback `43866344884` failed at assertion line704. Both were stale tests for the removed overwrite assignment; the production code correctly preserves first-error aggregation.
- AI handoff docs `43866344868`, infosvr `43866344724`, USB modem identity `43866344763`, VPN `43866344749`, miniupnpd `43866345059`, WAN gateway ARP `43866344761`, detect_internet `43866344886`, WAN teardown `43866344963`, AdGuardHome `43866344879`, Process inventory `43866344891`, DNSMasq `43866344739`, detect_link `43866344715` and AliDDNS `43866344897`: SUCCESS.
- Process `43866345006`, XY-C1 `43866344767`, NVRAM `43866344923` and HTTPD `43866344906` were IN_PROGRESS at last inspection.
- No same-SHA firmware Build/TRX/artifact L3 or hardware L4 claim is established.

## Exact first action for the next GPT

1. Inspect corrected WAN route/DHCP workflows plus Process, XY-C1, NVRAM and HTTPD on the new exact main SHA.
2. Continue PPP/IPoE required start-status propagation after the validator repair; preserve the first underlying error across recovery.
3. Continue WAN authentication and lease/callback cleanup review where it exposes a concrete false-success path.
4. Claim L3 only after same-SHA Build, TRX verification and artifact upload; physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
