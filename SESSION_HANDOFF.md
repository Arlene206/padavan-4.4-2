# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Historical handoffs are preserved byte-for-byte and indexed. Always fetch live main and exact-head Actions before relying on a SHA.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; maintained fork only.
- Current main: 2a5911b24548ac2a1ef06442e0b61771a2714733 — test(storage): source Optware lock helper in regression (2026-11-09 05:42:32Z).
- Remote branches: only main was present at last inventory; no separate checkpoint branch. Recheck at next takeover.
- Latest exact-head Actions: USB storage hotplug stability [42568851009](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568851009) SUCCESS and AI handoff documentation contract [42568850779](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568850779), both on exact HEAD 2a5911b24548ac2a1ef06442e0b61771a2714733.
- HTTPD Authentication and Security Regression [42568701873](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701873) SUCCESS on 2c8b5ff9797cbb6a0abf99c5fa18210d09df6671, the immediately preceding production-source commit. The latest 2a5911 change only fixes the storage shell test's helper sourcing.
- Storage run 42568701858 on 2c8b5ff failed in the new shell test because the test invoked optware_lifecycle_lock_acquire before sourcing the helper. Commit 2a5911 adds the missing source; exact-head storage run 42568851009 is SUCCESS.
- Validation scope: source review and host/Actions regressions only. No production MIPS/uClibc build, full firmware/TRX, live lease churn, live removable storage churn or hardware validation has been performed.

## Active task

LAN DHCP lifecycle reliability — AP-mode lease validation and changed-gateway route cleanup have bounded implementation and exact-head regression. Continue the deconfig / leasefail / NAK error paths and last-known-good state guarantees; keep the module PARTIAL / AUDITING.

### DHCP work already completed

- b96fa1ee123d45bd15d2e90ff6e4a183ecda0fd0 hardened AP-mode LAN DHCP bound/renew input validation, option normalization, withdrawn router/DNS/WINS/domain clearing and bounded decimal lease validation.
- 23321588f38c3ea9a28355ce7ccc50feb06ef1d8 now removes a changed previous default gateway before publishing any new lease NVRAM fields. ESRCH is accepted as already absent; other deletion errors are returned and leave old lease fields intact. udhcpc_lan_main rejects a null/empty interface name before dispatching callbacks.
- The DHCP policy regression covers successful delete, ESRCH and EIO. Workflow source checks verify route-delete failure return precedes the first new lease field publication and the interface guard.
- The first HTTPD run on b96fa1e failed only because a DNSMasq validator assertion still looked in dnsmasq_file.h after the function moved to dnsmasq_domain_policy.h. The test now checks the owning header; exact-head HTTPD run 42568560276 passed on 23321588.

### Next DHCP audit targets

- udhcpc_lan_deconfig() ignores ifconfig() and lan_down_auto() results. lan_down_auto() ignores route_del(), resolver-file open/write/close failures and update status. Trace caller expectations and state reset ordering before changing it.
- udhcpc_lan_leasefail() is a no-op and NAK only logs. Inspect BusyBox udhcpc event producer semantics and AP-mode policy before changing retry/rollback behavior.
- lan_up_auto() drops route_add(), resolver file and update errors. Trace all callers and decide whether any failure must prevent publishing state.
- Do not overstate the existing change: only changed-gateway removal before bound/renew publication is fail-closed and tested.

## Storage / Optware lifecycle follow-through

- Commit 2c8b5ff9797cbb6a0abf99c5fa18210d09df6671 added /sbin/optware_lifecycle_lock.sh and a shared flock used by opt-start.sh and opt-umount.sh. This serializes delayed Optware startup with stop/teardown hooks.
- services_stor.c now invokes the Optware teardown hook before unmounting relevant mounts and refuses to detach a mount when the hook returns failure. Eject-all does not fall through to generic umount_all_storage() after a failed per-device Optware-aware pass. Deferred storage-app startup acquires storage.lock, leaves pending flags intact on lock-acquisition failure and requests a retry alarm.
- Commit 2a5911b24548ac2a1ef06442e0b61771a2714733 fixes the new host test to source the helper before calling it. Storage workflow run 42568851009 passes on exact HEAD. The previous failed attempt was a test setup error, not evidence of a production-source failure.
- Audited caller graph: init_mdev() SD/MMC disk and partition rules route to mdev_sd_main()/mdev_mmc_main(); each takes a per-device lock then storage.lock. Synchronous _eval(..., ppid=NULL) keeps the parent lock across automount.sh and its synchronous hooks. Swap activation and mount/unmount run under the parent operation lock; automount_mountpoint.lock protects a separate mountpoint-ownership scope.
- HTTPD safe-eject aliases converge on RC handlers; USB eject delegates to safe storage removal; PID 1 ejected cleanup uses the public lock-taking umount_ejected() wrapper; already-locked paths use umount_ejected_locked() to avoid lock re-entry; eject-all's umount_all_storage() path is inside the shared operation lock.
- No direct in-repository bypass found in the inspected RC/libdisk/shared/HTTPD/script call graph. A broader Python caller-graph contract was drafted but remains uncommitted and is not part of current storage CI.
- Residual: external Optware init scripts and their descendants are not in the repo. Audit possible lock-order inversion if a custom startup script invokes storage operations while holding the Optware lifecycle lock. Also inspect whether opt-umount service-stop/unmount failures are surfaced sufficiently; the caller's physical umount is still the final busy-device gate. No claim of package-wide/external-hook coverage.

