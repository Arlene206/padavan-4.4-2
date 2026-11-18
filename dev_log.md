## 2026-11-17 — Keep the OpenVPN TUN directory traversable after privilege drop

- Parent main: `c0601f0b236bad685ca2b0e945d83861e2771d57`. The generated OpenVPN server config explicitly uses `user nobody` and `group nogroup`, but PID 1 previously accepted an existing root-owned `/dev/net` with mode 0700 or 0750 because it checked only owner execute and absence of group/world write. Such a directory can prevent the unprivileged daemon from traversing `/dev/net/tun` when it opens or reopens the persistent endpoint.
- Add a root/root directory policy requiring owner rwx, group+other search, no group/world write and no special bits. Unsafe permissions are repaired to 0755 and then revalidated with `lstat`. Extend the GNU89 regression matrix for 0755/0711 acceptance and private, non-traversable, writable, special-bit, non-directory and ownership rejection.
- The exact-source PID1 mdev and XY-C1 Build/TRX/artifact checks are pending for this commit. Physical-router L4 remains unverified.

---
## 2026-11-17 — Use complete Git tree diffs for the XY-C1 build gate

- The inline gate depended on GitHub push payload commits[] entries. Exact XY-C1 `44221748846` on `a4d5a10b` passed host tests but logged `Firmware production inputs changed: True` with no changed-path lines, then ran Build, TRX and artifact upload for a CI/docs-only commit.
- Replace webhook path parsing with git diff --name-only --no-renames -z BEFORE AFTER; verify checkout HEAD equals AFTER and fetch exact BEFORE. A complete tree diff distinguishes host-only from firmware source/config changes even when webhook path data is truncated or empty.
- Invalid/initial SHAs, non-push events, fetch/diff failures, timeouts and checkout mismatch fail safe to a full build. Add 10 standard-library unit regression cases. No product firmware source changes in this CI-gate slice.
- On parent 5fd23912, PID1 TUN policy/source contract `44223253251` PASS; AI docs failure from paraphrased atomic-main-commit wording was repaired in `889b8a09`, and exact AI run `44224426682` PASS. XY-C1 `44223253163` passed host checks and was in target Build at last poll.

---
## 2026-11-17 — Restore exact atomic-main-commit handoff contract

- Exact AI handoff run `44223253226` on `5fd23912733ba97a286e14df0c398d06d1414dd2` failed only because `SESSION_HANDOFF.md` paraphrased the required atomic-main-commit instruction and omitted the literal phrase `one atomic main commit`.
- Restore the canonical wording while retaining non-forced expected-SHA publication and the frozen-backup restriction. No product source or workflow trigger changed.
- Exact PID 1 mdev workflow `44223253251` passed its existing-config and OpenVPN TUN policy binaries plus caller source contract. XY-C1 `44223253163` passed host gates and entered Build; TRX/artifact remained pending at this checkpoint.

---
## 2026-11-17 — Validate the OpenVPN TUN device node before trusting it

- Parent main `de02f6c04d993d462983576e3db2914e9b179afe`: c500 exact-head USB, HTTPD, Process, NVRAM, Infosvr, AdGuardHome, AliDDNS, AliyunDrive, AI and XY-C1 all pass, with Build/TRX/artifact on the same SHA. Infosvr/AliyunDrive trigger-contract fixes pass on `45ed6be1`; AI checkpoint wording passes on `de02f6c0`.
- `init_nodes()` previously checked only whether `/dev/net/tun` existed. A regular file, symlink or different character device could bypass creation of the actual TUN endpoint.
- Add root-owned/non-group-world-writable `/dev/net` validation and an `lstat` policy for character device 10:200, root uid/gid, single link and read/write-only permissions. Replace invalid leaf entries without following symlinks, create/chown/chmod and revalidate the TUN node. Log failure without blocking unrelated router startup.
- Add GNU89 regression for wrong type/identity/owner/group/link count/mode and extend PID1/XY-C1 path filters. Exact-head CI pending; no physical L4 claim.

---

## 2026-11-18 11:03 Beijing — Restore mandatory handoff checkpoint wording

- Parent main SHA: `45ed6be1ad788a77982ce7f4e8683557265a496b`.
- The exact-head AI handoff run `44222138719` failed only because `SESSION_HANDOFF.md` retained the checkpoint heading and accuracy instructions but omitted the contract's required sentence: “The checkpoint is mandatory; a document edit is conditional.”
- Restore the literal required sentence and preserve the full end-of-session live-state/Actions/roadmap/first-task inspection. No product source or workflow trigger is changed.
- On parent 45ed, Infosvr `44222138695` and AliyunDrive `44222138760` PASS after their path-filter checker fixes. AI handoff requires this documentation-only rerun. No new firmware source validation is implied.

---

## 2026-11-17 — Align module checkers with narrowed documentation triggers

- Parent main `a4d5a10b426159712b7d85c368e7e3973f90b674` removed canonical handoff/roadmap/log files from the Infosvr and AliyunDrive workflow path filters to prevent documentation-only pushes from running unrelated product regressions.
- Exact Infosvr `44221748799` passed its GNU89 packet, SIGTERM and daemon-lifecycle runtime regressions, then failed because `scripts/check_infosvr_packets.py` still required `SESSION_HANDOFF.md` in both path filters.
- Exact AliyunDrive `44221748973` passed shell syntax, QEMU MIPS binary validation and private-workdir/lock/stop/start lifecycle regressions, then failed because its source checker still required all three canonical docs paths.
- Remove only those obsolete docs entries from the checkers. Preserve actual product source, test, workflow and checker path requirements; do not re-add doc triggers. Exact-head validation pending.

---

## 2026-11-18 — Narrow documentation CI triggers and gate XY-C1 target builds

- Parent main SHA: `c500c3a48328fc54e8949a75187de648ec0f78a6`. Audited all 54 root workflow files and exact-head fan-out. The `7f10d22` source push started 11 workflows; several module workflows ran only because canonical handoff/roadmap/log files were listed as product path triggers, while AI docs ran on every push.
- Removed those docs paths from AdGuardHome, AliDDNS, AliyunDrive, infosvr, NVRAM and Process lifecycle regression. Scoped AI handoff validation to main changes in canonical records, `docs/dev-log/**`, its validator and workflow.
- Removed the HTTPD Security workflow YAML from XY-C1 firmware-input paths. Kept host regressions, but gated target `make XY-C1`, TRX verification and artifact upload on actual production-input changes. Tests, checkers, workflow files and documentation no longer cause full MIPS builds; manual dispatch still builds.
- Added latest-push concurrency cancellation to selected expensive/cross-cutting workflows. Preserved full-tree NVRAM and Process Inventory scans, plus Storage's BusyBox/SmartDNS patterns, because their regression scripts use those source trees.
- No product firmware source changed in this CI-only slice. Exact-head CI is pending; a host-only run with target stages skipped is not an L3 firmware result.

---

## 2026-11-18 — Propagate NDIS metadata publication and cleanup failures

- Parent main SHA: `0985b536e5dd2a40387fcc2069f392519d17df49`. Exact storage run `44218995113` and AI handoff `44218995112` PASS; infosvr `44218995163`, AliyunDrive `44218995175`, AliDDNS `44218995159` and AdGuardHome `44218995324` PASS. NVRAM `44218995246` and Process inventory `44218995232` remained in progress.
- `mdev_net_main()` returned zero on every branch after acquiring its lock, even when NDIS metadata publication failed or stale metadata could not be removed. This covered unplug, unavailable USB topology, invalid devnum and writer failure.
- Check all explicit removals, propagate publisher -1/-2 status as handler failure, log residual stale-file cleanup, and return after unlocking. Keep topology-not-ready nonfatal when stale metadata cleanup succeeds.
- Extend USB/HTTPD NDIS source contracts and XY-C1 filters for the USB hotplug source and helper headers. Exact-head Actions pending; L3/L4 not claimed.

---

## 2026-11-18 — Make SD/MMC HDD-tune status contract branch-specific; restore handoff checkpoint

- Parent main SHA: `695d24501e52985cf6e10437fee84f285d9f34cb`. Storage check on 7f10 failed at the SD final-status assertion; collaborator commit 695d2450 added tune_status but unintentionally expected it in MMC too. The production source confirms only mdev_sd_main() runs hddtune; mdev_mmc_main() has no tune_status.
- Branch the source contract: SD checks its tune_status aggregate, MMC checks swapoff/mount/topology aggregate.
- Exact 695 AI handoff failed because SESSION_HANDOFF.md omitted the required literal end-of-session accuracy-checkpoint rule; restore the mandated sentence and no-op commit exception.
- Exact 695 infosvr, AdGuardHome, AliDDNS, AliyunDrive and Process inventory PASS; NVRAM in progress. Storage run was pending at last poll; XY-C1 7f10 Build/TRX/artifact still pending at last poll.

---

## 2026-11-18 — Align storage caller contract with HDD tune result propagation

- Exact storage run `44218214340` on `7f10d225` passed the new hddtune shell runtime regression, share-name policy, Optware/MTD/persistence/eject and lifecycle checks, then failed at line 585 because its old SD handler return assertion omitted the newly included `tune_status`.
- Correct the duplicated checker expectation; no production source or runtime-test behavior changes.
- The same source SHA HTTPD, AdGuardHome, AliDDNS, AliyunDrive, infosvr, AI docs and Process lifecycle passed; NVRAM, Process inventory and XY-C1 were still in progress. Exact storage rerun pending.

---

## 2026-11-18 — Aggregate HDD tuning failures without blocking storage mount

- Parent main SHA: `7f50d54c1c61ed821f063291e127da75a0268211`. USB modem `44216537116`, HTTPD `44216537047`, AI docs `44216537225`, infosvr `44216537061`, AdGuardHome `44216537101`, AliDDNS `44216537361`, AliyunDrive `44216537117`, Process inventory `44216537152`, Process lifecycle `44216537350`, and NVRAM `44216537363` PASS; XY-C1 `44216537251` remained in progress.
- `hddtune.sh` discarded `hdparm` failures and returned success. SD whole-disk hotplug also discarded the helper wait status, masking tuning failures from hotplug/PID1.
- Aggregate status across disks/options while continuing later operations. Capture/log SD tune status and include it in final handler status, but do not suppress partition detection or automount because tuning is best-effort.
- Add a stubbed shell regression for success, either option failing, later disk continuation, removable media, invalid names and missing `hdparm`; update storage and XY-C1 path filters. Exact-head Actions pending; L3/L4 not claimed.

---

## 2026-11-18 — Migrate WDM metadata to cleanup-aware publisher

- Parent SHA: c47a1b1fdfbd1d4061df4ef9294665fd21d3fc63. HTTPD 44216025750, AI 44216026808, AdGuardHome 44216025560, infosvr 44216025661, AliDDNS 44216025635 and AliyunDrive 44216025544 PASS; NVRAM 44216025596, Process inventory 44216025643 and XY-C1 firmware 44216025599 pending at last poll.
- mdev_wdm_main() ignored stale-record removal errors and used modem_node_file_write() directly. On write failure its return path could leave the previous USB identity associated with the cdc-wdm node while reporting success.
- Move to modem_node_file_publish_identity(), propagate publish/cleanup status and explicit remove errors, then add WDM caller contracts to USB and HTTPD regression workflows.

---

## 2026-11-18 — Fix stale semicolon requirement in HTTPD TTY contract

- Parent SHA: c24fd0e7398c400a61982b7c57b6a22f9e368ce7. HTTPD 44215677832 completed the metadata regression binary and first 34 Python source assertions, then failed because the last TTY removal contract insisted on the exact ignored-return statement modem_node_file_remove(...);.
- The TTY code correctly uses modem_node_file_remove(...) != 0 to catch cleanup errors. Update the HTTPD assertion to match this checked call. No C source behavior changes.
- Exact parent AI and AdGuardHome PASS; NVRAM, AliDDNS, infosvr, AliyunDrive, Process inventory and XY-C1 still running at last poll. Exact-head reruns pending.

