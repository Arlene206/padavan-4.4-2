## Current repository state

- Repository: `Arlene206/padavan-4.4-2`. Only `main` is authorized for development, commits and push validation. Current change is based directly on main SHA `18234ff0a41eb5343aaf58207b3cf641bbf4a244`; refetch the exact live head before every subsequent write.
- Branch inventory at recovery contained only `main`. Legacy PR #1 is CLOSED, not merged; its valid AliDDNS source, helper, package and host-test implementations were compared with main and are already present byte-identically. Main's workflow/checker have newer integrated contracts; no missing implementation needs to be cherry-picked. GPT Session Rotator remains immutable.
- The previous AliyunDrive WebDAV task is now L2-closed on exact main SHA `18234ff0a41eb5343aaf58207b3cf641bbf4a244`: lifecycle run [43287688178] / job [131425308994] passed launcher syntax, bounded stop, process-name signalling, duplicate-start/argv integrity, serialized start/stop and asynchronous process-readiness regression plus source/RC contracts. The corrected async fixture uses a real 50 ms scheduler yield and explicit timeout sentinel. It proves process visibility within the bounded poll; it does not prove WebDAV HTTP readiness.
- Same-SHA push Actions on `18234ff0a41eb5343aaf58207b3cf641bbf4a244` all completed SUCCESS: AI handoff docs [43287688283] / job [131425309408]; AdGuardHome [43287688231] / job [131425309147]; AliyunDrive [43287688178] / job [131425308994]; infosvr [43287688185] / job [131425309273]; AliDDNS security [43287688650] / job [131425310006]. Complete job steps/logs were inspected. The AI validator reported root log=75/119 and archive inventory dev-log=7, handoff=22, roadmap=18.
- The previous XY-C1 target firmware milestone remains L3-verified only on `fcdc05e76e6522f9f35b59b71bd222b884cb2ce4`: run [43273846470] / job [131376901927] passed MIPS build, TRX verification and artifact upload. Artifact size 12,573,414 bytes; SHA-256 `f4adfa9a13224e519cd580946e30e6a3d336b3da410f0842051eb697a7baee08`. This is not a build on current main. L4 router/hardware validation remains NOT VERIFIED.

## Active task

The resumed AliyunDrive lifecycle regression is closed at L2 on `18234ff0a41eb5343aaf58207b3cf641bbf4a244`. This iteration rotates to AliDDNS lifecycle reliability after tracing the complete control route: HTTPD `variables.c` maps `aliddns_*` and persistent `scripts.ddns_script.sh` changes to `EVM_RESTART_ALIDDNS`; `rc.c` dispatches `RCN_RESTART_ALIDDNS`; `services.c` previously implemented restart as separate stop then start calls; those calls launch `/usr/bin/aliddns.sh`.

THIS COMMIT adds:
1. A stable `/tmp/.aliddns.lifecycle.lock` inode, strict owner/mode/link-count and symlink checks, BusyBox descriptor-mode `flock -n 9`, and a bounded 10-second contention wait which fails closed. The lock is never unlinked.
2. One locked start/restart/stop/check dispatcher. Restart now executes as one shell action while holding the same lock instead of allowing concurrent RC/UI start or stop notifications to interleave the C wrapper's separate calls.
3. Explicit FD 9 closure when the launcher detaches the `keep` daemon or schedules a restart, so children cannot retain the short-lived lifecycle lock.
4. A POSIX host regression for lock-file trust, cross-process contention, bounded failure, descriptor release, action dispatch and atomic restart ordering; the semantic checker validates HTTPD→RC→service mapping, target BusyBox `CONFIG_FLOCK` and flock implementation, plus workflow path coverage.

The existing HTTPS-only AliDNS API, CA bootstrap, OpenSSL EVP_MAC HMAC helper and secret-safe stdin framing are preserved. Important remaining lifecycle issue: legacy `kill_ps()` still discovers processes through broad `ps | grep` matching and sends TERM/KILL without a bounded disappearance postcondition; this change serializes control operations but does not claim to repair process identity or stop confirmation. AliyunDrive refresh-token/auth-password argv exposure, FTP cleanup TOCTOU and NFS real-client evidence remain open.

## Verification boundary

- L1: bootstrap/protocol, live main, handoff, roadmap, newest root devlog, archive index, branch inventory and closed PR #1 implementation comparison reviewed. The AliDDNS HTTPD→RC→services notification chain and BusyBox target flock configuration/source were traced.
- L2 previous exact-head result: all five push workflows listed above SUCCESS on `18234ff0a41eb5343aaf58207b3cf641bbf4a244`; AliDDNS suite included HTTPS/API error handling, public IPv4 and IPv6 validation, bounded DNS lifecycle, custom-script source/atomic seeding, CA bootstrap, HMAC byte equivalence and AccessKeySecret process argv/environment privacy. AliyunDrive cross-process lifecycle/readiness tests also PASS on that SHA.
- L2 for this commit's new AliDDNS lifecycle-lock test, source contract and docs validator: PENDING exact-head push Actions; no success is inferred from the base SHA.
- No local full repository checkout was available; local shell/Python execution is NOT CLAIMED. The CI host tests are the intended L2 evidence.
- L3 MIPS/uClibc package/link and full firmware/TRX/artifact build are NOT VERIFIED for the current code. The older XY-C1 result above is not inherited by this commit.
- L4 physical router flashing, boot and runtime behavior: NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and inspect every push-triggered Actions run whose `head_sha` equals this commit. Read every job step and full log. Require AliDDNS launcher/test `sh -n`, the new lifecycle-lock host regression (unsafe lock-file rejection, cross-process contention, bounded failure, FD release and atomic restart ordering), the complete AliDDNS API/security suite, the semantic HTTPD→RC/BusyBox-flock contract and AI documentation/archive validator to pass on that exact SHA. Fix the first causal failure directly on `main` using the current exact SHA. After L2 closes, continue AliDDNS by designing a proved process-identity and bounded-stop postcondition for `kill_ps()`; then rotate to AliyunDrive secret argv and FRP/Adbyby audits. Do not weaken user custom `ddns_script.sh` preservation, FTP TOCTOU safeguards or NFS evidence requirements.
