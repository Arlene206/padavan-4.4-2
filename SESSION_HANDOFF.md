# 2026-11-18 continuation checkpoint — remove duplicated Shadowsocks test tail

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; authorized development branch: `main`. This test-only repair is based on parent `77ba640f2f8af88f8cb2856e5af62dd770ea765a`; resolve the current SHA from the live `main` ref during recovery.
- Preserve `backup/pre-plugin-overhaul-2026-11-14` permanently and read-only; observed SHA remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed and unmerged.
- Parent exact-head Actions: AI handoff `44304776554` PASS; Process fail-closed `44304776568` PASS; Process inventory `44304778927` PASS. Shadowsocks `44304776719` FAILS because `process_lifecycle.sh` has a second copy of its DNSMasq/run_bin/watchcat/DNS proxy tests appended after the first complete PASS sentinel. The first dangling command executes the config file and returns permission denied; a second stale rollback assertion also expects 1 instead of 19. The first complete test sequence already covers all those cases.
 
## Active task
- Remove only the duplicate tail after the first complete regression PASS sentinel. Keep the complete first suite, including its correct status-19 rollback assertion and ChinaDNS failure diagnostics. The resulting test file ends with exactly one PASS sentinel.
- Next P1 source task remains exact-directive DNSMasq cleanup in Shadowsocks `oversea`, `ssp_close()` and `ss-rules`. It must remove exact generated directives and preserve comments, unrelated `conf-file`, domain-scoped `server=`, `address=`/`ipset=` rules, SmartDNS port 6053 and similar paths containing `cdn`, `gfwlist` or `dnsmasq.dom`.

## Verification boundary
- Parent exact-head Shadowsocks run `44304776719` passed shell syntax and startup status, then printed the first PASS sentinel and failed on the appended duplicate tail with `Permission denied` executing the config path. The diagnostic upstream assertion lives inside the duplicate segment, so this log does not establish a production ChinaDNS publication defect.
- Parent AI handoff, Process fail-closed and inventory Actions all PASS. This commit changes only regression tests and checkpoint records; no new XY-C1 run is expected. Product source SHA `6910911ffdd65a7a04eeb97fa1088a1b8a74d886` has XY-C1 Build/TRX/artifact PASS.
- Full local repo harness unavailable. Target BusyBox runtime and physical-router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main` and verify exact-head Shadowsocks, AI handoff and Process Actions. Confirm the process test ends with one PASS sentinel and reaches Adblock publication and the Python source checker.
2. Implement the queued exact-match DNSMasq ownership helper as one atomic source/test/checker/workflow slice across Shadowsocks and ss-rules; this production-source change must pass its own XY-C1 Build, TRX and artifact steps.
3. After this slice passes, rotate to the Adbyby managed lazy/video paired-config rollback item in `REFACTOR_ROADMAP.md`; reproduce the failure window before implementation.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the file untouched and do not create a no-op commit.

Every session MUST compare this checkpoint with live repository state, verify the active engineering baseline and exact-head Actions, preserve the L3/L4 distinction, and retain an exact next action. The checkpoint changes only when the resumable task or validation boundary materially changes.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent work while checks are pending. Before context exhaustion, leave a live checkpoint with exact main/source SHA, tested evidence, CI status, unverified levels and next first action. Never force-push, change the frozen backup, reopen PR #1, or claim work that was not executed.
