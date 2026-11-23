# Cross-session execution mandate

## Current repository state

### 2026-11-23 continuation — current state

- **Main source baseline:** parent `be150b19aaed819b0692952475f4aa611f555601` (`test(raeth): scope GE PHY contracts to complete function bodies`). Parent Switch CLI and AI documentation checks pass. This commit adds proc MIB error visibility and path-filter coverage.
- **Backup branch:** `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, merge or delete it.
- **Completed MTK ESW configuration work:** checked port matrix/PVID/ingress/attribute/accept writes and caller propagation; VLAN applied-state PVID cache ordering; IGMP flood-to-CPU, per-port learning and MLD/PIC checked writes with first-error propagation through bridge/init/ioctls.
- **Completed paged PHY/link/TRGMII work:** failed page selection stops PHY register access; reset-ready polling is bounded/status-bearing; selector/data transactions are IRQ-safe serialized; GSW/ESW link notifications require successful samples; MT7621/MT7623 TRGMII setup/calibration propagate MDIO errors. MT7621 25MHz branches are braced; GE PHY workflow source contracts now isolate full function bodies.
- **Current proc MIB slice (THIS COMMIT):** `DUMP_EACH_PORT` prints `ERR` for failed reads instead of false zero counters; both `0x4fe0` snapshot-control write failures are reported. Switch CLI source contract and Switch CLI/XY-C1 path filters now include `ra_dbg_proc.h/.c`.
- **Parent exact-SHA Actions on `be150b19aaed819b0692952475f4aa611f555601`:** Switch CLI #45268640001 PASS; AI docs #45268640240 PASS. XY-C1 and NVRAM did not trigger for the workflow-only parent change. These results do not validate this MIB change.
- **Verification boundary:** no local Linux kernel/MIPS build or physical-router L4 is claimed. XY-C1 Build, TRX and artifact upload must all PASS on one identical production-source SHA before claiming L3.
- **Next implementation task:** inspect exact-head Switch CLI, AI docs, NVRAM and XY-C1 results for this proc MIB change. Then continue targeted audit of RAETH proc/debug telemetry for ignored MDIO reads/writes only where concrete failure paths exist.
- **Working rule:** source change → targeted regression → one complete Git tree/atomic commit → expected-SHA non-forced update of `main` → exact-SHA Actions. Pending checks do not block independent work.


## Active task

### RAETH proc MIB diagnostic failure visibility

- Parent `be150b19aaed819b0692952475f4aa611f555601` passed Switch CLI and AI docs. `DUMP_EACH_PORT` currently hides a failed `mii_mgr_read()` as zero, and `internal_gsw_cnt_read()` ignores both MIB snapshot-control write results.
- This commit prints aligned `ERR` markers for failed counter reads and explicit messages for each failed `0x4fe0` write. Source-contract checks and Switch CLI/XY-C1 path filters cover the proc files.
- **Next task:** inspect exact-head Actions, then audit remaining RAETH proc/debug telemetry for concrete ignored MDIO returns.


### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Parent source SHA `be150b19aaed819b0692952475f4aa611f555601`: Switch CLI #45268640001 PASS; AI docs #45268640240 PASS. XY-C1 and NVRAM did not trigger for that workflow-only parent. The proc MIB source change is this commit; exact-head checks pending.
- Earlier production SHA `59622f3c9612949ddd642fc738e02463f5400def`: XY-C1 Build/TRX/artifact #45252058834 PASS and NVRAM #45252058851 PASS; those historical results do not validate later source changes.
- Frozen backup remains untouched. Physical router hardware L4 and local MIPS compilation are NOT VERIFIED.
- LLDT config parser: dedicated L2 workflow #45238664960 PASS; Process umbrella #45239229205 PASS. XY-C1 #45237411963 PASS on exact source SHA `eef4417` (Build/TRX/artifact). AI docs #45239229201 needs one workflow path-filter correction (this commit).
- Shadowsocks DNS proxy preflight and upstream whitespace fixes: #45234515153 PASS, Process #45234515707 PASS and AI docs #45234515042 PASS on `f3acf67`; XY-C1 #45234516216 was pending.
- RStats `601ad73`: RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS; AI doc repair #45236207680 PASS on `3760b93`. XY-C1 #45234873441 still requires final same-SHA step confirmation before an L3 claim.
- Shadowsocks missing-executable test `5a43b18`: lifecycle #45235046114 and Process #45235046275 PASS; AI documentation repaired by #45236207680 on `3760b93`.
- Exact LLDT #45238664960 on `38d3732` passed GNU89/Werror/ASan/UBSan compile/runtime and its Python production/optional-tool checker; AI docs #45238664839 PASS. Process #45239229205 PASS on `79f4d53`. XY-C1 #45237411963 on the production source SHA `eef4417` passed Build, TRX and artifact upload. AI docs #45239229201 exposed only the path-filter asymmetry corrected in this commit. No local compile is claimed.
- Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-SHA Switch CLI, AI docs, NVRAM and XY-C1 Actions for this proc MIB change. Claim L3 only if Build, TRX and artifact upload all PASS on one identical production-source SHA.
2. Inspect any failed workflow logs and distinguish source-checker failures from host regressions or firmware failures.
3. Continue auditing RAETH proc/debug telemetry for concrete ignored MDIO returns, without changing unrelated counter interfaces.


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