---

## 2026-11-18 — Align HTTPD duplicate source contract with TTY metadata cleanup

- Parent main SHA: `7aca8abf312ed6ab029a8757a936a6a7f0793d07`. Exact USB, AI handoff, infosvr, AdGuardHome, AliyunDrive, AliDDNS, Process lifecycle, Process inventory and NVRAM Actions PASS.
- HTTPD Action `44215156710` failed only in its duplicate USB metadata source assertion: it still counted two direct metadata writes and required TTY direct write, despite `mdev_tty_main()` now using cleanup-aware `modem_node_file_publish_identity()`. The metadata policy regression binary passed before this assertion.
- Update the HTTPD contract to count only the remaining WDM direct write and assert TTY publication cleanup/status behavior. Test-only correction; production C unchanged. Exact-head verification pending.

---

## 2026-11-18 — Clean stale TTY identity after failed metadata replacement

- Parent main SHA: `9ff3bc667ea0507f311b417f16ed3b324cf19ae6`. Exact storage hotplug `44214586174` and process lifecycle inventory `44214586292` PASS; XY-C1 `44214586399`, NVRAM `44214586157` and other Actions pending at last poll.
- `mdev_tty_main()` used `modem_node_file_write()` directly. On write failure it neither removed the old TTY identity file nor returned failure, so a stale USB port/devnum record could survive a node replacement.
- Switch to `modem_node_file_publish_identity()`, which atomically replaces the file and removes stale metadata when a write fails; propagate publication and removal errors after releasing the lock. Extend the USB caller source contract; existing `modem_node_file.c` cleanup regression exercises the helper.

---

## 2026-11-18 — Report SD/MMC share-name formatting failures

- Parent main SHA: `e52d186ad67427672ea199210e59fc5bf42f2557`. Exact USB modem `44214240090`, AI docs `44214239903`, infosvr `44214239886`, HTTPD Security `44214240078`, AliDDNS `44214240027`, AdGuardHome `44214240003` and Process lifecycle `44214239950` PASS; NVRAM `44214240232`, AliyunDrive `44214239907` and Process inventory `44214239911` were in progress.
- In `mdev_sd_main()` and `mdev_mmc_main()`, share-name truncation jumped to cleanup with `topology_status` still zero, so a skipped automount was acknowledged as success. The old `>= sizeof` test also let negative `snprintf()` errors through.
- A shared `storage_hotplug_name_format_status()` accepts only nonnegative format result smaller than buffer capacity. Each caller logs and sets topology failure before cleanup. Existing root-partition no-mount path stays benign. Add GNU89/Werror/ASan/UBSan policy tests and source contract for both callers.
- Exact-head storage workflow pending. Firmware L3 / physical L4 NOT VERIFIED.

---

## 2026-11-18 — Fix NDIS metadata field separators

- Parent main SHA: `b103a8e47455eb65f37470df4897696e9e1f4e02`. Exact parent USB modem `44213786597` PASS; AI, AdGuardHome, infosvr, AliDDNS, AliyunDrive, HTTPD Security, Process lifecycle and Process inventory PASS. NVRAM audit `44213786481` remained IN_PROGRESS.
- `mdev_net_main()` encoded every separator as a literal backslash-n pair because the C format string contained `\\\\n` rather than `\\n`. The line-oriented `find_modem_node_by_usb_identity()` parser requires real line breaks and rejects the malformed record.
- Replace with real newline escapes matching the WDM/TTY metadata writers and add an exact NDIS source-format regression assertion to the USB modem workflow. Exact-head verification pending.

---

## 2026-11-18 — Propagate ZeroCD helper failures from SCSI generic hotplug

- Parent main SHA: `2e01dae1878bdc984326895d5f464789bdcc3d76`. Exact parent USB modem `44213177117` PASS; all SG/CD policy binaries, source contract and push/PR filters verified. AI, AdGuardHome, infosvr, AliDDNS, AliyunDrive, HTTPD Security, Process lifecycle and Process inventory also PASS; NVRAM audit `44213177187` remained IN_PROGRESS.
- In the `modem_zcd == 0` branch, `mdev_sg_main()` discarded the `_eval("/sbin/zerocd", ...)` result and unconditionally reported success. Capture the child exit/launch status; on nonzero, log, unlock and return failure. Preserve the prior no-op when USB topology or VID/PID lookup fails.
- Extend the SG pure status policy, GNU89/Werror/ASan/UBSan regression and exact workflow source contract. Exact-head USB rerun pending; firmware L3 and hardware L4 NOT VERIFIED.

---

## 2026-11-18 — Include SCSI generic module-readiness policy

- Parent main SHA: `bdeeee072c705524230d64986bdeffd5f41ff373`. Exact parent USB modem workflow `44212660230` compiled/ran both SCSI GNU89/Werror/ASan/UBSan policy binaries but failed the inline caller assertion because `hotplug_usb.c` used `modem_sg_module_ready_status()` without including `modem_sg_module_policy.h`.
- THIS commit adds the missing include; no other source semantics changed. Exact parent AI, Process inventory, AliyunDrive, HTTPD Security, infosvr, AliDDNS, AdGuardHome and process inventory PASS; NVRAM audit `44212660076` still in progress.
- Exact-head USB rerun pending; no firmware L3 or physical L4 claim.

---

## 2026-11-18 — Verify SCSI generic driver readiness after module load

- Parent main SHA: `40f0aecead8070a931809244ff5394a8afe01e90`. Exact USB modem `44212191127`, AI `44212191043`, infosvr `44212190976`, AliDDNS `44212191022`, AdGuardHome `44212190980` and Process inventory `44212191040` PASS; NVRAM `44212191003` and AliyunDrive `44212190996` pending at last observation.
- `mdev_sg_main()` discarded the ambiguity of `module_smart_load("sr_mod", NULL)`: zero means either already loaded or a failed load. The handler did not verify that `sr_mod` was in fact present, then printed success.
- Preserve sleep only when a new load occurs, probe `is_module_loaded_status("sr_mod")`, accept only positively present state, and fail closed on absent/unknown status. Add GNU89/Werror/ASan/UBSan policy cases and exact caller source-contract checks to USB modem workflow.
- Exact-head verification pending. Firmware L3 and physical L4 NOT VERIFIED.

---

## 2026-11-18 — Add USB SCSI policy files to pull_request filter

- Parent main SHA: `2fc75549023388f2e90da2c324c372873cee7ee9`. Exact USB modem workflow `44211715745` successfully compiled/ran the SCSI GNU89/Werror/ASan/UBSan policy test and passed inline source assertions, then failed its final path-filter assertion because the new policy header/test were omitted from the pull_request filter.
- THIS workflow-only correction adds both entries to the PR filter and verifies each exists exactly once under push and pull_request. Production C source unchanged.
- Exact parent AI handoff `44211715782` PASS; HTTPD Security `44211715867`, NVRAM audit `44211716167` and Process inventory `44211715739` were pending at last observation. Exact-head USB rerun pending.

---

## 2026-11-18 — Propagate USB SCSI CD eject/unload failures

- Parent main SHA: `2d91171b217e6f42a96fa5af7da3cdcacc499b3f`. Exact parent HTTPD Security `44211273335`, AI docs `44211273436`, infosvr `44211273238` PASS. XY-C1 `44211273337` remains in Build; NVRAM `44211273258` remains at raw ioctl/kernel caller audit; several other lifecycle/security checks were pending.
- `mdev_sr_main()` discarded both `eject -s` and `module_smart_unload("sr_mod", 1)` results, then always printed success. This hid incomplete virtual-CD eject or driver-unload operations.
- Capture/log eject and unload results. Because module_smart_unload returns zero when the module is absent or when unload fails, a tri-state `is_module_loaded_status()` check treats confirmed absence as idempotent success while rejecting present/unknown state. Preserve positive result for already-in-use no-op. Add GNU89/Werror/ASan/UBSan regression and exact USB modem source contract.
- Exact-head USB modem and XY-C1 Build/TRX/artifact pending. Physical L4 NOT VERIFIED.

---

## 2026-11-18 — Fix HTTPD WISP source-check variable names

- Parent main SHA: 01ce19d478735fe22ab2252929fe6dd0890ee330. Exact parent WAN route 44210860686 PASS (including manual WISP status propagation/source contract); AI handoff 44210860863 PASS.
- Exact parent HTTPD Security 44210860852 failed in its inline source step with `NameError: net_wan is not defined`. That step binds the code as `wan`, `rc` and `header`; the added WISP source assertions mistakenly referenced `net_wan`, `rc_main` and `rc_header`.
- Corrected only those references in the embedded Python contract. Product C source and assertion intent remain unchanged; exact-head HTTPD rerun pending.
- Parent XY-C1 44210860887 and NVRAM 44210860859 remain in progress; do not infer L3.

---

## 2026-11-18 — Propagate manual WISP reassociation failures into PID 1

- Parent main SHA: 4e52a861c3e227baee1a914376f293557c356559. Exact parent HTTPD Security 44210330988 PASS, AI handoff 44210331008 PASS, AdGuardHome 44210331086 PASS, infosvr 44210331070 PASS, AliDDNS 44210331128 PASS, Process inventory/lifecycle 44210331089 PASS and AliyunDrive 44210330979 PASS. XY-C1 44210331166 remains in Build; NVRAM audit 44210331025 remains in raw caller audit.
- manual_wisp_reassoc() captured reconnect_apcli() failure but returned void; PID1 discarded it and could acknowledge a failed reassociation. Convert to int, return reconnect status and merge it through rc_notify_merge_action_result(). Preserve the no-interface case as a successful no-op and keep its diagnostic.
- Extend WAN route source checker and HTTPD Security's duplicated lifecycle contract for the return/prototype/dispatcher path. Exact-head WAN route, HTTPD and AI runs pending.
- No MIPS L3 or physical hardware L4 claim.

---

## 2026-11-18 — Repair stale HTTPD contract after manual WAN disconnect status propagation

- Parent main SHA: `4bbbe8a3f7bd8b81638271d092287ff986e5fa81`. WAN route regression `44209928594` and PID 1 notification queue `44209928573` PASS. Exact parent XY-C1 `44209928351` remains at Build; NVRAM `44209928413` remains in raw ioctl/kernel caller audit.
- HTTPD Security `44209928509` passed steps 1–37 and failed only at step 38, which duplicated an outdated source assertion: `if (stop_wan_ppp() != 0)` in `manual_wan_disconnect()`. The API now assigns the selected PPP or ordinary teardown result to `status`, logs nonzero, and returns status. WAN route workflow's independent source contract already verifies this behavior.
- THIS correction changes the HTTPD workflow to assert both stop assignments, nonzero handling and status return. It does not change product source or relax the behavior contract. Exact-head HTTPD rerun pending.

---

## 2026-11-18 — Propagate manual WAN disconnect failures to PID 1

