# 2026-11-18 continuation checkpoint — preserve Shadowsocks firewall child status

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; active work is on `main`.
- Parent main for THIS focused source/test commit: `efe00be7f58c93feff92ea7d1f9102c2b2829582`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed and unmerged; do not reopen or modify it.
- XY-C1 concurrency is now isolated by `github.sha` and `cancel-in-progress: false`; subsequent pushes no longer cancel an earlier exact-SHA firmware build.
- Development-log archive rotation is complete: `docs/dev-log/2026/11/part-016.md` retains the 100 archived records oldest-first.

## PPPoE-MWAN probe fix completed
- `trunk/user/pppoe/detect.sh` uses `/sys/class/net/pppN` to distinguish expected WAN reconvergence from failed `ifconfig` probes on existing interfaces. It preserves the exact child error, continues reconciling independent peers and handles disappearance during the probe.
- Exact-head WAN route on source SHA `ba893707c6f5cb939980cf4eecb5cd2d5dd14871` passed run `44263552770`; Process lifecycle `44263552790` and Process inventory `44263552757` also passed. Newer XY-C1 run `44264231022` was cancelled under the old shared concurrency policy; the policy has since been fixed. No L3 is inferred from the cancelled run.
- Deferred WAN reconnect watchdog startup failure propagation is in `ffa7f6d07522d8a51b53a9c1fede22c42d3588b2`; WAN route regression passed on `0ecab3a6f91e6c835c24c92230d1c11ad7ad28de` (run `44265049894`).

## Active task
- The Shadowsocks `ssp_start()` path now accumulates first required-stage failures, gates dependent TCP/UDP/DNS startup, handles optional local SOCKS5 and cron maintenance separately, and propagates errors from CLI start/restart. Host failure injection was added in `trunk/user/shadowsocks/tests/start_status.sh`.
- A follow-up defect remains: `rules()` still masks the actual `start_rules` / `ss-rules` child exit status as generic 1. THIS commit captures and returns the exact nonzero child status and changes the host regression to extract and exercise the production `rules()` function instead of stubbing it. The source checker now pins that exact status contract.
- Exact parent run `44267823337` on `efe00be7f58c93feff92ea7d1f9102c2b2829582` passed the Shadowsocks lifecycle workflow after correcting a false-positive static assertion. The earlier same-source runtime suite passed the new startup, existing process-lifecycle and Adblock tests; only the static checker had failed on the prior SHA.
- Exact product-source XY-C1 run `44267333941` on `62ecae88a449deccfb697b8e490590c0c514d6c1` was IN PROGRESS at the last inspection: Build active; TRX and artifact upload pending. Its result must be polled before making any L3 claim.

## Verification boundary
- The current source/test change requires exact-SHA Shadowsocks lifecycle, Process inventory, AI handoff and XY-C1 checks after commit. Do not reuse a parent result.
- The parent `efe00be7` Shadowsocks lifecycle run `44267823337` PASS is the latest completed runtime/source-contract evidence. The exact product-input XY-C1 run `44267333941` remains pending as noted above.
- Historical L3 remains VERIFIED only for `a0cb9aad4844f7d044ce79efad6c417857c95972` (XY-C1 `44260005493`, Build/TRX/artifact). Newer product-input SHAs need their own complete Build, TRX and upload result. No physical-router test is claimed; L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main`; inspect this commit's exact-SHA Shadowsocks, Process inventory, AI docs and XY-C1 Actions. For XY-C1, inspect Build, TRX verification and artifact upload individually.
2. Poll XY-C1 run `44267333941` for source SHA `62ecae88a449deccfb697b8e490590c0c514d6c1`. If it completed Build/TRX/upload, record L3 for that SHA only; otherwise report the precise failed/pending step.
3. If the `rules()` fix passes, continue whole-repository RC/userland stability work with the next confirmed root-cause status-loss defect. Do not repeat completed PPPoE-MWAN or watchdog audits absent a new exact-head failure.
4. Preserve the frozen backup and keep physical-router L4 marked NOT VERIFIED.

## Mandatory handoff checkpoint — end-of-session accuracy
The checkpoint is mandatory; a document edit is conditional.
Every session MUST inspect the live repository state and exact-head Actions before ending. At session end, fetch live `main`, exact-head Actions and the next first task. Update this file only when the facts or first task materially change; do not create a no-op commit when all facts are already accurate.

# Cross-session execution mandate
Before context exhaustion, complete **one atomic main commit** per logical source/test slice, verify the exact-SHA Actions, and leave an accurate next first task. Continue source fixes and independent work while unrelated Actions are pending. Never force-push, mutate the backup, reopen the retired PR, or claim unperformed tests/build/hardware checks.
