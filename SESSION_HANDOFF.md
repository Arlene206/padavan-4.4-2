## Current repository state

- Live parent main before the RAETH MDIO transaction change: `4985f33bda8360f4dfb92581b7b7d47d7850a03c`.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains unchanged at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- FRP procfs read-error and newline-spoofing fixes: `0104186915ee6132d99871bede241ecf1835c279` and `5ca9761969591766c583d3146cf5f32155ab7891`; exact FRP lifecycle #45361194679, AI handoff #45361194624, Process inventory #45361194576 PASS.
- VSFTPD SITE CHMOD/UMASK strict octal validation: source `ea7d7e28be9988175fbcc423f9378e5d765de764`, checker correction `0c23ad7c82fc2e033f73d4a94f6fb02a77324ca5`; exact VSFTPD stability #45359061300 PASS.
- AliDNS POSIX byte encoder source `ca9267b3c71a9fb2a8de96e82a2878fcf31e5dc6`; exact AliDNS #45357686604 PASS. XY-C1 product-source baseline `ea7d7e2` passed exact Build/TRX/artifact #45358712652, Process #45358712716 and NVRAM #45358712663. That firmware claim applies to the earlier production source tree, not to the current unbuilt kernel edit. No physical L4 validation is claimed.
- Samba share ACL one-read snapshot: `d8f6092c6af34f1c8ec45c746ce9411e40522577`. The HTTPD legacy source contract was updated in `3026ffe460fe9d01ad5b72312978ff957b400479`; exact reruns for the latest follow-up commits are pending. The Samba workflow itself passed on d8.
- QMI client-ID workflow heredoc syntax was corrected in `4985f33bda8360f4dfb92581b7b7d47d7850a03c`; no modem runtime source changed.
- Main remains the active branch; frozen backup remains unchanged.

## Active task — Atomic RAETH indirect MMD page/data transactions

- Forced 1000/2000/2600 operation on PHY 31 performs page selection (Clause 22 register 0x1f), followed by low-word and high-word reads/writes. Before this change each sub-operation independently acquired/released `mdio_lock`; concurrent Clause 22/45 callers could interleave after page selection and silently redirect the remaining accesses to a different MMD register.
- This change factors the hardware operations into caller-locked primitives plus regular single-command wrappers. Extended public read/write hold the same `mdio_lock` across the whole three-command sequence and AN-polling disable/restore. Existing Clause-45 address/data operations remain serialized under that lock.
- Added GNU89 mock-bus tests for operation order, 32-bit word assembly/splitting, each intermediate failure and no further writes/reads after failure. An executable source checker ensures the transaction-wide lock precedes all three phases and direct/Clause-45 access still use the common lock. The switch stability workflow includes the test, checker, and both push/PR filters.
- Local policy regression: PASS under GNU89 + AddressSanitizer + UBSan. Exact-head workflow status is pending. No MIPS firmware or hardware validation for this new source change is claimed.

## Verification boundary

- Exact `5ca9761969591766c583d3146cf5f32155ab7891`: FRP lifecycle #45361194679 PASS; AI handoff #45361194624 PASS; Process inventory #45361194576 PASS.
- Exact `ea7d7e28be9988175fbcc423f9378e5d765de764`: XY-C1 Build/TRX/artifact #45358712652 PASS; Process #45358712716 PASS; NVRAM #45358712663 PASS.
- Exact `0c23ad7c82fc2e033f73d4a94f6fb02a77324ca5`: VSFTPD stability #45359061300 PASS.
- Samba/QMI checker corrections `3026ffe460fe9d01ad5b72312978ff957b400479` and `4985f33bda8360f4dfb92581b7b7d47d7850a03c`: exact-head CI status to be inspected.
- Current MMD transaction code: local standalone policy test PASS; exact-head switch workflow pending.
- No firmware build for the current RAETH source SHA and no L4 router test are claimed.

## Exact first action for the next GPT

1. Inspect exact-push Actions for this MMD commit, especially Switch CLI stability. If its regression or source checker fails, inspect the first failed assertion and correct it without weakening transaction-wide locking.
2. Inspect HTTPD and USB modem exact-head reruns for commits `3026ffe` and `4985f33`; fix any remaining checker regression.
3. Confirm whether the kernel-source change triggered the XY-C1 firmware pipeline; only report Build/TRX/artifact if all pass on the same exact SHA.
4. After this slice passes, continue the next uncompleted stability/security item from the roadmap. Keep the frozen backup immutable; only advance main through full-tree atomic commits with non-forced expected-SHA updates.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change must land as one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task.
