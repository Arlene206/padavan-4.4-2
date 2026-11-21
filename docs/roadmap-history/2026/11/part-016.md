# Padavan 4.4 — Whole-Repository Modernization Roadmap

> Current dashboard only. Historical snapshots are preserved byte-for-byte in the roadmap history index. A bounded fix does not prove module completion.

## Current source / CI checkpoint

- Repository: Arlene206/padavan-4.4-2 (maintained fork).
- Main: 2a5911b24548ac2a1ef06442e0b61771a2714733. Recent source commits: AP DHCP validation b96fa1e; fail-closed gateway route cleanup 2332158; Optware lifecycle serialization / safe-eject integration 2c8b5ff; storage shell-test correction 2a5911b.
- Exact latest-head CI: USB storage hotplug [42568851009](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568851009) PASS and AI handoff contract [42568850779](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568850779) PASS. HTTPD/security [42568701873](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701873) PASS on the immediately preceding production-source head 2c8b5ff.
- Validation ceiling: source review and host/Actions regressions only. No full production RC/BusyBox build, MIPS/uClibc cross-build, firmware/TRX or live hardware validation is claimed.

## Whole-repository module status

| Module | Status | Verified bounded work | Open reliability / security work |
|---|---|---|---|
| LAN DHCP client / AP-mode lease | PARTIAL / AUDITING | Required IPv4/netmask validation; bounded router/DNS/WINS/domain/lease normalization; withdrawn options clear prior values; changed default-gateway deletion precedes lease publication; ESRCH accepted, other errors fail closed. Exact-head policy + source contracts pass. | deconfig, leasefail, NAK semantics; lan_down_auto / lan_up_auto route, interface and resolver error propagation; partial callback rollback and last-known-good behavior. |
| WAN DHCP / route lifecycle | PARTIAL / AUDITING | Lease option validation, route-text safety, default-route refresh and route operation error propagation have bounded regressions. | Full event/callback review across renew/deconfig/leasefail/NAK, multi-WAN ownership and callback lifetime. |
| DNSMasq / resolver / DHCP files | PARTIAL / AUDITING | Safe staged publication, no-follow lease file validation, domain label validation, resolver refresh failure contract, duplicate restart guard and exact-head trust-boundary checks. | Daemon lifecycle, option ownership and restart/stop failure propagation. |
| USB / storage / hotplug / filesystem | PARTIAL / AUDITING | Complete mount inventory before side effects; swapoff-before-unmount; failure propagation; shared storage.lock across mdev, safe-remove and eject; Optware lifecycle flock for start/stop scripts; deferred startup retries preserve pending flags; bounded in-tree caller review. | External Optware init script internals/background descendants; lock-order inversion potential with user-supplied scripts; verify hook stop/unmount error propagation; broader caller graph CI draft not yet integrated; filesystem/runtime/churn/hardware. |
| Firewall / NAT / Netfilter | AUDITING | Firewall restore transaction failure handling, IPv6 table setup and bounded FULLCONENAT worker/notifier registration error handling have source contracts. | FULLCONENAT mapping hashes and dying-tuple queue ownership across network namespaces; keep separate kernel work. |
| Wireless / Wi-Fi drivers | PARTIAL / AUDITING | Ralink ioctl handling, scan bounds, restart config preflight, radio-state publication after successful driver calls and selected AP/APCLI/WDS bridge gates. | Remaining wif_control start/stop consumers, AP/APCLI/WDS/iNIC lifecycle, concurrency, board branches and live radio behavior. |
| HTTPD / CGI / authentication | PARTIAL / AUDITING | Existing auth, disclosure, safe publication, NVRAM restore and CGI policy regressions pass recent exact-head Actions. | Whole route/input/error inventory, upload/write boundaries, authorization and session ownership. |
| NVRAM / persistence / startup | PARTIAL / AUDITING | Atomic publication, checked persistence, restore failure gates and recovery-mode contracts have bounded coverage. | Remaining direct callers, flash failure recovery, board/partition edge cases. |
| Switch / PHY / MDIO / link state | PARTIAL / AUDITING | Selected MTK/RTL ioctl status propagation, PVID/link retry, CLI argument order and bounded MDIO timeout checks. | Other board drivers, switch status consumers and live hardware behavior. |
| NFS / Samba / FTP / accounts / add-ons | AUDITING | NFS complete mount-inventory parsing and selected Samba/account trust-boundary coverage. | Permissions, command injection, service failure, path traversal and external package lifecycle. |
| Init / PID 1 / service supervision | AUDITING | Notification deferral, child reaping, service termination waits and selected start/stop failure gates. | Cross-module service ordering, shutdown races and package supervision. |

