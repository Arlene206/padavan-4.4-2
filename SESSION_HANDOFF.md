# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic main commit on the main branch. Create all changed blobs and one complete tree, parent one commit to live main, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact head_sha; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.
- Before context exhaustion, finish the current safe slice, check the exact SHA and CI state, and leave an accurate first action for the next GPT.

## Current repository state

- Live main before this DHCP option snapshot slice: `077a917ef22e4d5516289264e60b6c4b6e5607f1`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- WAN protocol/static-LAN restore, static IPoE/PPP endpoint+credential snapshot checks, checked MAN/IPTV route clears/publication and EAPOL auth staging are committed on main.
- Exact `077a917ef22e4d5516289264e60b6c4b6e5607f1` Actions: handoff `136105910007`, WAN route `136105910004`, NVRAM `136105909660`, HTTPD `136105910813`, Process `136105910056`, inventory `136105910595`, Firewall `136105909706`, XY-C1 `136105912457`, and other checks were pending/in progress at last poll; one retrievable completed run was detect-link `136105910096` PASS.
- Same-SHA XY-C1 L3 is not established; physical-router L4 remains NOT VERIFIED.

## Active task

- The WAN reset now checks global MAN/virtual IPTV route-state clearing, global WAN route endpoint publication, EAPOL auth mode and credentials, static IPoE address/netmask/gateway/MTU, manual PPP endpoints, PPP credentials, peer and negotiation options.
- Additional source audit confirmed `dnsenable_x` controls both MAN DNS route installation and pppd `usepeerdns`; `vci` is passed as the DHCP client vendor-class option; and `hostname` is sent to udhcpc as its client hostname. These are configuration inputs, not optional telemetry.
- This slice moves all three into a required checked tuple; a failed snapshot now aborts before WAN activation instead of reusing stale DNS/VCI/hostname settings.
- Next candidate: inspect the final WAN hardware-address snapshot (`wan_hwaddr` and indexed `hwaddr`) for route/interface identity impact, then continue across userland rather than endlessly expanding this one reset function.

## Verification boundary

- Parent `077a917ef22e4d5516289264e60b6c4b6e5607f1` MAN/authentication snapshot exact-head Actions were queued/in progress at last poll; see current SHA runs before relying on that implementation.
- The DHCP/DNS/hostname checked tuple and source assertions await exact-head WAN route/NVRAM and full regression verification.
- Same-SHA XY-C1 detector, MIPS Build, TRX verification and artifact upload must all pass before L3; no physical-router L4 test is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA WAN route/NVRAM, handoff, HTTPD, Process and XY-C1 Actions for the DHCP/PPP option tuple and preceding MAN/auth changes; diagnose failures from exact job logs.
2. Require the same-SHA XY-C1 production detector, MIPS Build, TRX verification and artifact upload before claiming L3.
3. Inspect the WAN hardware-address copies (`wan_hwaddr`, indexed `hwaddr`) and only gate reset on them if a failed write demonstrably affects interface identity or routing. Keep the public void wrappers intact.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep source/host Actions, MIPS build/TRX and physical-device evidence separate. Only same-SHA detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
