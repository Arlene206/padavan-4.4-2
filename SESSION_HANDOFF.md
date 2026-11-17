# 2026-11-18 continuation checkpoint — static IPoE EAPOL cleanup failure handling

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS source slice: `fa36ad1823344790f17511bef5890df3a9f78b09`.
- Main-only development continues. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- MiniUPnP lifecycle serialization source is `03a993e558b23f5f163ca1a904cbf7b54f6ef7e9`. Its dedicated exact-head workflow `44195714916` on `fa36ad1` passed after the heredoc indentation repair; AI docs `44195714933` passed. Process lifecycle `44195398478` on `dc6d32d` and AI docs `44195398394` passed.
- XY-C1 run `44193165931` on product source baseline `88770d7` completed Build XY-C1, TRX verification and artifact upload successfully. The subsequent MiniUPnP changes through `fa36ad1` changed tests/workflows/docs only; the 03a product source was covered by that build.

## Active task
- THIS commit closes a P1 WAN startup defect. On static IPoE, when EAPOL authentication startup failed, `start_wan()` attempted to clear the static WAN address but discarded the `ifconfig()` result. If clearing failed, a stale address could remain on an administratively up physical WAN interface.
- The new code captures, aggregates and logs the clear status and attempts to take the physical interface down. A failed forced-down operation is also aggregated/logged. The original EAPOL failure remains the first status; dynamic IPoE is unchanged.
- Added a policy helper and GNU89 regression in `service_start_status.h` / `tests/service_start_status.c`; extended `scripts/check_wan_firewall_status.py` to enforce ordering, status propagation and diagnostics. Existing WAN and XY-C1 workflows already watch all touched files and compile the policy binary.

## Verification boundary
- Exact-head WAN regression/source check for THIS commit pending.
- Exact-head XY-C1 Build, TRX verification and artifact upload pending.
- No local checkout/MIPS toolchain is available; no local target compilation is claimed.
- Physical-router EAPOL/WAN behavior (L4) remains NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head WAN-route Actions; require the GNU89 policy regression and EAPOL source checker to pass.
2. Inspect exact-head XY-C1; require Build, TRX verification and artifact upload. If a checker fails, inspect the first causal assertion; if the build fails, inspect the first causal compiler error.
3. Continue the next independent P1 reliability/security slice without repeating the MiniUPnP/EAPOL audits.

## Mandatory handoff checkpoint
Mandatory handoff checkpoint: inspect live main, active task, exact-SHA Actions, validation boundary and frozen backup before ending. The checkpoint is mandatory; a document edit is conditional. Do not manufacture a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, then implement concrete defects. Do not wait idly on PENDING Actions or ask the user what to do next.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced compare-and-swap using the expected SHA. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, commit atomically, inspect exact-SHA Actions and leave a precise first action.
