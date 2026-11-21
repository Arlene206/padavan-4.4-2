# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then use a non-forced expected-SHA compare-and-swap update. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and required tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Do not weaken tests.
- Exact-head push Actions only validate the matching main SHA. PENDING is not a gate on independent work; inspect a relevant FAIL, fix its cause, and move forward.
- Before context exhaustion, finish the safe slice, publish the commit, inspect exact-SHA Actions and leave a truthful next action.
- Keep root `dev_log.md` at 20–119 newest-first entries; preserve `docs/dev-log/2026/11/part-021.md` and its index unchanged. Do not create handoff/roadmap snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending, verify this handoff's source baseline, active task, exact-head Actions, validation boundary, blockers and first next action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no no-op commit. Update this handoff only when next-session-critical facts materially change.

## Current repository state
- Live main parent for the current source slice: `6f47ae209274dd312c30499e8028ab059b68df72`; current commit adds fail-closed USB modem removal transport selection.
- Exact-head parent `6f47ae209274dd312c30499e8028ab059b68df72`: USB modem WDM/NDIS identity #44898560333 PASS (checked WAN NVRAM policy test + modem module contract), Process inventory/lifecycle #44898560487 PASS, AI docs #44898560510 PASS. The USB modem boot NVRAM selection change from that parent is L2 PASS.
- Newly confirmed runtime defect in `safe_remove_usb_modem()`: `nvram_get_int("modem_type")` collapsed NVRAM backend failure to zero, incorrectly treating an NDIS modem as RAS. This could skip NDIS WAN DHCP/network teardown before clearing selection and allowing physical removal. This commit now uses `nvram_get_checked()`, strict parsing and the existing NDIS/RAS status policy; unknown transport aborts before any NDIS/RAS stop action or selection-marker clear.
- Existing checked WAN NVRAM GNU89/ASan/UBSan policy test covers parsing and transport error status; source checker pins the removal gate before either teardown path. Exact-head validation for this new slice pending.
- XY-C1 run #44898560501 is still IN_PROGRESS at last poll on production source SHA `6f47ae209274dd312c30499e8028ab059b68df72`: Build XY-C1 running, TRX verification and artifact upload pending. L3 is NOT YET VERIFIED for this source SHA.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable. Legacy PR #1 remains closed/unmerged.

## Active task
- Verify exact-head USB modem identity, Process lifecycle and AI handoff Actions for the removal transport gate.
- Inspect any failing job logs; keep UNKNOWN transport fail-closed and preserve the existing absent-key RAS default.
- Inspect XY-C1 #44898560501 Build, TRX verification and artifact upload before any L3 claim for production-source SHA `6f47ae209274dd312c30499e8028ab059b68df72`.
- Continue to the next distinct P1 stability/security defect outside the repeated HTTPD/WAN/SkipDB areas after this modem lifecycle slice.

## Verification boundary
- USB boot modem-rule/type checked read on `6f47ae209274dd312c30499e8028ab059b68df72`: dedicated USB, Process and AI exact-head Actions PASS.
- USB removal transport fix: committed; exact-head L2 pending. No local tests run.
- XY-C1 L3 on `6f47ae209274dd312c30499e8028ab059b68df72`: pending Build + TRX + artifact from #44898560501.
- Physical router L4: NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head USB modem identity, Process lifecycle and AI docs push runs.
2. Diagnose the first causal failure log, if any; repair one focused assertion/source issue in an atomic main commit.
3. Inspect XY-C1 #44898560501 Build/TRX/artifact steps. Claim L3 only if all three succeeded for production source SHA `6f47ae209274dd312c30499e8028ab059b68df72`.
4. Continue broad RC startup/lifecycle/security work with one new P1 concrete defect.

Before context exhaustion, update this handoff only when active task, exact-head status or validation boundary changes; no no-op commit.
