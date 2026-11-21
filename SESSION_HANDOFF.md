# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Each logical task is one atomic main commit: create all blobs and one complete Git tree, then one commit and a non-forced expected-SHA update.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Fix real source defects with focused regressions; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, inspect the full diff, publish atomically, and preserve the exact next action here.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Inspect live `main`, verify the frozen backup SHA and check exact-SHA Actions at session end. Update this file only when the baseline, evidence, blocker or first next action materially changes. Do not make no-op commits.

## Current repository state

- Source parent SHA: `8f21e7fc31609caae6ab0d2260e4b345f3e6d487`. The present checkpoint is a documentation-only child of this exact source tip.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only; do not move it).
- This development window's source work is already integrated in the recent main history: HTTPD startup-wait rollback (`ab866c3e93c5131ed6723c7a2a7d3de5df0e693c`); MiniUPnP failed-start rollback (`b55d97afc5b09fed7658785f6415023ddb2934e7`); DNSMasq failed-start rollback (`ba695e6f523b1218bd7486fb77b9b42e4e1e4138`); Samba nmbd/shared-daemon rollback (`f8fc45e7ad1479e2544e093a6e52a074c35bf256`, `3da546fd4e58d09e50cdbc0c872289e7a5504dd0`); vsftpd rollback (`66edbcb149c7332c76330b4cb33c8847db21ac31`); storage application failed-start rollback (`acbc93decc462e5e0360552ef0da83310fed5512`); Inadyn failed-start rollback (`41b18c57237e70171e338ec6f416b5a613643fda`).
- The storage lifecycle validator required follow-up changes because source behavior now returns a checked start status and invokes fail-closed rollback rather than returning the old launch-result expression. The current checker contains the newer contract: `present`. It must be exercised by an exact-SHA Storage Hotplug run before marking PASS.
- Exact-SHA Actions for current source parent `8f21e7fc31609caae6ab0d2260e4b345f3e6d487`: XY-C1 Firmware Validation: in_progress (#44878734942); USB storage hotplug stability regression: in_progress (#44878734891).
- Inadyn source SHA `41b18c57237e70171e338ec6f416b5a613643fda`: XY-C1 Firmware Validation: in_progress (#44877952694); HTTPD Authentication and Security Regression: success (#44877952674); miniupnpd atomic configuration publication regression: success (#44877952721); Process lifecycle inventory - 41b18c57237e70171e338ec6f416b5a613643fda: success (#44877952692); DNSMasq lifecycle stability regression: success (#44877952738); Samba share configuration stability regression: success (#44877952776); Inadyn fail-closed lifecycle regression: success (#44877952666); WAN DHCP callback state stability regression: success (#44877952702); WAN route operation status regression: success (#44877952689); Process inventory and lifecycle fail-closed regression: success (#44877952774); NVRAM transaction boundary audit: success (#44877952690).
- Storage checker slice SHA `6c4e923fcd6ae1b7937617327c6dda2c800d9ab5`: AI handoff documentation contract: success (#44878146492); USB storage hotplug stability regression: failure (#44878146416).
- Latest branch Actions snapshot:
```
XY-C1 Firmware Validation: in_progress @ 8f21e7fc (#44878734942)
USB storage hotplug stability regression: in_progress @ 8f21e7fc (#44878734891)
USB storage hotplug stability regression: failure @ 671eb14b (#44878632870)
XY-C1 Firmware Validation: success @ 671eb14b (#44878632886)
WAN route operation status regression: success @ a9333978 (#44878517256)
GPIO button daemon startup and press timing stability regression: success @ a9333978 (#44878517295)
DDNSTO lifecycle stability regression: success @ a9333978 (#44878517091)
Firewall Startup Stability Regression: success @ a9333978 (#44878517202)
AdGuardHome config security regression: success @ a9333978 (#44878517217)
networkmap signal lifecycle stability regression: success @ a9333978 (#44878517201)
vsftpd configuration publication stability regression: success @ a9333978 (#44878517293)
watchdog signal lifecycle stability regression: success @ a9333978 (#44878517203)
Samba share configuration stability regression: success @ a9333978 (#44878517257)
FRP asynchronous startup lifecycle regression: success @ a9333978 (#44878517274)
DNSMasq lifecycle stability regression: success @ a9333978 (#44878517252)
HTTPD Authentication and Security Regression: success @ a9333978 (#44878517198)
```
- XY-C1 L3 is NOT VERIFIED unless Build, TRX validation and artifact upload all PASS on one identical source SHA. Physical-router L4 remains NOT VERIFIED.
- No local firmware build or cross-compile was performed in this continuation. Do not convert CI status into physical-device validation.

## Active task

1. Inspect the exact-SHA Storage Hotplug result after the safe-remove checker update. Diagnose the first remaining failing assertion; do not weaken the test to accommodate a source regression.
2. Inspect exact-SHA Inadyn, Process Lifecycle, WAN, NVRAM transaction and XY-C1 actions. PENDING does not block other independent work; record exact run IDs and SHA.
3. Continue the stability queue outside already-covered daemons. Re-examine the NFS server start/stop path (`trunk/user/nfs-utils/nfsd.sh`) and other RC userland process launch paths; implement only a reproduced/clear defect with a focused regression.
4. Preserve the loop: modify source -> test -> one atomic main commit -> inspect that exact SHA's Actions. FAIL means fix the earliest causal defect; PASS means move on.

## Verification boundary

- Existing verified historical exact-SHA passes remain: HTTPD daemon lifecycle #44875048202; HTTPD authentication/security #44875048156; Process lifecycle inventory #44875048297; WAN route status #44875048033 (all on source SHA `ab866c3e93c5131ed6723c7a2a7d3de5df0e693c`).
- These historical results do not certify the later daemon rollback slices. For those, use only the exact-SHA status recorded above or re-run the relevant workflows on the current source parent.
- Storage fresh-start rollback covers minidlna, mt-daapd, transmission-daemon and aria2c; policy tests cover ABSENT/RUNNING/UNKNOWN/invalid states. The dedicated storage workflow must PASS after the checker was updated.
- Inadyn rollback covers launcher failure and startup-confirmation failure; it stops under the existing lifecycle lock, re-inventories state and preserves the original error. Check its exact-SHA run above.
- No claim of whole-repository runtime completeness, XY-C1 L3, or physical-router L4 is made here.

## Exact first action for the next GPT

Fetch live `main` and confirm the archival backup is still `72f4f157a084f8184b9597ca5bf334b8080e5807`. Inspect the exact-SHA workflow records listed in Current repository state; first close the Storage Hotplug checker result and any Inadyn/Process Lifecycle failure. Then take one independently testable userland stability bug, implement it with a regression test, and commit the full Git tree atomically using expected-SHA non-forced update.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining the newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Host regression, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
