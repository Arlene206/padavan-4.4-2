# Cross-session execution mandate

Continue the exact repository engineering task below. Startup and handoff recovery have already been completed earlier; do not repeat the bootstrap audit or completed work.

Every logical task must create all changed files in one Git tree and one commit parented to freshly fetched main, then update main by exact expected SHA with force:false. Never force-push, rewrite history, or create partial commits. Preserve the immutable backup and all archive shards/index.

## Repository state

- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`.
- Latest code commit: `b2d3288b6282c16cf23859eb9e7f8fdce083cd62` (full LAN restart status propagation).
- Frozen archive ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not modify, move, rewrite or delete.
- Bootstrap, startup protocol, handoff, roadmap, log archive/index, branches and legacy PR #1 have already been audited; PR #1 is closed and substantive valid content is on main.

## Active verification failure

The code commit `b2d3288` updates `full_restart_lan()` to preserve first failure from status-bearing service stops, recovery, VPN/IPTV restart, NFS/NetBIOS reload, infosvr/LLTD/networkmap start, IAPP restart and HTTPD logout. It reuses `rc_first_service_start_status()`, extends `tests/service_start_status.c`, adds caller/source contracts in `scripts/check_process_lifecycle_integration.py`, and adds XY-C1 pre-build test execution.

Exact-head Process workflow run `43814394160` for `b2d3288` failed before any job ran because the PR path list in `.github/workflows/process-lifecycle-regression.yml` contains this malformed one-line fragment:
`      - "trunk/user/rc/src/rc.h"      - "trunk/user/rc/src/net_lan.c"      - "trunk/user/rc/src/service_start_status.h"      - "trunk/user/rc/src/tests/service_start_status.c"`
Replace it with four separate YAML list entries (one per line), preserving the existing rc.h entry. The push path list is already properly formatted.
Exact-head XY-C1 run `43814395678` and AI handoff run `43814395799` were in progress at last observation; Process inventory `43814395870` was also in progress. Query their final status as well as the new fix's Actions.

## Verification boundary

- Latest confirmed firmware L3 baseline remains `c6cf7147ee636ab6a715ee35f9219f4c5c92a3e1`: XY-C1 run `43811175036` successfully completed Build, TRX verification and artifact upload; Process `43811174980`, miniupnpd `43811175012`, and AI docs `43811174933` also passed on that exact SHA.
- On exact `b2d3288`, Process workflow `43814394160` failed due malformed YAML (no job execution); XY-C1 `43814395678`, AI docs `43814395799` and Process inventory `43814395870` were pending.
- No current-head firmware build/TRX/artifact or L4 hardware result is established.

## Exact first action for next GPT

1. Repair the malformed PR paths in `.github/workflows/process-lifecycle-regression.yml` with an atomic workflow + dev_log + handoff commit on current main.
2. Check exact-head XY-C1 and AI docs on `b2d3288`, plus new exact-head Process run after repair. Inspect the first failing step logs. Confirm the service-status test runs in Process and XY-C1, and checker passes.
3. For L3, require Build XY-C1, TRX verification and artifact upload success for the exact latest source SHA.
4. Continue broader P1 RC status-return audit only after closing the CI repair; work on independent areas while unrelated checks remain pending.

## Governance

Preserve security, architecture, compatibility, target-toolchain and test rules. Develop only on main. Update dev_log for each code/documentation commit; keep handoff current. Do not claim unrun checks or carry old firmware results forward.
