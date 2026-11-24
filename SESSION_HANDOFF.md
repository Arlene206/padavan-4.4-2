## Current repository state

- Active branch: `main`; secure-mode range-delete authorization slice is based on `0f7f90fa4d49835566dbaa0ef5a0ec08c0a74bb8`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPD selector, WAN source-PHY range and QMI WDS/PDH teardown fixes retain recorded exact-SHA L3 Build/TRX/artifact evidence. Physical-router L4 remains unverified.
- DHCPDECLINE retry/send-result chain: `4ca919b` / `02ddda7` / `4015869` / `aae46a6`. Exact-head udhcpc GNU89 ASan/UBSan regression/source contract #45400755028 PASS after the renamed test runner was fixed. AI handoff #45400755063 and Process lifecycle #45400755089 PASS; USB storage #45400755158 and NVRAM #45400755163 were pending at last poll. XY-C1 for production `4015869` #45400399157 was still building; TRX/artifact pending. Latest `aae46a6` is test/docs-only.
 
## Active task — Enforce secure-mode ownership before MiniUPnP range deletion

- `DeletePortMappingRange()` now preflights every mapping's internal IPv4 owner when `SECUREMODEMASK` is enabled. It verifies all entries before the first delete, returns SOAP 606 for a foreign owner, and fails closed with 501 if an entry cannot be resolved or parsed. This prevents both unauthorized deletion and partial range deletion before a later ownership denial.
- A host regression covers equal/different IPv4 identities and null inputs. The source checker pins the full preflight/cleanup order before the first mutation.
- At base `0f7f90f`, exact MiniUPnP range-delete workflow #45405070252 PASS and AI handoff #45405070245 PASS. The previous test banner was restored for the integrated source contract. New secure-mode C/source regressions must pass on this source SHA.

## Previous task — XUPnP generated Lua configuration pair

- Product-source commit eb0fbb1 stages and fsyncs both Lua files before pair publication through dnsmasq_file_publish_pair, with backup restoration after a second-publish failure.
- DNSMasq lifecycle #45402050207 on 2fd4c74 PASS. Process lifecycle #45402861054 on 364383b PASS after the whitespace-normalized checker correction; follow-on Process inventory #45403376647 on c2dc6d3 PASS. NVRAM #45401700436 on eb0fbb1 PASS.
- XY-C1 #45401700209 on exact product source eb0fbb1: Build XY-C1 PASS, TRX output verification PASS, artifact upload PASS. L3 is complete for this XUPnP ## Verification boundary

- MiniUPnP false-success fix from `c2dc6d3`: GNU89 ASan/UBSan policy test and dedicated range-delete source-order check PASS on exact MiniUPnP workflow #45405070252 at follow-up baseline `0f7f90f`. AI handoff #45405070245 PASS on the same SHA.
- This commit adds secure-mode owner preflight; exact MiniUPnP workflow and AI docs must pass on its new SHA. No L3 claim yet for this source slice.
- XY-C1 run #45403376426 for product source `c2dc6d3` was still building at last poll (TRX/artifact pending); NVRAM #45403376634 PASS and Process #45403376647 PASS. Do not infer a successful firmware artifact from a pending build.
- XUPnP pair implementation SHA `eb0fbb1` retains its separate exact Build/TRX/artifact PASS via #45401700209, DNSMasq #45402050207 PASS, and NVRAM #45401700436 PASS. Physical-router L4 remains unverified.
- Frozen backup remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Fetch live main and verify exact-head MiniUPnP secure-mode range preflight GNU89 ASan/UBSan test plus source-order contract.
2. Check XY-C1 Build/TRX/artifact on this exact production source SHA; update the firmware evidence boundary only after all three steps PASS.
3. Continue the next independent userland stability/security defect while unrelated CI is pending.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end, inspect live main, relevant exact-head Actions, validation boundary, blockers and the first next action. If all checkpoint facts remain accurate, make no file change and no commit.

# Cross-session execution mandate

Recover once and continue on main. Never create a development branch or PR; the backup ref is immutable.

For every multi-file logical slice, create all blobs, one complete tree based on latest main, one commit with the exact parent, then update main with a non-forced expected_sha compare-and-swap. This is one atomic main commit. Never create partial commits, force-push or rewrite history.

Continue implementing safe fixes rather than stopping at audits. PASS advances; FAIL is diagnosed and corrected; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc and router compatibility.

Before context exhaustion, finish the current safe slice, record the latest SHA/tests/CI/unresolved issue and one first action, append material milestones to dev_log.md, and update roadmap only for a material module/status/priority change.