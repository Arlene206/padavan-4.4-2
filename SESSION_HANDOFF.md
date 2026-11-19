# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Current main SHA: `76161729272bdfeb553725a43afdf4b142232e10` (documentation contract repair pending exact-head verification). Product source slice SHA: `fe286b8cabd4334664c8270c5212d26a15ce6393`.
- The preceding DHCPv6 startup errno-precedence fix is closed on exact SHA f50387e29be43e9de4a1b55344390742a4252b53. WAN DHCP callback run [44563154100](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154100), WAN route [44563153894](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153894), HTTPD security [44563153840](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153840), NVRAM transaction [44563154066](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154066), Process inventory [44563154013](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154013), and AI handoff docs [44563154047](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154047) all PASS on that exact push SHA.
- XY-C1 run [44563153885](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153885) passed its production-input detector, Build XY-C1, TRX verification and artifact upload on f50387e29be43e9de4a1b55344390742a4252b53 (L3 for that SHA). This evidence does not cover the new ARP changes below.
- Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; do not touch it.
- PR #1 remains closed and unmerged. Its prior valid CA bootstrap/HMAC/package/ROMFS work was already ported to main and verified; do not revive or re-audit it.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical-router L4 is NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task

- Selected from the repository P1/P2 network reliability ledger outside DHCPv6. Target files:
  - trunk/user/rc/src/detect_wan.c, function arpping_gateway();
  - trunk/user/rc/src/detect_wan_arp_result_policy.h;
  - trunk/user/rc/src/tests/detect_wan_arp_result_policy.c;
  - .github/workflows/detect-wan-arp-stability.yml.
- Confirmed source gap: the old packet receive condition required only a minimum frame length, Ethernet EtherType ARP, ARP reply opcode, ARP target hardware equal to the local MAC, and ARP sender IPv4 equal to the gateway. It did not validate the ARP hardware/protocol type or their lengths, Ethernet source/destination addresses, Ethernet-source versus ARP-sender MAC consistency, or the target IPv4 address. A malformed or unrelated frame could be accepted as proof that the gateway is alive and suppress a necessary DHCP recovery.
- THIS COMMIT adds a focused matcher for Ethernet/IPv4 ARP replies addressed to the current local MAC/IP, verifies source MAC consistency, and compares sender/target IPv4 via memcpy so packed unaligned bytes are safe on MIPS. Regression tests cover valid unaligned replies and malformed/unrelated identity fields. The existing local-I/O-error versus silent-no-reply behavior and three-probe DHCP gate remain unchanged.
- Exact-head WAN ARP GNU89 sanitizer regression/source contract (run `44571309902`), Process inventory (`44571310032`), Process lifecycle regression (`44571309881`), and NVRAM transaction audit (`44571309885`) all PASS on product source SHA `fe286b8cabd4334664c8270c5212d26a15ce6393`. AI docs runs `44571309978` and `44572344939` failed because an earlier handoff edit omitted the protocol's exact checkpoint sentence; the latter run confirmed the required literal was still absent. This current edit restores the exact sentence and root heading. XY-C1 run `44571310142` passed the input detector and all pre-build host checks; Build XY-C1, TRX verification, and artifact upload were still in progress at last poll. No local execution is claimed.

## Verification boundary

- L1 source review: the matcher and complete production call-site tuple are reviewed in product source SHA `fe286b8cabd4334664c8270c5212d26a15ce6393`.
- L2 host regression: PASS on exact product source SHA `fe286b8cabd4334664c8270c5212d26a15ce6393` via WAN ARP run `44571309902`; Process inventory, Process regression, and NVRAM audit also PASS. AI docs handoff contract remains the outstanding CI repair.
- L3 target build: exact product source SHA `fe286b8cabd4334664c8270c5212d26a15ce6393` passed the firmware input detector and pre-build host contracts; Build XY-C1, TRX verification and artifact upload were pending in run `44571310142` at last poll. The preceding `f50387e29be43e9de4a1b55344390742a4252b53` build cannot be carried forward.
- L4 physical router/gateway-loss recovery: NOT VERIFIED.
- No local build/test runner or physical router is available here.

## Exact first action for the next GPT

1. Verify the AI handoff documentation contract Action triggered by this handoff repair on its exact commit SHA; it must pass now that the mandated sentence and required section names are committed.
2. Inspect XY-C1 run `44571310142` by its named steps: Build XY-C1, TRX verification and artifact upload. The prior parent firmware build does not validate product source SHA `fe286b8cabd4334664c8270c5212d26a15ce6393`.
3. After ARP L2/L3 and handoff docs are closed, select the next highest-value unfinished P1 stability defect from another user-space module. A likely iNIC explicit-disable status drop in `start_inic_mii()` has been identified but is not yet fixed; verify it and add source-contract/regression coverage before modifying production code. Avoid repeating the closed DHCPv6/ARP socket/signal audit.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