## Other repository-wide open areas

- USB / storage / hotplug — PARTIAL / AUDITING: complete mount inventory before side effects, swapoff-before-unmount, process-shared operation lock, Optware startup/teardown serialization and bounded caller review. External hook implementations, async user scripts, filesystem runtime and hardware remain open.
- WAN / DHCP / resolver — PARTIAL / AUDITING: lease validation, route text safety, staged DNSMasq publication and resolver failure contracts have bounded checks; LAN deconfig and event failure handling remain open.
- Firewall / NAT — AUDITING: bounded FULLCONENAT worker/notifier failure handling exists; mapping-hash and dying-tuple queue namespace ownership remains separate kernel work.
- Wireless — PARTIAL / AUDITING: selected driver result and AP/APCLI/WDS bridge-state publication gates exist; remaining start/stop/iNIC and concurrent lifecycle callers remain open.
- HTTPD / CGI / authentication — PARTIAL / AUDITING: exact security regression passed on the immediately preceding production-source head; continue route/input/error inventory.
- NVRAM / init / process supervision / switch-MDIO / FTP / Samba / NFS / accounts / add-on packages — remain open; no broad completion claim.

## Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for AP DHCP required/optional input and changed-gateway cleanup ordering; PASS for bounded SD/MMC mdev/RC/HTTPD/libdisk/Optware caller review. |
| L2 host / GitHub Actions | Storage [42568851009](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568851009) and docs [42568850779](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568850779) PASS on exact HEAD 2a5911b; HTTPD [42568701873](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42568701873) PASS on production-source head 2c8b5ff. |
| L3 MIPS/uClibc / full firmware/TRX | NOT VERIFIED. |
| L4 live DHCP churn / storage hotplug / physical router | NOT VERIFIED. |

## Exact first action for the next GPT

1. Fetch live main, branches and Actions. Verify exact SHA before using this handoff. The expected current HEAD is 2a5911b24548ac2a1ef06442e0b61771a2714733.
2. Trace the BusyBox udhcpc event producer and AP LAN deconfig path: udhcpc_lan_deconfig(), lan_down_auto(), route_del(), ifconfig(), resolver writes, leasefail and NAK. Establish intended failure semantics and preserve last-known-good lease state; add a focused GNU89 host regression for any proved policy.
3. Audit the new Optware lifecycle critical section for lock-order/deadlock possibilities with custom init scripts. Confirm actual hook status and unmount failure propagation; do not assume the new flock alone proves end-to-end eject safety.
4. Complete the drafted storage caller-graph source-contract regression only after re-reading current files. Add it to storage-hotplug-stability.yml together with existing storage_mountpoint_lock.sh and verify exact-head CI.
5. Rotate to another userland module after these bounded tasks. Keep FULLCONENAT mapping namespace work separate from userland. Do not repeat already-completed SkipDBv2/HTTPD hardening.

## Recovery and archive rule

- Superseded root handoff preserved byte-for-byte as docs/handoffs/archive/2026/11/part-015.md; see docs/handoffs/INDEX.md.
- Superseded root roadmap preserved byte-for-byte as docs/roadmap-history/2026/11/part-015.md; see docs/roadmap-history/INDEX.md.
- The 2026-11-09 09:18 CST AP/APCLI bridge-publication record evicted from the rolling root log is appended verbatim to docs/dev-log/2026/11/part-005.md.
- Root dev_log.md retains exactly the newest 20 complete records. Run python3 scripts/check_ai_handoff_docs.py and verify the exact-head docs workflow after the handoff commit.

