# 2026-11-18 continuation checkpoint — serialize Shadowsocks DNSMasq updates through RC

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; authorized development branch: `main`. Current live main before this corrective slice is `dde3301aca4056774cd31696fdeb9b98bb0a3773`; the preceding source change routes Shadowsocks DNSMasq restart through the RC lifecycle lock.
- Preserve `backup/pre-plugin-overhaul-2026-11-14` permanently and read-only; its observed SHA remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed and unmerged.
- The exact parent SHA `e71d9e336f4c1ab03725fcbf5fb0da670098ec5a` has five successful push Actions: Shadowsocks lifecycle `44294135620`, Process fail-closed regression `44294135638`, Process inventory umbrella `44294135667`, AI handoff docs `44294135726`, and XY-C1 firmware `44294135701`. The XY-C1 job completed Build XY-C1, TRX verification and artifact upload successfully on that exact source SHA.

## Active task
- DNSMasq/ChinaDNS transaction remains staged, syntax-validated and rollback-protected. After publishing a validated storage include, Shadowsocks calls `ss_restart_dhcpd()` -> `/sbin/restart_dhcpd`; RC owns `/tmp/.dnsmasq.lifecycle.lock`, process inventory, authoritative configuration, NVRAM enablement and daemon readiness. On failure, the previous include is atomically restored and RC restart retried while preserving the original error.
- The RC-restart regression covers success, validation no-op, stop failure/restore, candidate launch failure and original failure-status propagation. `ssp_close()` also retains first-error status from the preceding source slice.
- Latest exact-head `dde3301aca4056774cd31696fdeb9b98bb0a3773` Actions: AI handoff `44301647352` PASS; Process fail-closed `44301647315` and Process inventory `44301647284` PASS; Shadowsocks `44301647472` FAIL only in the DNSMasq rollback case because the test expected 1 while the injected daemon's actual first-start status is 19. Shell syntax and startup-status test passed. The static checker did not run because the lifecycle test stopped the job first; it also contains four Python `script.indexOf()` typos, corrected in this commit.

## Verification boundary
- Current corrective source/test commit is not yet published; the preceding DDE SHA has exact-head Shadowsocks runtime failure `44301647472` (status 19 vs stale expected 1), while AI handoff and both Process Actions pass. No local repository shell test was run; CI is the execution evidence.
- This corrective slice changes only host test expectation, checker Python method calls and checkpoint records. No new XY-C1 run is expected from firmware-source filters; parent product-source L3 remains the separate evidence above.
- Target BusyBox runtime and physical router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Verify this corrective SHA's exact-head Shadowsocks lifecycle (`sh -n`, startup, runtime, Adblock and Python source-contract stages) and AI handoff Actions; resolve any first causal failure. DDE's exact-head Process and handoff Actions passed; XY-C1 is not triggered for these test-only edits.
2. Continue the queued P1 DNSMasq include fix: replace substring cleanup in `shadowsocks.sh` (`oversea`/`ssp_close`) and `ss-rules` with exact directive/path matching. Add injected regressions proving intended generated includes are removed while comments, `conf-file`, domain `address`/`ipset` rules and paths that merely contain `cdn`, `gfwlist`, or `dnsmasq.dom` remain. Review all config writers/callers and commit the complete source/test/checker slice atomically on main.
3. Keep the immutable backup untouched; do not repeat completed startup audits or old CI runs. Rotate to the next risk-ranked module after closing this slice.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the file untouched and do not create a no-op commit.

Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the file untouched and do not create a no-op commit.

Every session MUST compare this checkpoint with live GitHub state, verify the active engineering baseline and exact-head Actions, preserve the L3/L4 distinction, and retain an exact next action. The checkpoint changes only when the resumable task or validation boundary materially changes.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent work while checks are pending. Before context exhaustion, leave a live checkpoint with the exact main/source SHA, tested evidence, CI status, unverified levels and next first action. Never force-push, change the frozen backup, reopen PR #1, or claim work that was not executed.
