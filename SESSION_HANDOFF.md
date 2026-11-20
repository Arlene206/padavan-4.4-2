# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic main commit on the main branch. Create all changed blobs and one complete tree, parent one commit to live main, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact head_sha; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.
- Before context exhaustion, finish the current safe slice, check the exact SHA and CI state, and leave an accurate first action for the next GPT.

## Current repository state

- Live main before this PPP configuration-snapshot slice: `fb60034c4f8b46661c6c5ebe0531729fb1a793ef`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- WAN protocol staging and injected static-LAN restoration policy are on main. Static IPoE address/netmask/gateway/MTU snapshots now use checked first-error writes (commit `fb60034c4f8b46661c6c5ebe0531729fb1a793ef`).
- Exact `fb60034` checks: WAN route `136101623558`, HTTPD `136101624255`, handoff/docs `136101623351`, Process `136101624226`, inventory `136101624068`, Firewall `136101622994`, USB identity `136101623245`, VPN `136101623880`, WAN teardown `136101623663`, watchdog `136101623018`, wireless `136101622947`, ZeroTier `136101623645`, WAN DHCP `136101622777`, detection, dnsmasq and miniupnpd checks all PASS. NVRAM boundary `136101623717` and XY-C1 `136101623979` were in progress at last poll.
- Same-SHA XY-C1 L3 is not yet established; physical-router L4 remains NOT VERIFIED.

## Active task

- Static IPoE configuration now fails closed on an error copying address/netmask/gateway/MTU into the indexed WAN snapshot. The production policy and injected fail-fast regression are present in the WAN route workflow.
- This slice closes a related PPP risk: manual physical endpoint fields and authentication/negotiation settings were still copied through unchecked setters. A partial write could leave a stale remote peer, username/password, authentication mode or MPPE setting while PPP startup continued.
- Main now checks all four `man_*` values, the PPP username/password tuple, the PPP idle-time value, and the PPPoE/PPTP/L2TP negotiation/peer option tuple. Any failed write records reset status and aborts before WAN activation. The same tested ordered-copy policy is reused; the public void setter ABI is unchanged.
- Next bounded candidate: check the global `wanx_*` manual endpoint snapshot clears/publications and remaining authentication-mode source copies. Avoid broad conversion of telemetry/options unrelated to route/authentication.

## Verification boundary

- The static-IPoE checked-copy commit `fb60034` passed the exact-head WAN route, HTTPD, handoff, Process and independent regression checks. NVRAM boundary and XY-C1 were pending at last poll.
- PPP manual-endpoint and credentials/options fail-fast change awaits exact-head WAN route, NVRAM and full CI verification.
- Same-SHA XY-C1 production detector, MIPS Build, TRX verification and artifact upload must all pass before claiming L3; no physical-router L4 test is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head WAN route/NVRAM, AI handoff, HTTPD, Process and XY-C1 Actions for the PPP snapshot change; diagnose any FAIL by exact job log.
2. Require same-SHA XY-C1 production detector, MIPS Build, TRX verification and artifact upload before L3.
3. Continue the narrow WAN snapshot audit on global `wanx_*` clear/publication and authentication-mode copies. Ensure a failed write cannot leave stale routes/authentication or allow protocol activation; preserve public ABI.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep source/host Actions, MIPS build/TRX and physical-device evidence separate. Only same-SHA detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
