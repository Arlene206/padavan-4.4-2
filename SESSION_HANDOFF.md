# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If next-session-critical facts have changed, update this handoff; if it is already accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; telnetd/ttyd process-preflight and launch diagnostics landed at `89c971cb95347ef7be7545989ea625345eaecf65`. This follow-up fixes OpenSSH's shell wrapper, which previously forced exit 0 after sshd/keygen/stop errors.
- Boot diagnostics cover networkmap, watchdog, infosvr, GPIO, LLTD and rstats. This slice fixes SCUT/vlmcsd wrappers that previously masked daemon launch failure as success.
- LPD printer-removal PID identity hardening landed in `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3`; source-contract formatting correction is `022bc51507ca5f395c60a1f450cc461bf5403926`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P1 — Boot daemon startup error reporting

- Networkmap `start_networkmap(1)`, watchdog `start_watchdog()`, infosvr `start_infosvr()`, and now `start_lltd()` / `start_rstats()` log enabled launcher failures at the call site and return the original status.
- `start_gpio_btn()` already logs UNKNOWN inventory, launcher failure, UNKNOWN postcondition and immediate exit. Crond has bounded post-launch verification and failure logs.
- LLTD and rstats still return 1 when their NVRAM enable setting is off; only enabled `eval()` failures are logged. The process-lifecycle checker asserts those sentinels, result capture, diagnostic and status propagation. Both the process-lifecycle and XY-C1 workflows filter `services.c`.
- DNS Forwarder already checks launcher status, logs failures and verifies process appearance. The iperf3 package now includes its missing service script, with bounded startup/stop checks and fail-closed tool preflight. APP_MENTOHUST's RC wrapper already preflights process presence, but `mentohust.sh` previously printed failures while unconditionally exiting 0; this slice preserves launcher and stop statuses through the shell dispatcher.

### P1 common_ex PID-file SIGKILL audit — LEGACY PID-ONLY HELPER REMOVED

- All 36 top-level trunk/user/rc/src/*.c files were searched for kill_process_pidfile(). It had no in-tree RC C caller; its only implementation used PID-only kill_pidfile() and kill_pidfile_s(SIGKILL). The unused helper and its rc.h declaration are removed in this source slice.
- The ten live kill_services_with_pidfiles() callsites across VPN server/client, WAN/LAN DHCP, USB modem, detect_internet, detect_link and INICD use process_pidfile_read_identity() through service_lifecycle_policy.h. Existing tests cover UNKNOWN preflight, replacement identity blocking KILL escalation and force-only PPPD targets.
- This does not close Linux 4.4's final identity-check-to-kill(2) race; no atomic pidfd-like primitive is available in this target baseline.

## Verification boundary
- On exact parent SHA af9676af871386982c4a50b12b7ade9a8d2b9ad5: process-lifecycle #43735463425 PASS; process inventory artifact #43735463474 PASS; AI docs #43735463542 PASS; infosvr #43735463436 PASS; AdGuardHome #43735463446 PASS; AliDDNS #43735463393 PASS; AliyunDrive #43735463525 PASS. XY-C1 #43735463489 and NVRAM #43735463395 were IN_PROGRESS at last poll. This ttyd/telnetd slice requires fresh exact-head process/XY runs.
- Exact e950 Actions: process inventory/lifecycle #43734448717 SUCCESS including SCUT/vlmcsd mocked shell regressions; AI docs #43734448768 SUCCESS; process lifecycle inventory #43734448702 SUCCESS; infosvr #43734448825 SUCCESS; AdGuardHome #43734448705 SUCCESS; AliDDNS #43734448677 SUCCESS; AliyunDrive #43734448782 SUCCESS. XY-C1 #43734448706 and NVRAM audit #43734448863 were IN_PROGRESS at last poll; fresh exact-head validation is required for the iperf3 additions.

- XY-C1 #43734448706 for exact e950 was IN_PROGRESS at Build setup stage; it validates the SCUT/vlmcsd scripts but predates this iperf3 package change. Require the new exact-head build, TRX verification and artifact upload before L3.
- LPD source SHA `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3` initially failed only at wrapped-call substring contracts. The one-line formatting correction in 022bc was verified by process lifecycle #43732981693 and HTTPD #43732981614 PASS; AI docs #43732981749 PASS.
- Prior L3 evidence at product SHA `30c5b48ba8a8e996d13548280a54a9b7f5dbda7a` does not apply to later source changes. Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Verify exact-head process-lifecycle, AI docs and XY-C1 after the OpenSSH script correction; require Build XY-C1, TRX verification and artifact upload before any L3 claim.
2. Continue the shell-wrapper audit at other privileged daemon scripts; next inspect NFS daemon start/reload error propagation and postconditions.
3. Keep main commits atomic using a complete Git tree and expected-SHA non-forced ref update. Keep backup unchanged.

## Invariants

- A PENDING run does not block independent work; retain exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve docs/dev-log archive shards and docs/dev-log/INDEX.md byte-for-byte.
