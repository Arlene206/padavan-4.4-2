# Cross-session execution mandate

## Current repository state

### 2026-11-23 continuation — current state

- **Source commit:** `81c514f33641d47aa49062443e8fb23e80807765` (`fix(mtk-esw): propagate IGMP snooping MDIO failures`); this handoff repair is a documentation-only child. Fetch live `main` at takeover for the current HEAD.
- **Backup branch:** `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify or merge it.
- **Completed MTK ESW work:** checked port matrix/PVID/ingress/attribute/accept MDIO APIs and caller propagation are on `3ab6e43a2ca7db2ab7521962566c34a4b8b70b1e`; prep commit `a0ff74d73e6d8b9ecb45ba9937c267bf3f4d4916` added checked-write tests. Do not repeat this work.
- **Current source slice:** MTK ESW IGMP flood-to-CPU now uses masked checked MDIO updates; per-port learning and MLD/PIC writes use checked write APIs and aggregate the first error while attempting independent ports. Errors propagate through bridge reconfiguration, init-post and both IGMP ioctls. Runtime selector caches roll back on failed hardware programming.
- **Exact-SHA Actions on source SHA `81c514f33641d47aa49062443e8fb23e80807765`:** Switch CLI #45257584087 PASS; AI handoff docs #45257584133 FAIL because this file lacked the required `## Current repository state` heading (repaired in this documentation child); NVRAM #45257583956 and XY-C1 Firmware Validation #45257583931 were in progress at the last poll. Do not infer Build/TRX/artifact success until that identical-SHA run completes.
- **Tests:** host tests compile with GNU89/Werror/ASan/UBSan in Switch CLI workflow. No local kernel/MIPS compile or physical-router test is claimed. Verify new exact-SHA Actions before any L3 claim.
- **Next implementation task:** inspect remaining MTK ESW link-status and MIB telemetry read paths for error-collapsing `esw_reg_get()`/MDIO reads; add status-bearing interfaces where callers can preserve existing UAPI semantics, with failure-injection coverage.
- **Working rule:** source change → targeted regression → complete Git tree and single atomic commit → expected-SHA non-forced update of `main` → exact-SHA Actions. Pending checks do not block other independent work.

## Active task

### MTK ESW IGMP/MLD checked MDIO propagation

- The prior port/PVID/ingress/attribute/accept/matrix slice is committed in `3ab6e43`; applied-state PVID cache and multiport accept follow-up is committed in `196dcccc`.
- Source SHA `81c514f33641d47aa49062443e8fb23e80807765` converts IGMP flood-to-CPU to a checked masked update; IGMP port-learning and MLD/PIC writes to checked writes with first-error aggregation; and propagates failures through bridge, init-post and both IGMP ioctls. Runtime selector caches roll back on failed programming.
- Exact SHA `81c514f`: Switch CLI #45257584087 PASS; AI docs #45257584133 failed because the handoff heading was missing (fixed by this documentation child); NVRAM #45257583956 and XY-C1 #45257583931 were in progress at last poll. No local MIPS/kernel compile or hardware L4 is claimed.
- **Next task:** inspect status-bearing handling for remaining link/MIB telemetry reads, then add only UAPI-compatible error propagation and regression coverage.

### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Source SHA `81c514f33641d47aa49062443e8fb23e80807765`: Switch CLI #45257584087 PASS; AI docs #45257584133 failed on a missing handoff heading (added in this docs child); NVRAM #45257583956 and XY-C1 #45257583931 were in progress at last poll. Build, TRX and artifact upload are NOT claimed until confirmed on that identical SHA.
- Earlier production SHA `59622f3c9612949ddd642fc738e02463f5400def`: XY-C1 Build/TRX/artifact #45252058834 PASS and NVRAM #45252058851 PASS; those historical results do not validate later source changes.
- Frozen backup remains untouched. Physical router hardware L4 and local MIPS compilation are NOT VERIFIED.
- LLDT config parser: dedicated L2 workflow #45238664960 PASS; Process umbrella #45239229205 PASS. XY-C1 #45237411963 PASS on exact source SHA `eef4417` (Build/TRX/artifact). AI docs #45239229201 needs one workflow path-filter correction (this commit).
- Shadowsocks DNS proxy preflight and upstream whitespace fixes: #45234515153 PASS, Process #45234515707 PASS and AI docs #45234515042 PASS on `f3acf67`; XY-C1 #45234516216 was pending.
- RStats `601ad73`: RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS; AI doc repair #45236207680 PASS on `3760b93`. XY-C1 #45234873441 still requires final same-SHA step confirmation before an L3 claim.
- Shadowsocks missing-executable test `5a43b18`: lifecycle #45235046114 and Process #45235046275 PASS; AI documentation repaired by #45236207680 on `3760b93`.
- Exact LLDT #45238664960 on `38d3732` passed GNU89/Werror/ASan/UBSan compile/runtime and its Python production/optional-tool checker; AI docs #45238664839 PASS. Process #45239229205 PASS on `79f4d53`. XY-C1 #45237411963 on the production source SHA `eef4417` passed Build, TRX and artifact upload. AI docs #45239229201 exposed only the path-filter asymmetry corrected in this commit. No local compile is claimed.
- Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check NVRAM and XY-C1 runs for source SHA `81c514f`; claim L3 only if Build XY-C1, TRX verification and artifact upload all PASS on that exact source SHA.
2. Verify AI handoff documentation after this heading repair; diagnose any remaining checker failure from its log.
3. Inspect remaining MTK ESW link/MIB telemetry reads for swallowed MDIO errors and define a narrowly scoped status-safe fix with host regression and exact-source assertions.

## Cross-session operating constraints
Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on `main`; each logical source or CI change is one atomic main commit: create all changed blobs and one complete Git tree before creating that commit, then update `main` using a non-forced expected-SHA compare-and-swap. Do not ask the user what to do next.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop
- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.