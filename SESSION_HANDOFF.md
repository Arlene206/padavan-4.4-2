## Current repository state

- Repository: Arlene206/padavan-4.4-2. Only `main` is authorized for development, commits and push validation. Current base main SHA for this checkpoint: `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`; re-fetch live main before every write.
- Remote branch inventory contains only `main`. Legacy PR #1 is CLOSED (not merged) and remains read-only; no open PRs. GPT Session Rotator remains immutable.
- The previously active XY-C1 target-build task is now complete at L3 on this exact base SHA. XY-C1 push run [43273846470] / job [131376901927] passed all Samba, DHCPv6 DUID and vsftpd prechecks, the complete MIPS firmware build, TRX output verification and artifact upload. Uploaded artifact `XY-C1-firmware-fcdc05e76e6522f9f35b59b71bd222b884cb2ce4` is 12,573,414 bytes (SHA-256 `f4adfa9a13224e519cd580946e30e6a3d336b3da410f0842051eb697a7baee08`).
- The BusyBox ROMFS recursive Kbuild fix in this base is confirmed by the successful target build. Six other exact-head checks also passed: infosvr [43273846610], AliyunDrive [43273846558], AdGuardHome [43273846504], AI handoff docs [43273846442], HTTPD security [43273846549] and AliDDNS [43273846513].
- One relevant exact-head check is red: storage-hotplug [43273846489] / job [131376902484] failed in its standalone host BusyBox 1.24.2 build at `networking/udhcp/dhcp6c_common.c:38`, because that direct build omitted the shared source include path for `dhcp6c_duid_file_policy.h`. All preceding shell/Optware lifecycle regression steps passed; later storage regression steps were skipped after the BusyBox build failure.
- This is a host workflow integration gap, not evidence of a production firmware build failure: the firmware package Makefile supplies `EXTRA_CFLAGS += -I$(ROOTDIR)/user/shared/src` through package `MAKE_VARS`; the independent CI host build bypasses that package wrapper and had retained an empty `CONFIG_EXTRA_CFLAGS`.

## Active task — Repair storage-hotplug host BusyBox shared-header build

1. In `.github/workflows/storage-hotplug-stability.yml`, set the disposable host BusyBox `.config` `CONFIG_EXTRA_CFLAGS` to `-I$PWD/trunk/user/shared/src` before `oldconfig`; assert Kconfig retains that exact setting. Do not modify the tracked firmware board config.
2. Extend the workflow's semantic contract to require the DHCPv6 common source include, production package include flag, BusyBox `Makefile.flags` config mapping and host-test include propagation.
3. Push only to `main`; inspect the storage-hotplug and AI handoff documentation runs for the exact resulting `main` SHA. Confirm the BusyBox host build links and the downstream storage tests run; repair the first causal failure if another appears.
4. Preserve the confirmed XY-C1 L3 result above. L4 physical router flashing/boot is a separate unverified milestone.

## Verification boundary

- L1 recovery: bootstrap, development protocol, live main, handoff, roadmap, newest dev-log records and archive index reviewed; main-only branch policy confirmed.
- L2 on `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`: XY-C1 prechecks and AI docs PASS; infosvr, AliyunDrive, AdGuardHome, HTTPD security and AliDDNS exact-head push workflows PASS. Storage-hotplug FAILS at its standalone host BusyBox build because the test harness does not carry the shared-header CFLAGS. No later storage jobs in that workflow ran.
- L3 on `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`: XY-C1 MIPS firmware build PASS; TRX output verification PASS; non-empty 12,573,414-byte Actions artifact uploaded PASS. This validates the XY-C1 build path on this SHA, not every board or module.
- L4 physical router flashing, boot and runtime behavior: NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main`; inspect the exact-head storage-hotplug run triggered by this workflow correction and the AI handoff docs run. Require the host BusyBox build and every downstream storage regression to pass; otherwise fix the first causal error directly on `main`.