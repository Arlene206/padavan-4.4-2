# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic main commit on the main branch. Create all changed blobs and one complete tree, parent one commit to live main, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact head_sha; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.
- Before context exhaustion, finish the current safe slice, check the exact SHA and CI state, and leave an accurate first action for the next GPT.

## Current repository state

- Live main at start of this slice: `48e61b68f3f4578b21a2e44a2d0df0dac944a01e`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- Protocol staging commit `b376e2c` checks configured `wan_proto` before `get_wan_proto()`, preserves the public void `reset_wan_vars()` ABI, and blocks startup/restart/reconnect after a failed snapshot.
- Static-LAN route recovery is implemented through `rc_wan_restore_lan_routes_after_failed_reset()`; it attempts the restore before aborting, retains the protocol-reset error as primary, and surfaces a secondary restore error. WAN route workflow `136097663143` PASS.
- Exact `48e61b68f3f4578b21a2e44a2d0df0dac944a01e` results: AI handoff/records `136097660956` PASS; inventory `136097662843` PASS; WAN route `136097663143` PASS; HTTPD `136097661907` FAIL at embedded source assertion line 88 because the checker still looked for the old `status` reset variable after the implementation switched to `reset_status` and added the recovery policy; this slice corrects that contract. Process `136097661505`, ZeroTier `136097661454`, NVRAM `136097661666`, and XY-C1 `136097661278` were in progress at last poll. Other completed relevant WAN/Firewall/USB/teardown/VPN/watchdog/wireless/DNSMasq/detection checks passed.
- Same-SHA XY-C1 L3 is not established on this source yet; physical-router L4 remains NOT VERIFIED.

## Active task

- A WAN protocol snapshot failure can occur after WAN/bridge route flush, so returning immediately could leave the configured static LAN routes removed. Main now uses an injected recovery policy and test cases to verify restoration, first-error precedence, no restoration after successful reset, and fail-closed behavior when the restoration callback is missing.
- The exact-head HTTPD source contract failed because it expected the pre-refactor local `status` variable. The production source now uses `reset_status` and routes recovery through `rc_wan_restore_lan_routes_after_failed_reset()`; the checker is updated to require that actual ordering and both failure diagnostics.
- Next production slice remains a focused checked-publication pass on `reset_wan_vars_status()`: static IPoE IP/netmask/gateway/MTU first, then assess only manual PPP endpoint and authentication/options copies whose failure can alter routing or credentials. Keep public void ABI and first-error behavior.

## Verification boundary

- HTTPD source-contract correction awaits exact-head rerun.
- AI handoff, WAN route policy/test, and the independent completed checks on parent `48e61b68f3f4578b21a2e44a2d0df0dac944a01e` passed; Process, ZeroTier, NVRAM-boundary and XY-C1 remained pending at last poll.
- XY-C1 requires production-input detection, MIPS Build, TRX verification and artifact upload all passing on the same source SHA before L3 can be claimed.
- No physical-router L4 test is claimed.

## Exact first action for the next GPT

1. Fetch live main and check exact-head HTTPD, AI handoff, WAN route, NVRAM, Process, ZeroTier and XY-C1 Actions for this contract repair; diagnose any remaining failure from its job log.
2. Require same-SHA XY-C1 production detector, Build, TRX verification and artifact upload before claiming L3.
3. Continue the bounded checked-publication audit in `reset_wan_vars_status()`, starting with static IPoE address/netmask/gateway/MTU. Preserve the public void reset ABI and stop protocol activation on any critical write failure.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep source/host Actions, MIPS build/TRX and physical-device evidence separate. Only same-SHA detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
