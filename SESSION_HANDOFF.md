# Cross-session execution mandate

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Current parent main: `3d0a5692490b6eb1bc35fae74c74764be4fa234d`. Refetch live `main` before every write and use expected-SHA non-forced ref updates.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and must not be modified or deleted.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery were completed in earlier sessions; do not repeat that audit.
- Root `dev_log.md` currently contains 82 newest-first records; this commit adds record 83. Keep the root file between 20 and 119 records.

## Active task

Finish correcting the WAN modem regression checker after `8c3772e` and `58fced5`.

Production code on parent `58fced5` correctly:
- blocks NDIS WAN interface migration/DHCP launch when miniupnpd cannot stop;
- returns the real DHCP launch result and updates WAN error state;
- retains NDIS and RAS migration/launch failures in `start_wan()`;
- forwards WAN start results through restart/reconnect wrappers, boot status and PID1 notifications;
- preserves the original ordered full-WAN stop, bridge teardown and LAN bridge setup gates.

Parent `58fced5` corrected the RAS migration aggregation and the full-restart bridge gate. Parent `3d0a569` corrected the miniupnpd YAML heredoc indentation and the ZeroConf IPTV callback assertion. Review found the modem checker in `3d0a569` still used doubled backslashes in its new raw regex and retained an exact one-line RAS-order assertion. THIS COMMIT replaces both with one whitespace-tolerant regex, requires exactly three aggregation call sites and verifies one follows the RAS migration call.

No production behavior change is made in this checker-only follow-up. Do not claim the modem startup slice green until the new exact-head Actions pass.

## Parent verification

- On `58fced5de7155e7d90d5307f81a6f0a56c56ea39`, Process `43841042079` and XY-C1 `43841041502` passed both host policy binaries but failed the modem source checker.
- WAN DHCP callback `43841041827` failed a stale ZeroConf deconfig assertion; `3d0a569` updated it for actual interface+route+multicast accumulation.
- miniupnpd run `43841038765` had no jobs because its workflow heredoc assertions were outdented; `3d0a569` restored indentation.
- WAN teardown `43841041737`, WAN route `43841041407`, USB identity `43841041397`, Process inventory `43841041807` and AI docs `43841041398` passed on `58fced5`.
- Parent `3d0a5692490b6eb1bc35fae74c74764be4fa234d` Process run `43842343441` was queued when checked. Exact-head results for its other changed path filters were not yet confirmed.
- No L3 build/TRX/artifact or physical-router/L4 result is verified for `58fced5` or `3d0a569`.

## Next steps

1. Fetch current main, inspect exact-HEAD push Actions for Process and XY-C1, and confirm miniupnpd and WAN DHCP workflows are actually scheduled. Read failed logs rather than treating queued/completed as PASS. XY-C1 must reach Build, TRX verification and artifact upload for L3.
2. Once regression checks are green, fix the queued P1 NDIS TTY cross-device selection defect in `usb_modem.c`: match TTY metadata's exact physical USB port ID + devnum to the selected interface, preserve manual `modem_node` and `pref > 0` behavior only among that identity, and add GNU89/Werror/ASan/UBSan selection regressions plus a semantic source contract.
3. Then resume the device-aware `reload_modem_modules()` result/readiness contract. Do not make alternative drivers mandatory without evidence.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; frozen backup is read-only. Update root `dev_log.md` for each code/documentation commit; keep 20–119 records and preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, history rewrite, or unrun-test/build/hardware claim.
