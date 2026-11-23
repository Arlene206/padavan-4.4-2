# Cross-session execution mandate

## 2026-11-23 continuation handoff (parent main: f3acf67d58203ae7d18b6d2755014840ba10cccc; fetch live main at takeover)

### Completed engineering slices and exact evidence
- `ba0dfca34d3e87f68cc44190509e29b2ce4e5853`: AliyunDrive process presence bound to configured executable device/inode, rejecting BusyBox `pidof` comm/argv[0] spoofing. Exact lifecycle #45230625667 PASS (shell syntax, MIPSel CLI under QEMU, lifecycle, early-lock and RC integration); Process #45230625920 PASS; AI docs #45230625723 PASS. L2 host/QEMU only; target router shell runtime and physical L4 not verified.
- `9ebc57999dad5da47e20414174cd7572ee64bbd7`: shared process inventory revalidates `/proc/PID/stat` after cmdline reads and rejects PID reuse. Exact Process #45227637642, fail-closed #45227637610 and AI docs #45227637592 PASS. XY-C1 #45227637594 passed Build, TRX and artifact upload on the identical SHA (L3).
- `7e81b11d63b1bf20d8a1eb103cc7598b9b898b1a`: repair Shadowsocks backup-server startup, firewall node identity and required-stage error propagation. Exact Shadowsocks #45228682477, Process #45228682072 and AI docs #45228682265 PASS. XY-C1 #45228682113 passed Build/TRX/artifact on the same SHA (L3).
- `53c6f6623e5531deeb889762e81bf1943c1abf50`: DNSMasq split-DNS source-contract correction. Exact DNSMasq #45229862631 and AI handoff #45229862614 PASS.
- Shadowsocks DNS proxy preflight plus malformed-upstream whitespace rejection and fixture/checker updates: exact lifecycle #45234515153 PASS on `f3acf67d58203ae7d18b6d2755014840ba10cccc`; Process #45234515707 PASS; AI docs #45234515042 PASS. XY-C1 #45234516216 was IN_PROGRESS at the last poll. Previous exact lifecycle #45233294870 on `7fdf985` also passed.
- Current RStats source slice (parent `f3acf67d58203ae7d18b6d2755014840ba10cccc`): `save_history_json()` now reads its interface selector through `nvram_get_checked()`, copies the shared NVRAM cache value before another NVRAM operation, and returns before atomic output staging on backend, NULL-present or overlong state. Missing key retains legacy empty/default behaviour. Exact-head Actions pending.

### Repository invariants
- Repository `Arlene206/padavan-4.4-2`; work only on `main`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, reset, merge, rebase or force-push it.
- PR #1 is closed/unmerged. AliDDNS CA trust-store/HMAC work is represented or superseded on current `main`.
- No local checkout or MIPS/uClibc toolchain is available. Do not claim local GNU89 test execution, target firmware/TRX/artifact or physical-device validation; exact GitHub Actions are the verification source.
- XY-C1 disables `CONFIG_FIRMWARE_INCLUDE_ALDRIVER`; AliyunDrive shell runtime is not covered by XY-C1. Its dedicated workflow establishes only host/QEMU checks.

### Active task — RStats history selector
- Production: `trunk/user/rc/src/rstats.c`, `save_history_json()`; policy: `trunk/user/rc/src/rstats_persistence_policy.h`; test: `trunk/user/rc/src/tests/rstats_policy_test.c`; source contract: `.github/workflows/rstats-stability.yml`.
- `RSTATS_NVKEY_DM` is the history-interface selector. The old `nvram_safe_get()` collapsed absent key and backend failure, allowing a transient read/close failure to silently select history index 0.
- `rstats_nvram_copy_selector()` preserves absent-key legacy default, copies a present non-NULL selector only when it fits, and maps backend, present-NULL, overlong and invalid-destination states to errno while clearing stale output.
- `save_history_json()` captures errno immediately after checked read, copies the cache-backed value before further NVRAM operations, and returns before `rstats_file_open_stream()` on error. A valid-but-unmatched selector keeps the existing first-interface fallback.
- GNU89 policy regression covers present, absent, backend `EACCES`, NULL, overlong, NULL destination and zero capacity. Workflow contract requires checked read → copy → UNKNOWN gate → atomic stream open.

### Verification boundary
- Current pre-RStats source SHA `f3acf67` has Shadowsocks lifecycle #45234515153 PASS, Process #45234515707 PASS and AI handoff #45234515042 PASS; XY-C1 #45234516216 was running at last poll. The RStats commit must be checked independently on its exact SHA.
- Earlier proven L3 baselines: XY-C1 #45227637594 on `9ebc579` and #45228682113 on `7e81b11` both passed Build, TRX and artifact upload on the identical SHA.
- AliyunDrive L2 runs #45230625667/#45230625920/#45230625723 PASS; DNSMasq #45229862631 and AI docs #45229862614 PASS on `53c6f66`.
- RStats exact acceptance: GNU89/Werror/ASan/UBSan policy compile/run, source-order contract, Process umbrella, AI handoff and same-SHA XY-C1 Build/TRX/artifact. Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main`; inspect exact-push RStats, Process inventory, AI handoff and XY-C1 Actions for the current commit. Verify GNU89/Werror/ASan/UBSan selector policy, checked-read/copy/error/publication ordering, and same-SHA firmware steps. Fix exact failures before claiming L2/L3.
2. If relevant Actions remain PENDING, work on an independent security/reliability defect rather than waiting. If any test fails, diagnose the actual failing step and commit a focused correction with a new exact-SHA check.
3. Once RStats is closed, select the next highest-priority unaddressed defect in the whole-userland roadmap; do not re-audit process PID reuse, AliyunDrive inode identity, Shadowsocks backup/DNS lifecycle or DNSMasq split-DNS without new evidence.
4. For each logical change, create all changed blobs and one complete Git tree/commit, then update `main` with non-forced expected-SHA CAS. Preserve the backup branch.

## Cross-session operating constraints
Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on `main`; each logical source or CI change is one atomic main commit: create all changed blobs and one complete Git tree before creating that commit, then update `main` using a non-forced expected-SHA compare-and-swap. Do not ask the user what to do next.

## Mandatory handoff checkpoint
The checkpoint is mandatory; the document edit is conditional.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. If the checkpoint facts and first action remain accurate, explicitly conclude with no file change and no no-op commit.

## Development loop
- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.