- Parent main SHA: 94b5ddbf49fd71a06ce83d907c9e6b3b23caadd0. Exact parent PID 1 notification queue regression 44209458419 PASS, including positive wait-status mapping for failed command actions; AI handoff 44209458097 PASS. AliDDNS 44209458249, VPN 44209458344, Samba 44209458318, FRP 44209458244, infosvr 44209458058, rstats 44209458146, USB modem 44209458325, DNS Forwarder 44209458272, DNSMasq 44209458260, Firewall 44209458149 and iNIC 44209458219 PASS. WAN route 44209458297, XY-C1 44209458268, NVRAM 44209458222 and gateway ARP 44209458216 were pending at last poll.
- manual_wan_disconnect() returned void and silently returned early when stop_wan() or stop_wan_ppp() failed. PID 1 discarded it too, so the incomplete marker was cleared as if disconnect succeeded.
- Convert it to int, preserve modem-type and PPP-protocol stop selection, return/log the selected teardown result, update rc.h and merge it with PID 1 action_result. AP mode remains a successful no-op.
- Extend WAN source checker for every branch, status return, error log, public prototype and PID 1 aggregation. Existing RC notification queue C policy regression already verifies all nonzero (including positive errno/wait status) map to a retained failure.
- Exact-head WAN route policy/source contract pending. No physical L4 claim.

---

## 2026-11-18 — Propagate HDD tune notification failures

- Parent main SHA: `45eb7a00bb8e990ca3e6f8aab1eab0b0de352929`. Exact-head detect_internet `44208911848`, WAN teardown `44208911621`, AI `44208911735`, DNSMasq `44208911911`, ZeroTier `44208911792`, Wireless `44208911890`, infosvr `44208911810`, WAN DHCP `44208911737`, HTTPD Security `44208911702`, miniupnpd `44208911870`, USB modem `44208911916`, AdGuardHome `44208911853`, detect_link `44208911730`, Firewall `44208911825`, WAN route `44208911923`, AliyunDrive `44208911766`, AliDDNS `44208911855`, WAN gateway ARP `44208911946`, Process lifecycle `44208911854`, Process inventory `44208911822` PASS. XY-C1 `44208911610` and NVRAM `44208911609` remain IN_PROGRESS.
- PID 1's `RCN_RESTART_HDDTUNE` handler discarded the synchronous `system("/sbin/hddtune.sh")` status, leaving the notification `action_result` at zero after a failed storage-tuning command.
- Capture/log the command wait status and merge it via `rc_notify_merge_action_result()`. Always retain `set_pagecache_reclaim()`, which is an independent follow-up. Add a GNU89 regression for a representative positive wait status and a workflow source contract for the exact handler.
- Exact-head RC notification queue Actions, XY-C1 Build/TRX/artifact, and NVRAM follow-up pending. Physical L4 NOT VERIFIED.

---

## 2026-11-18 — Capture optional Internet-state hook exit status

- Parent main SHA: 6fb72f857ae1141705e7a7c4f1e91354e46503ed. Exact 6fb detect-internet action queue regression 44208495457, WAN route 44208495523, WAN interface teardown 44208495529, AI handoff 44208495654, Process lifecycle 44208495046, AdGuardHome 44208495456, ZeroTier 44208494833, detect_link 44208494945, miniupnpd 44208494872, VPN server 44208494879, DNSMasq 44208495154 and infosvr 44208495121 PASS. XY-C1 44208495458 at Build; NVRAM 44208494936 in raw IOCTL audit.
- After replacing automatic lost-Internet notify_rc() calls with status-aware queueing, the optional SCRIPT_INTERNET_STATE hook still discarded doSystem() wait status. A failing user-configured hook was not distinguishable from a successful callback.
- Capture/log the command status only when the script exists. Failure remains nonfatal to connectivity-state processing, and reboot/pause/USB queue behavior is unchanged.
- Extended the detect-internet workflow source contract to require the capture/log sequence. Exact-head sanitizer/source checker pending; no hardware L4 claim.

---

## 2026-11-18 — Report Internet-loss action enqueue failures

- Parent main SHA: `33ab220a9f023b274ee402c7fa04a297f3ba4f51`; exact WAN DHCP `44208003391`, AI `44208003368`, HTTPD `44208003400`, AliyunDrive `44208004391`, AliDDNS `44208003407`, infosvr `44208003447`, AdGuardHome `44208003338` PASS. Process inventory/fail-closed `44208003464` and NVRAM `44208003259` pending; XY-C1 `44208003430` at Build.
- Found three fire-and-forget `notify_rc()` calls in `notify_on_internet_state_changed()`: router reboot, automatic reconnect pause and USB-modem reconnect. PID 1 queue rejection was silently discarded.
- Replaced them with `notify_rc_status()` via a helper that normalizes errno/status and logs a failed event submission. No change to detector signal handling or Internet-state script dispatch.
- Added a GNU89/ASan/UBSan errno policy regression and extended exact workflow source/path checks. Exact-head Actions pending; prior WAN VPN stop and AP DHCP L3 validation do not validate this new source. Physical L4 NOT VERIFIED.

---

## 2026-11-18 — Align LAN DHCP ancillary regression contracts

- Parent main SHA: `355a5c2565b2e6d001a721289e1e690fda997931`.
- Exact `355a5c25`: WAN DHCP `44207255220` failed only because the workflow looked for the old C test symbol `test_successful_lease_commit_ignores_ancillary_hosts_failure`; the actual policy test is `test_successful_lease_commit_ignores_ancillary_refresh_failure`. Its preceding GNU89/ASan/UBSan and source-check steps passed.
- HTTPD Security `44207255662` failed on a stale literal EIO assertion; the current C test loops over `EIO`, `ENOSPC`, `ETIMEDOUT`, and `ECHILD`. Updated the embedded source contract to match the actual test.
- AI handoff `44207255510` failed because the handoff omitted the exact literal phrase `one atomic main commit`; restored that wording.
- NVRAM `44207255182` PASS; all other exact parent runs recorded in SESSION_HANDOFF. XY-C1 `44207255537` remained in Build with TRX/artifact pending. No production source changes in this correction; hardware L4 NOT VERIFIED.

---

## 2026-11-18 — Capture ancillary LAN DHCP follow-up failures

- Parent main SHA: `e5c5bdc37a722aa7953246409c9d0c1d16002864`. Exact b3a AI handoff `44205722872` PASS; AliyunDrive `44205723343` PASS on unchanged sources after isolated 21b5 failure `44205040903`; Process inventory/lifecycle `44205723100` / `44205723080`, WAN route `44205723137`, NVRAM `44205723095`, HTTPD `44205723230` and broad lifecycle/security checks PASS. XY-C1 `44205723068` was still building; no L3 claim.
- In `udhcpc_lan_bound()`, unchecked `system("/usr/bin/iappd.sh restart")` and discarded `restart_networkmap()` result could conceal ancillary LAN service failures after DHCP lease state had committed.
- Capture and log each nonzero result. Do not fail the BusyBox BOUND callback: this would leave BusyBox retaining an old lease while router interface/route/resolver state uses the new lease. Updated the policy comment, expanded the policy test with timeout/child-failure statuses, and extended the source checker.
- Exact-head WAN DHCP callback and XY-C1 verification pending. Hardware L4 NOT VERIFIED.

---

## 2026-11-18 — Restore the complete roadmap module dashboard after checker failure

- Parent main SHA: `21b5da973728481fafec579369d48c1c1f1ef564`; exact WAN route `44205041253`, WAN DHCP `44205041172`, WAN teardown `44205041312`, Process inventory/lifecycle `44205041217`/`44205041136`, infosvr `44205040920`, AdGuardHome `44205041507`, AliDDNS `44205041374`, Wireless `44205041398`, Firewall `44205041410`, miniupnpd `44205041416`, ZeroTier `44205041193`, detect_link `44205041309`, detect_internet `44205040982`, VPN server `44205040974`, DNSMasq `44205040969`, USB modem `44205040951`, HTTPD `44205040923`, NVRAM `44205040901` PASS.
- AI handoff run `44205041164` failed because a prior roadmap replacement accidentally removed the required `## Whole-repository module status` / `AUDITING` dashboard. THIS commit restores the complete 54 KiB roadmap body from the immediately preceding intact `fda7c018` version while adding a new current-status intro. The source regression itself did not fail.
- AliyunDrive run `44205040903` independently failed; cause left open for the next diagnosis. XY-C1 `44205041399` was still building, so no L3 claim for the WAN stop notification change.
- No production source changes. Physical L4 NOT VERIFIED.

---

## 2026-11-18 — Preserve WAN VPN stop notification enqueue status

- Parent main SHA: `fda7c018f1cb3ca4d0588f1f3e30baaba13f28c0`; exact HTTPD `44204487769`, AI `44204487682`, Process `44204487756`, infosvr `44204487704`, AdGuardHome `44204487714`, AliyunDrive `44204487708` and AliDDNS `44204487832` PASS; NVRAM `44204487741` and XY-C1 `44204488447` pending at last poll.
- Commit `816146ab` already preserves the WAN-up `start_vpn_client` enqueue result. The matching down path still called `notify_rc("stop_vpn_client")` and discarded enqueue errors, allowing WAN teardown to report success while the VPN stop event was not accepted into the queue.
- Changed WAN-down to call `notify_rc_status()`, aggregate/log any failure, and continue later IPv6, route and interface cleanup. Added a regression for failure preservation with later successful cleanup and a workflow source contract.
- Prior AP DHCP source SHA `509409f8` XY-C1 run `44202831040` completed Build, TRX and artifact upload successfully; this later WAN source change needs its own exact-head product build. Physical L4 NOT VERIFIED.

---

## 2026-11-18 — Checkpoint WAN deferred VPN enqueue status and exact product validation

- Source commit 816146abeff90634d307069edb91ea338b274f2f preserves notify_rc_status("start_vpn_client") and aggregates/logs enqueue failures without skipping later WAN checks. Exact WAN route 44204417571 and Process 44204417899 PASS.
- Exact 816 WAN DHCP callback 44204417722, AI docs 44204417806 and USB modem identity 44204417584 PASS.
- Exact descendant main 4a9b812b39de402211d9614a5ac086ec1d61ab17: HTTPD Security 44204487769 PASS; AI handoff 44204487682 PASS; NVRAM 44204487741 IN_PROGRESS.
- XY-C1 44204488447 on 4a9b812 remains IN_PROGRESS at Build XY-C1; TRX and artifact steps pending. Descendant changes after the production source were workflow/docs-only; still require same-SHA Build/TRX/artifact success.
- No local MIPS build or hardware L4 claim. Continue unrelated P1 implementation while pending runs proceed.

---

## 2026-11-18 — Align HTTPD AP DHCP retry source contract

- Parent main SHA: `816146abeff90634d307069edb91ea338b274f2f`; exact WAN DHCP `44203603261` PASS. Exact Process `44203603303`, infosvr `44203603262`, AliyunDrive `44203603304`, AliDDNS `44203603381`, AdGuardHome `44203603460` PASS on `d7f4bc9f`; AI `44203929395` and NVRAM `44203929348` PASS on `62f315b1`.
- XY-C1 run `44202831040` on source `509409f8` completed SUCCESS; step-level Build/TRX/artifact confirmation is the outstanding L3 evidence.
- HTTPD's embedded DHCP checks were stale after the LAN writer was extracted: bound callback delegates to `lan_dhcp_publish_hosts_and_schedule_retry()`, and AP refresh clears `lan_hosts_retry` only after a successful write. THIS updates those assertions and makes the pull_request path filter match the already-covered push filter.
- No production C changes. Exact-head HTTPD/AI reruns pending; physical hardware L4 NOT VERIFIED.

---

## 2026-11-18 — Restore mandatory end-of-session handoff checkpoint contract

