# 2026-11-18 continuation checkpoint — SmartDNS autostart source-contract correction

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, development branch `main`. Live main immediately before this checkpoint: `e0c8269b265618aa49ba9ce73796af9ac87458dc`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged; useful AliDDNS CA/HMAC/bootstrap changes are already present in main. Do not mutate the backup, PR #1 or GPT Session Rotator.
- SmartDNS production source SHA remains `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`; later SmartDNS commits are tests/checkers/documentation only.

## Active task
- The previous commit `b2c45feae66a367b1752628d3e20ed3d55a4beb1` corrected the optional autostart shell fixture to model an invalid saved-state failure (status 45), preserve byte-identical DNSMasq and SmartDNS state, log the error and continue independent DDNSTO/FRP startups.
- Its exact Process run `44322259809` failed before the shell fixture ran because `scripts/check_smartdns_lifecycle.py` read the SmartDNS process-lifecycle test into the variable `test`, but the new autostart assertions were mistakenly checked against that unrelated file. THIS COMMIT adds a separate `autostart_test` input and redirects only those assertions to it; the process-state regression assertions continue to use `test`.
- This is test/checker-only; no production SmartDNS or autostart runtime behavior is modified.

## Verification boundary
- Process run `44322259809` on `b2c45feae66a367b1752628d3e20ed3d55a4beb1`: process inventory PASS; SmartDNS lifecycle shell suite PASS; Python checker failed at the first misplaced autostart assertion (`SMARTDNS_STATUS=45` checked against the SmartDNS-specific test file). The corrected source contract in THIS COMMIT requires a new exact-head Process run.
- Process lifecycle inventory `44322259752` and AI documentation `44322259670` on `b2c45feae66a367b1752628d3e20ed3d55a4beb1`: PASS.
- On parent source `a36da98521b1d2fc3b7641f4cd98f73d103d89e7`: WAN route `44322134319`, WAN gateway ARP `44322134155`, VPN `44322134514`, detect_internet `44322134501`, detect_link `44322134278`, Wireless radio `44322134471`, HTTPD security `44322134417`, miniupnpd config publication `44322134271`, ZeroTier `44322134124`, USB modem `44322134111` and DNSMasq `44322134107` all PASS. XY-C1 `44322134680` and NVRAM audit `44322134384` were in progress at last observation.
- vsftpd `44320651576`, Infosvr `44320600854`, Crond `44320518213`, Networkmap `44320518215`, Process inventory `44319662359`, NVRAM audit `44319662342`, USB storage `44319662199`, Samba `44319662327`, and DNSMasq `44321070200` completed PASS on their exact respective SHAs.
- XY-C1 run `44318683585` on exact SmartDNS production SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c` passed Build, TRX verification and artifact upload (L3 verified for that source). BusyBox/uClibc live runtime, live DNS traffic and physical-router L4 remain NOT VERIFIED.
- No local checkout-based shell execution was available; GitHub Actions are the observed test evidence.

## Exact first action for the next GPT
1. Fetch live main and exact-head Actions; inspect the new Process run from THIS COMMIT. If it passes, continue the broad stability/security rotation; if it fails, diagnose the first causal failure.
2. Continue from `REFACTOR_ROADMAP.md`, preferring unclosed P1 WAN/firewall/network stability work over another SmartDNS audit.
3. Check current XY-C1/NVRAM runs on `a36da985` against their exact SHA and update L3 only where the full Build/TRX/artifact chain passed.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. If the active task, verification boundary and first action remain accurate, leave this handoff untouched.

## Cross-session execution mandate
One logical slice is one atomic main commit. Finish bounded source/test work, create one complete Git tree and commit, then update main with non-forced expected-SHA compare-and-swap. Continue independent work while relevant Actions are pending. Before context exhaustion, leave the next GPT the exact first action, commit SHA, tests and CI state. Never claim unobserved local tests, target builds or physical-device validation.
