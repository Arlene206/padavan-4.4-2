## Current repository state

- Active branch: `main`; MiniUPnP SOAP range-delete source fix and regressions parent: `364383be7889a9339887c1b4f04ce9ff9a273a10`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPD selector, WAN source-PHY range and QMI WDS/PDH teardown fixes retain recorded exact-SHA L3 Build/TRX/artifact evidence. Physical-router L4 remains unverified.
- DHCPDECLINE retry/send-result chain: `4ca919b` / `02ddda7` / `4015869` / `aae46a6`. Exact-head udhcpc GNU89 ASan/UBSan regression/source contract #45400755028 PASS after the renamed test runner was fixed. AI handoff #45400755063 and Process lifecycle #45400755089 PASS; USB storage #45400755158 and NVRAM #45400755163 were pending at last poll. XY-C1 for production `4015869` #45400399157 was still building; TRX/artifact pending. Latest `aae46a6` is test/docs-only.
 
## Active task — MiniUPnP DeletePortMappingRange false-success handling

- In the current source fix, `DeletePortMappingRange()` records every `upnp_delete_redirection()` result and returns SOAP 501 if any deletion failed. It no longer sends a success response merely because the loop finished. Empty range still maps to 730.
- Exact GNU89 ASan/UBSan and source-order checker are wired into `miniupnpd-config-stability.yml`. Check both on the exact commit SHA.
 
## Previous task — XUPnP generated Lua configuration pair

- `eb0fbb1` staged and fsynced both Lua files before pair publication through `dnsmasq_file_publish_pair()`, with backup restoration after a second-publish failure.
- DNSMasq workflow #45402050207 on validator commit `2fd4c74` PASS. Process lifecycle #45402388244 on `25db` failed at a checker string matching the multiline stage call; fixed in the follow-up commit before the MiniUPnP task. Recheck exact Process run on this current main SHA.
- NVRAM #45401700436 on `eb0fbb1` PASS. XY-C1 #45401700209 on product source `eb0fbb1` had build in progress at last poll; TRX and artifact were pending. No L3 claim for that slice until its exact-source Build/TRX/artifact all PASS.

## Verification boundary

- Parent `aae46a6391f1d50c3122ac4e7f4098175023d832`: udhcpc event-script policy/source regression, AI handoff and Process lifecycle PASS; USB storage and NVRAM audit pending at last poll.
- On `eb0fbb1`, DNSMasq lifecycle #45401700240 failed only its XUPnP source checker after the GNU89/ASan/UBSan C regression passed; Process lifecycle #45401700271 passed the XUPnPD storage/file-safety and earlier C tests, then failed at stale sequential-publication assumptions. DNSMasq source-offset comparison was corrected in `2fd4c74`; Process checker was updated in `25db`, but #45402388244 then exposed a second checker issue: the new staged calls are split across lines, while the checker searched raw text. This follow-up normalizes whitespace for those assertions. AI docs #45401700548, WAN DHCP #45401700416, WAN route #45401700385, USB #45401700357, HTTPD #45401700213, Process inventory #45401700188 and Firewall #45401700158 PASS. NVRAM #45401700436 and XY-C1 #45401700209 were still in progress at last poll. DNSMasq #45402050207 on `2fd4c74` PASS; Process #45402388244 on `25db` failed the checker-only raw multiline assertion (fixed in this follow-up). AI docs #45402388165 on `25db` PASS. NVRAM #45401700436 PASS. XY-C1 #45401700209 remained in progress on product source `eb0fbb1`; Build/TRX/artifact must all finish PASS before L3.

## Exact first action for the next GPT

1. Fetch live `main` and verify exact-head Process lifecycle checker and GNU89/ASan/UBSan tests after the multiline assertion correction.
2. Check same-SHA XY-C1 Build/TRX/artifact for product SHA `eb0fbb1`; record true status only.
3. Check exact-head MiniUPnP SOAP range-delete regression/checker and Process lifecycle. Continue independent userland stability/security defects while XY-C1 runs remain pending.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end, inspect live main, relevant exact-head Actions, validation boundary, blockers and the first next action. If all checkpoint facts remain accurate, make no file change and no commit.

# Cross-session execution mandate

Recover once and continue on main. Never create a development branch or PR; the backup ref is immutable.

For every multi-file logical slice, create all blobs, one complete tree based on latest main, one commit with the exact parent, then update main with a non-forced expected_sha compare-and-swap. This is one atomic main commit. Never create partial commits, force-push or rewrite history.

Continue implementing safe fixes rather than stopping at audits. PASS advances; FAIL is diagnosed and corrected; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc and router compatibility.

Before context exhaustion, finish the current safe slice, record the latest SHA/tests/CI/unresolved issue and one first action, append material milestones to dev_log.md, and update roadmap only for a material module/status/priority change.