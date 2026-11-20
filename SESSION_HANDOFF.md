# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic main commit on the main branch. Create all changed blobs and one complete tree, parent one commit to live main, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact head_sha; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.
- Before context exhaustion, finish the current safe slice, check the exact SHA and CI state, and leave an accurate first action for the next GPT.

## Current repository state

- Live main before this WAN checker correction: `eac0f17e12e394ede94e56dc4d432770256f9009`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- Production WAN reset now checks global MAN/virtual IPTV state clears, global route endpoint writes, EAPOL auth mode/credentials and DHCP DNS/VCI/hostname tuples, in addition to static-IPoE and PPP endpoint/options.
- Exact parent `077a917` WAN route job `136105910004` failed only at the final static source checker. All preceding GNU89/`-Werror`/ASan/UBSan policy executables shown in that job passed. The checker used the first `if (proto_status != 0)` in the entire reset body, which now belongs to the new MAN clear gate before protocol publication; the protocol-order assertion must anchor its failure search after `proto_write`. This commit makes that fix.
- Exact `eac0f17` Actions were queued/in progress at last poll; recheck after this checker correction. Same-SHA XY-C1 L3 is NOT VERIFIED; physical-router L4 remains NOT VERIFIED.

## Active task

- The WAN protocol source checker now anchors the protocol-publication failure gate after the protocol write itself, so earlier independent checked-reset gates cannot be mistaken for the protocol write's error condition.
- Production code on parent `eac0f17e12e394ede94e56dc4d432770256f9009` additionally checks `dnsenable_x`, DHCP VCI and hostname before WAN startup; these directly influence MAN DNS routes, pppd DNS negotiation and udhcpc identity/options.
- Next source task remains a focused inspection of `wan_hwaddr` and indexed `hwaddr`: determine whether a failed NVRAM write can leave stale interface identity/MAC and gate WAN startup if so. Avoid changing unrelated keys.

## Verification boundary

- Parent `077a917` WAN route check failed only at the final source contract due the checker indexing an earlier unrelated status gate; all earlier regression executable commands passed. Exact run URL/job: `136105910004`.
- The subsequent parent `eac0f17e12e394ede94e56dc4d432770256f9009` added the DNS/VCI/hostname tuple; its host CI was still queued/in progress at last poll. This checker correction and its re-run await exact-head status.
- Same-SHA XY-C1 detector, MIPS Build, TRX verification and artifact upload must all pass before L3. No physical-router L4 test is claimed.

## Exact first action for the next GPT

1. Fetch live main and check exact-SHA WAN route, NVRAM boundary, AI handoff, HTTPD, Process and XY-C1 runs for this checker fix.
2. If WAN route passes, inspect and verify the MAN/authentication/DHCP option source assertions on the exact parent source; diagnose any further checker or compile failure from its log.
3. Confirm same-SHA XY-C1 detector, MIPS Build, TRX verification and artifact upload. Then inspect `wan_hwaddr`/indexed `hwaddr` for real interface-identity impact.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep source/host Actions, MIPS build/TRX and physical-device evidence separate. Only same-SHA detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
