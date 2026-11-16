# Cross-session execution mandate

Recover live repository state once, then continue the exact recorded engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.

Before context exhaustion, record exact refs, Action states and the first next action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch `main`.
- Main before this checker follow-up: `f59d0a228a1c1c7bf3196c886d4a68baeccb35e6`; current checker + checkpoint update is THIS COMMIT.
- Frozen archival ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. It must remain immutable.
- Startup/handoff/roadmap/log index/branch inventory/backup warning/legacy PR #1 have already been recovered. PR #1 is CLOSED and its valid work is on main; do not repeat that audit.
- Product-source baseline `f9e24531c59e681cf866acc00e2534911f7219b9` includes VPN restart error aggregation; earlier DDNS baseline `c73fd70893d0bdee040bbea430f34d5749388eac` has exact-head XY-C1 Build/TRX/artifact PASS (run `43808563408`). The firmware result does not extend to `f9e2453` or later.
- Exact DNSMasq workflow repair commit `38cbf6e188713b7199dc32182ea5d36946327c4d` passed DNSMasq `43809585972` and AI docs `43809585961`.

## Active task

The production VPN change on `f9e2453` changed `restart_vpn_server()` to return its first stop/start/restore/unlock error and merge it into PID 1's restart notification result. Exact parent VPN job `43809878340` passed its three compiled host regressions, then its source checker failed because it still expected the old void-function bare returns.

Commit `f59d0a228a1c1c7bf3196c886d4a68baeccb35e6` updated the checker ordering for the int result and wired the new `vpn_server_restart_policy.c` regression into its dedicated workflow. A source review then caught that the new raw regex for the notification branch had over-escaped backslashes. THIS COMMIT replaces it with a direct, unambiguous assertion against the parsed `handle_notifications()` body. Both preceding workflow runs `43810407149` (VPN) and `43810407150` (AI handoff) were IN_PROGRESS at last inspection; do not claim their results.

## Verification boundary

- Exact `f9e2453`: Process inventory `43809878266`, AI docs `43809878572`, miniupnpd `43809878648`, HTTPD lifecycle `43809878807`, Inadyn `43809878543`, AliDDNS `43809878657`, and FRP `43809878472` SUCCESS. VPN lifecycle `43809878340` failed only at the stale checker after all three host test binaries passed. XY-C1 `43809878569`, DNSMasq `43809878632`, Process lifecycle `43809878580`, USB storage `43809878424`, Firewall `43809878473`, NVRAM `43809878237`, and HTTPD security `43809878265` were pending at the time.
- Exact `f59d0a2`: VPN lifecycle `43810407149` and AI handoff `43810407150` were in progress at last inspection. THIS COMMIT must be checked separately.
- Exact `c73fd70` XY-C1 run `43808563408` subsequently completed SUCCESS including Build, TRX and artifact upload. L3 applies only to that SHA. L4 hardware remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head VPN lifecycle and AI handoff runs for THIS COMMIT. Confirm the new policy regression compiles/runs and the source checker accepts the int-return/notification merge path. Repair any first causal failure directly on main.
2. Check XY-C1 run `43809878569` for its final state on `f9e2453`; do not describe it as completed unless Build, TRX and artifact upload all succeeded.
3. Continue the P1 UPnP notification-result defect on current live main: `restart_upnp()` in `services_ex.c` is void and returns early when `stop_upnp()` fails; `rc.c`'s `RCN_RESTART_UPNP` handler ignores the status. Return the stop result, merge it with `rc_notify_merge_action_result()`, and add a callback test proving failed stop prevents firewall restart.

## Governance constraints

- Preserve security, architecture, compatibility, target-toolchain and test rules.
- Develop only on `main`; never modify the frozen backup.
- No force-push, published-history rewrite, or invented test/CI/build/hardware claim.
- Update the root dev log for each code/documentation commit. Update roadmap only when module status or priority materially changes; preserve live handoff/roadmap files without routine snapshots.
- Before ending a session, distinguish the live main SHA from product-source baseline, record exact Actions states and identify one precise next task.
