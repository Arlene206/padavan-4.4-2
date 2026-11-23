# Cross-session execution mandate

## 2026-11-23 continuation handoff (parent main: 3760b93c10430cb1bdad3e158b35823147da75a6; fetch live main at takeover)

### Completed engineering slices and exact evidence
- `ba0dfca34d3e87f68cc44190509e29b2ce4e5853`: AliyunDrive process presence bound to configured executable device/inode, rejecting BusyBox `pidof` comm/argv[0] spoofing. Exact lifecycle #45230625667 PASS (shell syntax, MIPSel CLI under QEMU, lifecycle, early-lock and RC integration); Process #45230625920 PASS; AI docs #45230625723 PASS. L2 host/QEMU only; target router shell runtime and physical L4 not verified.
- `9ebc57999dad5da47e20414174cd7572ee64bbd7`: shared process inventory revalidates `/proc/PID/stat` after cmdline reads and rejects PID reuse. Exact Process #45227637642, fail-closed #45227637610 and AI docs #45227637592 PASS. XY-C1 #45227637594 passed Build, TRX and artifact upload on the identical SHA (L3).
- `7e81b11d63b1bf20d8a1eb103cc7598b9b898b1a`: repair Shadowsocks backup-server startup, firewall node identity and required-stage error propagation. Exact Shadowsocks #45228682477, Process #45228682072 and AI docs #45228682265 PASS. XY-C1 #45228682113 passed Build/TRX/artifact on the same SHA (L3).
- `53c6f6623e5531deeb889762e81bf1943c1abf50`: DNSMasq split-DNS source-contract correction. Exact DNSMasq #45229862631 and AI handoff #45229862614 PASS.
- Shadowsocks DNS proxy preflight plus malformed-upstream whitespace rejection and fixture/checker updates: exact lifecycle #45234515153 PASS on `f3acf67d58203ae7d18b6d2755014840ba10cccc`; Process #45234515707 PASS; AI docs #45234515042 PASS. XY-C1 #45234516216 was IN_PROGRESS at the last poll. Previous exact lifecycle #45233294870 on `7fdf985` also passed.
- Current RStats source slice (parent `f3acf67d58203ae7d18b6d2755014840ba10cccc`): `save_history_json()` now reads its interface selector through `nvram_get_checked()`, copies the shared NVRAM cache value before another NVRAM operation, and returns before atomic output staging on backend, NULL-present or overlong state. Missing key retains legacy empty/default behaviour. Exact-head Actions pending.

- `601ad73cb90227d01497a1a4f28a967de8c17a6a`: RStats selector uses checked NVRAM read and copies the cache-backed value before later operations. Exact RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS. Its AI-doc failure was corrected by `3760b93`, exact AI docs #45236207680 PASS; XY-C1 #45234873441 was pending.
- `5a43b18722b9d2f3f864ef6bd1025084ff1edb85`: missing-DNS-proxy-executable regression; exact Shadowsocks #45235046114 and Process #45235046275 PASS. Its AI-doc failure was likewise corrected by `3760b93`; exact AI docs #45236207680 PASS.

- Exact-source commit `eef441729725ff6b06c45a95b7c8414ab16d2f72` added bounded parsing to both `lld2d` and optional `lld2test`, including strict numeric parsing, ASan/UBSan cases, a dedicated workflow and XY-C1 path trigger. Its LLDT #45237411685 and Process #45237411877 failed only because seven fixture character literals were emitted as multi-character `'\\\\0'` instead of NUL `'\\0'`; follow-up corrects them and requires fresh exact-SHA verification. AI docs #45237411785 PASS. XY-C1 #45237411963 and NVRAM audit #45237411699 were still in progress at last poll.
- Follow-up `14be2b00c5f32eccb8694281f42d8b1bfed3dcb5`: NUL character spelling was corrected, but exact LLDT #45237898307 then reached execution and failed on the first fixture assertion because test input strings also contained doubled CR/LF escapes. The next test-only commit corrects these remaining test literals. AI handoff #45237898460 PASS; Process umbrella #45237898424, Process lifecycle #45237898195 and NVRAM #45237898280 were pending at last poll.

## Current repository state
- Repository `Arlene206/padavan-4.4-2`; work only on `main`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, reset, merge, rebase or force-push it.
- PR #1 is closed/unmerged. AliDDNS CA trust-store/HMAC work is represented or superseded on current `main`.
- No local checkout or MIPS/uClibc toolchain is available. Do not claim local GNU89 test execution, target firmware/TRX/artifact or physical-device validation; exact GitHub Actions are the verification source.
- XY-C1 disables `CONFIG_FIRMWARE_INCLUDE_ALDRIVER`; AliyunDrive shell runtime is not covered by XY-C1. Its dedicated workflow establishes only host/QEMU checks.

## Active task

### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- GNU89/Werror/ASan/UBSan tests cover assignment parsing, embedded NUL, oversized names/values, numeric boundaries and path termination. Exact source and firmware Actions are pending for the new commit.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- Shadowsocks DNS proxy preflight and upstream whitespace fixes: #45234515153 PASS, Process #45234515707 PASS and AI docs #45234515042 PASS on `f3acf67`; XY-C1 #45234516216 was pending.
- RStats `601ad73`: RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS; AI doc repair #45236207680 PASS on `3760b93`; XY-C1 #45234873441 was pending.
- Shadowsocks missing-executable test `5a43b18`: lifecycle #45235046114 and Process #45235046275 PASS; AI documentation repaired by #45236207680 on `3760b93`.
- Local GNU89/Werror/ASan/UBSan parser-policy fixture passed. No full router tree or MIPS/uClibc firmware compile was run locally. Exact-head LLDT L2 and same-SHA XY-C1 Build/TRX/artifact remain required.
- Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-head LLDT parser and Process Actions on the latest test-fixture follow-up. Confirm the GNU89/Werror/ASan/UBSan executable completes all cases, then confirm the Python source contract and Process umbrella are PASS. Review XY-C1 only if the production build input SHA contains parser source changes.
2. `eef4417`: first LLDT #45237411685 and Process #45237411877 failed compilation because seven C NUL character constants were multi-character literals. `14be2b0` corrected them, then LLDT #45237898307 reached runtime and failed the first string-parse assertion because the test input strings still encoded literal backslash-n/r. This is a fixture issue, not a confirmed production parser bug.
3. On `14be2b0`, AI docs #45237898460 PASS; Process umbrella #45237898424, Process inventory #45237898195 and NVRAM audit #45237898280 were still in progress at last poll. Recheck these, and the eef XY-C1 #45237411963, before closing the task.
4. Once LLDT fixture and integration checks pass, continue a new whole-userland reliability/security slice rather than repeating completed reviews. Keep the backup immutable, commit complete trees atomically with non-forced expected-SHA CAS.

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