# 2026-11-18 continuation checkpoint — align lifecycle checker with active rollback regression

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; authorized branch: `main`. This checker/docs fix is based on parent `2f8fb1d05a505afb5e9bde72f3d5b2b4a2ac6769`; resolve final live SHA after commit.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at observed SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. PR #1 remains closed/unmerged.
- Parent exact-head AI handoff `44305279681` PASS; Process lifecycle fail-closed `44305279690` was still running at latest inspection. Shadowsocks `44305279761` advanced through shell syntax, startup status, process regression and Adblock, then failed at the final Python checker. The checker expected the old label `dnsmasq restart failure triggers rollback`; the active regression correctly tests preservation of original error status 19 under label `dnsmasq restart failure preserves original status`.

## Active task
- Align the Python source checker with the active status-19 rollback regression label. No production logic or runtime test expectation change is needed in this corrective slice.
- Next P1 source task: replace broad substring DNSMasq cleanup across Shadowsocks `oversea`, `ssp_close()` and `ss-rules` with exact directive ownership matching. Preserve unrelated comments, conf-file includes, domain-scoped server/address/ipset, SmartDNS loopback port 6053 and similar paths containing cdn/gfwlist/dnsmasq.dom.

## Verification boundary
- Parent exact-head Shadowsocks job `44305279761` passed shell syntax, startup status, bounded process lifecycle and Adblock tests, then failed only the Python checker’s stale rollback-label assertion at line 211. The earlier duplicate test tail is removed; one terminal PASS sentinel remains.
- Parent AI handoff `44305279681` PASS. Process lifecycle umbrella was pending at latest inspection. This commit is checker/docs-only; no XY-C1 run expected.
- Full local repository harness unavailable. Target BusyBox runtime and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Verify exact-head Shadowsocks Python checker and AI/Process Actions for this repair. Do not assume further checker assertions pass until the job completes.
2. Implement exact-match DNSMasq ownership cleanup and preservation regressions as one atomic source/test/checker/workflow slice; verify its own exact-head XY-C1 Build, TRX and artifact upload.
3. Then rotate to the Adbyby lazy/video paired-config rollback risk in `REFACTOR_ROADMAP.md`, reproducing the failure window before implementation.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the file untouched and do not create a no-op commit.

Every session MUST compare live repository state with this checkpoint, verify exact-head Actions, preserve the L3/L4 distinction, and retain an exact next action.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice. Continue independent tasks while Actions are pending. Never force-push, change the frozen backup, reopen PR #1, or claim unexecuted verification.
