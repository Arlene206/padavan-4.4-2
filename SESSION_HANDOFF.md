# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic commit on `main`. Create all changed blobs and one complete tree, parent one commit to live `main`, then update with `force=false` and exact `expected_sha`.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact `head_sha`; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.

## Current repository state

- Main before this HTTPD test-contract repair: `b376e2c2e90c5b2d49edceab6c875b47f29dc27c`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- Protocol staging commit `b376e2c` checks `wan_proto` publication before protocol selection through private `reset_wan_vars_status()`; public void `reset_wan_vars()` ABI remains unchanged. Startup latches and rejects failed staging; full restart/reconnect return the status.
- On exact `b376e2c`: Process `44677505519`, Process inventory `44677505291`, AI docs `44677505499`, NVRAM boundary `44677505416`, and the listed WAN/USB/Firewall/watchdog/wireless/VPN/ZeroTier/DHCP/dnsmasq/miniupnpd checks passed. HTTPD `44677505315` failed only in its embedded Python source assertion at line 83: it still searched for `reset_wan_vars();` in full restart and reconnect, while the new status-aware caller correctly invokes `reset_wan_vars_status()`. The stale contract is repaired here.
- XY-C1 `44677505398` is still IN_PROGRESS at last poll. No L3 claim for `b376e2c` yet; L4 remains NOT VERIFIED.
- Earlier product SHA `68571af` has L3 PASS, but it predates the RAS/PPP/IPoE publication gates and protocol-staging hardening.

## Completed in this slice

- Updated HTTPD's embedded WAN-stop/source checker to verify the private status-returning reset function and its failure gate in full restart and reconnect, including that reconnect aborts before scheduling deferred work. It no longer expects the obsolete direct void-reset call.
- This is a test-contract-only correction; no C source behavior changed.

## Verification boundary

- HTTPD contract correction has not yet run on exact head.
- XY-C1 and NVRAM runs on `b376e2c` are still tracked by exact SHA. NVRAM `44677505416` PASS; XY-C1 `44677505398` PENDING.
- No physical-router L4 test.

## Exact first action for next GPT

1. Fetch live `main`; inspect exact-head HTTPD, Process, WAN route, USB modem, Firewall, NVRAM and XY-C1 Actions for this HTTPD contract repair. Verify the embedded checks pass.
2. If protocol staging fails during `full_restart_wan()`, restore static LAN routes before returning: route flush occurs before `reset_wan_vars_status()`, but the current failure path returns before `add_static_lan_routes(IFNAME_BR)`. Verify the required recovery ordering, aggregate any route restore error while preserving the original protocol-staging error, and add a regression/source contract. Preserve public reset ABI.
3. Continue focused NVRAM publication audit only on settings whose failure can change network mode, routes, interface state or credentials. No bulk conversion of optional metadata/telemetry.

## Mandatory checkpoint

Inspect the live repository before ending each session. Update handoff only when resumable facts or the next action materially change. Otherwise make no documentation-only commit.

## Evidence discipline

Keep host/source, cross-build/TRX, and physical-device evidence separate. Only exact-SHA passing detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
