# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic main commit on the main branch. Create all changed blobs and one complete tree, parent one commit to live main, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact head_sha; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.
- Before context exhaustion, finish the current safe slice, check the exact SHA and CI state, and leave an accurate first action for the next GPT.

## Current repository state

- Live main before this static-IPoE write-safety slice: `20f9a599a4d057b89e0b7d7edba31cea19c3b549`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- WAN protocol staging checks the configured protocol copy before reading the indexed protocol; static-LAN route restoration after staging failure is implemented through an injected, first-error policy with GNU89 sanitizer tests.
- The HTTPD source contract was updated for the injected route-recovery refactor in commit `20f9a599a4d057b89e0b7d7edba31cea19c3b549`; its exact-head HTTPD and handoff-doc checks were still running at the last poll.
- The prior exact-head AI-doc failure on `48e61b6` was resolved: the current handoff contains the required literal `one atomic main commit` and `## Active task

- Critical finding: static-IPoE reset copied `wan_ipaddr`, `wan_netmask`, `wan_gateway` and `wan_mtu` into the indexed WAN snapshot using the unchecked void setter. A transient NVRAM write error could therefore leave a stale last-known-good address or gateway, yet reset could proceed toward WAN activation.
- This slice adds a status-returning parameter-copy helper and a tested, fail-fast ordered-copy policy. Static IPoE reset now latches any one of the four write failures in `wan_vars_reset_status` and returns before activation; the legacy void setter ABI is untouched.
- The policy test covers successful ordering, stopping at the first injected failure, and invalid arguments. The existing WAN route workflow compiles it with GNU89, `-Werror`, ASan and UBSan; the source checker pins the production call path.
- Next candidate after validation: check only manual PPP endpoint and credential/negotiation copies where stale values can affect routes or authentication; avoid converting optional telemetry/metadata.

## Verification boundary

- Static-IPoE fail-fast source and injected policy regression await exact-head WAN route/NVRAM Actions.
- HTTPD status-aware recovery contract correction from parent `20f9a599a4d057b89e0b7d7edba31cea19c3b549` awaits its exact-head HTTPD and AI-doc reruns.
- Same-SHA XY-C1 production detector, MIPS Build, TRX verification and artifact upload are required before L3; no physical-router L4 test is claimed.

## Exact first action for the next GPT

1. Fetch live main; check exact-head WAN route/NVRAM regression and HTTPD/handoff-contract Actions for the static-IPoE slice and preceding contract repair. Diagnose any FAIL from its exact job log.
2. Check the same-SHA XY-C1 production detector, MIPS Build, TRX verification and artifact upload. Do not claim L3 from a parent SHA.
3. Continue the bounded WAN reset audit with manual PPP physical endpoint and PPP credentials/options only where a failed copy can change routing or authentication. Preserve fail-closed startup and the public void reset ABI.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep source/host Actions, MIPS build/TRX and physical-device evidence separate. Only same-SHA detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
