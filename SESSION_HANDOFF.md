# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; only active ref is `main`.
- Main at start of this validation repair: `3c7a5a125f670335cb1b305674790eece80b1dcc`. Use a live branch fetch and expected-SHA non-force update for each write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Startup branch recovery is complete.
- Rolling log rotation is complete; part-012 preserves the oldest 100 records and root holds 23 entries before this correction.
- Manual WAN route setup in `launch_wanx()` now accumulates status for static MAN address, full route, gateway route and physical IPv6. The associated WAN route workflow passes on `3c7a5a12`.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC startup failures propagate; optional hw_nat/LED work remains best-effort.
- Required WAN physical IFUP, MAN route and IPv6 control failures are preserved in startup status; WAN teardown returns route/modem stop errors.
- The remaining WAN DHCP workflow failure is stale test text: it expects direct static address setup to assign `launch_status`, whereas `launch_wanx()` now captures it in `address_status` and aggregates later route results without overwriting earlier failures.

## Active task

Correct the WAN DHCP callback workflow's static MAN source assertion to match `address_status`. Parent `43866769025` passed the compiled C regressions, then failed only on this outdated source assertion. No production code change is involved.

## Verification boundary

- Parent `3c7a5a125f670335cb1b305674790eece80b1dcc`: WAN route `43866768822` SUCCESS; AI docs `43866769102` SUCCESS; WAN DHCP `43866769025` FAILED at the stale `launch_status = ifconfig(...)` assertion after native tests passed.
- No same-SHA firmware L3 or physical-router L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head WAN DHCP and AI docs Actions for this source-contract correction.
2. Continue the independent PPP/IPoE startup propagation review: mandatory miniupnpd migration, PPTP/L2TP physical setup, pppd launch, demand readiness, static address, EAPOL and DHCP launch failures must be reflected in `start_wan()`'s return status.
3. Preserve the first cause across fallback/cleanup; optional tuning and notification remain nonfatal.
4. Firmware L3 requires same-SHA Build, TRX verification and artifact upload; physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
