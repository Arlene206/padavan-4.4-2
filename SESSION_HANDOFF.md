# 2026-11-18 continuation checkpoint — shared L2TP recovery after incomplete VPN client restart

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; work on `main` only.
- Current main SHA: `a0cb9aad4844f7d044ce79efad6c417857c95972` (`fix(vpn): restore shared L2TP after incomplete client stop`), parent `764d7c7aeae4577f287cc6b5462d36c7e86aaa0c`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains archival at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not check out, modify, merge, delete, rename or push it.
- PR #1 is closed and unmerged; do not reopen it or use its feature branch.

## Active task completed
- Fixed a VPN-client restart rollback gap in `trunk/user/rc/src/vpn_client.c`: after the forced VPNC stop successfully removes shared `xl2tpd`, the follow-on identity-aware client teardown can fail and previously returned immediately. The new failure path restores configured WAN-L2TP/VPN-server shared `xl2tpd` while still holding the lifecycle lock, logs restore failure, and preserves the original stop status and errno.
- Added a focused policy header and decision-matrix regression to `tests/vpn_server_restart_policy.c`; strengthened `scripts/check_vpn_server_lifecycle.py` to verify restoration order, failure logging and first-error preservation. Added the new header to VPN stability and XY-C1 path filters.
- The shared-daemon restart policy helper is intentionally gated by a confirmed successful forced stop and existing WAN-L2TP or L2TP-server configuration; a failed force-stop preflight does not trigger a new daemon start.

## Verification boundary
- Exact-head SHA `a0cb9aad4844f7d044ce79efad6c417857c95972`: VPN stability `44260005390` PASS; Firewall startup `44260005246` PASS; DNSMasq lifecycle `44260005238` PASS; HTTPD security `44260005241` PASS; Process inventory `44260005245` PASS; Process lifecycle regression `44260005256` PASS; NVRAM boundary audit `44260005237` PASS.
- Exact-head XY-C1 `44260005493` was IN_PROGRESS at the last poll: Build was running; TRX validation and artifact upload were PENDING. Do not claim L3 for `a0cb9aad` until all three finish successfully on that same SHA.
- Local isolated GNU89/Werror/ASan/UBSan test of the new decision-policy header passed. The full repository VPN host regression passed through VPN stability run `44260005390`.
- The prior Internet-monitor source SHA `8c272644dfc86a047eed50dd48aee42400607b72` has XY-C1 run `44258150922` SUCCESS, including Build, TRX and artifact upload. The checker-only follow-up `764d7c7aeae4577f287cc6b5462d36c7e86aaa0c` passed Detect Internet signal regression `44259182501` and AI handoff docs `44259181122`.
- No physical-router test is claimed. L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main` and recheck exact-head XY-C1 run `44260005493`; inspect Build, TRX and artifact step outcomes on SHA `a0cb9aad4844f7d044ce79efad6c417857c95972`. If any step fails, inspect its causal log before changing firmware/source.
2. With XY pending or after it passes, continue the P1 whole-userland RC startup/lifecycle reliability audit. Avoid re-auditing completed APCLI/WISP, XUPnPD, WAN sysctl, SCUT/MentoHUST, Internet-monitor, Adbyby, ttyd and this VPN rollback slice unless a new exact-head failure provides evidence.
3. Keep checking the exact SHA for each new atomic commit; pending independent jobs must not block other work.
4. Preserve the frozen backup and keep physical-router L4 marked NOT VERIFIED.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. At session end, inspect live main, exact-head Actions and the first next task. If these facts remain accurate, do not create a no-op documentation commit.

# Cross-session execution mandate
Before context exhaustion, leave a precise continuation checkpoint. Deliver one atomic main commit per logical source/test milestone, verify exact-SHA Actions, then continue to the next independent ranked defect. Never force-push, mutate the backup, reopen the retired PR, or claim unperformed tests/build/hardware checks.
