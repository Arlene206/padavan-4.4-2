# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic commit on `main`. Create all changed blobs and one complete tree, parent one commit to live `main`, then update with `force=false` and exact `expected_sha`.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact `head_sha`; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.

## Current repository state

- Parent main before static-LAN-route recovery: `da3081e5c2ae97ef65640cff0ed48931dda2ce7f`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- Protocol staging commit `b376e2c` checks `wan_proto` publication before protocol selection through private `reset_wan_vars_status()`; public void `reset_wan_vars()` ABI remains unchanged. Startup latches and rejects failed staging; full restart/reconnect propagate the status.
- Exact `b376e2c`: Process `44677505519`, inventory `44677505291`, AI docs `44677505499`, NVRAM boundary `44677505416`, WAN route `44677505399`, USB modem `44677505465`, Firewall Startup `44677505362`, watchdog `44677505504`, wireless `44677505287`, VPN `44677505502`, ZeroTier `44677505505`, WAN DHCP `44677505407`, WAN teardown `44677505480`, dnsmasq `44677505471`, miniupnpd `44677505472`, Detect WAN ARP `44677505438`, Detect link `44677505468`, Detect internet `44677505382` all passed. HTTPD `44677505315` failed because its embedded test still expected the old void reset call; corrected in `da3081e` and exact HTTPD run `44677916869` PASS.
- AI docs run `44677916863` failed only because the handoff checkpoint heading/text omitted the exact required wording; this handoff now carries both required phrases.
- NVRAM boundary `44677505416` PASS; XY-C1 `44677505398` was IN_PROGRESS at last poll. No L3 current-source claim. L4 remains NOT VERIFIED.

## Completed in this slice

- Fixed the recovery gap after protocol staging failure in `full_restart_wan()`. The function flushes the WAN/bridge route set before staging the active WAN protocol. Previously, a failed `wan_proto` snapshot returned immediately and skipped the later `add_static_lan_routes(IFNAME_BR)`, leaving LAN static routes removed even though WAN restart had correctly aborted.
- The failure path now retries restoration of static LAN routes before returning. It retains the protocol-staging error as the primary status; if route restoration also fails, it logs the secondary error and preserves first-error semantics. WAN activation remains aborted.
- Extended `check_wan_firewall_status.py` to require this recovery ordering and both error logs. Public reset ABI is unchanged.

## Verification boundary

- Static-LAN recovery source/checker change has not yet passed exact-head Actions.
- AI docs wording fix awaits exact-head revalidation.
- XY-C1 `44677505398` on `b376e2c` was still pending. This later source commit needs its own detector, Build XY-C1, TRX and artifact upload before L3 can be claimed.
- No physical-router L4 test.

## Exact first action for next GPT

1. Fetch live main and inspect exact-head AI docs, WAN route/Process/Firewall/HTTPD and XY-C1 Actions for this recovery change. Confirm the mandatory checkpoint phrases pass the documentation validator and inspect any remaining causal failure.
2. Require same-SHA XY-C1 detector, Build, TRX verification and artifact upload before claiming L3.
3. Continue the critical WAN config audit in `reset_wan_vars_status()`: assess static IP address/mask/gateway/MTU and PPP credential/options copies. Only make a bounded checked-publication change where stale values can alter routing or authentication; preserve first-error behavior and the public void wrapper. Avoid bulk-converting telemetry and optional metadata.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no documentation-only commit.

## Evidence discipline

Keep host/source, cross-build/TRX and physical-device evidence separate. Only exact-SHA passing detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
