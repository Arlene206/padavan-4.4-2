# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; only authorized development ref is `main`.
- Main at the start of this correction: `de6799e468caa60bfcd086b263e82bb149efd362`. Refetch live main before every write and use a non-forced expected-SHA update.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only. Startup branch/backup comparison was already performed once; do not repeat it.
- Startup governance recovery is complete. Root `dev_log.md` is restored to 21 records (this correction plus the newest 20 previous records); 100 older complete records are preserved in indexed `part-012.md`.
- PR #1 (`fix/aliddns-ca-bundle-bootstrap`, head `ba48e22609e0c81ad757b06756fe91d05f39cbf6`) is CLOSED and unmerged. HMAC helper source/Makefile, HMAC test, CA bootstrap test and transport fixture contents already match main; current main's package, launcher, workflow and checker have evolved and retain the functionality. No stale branch cherry-pick is needed.

## Engineering baseline

- LAN bridge/VLAN/switch required startup errors and required iNIC MII post-load verification propagate into PID 1; optional hw_nat/LED paths remain best-effort.
- Shared bridge-add/member helpers preserve ioctl errno across socket close.
- WAN startup includes required physical interface IFUP failure, static WAN route/service setup, and IPv6 tunnel errors. Required IPv6 controls on the VLAN parent, WAN interface and AP-client WISP interface now return failure before publishing WAN identity. Static route teardown and USB modem stop errors are preserved.
- The current product-source SHA is `de6799e468caa60bfcd086b263e82bb149efd362`; this correction does not alter product source.

## Active task

Repair the new IPv6 WAN source checker to include each enclosing runtime IPv6 guard when checking the control call and its fail-closed return path. Parent Process and XY-C1 runs failed only at that checker after GNU89 policy binaries passed. The AI docs parent run also detected 120 root log records; this correction losslessly rotates the oldest 100 into part-012 and leaves 21 root entries.

## Verification boundary

- Parent `de6799e468caa60bfcd086b263e82bb149efd362`: Process `43864900710` FAILED at `scripts/check_modem_wan_start_status.py:48`; service-status and modem WAN policy binaries passed first. XY-C1 `43864901114` failed at the same source checker; Build, TRX verification and artifact upload were skipped.
- AI handoff docs `43864900749` FAILED because root `dev_log.md` reached 120 dated entries; this batch moves the oldest 100 verbatim and updates the index.
- Parent successes: WAN route `43864900904`, WAN DHCP `43864900880`, WAN teardown `43864900911`, infosvr `43864900685`, AdGuardHome `43864901024`, AliDDNS `43864900761`, AliyunDrive `43864900934`, DNSMasq `43864900856`, miniupnpd `43864900839`, detect_link `43864900673`, detect_internet `43864900826`, WAN gateway ARP `43864900950`, USB modem identity `43864900745`, HTTPD `43864900784`, VPN server `43864900910` and Process inventory `43864900896`. NVRAM `43864900907` remained IN_PROGRESS at last inspection.
- No same-SHA firmware Build/TRX/artifact L3 or physical-router L4 result exists for the IPv6-control source.

## Exact first action for the next GPT

1. Inspect this correction's exact-head Process, AI handoff docs and XY-C1 Actions. Diagnose any remaining failure at its first causal step.
2. Once the source checker passes, continue the WAN startup failure-propagation slice: PPP interface migration/UPnP stop, PPTP/L2TP physical setup, PPPD launch, demand PPP readiness, static IP configuration, EAPOL startup and DHCP client launch failures must reach `start_wan()` status when required.
3. Preserve first-error semantics if best-effort cleanup also fails. Do not make optional MTU tuning or optional UPnP notification errors fatal without a verified invariant.
4. Claim firmware L3 only after Build, TRX verification and artifact upload all pass on one exact source SHA; physical-router L4 remains unverified.

## Governance

Main only; frozen backup read-only. One full tree and atomic commit per logical task, then non-forced expected-SHA update. Update `dev_log.md` for source changes and preserve archive history. Distinguish host tests, exact-SHA Actions, firmware/TRX L3 and physical-router L4. Never claim an unrun result.
