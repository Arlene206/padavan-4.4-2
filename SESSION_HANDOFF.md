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
- Parent main for this checker-correction slice: `215892b1acde4eea1be4be7785f4ef2b2b197e55`. The iNIC production fix commit `215892b1acde4eea1be4be7785f4ef2b2b197e55` changes `check_inic_mii_rebooted()` to return normalized command failure for forced-off and guest-AP recovery, and has `inicd_main()` log failure while retrying at the next poll.
- Exact-head Wireless radio state `#44896853653` FAIL at its embedded source checker line 566. The checker expected `"int check_inic_mii_rebooted(void)"` as a same-line string, but source formatting correctly puts `int` on one line and the function name on the next. C policy tests and bridge/guest-LAN tests passed before this whitespace-only assertion. This follow-up normalizes both iNIC and Wireless checker declaration tests to the actual multiline source signature.
- Exact-head iNIC signal workflow `#44896853546` was IN_PROGRESS (job `136824261395`) at last poll. Wireless run #44896853653 used job `136824261621`.
- Additional exact SHA `215892b1acde4eea1be4be7785f4ef2b2b197e55`: XY-C1 `#44896853565` IN_PROGRESS; WAN route `#44896853617` PASS; USB modem `#44896853632` PASS; infosvr `#44896853650` PASS; vsftpd `#44896853664` PASS; VPN server `#44896853652` PASS; Networkmap `#44896853531` PASS; MiniUPnP `#44896853553` PASS; Samba `#44896853473` PASS; Adbyby `#44896853659` PASS. Remaining exact-head runs are queued/in progress and must not be represented as passed.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable. Prior XY-C1 L3 source `20e139d230b73eabc5f25a2b17d56a3a5768af11` / run `#44887656164`; no L3 claim for current production source yet; hardware L4 NOT VERIFIED.

## Active task
- Re-run exact-head Wireless radio state and iNIC signal source workflows after the multiline declaration assertion correction. Confirm the iNIC callback source checker verifies the disabled-radio force-off error gate, guest-up recovery failure gate, normalized negative return, and daemon poll logging/continued retry.
- Continue inspecting exact-head CI failures in order; pending unrelated Actions do not block independent implementation.
- After iNIC checks converge, select another concrete P1 reliability/security defect in a less-worked userland subsystem; do not return to WAN/HTTPD unless new regression evidence justifies it. Legacy PR #1 HMAC-SHA1 and CA bootstrap functions are already preserved in current main.

## Verification boundary
- No local checkout/runtime available; no local C/Python tests, sanitizer run or firmware build is claimed.
- This exact-source commit's GitHub Actions are the L2 verification gate. XY-C1 Build + TRX + artifact on one exact production-source SHA is required for L3; hardware/router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head iNIC signal + Wireless radio state Actions after the signature checker correction.
2. Inspect first failing step and fix causal checker/source issue without weakening coverage.
3. Check XY-C1 Build/TRX/artifact completion for `#44896853565`; otherwise launch/verify a fresh source-matching XY-C1 if needed.
4. Once these checks converge, advance to another specific P1 userland stability/security bug.

Before context exhaustion, checkpoint current exact-head status and first actionable failure; no no-op commit.
