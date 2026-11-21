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
- Parent main for the iNIC recovery-status slice: `6dd282fc7112b86ccdb167100e44765fbca97b73`; WAN bounded NVRAM-copy regression was closed on exact SHA `6dd282fc7112b86ccdb167100e44765fbca97b73` (WAN DHCP callback `#44896408504` PASS; AI handoff docs `#44896408521` PASS). WAN route status `#44896283559` PASS on the prior source-equivalent SHA `8a2f50bedb44fdff0cb8b322372090fc190d978c`; HTTPD security `#44896094278` PASS on `728468e145d625164bf2a6aaba6883b183cb20c2`.
- iNIC MII health monitor source had a concrete status-loss defect: `check_inic_mii_rebooted()` discarded the `doSystem(iwpriv ... RadioOn=0)` failure when configuration disabled the RT3352 radio. Guest-AP recovery did log failure but the void callback could not return status. This slice makes both branches return a normalized negative error, changes the callback prototype to `int`, and has `inicd_main()` log failed poll work while continuing periodic retries.
- Relevant workflow paths already include the changed source: iNIC `.github/workflows/inicd-signal-stability.yml` watches `inicd.c`, `net_wifi.c`, `rc.h`, and the checker; Wireless radio state watches `net_wifi.c`, `rc.h`, and `tests/wireless_radio_policy.c`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- XY-C1 run `#44895865716` is still IN_PROGRESS on source SHA `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6` at the latest poll; this does not validate the later main. Prior L3 source baseline `20e139d230b73eabc5f25a2b17d56a3a5768af11` / `#44887656164`; current-source L3 and hardware L4 NOT VERIFIED.

## Active task
- Verify the iNIC force-disable and guest-AP repair status propagation: the exact-head iNIC workflow must run the GNU89/Werror/ASan/UBSan signal policy test and source checker; Wireless radio state must run its radio policy test and updated embedded callback contract.
- On FAIL inspect the first failing job step and preserve the source contract; do not normalize a failed check to success. PENDING does not block work on independent modules.
- After iNIC status propagation closes, select a distinct P1 reliability/security task outside already heavily worked WAN/HTTPD/SkipDB modules. Legacy PR #1's HMAC-SHA1 and CA bootstrap work is already present on current main; no re-import is needed.

## Verification boundary
- No local checkout/runtime available; no local C/Python tests, sanitizer run or firmware build is claimed.
- Current Actions provide L2 evidence only. XY-C1 Build + TRX + artifact on one exact production-source SHA is required for L3; hardware/router L4 NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head iNIC signal and Wireless radio state Actions.
2. If either fails, inspect its first causal failure and correct the source/test/checker integration.
3. Revisit XY-C1 run `#44895865716` and check Build, TRX and artifact steps; do not infer completion from an old SHA.
4. After iNIC closes, choose another concrete P1 reliability/security defect in a less-worked userland subsystem.

Before context exhaustion, checkpoint changed exact-head results and first action only when next-session-critical facts materially change; no no-op commit.
