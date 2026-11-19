# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the exact live source and CI state once, then resume the concrete engineering task. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits, or stop at planning. Before context exhaustion, complete the safe slice and preserve its exact source/CI boundary.

- One atomic main commit per logical task: create all blobs, one complete tree and one commit with the current main parent, then update main using a non-forced compare-and-swap with that exact expected SHA.
- Verify push-triggered Actions by exact head_sha; a pending run does not block independent work, while a failed relevant run must be triaged before the slice is called validated.
- Preserve security, architecture, compatibility, existing test coverage, lifecycle identity checks, bounded waits and fail-closed UNKNOWN handling.
- The frozen backup backup/pre-plugin-overhaul-2026-11-14 is archival only and must never be modified or deleted. Development stays on main.

## Current repository state

- Main baseline before the active WAN route cleanup: `28fa4dfa2855fd49ea857e3f6ea2f655180beed6`.
- Exact-head Actions on `28fa4dfa2855fd49ea857e3f6ea2f655180beed6`: USB modem WDM/NDIS identity `44582892745` PASS; NVRAM `44582892741` PASS; AI handoff docs `44582892806` PASS; HTTPD security `44582892824` PASS; Process lifecycle inventory `44582892966` PASS; XY-C1 firmware validation `44582892773` PASS, including production-input gates, Build XY-C1, TRX verification and artifact upload (artifact ID recorded on the run; L3 VERIFIED for 28fa only).
- USB checked queue notifications now cover modem NDIS/TTY and USB printer plug/unplug handlers. The NDIS undeclared-local compile regression and stale USB checker assertion are fixed on the same source history; do not repeat that slice.
- Frozen backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify it. PR #1 remains closed and unmerged; do not revive or re-audit.
- No local repository checkout or MIPS toolchain is available in this connector-only environment. Physical router, printer and modem L4 remains NOT VERIFIED.
## Closed prior task — DHCPv6 startup cleanup errno

- start_dhcp6c() used the errno value saved immediately after a successful eval(), even though successful launch does not define errno. If the subsequent temporary callback-environment cleanup failed, the caller could receive stale errno instead of the actual cleanup error.
- The committed helper in trunk/user/rc/src/dhcp6c_callback_generation_policy.h now ignores stale errno after successful launch, uses cleanup errno (or EIO fallback), preserves a real launch error as primary, and is covered by GNU89 callback-generation regression plus the WAN DHCP source contract.
- Exact WAN DHCP and current XY-C1 Build/TRX/artifact plus HTTPD, Process, NVRAM, WAN route and AI docs are all PASS on f50387e29be43e9de4a1b55344390742a4252b53; do not repeat this task.

## Active task — remove the orphaned off-subnet WAN gateway host route

- Parent main SHA: `28fa4dfa2855fd49ea857e3f6ea2f655180beed6`. The USB checked-notification slice is closed at L2/L3: USB `44582892745`, NVRAM `44582892741`, AI docs `44582892806`, HTTPD security `44582892824`, Process inventory `44582892966`, and XY-C1 `44582892773` all PASS on this exact SHA.
- Source review found a lifecycle asymmetry: `wan_up_internal()` installs an explicit metric-1 `/32` route to a valid gateway when `ppp_ifindex(wan_ifname) < 0` and the gateway is outside the leased IPv4 subnet. `wan_down_internal()` removed the default route, DHCP route inventory and configured static routes, but never deleted this separate automatically-added `/32`. It could survive lease replacement or teardown.
- The fix uses one route-policy predicate for both WAN up and down. It preserves the PPP exclusion and old lease IP/netmask check; teardown deletes the `/32` after attempting the default-route delete, normalizes only ESRCH as already-absent success, aggregates other failures and continues with the static-route cleanup.
- Added regression cases for valid off-subnet non-PPP setup/cleanup, on-subnet gateway, PPP interfaces, invalid gateway, absent host route and preservation of an earlier cleanup error. WAN route workflow source contracts assert setup/teardown symmetry and the required cleanup ordering.
- This production-source commit requires its own exact-head WAN route host regression and XY-C1 detector, Build, TRX and artifact verification. No local MIPS build or physical router test is available.
## Verification boundary

- L1: source review confirms the same predicate guards the automatically installed off-subnet gateway `/32` on WAN up and its teardown; PPP, invalid and on-subnet cases are excluded.
- L2: route-policy GNU89/Werror/ASan/UBSan regression and WAN route source contracts require the exact-head WAN route Action; pending until it runs on the new commit.
- L3: exact-head XY-C1 production-input detector, Build, TRX and artifact upload pending for this production source change.
- L4: live WAN lease renew/deconfig, gateway reachability and physical router route-table validation NOT VERIFIED.
## Exact first action for the next GPT

1. Inspect exact-head USB modem identity and XY-C1 Act## Exact first action for the next GPT

1. Inspect exact-head WAN route and XY-C1 Actions for this `/32` lifecycle fix. Confirm policy regression, source contracts, Build XY-C1, TRX verification and artifact upload.
2. Diagnose any relevant failure from its first causal log; do not weaken PPP/subnet exclusions or treat a missing route as a hard cleanup error.
3. After L2/L3 close, continue the next independent P1 stability/security issue outside WAN/USB.
## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session must perform an end-of-session accuracy checkpoint before context exhaustion. Compare live main, source baseline, the exact-head Actions boundary, current module status and this first action. Update this file only when those resumable facts materially change; do not create SHA-refresh or routine snapshot commits.

## Development invariants

- Main-only; never modify the frozen backup or resume PR #1.
- One complete Git tree and one atomic commit per logical task; use force=false with exact parent expected_sha.
- Never force-push, rewrite history, publish partial commits, or weaken regression coverage.
- Report exact SHA Actions and separate L1/L2/L3/L4. Host success is not firmware/hardware success.

