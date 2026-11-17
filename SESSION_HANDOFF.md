# 2026-11-18 continuation checkpoint — static IPoE address cleanup fail-closed handling

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS source slice: `b83c1cbf99d457c5155a4d613c49f8871f3c1fff`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Factory-reset production fix `0d58928ee6ab852cbbc37f4a2201a685c8252e1e`: GPIO run `44197451556` PASS including GNU89 ASan/UBSan and source contracts. Exact-head AI documentation rerun `44197796498` on `b83c1cb` PASS; Process `44197451701` and MiniUPnP `44197451163` on `0d58928e` PASS.
- Exact XY-C1 run `44197454090` on `0d58928e` had passed all pre-build checks through WAN and PPPoE-MWAN gates; Build XY-C1 was still IN_PROGRESS at last poll, with TRX verification and artifact upload PENDING. Do not claim L3 until all three complete.
- The prior static IPoE EAPOL-launch cleanup fix `5d0618c` and its multiline checker repair `80dcdc6` are complete; exact WAN run `44197177575` on `80dcdc6` PASS.
- No full local checkout/MIPS toolchain is available. Local target compilation is NOT RUN; physical-router L4 remains NOT VERIFIED.

## Active task
- THIS source slice closes a separate P1 static-IPoE failure path. When assigning a configured static IPv4 address failed, the code attempted to clear a potentially partial address but discarded the clear result. The physical WAN interface could stay administratively UP with a stale address.
- On failed clear, the code now aggregates/logs that error, forces the physical interface down and aggregates/logs any failed force-down. A per-WAN guard prevents EAPOL and the remaining IPoE protocol activation from running after this cleanup failure. Successful clears and dynamic IPoE are unchanged.
- Added a GNU89 policy regression and expanded `scripts/check_wan_firewall_status.py` to enforce the error/cleanup/skip ordering. Existing WAN/XY-C1 workflows already include the touched source, header, test and checker paths.

## Verification boundary
- Exact-head WAN route GNU89/ASan/UBSan regression and source contract: PENDING for THIS commit.
- Exact-head XY-C1 Build, TRX verification and artifact upload: PENDING for THIS commit. The prior `0d58928e` build `44197454090` is still in progress at the last poll and does not verify this new source SHA.
- Factory-reset GPIO regression on `0d58928e`: PASS. AI handoff on `b83c1cb`: PASS. No local host/target compile or hardware L4 claim.

## Exact first action for the next GPT
1. Fetch live `main`; inspect exact-`head_sha` push runs for WAN route, XY-C1 and AI handoff on THIS commit.
2. Require the WAN policy binary and complete static-IP/EAPOL/firewall source contract to PASS. For XY-C1, require Build XY-C1, TRX verification and artifact upload.
3. Separately recheck `0d58928e` run `44197454090`; if it completes, record the Build/TRX/artifact steps without treating it as validation of THIS newer WAN source.
4. Repair only the first causal failure in a separate atomic commit; continue independent P1 work while any build remains PENDING. Keep backup immutable and hardware L4 NOT VERIFIED.

## Mandatory handoff checkpoint
Every session MUST inspect the live repository state, exact-SHA Actions that materially changed, current handoff/roadmap facts, and the first actionable next task before ending. The checkpoint is mandatory; a document edit is conditional. Record new PASS/FAIL/PENDING state accurately. If the facts and first action remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, then implement concrete defects. Do not wait idly on PENDING Actions or ask the user what to do next.

Every logical multi-file change must use Git data APIs to create all blobs, one complete tree and one atomic main commit, parented to freshly fetched main and published with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs or modify the frozen backup.

Before context exhaustion, finish a safe slice, commit atomically, inspect exact-SHA Actions and leave a precise first action.
