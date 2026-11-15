# Cross-session execution mandate (read before resuming)

Recover live state once, then continue the exact P1 task. Do not repeat completed audits, stop at a plan, or ask the user to select work. Continue independent engineering while CI is pending. Before context exhaustion, refresh the exact first action and verification boundary so the next GPT can resume without repeating completed work.

**Mandatory handoff checkpoint:** Verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional.

For each multi-file logical slice, publish source, tests/contracts and necessary docs in **one atomic main commit**: create all blobs, one tree, one commit parented to freshly fetched main, then non-forced update of main using that exact expected SHA. No branches/PRs, no force-push, no archive modifications.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; work continues on `main`. LLTD/rstats boot diagnostics landed at `32e10009d28e4d37013b5defbdc526d7e9aa5063` (parent `022bc51507ca5f395c60a1f450cc461bf5403926`). Current SCUT/vlmcsd shell launch fix is parented to the verified live head `ba29a126870c7f57d4317edd2a0bd9c48508fa7d`.
- Boot diagnostics cover networkmap, watchdog, infosvr, GPIO, LLTD and rstats. This slice fixes SCUT/vlmcsd wrappers that previously masked daemon launch failure as success.
- LPD printer-removal PID identity hardening landed in `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3`; source-contract formatting correction is `022bc51507ca5f395c60a1f450cc461bf5403926`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, read-only. No local checkout/MIPS toolchain or physical-router L4 result is claimed.

## Active task

### P1 — Boot daemon startup error reporting

- Networkmap `start_networkmap(1)`, watchdog `start_watchdog()`, infosvr `start_infosvr()`, and now `start_lltd()` / `start_rstats()` log enabled launcher failures at the call site and return the original status.
- `start_gpio_btn()` already logs UNKNOWN inventory, launcher failure, UNKNOWN postcondition and immediate exit. Crond has bounded post-launch verification and failure logs.
- LLTD and rstats still return 1 when their NVRAM enable setting is off; only enabled `eval()` failures are logged. The process-lifecycle checker asserts those sentinels, result capture, diagnostic and status propagation. Both the process-lifecycle and XY-C1 workflows filter `services.c`.
- DNS Forwarder already checks its launcher status and daemon postcondition. Next investigate `start_iperf3()` referencing `/usr/bin/iperf3.sh` even though the tracked iperf3 package Makefile installs only `/bin/iperf3`.

## Verification boundary
- AI handoff docs #43733575878 FAILED at the required cross-session handoff phrase because the replacement checkpoint omitted the literal `Before context exhaustion`; this documentation-only follow-up restores the rule. Other exact-head 32e workflow results are to be polled; no inference from PENDING jobs.

- Exact source SHA `32e10009d28e4d37013b5defbdc526d7e9aa5063`: process-lifecycle checker now includes LLTD/rstats status contracts; exact-head run results are pending. Parent `022bc51507ca5f395c60a1f450cc461bf5403926` had process-lifecycle regression #43732981693 PASS; HTTPD security #43732981614 PASS; AI handoff docs #43732981749 PASS; process inventory #43732981821 PASS; infosvr #43732981611 PASS; AdGuardHome #43732981580 PASS; FRP #43732981591 PASS; AliDDNS #43732981623 PASS; AliyunDrive #43732981731 PASS; NVRAM transaction audit #43732981609 SUCCESS.
- XY-C1 #43733575797 for parent 32e1 was QUEUED. This slice adds both package-script paths to its path filter; require fresh exact-head Build, TRX verification and artifact upload before claiming L3.
- LPD source SHA `72ce1bb97323d4c417e8a5d891bbaf9c6ed3f2f3` initially failed only at wrapped-call substring contracts. The one-line formatting correction in 022bc was verified by process lifecycle #43732981693 and HTTPD #43732981614 PASS; AI docs #43732981749 PASS.
- Prior L3 evidence at product SHA `30c5b48ba8a8e996d13548280a54a9b7f5dbda7a` does not apply to later source changes. Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head process-lifecycle regression, AI handoff and XY-C1 for `32e10009d28e4d37013b5defbdc526d7e9aa5063`; repair any causal failure. Require Build XY-C1, TRX verification and artifact upload before any L3 claim.
2. Implement the missing iperf3 lifecycle script referenced by `start_iperf3()`, install it into ROMFS and add a mock-command regression with bounded startup readiness verification.
3. Keep main commits atomic using a complete Git tree and expected-SHA non-forced ref update. Keep backup unchanged.

## Invariants

- A PENDING run does not block independent work; retain exact run ID/SHA and continue safe tasks.
- DNSMasq/VPN lock-order and scoped RC process-signal audits are complete; do not repeat without new evidence.
- Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved. FTP failed-upload cleanup, NFS durability/AUTH_SYS and physical-router behavior remain separate unverified areas.
- Preserve docs/dev-log archive shards and docs/dev-log/INDEX.md byte-for-byte.
