# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository and exact-SHA Actions before changing source. Record the source baseline, current CI state, validation boundary and the next precise task. Use one atomic main commit for material changes only. Before context exhaustion, finish any safe in-progress slice and record exact SHA, test and CI state for the next GPT.

- Work only on `main`; never force-push, create/use side branches or PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.
- Avoid repeating completed PID 1, storage, SmartDNS and AliDDNS audits below.

## Current repository state

- Product/source baseline before this handoff-only documentation commit: `33a91907446b7854c9736d3246152382dbb5f218`.
- Frozen backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; last checked unchanged.
- Handoff was refreshed after a sequence of source/test commits. The documentation commit itself changes only this file and `dev_log.md`; on takeover, fetch live `main` first and treat its SHA as authoritative.

## Completed implementation slices in this session

### SmartDNS / Adbyby
- `4cd7a0c90abb8142041f5d6da8622cdd3885cce0`: added checked Adbyby NVRAM reads; an unreadable `adbyby_enable` or `adbyby_add` now aborts the DNS ownership transition without changing the active host mode or attempting a write. Extended `trunk/user/smartdns/tests/dnsmasq_config_preservation.sh` and `scripts/check_smartdns_dnsmasq_config.py`.
- `8a680956a3c93c8147d8178f5ebbf35744cedd15`: added fail-closed checked reads for SmartDNS primary/secondary enable, port and redirect settings before lifecycle changes; expanded the same runtime test and source checker.
- Existing unsupported `/usr/bin/adbyby.sh switch` dispatch remains absent; no repeated work is needed on that already-closed item.

### AliDDNS
- `56f1a914f1dda580c23c55d3c49f53d1363b4a5b`: checked startup reads for enable, interval, API credentials, domain/record names, TTL and retry configuration. A successful empty enable read still persists the legacy disabled default; a backend read failure is no longer mistaken for an absent key. The keeper retries on an unreadable live enable state rather than stopping a healthy process.
- `a838773749f5575022173530c7ad12181d7e8975`: checked the persisted configuration fingerprint read and write; errors no longer publish an unknown restart decision.
- `00f91436e5acb98acbe43962ee11154eb3c4faaa`: made retry-cancel reads status-bearing; an NVRAM error is not interpreted as a request to cancel cooldown.
- `b884b840d0518915826d85aa6436ccd35f26cfbc`: introduced checked retry-counter and pre-relaunch fingerprint-reset writes.
- `a6cad2013377022f9e37d1206fe274c71ca5346a`: checked retry lock file creation/update/removal, rejected symlink writes and non-regular removal targets, and propagated filesystem failures.
- Test/checker follow-ups: `dec612dca4834356f56f96cb04469a0156467500`, `b8c42c7c2084869f3efc4401229832612ff35387`, `674963330b80848e76a15bfb69fd7574a3daf0be`, `58eb9d5e6f28cc80161eca74cdef701bb6db9376`, and `296dd893b20d14c7487bda4565cd15562f619c86`. They correct fixture newline escapes, checker marker registration and an existing-directory fixture for the retry lock; production behavior is in the source commits above.

### SCUTClient stop / logout
- `33a91907446b7854c9736d3246152382dbb5f218`: corrected `func_stop()` in `trunk/user/scutclient/scripts/scutclient.sh`. It now resolves `wan_ifname` via `get_checked` before stopping the local daemon, so an NVRAM read failure leaves the daemon running and avoids a stranded remote session. It also propagates the logout command's failure rather than always reporting success.
- Added regression cases to `scripts/tests/legacy_wrapper_pidfile_safety.sh`: failed WAN-interface read must not stop/logout; successful stop performs logout; failed logout returns its status. The test's process lifecycle mock records stop attempts.

### CI trigger correction
- `f3e0945c611a3c553bdc74c8aaf35388850ce076`: aligned push and pull-request path filters for the process lifecycle workflow so the authentication-client NVRAM regression path is included in both triggers.