- Parent main SHA: d7f4bc9f2e41fb5764e807409d4fe9420ddd6bed. Exact WAN DHCP callback run 44203603261 PASS after the embedded source-contract fix in that parent.
- Exact AI docs run 44203603286 failed because the live handoff still had the heading but no longer contained the exact required sentence: "The checkpoint is mandatory; a document edit is conditional." The development protocol and bootstrap already contained the full checkpoint requirements.
- THIS commit restores the exact handoff literal and makes the verification steps explicit: inspect exact-SHA changes, compare handoff facts, and make no commit when facts/first action remain accurate. No production code is changed.
- Exact Process 44203603303, NVRAM 44203603328, AliDDNS 44203603381, AdGuardHome 44203603460, infosvr 44203603262 and AliyunDrive 44203603304 PASS on d7f4bc9f. Exact 509 XY-C1 run 44202831040 remained in progress at Build; no L3 claim.
- Next exact-head validation is AI handoff. Afterward continue the discovered WAN VPN-start notification return-status defect. No local MIPS or hardware L4 claim.

---

## 2026-11-18 — Align WAN DHCP callback workflow with extracted hosts retry helper

- Parent main SHA: `509409f8a9d72127bf40641ca779f0a43c8d5783`; exact 509 WAN DHCP run `44202830940` failed at embedded Python line 906 because its contract still required direct `create_hosts_lan()` and the corresponding failure message inside `udhcpc_lan_bound()`.
- The production implementation correctly extracts ancillary publication into `lan_dhcp_publish_hosts_and_schedule_retry()`; this helper performs the hosts write, AP retry marker, DHCPD restart notification and diagnostic. The standalone checker already verifies that helper's retry semantics.
- THIS commit changes only the embedded workflow test: assert that lease NVRAM publication precedes the helper call, and inspect the helper body for `create_hosts_lan()` and the expected error message. No production source changes.
- Exact 509 AI `44202830922`, AdGuardHome `44202830932`, Process `44202830930`, infosvr `44202831074`, AliDDNS `44202830944`, AliyunDrive `44202830935`, NVRAM `44202831128` PASS; XY-C1 `44202831040` IN_PROGRESS at last poll.
- Exact-head WAN DHCP rerun required; callback policy binary was not reached in the parent failure. No local target build or hardware L4 claim.

---

## 2026-11-17 — Make AP DHCP retry checker robust to C line wrapping

- Parent main SHA: `0af358f3cfd9083994054fa97d22371245246eef`; exact WAN DHCP run `44202380584` on `0af358f3` compiled and ran the DHCP lease policy binary successfully, then failed at `check_lan_dhcp_deconfig.py:89` because a helper call formatted over multiple C lines was compared against a one-line literal.
- Confirmed the implementation calls `lan_dhcp_hosts_refresh_required(is_changed, is_renew, retry_pending)` with the retry-pending state derived from AP mode and `lan_hosts_retry`; the wrap is whitespace-only.
- THIS checker repair normalizes whitespace, verifies the exact `refresh_hosts` assignment and both lease-change/renew arguments, and leaves production behavior unchanged. The existing workflow compiles/runs the callback policy regression with GNU89, ASan and UBSan after the checker passes.
- Exact-head WAN DHCP/AI/XY checks pending; parent XY-C1 `44202380583` and NVRAM `44202380759` were in progress at last poll. No local MIPS build or hardware L4 claim.

---

## 2026-11-17 — Retry failed AP LAN DHCP host-file publication

- Parent main SHA: `d7eea9c037ad0ec9074022c1e1da30d87002d7a9`; exact parent Actions IAPP `44201354894`, AI handoff `44201354982`, Process lifecycle `44201354872` PASS; XY-C1 `44201354975` was still building at last poll.
- Found a retry gap after successful lease transaction: failed AP `create_hosts_lan()` was deliberately non-fatal to BusyBox's lease commit, but an unchanged RENEW skipped host publication forever.
- Added temporary `lan_hosts_retry` state, a secure `notify_rc_status(RCN_RESTART_DHCPD)` immediate retry, and same-value RENEW retry restricted to hostname publication. AP event/start host refresh clears the marker only after a successful write; non-AP behavior and lease callback status remain unchanged.
- Added GNU89 policy regression and source-contract assertions for first failure, unchanged renewal, retry marker clear and notification ordering. Exact-head WAN DHCP/AI/XY-C1 pending; no local MIPS build or hardware L4 claim.

---

## 2026-11-18 — Correct WAN teardown cleanup declaration contract

- Parent main SHA: `853748bd0a49661ac29c7b854a58e40571393a20`.
- Exact 853 WAN interface teardown `44201908894` PASS; AI handoff `44201908973` PASS. WAN route `44201908899` failed only at embedded Python assertion line 220: source checker expected `int status = 0, route_status;` after the new teardown force-down path correctly added `interface_cleanup_status`.
- THIS commit updates the assertion to `int status = 0, route_status, interface_cleanup_status;`; production code is unchanged.
- Exact 853 XY-C1 `44201909002` and NVRAM `44201909080` were IN_PROGRESS at last poll. Exact route rerun pending; hardware L4 NOT VERIFIED.

---

## 2026-11-18 — Fail closed on static IPoE address cleanup during WAN teardown

- Parent main SHA: `a700cb6c77cd7f12074950d2d92b96b297dd910a`; exact a700 IAPP `44201354894` PASS and AI handoff `44201354982` PASS. Process `44201354872`, XY-C1 `44201354975`, and NVRAM `44201355174` were in progress at last poll.
- Found a distinct WAN-down edge: failure to clear a configured static IPoE address could leave the physical WAN interface administratively up with a stale address.
- THIS slice forces interface-down on clear failure, aggregates/logs any force-down failure without masking the first clear error, and continues independent teardown.
- Added GNU89 policy regression and source-order assertions. Exact-head WAN/XY-C1 checks pending. No local MIPS compile or hardware L4 claim.

---

## 2026-11-18 — Align IAPP UDP rollback assertion and restore handoff contract token

- Parent main SHA: `8e92caa7e3e3b6f66523c2fa6a051736d1030cc0`.
- Exact IAPP workflow `44201107917` on `8e92caa7` passed shell syntax and the rollback-diagnostic assertion after the earlier correction, then failed because the UDP failure test still expected the pre-ownership untagged TCP delete syntax. THIS repair now expects the tagged `ralinkiappd-tcp` rule, which matches the implemented ownership boundary.
- Exact AI handoff `44201108042` failed because `SESSION_HANDOFF.md` no longer contained the mandatory literal `one atomic main commit`; the full non-forced expected-SHA rule is restored.
- Exact Process lifecycle run `44201108009` was still IN_PROGRESS at last poll; its earlier failure was in the same IAPP checker invoked from the optional app status checker. Recheck after this commit.
- NVRAM and XY-C1 on `8e92caa7` were pending; this source SHA requires its own checks. No local shell/MIPS build or hardware L4 claim.

---

## 2026-11-18 — Align IAPP rollback regression with diagnostic

- Parent main `e5338402f12fb46e44c8e7a752863ed48fe14f4a`; exact IAPP run `44200753434` passed POSIX syntax and failed only at line 106 because the test expected the old `daemon startup rollback failed` message, while the updated rollback path emits `daemon/firewall startup rollback failed`.
- THIS test-only correction updates the precise diagnostic assertion; production IAPP lifecycle behavior is unchanged.
- Exact e533 AI handoff `44200753459` and Process inventory `44200753122` PASS. Process lifecycle run `44200753226` failed in its lifecycle status propagation step; NVRAM `44200753245` and XY-C1 `44200753260` were pending (XY Build/TRX/artifact not started).
- Next: exact-head IAPP rerun, Process job log diagnosis, and NVRAM/XY-C1 status check. No local shell/firmware or hardware L4 claim.

---

## 2026-11-18 — Reconcile Ralink IAPP firewall rules with daemon lifecycle

- Parent main SHA: `b4ffae1ce9eae0f05f07bdcd9ed488ac38e101c0`; `iappd.sh` previously installed two port-3517 INPUT allow rules but never removed them on normal stop, leaving access open after service disable and allowing repeated restart to accumulate duplicates.
- Add comment ownership tags, inspect filter state before deletion, bound duplicate cleanup, remove stale owned rules before startup, and clean active rules only once daemon absence is confirmed. Foreign untagged rules with the same tuple are not deleted.
- Extend shell mock to model iptables-save/append/delete and test ordinary stop, stale-rule cleanup, start/stop/restart idempotency, foreign rule preservation, and failed cleanup followed by successful retry. Add dedicated IAPP workflow and XY-C1 pre-build validation.
- Exact-head tests are pending. Parent `2a98a830` AI, WAN DHCP, Process, MiniUPnP, firewall, and other module runs were PASS except NVRAM transaction `44200079736` and XY-C1 `44200079523` still IN_PROGRESS at last poll. Prior exact `0d58928e` XY-C1 completed successfully; this new source requires its own build.

---

## 2026-11-18 — Preserve optional post-WAN down-hook exit status

- Parent main SHA: `2a98a8303c26414a8f5084b78f1642ea282d164c`; exact WAN route `44200079558` and AI handoff `44200079479` for the preceding post-WAN up-hook source commit both PASS.
- Found the matching discarded result in `wan_down_internal()`: configured `SCRIPT_POST_WAN` was invoked with the `down` event but its exit status was ignored.
- THIS slice captures, aggregates and logs the down-hook result without skipping later PPPoE-MWAN detection/reconfiguration. Added a GNU89 first-error/later-action regression and exact source-order contract.
- Exact-head WAN/AI/XY-C1 verification pending. The preceding `2a98a83` XY-C1 run `44200079523` and `93c8ab7` XY-C1 run `44198950404` were still building at the last poll; factory-reset source `0d58928e` exact XY-C1 run `44197454090` passed Build/TRX/artifact. No local MIPS build or hardware L4 claim.

---

## 2026-11-18 — Preserve optional post-WAN script exit status

- Parent main SHA: `93c8ab702d886e7da1fe145bef5655c37dbe4a6d`; exact WAN route `44198950271` and AI handoff `44198949896` for the static-IPoE address cleanup slice both PASS.
- Found a remaining discarded return in `wan_up_internal()`: the configured `SCRIPT_POST_WAN` was run with `eval()` but its nonzero exit status was ignored, although WAN-up returns accumulated status from other optional starts.
- THIS slice captures, aggregates and logs the hook result without short-circuiting later independent ZeroTier/PPPoE-MWAN actions. Added a GNU89 first-error/later-action regression and extended the existing WAN post-start source contract.
- Exact-head WAN/AI/XY-C1 verification pending. The prior factory-reset source `0d58928e` passed exact XY-C1 Build/TRX/artifact on run `44197454090`; `93c8ab7` XY run `44198950404` was `IN_PROGRESS` at last poll. No local MIPS build or hardware L4 claim.

---

## 2026-11-18 — Fail closed after partial static IPoE address setup cleanup fails

- Parent main SHA: `b83c1cbf99d457c5155a4d613c49f8871f3c1fff`; previous factory-reset source `0d58928e` GPIO regression `44197451556` PASS, and the required AI handoff checkpoint contract passed on `b83c1cb` run `44197796498`.
- Found a separate static IPoE failure path: after configured IPv4 assignment failed, clearing a partially configured address could also fail, but the result was only logged and the physical interface stayed administratively UP.
- THIS source slice propagates the clear error, forces the WAN interface down, preserves the first address-configuration failure, records a failed force-down, and skips EAPOL/remaining IPoE protocol activation for that WAN unit. Dynamic IPoE and successful cleanup behavior are unchanged.
- Added GNU89 policy regression and source-order/protocol-gating assertions. Exact-head WAN and XY-C1 validation pending; the older `0d58928e` XY-C1 run `44197454090` had reached Build XY-C1 but had not completed at last poll. No local MIPS build or hardware L4 claim.

---

## 2026-11-18 — Record factory-reset CI results and close handoff checkpoint contract

