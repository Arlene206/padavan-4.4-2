## 2026-11-28 continuation checkpoint

- Repository: `Arlene206/padavan-4.4-2`; development and commits remain main-only. Parent for this NTP bounds slice: `32eda494808267b3798cc269e39168df0efcbbc9`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable, warning marker read. PR #1 remains closed/unmerged; all 13 historical changed paths exist on main in evolved form (two AliDDNS Makefiles unchanged); no blind port pending. Startup/protocol recovery completed once; do not repeat.
- AliyunDrive WebDAV PID-generation hardening: product source `19d51bab853ed2b904e44333e3f6ce0bd72059a4`, checker/docs-only descendants `a4758f6...` and `32eda494...`. Aliyun L2 suite #`46135915187` PASS; Process inventory #`46135763586` PASS; AI docs #`46135915198` PASS. XY-C1 #`46135763583` on source SHA `19d51...` completed PASS: firmware Build, TRX validation and artifact upload each passed; artifact exists. Thus L3 is PASS for that product source tree. Checker/docs-only commits after `19d51...` do not alter firmware inputs. Physical router L4 remains unverified.
- Aliyun residual limitation remains explicit: starttime identity is revalidated before TERM/KILL, but shell `kill(pid)` is not pidfd-bound; a final procfs-read-to-syscall race remains.
- Earlier exact checks: PID1 mdev host #`46134978792` + Process #`46134978799` PASS (`53dd5...`); Ralink host #`46134866158`, wireless radio #`46134866143`, Process #`46134866151` PASS (`e7b8267...`); HTTPD #`46135186083` and AI docs #`46135186051` PASS (`e7418224...`); Inadyn #`46135344758`, WAN route #`46135344709`, Process #`46135344701`, NVRAM #`46135344754` PASS (`5848adac...`); DNSMasq #`46135577877` PASS (`8ca5208...`).
- The active NTP resolver API accepted caller-provided buffer capacities but used unbounded `strlen()` before checking destination sizes. A nonterminated buffer could be read past its boundary. This slice adds bounded terminator discovery and regression coverage that checks both argument positions and verifies no partial mutation on rejection.

## Current repository state

- Live main immediately before this change: `32eda494808267b3798cc269e39168df0efcbbc9`.
- NTP helper/test/static-checker changes are in `trunk/user/rc/src/watchdog_ntp_policy.h`, `trunk/user/rc/src/tests/watchdog_ntp_policy.c`, and `scripts/check_watchdog_ntp_lifecycle.py`.
- The watchdog workflow path filters already include all three paths. It compiles the policy test with GNU89, `-Wall -Wextra -Werror`, ASan and UBSan.
- No local checkout or host compile available in this session. New exact-SHA Actions are required for L2 and L3.

## Active task

- Bound NTP server resolver reads to the caller-supplied capacities. `watchdog_ntp_bounded_string_length()` must use `memchr` and return `ENAMETOOLONG` if a terminator is absent within capacity.
- Validate both inputs before mutating either. Preserve legacy one-server mirror and `pool.ntp.org` fallback. Do not introduce output truncation or partial mutation on insufficient destination capacity.
- Host regression covers unterminated server0 and server1, rejection before either buffer changes, plus existing valid/missing/overlong NVRAM behavior.

## Exact first action for the next GPT

1. Fetch live main and confirm the result SHA of this atomic NTP hardening commit.
2. Check exact-SHA Watchdog NTP host regression/source contract, Process inventory, AI handoff docs, NVRAM audit and XY-C1 Build/TRX/artifact.
3. If any checker or host test fails, inspect only the failing step/log and repair the cause in a new atomic commit. PENDING allows independent work.
4. After this bounded API fix passes, select another genuinely unresolved P1 from `REFACTOR_ROADMAP.md`; do not re-audit complete slices.
5. L4 is NOT VERIFIED without a physical router test.

## Verification boundary

- L1: resolver previously called unbounded `strlen()` despite explicit capacities; both caller inputs are now validated with bounded `memchr` before any write.
- L2: pending exact-head Watchdog host/source-contract and related Actions; no local test was run.
- L3: prior Aliyun product source L3 PASS on `19d51...` (XY-C1 #`46135763583`, Build/TRX/artifact all PASS). This NTP production change requires a fresh same-SHA XY-C1 Build/TRX/artifact PASS.
- L4: NOT VERIFIED; no physical-router validation.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Before ending, inspect live main, exact-head Actions, active task, validation boundary and first next action. Update only when facts materially change; do not create a no-op commit.

# Cross-session execution mandate

- One logical multi-file task, one atomic main commit: create all blobs, one tree and one commit, then update main with a non-forced expected-SHA compare-and-swap. Never force-push or publish per-file intermediate commits.
- Before context exhaustion, finish the current safe slice, publish it, inspect exact-head Actions and leave the next GPT a precise first action and truthful L1-L4 validation boundary.
- Recover once, then execute; do not repeat completed audits or ask the user what to do next.
- PASS permits the next issue, FAIL requires a causal fix, and PENDING permits independent work.