## Active task

- The committed source/test baseline is `33a91907446b7854c9736d3246152382dbb5f218`; the handoff-only commit follows it. XY-C1 run #45066964181 is still pending at the checkpoint; no source change is awaiting review.
- Inspection of the exact bundled SCUTClient upstream revision `a58996875c61d0cdf8ec435057befc1c87bd6863` confirmed `src/main.c` supports only `-u/--username` and `-p/--password`, assigns `optarg` directly to `UserName` / `Password`, and has no config-file or file-descriptor credential input. The package README documents the same argv interface. This is a confirmed argv credential exposure surface and is not yet fixed; do not use environment variables as a superficial workaround.
- Next implementation: add a maintained patch/overlay to the pinned upstream source for a private credential-file interface. Use `O_NOFOLLOW`, strict regular-file and permission checks, bounded parsing, unlink immediately after successful open/read, and wrapper-side atomic mode-0600 file creation/cleanup. Add tests proving secrets are absent from process argv and malformed, symlinked, unsafe-mode and read-failure files are rejected. Integrate patching idempotently into the package extraction/build path, then validate with XY-C1 cross-build.

## Verification boundary

Latest source/test SHA `33a91907446b7854c9736d3246152382dbb5f218`:
- Legacy wrapper stale-pidfile safety #45066964140 — completed / PASS (includes the updated SCUTClient stop/logout regressions).
- Process inventory and lifecycle fail-closed #45066964156 — completed / PASS.
- Process lifecycle inventory #45066964164 — completed / PASS.
- XY-C1 Firmware Validation #45066964181 — IN PROGRESS at the checkpoint; the job was still installing build dependencies. Do not claim Build, TRX validation or artifact upload until this exact run completes successfully.

Previous exact source/test SHA `296dd893b20d14c7487bda4565cd15562f619c86`:
- AliDDNS signed API transport security regression #45066583234 — completed / PASS.
- Process lifecycle inventory #45066583292 — completed / PASS.

Earlier same-area evidence:
- SmartDNS/DNSMasq lifecycle regression #45064508066 on `8a680956a3c93c8147d8178f5ebbf35744cedd15` — completed / PASS; its source checker and runtime configuration preservation coverage include the new checked-read behavior.
- AliDDNS test failures on intermediate commits were traced to test fixture newline escapes / assertion fixtures, not the production change. They were corrected in descendants; exact run #45066583234 on `296dd893...` passed. Do not reopen unless a new exact-head failure appears.
- DNSMasq C parser multicharacter-constant correction on `8167ccca55841ff09e1af4e769318ee4a2b1df12` passed its exact DNSMasq, process inventory and NVRAM workflows; it was performed by a parallel main contributor, not this session.

No local shell/C execution was performed in this session. GitHub Actions host tests are the only claimed execution evidence. XY-C1 build/TRX/artifact status on the latest source SHA remained pending; router/hardware L4 validation is NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch the current `main` SHA and exact-SHA Actions. Preserve the frozen backup.
2. Investigate SCUTClient credential exposure: inspect the bundled upstream source archive `trunk/user/scutclient/scutclient-a58996875c61d0cdf8ec435057befc1c87bd6863.tar.gz` to determine whether `-u` / `-p` credentials passed by the wrapper are exposed in process arguments and whether upstream supports a private config file, protected file descriptor or another non-argv interface. Do not invent an unsupported CLI option. If a viable non-argv interface exists, implement it with mode-0600/atomic config handling and a regression proving credentials do not appear in process arguments; otherwise document the exact upstream constraint and select the next concrete stability/security defect.
3. Revisit XY-C1 run #45066964181 for the exact `33a91907446b7854c9736d3246152382dbb5f218` SHA. Record Build, TRX and artifact stages separately; hardware L4 remains separate.
4. Continue with a source/test/checker slice, one atomic commit and exact-SHA Actions. Update this handoff only when next-session-critical facts materially change.
