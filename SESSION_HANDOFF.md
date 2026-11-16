# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active development ref is main.
- Main baseline for this validator repair: ccca92b531d44c2476b6041a8adc3cbdec2f3a9f. Refetch current main before each write and use non-forced expected-SHA update.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only. Startup branch recovery was done once and must not be repeated.
- Root dev_log.md remains below 30 records; archive part-012.md preserves the oldest 100 entries.
- Closed unmerged PR #1 HMAC/CA feature work is already represented in main.

## Engineering baseline

- Required LAN bridge/VLAN/switch and iNIC MII startup errors propagate; optional hw_nat/LED paths remain best-effort.
- Required WAN physical IFUP, IPv6 control, MAN static address/route/gateway, PPP migration/launch/demand readiness, IPoE static address, EAPOL and DHCP failures now enter first-error startup status. PPPoE optional physical setup, MTU tuning, UPnP notifications and cleanup remain best-effort.
- wait_ppp_up() returns zero on success and specific nonzero errors on interface timeout, socket inspection and demand route preinstallation.

## Active task

This repair aligns WAN route, miniupnpd and HTTPD cross-module source contracts with the PPP status refactor and corrects check_modem_wan_start_status.py to verify errno-normalized failure_status rather than expecting an unnormalized raw interface return.

## Verification boundary

- Parent exact SHA ccca92b531d44c2476b6041a8adc3cbdec2f3a9f: Process 43867497793 failed at shared checker assertion return interface_status; XY-C1 43867497757 failed at the same checker before Build, TRX verification or artifact upload.
- WAN route 43867497676 failed because its source checker searched for a multi-line C accumulator call as a single-line string.
- miniupnpd 43867497529 failed because its contract expected the pre-refactor inline PPP/IPoE migration guards.
- HTTPD security 43867497805 failed because its PPP credential policy expected the pre-refactor inline launch_wan_pppd() and boolean demand-readiness call.
- Same-SHA successes: AI docs 43867497763, infosvr 43867497744, VPN 43867497759, WAN gateway ARP 43867497652, detect_link 43867497598, USB modem identity 43867497710, WAN DHCP callback 43867497514, DNSMasq 43867497551, process lifecycle inventory 43867497797, detect_internet 43867497877, AliDDNS 43867497543, AdGuardHome 43867497706, WAN teardown 43867497629, AliyunDrive 43867497557. NVRAM 43867497808 was still in progress.
- No successful same-SHA XY-C1 Build/TRX/artifact or hardware L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head WAN route, miniupnpd, HTTPD, Process/WAN checker and XY-C1 Actions for this repair.
2. If they pass, continue review of other required start_wan() transitions and WAN callbacks for failures still hidden behind logging.
3. Preserve first-error semantics and keep optional MTU, notifications and cleanup best-effort.
4. Claim L3 only when Build, TRX verification and artifact upload all pass for one exact source SHA. Router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
