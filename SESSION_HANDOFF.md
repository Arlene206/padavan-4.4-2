# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Main before this protocol staging correction: `616f06e9ba246c47419672693666431fac5deb8f`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- WAN active-interface product commit `822f88b` gates RAS/PPP/IPoE firewall/client startup on checked `wanN_ifname_t` writes. Its WAN route regression passed; three shared workflows initially failed only on the stale 22-versus-28 first-error aggregate count. Checker repair commit `616f06e` expects 28 and explicitly verifies all three new paths. On exact `616f06e`, Firewall Startup `44676888660`, USB modem identity `44676888577` and AI docs `44676888701` PASS; Process regression `44676888646` was IN_PROGRESS at last check.
- The original WAN recovery fix `68571af` has L3 VERIFIED: XY-C1 run `44674653396` passed production detector, MIPS build, TRX and artifact upload on that exact source SHA. Do not generalize that result to newer source commits. Physical-router L4 remains NOT VERIFIED.
- No exact-head XY-C1/Build/TRX/artifact result for `822f88b` is established yet; its first XY-C1 run stopped before build because the shared startup checker had the stale aggregate-count assertion.

## Active task completed in this slice

- Confirmed `get_wan_proto(int unit)` in `trunk/user/shared/src/netutils.c` reads `wanN_proto`; `reset_wan_vars()` copied configured `wan_proto` into this key using unchecked void `set_wan_unit_param()`, then immediately used it to choose PPP/IPoE setup. If the NVRAM write failed, a stale prior protocol could route the restart down the wrong WAN branch.
- Added checked staging via `set_wan_unit_value_status(unit, "proto", nvram_safe_get("wan_proto"))`. On failure the errno-normalized status is logged, latched and returned before `get_wan_proto()`.
- Preserved the public `void reset_wan_vars(void)` ABI with a wrapper. The boot path's legacy caller is covered by the private latch; `start_wan()` aborts before sysctl/VLAN/firewall/network side effects if the staged protocol failed. Full restart and reconnect use the status implementation directly and return the failure immediately.
- Extended `scripts/check_wan_firewall_status.py` to pin stage-before-read, failure latch/return, the legacy wrapper, and fail-closed gates in boot/start/restart/reconnect paths. Existing GNU89/Werror/ASan/UBSan WAN NVRAM status test covers errno normalization; the WAN route workflow already runs this checker.
- Exact-head verification for THIS commit is pending. No production-source L3 or physical L4 claim is made for this change.

## Verification boundary

- On exact checker head `616f06e`, Firewall Startup run `44676888660`, USB modem identity run `44676888577`, and AI docs run `44676888701` PASS. Process lifecycle regression run `44676888646` was still IN_PROGRESS at last poll.
- No local full-repository build is available. Do not claim a MIPS build or host binary pass for the new protocol-reset source change until exact Actions report it.
- L3 requires the production-input detector, Build XY-C1, TRX verification and artifact upload all PASS on one identical source SHA. L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head WAN route, Firewall Startup, Process regression/inventory, USB modem identity, AI docs and NVRAM transaction Actions for THIS protocol-reset commit; diagnose any first causal log.
2. Require same-SHA XY-C1 detector, MIPS Build, TRX verification and artifact upload before claiming L3 for this source change.
3. Continue the critical reset/NVRAM staging audit from `reset_wan_vars_status()`: inspect only failure-sensitive `set_wan_unit_param()` copies for static IP/netmask/gateway/MTU and PPP options that later drive `ifconfig`, routes, authentication and daemon launch. Preserve optional telemetry and void ABI; fix the next proven callsite with first-error propagation and an injected/source regression.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped host step is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
