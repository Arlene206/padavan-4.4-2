# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST perform an end-of-session accuracy checkpoint before ending. The checkpoint is mandatory; a document edit is conditional.

Continue the recorded engineering task across ChatGPT conversations. Recover live repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched `main`; update `main` with exact expected SHA using a non-forced compare-and-swap. Never force-push, rewrite history, create partial commits, or touch the frozen backup. Before context exhaustion, record precise live refs, exact Action results and the next concrete task.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Parent main for THIS corrective slice: `d4952540d5742868cf5cbe8d84bc4bc53e9450ad`. Re-fetch the live ref before each later write and use expected-SHA non-forced updates.
- Frozen archive: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable.
- Startup protocol, root governance, archive index, backup marker and PR #1 recovery are complete; do not repeat that audit.
- Root `dev_log.md` has 84 newest-first records before THIS commit; it will have 85 after this record. Keep 20–119 records and preserve archive shards/indexes.

## Active task

Close the NDIS/RAS WAN startup status integration gaps without weakening best-effort sequencing.

- The RAS modem branch already retained miniupnpd migration and `launch_wan_modem_ras()` failures, but only logged a nonzero `launch_wanx()` physical-WAN setup result. That allowed callers to see success when required interface/DHCP/ZeroConf setup had failed.
- THIS commit aggregates RAS `wanx_status` using the existing first-error helper before continuing to `launch_wan_modem_ras()`. The modem launcher remains best-effort and is still attempted.
- The GNU89 policy regression covers a physical-setup failure followed by a RAS launch result and verifies first-error preservation.
- The modem source checker uses whitespace-tolerant RC notification matching and explicitly checks RAS WANX error aggregation.
- The WAN DHCP callback workflow now extracts the current int-return `start_wan(void)` function instead of searching for the obsolete void signature.

## Verification boundary

Exact-head observations on parent `d4952540d5742868cf5cbe8d84bc4bc53e9450ad`:

- Process run `43843270795`: FAIL at the source checker’s one-line RC notification assertion. Its preceding RC boot-status and modem WAN policy binaries printed PASS.
- XY-C1 run `43843270817`: IN_PROGRESS at last inspection; it had encountered the same notification-checker failure after both host policy binaries printed PASS. It had not reached Build XY-C1, TRX verification or artifact upload; no L3 result.
- AI handoff documentation run `43843270727`: FAIL because this file lacked the mandatory end-of-session accuracy-checkpoint wording. THIS commit restores the required phrase.
- NVRAM transaction-boundary run `43843270745`: IN_PROGRESS at last inspection; independent of this WAN slice.
- AdGuardHome run `43843270763`, AliDDNS run `43843270787`, and AliyunDrive run `43843270760`: SUCCESS on this exact parent.
- WAN DHCP callback run `43842343094` on `3d0a5692490b6eb1bc35fae74c74764be4fa234d` failed because its contract still searched for the obsolete `void start_wan(void)` signature. It was not rerun on `d4952540d5742868cf5cbe8d84bc4bc53e9450ad`; THIS commit updates that assertion.
- Infosvr run `43842801299` on `1e9f5f17d51ac43e9e6bd8911ba1e140f45634ab` failed because the handoff lacked the exact `## Verification boundary` section. THIS commit restores the required heading.
- Miniupnpd config run `43842343222` on `3d0a5692490b6eb1bc35fae74c74764be4fa234d` passed; this commit does not touch that workflow.
- No current-parent XY-C1 Build/TRX/artifact result or physical-router/L4 validation is established.

Exact-head Actions for THIS commit remain pending after publication. A workflow result is PASS only when its corresponding job and required steps complete successfully.

## Exact first action for the next GPT

1. Fetch live `main` and inspect Actions whose `head_sha` equals THIS commit. Prioritize Process, XY-C1, WAN DHCP callback, infosvr and AI handoff; inspect failed logs rather than inferring success from workflow completion. Confirm Build XY-C1, TRX verification and artifact upload before recording L3.
2. Once those integration gates are green, implement the queued P1 NDIS control-TTY identity fix in `usb_modem.c`: bind TTY metadata to the selected NDIS interface’s exact physical USB port ID + devnum; preserve manual `modem_node` and `pref > 0` behavior only within that physical identity; add GNU89/Werror/ASan/UBSan selection regressions and a semantic source contract.
3. Then resume the device-aware `reload_modem_modules()` result/readiness contract. Do not make alternative drivers mandatory without evidence.

## Governance constraints

Preserve security, architecture, compatibility, target-toolchain and mandatory tests. Use only `main`; the frozen backup is read-only. Update root `dev_log.md` for every code/documentation commit, keep 20–119 records, preserve archive shards/indexes verbatim, and distinguish source, CI, firmware and physical-router validation. No force-push, history rewrite, or unrun-test/build/hardware claim.