- Exact source SHA `0d58928ee6ab852cbbc37f4a2201a685c8252e1e`: GPIO button run `44197451556` PASS, including the new GNU89 ASan/UBSan factory-reset sequence regression; Process `44197451701` PASS; MiniUPnP `44197451163` PASS.
- AI docs run `44197451610` failed only because SESSION_HANDOFF did not explicitly require an end-of-session accuracy checkpoint. THIS documentation commit adds the mandatory section and states that a checkpoint is required but a file edit/no-op commit is conditional.
- XY-C1 run `44197454090` was QUEUED; no L3 claim until exact-head Build, TRX verification and artifact upload pass. Local host/MIPS tests NOT RUN; physical-router L4 NOT VERIFIED.

---

## 2026-11-18 — Propagate factory-reset MTD and NVRAM failures

- Parent main SHA: `80dcdc629d72c11136e1b766fe730e0d33f21bd4`; runtime changes span `rc.c`, `rc.h`, `btn_action.c` and `gpio_btn.c`, with a new GNU89 policy regression, source-order checker and GPIO workflow coverage.
- The old `void erase_storage()` discarded the MTD script's wait status. Both button-reset paths ignored `erase_nvram()` and always signalled PID 1; a failed rstats stop/flash erase or a failed durable reset-marker commit could still reboot into mixed persistent state.
- Return the MTD erase status, perform storage erase before committing the NVRAM reset marker, and request PID 1 exit only if both persistent steps succeed. On every abort, attempt to restore configured rstats; the physical reset-button caller restores its normal power LED when reset fails. The source checker verifies short-circuit order, MTD exit-status preservation and both button paths.
- Exact-head GPIO regression and XY-C1 Build/TRX/artifact are pending. Local host tests were NOT RUN (no full checkout/toolchain in this session). Physical-router L4 remains NOT VERIFIED.

## 2026-11-18 07:11 CST (UTC+8) — Match EAPOL source checker to multiline first-error call

- Parent main SHA: `5d0618cc29aa67cc11d93a6487f2d5bc71c777ce`.
- The exact-head WAN route run `44196817787` compiled and ran the existing ASan/UBSan regressions successfully, then the expanded source checker failed at its first EAPOL ordering assertion. Production code is correctly formatted as a two-line `rc_first_service_start_status()` call; THIS checker correction splits the expected call across its actual two source lines rather than looking for a nonexistent single-line form.
- No production source or policy-test behavior changes in THIS correction. Exact-head WAN and XY-C1 reruns remain required; run `44196817911` was still in cross-build setup at last poll.
- The exact XY-C1 build for the earlier MiniUPnP source `03a993e` is already complete on `88770d7`: Build, TRX verification and artifact upload all SUCCESS. The present WAN change is separate and needs its own exact-SHA firmware build.

---

## 2026-11-18 07:08 CST (UTC+8) — Fail closed on static IPoE EAPOL address cleanup

- Parent main SHA: `fa36ad1823344790f17511bef5890df3a9f78b09`. MiniUPnP lock-check repair is complete: exact fa36 MiniUPnP run `44195714916` and AI docs `44195714933` PASS; Process `44195398478` and AI docs `44195398394` pass on dc6.
- Exact XY-C1 run `44193165931` for product source baseline `88770d7` completed Build XY-C1, TRX verification and artifact upload. Descendant MiniUPnP work changed test/workflow/docs only.
- Found a concrete P1 WAN defect: static IPoE EAPOL failure attempted to clear the WAN IPv4 address but discarded the ifconfig result. The interface could remain UP with a stale address. THIS change captures/logs/aggregates the clear error and forces the interface down; failed interface-down status is also preserved, while the original EAPOL error remains first.
- Added static-vs-dynamic and clear-success/error policy regression and extended the WAN checker to enforce operation order and diagnostics. Exact WAN and XY-C1 checks pending; hardware L4 NOT VERIFIED.

---

## 2026-11-18 07:01 CST (UTC+8) — Validate MiniUPnP lock-release status in source contracts

- Parent main SHA: `654c146b2d0dcf5cbb1187a544ddaf429edc8c55`; production lock implementation remains `03a993e558b23f5f163ca1a904cbf7b54f6ef7e9`.
- Exact e74 Process `44194600337` and MiniUPnP `44194600347` both failed because source contracts expected direct `return ret;` from the public restart wrapper. Exact 654 MiniUPnP run `44195030330` repeated the failure after commit 654 only normalized the return-code predicate. Correct contracts must assert `return miniupnpd_lifecycle_lock_finish(lock_fd, ret, "restart");` and reject a direct return that would hide unlock failure.
- The Process checker has duplicated copies of MiniUPnP start/restart assertions. THIS commit updates every remaining stale copy to inspect `start_upnp_locked()`, call `stop_upnp_locked` beneath restart, and validate the lock-finish return.
- Exact e74 AI handoff docs `44194600326` PASS. Exact 654 AI docs `44195030309` was pending at last query. Parent `88770d7` NVRAM `44193165743` completed SUCCESS; XY-C1 `44193165931` was still in Build with TRX/artifact pending.
- No production runtime source changed. Exact-head Process/MiniUPnP reruns and the XY-C1 completion check are next. Physical-router L4 NOT VERIFIED.

---

## 2026-11-18 — Align MiniUPnP lifecycle integration contracts with lock wrappers

- Parent main SHA: `03a993e558b23f5f163ca1a904cbf7b54f6ef7e9`, which introduced stable-inode lifecycle serialization around MiniUPnP public start/stop/WAN/update/restart entry points.
- Exact-head Process run `44192259665` compiled and passed the process inventory, service lifecycle, PID-file, SSH and earlier C policy regressions, then failed in `scripts/check_modem_wan_start_status.py:671`: its WAN firewall contract still expected `rc_upnp_restart_run(stop_upnp, restart_firewall)`, but the now-locked implementation correctly uses `stop_upnp_locked` while holding the lifecycle lock. XY-C1 `44192259641` stopped at the same stale assertion before Build.
- Dedicated MiniUPnP run `44192259644` passed atomic config publication and GNU89 daemon policy tests, then failed its embedded source checker because it still extracted public wrappers as though they contained the internal start/stop/WAN/update operations. Update it to inspect the `*_locked` operation bodies and verify the public restart lock ordering.
- Exact 03a AI docs `44192259762`, Samba `44192259623`, Inadyn `44192259711`, DNSMasq `44192259636`, infosvr `44192259808`, AdGuardHome `44192259712`, AliDDNS `44192259810`, WAN route `44192259691` and HTTPD security `44192259847` passed. MiniUPnP-specific Process failure `44192259665` and MiniUPnP test-workflow failure `44192259644` are checker-only at the first causal failure; NVRAM `44192259875` and AliyunDrive `44192259658` were still running at last poll.
- THIS commit corrects source contracts and ensures the dedicated workflow watches/runs the new lock policy checker; no production behavior changes. Exact-head Process, MiniUPnP, AI and XY-C1 checks must rerun. XY-C1 Build/TRX/artifact on 03a was skipped after its pre-build checker failed; no L3 claim.

## 2026-11-18 — Serialize MiniUPnP lifecycle transitions across RC callers

- Parent main SHA: `7bd030fa1e515250e14b6c3a075901eb55d6bfa2`; source slice changes `trunk/user/rc/src/services_ex.c`, adds `miniupnpd_lifecycle_lock.h` and `tests/miniupnpd_lifecycle_lock.c`, adds `scripts/check_miniupnpd_lifecycle_lock.py`, and integrates the test in Process lifecycle workflow.
- Audit found public MiniUPnP start, stop, WAN-interface cleanup, lease reload and firewall restart entry points had no shared lifecycle lock. Start scanned ABSENT then published config before launch, allowing concurrent RC/WAN notifications to both pass preflight and interleave config/start/stop.
- One stable-inode private fcntl lock now serializes every public state-changing entry point. Internal `*_locked` helpers prevent recursive acquisition. Restart retains lock through firewall refresh; update covers inventory/decision/signal; WAN check covers scan and optional stop.
- Startup re-scans daemon presence after config publication and immediately before exec. The GNU89 ASan/UBSan regression covers cross-process contention, hardlink/symlink rejection, invalid paths and release; the Python checker validates wrapper coverage and ordering.
- Exact-head Process, AI handoff and XY-C1 Build/TRX/artifact checks pending. No hardware L4 claim.

## 2026-11-18 — Restore required cross-session handoff heading

- Parent main SHA: `0ab0849c47cb210a24264236721c18c7f4dc077c`.
- The AI handoff checker on exact `0ab0849c`, run `44191156095`, passed the restored 32-entry `dev_log.md` retention check but failed the next exact-text contract: `SESSION_HANDOFF.md` was missing the literal `# Cross-session execution mandate` heading. The prior top-checkpoint replacement had inadvertently dropped the heading while retaining its body rules. THIS correction restores the required heading immediately before those rules.
- Other exact `0ab0849c` checks passed: Process lifecycle `44191156121`, PID1 notification queue `44191156366`, infosvr `44191156248`, AliDDNS `44191156160`, AliyunDrive `44191156171`, and AdGuardHome `44191156036`. NVRAM `44191156088` remained in progress.
- The production-source XY-C1 run is on f9 source SHA `f9afd25e`, run `44190333512`; all pre-build gates passed but Build XY-C1 was still in progress, with TRX/artifact pending. The 0ab correction changes only CI assertions/docs, so XY-C1 is not retriggered by its path filter.
- This documentation-only commit corrects the exact handoff contract; the AI check must be rerun on this SHA. Hardware L4 remains NOT VERIFIED.

## 2026-11-18 — Repair PID 1 SSH lifecycle contract and restore rolling dev log

- Parent main SHA: `f9afd25e8957de96c5661f9c0d00c29cf5d22da3`; source change `fix(sshd): verify daemon exit with identity-bound stop` routes C-controlled SSH stop through `kill_services()`, with Dropbear-first/OpenSSH fallback selection.
- Exact f9 Process lifecycle `44190334132` and Process inventory/lifecycle regression `44190334289` passed, including SSH policy compile and source checker. PID 1 notification workflow `44190333392` compiled and ran all policy binaries, then failed its Python source contract because it still expected the pre-refactor `start_sshd()` to directly return `eval()`; update the contract to require captured launcher status, bounded RUNNING postcondition, and identity-bound `stop_sshd()`.
- AI handoff docs run `44190333813` failed because the parent commit replaced the 31-entry rolling `dev_log.md` with one entry. Restore the complete 31-entry file from parent `07b8266e` and prepend this checkpoint, retaining the >=20-entry contract.
- Exact 6a5 source/test SHA completed all gates including XY-C1 `44188951477` Build, TRX verification and artifact upload. On f9, XY-C1 `44190333512` remained at Build at last poll; AliyunDrive rerun `44190333972` completed success; AdGuardHome `44190334184` and NVRAM `44190334351` were in progress.
- This correction changes only PID1 checker expectations and restores development history/document accuracy. Exact-head PID1/AI/XY checks must pass on this correction. Physical-router L4 remains NOT VERIFIED.

## 2026-11-18 - Scope HTTPD checker ordering to the firewall rollback block

- Parent main SHA: 6a5f1477771ecb1f4aa612e59a12472937051e0f; changed scripts/check_modem_wan_start_status.py, SESSION_HANDOFF.md and REFACTOR_ROADMAP.md.
- The first checker adjustment 773642be removed the obsolete unconditional firewall-error return requirement, but its new order assertion accidentally selected the initial pre-launch httpd_started=0 assignment. Exact USB modem WDM/NDIS, Firewall Startup and XY-C1 jobs exposed this as line 635.
- Scope the expected order to the block beginning at firewall_status = restart_firewall();. The checker now ensures error capture -> stop gate -> locked stop -> fresh process scan -> ABSENT/uncertain state publication -> original errno/status return.
- Exact 6a5f1477 Firewall Startup 44188951449 and AI docs 44188951370 passed. Process 44188951409, USB modem WDM/NDIS 44188951386 and XY-C1 44188951477 were pending/in progress at last check.
- Frozen backup 72f4f157a084f8184b9597ca5bf334b8080e5807 unchanged; no XY-C1 L3 or physical-router L4 claim.

