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
- Live main at the start of this slice: `8a2f50bedb44fdff0cb8b322372090fc190d978c`, commit `test(wan): finish checked renewal state contracts`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- On exact SHA `8a2f50bedb44fdff0cb8b322372090fc190d978c`: AI handoff docs `#44896283589` PASS; WAN route status `#44896283559` PASS; HTTPD Security inherited from `728468e145d625164bf2a6aaba6883b183cb20c2` `#44896094278` PASS. WAN DHCP callback `#44896283833` FAIL in the later static IPTV checker because it still demanded manual `strlen()` bounds checks after the source switched to `rc_wan_nvram_copy_value(..., destination, sizeof(destination))`, whose shared helper rejects truncation with `ENAMETOOLONG`. This next commit updates the test contract to the actual bounded-copy API; it does not weaken the bounds guarantee.
- WAN physical/IPTV renewal source uses checked status reads and `dhcp_wan_bound_retry_decide()`; WAN route checker validates checked IPTV mode and copied settings; HTTPD WAN route-option checker validates three route option reads/validations/comparisons.
- Exact SHA `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6`: most independent services passed (Process lifecycle inventory, NVRAM, MiniUPnP, Process lifecycle, USB modem identity, Firewall startup, Wireless, watchdog, detect_link/internet, WAN ARP and teardown); results remain scoped to that exact source SHA.
- XY-C1 L3 run `#44895865716` for `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6` was still IN_PROGRESS at last poll. Prior proven L3 baseline: `20e139d230b73eabc5f25a2b17d56a3a5768af11` / `#44887656164`. Current-source L3 and physical-router L4 NOT VERIFIED.

## Active task
- Close remaining WAN DHCP callback contracts after the source changes to checked NVRAM retry-state reads and bounded cache copies. Current failing assertion is the IPTV static settings size check in the embedded WAN DHCP workflow. The shared copy helper guarantees `length < destination_size` and returns `ENAMETOOLONG`; contracts should assert the checked copy API and its error return.
- Once exact-head WAN DHCP, WAN route, HTTPD and AI docs regressions converge, rotate to a distinct P1 reliability/security issue elsewhere in Padavan userland. Continue reviewing valid unmerged work from legacy PR #1 only by porting to current main; never use its branch as a validation target.

## Verification boundary
- No local checkout/runtime available; no local C/Python tests, sanitizer binary or firmware build is claimed.
- Remote exact-head Actions are the regression evidence. Host validation is L2. XY-C1 Build + TRX + artifact on one exact production-source SHA is L3; hardware/router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head WAN DHCP callback, AI docs, WAN route and HTTPD Actions after the checked-copy assertion update.
2. Diagnose any new failure at its first failing step; do not infer later steps passed if an earlier source checker failed.
3. Revisit the XY-C1 run #44895865716 and current-source target verification; then proceed to a distinct userland P1 issue.
4. Continue legacy PR #1 preservation review only after the active WAN validation gate is closed.

Before context exhaustion, checkpoint every material active-task, exact-SHA and validation-boundary change; do not create a no-op commit when this handoff remains accurate.
