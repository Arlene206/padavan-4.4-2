# 2026-11-18 continuation checkpoint — DNSMasq exact cleanup

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Current main before this correction is `25394b7d214e9a70c78960fb6d7833fd8645e886`; production source fix is `38a2e1b80c78d250c892377647a43a454623228b`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Active task

- Shadowsocks DNSMasq restart is owned by RC `/sbin/restart_dhcpd` and its `/tmp/.dnsmasq.lifecycle.lock` rather than a separate direct kill/launch path.
- Follow-up production fix on `38a2e1b` replaces broad resolver substring deletion with exact removal of `no-resolv`, the bare loopback server and the prior generated `#65353` line; it anchors cache key matches. The first complete test sequence includes preservation checks for comments, no-resolv near-matches, SmartDNS #5353/#6053, domain-scoped server rules and similarly named cache/forward keys.
- Exact-head 38a Shadowsocks run failed only after the first complete PASS sentinel: a stale duplicate tail appended after that marker repeated part of the test suite and contained a malformed line. This correction truncates the file at the first valid final PASS marker; the preceding complete suite is retained.
- Broad substring filters in OverSea mode, `ssp_close()` and both `ss-rules` paths were replaced with the shared `dnsmasq_config_lifecycle.sh` exact-line helper. It rejects symlinks, stages in the same directory, preserves metadata, and uses atomic rename. It preserves SmartDNS `server=127.0.0.1#5353`, custom includes/comments, domain server/ipset/address lines, and paths that merely share `cdn`, `gfwlist` or `dnsmasq.dom` substrings.
- Added host cleanup regression, shutdown cleanup-error injection, source checker contracts, and workflow syntax/runtime coverage. Removed the duplicated process-lifecycle test tail; one terminal PASS sentinel remains and the successful DNS transaction asserts the RC restart call count.

## Verification boundary
- Parent SHA `25394b7d214e9a70c78960fb6d7833fd8645e886` exact-head Actions at last poll: Process inventory umbrella `44308109817`, AI docs `44308109899`, Process fail-closed `44308109826`, and Shadowsocks lifecycle `44308109878` were IN_PROGRESS. The prior 38a run `44307638690` passed shell syntax, startup and standalone DNSMasq cleanup, then failed after the first fully completed test suite because a stale duplicate tail continued after the PASS marker; AI `44307639057` failed only on the missing `## Verification boundary` heading, now restored.
- Original source SHA `ef855ea69acec37a6c0b5ddefaab3214da26fbf7` Shadowsocks `44306582439`, AI `44306582505`, Process inventory `44306583185` and Process fail-closed `44306582350` PASS. XY-C1 `44306582528` was still building at last poll; no same-SHA TRX/artifact PASS is known yet.
- Thus this source SHA's Shadowsocks/AI/process/static/runtime tests are PASS, but XY-C1 L3 remains PENDING until Build, TRX verification and artifact upload complete on that exact SHA. The earlier XY-C1 PASS on `6910911` is historical and does not establish L3 for `ef855`. Hardware L4 remains NOT VERIFIED.
- Host shell tests ran in GitHub Actions; local checkout-based shell execution was NOT RUN in this session.

## Exact first action for the next GPT
1. Verify this correction's exact-head Shadowsocks/AI/Process Actions. Confirm the process lifecycle job now terminates at one terminal PASS marker and reaches Adblock plus the Python source checker.
2. Poll the prior ef855 XY-C1 `44306582528` and 38a XY-C1 `44307638792` by exact SHA; do not claim L3 until the relevant source SHA Build/TRX/artifact upload pass.
3. Then continue scanning other DNSMasq config publishers/cleanup paths for broad destructive substring matching.
3. Keep the frozen backup immutable and use one complete tree plus one atomic commit per logical slice, non-forced expected-SHA updates; do not repeat the completed startup audit.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate
Use small source/test slices, one complete Git tree and one atomic main commit per logical task. Before context exhaustion, leave the next GPT the exact source SHA, tests actually run, exact-head CI status, unverified levels and first action. Never force-push, change the frozen backup or claim validation not run on the same SHA.
