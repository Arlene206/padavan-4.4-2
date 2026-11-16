# Cross-session execution mandate

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Current parent main: `58fced5de7155e7d90d5307f81a6f0a56c56ea39`. Refetch live `main` before every write and use expected-SHA non-forced ref updates.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and must not be modified or deleted.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery were completed in earlier sessions; do not repeat that audit.
- Root `dev_log.md` currently contains 81 newest-first records; this commit adds record 82. Keep the root file between 20 and 119 records and within the size limit.

## Active task

Correct the WAN modem status integration checks following `58fced5de7155e7d90d5307f81a6f0a56c56ea39`.

The production changes from `8c3772e` and corrective commit `58fced5` now:
- block NDIS interface migration and DHCP launch if miniupnpd cannot stop, return the DHCP launch status, and set the WAN error flag;
- aggregate NDIS and RAS migration/launch errors through `start_wan()`, including RAS miniupnpd migration failure;
- propagate WAN start results through full restart/reconnect wrappers, boot-service health and PID1 notification results;
- preserve the original ordered `stop_wan()`, bridge teardown and LAN bridge setup gates in `full_restart_wan()`.

The remaining red checks on exact parent SHA `58fced5` are test-contract/integration defects, not evidence that these production changes should be rolled back:
1. Process and XY-C1 fail at `scripts/check_modem_wan_start_status.py`: the source checker counts an exact single-line C call, but production deliberately wraps the function arguments over multiple lines. Replace that brittle literal count with a whitespace-tolerant regular expression; retain the exact count of three call sites and the RAS-migration ordering/EAPOL isolation assertions.
2. WAN DHCP callback fails at the ZeroConf IPTV deconfig assertion: production returns first-error accumulation of interface, route and multicast cleanup statuses. Assert that nested accumulated expression instead of the old two-status return.
3. miniupnpd workflow run failed before creating jobs because two new Python assertions were accidentally outdented from the YAML `run: |` heredoc. Restore the same indentation as the surrounding script lines.

Do not weaken behavior assertions. The actual RAS error accumulation and full-restart bridge gates were restored in `58fced5`; the goal of this slice is to make the checks accurately validate that source.

## Exact-head verification for parent `58fced5de7155e7d90d5307f81a6f0a56c56ea39`

- Process regression run `43841042079`: FAIL in the source checker at the exact string-count assertion. The preceding service-start and modem policy host binaries both printed PASS.
- XY-C1 run `43841041502`: FAIL at the same checker before Build XY-C1, TRX verification and artifact upload; no L3 result for this SHA.
- WAN DHCP callback run `43841041827`: FAIL only at the stale ZeroConf accumulated-status source assertion.
- miniupnpd workflow run `43841038765`: workflow failed without creating a job; inspect as invalid YAML/heredoc indentation and fix it.
- WAN teardown run `43841041737`: SUCCESS.
- WAN route status run `43841041407`: SUCCESS.
- USB modem WDM/NDIS identity run `43841041397`: SUCCESS.
- Process lifecycle inventory run `43841041807`: SUCCESS.
- AI handoff documentation contract run `43841041398`: SUCCESS.
- HTTPD Authentication run `43841041572` failed in an independent switch-port ioctl output contract, unrelated to this slice's changed paths; do not attribute it to modem/WAN changes.

No new XY-C1 build/TRX/artifact or physical-router/L4 result is established by parent `58fced5`.

## Files in this corrective slice

- `scripts/check_modem_wan_start_status.py`: whitespace-tolerant verification of the three first-error call sites.
- `.github/workflows/miniupnpd-config-stability.yml`: restore valid heredoc indentation.
- `.github/workflows/wan-dhcp-callback-stability.yml`: verify interface + route + multicast error accumulation.

## Exact first action for the next GPT

1. Fetch live `main` and inspect push Actions with `head_sha` equal to the new commit: Process, XY-C1, WAN DHCP callback and miniupnpd config stability. Read logs for any failed job; do not infer success from completion alone. XY-C1 must reach Build XY-C1, TRX verification and artifact upload before recording L3.
2. If those checks pass, implement the queued P1 NDIS control-TTY cross-device selection fix in `usb_modem.c`: bind TTY metadata to the selected NDIS interface's exact physical USB port ID + devnum, preserve manual `modem_node` and `pref > 0` selection only within that same identity, and add GNU89/Werror/ASan/UBSan regressions plus a semantic source contract.
3. Then resume the device-aware `reload_modem_modules()` result/readiness contract. Do not make alternative drivers mandatory or change their legacy semantics without evidence.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; frozen backup remains read-only. Update root `dev_log.md` for every code/documentation commit; keep it within 20–119 records and preserve archived shards/indexes verbatim. Distinguish source, CI and firmware validation levels. No force-push, history rewrite, or unrun-test/build/hardware claim.
