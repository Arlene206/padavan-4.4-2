# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries, and the next precise task. If facts and first action remain accurate, explicitly conclude the check with no file change and no commit. Use one atomic main commit for material changes only; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis/correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`.
- Current source baseline: `9a1d4e442da2af1d64cef9397fcd973386831617`. DHCPv6 status getters and pre-stage NVRAM snapshot landed in `e100bb3b88a68e3db6e23721bf6852631126895e`; later commits align its regression/source contracts.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 is closed/unmerged.
- WAN PPP launch mode uses a checked NVRAM transaction snapshot and immutable flags for lock selection, daemon stop/start, multi-WAN startup and rollback. Invalid WAN unit inputs return deterministic EINVAL.
- Modem WAN link-change reconnect uses checked modem readiness/WISP state; Inadyn startup checks DDNS enable state before stopping the existing daemon.
- DNSMasq resolver flags, DHCP integers, static-host snapshots, domain validation, resolver-file creation and lifecycle publication are covered by its workflow; this slice adds DHCPv6 status/snapshot preflight.

## Active task

Manual RT/WL radio changes previously used `nvram_wlan_get_int()` (safe_get + atoi, collapsing backend errors to zero) and `nvram_wlan_set_int()` (discarding write status). The common `manual_change_radio_status()` now reads the precise `rt_radio_x`/`wl_radio_x` key with `nvram_get_checked()`, parses it fail-closed, propagates read errors before no-op, and persists through checked `nvram_set_int()` before hardware changes. PID-1's radio applet now obtains the same checked prior state from this operation instead of making a second unchecked read. The wireless workflow contains a GNU89 policy regression and source-order contract.
## Verification boundary

- Local C/Python tests were not run in this connector-only session.
- Wi-Fi manual-radio source SHA `9957ef226468e6766fdd222b53551459d7a7d3d3`: GNU89/ASan/UBSan wireless policy binary PASS; embedded wireless source-contract failed at Python assertion line 93 because the new “no unchecked safe getter” assertion matched the literal function name inside its explanatory C comment. The production behavior/order assertions before that check passed. A comment-only source correction removes the false-positive, and this follow-up re-runs the workflow.
- Previous exact XY-C1 SHA `9a1d4e442da2af1d64cef9397fcd973386831617`: run #45035251615 PASS including Build XY-C1, TRX verification and artifact upload. This is historical L3 evidence for that SHA only, not for Wi-Fi descendants.
- No current-SHA XY-C1 L3 result or router hardware L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-SHA wireless radio rerun for the comment/source-contract false-positive correction, and verify XY-C1 Build/TRX/artifact on one identical current source SHA.
2. Continue broad userland stability/security work outside WAN/PPP, USB modem, Inadyn, DNSMasq and manual Wi-Fi radio controls.
3. Keep backup immutable. XY-C1 L3 requires Build, TRX and artifact upload all PASS on one SHA; L4 requires physical device verification.
