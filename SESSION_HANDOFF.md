# 2026-11-18 continuation checkpoint — serialize Shadowsocks DNSMasq updates through RC

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; authorized development branch: `main`. This logical source/test slice is based on parent `e71d9e336f4c1ab03725fcbf5fb0da670098ec5a`; the new commit created from it is the current checkpoint (resolve its final SHA from the live main ref on recovery).
- Preserve `backup/pre-plugin-overhaul-2026-11-14` permanently and read-only; its observed SHA remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed and unmerged.
- The exact parent SHA `e71d9e336f4c1ab03725fcbf5fb0da670098ec5a` has five successful push Actions: Shadowsocks lifecycle `44294135620`, Process fail-closed regression `44294135638`, Process inventory umbrella `44294135667`, AI handoff docs `44294135726`, and XY-C1 firmware `44294135701`. The XY-C1 job completed Build XY-C1, TRX verification and artifact upload successfully on that exact source SHA.

## Active task
- DNSMasq/ChinaDNS transaction remains staged, syntax-validated and rollback-protected. This slice removes the independent Shadowsocks DNSMasq kill/launch path: after publishing a validated storage include, it now calls the existing `ss_restart_dhcpd()` wrapper, which delegates to `/sbin/restart_dhcpd`. That RC API uses `/tmp/.dnsmasq.lifecycle.lock`, checks process inventory, regenerates the authoritative RC configuration, honors service enablement and verifies the resulting daemon state. On restart failure the previous include is atomically restored and the RC restart is attempted again; the original failure status is retained.
- The Shadowsocks process regression injects the RC restart boundary and covers the successful transaction, validation failure (no restart), stop failure with restored live config, and failed replacement launch followed by old-config restart. The static checker rejects a direct kill/launch path and pins the RC restart/rollback order.
- The preceding `ssp_close()` first-error propagation fix is included in parent `e71d9e336f4c1ab03725fcbf5fb0da670098ec5a`; all five exact-head parent Actions listed above PASS.

## Verification boundary
- New source/test/checker commit: exact-head push Actions are pending at the time this handoff is written; fetch and match every run to the eventual main SHA before marking verified. Host runtime was not executed locally because a repository checkout was unavailable in this session; CI is the execution evidence.
- Parent `e71d9e336f4c1ab03725fcbf5fb0da670098ec5a` has L3 PASS (XY-C1 build, TRX and uploaded artifact). Do not transfer that result to this source-changing commit until its own exact-SHA XY-C1 job completes those three steps.
- Target BusyBox runtime and physical router L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main` and inspect the Shadowsocks lifecycle, AI handoff and XY-C1 push runs for this commit by exact `head_sha`; repair any real failed step before calling the slice validated. Check Process inventory jobs if the commit triggered them; do not invent a non-triggered run.
2. Continue the next queued P1 fix: replace broad substring-based DNSMasq include cleanup in the `oversea` mode, `ssp_close()`, and `ss-rules` with exact directive matching. Add injected regression cases proving target directives are removed while unrelated comments, `conf-file`, domain address/ipset rules and paths that merely contain `cdn`, `gfwlist`, or `dnsmasq.dom` are preserved. Include all source/test/checker changes in one atomic main commit and match its exact SHA Actions.
3. Keep the immutable backup untouched. Do not repeat the completed startup audit or re-run old CI history; rotate to the next highest-risk module after closing this slice.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the file untouched and do not create a no-op commit.

Every session MUST compare this checkpoint with live GitHub state, verify the active engineering baseline and exact-head Actions, preserve the L3/L4 distinction, and retain an exact next action. The checkpoint changes only when the resumable task or validation boundary materially changes.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent work while checks are pending. Before context exhaustion, leave a live checkpoint with the exact main/source SHA, tested evidence, CI status, unverified levels and next first action. Never force-push, change the frozen backup, reopen PR #1, or claim work that was not executed.
