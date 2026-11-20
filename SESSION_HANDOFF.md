## Current repository state

- Parent `main` for this redirect-metadata slice: `22a47cebeb8c09a51e6eb40919a4f0bca05d7875`. Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and read-only.
- IPTC ownership ordering correction `46d79504f70236b403ac3153ca155715d9c65bb0`: MiniUPnP #44794517972 PASS; AI handoff #44794518071 PASS; process inventory #44794518177 PASS.
- NAT/filter delete compensation `b7a803dcc95ae88566ffe01ff8fd8ea4c5cefd33`: exact-head MiniUPnP #44793663690 PASS.
- NAT/filter update rollback `bfc1c1663f9d3dc6cdb4349ba1b3601f175e27ad` and checker correction `ea745da6e423a778a65c0aba6ce85424e6a642a3`: MiniUPnP #44792212170 PASS; AI #44792212195 PASS; exact-source XY-C1 #44791960769 on bfc1c16 PASS (Build, TRX, artifact upload).
- Peer SNAT/DSCP compensation and warning cleanup are in main through `22a47ceb`; exact-head Actions for that chain were queued/in progress when this slice began.
- This slice fixes OOM loss of in-memory redirect descriptions/lease timestamps. Its local GNU89 ASan/UBSan policy test passed; exact-head CI follows this commit.
- MIPS/uClibc runtime and physical-router L4 remain NOT VERIFIED.

## Active task

Make MiniUPnP redirect description and lease-expiry metadata publication transactional under memory pressure:

- Allocate metadata before new redirect or peer NAT installation; OOM returns before iptables mutation.
- For metadata-only updates, allocate before unlinking the old record so OOM preserves the prior description and expiry.
- For changed internal ports, stage metadata before the NAT commit and publish only after the matching filter update commits. Every failure/rollback path frees the staged record.
- The host test covers allocation success, injected ENOMEM, invalid allocator and size overflow; the source contract verifies preparation/publication ordering.

After exact-head MiniUPnP and XY-C1 validation, continue MiniUPnP teardown and other userland stability auditing. Do not repeat completed IPTC ownership or NAT/filter rollback work.

## Verification boundary

- Peer IPTC handle-order correction #44794517972 on `46d79504`: MiniUPnP PASS; AI handoff #44794518071 PASS; process inventory #44794518177 PASS.
- NAT/filter deletion compensation #44793663690 and update rollback #44792212170 on their exact source/checker SHAs: PASS.
- Redirect metadata policy binary passed locally with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan. Exact-head MiniUPnP, AI handoff and XY-C1 Build/TRX/artifact are required after this source commit.
- Physical hardware L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`, confirm frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807`, and inspect exact-head MiniUPnP, AI handoff and XY-C1 workflows for the redirect-metadata commit.
2. Require the GNU89 sanitizer allocation regression and source-order checker to pass; require XY-C1 Build, TRX verification and artifact upload before accepting L3.
3. Continue auditing MiniUPnP redirect teardown and whole-userland stability. Keep physical-router L4 unverified until target hardware testing.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