## 2026-11-18 — Verify SSH detached startup and fail-closed restart preflight

- Parent main SHA: `f1f3edd6961e2ea7f20b63da76f2304e0ab72db3`. On the parent HTTPD slice, exact-head HTTPD security `44186935262`, HTTPD lifecycle `44186935549`, and AI handoff `44186935541` passed. Process lifecycle `44186935557`, WAN route `44186935517`, NVRAM `44186935696`, and XY-C1 `44186935585` were still running at last inspection.
- `start_sshd()` previously trusted the detached OpenSSH/Dropbear wrapper status. It now uses the shared bounded startup policy (one-second initial wait, up to four process-inventory probes); only a confirmed RUNNING child returns success. UNKNOWN preserves the inventory error, continued ABSENT returns `ETIMEDOUT`, and launcher failures remain visible.
- `restart_sshd()` previously invoked the broad name-based stop script even when its initial process inventory was UNKNOWN. It now aborts before stop on UNKNOWN, skips unnecessary stop when already ABSENT, and only stops after a confirmed RUNNING snapshot; the post-stop scan must still confirm ABSENT before start.
- Extend the existing GNU89/ASan/UBSan SSH policy test and `check_sshd_firewall_status.py` source-order contract. Existing Process workflow compiles/runs the policy binary; XY-C1 tracks changed production `services.c`.
- Exact-head Process and XY-C1 checks for THIS source commit are pending. Parent CI is not a substitute for this SHA's firmware validation. No MIPS/uClibc L3 or physical-router L4 claim.

## 2026-11-18 — Withdraw HTTPD listener when firewall refresh fails

- Parent main SHA: `0a297f48326221209bdbdf57173db719cd3a79b8`; production path `trunk/user/rc/src/services.c`, policy `httpd_daemon_policy.h`; regression: GNU89 HTTPD daemon policy and existing HTTPD lifecycle source contract.
- `start_httpd_locked()` could publish `httpd_started=1`, then return a required public HTTP/HTTPS firewall refresh error while leaving the listener alive under potentially stale or incomplete rules.
- On refresh failure, invoke `stop_httpd_locked()` under the existing lifecycle lock, rescan process inventory, publish `httpd_started=0` only for confirmed PROCESS_PRESENCE_ABSENT, otherwise retain `1` and log uncertainty. Preserve original firewall errno and status.
- Add `httpd_firewall_failure_requires_stop()` and six GNU89 unit cases across zero/error and ABSENT/RUNNING/UNKNOWN/invalid states. Extend workflow assertions to require stop → inventory postcondition → state publication before returning the firewall error.
- Parent SSH SHA `36b2b171`: Process `44185018675` and WAN route `44185018710` passed. XY-C1 `44185018862` was still building at last observation. THIS source SHA requires fresh HTTPD/Process/XY-C1 verification; router hardware L4 remains NOT VERIFIED; frozen backup is `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## 2026-11-18 — Withdraw SSH listener when public firewall refresh fails

- Parent main SHA: `7ffa87073cb7a18fadd5c7dc0fdbdd794b272f20`; production paths `trunk/user/rc/src/services.c` and `sshd_daemon_policy.h`; regression: GNU89 SSH policy test plus `scripts/check_sshd_firewall_status.py`.
- `restart_sshd()` could start a listener after a process-state transition, call `restart_firewall()` for enabled public SSH, then return the refresh error without stopping the newly active listener. That could leave SSH reachable under an old or incomplete ruleset.
- Add `sshd_firewall_failure_requires_stop()`: nonzero refresh requires shutdown unless state is confirmed ABSENT. On failure invoke `stop_sshd()`, rescan the daemon, and treat only PROCESS_PRESENCE_ABSENT as confirmed withdrawn. RUNNING/UNKNOWN is logged as unresolved; original refresh errno is preserved.
- Add GNU89 `-Wall -Wextra -Werror` ASan/UBSan coverage for zero/error and ABSENT/RUNNING/UNKNOWN/invalid states; add a source-order contract; wire into Process CI and PR path filters.
- Prior 319fd181 passed exact-head XY-C1 Build/TRX/artifact. PPPoE-MWAN follow-ups through `7ffa8707` require exact-head validation. THIS SSH source commit needs fresh exact-head Process/XY-C1 checks; router L4 remains NOT VERIFIED; backup frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## 2026-11-18 — Correct PPPoE-MWAN source-checker newline delimiters

- Parent main SHA: c8b8ba8c107bebb2dd73f3390f2e0fa53f68a86e; path scripts/check_pppoe_mwan_status.py.
- The first detector gate attempt (ac2b1f6) failed its baseline route assertion because the new helper incorrectly set MWAN_FLUSH_FAILED on successful iptables deletions. c8b8ba8 now only poisons the phase for nonzero statuses.
- While preparing the exact rerun, inspect the Python contract source: the generated strings contained two backslashes, so Python searched for literal \n text rather than actual newline characters. Use chr(10) in both the helper-body and pre-policy boundary lookups.
- Parent 319fd181 XY-C1 run 44180704552 passed pre-build regressions, Build XY-C1, TRX verification and artifact upload. Current PPPoE detector changes still await new-SHA verification.

## 2026-11-18 — Only poison PPPoE-MWAN cleanup on nonzero status

- Parent main SHA: ac2b1f630e5bb0a51fd75aa6a7c090d2d7865c58; changed detect.sh and its Python source contract plus checkpoint docs.
- Exact ac2b1f6 WAN-route run 44183697784 and XY-C1 run 44183697726 both failed first at the pre-existing normal multipath default-route test. Root cause was the new cleanup recorder setting MWAN_FLUSH_FAILED unconditionally even for successful iptables -D (status 0).
- Guard the cleanup poison assignment with a nonzero status check; successful deletions leave cleanup state healthy, while snapshot, parse and deletion failures still block new rules/routes. Extend source checker to require this zero-status guard.
- The four new failure-injection branches were not reached in the failed run and remain unverified until exact-head rerun.
- Parent 319fd181 XY-C1 run 44180704552 did pass all pre-build checks, Build XY-C1, TRX verification and artifact upload; target L3 proven at that SHA. Router hardware L4 not run.

## 2026-11-18 — Prevent PPPoE-MWAN policy rebuild after failed cleanup

- Parent main SHA: 319fd18123df5e47929cbe1c27f2989fd129bef4; changed detect.sh, detect_status.sh, check_pppoe_mwan_status.py and XY-C1 workflow atomically with handoff/roadmap/log.
- The detector previously recorded iptables-save, malformed snapshot parsing and iptables -D failures but continued adding new mangle/NAT rules and policy/default routes. This could layer a new multi-WAN policy over stale or partially removed rules.
- A cleanup failure now poisons the previous-policy phase while all three independent cleanup tables are still attempted. The detector flushes route cache, preserves the first error, removes its owned lock via EXIT trap, and exits before any new mangle/NAT/policy/default route writes.
- Added shell failure-injection cases for failed mangle snapshot, failed NAT snapshot, rule-deletion failure and malformed saved rule; each asserts no new policy or route mutation and cache cleanup. XY-C1 now runs this detector suite before Build in addition to WAN-route CI.
- On parent 319fd181, WAN DHCP, Firewall Startup, Process, USB modem, NVRAM, AI docs, AdGuardHome, AliDDNS, AliyunDrive and infosvr exact-SHA Actions passed. Its XY-C1 run 44180704552 had pre-build regressions green and Build XY-C1 in progress; this new SHA requires its own WAN/XY verification. No hardware L4 claim.

## 2026-11-18 — Correct RAS startup aggregation count and WAN DHCP static-IPoE boundary

- Parent main SHA: `62fbefe3d562c332c9929b22f6d2d8ea078e1ade`; only regression contracts and handoff/log changed.
- Exact-head `659c0720` runs Process `44179526342`, Firewall Startup `44179526456`, USB modem `44179526516`, and XY-C1 `44179526334` advanced through the earlier source-boundary fixes and stopped at `scripts/check_modem_wan_start_status.py:315`: the checker still expected four `modem_start_status` aggregates. The current `start_wan()` path has five distinct results, including USB-net startup, RAS interface migration, RAS firewall rebuild, firewall-failure physical-WAN restoration, and RAS launch. The checker now expects five and explicitly verifies RAS restoration is followed by result aggregation.
- Exact-head `62fbefe3` WAN DHCP callback `44179778384` passed the APCLI call-boundary slice but failed later at embedded Python line 852 because its static-IPoE block still ended at the removed legacy iptables comment. That inline checker now ends at the actual `vlan_result = start_firewall_ex();` gate after static address configuration.
- On `659c0720`, AI handoff `44179526396`, AdGuardHome `44179526443`, infosvr `44179526331`, AliyunDrive `44179526451`, and AliDDNS `44179526391` passed. Process/Firewall/USB/XY failures were at the shared checker count, not production compile failures; XY-C1 Build/TRX/artifact were skipped.
- Exact `62fbefe3` AI handoff `44179778471` and AliyunDrive `44179778466` passed; WAN DHCP `44179778384` failed only at the second stale inline source boundary. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`. No target L3 or router L4 result claimed.

## 2026-11-18 — Replace the final stale AP-client boundary in WAN DHCP CI

- Parent main SHA: `659c0720a1d0baccdd5ec3644a9d2177b199bf69`; workflow source-contract correction only, no production C changes.
- After the common modem checker was advanced to the current IPoE firewall gate in `659c0720`, one duplicated stale AP-client comment remained in `.github/workflows/wan-dhcp-callback-stability.yml`. Its physical-interface setup slice now ends at the real `vlan_result = reconnect_apcli(wan_ifname, 0);` caller.
- The obsolete marker was previously exposed by the exact `8e9ea213` WAN DHCP callback run `44177798012`; this workflow was not triggered by `659c0720`, so no result on the newer source SHA is claimed yet.
- This commit must trigger and validate WAN DHCP callback CI. Other exact-head `659c0720` workflows were still in progress at handoff; inspect their corresponding run IDs before recording results. No MIPS build or router L4 claim. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## 2026-11-18 05:32 CST — Align modem checker with the new IPoE firewall gate boundary

- Parent main SHA: `ece8011f4cd14ed37e24ab6a95d2b847196d6d0a`; path: `scripts/check_modem_wan_start_status.py`.
- Exact-head ece8011 run failures: Firewall Startup `44178773543`, Process `44178773567`, and XY-C1 `44178773562`. Their GNU89 service status binary and earlier firewall-specific regression passed; all three stopped in the later modem checker at line 304 because it searched for the removed `/* re-build iptables rules (final stage for IPoE static) */` marker.
- Replace the static-address validation end marker with the current `/* Rebuild required firewall policy before EAPOL, DHCP or route activation. */` comment. This preserves the checker’s intended scope (address configuration up to firewall stage) rather than weakening assertions.
- Exact-head ece8011: WAN route `44178774262`, Wireless radio `44178774053`, AI handoff `44178773961`, AdGuardHome `44178773571`, AliyunDrive `44178773589`, infosvr `44178773674`, and NVRAM `44178774246` PASS. XY-C1 Build/TRX/artifact remained skipped before this correction; no L3 claim.

## 2026-11-18 05:16 CST — Repair exact-head WAN and APCLI source-contract drift

