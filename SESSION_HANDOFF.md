# 2026-11-19 continuation checkpoint — strict WAN gateway ARP reply identity

## Cross-session execution mandate

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Do not repeat completed audits, create status-only commits, or stop at planning.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Parent main SHA for the current source slice: f50387e29be43e9de4a1b55344390742a4252b53.
- The preceding DHCPv6 startup errno-precedence fix is closed on exact SHA f50387e29be43e9de4a1b55344390742a4252b53. WAN DHCP callback run [44563154100](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154100), WAN route [44563153894](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153894), HTTPD security [44563153840](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153840), NVRAM transaction [44563154066](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154066), Process inventory [44563154013](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154013), and AI handoff docs [44563154047](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154047) all PASS on that exact push SHA.
- XY-C1 run [44563153885](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153885) passed its production-input detector, Build XY-C1, TRX verification and artifact upload on f50387e29be43e9de4a1b55344390742a4252b53 (L3 for that SHA). This evidence does not cover the new ARP changes below.
- Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; do not touch it.
- PR #1 remains closed and unmerged. Its prior valid CA bootstrap/HMAC/package/ROMFS work was already ported to main and verified; do not revive or re-audit it.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical-router L4 is NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active engineering slice — validate the complete WAN gateway ARP reply

- Selected from the repository P1/P2 network reliability ledger outside DHCPv6. Target files:
  - trunk/user/rc/src/detect_wan.c, function arpping_gateway();
  - trunk/user/rc/src/detect_wan_arp_result_policy.h;
  - trunk/user/rc/src/tests/detect_wan_arp_result_policy.c;
  - .github/workflows/detect-wan-arp-stability.yml.
- Confirmed source gap: the old packet receive condition required only a minimum frame length, Ethernet EtherType ARP, ARP reply opcode, ARP target hardware equal to the local MAC, and ARP sender IPv4 equal to the gateway. It did not validate the ARP hardware/protocol type or their lengths, Ethernet source/destination addresses, Ethernet-source versus ARP-sender MAC consistency, or the target IPv4 address. A malformed or unrelated frame could be accepted as proof that the gateway is alive and suppress a necessary DHCP recovery.
- THIS COMMIT adds a focused matcher for Ethernet/IPv4 ARP replies addressed to the current local MAC/IP, verifies source MAC consistency, and compares sender/target IPv4 via memcpy so packed unaligned bytes are safe on MIPS. Regression tests cover valid unaligned replies and malformed/unrelated identity fields. The existing local-I/O-error versus silent-no-reply behavior and three-probe DHCP gate remain unchanged.
- Exact-head WAN ARP policy sanitizer/source-contract, Process inventory, AI docs and XY-C1 Build/TRX/artifact are pending at commit creation; check them by the resulting main SHA. No local execution is claimed.

## Verification boundary

- L1 source review: the matcher and complete production call-site tuple are reviewed in THIS COMMIT.
- L2 host regression: pending exact-head workflow detect-wan-arp-stability.yml; its GNU89 -Wall -Wextra -Werror ASan/UBSan tests must execute and the production source contract must pass.
- L3 target build: pending exact-head XY-C1 production-input detector, Build XY-C1, TRX verification and artifact upload; the preceding f50387e29be43e9de4a1b55344390742a4252b53 build cannot be carried forward.
- L4 physical router/gateway-loss recovery: NOT VERIFIED.
- No local build/test runner or physical router is available here.

## Exact first action for the next GPT

1. Fetch live main; inspect exact push Actions for the ARP-reply-validation commit by head_sha, starting with WAN ARP sanitizer/source-contract, Process inventory, AI handoff docs and XY-C1. Confirm Build, TRX verification and artifact upload separately; do not reuse parent L3.
2. If any exact-head run failed, inspect the first causal job/log and repair that failure with a new atomic main commit without weakening the tuple, no-reply, local-error, or DHCP-renew assertions. Pending runs may be left for a later useful checkpoint while other independent work proceeds.
3. Once the ARP slice is closed, select the next highest-value unfinished P1 stability defect from a different user-space module in REFACTOR_ROADMAP.md; keep the project rotation broad and avoid repeating the closed DHCPv6/ARP socket/signal audit.

## Mandatory handoff checkpoint

Before a conversation ends, compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

