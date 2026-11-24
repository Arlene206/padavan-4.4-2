## Current repository state

- Active development branch: `main`; source/test commit under verification: `86d3078c1ead2dfd5578830e069b684e12a4b072`. Always fetch the live `main` head again before any write.
- Frozen archive `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable and must not be modified, used for development, or deleted.
- No open PRs; legacy PR #1 is closed and its valid unmerged work was ported to main.
- MTK AP private-ioctl safety is L3 VERIFIED on exact SHA `d33fcf06b73fbe2771ab0b559116d3e750dc4918`: MTK #45384680997, NVRAM #45384681048, AI docs #45384680982 and XY-C1 #45384680986 all PASS on push with that head SHA; XY-C1 Build/TRX/artifact all passed. Physical-router L4 remains NOT VERIFIED.

## Active task — Finish HTTPD protocol-selector validation

- Production fix in `trunk/user/rc/src/httpd_daemon_policy.h` rejects HTTPS-build `http_proto` values outside 0/1/2 with `EINVAL` before listener-specific reads, state publication, port normalization or daemon launch. GNU89 ASan/UBSan policy regression for `-1`, `3`, `99` passed in HTTPD run #45387713984 on `86d3078c1ead2dfd5578830e069b684e12a4b072`.
- Exact HTTPD run #45387713984 FAILED only in its Python source integration contract: the new matcher compared literal escaped newline/tab text against actual C whitespace. The code regression compile/run step passed; no production compiler failure was reported. THIS COMMIT changes the contract to whitespace-normalized matching.
- No XY-C1 run was created for `86d3078`: the dedicated `xy-c1-firmware-validation.yml` path filter omitted `trunk/user/rc/src/httpd_daemon_policy.h`. THIS COMMIT adds the header to both push and pull-request path filters so the policy change receives a same-SHA firmware build.
- Exact HTTPD run #45387713984 source-contract result must be rechecked on this follow-up SHA. Process inventory #45387714115, HTTPD Authentication/Security #45387714072, and AI handoff #45387714060 passed on `86d3078`. NVRAM #45387714231 was still running at the last check. XY-C1 was not triggered on `86d3078`; do not claim L3 for that SHA.
- HTTP-only builds still preserve their legacy default and skip the selector read. The UI-supported HTTPS selector values are exactly 0, 1 and 2.

## Verification boundary

- New follow-up checker/path-filter changes are contained in THIS COMMIT. Fetch live main and inspect the exact-head HTTPD daemon lifecycle, XY-C1 Build/TRX/artifact, AI docs, Process and NVRAM Actions.
- If the HTTPD source contract passes and XY-C1 Build/TRX/artifact pass on the same exact SHA, L3 is verified for this code. Otherwise triage the first causal error and keep L3 open.
- Physical router L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current `main` and query push Actions by exact live SHA; inspect HTTPD daemon lifecycle, XY-C1 Build/TRX/artifact, AI docs, Process and NVRAM.
2. Fix any exact-head failure with one bounded atomic main commit; pending checks do not block independent work.
3. After this selector slice is verified, continue the next highest-priority unclosed stability/security defect outside the completed MTK AP ioctl slice. Do not repeat completed audits absent new evidence.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures, or a changed first action. Do not manufacture documentation churn or no-op commits.

A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once from live repository state and execute development on `main`. Never create or use a development branch or PR; the frozen backup is immutable.

For each multi-file logical slice, create all blobs, one complete tree based on the latest main tree and one commit with the exact main parent, then update `main` using a non-forced compare-and-swap with `expected_sha`. This is one atomic main commit. Never issue sequential partial file commits, force-push, amend, or rewrite published history.

Continue implementation rather than repeating completed audits or stopping at planning. PASS advances; FAIL is investigated and fixed; PENDING does not block independent tasks. Preserve Linux 4.4, GNU89, MIPS/uClibc, router compatibility, security and configuration.

Before context exhaustion, finish the current safe slice and record exact latest main SHA, tests, CI states, unresolved issues and one concrete first action for the next GPT. At session end, inspect live main and exact-head push Actions; update this handoff only when active task or first action materially changes, append substantive milestones to `dev_log.md`, and update roadmap only for material module/status/priority changes. Do not create no-op documentation commits.
