# Session Handoff — 2026-11-26

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main before the current IAPP monotonic TCP-deadline slice: 1ddc1f33e12789cc218f6ccfa8dd96d00c93fd93.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807; it is read-only and must not be modified. Legacy PR #1 remains closed/unmerged.
- Startup/bootstrap/protocol/handoff recovery was completed once. Do not repeat audits for already-completed work.
- Publish each logical task as one complete Git tree and one atomic main commit; update main only with a non-forced expected-SHA compare-and-swap. Never force-push or modify the backup.

## Active task

- Fix the Ralink IAPP Linux TCP receive path: its nominal five-second total frame deadline used `time(NULL)`, so wall-clock/NTP changes could extend or prematurely expire a peer session and block the daemon's event loop. Use one absolute `CLOCK_MONOTONIC` deadline for all frame reads and the overflow-byte check; keep the legacy non-Linux fallback.
- Verify the new deadline conversion policy under GNU89/Werror/ASan/UBSan, then inspect exact-SHA IAPP and XY-C1 Build/TRX/artifact Actions. Pending checks remain non-blocking for independent work.

## Completed source slice

- Entware generic.sh root bootstrap now downloads through verified HTTPS into private staging, patches the upstream HTTP origin and feed config to HTTPS, uses bounded timeouts and set -e, validates patched shell before execution and verifies final opkg/config postconditions.
- The legacy Oleg feed still points to http://ipkg.nslu2-linux.org/feeds/optware/oleg/cross/stable; no verified HTTPS mirror was established. The hardened ipkg shell client refuses HTTP/FTP/unknown schemes for both index/file downloads and direct package installs, allows HTTPS and local files, and returns failure for unsupported input instead of reporting false success.
- Legacy package bootstrap validates every configured source before privileged package operations and refuses the shipped HTTP-only feed rather than guessing an HTTPS endpoint. It uses the firmware shell package client for index updates, not the unaudited legacy binary. Optware startup releases its lifecycle lock and returns failure if package bootstrap/update fails.
- Samba legacy config file publishing now syncs the staged file before rename and syncs the containing directory after rename so a successful publish means both contents and directory entry were handed to the filesystem for durability.
- Ralink IAPP Linux TCP frame reception now uses a single absolute `CLOCK_MONOTONIC` deadline across partial reads, EINTR retries and the final overflow check. The timeval conversion rounds positive sub-microsecond time upward; the non-Linux wall-clock fallback is retained.


## Verification boundary

- Entware source commit: 967737f91d830c373e8400afe3ff16d0a7d5e080; checker correction 5b8939f35e2d318f1d5fb964dc393366eb7e6e23; OpenSSL command-loop correction 48d442a96a3f2455703e3e73042f7c882505110c.
- Legacy ipkg complete source/test correction: b63ce11dea3ee18b00a32ad79a1786c6eb482160; follow-up direct-URI/path-filter/handoff correction: 93ec42b2b3927ab0093c26b543e820cf86f51f40.
- Exact-head Process regression #45772642959 PASS, Process inventory #45772642896 PASS and AI handoff #45772642788 PASS on source SHA 93ec42b2b3927ab0093c26b543e820cf86f51f40. The Process run executed the legacy ipkg transport shell fixture and static source contract. XY-C1 Build run #45772642912 remained IN_PROGRESS at the last poll; do not claim its Build/TRX/artifact validation until it completes.
- Host-only Entware shell fixture passed five groups; host-only ipkg transport fixture passed four groups. No local XY-C1 firmware build, target runtime, Samba hardware test or physical-router L4 is claimed.
- Exact-head Samba share configuration stability #45773307948 PASS, AI handoff #45773307934 PASS and Process lifecycle inventory #45773307939 PASS on 1ddc1f33e12789cc218f6ccfa8dd96d00c93fd93. NVRAM audit #45773308089 was IN_PROGRESS at last poll; no router power-cut test is claimed.

- Exact-head legacy-ipkg Process #45772642959 PASS, Process inventory #45772642896 PASS and AI handoff #45772642788 PASS on 93ec42b2b3927ab0093c26b543e820cf86f51f40. Its XY-C1 Build run #45772642912 was IN_PROGRESS at the last poll.
- IAPP monotonic-deadline host C policy regression PASS under GNU89/Werror/ASan/UBSan. The new IAPP production slice has not yet been published or checked by Actions. No local XY-C1 build, target runtime or physical-router L4 is claimed.

## Exact first action for the next GPT

1. Fetch live main, then inspect the exact-SHA IAPP lifecycle and XY-C1 Actions for the monotonic TCP deadline slice. Read failed job logs and fix source/checker/workflow regressions; confirm Build XY-C1, TRX validation and artifact upload separately.
2. Resolve any still-pending exact-head legacy-ipkg XY-C1 run from SHA 93ec42b2b3927ab0093c26b543e820cf86f51f40 without repeating its source audit; likewise close the NVRAM audit for the Samba SHA if still pending.
3. Continue a concrete stability/security source fix in another Padavan userland service; do not repeat completed startup recovery or the Entware/legacy-ipkg transport audits.

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For every logical slice, create all blobs, one complete tree, one atomic main commit, and publish with a non-forced expected-SHA compare-and-swap. Before context exhaustion, finish the current safe slice and checkpoint its first concrete next action. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect live refs and exact-SHA Actions before ending. If the current handoff remains accurate, leave it untouched; do not create a no-op commit. Record material PASS/FAIL/PENDING changes and the next concrete task. A commit cannot contain its own resulting SHA; the next GPT must fetch live main before checking its Actions.
