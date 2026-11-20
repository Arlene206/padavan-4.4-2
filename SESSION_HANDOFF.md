# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Recover the repository once, then resume the exact active task; avoid repeated audits, planning-only stops and status-only commits.

- Each logical task ends in one atomic main commit. For multi-file changes create all blobs, one complete Git tree and one commit parented to live `main`, then update `main` with `force=false` and exact `expected_sha`.
- Never create/use development branches or PRs, force-push, amend published commits, or modify the frozen backup.
- Check Actions by exact `head_sha`. Diagnose FAIL from causal logs; while checks are PENDING, continue independent work.
- Preserve GNU89, MIPS/uClibc, BusyBox/Linux 4.4 compatibility, fail-closed behavior, process identity checks, bounded operations and meaningful regression coverage.
- Do not ask the user what to do next. Before context exhaustion, finish the current safe slice and leave an accurate next action.

## Current repository state

- Parent `main` before this WAN fallback slice: `759a19abeff06bbfc6abcb1a13ee1263575a4945`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; read-only and untouched.
- Checker repair commit `759a19a` updates the GAI callback declarations expected by `check_dev_init_boot.py`. Its exact-head Process run `44674401664` and AI docs run `44674401794` were IN_PROGRESS at last poll; no PASS is claimed.
- The original GAI Process run `44673235684` failed only in the PID 1 semantic checker because it expected a callback without `void *context`; the GAI policy test was skipped after that early failure. The production callback signatures were correct. The focused GAI policy binary passed locally with GNU89/Werror/ASan/UBSan. GAI source commit `24dfe60` XY-C1 `44673236140` was still in progress at last poll.
- TCP policy slice `e387b92` Process `44672214134` and AI docs `44672214038` passed. Its XY-C1 completion was not confirmed at last poll.
- Historical PR #1 is CLOSED/UNMERGED; CA trust-store bootstrap and AliDDNS HMAC implementation/tests are already present on `main`.
- Physical-router L4 remains NOT VERIFIED.

## Active task completed in this slice

- Fixed a confirmed fail-closed gap in PPP-start firewall recovery. `restore_wan_after_ppp_start_failure()` previously ignored the return from publishing `wanN_ifname_t` via `set_wan_unit_value()`, then rebuilt the firewall even when the NVRAM write had failed. The firewall could therefore be rebuilt against stale PPP/physical interface state and incorrectly report a successful physical-WAN restoration.
- Added a private status-returning setter for this sensitive callsite without changing the legacy public void setter ABI or other legacy callers. It clears/captures `errno`, normalizes NVRAM `0/-1` outcomes to positive errno (fallback EIO), and rejects truncation. Recovery now returns before firewall rebuild if interface publication fails; the existing outer recovery then attempts to keep the physical WAN interface down.
- Added GNU89 unit coverage for success/stale errno, concrete errno propagation, missing errno fallback and unexpected nonzero API results. Extended the WAN route workflow (push + pull_request) and DDNS source contract to pin checked publication-before-firewall ordering and fail-closed ifdown behavior. The policy test passed locally under GNU89/Werror/ASan/UBSan.

## Verification boundary

- The WAN NVRAM fix has not yet been validated by exact-head Actions at this checkpoint. The checker repair's Process and AI docs runs were pending at last poll. No current-source XY-C1 Build/TRX/artifact PASS is claimed; no physical-router L4 test was performed.
- No full firmware build was available locally. Host evidence is limited to the new WAN NVRAM policy binary and the prior focused GAI/TCP policy binaries.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head Actions for this WAN NVRAM fix. Require WAN route policy + `check_wan_ddns_status.py`, Process, AI handoff docs and applicable workflow contracts to pass. Diagnose any failure from its exact job log.
2. Recheck exact-head XY-C1 on the current production source: detector, Build XY-C1, TRX verification and artifact upload. Do not claim L3 from older source SHAs.
3. Continue the next P1 RC state-publication audit: trace `set_wan_unit_value_int()`, `set_wan_unit_param()` and other NVRAM publication calls specifically where later firewall, interface, route or daemon state depends on the published value. Preserve ABI; only change the next proven failure-sensitive callsite and add an injected regression. Avoid converting all legacy telemetry writes speculatively.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state before ending. Update this handoff only when resumable facts, validation state, blocker or first action materially change. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

## Evidence discipline

Keep L1 source review, L2 host/Actions, L3 MIPS build/TRX and L4 physical-device evidence separate. A skipped build is not PASS, a successful cross-build does not establish router runtime behavior, and every claim must name the exact source SHA and run.
