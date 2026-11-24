## Current repository state

- Active branch: `main`; single- and range-delete secure-mode authorization correction parent: `13ae1bcdb18aa24656c65c8962d14c589f41a496`. Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. No open PRs.
- MTK AP private-ioctl, HTTPD selector, WAN source-PHY range and QMI WDS/PDH teardown fixes retain recorded exact-SHA L3 Build/TRX/artifact evidence. Physical-router L4 remains unverified.
- DHCPDECLINE retry/send-result chain: `4ca919b` / `02ddda7` / `4015869` / `aae46a6`. Exact-head udhcpc GNU89 ASan/UBSan regression/source contract #45400755028 PASS after the renamed test runner was fixed. AI handoff #45400755063 and Process lifecycle #45400755089 PASS; USB storage #45400755158 and NVRAM #45400755163 were pending at last poll. XY-C1 for production `4015869` #45400399157 was still building; TRX/artifact pending. Latest `aae46a6` is test/docs-only.
 
## Active task — Fail closed on MiniUPnP secure-mode delete ownership checks

- Range-delete source in `852eb72` preflights every mapping before the first deletion and returns 606 for a foreign owner or 501 when lookup/address parsing fails.
- A second fail-open path was found in single-entry `DeletePortMapping()`: it allowed deletion when `upnp_get_redirection_infos()` failed or returned an unparsable owner address. This follow-up now returns 501 on either identity-resolution failure and uses the shared IPv4 peer comparator for the 606 ownership decision.
- The integrated workflow expects the expanded SOAP policy test banner; it is restored. The dedicated source checker now verifies both single-entry and range secure-mode gates before their respective delete calls.
- At base `13ae1bc`, AI docs #45405754230 PASS; MiniUPnP #45405754223 passed its numeric policy, range checker, XML/pinhole/lease tests but failed the final integrated source-contract step because the banner was temporarily reverted. Exact XY-C1 #45405466882 was still building; Process #45405467000 and NVRAM #45405466901 PASS.

## Previous task — XUPnP generated Lua configuration pair

- Product-source commit eb0fbb1 stages and fsyncs both Lua files before pair publication through dnsmasq_file_publish_pair, with backup restoration after a second-publish failure.
- DNSMasq lifecycle #45402050207 on 2fd4c74 PASS. Process lifecycle #45402861054 on 364383b PASS after the whitespace-normalized checker correction; follow-on Process inventory #45403376647 on c2dc6d3 PASS. NVRAM #45401700436 on eb0fbb1 PASS.
- XY-C1 #45401700209 on exact product source eb0fbb1: Build XY-C1 PASS, TRX output verification PASS, artifact upload PASS. L3 is complete for this XUPnP ## Verification boundary

- Range-delete backend-failure handling and secure-mode preflight on `852eb72`: the GNU89 ASan/UBSan policy test and dedicated range source contract passed; integrated MiniUPnP workflow #45405466908 later stopped on a checker quoting error, fixed in `13ae1bc`.
- Exact MiniUPnP #45405754223 on `13ae1bc`: numeric policy, dedicated range checker, XML escaping, pinhole, lease and related regressions PASS; final integrated source contract failed because the test banner had been reverted to the older text. The banner is restored in this follow-up; new exact-head run required.
- AI handoff #45405754230, Process inventory #45405467000 and NVRAM #45405466901 PASS. XY-C1 #45405466882 on `852eb72` remained in Build with TRX/artifact pending at last poll. No L3 claim for the secure-mode slice yet.
- XUPnP pair implementation SHA `eb0fbb1` retains separate exact Build/TRX/artifact PASS (#45401700209); physical-router L4 remains unverified.
- Frozen backup remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Exact first action for the next GPT

1. Fetch live main; verify both single and range secure-mode owner checks, integrated MiniUPnP regressions and AI docs on the exact source SHA.
2. Check XY-C1 Build/TRX/artifact on the current source SHA and record only complete success.
3. Continue an independent userland stability/security defect while pending jobs run.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At session end, inspect live main, relevant exact-head Actions, validation boundary, blockers and the first next action. If all checkpoint facts remain accurate, make no file change and no commit.

# Cross-session execution mandate

Recover once and continue on main. Never create a development branch or PR; the backup ref is immutable.

For every multi-file logical slice, create all blobs, one complete tree based on latest main, one commit with the exact parent, then update main with a non-forced expected_sha compare-and-swap. This is one atomic main commit. Never create partial commits, force-push or rewrite history.

Continue implementing safe fixes rather than stopping at audits. PASS advances; FAIL is diagnosed and corrected; PENDING does not block independent work. Preserve Linux 4.4, GNU89, MIPS/uClibc and router compatibility.

Before context exhaustion, finish the current safe slice, record the latest SHA/tests/CI/unresolved issue and one first action, append material milestones to dev_log.md, and update roadmap only for a material module/status/priority change.