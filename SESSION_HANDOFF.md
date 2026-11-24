## Current repository state

- Live branch `main` parent for this follow-up: `18d90355fb56b33100c05439df30ecc3e61ef85f`. Clause 22 width guards were added in `18d90355fb56b33100c05439df30ecc3e61ef85f`; exact-head Switch CLI #45439391302 and AI handoff #45439390998 PASS. XY-C1 #45439391086 and NVRAM #45439391348 were still running at last poll.
- Clause 45 write-payload guard: `7ae410c1ff14d974e659b8c29bbcd83d18269f0c`; checker input fix `52e34a5`; test-count + handoff fix `e83ac62`.
- Clause 45 address-policy include fix: `a650ed8c8fdd8b0100bc26bb91458cdf4bde9277`. RC firewall boot-result source fix: `7ca46fc7049f3380755f6b2fe5073d86727c6382`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. Legacy PR #1 remains closed/unmerged. Use one atomic main commit per logical task with non-forced expected-SHA updates.

## Active task

### Preserve the special indirect Clause 22 register/data ranges

- The direct C22 width guard commit `18d9035` initially applied five-bit register and 16-bit write-data limits before deciding whether an architecture-specific register-31 access uses the indirect page interface.
- Existing `mii_mgr_indirect_policy.h` tests deliberately exercise logical register `0x1234` and a 32-bit value `0x89abcdef`: indirect access selects a page, reads/writes a low-word register and a high-word register, and supports a 16-bit logical register address/full u32 value. Applying direct C22 caps universally rejects valid TRGMII/forced-GE1 indirect access.
- The follow-up keeps PHY address validation common and selects policy by path. Direct C22 requires register <=31 and data <=0xffff; indirect register-31 mode permits a 16-bit logical register <=0xffff and full u32 data (split into two 16-bit hardware writes). The indirect max also blocks aliasing above 0xffff.
- Added policy boundary tests for logical register 0, 31, 0xffff, 0x10000 and UINT32_MAX; static workflow contract requires indirect guards before indirect calls and direct-only bounds before direct MMIO APIs.

## Verification boundary

- Exact Clause 22 source SHA `18d90355fb56b33100c05439df30ecc3e61ef85f`: Switch CLI #45439391302 PASS (GNU89 address/register/data tests and source contracts); AI docs #45439390998 PASS. XY-C1 #45439391086 and NVRAM #45439391348 were still in progress at last poll.
- Clause 45 payload guard `7ae410c`: policy binary passed; follow-up static checker count assertion was corrected in `e83ac62`. Exact XY-C1 #45438348129 and NVRAM #45438348160 status still needs final closure.
- Clause 45 address include fix `a650ed8`: NVRAM #45437414867 PASS; XY-C1 #45437415030 requires final Build/TRX/artifact step inspection.
- RC firewall source fix `7ca46fc` is L3 VERIFIED: XY-C1 #45434352284 Build, TRX and artifact all PASS on that exact SHA. Physical-device L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head Switch CLI and AI handoff runs for this indirect-range correction.
2. Poll XY-C1 #45439391086 and NVRAM #45439391348 for exact C22 source SHA `18d9035`, checking Build/TRX/artifact individually.
3. Poll Clause45 source XY-C1 #45438348129 and the include-fix source XY-C1 #45437415030. Keep their SHA results separate.
4. Once the C22 direct/indirect range split is verified, return to the broad RC startup-result propagation P1 queue or another high-risk userland/kernel stability defect. Pending CI does not block independent implementation.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. At each session end inspect live main, frozen backup, source SHA, exact-head Actions, L2/L3/L4 evidence and first next action. If accurate, do not create a no-op handoff commit.

# Cross-session execution mandate

Recover once, implement, test and hand off. Use one atomic main commit per logical task with non-forced expected-SHA updates. Never force-push or overstate firmware or hardware validation.
