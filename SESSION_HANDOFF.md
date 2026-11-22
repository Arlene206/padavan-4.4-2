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
- Parent of the current PPP integration correction: `d3805365ca95a17d5bd54736f29d5627580fa305`; the current source checkpoint must be obtained from live `main` after this atomic commit.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable/read-only.
- Branch inventory remains `main` plus the frozen backup only. Legacy PR #1 is closed/unmerged; do not merge it wholesale.
- WAN PPP mode selection uses one checked NVRAM transaction for `pppoemwan_enable` and, only for APP_RPL2TP L2TP, `wan_l2tpd`. One immutable selector result drives lifecycle lock selection, daemon stop/start, multi-WAN launch and rollback.
- `safe_start_xl2tpd_locked()` already snapshots VPN server/client selectors and credentials before process scan/config truncation.
- Root records remain `SESSION_HANDOFF.md` and `dev_log.md`; do not repeat startup audits or create snapshot archives.

## Active task

The parent commit `d3805365ca95a17d5bd54736f29d5627580fa305` exposed five dependent contract failures and one true production defect in the PPP launch-mode slice. The APP_RPL2TP start path still referenced a removed local selector, while the stop path used the new checked mode field; the correction switches startup to the same immutable `launch_mode->use_rpl2tp`. The WAN DHCP, VPN lifecycle, WAN route, HTTPD and XY-C1 source contracts have been updated for the new worker signature and selector model. The WAN PPP option checker now correctly inspects pointer-member usage inside the locked worker.

## Verification boundary

- Local C/Python tests were not run in this connector-only session.
- On parent `d3805365ca95a17d5bd54736f29d5627580fa305`: AI handoff #45030597515 PASS; process lifecycle inventory #45030597384 PASS; NVRAM transaction audit #45030597487 was pending at the last check.
- On parent `d3805365ca95a17d5bd54736f29d5627580fa305`: WAN DHCP #45030597508 failed at the WAN interface-name checker because it indexed the removed two-argument worker signature; VPN lifecycle #45030597433 failed on the same stale signature in its caller contract; WAN route #45030597395 failed because the new PPP mode source-contract asserted the wrong member syntax; HTTPD #45030597411 failed in the embedded L2TP contract which expected the old signature and deleted direct NVRAM selector; XY-C1 #45030597436 failed in the modem WAN checker before the cross-build step because of its stale signature.
- On that parent the PPP WAN option regression binary compiled and passed with GNU89, ASan and UBSan. No full firmware build, TRX validation, artifact upload or hardware result is claimed.
- The backup branch remains untouched. A green parent Action is not evidence for this corrective SHA.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head WAN DHCP, VPN lifecycle, WAN route, HTTPD, XY-C1, AI handoff and NVRAM transaction Actions for the correction commit. Fix any concrete failure; treat PENDING as non-blocking.
2. Resolve the independent IPv6 source-contract failure from exact parent run #45030079720: in `scripts/check_ipv6_service_type_status.py`, select the later `update_hosts_router()` IPv6 hosts-emission conditional after `dnsmasq_file_open(hostname_temp)`, while keeping the earlier checked LAN IPv6-address read/error gate before staging. Do not change valid production code for a checker-only failure.
3. Continue broader Padavan userland stability/security work; do not reopen completed LAN/WAN IPv6 product paths without new evidence.
4. XY-C1 L3 requires Build, TRX validation and artifact upload all PASS on the same source SHA. Do not claim L4 without router hardware verification.
