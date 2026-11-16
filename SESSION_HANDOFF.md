# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect live repository state and compare it with this file before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers and precise first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave this file untouched and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task here.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, and do not ask the user to choose the next task.

Every multi-file logical change must be created as one complete Git tree and one atomic main commit parented to freshly fetched main, then update main with a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, or modify the frozen backup branch.

## Current repository state

- Latest main parent before THIS COMMIT: `4a299088849a2f22993ea89f551da1bf1a56be67`. Exact-head Actions at last poll: AI handoff documentation contract 43916525818 completed/success; AliDDNS signed API transport security regression 43916526281 completed/success; Process lifecycle inventory - 4a299088849a2f22993ea89f551da1bf1a56be67 43916526118 completed/success; Process inventory and lifecycle fail-closed regression 43916526135 completed/success; AdGuardHome config security regression 43916526309 completed/success; infosvr packet and signal lifecycle stability regression 43916526519 completed/success; AliyunDrive WebDAV lifecycle stability regression 43916526362 completed/success; XY-C1 Firmware Validation 43916527161 in_progress; USB storage hotplug stability regression 43916525886 completed/success; NVRAM transaction boundary audit 43916525919 completed/success
- Frozen backup branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- USB storage automount now uses checked `/proc/modules` snapshots, checked `modprobe -l` catalog capture, and a checked tri-state `/proc/mounts` inventory. Preflight, revalidation, safe unmount and final source-device ownership use the mount table directly; UNKNOWN/malformed state aborts.
- SmartDNS Adblock lifecycle regression setup was repaired in parent history; current Process inventory and USB storage Actions must independently establish the fixture passes.

## Active task

THIS COMMIT fixes an NDIS USB-modem startup error-propagation gap in `launch_wan_usbnet()`: the initial `ifconfig <wwan> mtu ... up 0.0.0.0` was unchecked, allowing firewall setup, modem-control commands and DHCP launch to continue against an interface that had failed to come up. It now captures `doSystem()` status, marks the WAN unit errored, logs the exact interface/status, and returns before any dependent startup. Added a GNU89 regression simulating the failed-IFUP transition and a source checker asserting the real call/guard order.

## Verification boundary

- Parent `4a299088849a2f22993ea89f551da1bf1a56be67` exact-head Actions at commit time: AI handoff documentation contract 43916525818 completed/success; AliDDNS signed API transport security regression 43916526281 completed/success; Process lifecycle inventory - 4a299088849a2f22993ea89f551da1bf1a56be67 43916526118 completed/success; Process inventory and lifecycle fail-closed regression 43916526135 completed/success; AdGuardHome config security regression 43916526309 completed/success; infosvr packet and signal lifecycle stability regression 43916526519 completed/success; AliyunDrive WebDAV lifecycle stability regression 43916526362 completed/success; XY-C1 Firmware Validation 43916527161 in_progress; USB storage hotplug stability regression 43916525886 completed/success; NVRAM transaction boundary audit 43916525919 completed/success
- The XY-C1 workflow already includes `usb_modem.c`, `modem_wan_start_policy.c`, and `check_modem_wan_start_status.py` in its trigger and invokes the GNU89/Werror/ASan/UBSan test plus semantic source checker. This commit therefore triggers same-SHA product build/TRX/artifact verification.
- No full local repository build is available or claimed. Physical-router L4 remains NOT VERIFIED.
- Frozen backup remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Inspect exact-head XY-C1 actions for THIS COMMIT; require modem policy regression and `check_modem_wan_start_status.py` to pass, then inspect MIPS Build, TRX verification and artifact upload.
2. Inspect current exact-head USB storage and Process inventory runs already pending at parent; confirm SmartDNS fixture and automount regressions are green without re-auditing prior work.
3. If Actions remain pending, continue another independent WAN/USB-modem reliability defect.
4. Keep backup read-only and physical-router L4 marked NOT VERIFIED.
