## Current repository state

- Active branch: `main`; current VSFTPD ACL follow-up is based on `6725aafafbac3a792591a46c73a9cf94c9c3aa56`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPD selector, WAN source-PHY range and QMI WDS/PDH teardown fixes retain recorded exact-SHA L3 Build/TRX/artifact evidence. Physical-router L4 remains unverified.
- DHCPDECLINE retry/send-result chain: `4ca919b` / `02ddda7` / `4015869` / `aae46a6`. Exact-head udhcpc GNU89 ASan/UBSan regression/source contract #45400755028 PASS after the renamed test runner was fixed. AI handoff #45400755063 and Process lifecycle #45400755089 PASS; USB storage #45400755158 and NVRAM #45400755163 were pending at last poll. XY-C1 for production `4015869` #45400399157 was still building; TRX/artifact pending. Latest `aae46a6` is test/docs-only.
 
## Active task — Complete MiniUPnP secure-mode delete verification

- `DeletePortMapping()` now fails closed with SOAP 501 when secure-mode owner lookup or IPv4 parsing fails and 606 for a foreign owner. `DeletePortMappingRange()` performs the equivalent all-entry preflight before the first delete, preventing partial unauthorized range mutation.
- Exact workflow #45406283079 on `5f0360d`: GNU89 ASan/UBSan SOAP policy executable PASS; dedicated source checker then failed only because its final test-banner assertion still expected the previous text. The integrated workflow and host binary require the expanded banner `MiniUPnP SOAP unsigned input and range-delete result policy: PASS`; the checker is corrected in this follow-up.
- AI docs #45406283195 PASS and Process inventory #45406283064 PASS on `5f0360d`. NVRAM #45406283086 and XY-C1 #45406283012 were still running at last poll.

## Active task — Reject malformed VSFTPD ACL records for the requested share

- The exact-record parser introduced at `6725aaf` closed newline-boundary matching, but a second edge case remained: it checked expected line length before recognizing the requested `*folder=` key. An overlong malformed record (for example `*secret=000EXTRA`) could be skipped and a later valid duplicate record could grant permission.
- The parser now recognizes the exact key first, then rejects malformed length, invalid digits or any duplicate key. Regression cases cover malformed-only, malformed-before-valid and valid-before-malformed orderings.
- The VSFTPD workflow source contract now requires key-first/strict-length checks. Exact VSFTPD + AI + firmware validation for the correction SHA is pending.

## Previous task — MiniUPnP secure-mode delete authorization

- `DeletePortMapping()` fails closed with SOAP 501 when secure-mode owner lookup or IPv4 parsing fails, and returns 606 for a foreign owner.
- `DeletePortMappingRange()` checks every mapping owner before the first deletion, returning 501 on lookup/address failure and 606 on foreign ownership. This prevents partial range mutation before a later authorization denial.
- Exact MiniUPnP workflow #45406562259 and AI handoff #45406562291 PASS on `eea1254`. The MiniUPnP source/tests have since been carried forward; the current ACL correction does not modify them.

## Previous task — XUPnP generated Lua configuration pair

- Product-source commit eb0fbb1 stages and fsyncs both Lua files before pair publication through dnsmasq_file_publish_pair, with backup restoration after a second-publish failure.
- DNSMasq lifecycle #45402050207 on 2fd4c74 PASS. Process lifecycle #45402861054 on 364383b PASS after the whitespace-normalized checker correction; follow-on Process inventory #45403376647 on c2dc6d3 PASS. NVRAM #45401700436 on eb0fbb1 PASS.
- XY-C1 #45401700209 on exact product source eb0fbb1: Build XY-C1 PASS, TRX output verification PASS, artifact upload PASS. L3 is complete for this XUPnP ## Verification boundary

- VSFTPD ACL base `6725aaf`: exact VSFTPD workflow #45406961857 PASS; AI handoff #45406961491 PASS; Process inventory #45406961396 PASS; NVRAM #45406961526 PASS. XY-C1 #45406961610 was still running at last poll. The malformed-record follow-up needs new exact-head VSFTPD/AI checks.
- MiniUPnP secure-mode single/range owner checks: exact MiniUPnP #45406562259 and AI docs #45406562291 PASS on `eea1254`. No current-SHA firmware L3 claim.
- XUPnP pair SHA `eb0fbb1`: exact Build/TRX/artifact PASS (#45401700209); DNSMasq #45402050207 and NVRAM #45401700436 PASS. Hardware L4 remains unverified.
- Frozen backup remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Check exact-head VSFTPD ACL regression and source contract for malformed matching records and duplicate order.
2. Check AI handoff, Process/NVRAM and XY-C1 Build/TRX/artifact on the exact commit SHA; no L3 claim unless Build, TRX and artifact upload all pass on that SHA.
3. Continue the next independent userland security/stability defect.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end, inspect live main, relevant exact-head Actions, validation boundary, blockers and the first next action. If all checkpoint facts remain accurate, make no file change and no commit.

# Cross-session execution mandate

Recover once and continue on main. Never create a development branch or PR; the backup ref is immutable.

For every multi-file logical slice, create all blobs, one complete tree based on latest main, one commit with the exact parent, then update main with a non-forced expected_sha compare-and-swap. This is one atomic main commit. Never create partial commits, force-push or rewrite history.

Continue implementing safe fixes rather than stopping at audits. PASS advances; FAIL is diagnosed and corrected; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc and router compatibility.

Before context exhaustion, finish the current safe slice, record the latest SHA/tests/CI/unresolved issue and one first action, append material milestones to dev_log.md, and update roadmap only for a material module/status/priority change.