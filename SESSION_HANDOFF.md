# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`; code/test SHA for the just-closed ZeroTier readiness slice: `3f435a03995afb1130da7d3acfecf65c8531ee9f` (parent `fdf89096d631245efab7534cd399abeb498a0acd`). Remote branch inventory contains only `main`; no checkpoint branch exists.
- ZeroTier stop/start and readiness are now L1/L2 CLOSED on exact SHA `3f435a03995afb1130da7d3acfecf65c8531ee9f`: lifecycle [43148551947], AI handoff docs [43148552085] and infosvr packet/signal [43148551957] all completed SUCCESS. All steps and complete logs were inspected; each checkout's `git log -1` matched this SHA.
- The ZeroTier host regression printed `ZeroTier bounded stop/start, readiness and persistent-config preservation: PASS`; the semantic checker printed `ZeroTier bounded lifecycle, persistent-config and RC integration contracts: PASS`. Docs validator reported root log=43/119 and archives dev-log=7, handoff=22, roadmap=18. The earlier checker false-negative on `fdf89096...` was corrected and the latest exact-head rerun passed.
- DDNSTO stale-PID stop/restart remains L1/L2 CLOSED on exact SHA `f39638ac28bae38e6ec8d2694831685b3be98d4e`: DDNSTO [43140910335] and AI docs [43140910294] SUCCESS.
- Root `dev_log.md` has 44 complete records after THIS COMMIT; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- FTP failed-upload cleanup TOCTOU remains P2 OPEN/BLOCKED pending a Linux 4.4-safe inode/parent proof preserving STOR/APPE/REST/STOU and concurrent external writers. NFS async durability/AUTH_SYS/root-squash remains real-client evidence-gated.
- The development container has no full repository checkout; source and host regression verification was performed through exact-head GitHub Actions. Production MIPS/uClibc, full firmware/TRX and L4 results remain distinct and unverified.

## Active task — Continue whole-tree add-on lifecycle audit: AdGuardHome (P2 stability)

- The previously active ZeroTier stop/start and readiness issues are closed at L1/L2 on exact code/test SHA `3f435a03995afb1130da7d3acfecf65c8531ee9f`. Do not repeat that work absent a new P0/P1 finding.
- The next package path is `trunk/user/adguardhome/` (confirmed in the repository's `trunk/user` directory). Trace its Makefile installation, shell wrappers, all RC callers, process identity, bounded start/stop postconditions, duplicate/concurrent starts, configuration and secret publication, firewall/route/interface cleanup, resource lifetime and error propagation.
- Choose one concrete, reproducible stability/security defect. Keep the change bounded; add an isolated host regression and source/caller-graph contract, using the existing CI pattern only where useful. Do not infer a bug before examining the full call path.
- After AdGuardHome, rotate to AliyunDrive WebDAV, FRP, AliDDNS or Adbyby based on the whole-tree inventory; do not stay in one module without a new high-priority finding.
- FTP failed-upload cleanup TOCTOU remains P2 OPEN/BLOCKED until a Linux 4.4-safe inode/parent design proves preservation of STOR/APPE/REST/STOU and concurrent writers. NFS remains real-client evidence-gated.

## Verification boundary

- ZeroTier stop/start and fail-closed persistent cleanup: L1/L2 CLOSED on `1bf8a93bfa47770e8005d5241f7077287c86c4c4`; lifecycle [43143884566] and docs [43143884420] SUCCESS.
- ZeroTier interface and NAT-route readiness: L1/L2 CLOSED on exact SHA `3f435a03995afb1130da7d3acfecf65c8531ee9f`; lifecycle [43148551947], AI docs [43148552085] and infosvr [43148551957] all SUCCESS. The lifecycle regression and full source/RC/ROMFS semantic contract both printed PASS; the docs validator passed at 43/119 with archive counts 7/22/18. Every job step/full log was inspected.
- DDNSTO stop/restart L1/L2 CLOSED on exact SHA `f39638ac28bae38e6ec8d2694831685b3be98d4e`; DDNSTO [43140910335] and docs [43140910294] SUCCESS.
- No local full clone was used. Host tests and source contracts were executed by Actions; no local test result is claimed.
- Production MIPS/uClibc, target BusyBox command/output semantics, live router interface/route and firewall state, complete firmware/TRX and L4 physical-router behavior remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`, the remote branch inventory and the newest Actions; record exact SHAs. The ZeroTier L1/L2 evidence is already closed on code SHA `3f435a03995afb1130da7d3acfecf65c8531ee9f` by [43148551947, 43148552085, 43148551957]. If newer code has landed, reconcile before proceeding; do not rerun or reimplement the closed ZeroTier slice without cause.
2. Begin the next stability audit at `trunk/user/adguardhome/`: read its package Makefile and all scripts, then trace every startup/shutdown/restart path and RC caller before selecting one reproducible bug. Add a focused host regression and semantic source contract; inspect exact-head Actions and persist the changed handoff state.
3. Keep FTP upload-cleanup TOCTOU OPEN/BLOCKED until a Linux 4.4-safe inode/parent design proves STOR/APPE/REST/STOU and external-writer compatibility. NFS requires real-client evidence; MIPS/uClibc firmware/TRX and L4 remain NOT VERIFIED.
