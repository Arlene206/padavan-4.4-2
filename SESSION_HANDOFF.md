# 2026-11-18 continuation checkpoint — serialize Shadowsocks DNSMasq updates through RC

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; authorized development branch: `main`. Current live main before this diagnostic slice is `1e75f56d758468e8cacd088af52920c261e9b48a`; product source baseline remains `dde3301aca4056774cd31696fdeb9b98bb0a3773`.
- Preserve `backup/pre-plugin-overhaul-2026-11-14` permanently and read-only; its observed SHA remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed and unmerged.
- The exact parent SHA `e71d9e336f4c1ab03725fcbf5fb0da670098ec5a` has five successful push Actions: Shadowsocks lifecycle `44294135620`, Process fail-closed regression `44294135638`, Process inventory umbrella `44294135667`, AI handoff docs `44294135726`, and XY-C1 firmware `44294135701`. The XY-C1 job completed Build XY-C1, TRX verification and artifact upload successfully on that exact source SHA.

## Active task
- DNSMasq/ChinaDNS transaction remains staged, syntax-validated and rollback-protected. After publishing a validated storage include, Shadowsocks calls `ss_restart_dhcpd()` -> `/sbin/restart_dhcpd`; RC owns `/tmp/.dnsmasq.lifecycle.lock`, process inventory, authoritative configuration, NVRAM enablement and daemon readiness. On failure, the previous include is atomically restored and RC restart retried while preserving the original error.
- The RC-restart regression covers success, validation no-op, stop failure/restore, candidate launch failure and original failure-status propagation. `ssp_close()` also retains first-error status from the preceding source slice.
- Exact-head `dde3301aca4056774cd31696fdeb9b98bb0a3773`: AI handoff `44301647352`, Process fail-closed `44301647315`, and Process inventory `44301647284` PASS; Shadowsocks `44301647472` failed the rollback expectation (expected 1 vs actual 19). Corrective SHA `1e75f56d758468e8cacd088af52920c261e9b48a`: AI handoff `44303526325`, Process fail-closed `44303526428`, and Process inventory `44303526333` PASS; Shadowsocks `44303526422` passed shell syntax/startup and the corrected status-19 rollback check, then failed because the expected `server=127.0.0.1#65353` directive was absent after the successful config transaction. Python checker was skipped; its four `str.indexOf()` calls have now been corrected in the same corrective SHA.

## Verification boundary
- Current main `1e75f56d758468e8cacd088af52920c261e9b48a` fixes the rollback test's incorrect status expectation and Python checker method calls. Its exact-head Shadowsocks run `44303526422` still fails because the published ChinaDNS server directive is absent; the test now emits the complete escaped config contents on that failure. AI handoff and both Process Actions pass. No local shell test was run; CI is the execution evidence.
- This diagnostic slice changes only the regression assertion and handoff/roadmap records. No new XY-C1 run is expected from firmware-source filters; no new product code was changed. Parent product-source L3 remains separate evidence.
- Target BusyBox runtime and physical router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect the exact-head Shadowsocks rerun for this diagnostic commit. Its failure output includes escaped `dnsmasq.conf` lines; identify why the `use_chinadns=1` transaction did not publish the required local upstream and fix the underlying defect, not just the assertion.
2. After the lifecycle suite reaches the Python checker and passes, implement the queued exact DNSMasq include cleanup in `shadowsocks.sh` (`oversea`/`ssp_close`) and `ss-rules`. Test that only the exact generated `conf-dir` entries are removed while comments, `conf-file`, domain `address`/`ipset` rules and unrelated paths containing `cdn`, `gfwlist`, or `dnsmasq.dom` remain.
3. Keep the immutable backup untouched; do not repeat completed startup audits or old CI runs. Rotate to the next risk-ranked module after closing this slice.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the file untouched and do not create a no-op commit.

Every session MUST compare this checkpoint with live GitHub state, verify the active engineering baseline and exact-head Actions, preserve the L3/L4 distinction, and retain an exact next action. The checkpoint changes only when the resumable task or validation boundary materially changes.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent work while checks are pending. Before context exhaustion, leave a live checkpoint with the exact main/source SHA, tested evidence, CI status, unverified levels and next first action. Never force-push, change the frozen backup, reopen PR #1, or claim work that was not executed.
