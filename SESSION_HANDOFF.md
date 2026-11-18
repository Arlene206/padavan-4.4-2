# 2026-11-17 continuation checkpoint — secure OpenVPN TUN device-node setup

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; parent main before THIS PID1 TUN-node hardening: `de02f6c04d993d462983576e3db2914e9b179afe`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; unchanged.
- Exact `c500c3a4` USB modem `44219360846`, HTTPD Security `44219360995`, XY-C1 Firmware `44219360799`, Process inventory `44219360836`, Process lifecycle inventory `44219360992`, NVRAM `44219360895`, infosvr `44219360979`, AdGuardHome `44219360852`, AliDDNS `44219361050`, AliyunDrive `44219360927`, and AI handoff `44219360924` all PASS. XY-C1 Build, TRX and artifact upload passed on that same SHA (L3 VERIFIED for `c500c3a4`).
- CI-fanout commit `a4d5a10b` removed docs-only triggers from product workflows and added production-input detection before target builds. Its Infosvr `44221748799` and AliyunDrive `44221748973` runtime checks passed but final source-contract steps failed because their validators still expected removed canonical-document path filters. The checker corrections in `45ed6be1` make Infosvr `44222138695` and AliyunDrive `44222138760` PASS. AI handoff failed on `45ed6be1` only because the exact mandatory checkpoint wording was missing; `de02f6c0` restored it and AI docs `44222506147` PASS.
- Storage HDD tuning error aggregation and SD hotplug status propagation are in `7f10d225`; exact storage regression/caller graph passed on `0985b536`.
- NDIS metadata failure propagation is verified by exact-head USB/HTTPD PASS and XY-C1 L3 on `c500c3a4`. No physical-router L4 evidence.
- Found a PID1 device-node trust gap: `init_nodes()` used `check_if_dev_exist("/dev/net/tun")` and skipped validation whenever the path existed. A symlink, regular file, wrong character device, unexpected owner/group, hardlink or unsafe mode could be treated as the OpenVPN TUN endpoint.
- THIS slice hardens `/dev/net` as a root-owned non-group/world-writable directory; validates `/dev/net/tun` with `lstat`, character-device type, major/minor 10:200, root ownership and single link; unlinks invalid leaf entries without following symlinks; and recreates/revalidates the expected node. It accepts stricter read/write-only permissions and keeps failure nonfatal to unrelated router startup. A GNU89 regression covers device identity, owner/group, link count and mode policy.

## Active task
- Verify exact-head PID1 mdev policy and XY-C1 Build/TRX/artifact for the TUN-node hardening.

## Verification boundary
- Parent `c500c3a4`: all eleven exact-SHA workflows pass, including same-SHA XY-C1 Build/TRX/artifact.
- Parent `a4d5a10b`: Infosvr/AliyunDrive failed due stale docs-path checks; `45ed6be1` has both passing and `de02f6c0` has AI docs PASS. XY-C1 `44221748846` was still building at last poll.
- THIS TUN-node slice's PID1 regression and XY-C1 exact-head Actions are pending. Physical L4 is NOT VERIFIED.

## Exact first action for the next GPT
1. Inspect exact-head init-mdev security and XY-C1 workflows; require the TUN policy test/source contract to PASS and same-SHA Build, TRX and artifact success before claiming L3.
2. Check the exact `a4d5a10b` XY-C1 result and any remaining NVRAM/Process checks.
3. Continue P1 reliability work across broader Padavan userland after this PID1 device-node slice.
4. Keep frozen backup unchanged; publish complete trees with non-forced expected-SHA updates.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. At session end inspect live main, exact-SHA Actions, current handoff/roadmap, validation boundary and first actionable task. If all remain accurate, explicitly conclude with no file change and no commit.

Do not create no-op documentation commits; update only at meaningful validation boundaries.

# Cross-session execution mandate

Continue one engineering effort across GPT conversations. Recover once, implement concrete defects, test and commit atomically; do not wait idly on pending CI.

Every logical slice is one atomic main commit: create a complete tree and commit parented to freshly fetched main, then publish with non-forced expected-SHA compare-and-swap. Never force-push, create development branches/PRs, or modify frozen backup.

Before context exhaustion, finish a safe slice, inspect exact-SHA Actions and leave the next GPT a precise first task.
