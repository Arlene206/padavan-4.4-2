# Cross-session execution mandate

Recover live repository state once, then continue the active engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task: create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.
Before context exhaustion, record exact refs, Action states and the first next action.
Only edit it when the live facts or first next action change materially.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`.
- Current main parent for the Inadyn manual-refresh identity migration: `eee736ad62722846db11e035613a26a8433ffc12`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Exact 8f550 whole-tree lifecycle inventory #43776729995 PASS: 9,501 tracked entries, 6,255 scanned text files, 0 missing candidates / read errors; artifact #14186597970. It identified 27 actual legacy `pids()` consumers and no in-tree callers of `is_dns_dhcpd_run()`.
- AiDisk Samba/FTP off-path fix commit `891b46b51481d171aab6de88ac6634f4d6f56124`: exact Process lifecycle #43777935786, inventory #43777935804, AI docs #43777935812, HTTPD security #43777935793, AdGuardHome #43777935745, infosvr #43777935794, AliDDNS #43777935756, AliyunDrive #43777935761 and NVRAM #43777935755 PASS. XY-C1 #43777935841 was still building at last poll; TRX/artifact pending.
- SmartDNS commit `c679c2eeaad06c44410ce38ae051a09bd5f30a19` had Process regression #43778456682 FAIL due to a stale checker path assertion. Follow-up `eee736ad62722846db11e035613a26a8433ffc12` fixed the assertion; exact eee Process #43778610041 and AI docs #43778609888 PASS. XY-C1 #43778456640 was still building at last poll; it includes c679 SmartDNS source and 891b AiDisk source.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No physical-router/L4 validation.

## Active task

### Route manual DDNS refresh through PID-identity-bound signaling

The HTTPD `ddnsclient` action currently uses `pids("inadyn")` and then `killall -SIGUSR1 inadyn`. This loses UNKNOWN process-inventory errors and sends a name-wide signal instead of signaling only identities captured from procfs. The RC already has an identity-validating Inadyn signal policy for SIGHUP; this change extends it to SIGUSR1, adds a dedicated RC event and a helper that treats ABSENT as a no-op but fails closed on UNKNOWN, validates captured identities and signals only those PIDs. GNU89/ASan/UBSan host coverage and integration assertions cover HTTPD→notify_rc→RC→signal policy. Exact-head Inadyn, Process, AI-doc and XY-C1 Build/TRX/artifact results are pending.

## Verification boundary

- Exact 8f550 inventory #43776729995 and AiDisk parent validations as above. Exact 891b Process #43777935786 and AI docs #43777935812 pass; XY-C1 #43777935841 was still in Build at the last poll.
- SmartDNS checker failure on c679 was corrected by eee; exact eee Process #43778610041 and AI docs #43778609888 pass.
- THIS COMMIT changes HTTPD / RC product source, Inadyn signal policy/test, event header, two workflow filters, checker and live docs. Exact-head Inadyn lifecycle, Process lifecycle, AI docs and XY-C1 Build/TRX/artifact must be checked. No L3/L4 is claimed unless current-SHA firmware Build, TRX and artifact upload all pass.

## Exact first action for the next GPT

1. Inspect exact-head Inadyn lifecycle, Process regression, AI handoff-doc and XY-C1 Actions for THIS COMMIT. Confirm SIGUSR1 policy compile/test, HTTPD→RC source contracts and same-SHA Build XY-C1 / TRX / artifact upload.
2. Remove the now-unused `SET_AIDISK_STATUS_SUCCESS:` label from `ej_set_AiDisk_status()` and adjust the AiDisk integration checker to delimit the function at `ej_get_all_accounts()`; verify no unused-label warning or firmware failure.
3. Continue reviewing remaining real `pids()` consumers and name-wide signals from the 8f550 inventory artifact; do not rewrite legacy `pids()` globally. No L4 without hardware.
