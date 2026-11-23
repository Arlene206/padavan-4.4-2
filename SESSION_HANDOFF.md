# Session Handoff

## Current repository state

- Live `main`: `3ec4e18b735a1e132884d91ab259b365a420aebc` (current at the handoff checkpoint).
- Read-only backup: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. This session did not modify it.
- Work in this session was committed as complete atomic trees to `main` only; no force-push was used.
- The older EPHY handoff below this file was stale (it described `02f174a...` as current). Do not use its old current-SHA/Actions claims as live state. The FE/GE calibration changes remain part of repository history, but this session's active source work moved into userland stability.

## Active task

### Userland stability: checked NVRAM errors and safe service transitions

Completed source slices on `main`:

- `75a29ff3ac4f2791a3fbfbf60b20e54e7f5f0cae`: IPTV proxy identity/policy reads changed to checked NVRAM reads and copied buffers. `stop_igmpproxy`, `start_igmpproxy`, and `restart_iptv` preflight the settings they need before service/bridge side effects.
- `05de0ec91e89a44fcf5c9fd321e25c94b0fd5c8b`: corrected the legacy missing-key defaults after tracing `nvram_invmatch` semantics: absent `mr_enable_x` maps to `"1"` (the old code did not early-return), and absent `mr_qleave_x` maps to `"0"` (quickleave remains disabled). Actual backend read errors still abort the transition. This supersedes the intermediate default mapping in `7c352fcc...`.
- `6a9358e03d930057e46f69a8f74c0ef71d75cb29` + `50b712a8c1a62648f51ac223d7ed0f57b53cc8cf`: HTTPD Basic-auth username/password are read as one checked NVRAM transaction, copied before shared-cache reuse, and the authentication cache remains invalid on read/close/length failure. The legacy `static void load_nvram_auth(void)` signature is preserved for source-contract compatibility.
- `2b078871dd959e65bda80c8f00a791fabe1c3c11`, `108c1c712407ecab8cfc6758a87ab49838f47ec4`, and `3ec4e18b735a1e132884d91ab259b365a420aebc`: HTTPD `http_access` ACL loading now distinguishes absent configuration from NVRAM errors; unknown configuration blocks remote clients while retaining loopback recovery. Malformed, negative, trailing-junk, and overflowing numeric values are rejected instead of becoming ACL mode zero through `atoi`. The last commit corrected C `'\\0'`/test escape literals; exact-head HTTPD CI passed.
- `ebb80e3df48044b48261df53830057c22016ece5`, `4363bd2ce8b131c33ffbc9f488019b8536762c24`, and `7a3e3045f7437b434cf37bc936426c0ee3122a57`: HTTPD NVRAM group compaction now distinguishes missing/read-error values, aborts before mutation if snapshot reads fail, and holds one NVRAM transaction across snapshot, compaction, and rollback. The workflow's source assertion was updated to match the transaction-scoped getter.
- The source regressions are integrated into the existing HTTPD and WAN workflows. CI failures encountered while bringing the contracts into sync were corrected in follow-up atomic commits; do not treat an intermediate failed run as the final state.

Next independent candidate (not yet confirmed as a vulnerability): inspect `trunk/user/httpd/src/aidisk.c::ej_get_folder_tree` for symlink/path escape behavior in USB folder enumeration. Trace the browser caller and any downstream path consumer first; only implement a fix if the data flow confirms a security boundary violation. Add a regression for the demonstrated case.

## Verification boundary

- Current exact `main` SHA: `3ec4e18b735a1e132884d91ab259b365a420aebc`.
  - HTTPD Authentication and Security Regression #45289740712: **PASS** on this exact SHA.
  - Process lifecycle inventory #45289740731: **PASS** on this exact SHA.
  - NVRAM transaction boundary audit #45289740708: **PASS** on this exact SHA.
- Product-source commit `05de0ec91e89a44fcf5c9fd321e25c94b0fd5c8b` exact-SHA XY-C1 run #45289029423 was **IN_PROGRESS** at the last check. On that same SHA, WAN DHCP #45289029808, WAN route #45289029458, firewall #45289029699, HTTPD #45289029732, process inventory #45289029552, USB storage #45289029901, process lifecycle #45289029518, and NVRAM audit #45289029376 completed **PASS**.
- The current `main` SHA is later than the product-source SHA above. No exact-`main` XY-C1 Build + TRX + artifact run was observed. Therefore current-tree L3 firmware validation remains **NOT VERIFIED**; do not infer a same-SHA pass from the older run.
- No local MIPS/kernel cross-build or physical router/hardware test is claimed. Hardware L4 is **NOT VERIFIED**.

## Exact first action for the next GPT

1. Poll XY-C1 run #45289029423 and record its terminal result for source SHA `05de0ec91e89a44fcf5c9fd321e25c94b0fd5c8b`; do not label it an exact-`main` result.
2. Continue the independent AiDisk path/symlink data-flow investigation described above while any unrelated CI is pending.
3. Implement only a confirmed defect with a focused regression, create one complete tree/one atomic commit using a non-forced expected-SHA update of `main`, and inspect Actions for that exact new SHA.
4. Before context exhaustion, update this handoff with exact live refs, test results, Action states, and one concrete first task.

---

# Cross-session execution mandate

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
