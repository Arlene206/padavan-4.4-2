# Session Handoff — 2026-11-26

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main before the legacy ipkg verified-transport slice: 48d442a96a3f2455703e3e73042f7c882505110c.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807; it is read-only and must not be modified. Legacy PR #1 remains closed/unmerged.
- Startup/bootstrap/protocol/handoff recovery was completed once. Do not repeat audits for already-completed work.
- Publish each logical task as one complete Git tree and one atomic main commit; update main only with a non-forced expected-SHA compare-and-swap. Never force-push or modify the backup.

## Active task

- Validate the legacy ipkg HTTPS-only transport contract on the latest main SHA. The first exact-head Process run exposed one missing explicit path-filter entry for opt-ipkg-upd.sh; the AI handoff run exposed this required section heading. The corrective source/checker/workflow/handoff commit adds both and ensures unknown direct-install URL schemes fail closed.
- Fetch the new live main and inspect exact-SHA Process and AI handoff results. PENDING remains non-blocking for independent work.

## Completed source slice

- Entware generic.sh root bootstrap now downloads through verified HTTPS into private staging, patches the upstream HTTP origin and feed config to HTTPS, uses bounded timeouts and set -e, validates patched shell before execution and verifies final opkg/config postconditions.
- The legacy Oleg feed still points to http://ipkg.nslu2-linux.org/feeds/optware/oleg/cross/stable; no verified HTTPS mirror was established. The hardened ipkg shell client refuses HTTP/FTP/unknown schemes for both index/file downloads and direct package installs, allows HTTPS and local files, and returns failure for unsupported input instead of reporting false success.
- Legacy package bootstrap validates every configured source before privileged package operations and refuses the shipped HTTP-only feed rather than guessing an HTTPS endpoint. It uses the firmware shell package client for index updates, not the unaudited legacy binary. Optware startup releases its lifecycle lock and returns failure if package bootstrap/update fails.
- Added POSIX shell regressions for secure URL schemes, HTTPS/local-file config, HTTP/FTP/malformed/empty config and symlink-config refusal; wired Process CI and ROMFS installation.

## Verification boundary

- Entware source commit: 967737f91d830c373e8400afe3ff16d0a7d5e080; checker correction 5b8939f35e2d318f1d5fb964dc393366eb7e6e23; OpenSSL command-loop correction 48d442a96a3f2455703e3e73042f7c882505110c.
- Exact-head AI handoff #45771459535 PASS and Process regression #45771459520 PASS on parent SHA 48d442a96a3f2455703e3e73042f7c882505110c. The earlier Process run #45771308852 failed because an earlier static checker expected a literal OpenSSL command; that checker defect was superseded by the passing exact-head run.
- Host-only Entware shell fixture: five PASS groups; static source checker: PASS. No local XY-C1 firmware build, target runtime or physical-router L4 is claimed. Exact-source XY-C1 Build/TRX/artifact validation for the Entware changes must be checked on its source SHA.
- The current legacy ipkg source/test/workflow slice has not yet been checked by Actions. The exact-head Process regression and AI handoff must be fetched after publication. No legacy package-feed download or target runtime is claimed.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Process and AI handoff Actions for legacy ipkg URL/config transport; diagnose any failure from its exact job log.
2. Check XY-C1 Build, TRX verification and artifact upload on the Entware source SHA 967737f91d830c373e8400afe3ff16d0a7d5e080.
3. Continue a concrete stability/security source fix in another Padavan userland service; do not repeat completed startup recovery or Entware/ipkg transport audits.

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For every logical slice, create all blobs, one complete tree, one atomic main commit, and publish with a non-forced expected-SHA compare-and-swap. Before context exhaustion, finish the current safe slice and checkpoint its first concrete next action. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect live refs and exact-SHA Actions before ending. If the current handoff remains accurate, leave it untouched; do not create a no-op commit. Record material PASS/FAIL/PENDING changes and the next concrete task. A commit cannot contain its own resulting SHA; the next GPT must fetch live main before checking its Actions.
