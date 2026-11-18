# 2026-11-18 continuation checkpoint — align lifecycle checker with active rollback regression

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; authorized branch: `main`. Current main before this documentation-contract repair is `8569fe71229f4ef202b4f234bb9a1b6d26074714`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at observed SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. PR #1 remains closed/unmerged.
- Exact-head `8569fe71229f4ef202b4f234bb9a1b6d26074714`: Shadowsocks lifecycle `44305672569` PASS across shell syntax, startup status, process lifecycle, Adblock and Python source contracts. AI handoff `44305672773` failed only because the cross-session mandate omitted the required phrase `Before context exhaustion`; this docs-only correction restores the complete checkpoint rule. Parent `2f8fb1d` AI `44305279681`, Process fail-closed `44305279690` and Process inventory `44305279672` all PASS.

## Active task
- The duplicate tail is removed and the status-19 test label is aligned with the Python source checker. Exact-head Shadowsocks lifecycle `44305672569` now PASSes all seven job steps, including the source checker.
- Restore the mandatory cross-session checkpoint phrase in the handoff; no production logic change is needed.
- Next P1 source task: replace broad substring DNSMasq cleanup across Shadowsocks `oversea`, `ssp_close()` and `ss-rules` with exact directive ownership matching. Preserve unrelated comments, conf-file includes, domain-scoped server/address/ipset, SmartDNS loopback port 6053 and similar paths containing cdn/gfwlist/dnsmasq.dom.

## Verification boundary
- Exact-head Shadowsocks workflow `44305672569` completed successfully through all seven steps: shell syntax, startup regression, bounded process lifecycle, Adblock publication and Python source checker. AI workflow `44305672773` failed its documentation rule because the required phrase `Before context exhaustion` was absent; this commit restores that clause. Process fail-closed `44305279690` and Process inventory `44305279672` on parent `2f8fb1d` PASS.
- This is a handoff/roadmap/log-only correction. No new XY-C1 run expected; product source `6910911ffdd65a7a04eeb97fa1088a1b8a74d886` completed Build/TRX/artifact in `44299349040`.
- Full local repository harness unavailable. Target BusyBox runtime and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Verify the exact-head AI handoff rerun for this phrase restoration; the Shadowsocks lifecycle source/checker is already PASS on `8569fe7`.
2. Implement exact-match DNSMasq ownership cleanup and preservation regressions as one atomic source/test/checker/workflow slice across Shadowsocks `oversea`/`ssp_close()` and `ss-rules`; verify exact-head Shadowsocks, Process, AI and XY-C1 Build/TRX/artifact.
3. Then rotate to the Adbyby lazy/video paired-config rollback risk in `REFACTOR_ROADMAP.md`, reproducing the failure window before implementation.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the file untouched and do not create a no-op commit.

Every session MUST compare live repository state with this checkpoint, verify exact-head Actions, preserve the L3/L4 distinction, and retain an exact next action.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent tasks while checks are pending. Before context exhaustion, leave a live checkpoint with the exact main/source SHA, tested evidence, exact-head CI status, unverified levels, and the next first action. Never force-push, change the frozen backup, reopen PR #1, or claim work that was not executed.
