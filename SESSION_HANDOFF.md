# 2026-11-18 continuation checkpoint — DNSMasq exact cleanup

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, branch `main`. Parent source/test commit: `ef855ea69acec37a6c0b5ddefaab3214da26fbf7`. Current main before this follow-up is `91fc1074238a31a8ce2360f1d57d3a83567b6d47`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 remains closed/unmerged.

## Active task

- Shadowsocks DNSMasq restart is owned by RC `/sbin/restart_dhcpd` and its `/tmp/.dnsmasq.lifecycle.lock` rather than a separate direct kill/launch path.
- Follow-up found and fixes one remaining broad staged-config filter: ChinaDNS used substring deletion of `server=127.0.0.1`, which also removed SmartDNS #5353. The transaction now removes exact resolver lines using the shared helper and anchors cache option keys.
- Broad substring filters in OverSea mode, `ssp_close()` and both `ss-rules` paths were replaced with the shared `dnsmasq_config_lifecycle.sh` exact-line helper. It rejects symlinks, stages in the same directory, preserves metadata, and uses atomic rename. It preserves SmartDNS `server=127.0.0.1#5353`, custom includes/comments, domain server/ipset/address lines, and paths that merely share `cdn`, `gfwlist` or `dnsmasq.dom` substrings.
- Added host cleanup regression, shutdown cleanup-error injection, source checker contracts, and workflow syntax/runtime coverage. Removed the duplicated process-lifecycle test tail; one terminal PASS sentinel remains and the successful DNS transaction asserts the RC restart call count.

## Exact-head verification boundary
- Source/test SHA `ef855ea69acec37a6c0b5ddefaab3214da26fbf7`: Shadowsocks `44306582439` PASS; AI handoff `44306582505` PASS; Process inventory `44306583185` PASS; Process fail-closed `44306582350` PASS. XY-C1 `44306582528` remained IN_PROGRESS at build step 32; TRX and artifact upload still pending at last poll.
- Thus this source SHA's Shadowsocks/AI/process/static/runtime tests are PASS, but XY-C1 L3 remains PENDING until Build, TRX verification and artifact upload complete on that exact SHA. The earlier XY-C1 PASS on `6910911` is historical and does not establish L3 for `ef855`. Hardware L4 remains NOT VERIFIED.
- Host shell tests ran in GitHub Actions; local checkout-based shell execution was NOT RUN in this session.

## Exact first action for the next GPT
1. Verify this follow-up source SHA's exact-head Shadowsocks and Process/AI/XY-C1 Actions. The prior `ef855` run has the above four PASS workflows; firmware job `44306582528` was still building when last polled.
2. After ChinaDNS staging preservation tests pass, scan remaining daemon-managed config writes for substring-based destructive cleanup.
3. Keep the frozen backup immutable and use one complete tree plus one atomic commit per logical slice, non-forced expected-SHA updates; do not repeat the completed startup audit.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and end with an accuracy checkpoint before context exhaustion. The checkpoint is mandatory; a document edit is conditional. If all facts remain accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate
Use small source/test slices, one complete Git tree and one atomic main commit per logical task. Before context exhaustion, leave the next GPT the exact source SHA, tests actually run, exact-head CI status, unverified levels and first action. Never force-push, change the frozen backup or claim validation not run on the same SHA.
