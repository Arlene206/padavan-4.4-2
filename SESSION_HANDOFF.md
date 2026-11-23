# Cross-session execution mandate

## Current repository state

### 2026-11-23 continuation — current state

- **Source baseline:** parent `5ba21118a12464be1623b03c35dc3b48cb43f66d`; this commit adds checked switch-link IRQ sampling and failure regression. Fetch live `main` at takeover for the resulting exact SHA.
- **Backup branch:** `backup/pre-plugin-overhaul-2026-11-14` remains frozen at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, merge or delete it.
- **Completed MTK ESW configuration work:** checked port matrix/PVID/ingress/attribute/accept writes and caller propagation; VLAN applied-state PVID cache ordering; IGMP flood-to-CPU, per-port learning and MLD/PIC checked writes with first-error propagation through bridge/init/ioctls. Do not repeat these completed slices.
- **Completed paged PHY MDIO work:** page-select failure stops global/local PHY access; reset-ready polling is bounded/status-bearing; page selector/data sequences are serialized under a dedicated IRQ-safe lock.
- **Current source slice (THIS COMMIT):** GSW and legacy ESW link IRQ handlers consume status-bearing MDIO samples. Failed reads cannot create a false link-down callback/cache transition; GSW W1C acknowledgement is limited to successfully sampled port flags so failed flags remain pending. GNU89/Werror/ASan/UBSan policy regression passes locally; exact-head workflow contract and Actions must be checked after landing.
- **Parent exact-SHA Actions on `5ba21118a12464be1623b03c35dc3b48cb43f66d`:** Switch CLI #45266083220 IN_PROGRESS; XY-C1 #45266083296 IN_PROGRESS; AI handoff docs #45266083382 IN_PROGRESS; NVRAM #45266083302 QUEUED.. Verify parent runs only against that SHA; they do not validate this new commit.
- **Verification boundary:** no local Linux kernel/MIPS compile, no confirmed same-source XY-C1 Build/TRX/artifact for this slice, and no physical-router L4. L4 remains NOT VERIFIED.
- **Next implementation task:** inspect definitions and callers of `esw_get_port_mib_rgpc()` and `esw_get_port_mib_tgpc()` in the WAN traffic telemetry path. Convert any error-collapsing packet-counter reads to status-bearing internal helpers without changing exported ABI; add failure-injection coverage.
- **Working rule:** source change → targeted regression → one complete Git tree/atomic commit → expected-SHA non-forced update of `main` → exact-SHA Actions. Pending checks do not block independent tasks.


## Active task

### MTK switch IRQ link-status MDIO failure handling

- This commit adds `ra_switch_mdio_read_checked()` and `ra_switch_mdio_read_link_status()` to the private RAETH MDIO policy. A failed callback never publishes its partial output or overwrites the caller's previous link state.
- `gsw_interrupt()` notifies the link hook only after a successful sample and acknowledges only successfully sampled port-change bits; failed flags remain pending. The legacy `esw_interrupt()` preserves its previous cached state and skips transition/debug reporting on an MDIO failure.
- Host policy regression: GNU89/Werror/ASan/UBSan compile and runtime PASS locally. Exact-source workflow contract and exact-head Switch CLI/XY-C1 Actions remain pending for this commit.
- **Next task:** inspect `esw_get_port_mib_rgpc()` / `esw_get_port_mib_tgpc()` definitions and caller graph for swallowed MIB packet-counter MDIO failures.

### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Parent source SHA `5ba21118a12464be1623b03c35dc3b48cb43f66d`: Switch CLI #45266083220 IN_PROGRESS; XY-C1 #45266083296 IN_PROGRESS; AI handoff docs #45266083382 IN_PROGRESS; NVRAM #45266083302 QUEUED.. The link-IRQ implementation is THIS COMMIT; exact-source actions have not yet run.
- Earlier production SHA `59622f3c9612949ddd642fc738e02463f5400def`: XY-C1 Build/TRX/artifact #45252058834 PASS and NVRAM #45252058851 PASS; those historical results do not validate later source changes.
- Frozen backup remains untouched. Physical router hardware L4 and local MIPS compilation are NOT VERIFIED.
- LLDT config parser: dedicated L2 workflow #45238664960 PASS; Process umbrella #45239229205 PASS. XY-C1 #45237411963 PASS on exact source SHA `eef4417` (Build/TRX/artifact). AI docs #45239229201 needs one workflow path-filter correction (this commit).
- Shadowsocks DNS proxy preflight and upstream whitespace fixes: #45234515153 PASS, Process #45234515707 PASS and AI docs #45234515042 PASS on `f3acf67`; XY-C1 #45234516216 was pending.
- RStats `601ad73`: RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS; AI doc repair #45236207680 PASS on `3760b93`. XY-C1 #45234873441 still requires final same-SHA step confirmation before an L3 claim.
- Shadowsocks missing-executable test `5a43b18`: lifecycle #45235046114 and Process #45235046275 PASS; AI documentation repaired by #45236207680 on `3760b93`.
- Exact LLDT #45238664960 on `38d3732` passed GNU89/Werror/ASan/UBSan compile/runtime and its Python production/optional-tool checker; AI docs #45238664839 PASS. Process #45239229205 PASS on `79f4d53`. XY-C1 #45237411963 on the production source SHA `eef4417` passed Build, TRX and artifact upload. AI docs #45239229201 exposed only the path-filter asymmetry corrected in this commit. No local compile is claimed.
- Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect push Actions for its exact SHA. Resolve the parent runs on `5ba21118a12464be1623b03c35dc3b48cb43f66d`; claim L3 only if Build, TRX verification and artifact upload all PASS on one identical product-source SHA.
2. Inspect exact-head Switch CLI and AI handoff documentation results for this link-IRQ commit; triage any failure from its job log before calling L2 closed.
3. Trace `esw_get_port_mib_rgpc()` and `esw_get_port_mib_tgpc()` definitions/callers; preserve packet-counter MDIO failures through status-bearing internals without changing exported ABI.

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