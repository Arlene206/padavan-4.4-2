# 2026-11-18 continuation checkpoint — exact DNSMasq directive cleanup

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, active branch `main`; this atomic source/test/checker/workflow slice is based on parent `e71777167669441d9866c0eec007580b70e6f02b`. Resolve the resulting commit SHA from live main during recovery.
- Frozen branch `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed and unmerged.
- Parent `e717771` AI handoff workflow `44305951729` PASS. Its product/test parent `8569fe7` Shadowsocks workflow `44305672569` PASSed syntax, startup status, process lifecycle, Adblock and Python source checker. The `8569fe7` AI run failed only on the missing `Before context exhaustion` phrase, restored by `e717771`. No XY-C1 run was present on these parents. Prior source SHA `6910911` has exact XY-C1 Build/TRX/artifact PASS; that result does not transfer to this source commit.

## Active task
- Replace broad substring deletion in Shadowsocks `oversea`, `ssp_close()` and both `ss-rules` cleanup sites with a shared exact-directive helper. It trims surrounding whitespace only for comparison, rejects symlink configs, stages in the same directory, preserves metadata and atomically renames. The allowlist removes only Shadowsocks-owned include/resolver directives; it deliberately preserves SmartDNS `server=127.0.0.1#5353`, domain-scoped DNS rules, comments, unrelated `conf-file` entries and similar paths.
- Host regressions cover exact removal, idempotence, comments and hostile near-match preservation, custom includes, domain server/ipset/address rules, missing files, invalid arguments and symlink refusal. Shutdown tests inject helper failure. Static contracts and Actions include the new helper/test. The duplicated process-test tail is removed; one terminal PASS remains, and the successful DNS transaction asserts the RC restart call count.

## Verification boundary
- This source/test/checker/workflow commit has not yet run exact-head Actions. Host shell tests are NOT RUN locally; CI must establish runtime status on this exact SHA.
- Parent `8569fe7` Shadowsocks is PASS; parent `e717771` AI docs contract is PASS. This source-changing commit needs its own exact-head Shadowsocks and AI results, plus XY-C1 Build/TRX/artifact verification before claiming L3.
- Target BusyBox/router runtime and physical hardware L4 remain NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect this commit's exact-head Actions by `head_sha`: Shadowsocks shell syntax/startup/process/Adblock/static checker, AI handoff, and XY-C1 Build/TRX/artifact. Fix an actual failing step and keep L3 evidence SHA-scoped.
2. Continue independent reliability work while checks are pending. After exact cleanup passes, scan other daemon-managed config files for destructive substring deletion and add similarly narrow preservation tests.
3. Keep the frozen backup untouched; use one complete Git tree/commit per logical slice, non-forced expected-SHA main updates, and do not repeat the completed startup audit.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts and the next action are already accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate
Use one atomic main commit per logical source/test slice and continue independent work while Actions are pending. Before context exhaustion, leave the next GPT the exact source SHA, tests actually run, exact-head CI status, unverified levels and first action. Never force-push, alter the frozen backup, reopen PR #1 or claim unperformed validation.