- Parent main SHA: `8e9ea2131d7be59833756c15d03cf58ba288085c`; paths: `scripts/check_modem_wan_start_status.py`, `scripts/check_wan_initial_mtu_status.py`, and `.github/workflows/wireless-radio-state.yml`.
- Exact-head Firewall Startup `44177798054` failed after all firewall policy binaries and its firewall caller checker passed; the later modem checker still searched for the old `/* perform ApCli reconnect */` comment. Process `44177797717` compiled and ran the updated GNU89 service-start policy regression successfully, then stopped at the same checker boundary.
- WAN route `44177797840` passed its earlier firewall result propagation contract, then its initial MTU checker searched for the former IPoE firewall comment. Update both checkers to current source boundaries.
- Wireless radio `44177798027` passed its C/policy, bridge and guest-isolation tests but its APCLI source checker counted both command and log strings. Count the four actual `doSystem(...ApCliAutoConnect...)` invocations. Also add the missing `net_wan.c` path to the PR filter and checker list.
- AI handoff `44177797925` PASS; XY-C1 `44177798165` was in progress at last poll. No code-level failure was observed in the preceding firewall or service-status regression steps. Exact-head reruns pending; MIPS build/hardware L4 not yet claimed.

## 2026-11-18 04:48 CST — Fail closed WAN protocol startup on firewall rebuild errors

- Parent main SHA: `be0f5b56efa203fc802334e08b846381e9a3f04a`; source paths: `trunk/user/rc/src/net_wan.c`, `trunk/user/rc/src/service_start_status.h`, `trunk/user/rc/src/tests/service_start_status.c`, and `scripts/check_wan_firewall_status.py`.
- `start_wan()` previously accumulated `start_firewall_ex()` errors but continued to launch USB RAS/PPP clients and IPoE EAPOL/DHCP/route setup. The firewall API reports nonzero when the requested ruleset or configured extension hooks/snapshot cleanup are incomplete; reporting the error without gating protocol launch could expose WAN traffic under stale or partial policy.
- Added `rc_wan_firewall_start_allows_protocol()` and regression cases for zero, negative, EIO, EBUSY and EACCES results. The firewall source checker now asserts all three gate-before-launch paths and recovery/cleanup ordering.
- USB RAS and PPP launch are skipped on firewall failure; physical-WAN policy restoration is retried, with interface-down fallback if restoration fails. IPoE EAPOL/DHCP/route/IPv6 setup is skipped; static address cleanup is attempted and interface-down is the fallback if that clear fails. First-error aggregation is preserved.
- Standalone GNU89 policy harness with `-Wall -Wextra -Werror -fsanitize=address,undefined`: PASS. Full repository C regression binary, full Python source checker and MIPS XY-C1 build were not run locally. Exact-head WAN-route/XY-C1/AI handoff Actions remain pending/not verified; no target L3 or L4 claim. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## 2026-11-17 — Propagate AP-client reconnect driver failures into WAN startup

- `reconnect_apcli()` in `net_wifi.c` was `void` and discarded all `doSystem()` results for APCLI `ApCliAutoConnect` and forced `ApCliEnable=0/1`. Failed driver commands could leave WISP unassociated while `start_wan()` still returned success.
- Return normalized status using `wireless_radio_restart_status()`; retain the first failure while still attempting forced re-enable after disable error. `start_wan()` aggregates the error and continues other WAN setup; manual WISP reassociation logs nonzero status. No-op cases remain successful.
- Extend GNU89 wireless policy regression and exact source contract across `net_wifi.c`, `rc.h`, and `net_wan.c`; path filters now cover the WAN caller. Exact-head Wireless radio Actions pending; local host compile not available in this runtime.
- Preceding verifications: Samba `44173796606`, Process `44174094871`, NVRAM `44174094746`, infosvr `44174094855`, AI docs `44174094903`, AliDDNS `44174094743`, AdGuardHome `44174094915`, AliyunDrive `44174094822` PASS.

## 2026-11-17 — Correct Process lifecycle syslogd/klogd source-boundary escapes

- Process exact `222cbfba37ef2e4c17ae13bd9275ed648c6ab407` run `44173365883` reached the syslogd/klogd source-contract section after prior C/RC integration checks passed. Its Python checker split on `"int\\\\nstart_syslogd(void)"` and `"\\\\n}"`, which searched literal backslash-n rather than C newlines, causing `IndexError`.
- Correct both syslogd and klogd delimiters to Python newline escapes. No production code or test binary changes in this correction. Current-head Process rerun `44173796903` was in progress at last poll.
- Samba exact run `44173796606` on current ancestor `6174c78` passed after the preceding whitespace-insensitive checker correction. Current exact NVRAM `44173796346` remained pending.

## 2026-11-18 — Normalize Samba WINS restart source-contract spacing

- The exact Samba run `44172985076` on `2e6974211a9e3a487913076b84dfe89eb9817b0d` passed its GNU89/Werror/ASan/UBSan C binaries, including the WINS restoration decision matrix, but the final integration checker failed at a one-line spelling assertion for a deliberately wrapped helper call.
- Update the checker to remove whitespace from the extracted `restart_smbd()` body before matching the complete argument sequence. This addresses the checker false negative; no production C change is needed.
- Exact rerun of the Samba workflow is pending. NFS `44172985066`, AI docs `44172984806`, and AdGuardHome `44172984782` passed on that source SHA.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-18 — Verify detached syslogd and klogd children after launcher success

- Parent main SHA: `2e6974211a9e3a487913076b84dfe89eb9817b0d`; THIS COMMIT modifies RC startup, Process source integration contracts/PR filters and the live handoff/roadmap.
- BusyBox syslogd forks by default unless `-n` is supplied; RC's argv has no `-n`. The parent returns after `bb_daemonize_or_rexec()`; the detached child writes its PID file and runs `do_syslogd()`. RC `_eval()` waits only for the launcher.
- BusyBox klogd also daemonizes by default unless foreground mode is requested; its detached child opens kernel log and enters the read loop while RC `eval()` only sees the launcher.
- Add optional-daemon tri-state preflight and bounded child checks. Already-running is success; UNKNOWN fails closed; nonzero launcher status is preserved; only RUNNING after a 1s initial delay and four probes returns success; UNKNOWN post-scan preserves errno; ABSENT reports `ETIMEDOUT`.
- Reuse GNU89-tested `process_presence_start_postcondition()`. Source checker verifies BusyBox daemonization plus RC launch/preflight/poll/result order; Process workflow push/PR filters cover the BusyBox sources.
- Parent `db362b5` Process fail-closed `44171722700` PASS; NVRAM `44171722663` pending. Parent `9be5d29` HTTPD `44171043495` PASS; XY-C1 `44171043411` pending. Exact-head Process/XY-C1 for THIS slice pending; no hardware L4 claim.

## 2026-11-18 — Restore WINS after restarting disabled Samba shares

- Review found that `restart_smbd()` force-stops `nmbd`, then selects `run_samba()` solely from mounted-storage presence. `run_samba()` returns success immediately if `enable_samba=0` or `st_samba_mode=0`; with mounted storage and `wins_enable=1`, the restart silently left the independent WINS daemon down.
- Added a fail-closed restart-action policy that selects Samba only when shares are enabled and storage is mounted; otherwise it restores WINS via `start_wins()` (which independently respects `wins_enable`). Invalid enablement or mount-state inputs abort.
- Extended `samba_process_policy.c` with GNU89 cases covering Samba enabled/disabled × mounted/empty, plus negative/invalid inventory and invalid enablement. Updated the Samba workflow’s embedded production source contract to enforce the new branch and unit-test wiring.
- Local full Samba process-policy test compiled with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan: PASS. Local targeted decision-policy matrix also PASS.
- Exact-head Samba stability and XY-C1 runs for this source commit are pending; no target build or physical-router L4 result is claimed yet. Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.

---

## 2026-11-17 — Fail closed on IPTV DHCP NAK/expiry/release cleanup

- The first IPTV DHCP deconfig implementation restored the old address when route deletion failed. Review showed this could resurrect a lease already revoked by NAK, expiry or release. The policy now never restores the address and attempts route cleanup even when address clear fails; empty NVRAM address publishes only when both complete. BusyBox retries the retained lease/route identity after an error.
- Ensure igmpproxy teardown still runs after either cleanup failure and aggregate the first interface/route/multicast error. Extend GNU89 regression with simultaneous failure and route-retry cases; update both WAN DHCP and HTTPD workflow contracts for the transactional dispatch and error handling.
- Exact product-source WAN DHCP run 44170820583 PASS, including GNU89 -Wall -Wextra -Werror with ASan/UBSan and semantic source contracts. HTTPD checker run 44171043495 on 9be5d29 PASS after fixing the escaped-newline endpoint. Exact current-source XY-C1 44171043411 remains IN_PROGRESS; Build/TRX/artifact not yet verified.
- Concurrent rstats/LLTD source `92d95b8` Process run 44169139271 PASS; XY-C1 44171043411 pending. Backup branch remains frozen.

## 2026-11-18 — Verify detached rstats and LLTD startup children

- Parent main SHA: 9e6d19f6ca53cbfeeda33e109abf28b6251f4000; this commit modifies RC startup, Process integration contracts/workflow filters and handoff/roadmap.
- rstats_main() daemonizes before alloc_rstats() and process_pidfile_write(RSTATS_PID_FILE, "rstats"); the child can exit on either failure while the parent launcher has already returned zero.
- LLTD main() calls osl_become_daemon(); that implementation double-forks and calls setsid(). The detached child writes its interface PID file, drops privileges and enters event_mainloop() only after the parent exits.
- start_rstats() and start_lltd() now retain their disabled sentinel, preflight tri-state presence, treat already-running as success, fail closed on UNKNOWN, and verify post-launch RUNNING with the shared 1s + four-probe bounded policy. ABSENT after attempts reports ETIMEDOUT; launcher errors are preserved.
- Reuse the GNU89-tested process_presence_start_postcondition() policy. The Process checker validates startup ordering and daemonization source; its PR path filters include all referenced LLTD/rstats files.
- Exact parent 6b5f551 DDNSTO, Process, NVRAM, AliyunDrive, infosvr, AliDDNS, AdGuardHome and AI-handoff runs passed. Telnetd product-source 989409e XY-C1 Build/TRX/artifact 44164998281 passed. Latest IPTV DHCP source 32622d8 checker follow-up 9e6d19f had WAN DHCP run 44168515283 in progress at last observation. THIS COMMIT exact-head Process/XY-C1 pending; no hardware L4 claimed.

## 2026-11-18 04:32 CST — Fix whitespace-sensitive IPTV DHCP checker assertion

- Exact source commit 32622d8549a0b5c022bfb765b189e7c3214a801c WAN DHCP run 44168062187 compiled and ran dhcp_lease_policy.c under GNU89, -Wall -Wextra -Werror, AddressSanitizer and UBSan; all five new IPTV deconfig transaction cases passed. Earlier DHCP policy steps also passed.
- Embedded Python caller contract then failed at line 171: it compared a multiline snprintf() call against a single-line string. Source formatting is valid; normalize whitespace before checking both old-address snapshots. No production or C-test change required for this correction.
- Exact 32622 WAN route run 44168062198 and AI docs run 44168062208 passed. XY-C1 44168062063, Firewall 44168062335 and NVRAM 44168062259 were still in progress at observation. Telnetd source 989409e XY-C1 44164998281 completed Build/TRX/artifact SUCCESS.

## 2026-11-18 04:25 CST — Clear stale IPTV DHCP address on lease deconfig

