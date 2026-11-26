# Session Handoff — 2026-11-26

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Parent main before the current Samba configuration durability slice: 93ec42b2b3927ab0093c26b543e820cf86f51f40.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains at 72f4f157a084f8184b9597ca5bf334b8080e5807; it is read-only and must not be modified. Legacy PR #1 remains closed/unmerged.
- Startup/bootstrap/protocol/handoff recovery was completed once. Do not repeat audits for already-completed work.
- Publish each logical task as one complete Git tree and one atomic main commit; update main only with a non-forced expected-SHA compare-and-swap. Never force-push or modify the backup.

## Active task

- The Samba config publisher previously checked stdio flush/close and atomic rename, but did not fsync the staged file or its parent directory. A power interruption could therefore leave a published directory entry without durable new configuration contents, despite the success status.
- The current source slice adds staged-file fsync before close and a no-follow parent-directory fsync after rename. The helper rejects empty paths and does not follow a symlink as the final parent component. The GNU89 Samba config-file regression now checks successful parent sync, invalid path rejection and symlink-parent refusal.
- Exact-head Samba share-stability Actions for this source change must be checked after publication; no router hardware or power-cut test is claimed.

## Completed source slice

- Entware generic.sh root bootstrap now downloads through verified HTTPS into private staging, patches the upstream HTTP origin and feed config to HTTPS, uses bounded timeouts and set -e, validates patched shell before execution and verifies final opkg/config postconditions.
- The legacy Oleg feed still points to http://ipkg.nslu2-linux.org/feeds/optware/oleg/cross/stable; no verified HTTPS mirror was established. The hardened ipkg shell client refuses HTTP/FTP/unknown schemes for both index/file downloads and direct package installs, allows HTTPS and local files, and returns failure for unsupported input instead of reporting false success.
- Legacy package bootstrap validates every configured source before privileged package operations and refuses the shipped HTTP-only feed rather than guessing an HTTPS endpoint. It uses the firmware shell package client for index updates, not the unaudited legacy binary. Optware startup releases its lifecycle lock and returns failure if package bootstrap/update fails.
- Samba legacy config file publishing now syncs the staged file before rename and syncs the containing directory after rename so a successful publish means both contents and directory entry were handed to the filesystem for durability.

## Verification boundary

- Entware source commit: 967737f91d830c373e8400afe3ff16d0a7d5e080; checker correction 5b8939f35e2d318f1d5fb964dc393366eb7e6e23; OpenSSL command-loop correction 48d442a96a3f2455703e3e73042f7c882505110c.
- Legacy ipkg complete source/test correction: b63ce11dea3ee18b00a32ad79a1786c6eb482160; follow-up direct-URI/path-filter/handoff correction: 93ec42b2b3927ab0093c26b543e820cf86f51f40.
- Exact-head Process regression #45772642959 PASS, Process inventory #45772642896 PASS and AI handoff #45772642788 PASS on source SHA 93ec42b2b3927ab0093c26b543e820cf86f51f40. The Process run executed the legacy ipkg transport shell fixture and static source contract. XY-C1 Build run #45772642912 remained IN_PROGRESS at the last poll; do not claim its Build/TRX/artifact validation until it completes.
- Host-only Entware shell fixture passed five groups; host-only ipkg transport fixture passed four groups. No local XY-C1 firmware build, target runtime, Samba hardware test or physical-router L4 is claimed.
- The current Samba durability helper/test changes are pending their exact-head Samba share-stability run after publication.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head Samba share-stability (GNU89 config-file regression plus all Samba contracts), AI handoff and XY-C1 Build/TRX/artifact Actions.
2. Diagnose any failure from its exact job log. Do not weaken fsync/error propagation or symlink-parent rejection to make the check pass.
3. Continue a concrete stability/security fix in another Padavan userland subsystem; do not repeat the already-completed Entware/ipkg source audit.

# Cross-session execution mandate

Continue one autonomous engineering stream on main. For every logical slice, create all blobs, one complete tree, one atomic main commit, and publish with a non-forced expected-SHA compare-and-swap. Before context exhaustion, finish the current safe slice and checkpoint its first concrete next action. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session must inspect live refs and exact-SHA Actions before ending. If the current handoff remains accurate, leave it untouched; do not create a no-op commit. Record material PASS/FAIL/PENDING changes and the next concrete task. A commit cannot contain its own resulting SHA; the next GPT must fetch live main before checking its Actions.
