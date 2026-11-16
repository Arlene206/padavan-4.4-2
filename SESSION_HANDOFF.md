# Cross-session execution mandate

Recover live repository state once, then continue the exact recorded engineering task. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all blobs, one complete Git tree and one commit parented to freshly fetched main; update main using the exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve the frozen backup and all archived log shards/index. Mandatory handoff checkpoint: The checkpoint is mandatory; a document edit is conditional.

Before context exhaustion, record exact refs, Action states and the first next action.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch `main`.
- Main immediately before this commit: `f9e24531c59e681cf866acc00e2534911f7219b9`; this checker/test-wiring repair is committed as THIS COMMIT.
- Frozen archival ref: `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`. It is immutable and must never be modified, moved, rewritten or deleted.
- The startup protocol, handoff, roadmap, log archive/index, branch inventory, backup warning and legacy PR #1 have already been recovered in this continuing work sequence. PR #1 is CLOSED; valid content has already been incorporated on main. Do not repeat the bootstrap audit.
- Product-source change `f9e24531c59e681cf866acc00e2534911f7219b9` includes VPN server restart failure propagation. The immediately preceding source baseline `c73fd70893d0bdee040bbea430f34d5749388eac` adds DDNS restart failure propagation.
- Product-source baseline `1b4a6bbb8478255da842be6fa058d15e59e20b5e` has exact-head Process lifecycle, AI handoff, all 40 returned push workflows and XY-C1 Build/TRX/artifact upload all SUCCESS. The later `c73fd70` XY-C1 run `43808563408` subsequently completed SUCCESS, so its L3 evidence applies to `c73fd70`; it does not apply to the newer VPN source changes in `f9e2453`.
- The recent DNSMasq CI contract truncation was independently restored on main by `38cbf6e188713b7199dc32182ea5d36946327c4d`; exact-head DNSMasq run `43809585972` and AI docs run `43809585961` both PASS. Do not re-audit that fix.

## Active task

The current task fixes the VPN server lifecycle workflow after exact-head failure on `f9e24531c59e681cf866acc00e2534911f7219b9`. Run `43809878340` compiled and passed the lifecycle-lock, xl2tpd-start and shared lease-lock policy tests, but failed in `scripts/check_vpn_server_lifecycle.py`: its static ordering contract still expected the former void `restart_vpn_server()` shape, including two bare `return;` statements. The current production implementation returns the first stop/start/restore/unlock failure, and releases the lock on the stop-failure branch before returning. The new int result is merged into the `RCN_RESTART_VPNSVR` notification result.

THIS COMMIT updates the checker to that real API and return path, asserts notification aggregation, adds the standalone `vpn_server_restart_policy.c` GNU89/Werror regression to the VPN workflow, and adds the policy/helper test to both push and pull_request path filters. Production source is unchanged in this checker/test repair.

## Verification boundary

- Exact `f9e2453`: Process inventory `43809878266` PASS; AI docs `43809878572` PASS; miniupnpd `43809878648` PASS; HTTPD lifecycle `43809878807` PASS; Inadyn `43809878543` PASS; AliDDNS `43809878657` PASS; FRP `43809878472` PASS; DNS Forwarder `43809878210` PASS; ZeroTier `43809878652` PASS; VPN server lifecycle `43809878340` FAIL at the stale source assertion before this repair.
- Exact `f9e2453` XY-C1 run `43809878569`, DNSMasq `43809878632`, Process lifecycle `43809878580`, USB storage `43809878424`, firewall startup `43809878473`, NVRAM audit `43809878237`, HTTPD security `43809878265`, plus several other runs, were IN_PROGRESS at last check. These are pending, not PASS.
- Exact `c73fd70` XY-C1 `43808563408` subsequently completed SUCCESS, including Build XY-C1, TRX verification and artifact upload. Exact `c73fd70` Process `43808563348` and AI docs `43808563463` were SUCCESS.
- No physical-router runtime or L4 validation has been performed. Do not transfer the `c73fd70` firmware result to `f9e2453`.

## Exact first action for the next GPT

1. Check exact-head VPN server lifecycle and AI handoff Actions for THIS COMMIT. The new dedicated policy regression must compile/run and the source checker must accept the production int status path. Repair any first causal failure on main.
2. Check the existing XY-C1 run `43809878569` for its final status on `f9e2453`; it is the required target build/TRX/artifact evidence for the newer VPN source. Continue independent work without waiting if it remains pending.
3. Then implement the independent P1 UPnP notification issue: on the current source, `restart_upnp()` is void and returns early after failed `stop_upnp()`; the `RCN_RESTART_UPNP` handler currently discards that status. Convert it to an int result, use `rc_notify_merge_action_result()`, and add callback-driven regression coverage proving failed stop suppresses firewall restart.

## Governance constraints

- Preserve security, architecture, compatibility, target-toolchain and required test rules.
- Use only `main`. Never use or modify the frozen backup.
- Never force-push, rewrite published history, claim unrun tests/Actions/builds or claim L3/L4 beyond the exact SHA evidenced.
- Update `dev_log.md` for each code/documentation commit. Update the roadmap only when repository-wide module status or priority changes materially. Preserve the single live handoff/roadmap; no routine archive snapshots.
- At each end-of-session checkpoint distinguish live main from product-source baseline, list pending/failed Actions with exact run IDs/head SHAs, and write the precise first next action.
