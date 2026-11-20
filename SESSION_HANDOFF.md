# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic main commit on the main branch. Create all changed blobs and one complete tree, parent one commit to live main, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact head_sha; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.
- Before context exhaustion, finish the current safe slice, check the exact SHA and CI state, and leave an accurate first action for the next GPT.

## Current repository state

- Main before injected route-recovery regression: 0b6bccb9c0bc7324bf22083b66a4ad31625dd4dc.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Protocol staging commit b376e2c checks configured wan_proto before get_wan_proto(), preserves the public void reset_wan_vars() ABI, and blocks startup/restart/reconnect after a failed snapshot.
- HTTPD source-contract repair da3081e passed exact HTTPD run 44678385087 on 0b6bccb.
- Exact 0b6bccb checks: WAN route, Firewall Startup, HTTPD, Process regression/inventory, USB modem, VPN, DHCP, teardown, ARP, watchdog, wireless, ZeroTier, DNSMasq, miniupnpd and detection suites passed. AI docs run 44678384983 failed because this handoff lacked the literal phrase "one atomic main commit"; NVRAM boundary 44678384900 and XY-C1 44678385066 were in progress at last poll.
- Same-SHA XY-C1 L3 is not established for b376e2c or 0b6bccb; physical-router L4 remains NOT VERIFIED. Older 68571af L3 does not apply to current source.

## Active task

- The WAN restart path flushes active WAN and br0 routes plus global cache before resetting the active WAN protocol. A protocol-snapshot failure previously aborted before the ordinary add_static_lan_routes(IFNAME_BR), leaving static LAN routes absent.
- Current main retries LAN static route restoration before aborting, keeps the original protocol error as the first return value, and logs a secondary route-repair error. This patch moves the callback/first-error contract into a unit-testable policy and adds injected route restoration tests.
- The sanitizer-ready WAN route unit already compiles through the existing workflow with GNU89, -Werror, ASan and UBSan. The source checker verifies policy registration and restore-before-return / no WAN-reactivation ordering.
- The current patch also restores required AI validator literals and the mandatory Active task heading; the previous documentation run failed before those phrases were corrected.

## Verification boundary

- This policy/test hardening awaits exact-head WAN route, HTTPD, Process, AI docs, Firewall Startup, USB identity and NVRAM-boundary reruns.
- XY-C1 requires production-input detection, MIPS Build, TRX verification and artifact upload all passing on the same source SHA before L3 can be claimed.
- No physical-router L4 test is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head AI docs, WAN route regression (including the injected callback cases), HTTPD, Process, Firewall Startup, USB identity and NVRAM-boundary Actions for this commit.
2. Check the XY-C1 production detector, Build, TRX verification and artifact upload on this identical SHA; do not reuse older images.
3. Continue the narrowly scoped WAN config audit in reset_wan_vars_status(): static IP address/netmask/gateway/MTU and PPP credentials/options only where failed publication can affect routing, authentication or daemon launch. Preserve the public void reset ABI and first-error behavior; do not bulk-convert telemetry.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep source/host Actions, MIPS build/TRX and physical-device evidence separate. Only same-SHA detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
