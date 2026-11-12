# Padavan 4.4 — Live session handoff

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`; code baseline before THIS COMMIT: `f8c154c5443514df45c01d8d74ffcb21366b7faf`. Remote branch inventory contains only `main`; no checkpoint branch exists.
- The preceding ZeroTier bounded stop/start and interface/NAT-route readiness work remains L1/L2 CLOSED on exact code/test SHA `3f435a03995afb1130da7d3acfecf65c8531ee9f`: lifecycle [43148551947], AI handoff docs [43148552085] and infosvr [43148551957] all SUCCESS. Current main's doc-only successor `f8c154c5443514df45c01d8d74ffcb21366b7faf` was reconciled; current-head infosvr [43149323346] and AI docs [43149323154] both SUCCESS.
- Exact-head logs/steps were inspected for all five runs. Current main docs validator reports root log=44/119 and archive inventory dev-log=7, handoff=22, roadmap=18. ZeroTier lifecycle and source/RC/ROMFS contracts remain PASS.
- AdGuardHome audit traced package selection and ROMFS installation, shell config/start/stop functions, RC service wrappers, stop_services integration and RCN_RESTART_ADGUARDHOME notification dispatch. Found generated `/etc/storage/adg.sh` contains the Web UI password and TLS private-key field but was created with mode 0755; existing configs were never tightened on subsequent starts.
- THIS COMMIT stages generated defaults with same-directory mktemp under umask 077, rejects symlink/non-regular destinations, checks stage write/chmod/rename, publishes atomically, and enforces mode 0600 for both new and existing configuration files. `start_adg()` now fails closed before DNS/firewall mutation when config protection/publication fails. The new host regression and semantic RC/ROMFS contract workflow are pending exact-head Actions.
- Root `dev_log.md` has 45 complete records after THIS COMMIT; no rotation or INDEX update is due. Archive inventory remains dev-log=7, handoff=22, roadmap=18.
- FTP failed-upload cleanup TOCTOU remains P2 OPEN/BLOCKED pending a Linux 4.4-safe inode/parent proof preserving STOR/APPE/REST/STOU and concurrent external writers. NFS async durability/AUTH_SYS/root-squash remains real-client evidence-gated.
- No full local repository checkout was used. Production MIPS/uClibc, complete firmware/TRX, target BusyBox behavior and L4 hardware results remain NOT VERIFIED.

## Active task — Verify AdGuardHome secret-config publication, then continue its lifecycle audit

- The bounded AdGuardHome config confidentiality fix is in THIS COMMIT. Inspect the exact-head `.github/workflows/adguardhome-config-security.yml` and AI handoff-doc workflow, including every step and full logs. Require the 0600/default+existing config tests, symlink/FIFO rejection, failed rename/chmod cleanup, startup fail-closed ordering, ROMFS/package/RC caller contracts, symmetric path filters and docs validation to pass on one exact main SHA.
- On green exact-head evidence, close this config-publication slice at L1/L2. Do not claim production target or router verification.
- Continue the same whole-package audit without repeating the completed config work: duplicate/concurrent starts, daemon TERM/KILL and bounded stop postconditions, process identity, resolver rollback, duplicate/stale IPv4/IPv6 PREROUTING redirects when addresses change, cleanup errors and RC notification producer/caller chain. Pick the next separately reproducible defect only after tracing all callers.
- After AdGuardHome, rotate across AliyunDrive WebDAV, FRP, AliDDNS or Adbyby rather than staying in one add-on without a new high-priority finding.
- FTP failed-upload cleanup TOCTOU remains P2 OPEN/BLOCKED until a Linux 4.4-safe inode/parent design proves STOR/APPE/REST/STOU and concurrent external-writer compatibility. NFS requires real-client evidence.

## Verification boundary

- ZeroTier stop/start and fail-closed persistent cleanup: L1/L2 CLOSED on `1bf8a93bfa47770e8005d5241f7077287c86c4c4`; lifecycle [43143884566] and docs [43143884420] SUCCESS.
- ZeroTier interface and NAT-route readiness: L1/L2 CLOSED on exact SHA `3f435a03995afb1130da7d3acfecf65c8531ee9f`; lifecycle [43148551947], AI docs [43148552085] and infosvr [43148551957] all SUCCESS. Host regression and full source/RC/ROMFS contract passed; docs validator reported 43/119 and archives 7/22/18.
- Current-head documentation/infosvr checks on `f8c154c5443514df45c01d8d74ffcb21366b7faf`: AI docs [43149323154] and infosvr [43149323346] all SUCCESS; docs validator reported 44/119 and archives 7/22/18.
- AdGuardHome new regression/source contract are committed in THIS COMMIT; exact-head Actions are pending. No local execution or target result is claimed.
- Production MIPS/uClibc, target BusyBox runtime, AdGuardHome live resolver/firewall/process behavior, complete firmware/TRX and L4 physical-router behavior remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and remote branch inventory; record exact SHA and inspect exact-head AdGuardHome lifecycle/security and AI docs workflows, every step and complete logs. The code/test change is THIS COMMIT, based on `f8c154c5443514df45c01d8d74ffcb21366b7faf`; do not rely on ancestor Actions.
2. If both new regressions and docs validation pass on the same SHA, close config confidentiality at L1/L2 and continue AdGuardHome's remaining lifecycle audit, starting with stop/duplicate/concurrent start behavior and address-change-safe DNS/firewall cleanup. If any check fails, fix only the demonstrated regression and rerun exact-head Actions.
3. Preserve FTP TOCTOU OPEN/BLOCKED and NFS real-client evidence gate. MIPS/uClibc firmware/TRX and L4 remain NOT VERIFIED.
