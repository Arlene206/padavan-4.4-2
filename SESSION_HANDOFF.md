# Cross-session execution mandate

## 2026-11-23 continuation — current state

- **main parent at this handoff:** `2ea2105e917f1b00e7269650f46dbe2831f877f7`; this checkpoint is committed atomically with the source slice below. Fetch live `main` at takeover for the resulting SHA.
- **Backup branch:** `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not modify or merge it.
- **Completed MTK ESW work:** checked port matrix/PVID/ingress/attribute/accept MDIO APIs and caller propagation are on `3ab6e43a2ca7db2ab7521962566c34a4b8b70b1e`; prep commit `a0ff74d73e6d8b9ecb45ba9937c267bf3f4d4916` added checked-write tests. Do not repeat this work.
- **Current source slice:** defer publication of `g_vlan_pvid_wan_untagged` until VLAN-table and port-register programming all succeed; make the multiport VLAN accept ioctl continue independent selected ports and return the first error. Add host policy failure-injection regression and exact-source assertions.
- **Exact-SHA ancestor checks:** Switch CLI #45256010667 and AI docs #45256010835 completed/success on `3ab6e43`; NVRAM #45256010659 completed/success there. XY-C1 Firmware Validation #45256010859 was in progress at last poll. The docs-only child `2ea2105` has AI docs #45256947168 in progress. Do not transfer these statuses to the new source SHA.
- **Tests:** host tests compile with GNU89/Werror/ASan/UBSan in Switch CLI workflow. No local kernel/MIPS compile or physical-router test is claimed. Verify new exact-SHA Actions before any L3 claim.
- **Next implementation task:** convert remaining optional IGMP flood/port-learning/MLD-snooping paths that still use error-collapsing `esw_reg_get/set` into checked MDIO APIs, accumulate per-port failures, and propagate status through VLAN bridge/init callers. Then address telemetry-only link/MIB reads through status-safe interfaces.
- **Working rule:** source change → targeted regression → complete Git tree and single atomic commit → expected-SHA non-forced update of `main` → exact-SHA Actions. Pending checks do not block other independent work.

## Active task

### MTK ESW applied-state cache correctness

- Checked per-port matrix/PVID/ingress/attribute/accept operations and their bridge/VLAN/init/ioctl error propagation are already committed on `main` (`3ab6e43a2ca7db2ab7521962566c34a4b8b70b1e`); do not repeat.
- This source slice fixes a remaining applied-state bug: `g_vlan_pvid_wan_untagged` was published before the VLAN table and port configuration completed. It now commits only after every required hardware stage succeeds. VLAN accept mode also attempts all selected independent ports while preserving the first MDIO error.
- Added a host-testable applied-value commit policy covering negative errors, positive callback normalization, success and invalid pointers. The Switch workflow source checker verifies the cache update is after all port setters and that accept mode does not exit on the first port failure.
- **Exact-SHA Actions:** pending for the new source SHA. Check Switch CLI, XY-C1 Build/TRX/artifact and NVRAM runs independently; no local kernel/MIPS compile is claimed.
- **Next task:** checked MDIO status propagation for optional IGMP flood, per-port learning and MLD snooping operations, followed by safe link/MIB telemetry reads. Add fault-injection coverage and exact-source contracts.

### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Production SHA `59622f3c9612949ddd642fc738e02463f5400def`: XY-C1 Build/TRX/artifact #45252058834 PASS, NVRAM #45252058851 PASS, AI docs #45252059088 PASS. Switch CLI #45252851887 and AI docs #45252851808 PASS on descendant `6acc500`.
- Current source SHA `3ab6e43a2ca7db2ab7521962566c34a4b8b70b1e`: exact-head CI statuses must be checked separately; no Build/TRX/artifact pass is attributed without all three passing on this identical SHA.
- Frozen backup remains untouched. Physical router hardware L4 and local MIPS compilation are NOT VERIFIED.
- LLDT config parser: dedicated L2 workflow #45238664960 PASS; Process umbrella #45239229205 PASS. XY-C1 #45237411963 PASS on exact source SHA `eef4417` (Build/TRX/artifact). AI docs #45239229201 needs one workflow path-filter correction (this commit).
- Shadowsocks DNS proxy preflight and upstream whitespace fixes: #45234515153 PASS, Process #45234515707 PASS and AI docs #45234515042 PASS on `f3acf67`; XY-C1 #45234516216 was pending.
- RStats `601ad73`: RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS; AI doc repair #45236207680 PASS on `3760b93`. XY-C1 #45234873441 still requires final same-SHA step confirmation before an L3 claim.
- Shadowsocks missing-executable test `5a43b18`: lifecycle #45235046114 and Process #45235046275 PASS; AI documentation repaired by #45236207680 on `3760b93`.
- Exact LLDT #45238664960 on `38d3732` passed GNU89/Werror/ASan/UBSan compile/runtime and its Python production/optional-tool checker; AI docs #45238664839 PASS. Process #45239229205 PASS on `79f4d53`. XY-C1 #45237411963 on the production source SHA `eef4417` passed Build, TRX and artifact upload. AI docs #45239229201 exposed only the path-filter asymmetry corrected in this commit. No local compile is claimed.
- Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect all Actions with `head_sha` equal to the source commit in this handoff. Confirm Switch CLI host tests/source assertions, NVRAM audit, and XY-C1 Build, TRX and artifact upload on the identical SHA.
2. Diagnose any failed exact-SHA job from its log and fix the source/checker failure atomically. Pending Actions do not block independent implementation.
3. Convert `esw_igmp_flood_to_cpu`, `esw_igmp_ports_config` and `esw_igmp_mld_snooping` from legacy unchecked `esw_reg_get/set` access to checked reads/writes; propagate results through `esw_vlan_bridge_isolate()` and `esw_ioctl_init_post()`. Add host failure-injection tests and source assertions, then inspect link/MIB read error propagation.

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