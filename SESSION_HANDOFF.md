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
- Parent main for this iNIC checker repair: `07e68ffbe95b7d8954274207e4712997a3c1548a`.
- Production fix `215892b1acde4eea1be4be7785f4ef2b2b197e55` propagates force-off/guest-AP recovery failures from `check_inic_mii_rebooted()` and has `inicd_main()` log an incomplete health check while continuing to the next poll.
- Exact SHA `07e68ffbe95b7d8954274207e4712997a3c1548a`: Wireless radio state `#44896945159` PASS; AI handoff docs `#44896945122` PASS; iNIC signal `#44896945147` FAIL in checker line 103. Failure is checker-only: it searched for exact `if (events == 0)`, but production uses `if (events == 0 &&` to include `sigsuspend()`. The GNU89/Werror/ASan/UBSan iNIC signal test passed. This commit fixes the whitespace/operator-sensitive marker while still asserting the error log precedes the retry wait and poll errors do not break the daemon loop.
- Exact SHA `215892b1acde4eea1be4be7785f4ef2b2b197e55`: WAN route `#44896853617`, USB modem `#44896853632`, infosvr `#44896853650`, vsftpd `#44896853664`, VPN `#44896853652`, networkmap `#44896853531`, MiniUPnP `#44896853553`, Samba `#44896853473`, Adbyby `#44896853659`, NFS `#44896853564`, Firewall `#44896853557`, AI docs `#44896853658`, detect_internet `#44896853618`, DDNSTO `#44896853651`, watchdog `#44896853689`, DNS Forwarder `#44896853682`, HTTPD `#44896853708`, Crond `#44896853693`, OpenVPN `#44896853688`, PID 1 queue `#44896853685`, FRP `#44896853703`, ZeroTier `#44896853642`, DNSMasq `#44896853638`, detect_link `#44896854319`, USB storage `#44896853739`, AdGuard `#44896853573`, Inadyn `#44896853631`, process lifecycle inventory `#44896853588`, NVRAM audit `#44896853602`, HTTPD security `#44896853580`, XY-C1 `#44896853565` and several others completed or remained in progress; treat only completed successes as PASS.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, immutable.
- XY-C1 `#44896853565` on source SHA `215892b1acde4eea1be4be7785f4ef2b2b197e55` was still in progress (Build job `136824261550`); no L3 claim. Prior L3 baseline `20e139d230b73eabc5f25a2b17d56a3a5768af11` / `#44887656164`; hardware L4 NOT VERIFIED.

## Active task
- Re-run exact-head iNIC signal workflow after the retry-wait checker marker correction. Wireless state passed the production change's source contract on 07e68ff; ensure no new iNIC checker failures.
- Continue inspecting exact-head workflows whose prior results were PENDING; resolve the first causal failure without weakening assertions.
- After the iNIC poll status work closes, advance to another concrete P1 reliability/security defect outside WAN/HTTPD/SkipDB.

## Verification boundary
- No local checkout/runtime available; no local test/sanitizer/firmware build is claimed.
- Remote Actions on exact SHA provide L2 evidence. XY-C1 Build + TRX + artifact on a single production-source SHA is required for L3; hardware/router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head iNIC signal and AI docs Actions for this correction.
2. Inspect first failing step and fix causal checker/source issue without weakening coverage.
3. Check XY-C1 Build/TRX/artifact completion for `#44896853565`; otherwise run a fresh production-source-targeted validation.
4. Once these checks converge, advance to another specific P1 userland stability/security bug.

Before context exhaustion, checkpoint current exact-head status and first actionable failure; do not create a no-op commit.
