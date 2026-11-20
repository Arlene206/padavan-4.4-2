# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic main commit on the main branch. Create all changed blobs and one complete tree, parent one commit to live main, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact head_sha; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.
- Before context exhaustion, finish the current safe slice, check the exact SHA and CI state, and leave an accurate first action for the next GPT.

## Current repository state

- Live main before this global MAN/authentication snapshot slice: `8e203b34bf1375ac5fd944fb46cbf46d2013bac2`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- WAN protocol/static-LAN recovery and checked static-IPoE/PPP endpoint+credential snapshots are on main (parent `8e203b34bf1375ac5fd944fb46cbf46d2013bac2`).
- Exact `8e203b34bf1375ac5fd944fb46cbf46d2013bac2` checks: WAN route `136104091829`, HTTPD `136104092903`, handoff `136104092136`, Process `136104093582`, inventory `136104093045`, Firewall `136104093085`, USB `136104092508`, VPN `136104092436`, teardown `136104092593`, ZeroTier `136104092467`, wireless `136104092617`, dnsmasq `136104092317`, miniupnpd `136104093259`, watchdog `136104092229`, WAN DHCP `136104091737`, and detection checks all PASS. NVRAM boundary `136104092206` and XY-C1 `136104093608` were still in progress at last poll.
- Same-SHA XY-C1 L3 is not established; physical-router L4 remains NOT VERIFIED.

## Active task

- Static IPoE and PPP endpoint/credential snapshot writes are checked and fail closed. The remaining route-critical gap was unchecked clearing of global `wanx_*` and IPTV `manv_*` route/lease state in `reset_man_vars()`; stale gateways/routes could survive a failed NVRAM clear.
- This slice introduces private `reset_man_vars_status()` with first-error checked clears for all existing WAN MAN and virtual-IPTV snapshot keys, preserving the public void wrapper. `reset_wan_vars_status()` calls the status helper and blocks activation on any failed clear. Global MAN IP/netmask/gateway publications are checked as well.
- EAPOL `auth_mode`, `auth_user` and `auth_pass` now use a required checked tuple before WAN startup, so stale authentication mode or credentials cannot be silently reused.
- Regression coverage is extended in the existing WAN source checker; the generic first-error injected policy tests already compile in the WAN route workflow under GNU89/`-Werror`/ASan/UBSan.
- Next candidate: review `dnsenable_x`, PPPoE `vci` and `hostname` only if source use shows stale values affect routing/negotiation/security; avoid broad telemetry conversion.

## Verification boundary

- Parent `8e203b34bf1375ac5fd944fb46cbf46d2013bac2` WAN route and all completed regression checks listed above passed; NVRAM-boundary and XY-C1 were in progress at last poll.
- Checked MAN/IPTV clears, global WAN endpoint publications and EAPOL auth tuple need exact-head WAN route/NVRAM and full CI verification.
- Same-SHA XY-C1 detector, MIPS Build, TRX verification and artifact upload must all pass before L3; no physical-router L4 test is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA WAN route/NVRAM, handoff, HTTPD, Process and XY-C1 Actions for the MAN/authentication snapshot changes; diagnose any failure from its job log.
2. Confirm same-SHA XY-C1 production detector, MIPS Build, TRX verification and artifact upload before claiming L3.
3. Review the remaining initial WAN copies `dnsenable_x`, `vci`, `hostname` for direct route/negotiation/security impact; only convert fields with an evidenced stale-state failure mode, preserving public ABI.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep source/host Actions, MIPS build/TRX and physical-device evidence separate. Only same-SHA detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