## Priority queue

1. P1/P2 — LAN DHCP deconfig correctness. Trace BusyBox udhcpc event producers and AP deconfig -> lan_down_auto -> route/resolver/interface cleanup. Establish error semantics and preserve last-known-good lease state; review leasefail and NAK behavior separately.
2. P2 — Optware/storage eject completion. Audit lock order with custom init scripts; confirm opt-umount service stop and /opt unmount failure propagation; verify no delayed start can touch a device after safe removal.
3. P2 — Storage caller-graph regression. Re-read current sources and integrate the drafted Python in-tree caller contract into storage-hotplug-stability.yml; include storage_mountpoint_lock.sh and run exact-head Actions.
4. P2 — Cross-module rotation. After the above bounded reliability tasks, move to a different userland area such as FTP/Samba/account or add-on service lifecycle. Do not get stuck in SkipDBv2, HTTPD, storage or DHCP.
5. Separate kernel work — FULLCONENAT mapping namespace state. Audit module-global tuple hashes / dying-tuple queues against per-net lifecycle. Do not mix kernel/MTK work into routine userland commits.

## Working rules

- Trace callers/callees, side effects, error returns and cleanup before changing behavior.
- Prefer small GNU89 host policy regressions and source contracts, but test semantic behavior rather than brittle symbol placement.
- Each path-filtered workflow must watch production source, helper, test and workflow files under push and pull_request.
- Results count only for the exact head SHA. Inspect failures; distinguish source defects from stale test locators.
- Keep dev_log.md at exactly 20 whole dated records; move only complete oldest records to indexed shards and maintain exact byte counts / Git blob SHAs.
- Preserve superseded root handoff and roadmap snapshots byte-for-byte in docs archives.
- Run scripts/check_ai_handoff_docs.py and verify the docs workflow on the final exact HEAD.

## Verification matrix

| Level | Current evidence | Limits |
|---|---|---|
| L1 source review | PASS for AP DHCP input / gateway-delete-before-publish policy and bounded SD/MMC/RC/HTTPD/libdisk/Optware caller review. | Does not establish full deconfig callback correctness, every external package script or deadlock freedom under custom Optware scripts. |
| L2 host / GitHub Actions | Storage [42568851009](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568851009) and docs [42568850779](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568850779) PASS on 2a5911b; HTTPD [42568701873](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701873) PASS on 2c8b5ff. | Caller-graph Python draft not yet included in CI. |
| L3 MIPS/uClibc / production objects / firmware/TRX | NOT VERIFIED. | Full toolchain stage required. |
| L4 live DHCP / storage churn / physical router | NOT VERIFIED. | Device integration required. |

## Handoff requirements

- Keep SESSION_HANDOFF.md as the one actionable continuation point.
- Update handoff, roadmap and dev_log in the same milestone commit as source/test changes.
- Keep the root dev log at exactly 20 complete entries; shards split only between records and remain below 512 KiB.
- Run scripts/check_ai_handoff_docs.py and exact-head documentation Actions before handing off.
- Re-fetch live main, checkpoint branches and Actions on every takeover.

