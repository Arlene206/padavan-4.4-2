# Cross-session execution mandate

Recover live repository state once, then continue the recorded engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

Create all blobs, one complete Git tree and one commit parented to freshly fetched main per logical task; update main with exact expected SHA and force:false. Never force-push or create partial commits. Preserve frozen backup and archive shards/index.

## Repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch `main`.
- Parent main before the full LAN restart result-propagation slice: `2c75de8133751e8063ded244faa225011f2891b6`; source, regression, workflows, dev log and this handoff are submitted together in THIS COMMIT.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. Immutable.
- Bootstrap protocol, handoff, roadmap, log archive/index, branch inventory and PR #1 were audited earlier. PR #1 is closed and valid AliDDNS changes are already integrated; no repeat audit.
- UPnP stop-result status fix: `68da0e2bd19dab24b3ab0b576c87cbf43e25832a`. Checker/filter follow-up: `c6cf7147ee636ab6a715ee35f9219f4c5c92a3e1`; exact c6 Process `43811174980`, miniupnpd `43811175012`, AI docs `43811174933`, XY-C1 `43811175036` all PASS, with exact XY Build/TRX/artifact complete. L3 evidence is scoped to c6.
- Parent `01b7fc9163a8445ada6b96a7bfdee6305a2c95fa` includes VPN restart handling of PPP options file-write errors.

## Active source slice in THIS COMMIT

`full_restart_lan()` returned only DNSMasq status although PID 1 merges its int result. It ignored several status-bearing stops (logger, LLTD, infosvr, networkmap, miniupnpd), VPN/IPTV restarts, NFS/NetBIOS reloads, infosvr/LLTD/networkmap starts, IAPP restart and HTTPD identity signalling.

THIS COMMIT reuses `rc_first_service_start_status()` to retain the first nonzero service status, records all listed actions plus recovery/lock-release results, and returns that aggregate. Existing VPN-before-DNSMasq lock ordering, DNSMasq restart gate, early aborts and recovery paths remain. The existing service-status GNU89 regression now includes first-error survival and negative/positive status preservation; Process source checker and Process PR/XY-C1 path filters were extended; XY-C1 runs the test pre-build.

## Verification boundary before THIS COMMIT

- Exact c6 `c6cf7147ee636ab6a715ee35f9219f4c5c92a3e1`: Process `43811174980`, miniupnpd `43811175012`, AI `43811174933`, XY-C1 `43811175036` all SUCCESS; XY Build, TRX and artifact upload passed.
- Exact parent `01b7fc9163a8445ada6b96a7bfdee6305a2c95fa`: Process `43813748377`, inventory `43813748353`, VPN `43813748332`, AI docs `43813748506`, WAN DHCP `43813748381`, OpenVPN credentials `43813748366` SUCCESS; XY `43813748358` and NVRAM `43813748378` were IN_PROGRESS at last check.
- New exact-head Actions for THIS COMMIT are pending. No newer L3 or L4 result is claimed.

## Exact first action for next GPT

1. Check exact-head Process integration, XY-C1 and AI handoff runs for THIS COMMIT. Confirm the LAN first-error regression runs and source checker passes. Diagnose first causal failure if needed.
2. L3 is current only when exact-head XY-C1 Build, TRX verification and artifact upload all succeed; do not transfer older c6 evidence.
3. Continue P1 RC status propagation audit outside `full_restart_lan()`, fixing only confirmed dropped return statuses with caller-aware tests.

## Governance

- Preserve security, architecture, compatibility, target toolchain and mandatory tests.
- Develop only on `main`; frozen backup remains untouched.
- No force-push, history rewrite or unrun test/CI/build/hardware claim.
- Update root dev_log for each code/documentation commit; update roadmap only on material status changes; preserve archive shards/index byte-for-byte.
- At each checkpoint record exact main, product source SHA, exact Action IDs/statuses, and one next action.
