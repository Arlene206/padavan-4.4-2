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
- Parent main for this iNIC checker repair: `ca6862c97fd964d0aaa9451e9fb7116773411c8d`.
- Production fix `215892b1acde4eea1be4be7785f4ef2b2b197e55` propagates force-off and guest-AP recovery failures from `check_inic_mii_rebooted()`; `inicd_main()` logs an incomplete health check while continuing periodic retries.
- Exact SHA `07e68ffbe95b7d8954274207e4712997a3c1548a`: Wireless radio state `#44896945159` PASS; AI docs `#44896945122` PASS; iNIC `#44896945147` failed in retry-wait checker marker. Follow-up `ca6862c97fd964d0aaa9451e9fb7116773411c8d` fixed the marker but exact-head iNIC `#44897089095` still failed in the checker: it used `rc / "tests/wireless_radio_policy.c"` even though `rc` is a string containing `rc.c`, not a `Path`. This commit switches to the existing `read()` helper. The GNU89/Werror/ASan/UBSan iNIC policy executable passed before both source-contract failures.
- Exact SHA `215892b1acde4eea1be4be7785f4ef2b2b197e55`: a broad service regression set completed SUCCESS, including NFS `#44896853564`, Process lifecycle inventory `#44896853588`, Firewall `#44896853557`, Samba `#44896853473`, USB modem `#44896853632`, AliyunDrive `#44896853654`, WAN route `#44896853617`, WAN DHCP `#44896853582`, infosvr `#44896853650`, and other module checks. Treat only completed success as PASS.
- Wireless radio state `#44896945159` and AI handoff docs `#44896945122` passed on exact SHA 07e68ff. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- XY-C1 run `#44896853565` for source SHA `215892b1acde4eea1be4be7785f4ef2b2b197e55` was IN_PROGRESS at last poll; no L3 claim. Prior L3 baseline `20e139d230b73eabc5f25a2b17d56a3a5768af11` / `#44887656164`; hardware L4 NOT VERIFIED.

## Active task
- Re-run exact-head iNIC signal action after replacing the incorrect string-as-Path test reference. Ensure it passes both sanitizer policy binary and source checker.
- Check XY-C1 Build/TRX/artifact completion for #44896853565 and re-poll previously pending broad Actions.
- After the iNIC poll status work closes, advance to another concrete P1 reliability/security defect outside WAN/HTTPD/SkipDB.

## Verification boundary
- No local checkout/runtime available; no local test/sanitizer/firmware build is claimed.
- Remote Actions on exact SHA provide L2 evidence only. XY-C1 Build + TRX + artifact on one exact production-source SHA is required for L3; hardware/router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head iNIC Action for this correction.
2. If it fails, inspect the first source-contract failure and correct the checker; preserve explicit positive/negative system wait status tests.
3. Revisit XY-C1 run #44896853565 and inspect Build, TRX verification and artifact upload job conclusions.
4. Select another distinct P1 userland reliability/security issue once iNIC is complete.

Before context exhaustion, refresh only changed exact-head statuses and the first actionable failure; do not create a no-op commit.
