# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint before ending. Record the current source SHA, exact-SHA Action state, test/build boundaries, and next concrete task. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Make one atomic main commit for a material checkpoint change; never create a no-op commit. Before context exhaustion, leave the next GPT a precise first action and truthful verification status.

- Work only on `main`. Never force-push, use side branches/PRs, or modify the frozen backup.
- For each logical engineering slice, change source + regression test + source contract together; create all blobs, one complete tree and one atomic commit; update `main` using a non-forced compare-and-swap with the fetched expected SHA.
- Inspect the exact product-source SHA's push Actions. PASS permits the next item; FAIL must be diagnosed and fixed; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; default branch: `main`.
- Parent SHA for the current WAN PPP launch-mode slice: `0af768201a45cf28482ffa4e8c42b6846e16081a`. The source/handoff checkpoint produced by this slice is THIS COMMIT; resolve the live commit SHA on next recovery.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable/read-only.
- Branch inventory at recovery: `main` plus the frozen backup only. Legacy PR #1 is closed/unmerged; do not merge it wholesale.
- `safe_start_xl2tpd_locked()` already snapshots VPN server/client selectors and credentials transactionally before process scan/config truncation.
- WAN PPP launch mode is now read as one checked NVRAM transaction: `pppoemwan_enable` always, and `wan_l2tpd` only for APP_RPL2TP L2TP. The wrapper rejects read/close/copy errors before session/lifecycle effects and passes immutable flags into the locked path.
- `SESSION_HANDOFF.md` and `dev_log.md` are the current root handoff/log records; do not repeat startup audits or create archive snapshots.

## Active task

The WAN PPP launch selector failure mode has been addressed in this atomic source slice. Previously, unchecked `nvram_match()` calls could treat an NVRAM backend failure as a disabled multi-WAN setting, bypass the lifecycle lock, select the wrong L2TP daemon, or skip rollback. The new transaction snapshot and pure policy preserve exact literal-`1` defaults while failing closed on read/close/copy errors. One snapshot now drives lock selection, L2TP stop/start, multi-WAN start and partial-failure cleanup.

Regression coverage lives in `trunk/user/rc/src/tests/ppp_wan_option_policy.c`; the source contract is `scripts/check_ppp_wan_options.py`.

## Verification boundary

- Local C/Python tests were not run in this connector-only session.
- On parent SHA `0af768201a45cf28482ffa4e8c42b6846e16081a`: WAN route #45030079767 PASS; IPv6 service-type #45030079720 compiled/passed its GNU89 sanitizer test but its source checker failed at the router-hosts ordering assertion; AI handoff #45030079766 failed because the former handoff omitted this mandatory checkpoint section/wording.
- This commit must be checked by exact-head WAN route, VPN lifecycle, HTTPD security, XY-C1 and AI handoff Actions. No target firmware build, TRX artifact, or hardware validation is claimed for this slice.
- A green parent Action is not evidence for this new SHA. The backup branch remains untouched.

## Exact first action for the next GPT

1. Fetch live `main` and inspect every push Action for this exact checkpoint SHA. Fix any concrete failure before treating the PPP launch-mode slice as complete; while unrelated checks are pending, continue independent work.
2. Fix the IPv6 source-contract ordering failure separately: in `scripts/check_ipv6_service_type_status.py`, make the `update_hosts_router()` contract select the later IPv6 hosts-emission branch after `dnsmasq_file_open(hostname_temp)`, while retaining the earlier checked LAN-IPv6 read/error gate before staging. Do not reorder valid production code merely to satisfy the checker.
3. Continue the broader Padavan userland stability/security rotation. Do not reopen completed LAN/WAN IPv6 work without new failure evidence.
4. XY-C1 L3 is verified only when Build, TRX validation and artifact upload all PASS on the same source SHA. Do not claim L4 without router hardware verification.