- Exact 6b5f5514f412e9a9ba173cc2148b5945ea082df3 DDNSTO 44166279131 passed shell syntax, full lifecycle regression and Python source contract. Process 44166279162, Process inventory 44166279188, AliDDNS 44166279025, AI docs 44166279079, AdGuardHome 44166279220, NVRAM 44166278930, infosvr 44166279207, and AliyunDrive 44166279170 also passed.
- Found an IPTV DHCP deconfig defect: BusyBox invokes the script for NAK with an active lease, lease expiry and explicit release, but udhcpc_viptv_main(deconfig) called only manv_route_deconfig(). Routes/gateway state were removed without clearing the interface address or manv_ipaddr.
- Implement failure-aware clear-address → route-removal transaction. Restore the previous address if either step fails; publish manv_ipaddr=0.0.0.0 only after both succeed; then stop igmpproxy and propagate its status. Extend dhcp_lease_policy.h, GNU89 fault-injection tests and WAN-DHCP workflow source assertions. BusyBox deconfig retry behavior is unchanged.
- Local host compile/run was NOT performed. Exact-head WAN-DHCP callback policy/source contract and XY-C1 are pending after this commit.

## 2026-11-18 04:05 CST — Make the DDNSTO UNKNOWN-startup regression deterministic

- Exact-head DDNSTO push run `44163173235` on `34c02687139afc6db6484c2eb888cca2f8f01612` passed shell syntax, then failed at `UNKNOWN startup inventory was incorrectly retried`; the semantic source-contract step was skipped. The background mock could update its state after the first post-launch `pidof`, allowing a legitimate ABSENT observation to cause a retry before UNKNOWN was published.
- Make only the test fixture deterministic: count pidof calls for this case, let the pre-launch stop inventory observe ABSENT, and force the first post-launch inventory to return UNKNOWN. Wait boundedly for the detached mock to publish its state; continue asserting that the production wait path did not sleep/retry UNKNOWN.
- No production DDNSTO source changed. Local host shell execution was NOT RUN in this environment; exact-head lifecycle + Python source-contract rerun is pending. On current parent `b00ff490`, Process lifecycle `44165568972`, infosvr `44165568767`, AliDDNS `44165568791`, AdGuardHome `44165569096`, and AI docs `44165568764` passed. XY-C1 `44164998281` for product-source `989409e` was still IN_PROGRESS; no current-source L3/L4 claim.

## 2026-11-18 — Fix stale Process regression output marker

- Parent main SHA: `989409e1a02a92ea5018f3550d52d75c33b028f8`.
- Exact Process lifecycle run `44164998210` compiled and ran the expanded GNU89 process-presence policy binary successfully. Its shared integration checker passed prior contracts, then failed only because its legacy assertion at line 1015 expected `Process presence start/notify tri-state policy: PASS`, while the test now prints `Process presence start/notify and detached-daemon postcondition policy: PASS`.
- Correct the exact expected test output. This follow-up changes checker/handoff/roadmap only; no production source changes. Re-run the Process checker on the exact SHA.

## 2026-11-18 — Verify BusyBox Telnetd after detached launch

- Parent main SHA: `34c02687139afc6db6484c2eb888cca2f8f01612`; THIS COMMIT updates RC startup, shared process-presence policy/test, the Process integration checker/workflow path filter, and the handoff/roadmap.
- BusyBox `telnetd_main()` calls `bb_daemonize_or_rexec()` unless `-F` is specified. RC calls `eval("telnetd")` without foreground mode; the detached child binds/listens only after daemonization. A port-in-use or post-fork setup failure can leave the parent launcher successful while Telnetd has exited.
- Add bounded child-presence verification after launch: one-second initial wait, four probes, one-second retry. Only RUNNING succeeds; UNKNOWN fails closed with the procfs scan errno; ABSENT after the probes reports `ETIMEDOUT`; launcher failures are preserved.
- Extend the GNU89 process-presence test for RUNNING/ABSENT/UNKNOWN/invalid and launcher-status cases. Update both Telnetd checker copies for BusyBox daemonize-before-bind and the launch/wait/inventory/postcondition sequence. Add BusyBox `telnetd.c` to the Process workflow push/PR filters.
- Parent exact-head Networkmap `44163174212`, Process `44163173313`, HTTPD daemon `44163173359`, infosvr `44163173874`, SkipDBv2 `44163173539`, and USB storage `44163176401` passed on `34c0268`; XY-C1 `44163173638` and NVRAM `44163173644` were pending. THIS COMMIT's validation is pending; no L4 hardware result is claimed.

## 2026-11-17 — Verify the detached networkmap child after daemonizing launch

- `trunk/user/networkmap/src/networkmap.c::main()` binds the ARP socket, installs signal handlers, calls `daemon(0, 0)`, writes its pidfile and starts the scan loop (or waits five seconds for `-w`). `start_networkmap()` previously returned the direct `eval()` result without confirming that the detached child survived.
- Add a bounded process-inventory postcondition after a zero launcher status: one-second initial delay, up to four probes, one-second retry spacing. Report success only after `networkmap` is RUNNING; UNKNOWN inventory fails closed with its scan error and a child that never appears returns `ETIMEDOUT`. Existing pre-launch tri-state absence gate, already-running no-op and first-call `-w` behavior are preserved.
- Add GNU89 policy tests for RUNNING/ABSENT/UNKNOWN/invalid states and preservation of nonzero launch status; update the networkmap source checker to enforce launch → wait → post-launch inventory order. The existing networkmap workflow tracks every changed source/test/checker path.
- Parent `9f9439ed` infosvr, process inventory/lifecycle, AI, networkmap signal, and 20 other current-head contracts passed. Parent storage, NVRAM, SkipDB and XY-C1 were still running at last poll; new exact-head networkmap check pending.
## 2026-11-17 — Verify the detached infosvr child after daemonizing launch

- Repository inspection confirmed `trunk/user/infosvr/src/infosvr.c::main()` binds the UDP socket and installs SIGTERM handling, then calls `daemon(0, 0)`, writes `/var/run/infosvr.pid` and enters its long-lived packet loop. `eval("/usr/sbin/infosvr", IFNAME_BR)` only reports the launcher/daemonizing parent status.
- `start_infosvr()` previously returned success as soon as `eval()` returned zero; a detached child that exited before the first packet loop could therefore be reported as successfully started. Add bounded process-inventory verification (initial 1-second wait, up to four probes) and report success only after `infosvr` is confirmed RUNNING. UNKNOWN inventory fails closed with the actual scan error; a missing child returns `ETIMEDOUT`.
- Preserve the existing pre-launch tri-state gate and disabled-service no-op. Add GNU89 policy tests for RUNNING/ABSENT/UNKNOWN/invalid state and launcher error preservation; extend the infosvr source checker for launch → wait → inventory ordering. Existing workflow already compiles the policy test and runs the checker.
- Parent exact-head storage, Process lifecycle/inventory, AliDDNS, AliyunDrive, infosvr, AI, AdGuardHome and NVRAM checks all passed on `dec7924b`. New exact-head infosvr CI pending. XY-C1 L3 Build/TRX/artifact is green for firmware source `fb99c2a`; no hardware L4 claim.
## 2026-11-17 — Make the swap staging source contract whitespace-tolerant

- Exact-head storage run `44159705744` on `9cf66b7bec1a7befd0408ef8a9c2aec9b44563b3` completed shell/ash, MTD, storage C policy tests including the new `/proc/swaps` parser cases, and earlier lifecycle tests successfully. It then failed in the final production caller graph at the source assertion for `storage_ejected_inventory_complete(read_error, close_error, parse_error)`.
- The production call is line-wrapped after the second argument; the checker compared the full call as one raw substring. Normalize whitespace in `swap_stage` before matching. This changes only the test harness, not production C code.
- Parent HTTPD, AI handoff, Process lifecycle/inventory, AdGuardHome, AliDDNS, AliyunDrive and infosvr passed. Parent NVRAM was still in progress at last poll.
## 2026-11-17 — Fix newline escapes in raw swap inventory tests

- Exact-head HTTPD run `44159082928` on `fb99c2a7dbf4c1576e4d863d5d451f18633fefd3` passed the previous HTTPD/NVRAM/wireless/storage source checks and then failed at the newly added `storage_ejected_parse_swaps_header()` policy assertion.
- Inspection of the actual C source showed the `/proc/swaps` header and row test literals contained two backslashes before `n`; the compiled string therefore ended in literal `\n` rather than a newline and correctly failed `storage_ejected_line_is_complete()`. Normalize only the new swap fixtures to single C newline escapes.
- Move the test's PASS message from before the new assertions to after them so the output cannot report success before all cases run. No production source changed in this test repair.
- Parent exact-head AliDDNS, AliyunDrive, Firewall, vsftpd, AI docs, USB modem, NFS, Samba, infosvr and AdGuardHome passed; storage, Process, NVRAM and XY-C1 were still in progress at last poll.
## 2026-11-17 — Retry raw swap-only devices during physical eject

- `umount_ejected_locked()` previously returned success immediately if the complete `/proc/mounts` inventory contained no removable storage mounts. Its swapoff retry loop only visited stale mounted partitions. A raw SD/MMC swap partition can exist without a filesystem mount; if the mdev partition-remove callback's first swapoff attempt failed, the subsequent whole-device notification had no stale mount to drive a deferred retry.
- Stage and validate `/proc/swaps` independently from `/proc/mounts`, then compare raw removable swap device names against a complete `/proc/partitions` snapshot. Retry swapoff for each missing `/dev/sd*`/`/dev/mmcblk*` device. A failed/UNKNOWN attempt returns failure even with no mountpoints, so `umount_ejected()` schedules the existing `usb_unplug_ms` retry. Non-removable swap files are ignored by the raw-device path parser and continue through the existing Optware mount teardown hook.
- Add GNU89 tests for complete five-column swap headers, exact five-field rows, truncated/extra-field rejection, SD/MMC whole-disk and partition identity, non-removable swapfiles and malformed paths. Extend storage source contracts for inventory-before-no-work and swapoff-before-stop/unmount.
- Parent `12f4ba4a` AI docs, AdGuardHome, infosvr, Process inventory, AliDDNS and AliyunDrive checks passed; NVRAM was still in progress. No MIPS/uClibc or physical unplug result is claimed.
## 2026-11-17 — Restore the live module dashboard after accidental roadmap truncation

- Exact-head AI handoff run `44156954920` on `5bdf59f0bcceb7802bd94309bd0bdb5b467381b7` failed because `REFACTOR_ROADMAP.md` lacked the required `## Whole-repository module status` dashboard. The job log identified this single invariant; other document checks passed.
- The live roadmap had been reduced to a 996-byte checkpoint-only note, losing the 51,338-byte module dashboard, priority queue and verification matrix. Restore the full version from Git history at `e90549004908d388123e1807abf0ddeb4a6a11fd`, update the current checkpoint, and keep the historical dashboard content intact.
- Parent WAN teardown `44156954967`, HTTPD `44156954859`, Process lifecycle/inventory `44156955420`/`44156955350`, and NVRAM `44156954872` passed. Exact parent XY-C1 `44156955220` was still building; TRX/artifact pending.
## 2026-11-17 — Stop MSD Lite during WAN teardown

- `start_udpxy()` can launch the optional `msd_lite` proxy, and the shared proxy stop list already includes it. The independent `stop_wan_locked()` inventory omitted `msd_lite`, so a WAN-down/reconnect teardown could leave the daemon alive with stale WAN routing and interface state.
- Add `msd_lite` to the WAN daemon stop inventory and extend the WAN teardown source contract to require all four WAN proxy daemons, including MSD Lite.
- Add `hotplug_stor.c` to the XY-C1 workflow path filters. The tri-state SD/MMC source change had only storage/HTTPD coverage; adding the source filter ensures future hotplug edits also trigger firmware validation. This commit changes `net_wan.c`, so XY-C1 is also triggered on this exact SHA.
- Exact-head WAN, XY-C1 Build/TRX/artifact, Process, AI handoff and NVRAM checks: pending. No local MIPS/uClibc build or hardware result is claimed.

