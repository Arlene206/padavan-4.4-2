# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; only active development ref is `main`.
- Main at start of PPP/IPoE propagation slice: `48ff82881c7e83c29ab64a5285ee9d2bf8be6955`. Refetch current main before each write and use non-forced expected-SHA updates.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Startup/backup recovery is complete and must not be repeated.
- Root `dev_log.md` has 24 records before this slice; part-012 preserves the oldest 100 complete records verbatim.
- Closed unmerged PR #1's HMAC/CA features remain integrated in main; no stale cherry-pick is needed.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII startup errors propagate to PID 1; optional hw_nat/LED remain best-effort.
- WAN startup preserves physical IFUP, MAN route and required IPv6-control failures. The recent MAN route fix aggregates static address, full route, gateway and physical IPv6 results.
- This slice propagates all identified required PPP/IPoE launch failures (UPnP-stop migration gate, PPTP/L2TP physical setup, pppd launch, demand readiness, static IPv4 address, EAPOL and DHCP) into the first-error WAN status. Best-effort cleanup remains secondary.

## Active task

`start_wan()` now aggregates required PPP/IPoE transition errors. `wait_ppp_up()` changes to a 0-success/nonzero-error contract and returns the specific interface, socket or demand-route failure, rather than boolean false. The modem WAN policy binary gains first-error cases, the shared source checker verifies all production branches, and the WAN route workflow asserts status return/propagation before fallback.

## Verification boundary

- Parent `48ff82881c7e83c29ab64a5285ee9d2bf8be6955` Actions were queued at last inspection, including AI docs `43867282875`, WAN DHCP callback `43867282815`, Process `43867282752`, NVRAM `43867282884`, AliDDNS `43867282992`, infosvr `43867282926`, AliyunDrive `43867282758` and AdGuardHome `43867282753`.
- Parent `3c7a5a125f670335cb1b305674790eece80b1dcc`: WAN route `43866768822`, AI docs `43866769102`, infosvr `43866769082`, AdGuardHome `43866769028`, AliyunDrive `43866768813`, AliDDNS `43866768867`, NVRAM `43866768743` and Process `43866769041` passed. WAN DHCP `43866769025` failed only because a source assertion still expected the old MAN address variable, corrected on `48ff8288`.
- No same-SHA firmware Build/TRX/artifact L3 or hardware L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head Process, XY-C1, WAN route, WAN DHCP and AI docs for this PPP/IPoE change; repair the first concrete failure.
2. Continue reviewing required WAN authentication, lease and PPP callback transitions, ensuring failures reach their originating callback / startup status without undoing valid lease state.
3. Preserve first-error semantics; do not escalate optional MTU, notifications or cleanup.
4. Claim firmware L3 only after same-SHA Build, TRX verification and artifact upload pass. Physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
