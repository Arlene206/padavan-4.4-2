# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume exact active work; avoid repeated audits and planning-only stops.

- Each logical task ends in one atomic main commit on the main branch. Create all changed blobs and one complete tree, parent one commit to live main, then update with force=false and exact expected_sha.
- Never force-push, amend published commits, develop on branches/PRs, or modify the frozen backup.
- Check Actions by exact head_sha; diagnose failures from causal logs, and work on independent tasks while checks are pending.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, and meaningful regressions. Do not ask what to do next.
- Before context exhaustion, finish the current safe slice, check the exact SHA and CI state, and leave an accurate first action for the next GPT.

## Current repository state

- Live main before WAN MAC snapshot validation: `e487695db417478a5879b99da8d18871c3363d62`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only.
- WAN route checker order fix commit `e487695db417478a5879b99da8d18871c3363d62`: exact WAN-route `136106824574` PASS; handoff validator `136106824953` PASS. XY-C1 run `136106825395` remained in progress at last poll.
- Source parent `e487695db417478a5879b99da8d18871c3363d62` includes checked MAN/IPTV reset/route state, EAPOL credentials and DNS/VCI/hostname tuples; its exact WAN route run still needed rerun after the checker correction. No same-SHA L3 claim.
- Physical-router L4 remains NOT VERIFIED.

## Active task

- The remaining WAN reset identity gap was `wan_hwaddr`: the configured MAC was written through unchecked persistent `nvram_set()`, then copied to indexed `hwaddr` through the void setter. These values are consumed by WAN VLAN/soft-bridge setup (`set_interface_hwaddr`, tracked VLAN creation), so a failed write could configure an interface using a stale MAC.
- This slice checks both the global persistent MAC update and indexed per-WAN copy, preserving the configured-MAC/fallback selection. Any write failure latches reset status and stops before later interface/VLAN activation.
- Next, move beyond this WAN reset queue after CI verifies this change: inspect the most serious cross-userland process/service lifecycle or security defect in the current source tree, rather than continuing low-value config-key conversions.

## Verification boundary

- `e487695` exact WAN-route and handoff checks passed. Its XY-C1 cross-build was still in progress at last poll; no detector/build/TRX/artifact L3 claim.
- WAN MAC checked-write changes and the expanded source contract await exact-head WAN route/NVRAM and firmware CI.
- Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-SHA WAN route, NVRAM boundary, handoff and XY-C1 build/verification/artifact steps for this MAC snapshot change.
2. Diagnose any production compile or source-contract failure from the exact job log; do not claim L3 until detector, MIPS build, TRX and artifact upload pass on the same SHA.
3. Once the WAN queue is green, switch to a separate high-impact service/process/security bug across Padavan userland.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts or the next action materially change. Otherwise make no no-op documentation commit.

## Evidence discipline

Keep source/host Actions, MIPS build/TRX and physical-device evidence separate. Only same-SHA detector, build, TRX and artifact upload establish L3. A cross-build does not establish L4.
