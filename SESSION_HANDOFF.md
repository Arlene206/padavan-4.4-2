# 2026-11-18 continuation checkpoint — DNSMasq exact cleanup

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Source/test commit: `ef855ea69acec37a6c0b5ddefaab3214da26fbf7`. This doc-only checkpoint is based on that source commit; resolve the checkpoint commit SHA from live main.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Implemented source slice
- Shadowsocks DNSMasq restart is owned by RC `/sbin/restart_dhcpd` and its `/tmp/.dnsmasq.lifecycle.lock` rather than a separate direct kill/launch path.
- Broad substring filters in OverSea mode, `ssp_close()` and both `ss-rules` paths were replaced with the shared `dnsmasq_config_lifecycle.sh` exact-line helper. It rejects symlinks, stages in the same directory, preserves metadata, and uses atomic rename. It preserves SmartDNS `server=127.0.0.1#5353`, custom includes/comments, domain server/ipset/address lines, and paths that merely share `cdn`, `gfwlist` or `dnsmasq.dom` substrings.
- Added host cleanup regression, shutdown cleanup-error injection, source checker contracts, and workflow syntax/runtime coverage. Removed the duplicated process-lifecycle test tail; one terminal PASS sentinel remains and the successful DNS transaction asserts the RC restart call count.

## Exact-head verification boundary
- Source/test SHA `ef855ea69acec37a6c0b5ddefaab3214da26fbf7`: Shadowsocks lifecycle run `44306582439` PASS; AI handoff run `44306582417` PASS; Process inventory umbrella run `44306582491` PASS. Process fail-closed run `44306582451` and XY-C1 firmware run `44306582528` were PENDING at the last retrieved status check.
- Thus this source SHA's Shadowsocks/AI/static/runtime tests are PASS, but XY-C1 L3 remains PENDING pending Build, TRX verification and artifact upload on that exact SHA. The earlier XY-C1 PASS on `6910911` is historical and does not establish L3 for `ef855`. Hardware L4 remains NOT VERIFIED.
- Host shell tests ran in GitHub Actions; local checkout-based shell execution was NOT RUN in this session.

## Exact first action for the next GPT
1. Fetch live main, then re-query `actions/runs?head_sha=ef855ea69acec37a6c0b5ddefaab3214da26fbf7`. Resolve pending Process fail-closed `44306582451` and XY-C1 `44306582528`; inspect job steps/logs if either fails. Mark L3 PASS only after same-SHA Build, TRX and artifact upload complete.
2. Continue independent stability work while CI is pending. Then scan remaining daemon-managed configuration paths for destructive substring deletion, with preservation tests.
3. Keep the frozen backup immutable and use one complete tree plus one atomic commit per logical slice, non-forced expected-SHA updates; do not repeat the completed startup audit.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate
Use small source/test slices, one complete Git tree and one atomic main commit per logical task. Before context exhaustion, leave the next GPT the exact source SHA, tests actually run, exact-head CI status, unverified levels and first action. Never force-push, change the frozen backup or claim validation not run on the same SHA.
