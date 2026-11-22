# Cross-session execution handoff

## Mandatory operating rules

Continue as the same repository maintainer. Recover live state once, then implement and verify. Prefer a concrete stability/security defect over planning or documentation reorganization.

- Work directly on `main`. Never force-push, never modify the frozen backup branch, and do not use side branches or PRs.
- For each logical change: edit source + regression test + source contract, create all blobs, one complete Git tree and one atomic commit, then update `main` with `force:false` and `expected_sha` equal to the fetched parent.
- Inspect Actions for the exact product-source SHA. PASS permits the next task; FAIL requires fixing the specific failing source/test contract; PENDING means move to independent work.
- Do not claim local testing: this connector-only session did not have a checkout. CI outputs are the authoritative test evidence.
- Update this handoff only when the live checkpoint or next task materially changes; no no-op commits.

## Live checkpoint

- Repository: `Arlene206/padavan-4.4-2`.
- Source checkpoint immediately before this handoff-only commit: `d8fd7b6d9792d35503125145c0ad7ebedfdb08cf`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807 (last observed; direct branch lookup to re-confirm)`. Do not modify it.
- Current source presence checks:
- WAN IPv6 startup address/gateway tuple snapshot in `wan6_up()`: present in live source (SIT: ipaddr/gateway/addr6; native static: addr6/gate6).
- OpenVPN IPv6 endpoint checked status API and fail-closed export path: not found in net_ppp.c; verify net_wan6.c during next task.
- DHCPv6 LAN IPv6 address lookup/publication errno propagation: present in live `net_wan6.c`.
- LAN reset snapshot / RA / DHCPv6 mode checked reads live in `net_lan6.c` (verify helper bodies before altering).
- xl2tpd VPN selector/credential transaction snapshot: present in live `net_ppp.c`.
- WAN PPP `pppoemwan_enable` / `wan_l2tpd` launch-mode snapshot: not present in live `net_ppp.c`.

## Exact-head Actions at source checkpoint

- #45029919205 NVRAM transaction boundary audit: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919238 rstats persistence and signal lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919258 DNS Forwarder launch path stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919144 VPN server lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919055 Process inventory and lifecycle fail-closed regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029918995 detect_internet signal lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919071 WAN DHCP callback state stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919084 detect_link signal lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919107 USB modem WDM/NDIS physical identity stability: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919097 infosvr packet and signal lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919001 WAN interface teardown stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919046 watchdog signal lifecycle stability regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919114 HTTPD Authentication and Security Regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919006 Samba share configuration stability regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919136 GPIO button daemon startup and press timing stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029918985 Firewall Startup Stability Regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919129 iNIC signal lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919112 Process lifecycle inventory - d8fd7b6d9792d35503125145c0ad7ebedfdb08cf: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919219 WAN gateway ARP probe socket stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029918967 DDNSTO lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919140 ZeroTier lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919145 Inadyn fail-closed lifecycle regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919165 Crond daemon lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919139 NFS daemon start and reload lifecycle regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029918983 OpenVPN client credential staging regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029918980 USB storage hotplug stability regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919096 miniupnpd atomic configuration publication regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919131 FRP asynchronous startup lifecycle regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029918976 vsftpd configuration publication stability regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919199 WAN route operation status regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919193 HTTPD daemon lifecycle stability: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919209 Wireless radio state regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919204 PID 1 notification queue stability regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919135 AdGuardHome config security regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029918981 IPv6 service-type checked-read regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919106 Router mode NVRAM update failure regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919047 Adbyby download and dnsmasq configuration stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919186 networkmap signal lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919090 DNSMasq lifecycle stability regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919195 AliDDNS signed API transport security regression: queued/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919143 AliyunDrive WebDAV lifecycle stability regression: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919048 AI handoff documentation contract: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)
- #45029919092 XY-C1 Firmware Validation: in_progress/no conclusion (exact head d8fd7b6d9792d35503125145c0ad7ebedfdb08cf)

Runs from earlier SHAs are not evidence for this source checkpoint. Fetch exact-head Actions again after the handoff commit, because this document-only commit creates a new SHA.

## Recent implementation trail

- LAN IPv6 reset mode/address/prefix now uses a checked NVRAM transaction and aborts before publishing `lan_addr6` if any read or transaction close fails; legacy missing/malformed defaults are retained. See commits `e45027e842af2c42de6760991fbe9d80fa21967c` and `75a61dfe4f48664a6a35caa0e0f1df6243fe48a8`.
- `is_lan_radv_on()` now returns unknown (`-1`) on an unreadable `ip6_lan_radv` value while preserving the legacy empty-value enabled default. See `e3cc86e9503b978ddbe2ab49ff498db81076da54`; source-checker variable correction: `d7b9063ffd43613534909b7707783c7b9f967d12`.
- `get_lan_dhcp6s_mode()` now uses a checked read and preserves `nvram_get_int()/atoi()` defaults for clean missing or malformed values. See `36345cb4f016e07a9b621aa9e7161321e8dbe836`.
- WAN IPv6 interface identity and address snapshots, SIT preflight, DNS read-error handling, DHCPv6 callback error propagation and OpenVPN WAN IPv6 endpoint error handling are on main; see `129697133e29a7757adeff9c61e590b825aba915`, `db11b15bb545dbf9dafff9c4795600735884e0dc`, and current `dev_log.md`.
- `safe_start_xl2tpd_locked()` takes a checked selector/credential snapshot before process scan and config truncation. Source-policy checker was corrected to inspect the actual transaction adapter wrappers. See `dcda9e5a118711d1b35b8e347e5fde54ccdc8270`, `ba5aaa320963ba7a0322effe5388e1e5b09d6166`, and `a04c9aa07f0339146af954e688f39812fbb3b400`. Exact VPN lifecycle and AI handoff runs on `a04c9aa...` were observed PASS.

## First task for the next GPT

1. Re-fetch live `main`, backup ref, and exact-head Actions. Check the most recent workflow run for VPN server lifecycle and determine whether the WAN PPP launch-mode slice is present in the source checkpoint.
2. If `ppp_wan_launch_mode_snapshot_read()` is present, inspect its exact-SHA VPN workflow. Fix any real compile/source-contract failure, then review the remaining `net_ppp.c` unchecked selectors. If it is absent, implement a two-key checked NVRAM snapshot for `wan_l2tpd` and `pppoemwan_enable`, use the same immutable flags for stop/start/rollback, and add GNU89 policy regressions.
3. Continue the broader userland stability/security rotation after exact-SHA verification. Do not re-open completed WAN6/LAN6 tasks without new evidence.
4. XY-C1 L3 is verified only when Build, TRX validation and artifact upload all PASS on the same source SHA. No hardware L4 claim without device verification.

## Verification caveats

- Local C/Python test suites were not run in this connector-only session.
- A green Actions run on a parent SHA is not a green run for a later commit.
- The backup branch is immutable. PR #1 remains a closed legacy ref; do not merge wholesale.
