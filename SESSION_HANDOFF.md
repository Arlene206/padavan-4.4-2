## Current repository state

- Active development branch: `main`; parent source SHA for the current HTTPD selector slice: `d33fcf06b73fbe2771ab0b559116d3e750dc4918`. Always fetch the live `main` head again before any write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable and must not be modified, used for development, or deleted.
- No open PRs; legacy PR #1 is closed and its valid unmerged work was ported to main.
- MTK AP private-ioctl safety slice is implemented through deauthentication: exact main SHA `d33fcf06b73fbe2771ab0b559116d3e750dc4918` Actions MTK #45384680997, NVRAM #45384681048, AI handoff #45384680982, and XY-C1 #45384680986 all completed PASS on push with this exact head SHA. XY-C1 Build, TRX verification and artifact upload all PASS; L3 is verified for this source SHA. Physical router L4 remains NOT VERIFIED.

## Active task — Reject invalid HTTPD listener protocol selectors

- Source: `trunk/user/rc/src/httpd_daemon_policy.h`, called by `start_httpd_locked()` in `trunk/user/rc/src/services.c`.
- The Advanced Services UI offers exactly `http_proto=0` (HTTP), `1` (HTTPS), and `2` (HTTP+HTTPS). In HTTPS-enabled builds, `httpd_start_config_read()` previously accepted negative or out-of-range persisted values. Those values selected neither listener branch, so `start_httpd_locked()` could continue to launch the daemon with no explicit `-p`/`-s` listener arguments while `httpd_start_config_firewall_required()` treated the selector as exposing no listener.
- THIS COMMIT rejects `http_proto < 0 || http_proto > 2` with `EINVAL` immediately after the checked read and before any listener-specific NVRAM reads, port normalization, state publication, or daemon launch. HTTP-only builds retain their legacy behavior and do not read the HTTPS selector.
- Regression: `trunk/user/rc/src/tests/httpd_daemon_policy.c` exercises `-1`, `3`, and `99`, requiring one NVRAM read and an untouched output snapshot. `.github/workflows/httpd-daemon-lifecycle.yml` also pins validation-before-listener-read and the new test invocation.
- No local checkout is available in this runtime. The exact-head HTTPD workflow will compile/run the GNU89 ASan/UBSan regression and integration contract; XY-C1 Actions are required for L3.

## Verification boundary

- The HTTPD source/test/checker update is contained in THIS COMMIT; fetch live main and inspect exact `push` Actions by `head_sha` before declaring it verified.
- Require HTTPD daemon lifecycle stability regression and AI handoff contract PASS. Require same-SHA XY-C1 Build, TRX and artifact PASS before L3.
- L4 physical-router behavior: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current `main` and query Actions by the exact live SHA. Confirm the HTTPD daemon lifecycle run compiles/runs the new invalid-protocol regression and its source contract; also inspect AI docs and XY-C1 Build/TRX/artifact on the same SHA.
2. If the exact HTTPD or docs run fails, diagnose the first causal assertion/compiler failure and make a bounded atomic main fix. A pending XY-C1 build does not block independent work.
3. After the HTTPD selector fix passes, continue the next highest-priority unclosed stability defect outside this already-validated MTK AP ioctl cluster; do not repeat closed HTTPD/MTK checks absent new evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
