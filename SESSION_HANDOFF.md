# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Current main SHA: `3d86aebbef26086a4383489efc4470e35e91b9df`. Previous WAN ARP product-source SHA `fe286b8cabd4334664c8270c5212d26a15ce6393` has exact L2/L3 PASS; AI docs correction `f3f39d7d6e7acde293982a51e9dcab76994b3c8e` also PASS.
- The preceding DHCPv6 startup errno-precedence fix is closed on exact SHA f50387e29be43e9de4a1b55344390742a4252b53. WAN DHCP callback run [44563154100](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154100), WAN route [44563153894](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153894), HTTPD security [44563153840](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153840), NVRAM transaction [44563154066](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154066), Process inventory [44563154013](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154013), and AI handoff docs [44563154047](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563154047) all PASS on that exact push SHA.
- XY-C1 run [44563153885](https://github.com/Arlene206/padavan-4.4-2/actions/runs/44563153885) passed its production-input detector, Build XY-C1, TRX verification and artifact upload on f50387e29be43e9de4a1b55344390742a4252b53 (L3 for that SHA). This evidence does not cover the new ARP changes below.
- Frozen backup remains backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807; do not touch it.
- PR #1 remains closed and unmerged. Its prior valid CA bootstrap/HMAC/package/ROMFS work was already ported to main and verified; do not revive or re-audit it.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical-router L4 is NOT VERIFIED.

## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — preserve iNIC explicit-disable interface status

- Product reliability source `fe286b8cabd4334664c8270c5212d26a15ce6393` completed strict WAN gateway ARP reply identity validation. Exact-head WAN ARP GNU89 sanitizer/source contract `44571309902`, Process inventory `44571310032`, Process lifecycle regression `44571309881`, and NVRAM transaction audit `44571309885` PASS. XY-C1 Build `44571310142` passed the production-input detector, Build XY-C1, TRX verification, and artifact upload (L3 PASS on that exact product source SHA).
- The AI handoff validator initially failed because the committed handoff omitted its exact checkpoint sentence; a second edit attempt still did not contain the literal. This was corrected in `f3f39d7d6e7acde293982a51e9dcab76994b3c8e`; exact-head AI handoff Action `44573162136` PASS. The docs-only intermediate commit `76161729272bdfeb553725a43afdf4b142232e10` failed the same check and is superseded by the passing correction.
- New source task: in `trunk/user/rc/src/net_wifi.c::start_inic_mii()`, the `inic_disable` force-disable branch discarded `wif_control_success(ifname_inic, 0)` with a void cast. Later bridge detach, PHY isolation, RGMII disable and reset were attempted, but the returned aggregate only reflected those later operations. If `ifconfig rai0 down` failed while later cleanup succeeded, the caller could receive success even though the interface-down operation failed.
- THIS COMMIT folds the failed rai0 down result into the sticky `all_started` status without adding an early return, so all safety cleanup continues. A GNU89 regression models the failed interface-down followed by successful bridge/PHY cleanup; the wireless radio workflow pins both the production aggregation and regression call. Wireless radio run `44573883138` compiled and ran the GNU89 radio policy binary successfully but its production source contract failed on the new checker assertion: the checker expected an over-escaped multiline string. This follow-up changes the assertion to independent stable source tokens. AI docs `44573883136`, Process inventory `44573883508`, iNIC signal `44573883181`, watchdog signal `44573883165`, and HTTPD security `44573883284` PASS on `3d86aebbef26086a4383489efc4470e35e91b9df`; Process regression `44573883051`, NVRAM audit `44573883023`, and XY-C1 `44573883318` were still pending at last poll. The exact-head Wireless checker correction and new XY-C1 result remain pending on this follow-up.

## Verification boundary

- L1 source review: strict WAN ARP reply identity is closed on `fe286b8cabd4334664c8270c5212d26a15ce6393`; the iNIC force-disable status gap is reviewed in THIS COMMIT.
- L2 host regression: the standalone GNU89 wireless policy binary passed on source SHA `3d86aebbef26086a4383489efc4470e35e91b9df`; the exact source-contract check failed only because its multiline expected string was over-escaped. This follow-up repairs that assertion and must pass the exact-head Wireless workflow. Prior WAN ARP L2, AI docs, inventory, signal and HTTPD checks have passed.
- L3 target build: XY-C1 run `44571310142` PASS on product SHA `fe286b8cabd4334664c8270c5212d26a15ce6393`, including Build XY-C1, TRX verification, and artifact upload. The new iNIC source change in THIS COMMIT requires its own exact-head detector/build/TRX/artifact run.
- L4 physical router/gateway-loss recovery: NOT VERIFIED.
- No local build/test runner or physical router is available here.

## Exact first action for the next GPT

1. Verify the new exact-head Wireless radio Action for this checker-only repair; its GNU89 policy binary already passed on parent `3d86aeb`, and the source matcher must now pass with independent token checks.
2. Inspect exact-head Process regression, NVRAM audit and XY-C1 run `44573883318`; confirm firmware detector, Build XY-C1, TRX and artifact. The new workflow contract change does not alter production source but the code source remains the current parent plus this checker fix.
3. After closing iNIC L2/L3, select the next highest-priority unfinished P1 issue from a different subsystem; keep source work broad and do not revisit closed DHCPv6, WAN ARP socket, or gateway ARP identity audits.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

