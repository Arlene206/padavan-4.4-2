# 2026-11-18 continuation checkpoint — bounded wait source-contract alignment

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; active branch `main`. The ABSENT-under-errexit source fix is `cfbf00f9d9100daf82017403e63342658bbac29e`. Current parent `2f3b3f47c40e4453a654d4fc202b7bdeb2073cdc` only synchronized the checker, but its revised expected count was still wrong.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 closed/unmerged.
- XY-C1 run `44283616981` targets cfbf source and was IN_PROGRESS at last observation. Build XY-C1, TRX verification and artifact upload have not been confirmed on cfbf; this checker-only commit does not itself claim L3. Physical-router L4 NOT VERIFIED.

## Active task
- The cfbf shell regression (syntax, startup status, process lifecycle, Adblock) passed, as did AI handoff and Process inventory workflows. Its final checker failed due a stale count for bounded `sleep 1 || return 2` guards.
- Parent 2f3b changed the checker to expect seven guards, but exact-head Shadowsocks run `44284415648` still failed at that same aggregate assertion. Direct inspection of the checked-in helper shows exactly four bounded sleep sites: named shutdown, pattern shutdown, named PID-count startup, and pattern PID-count startup. The same assertion separately checks both shutdown limits and both startup count-loop bounds, so expected count four matches the four actual wait loops.
- This atomic correction changes only the checker count from seven to four. The lifecycle shell regression itself passed on both cfbf and 2f3b; AI docs on 2f3b `44284415671` PASS.

- Exact-head `9204e82...` Shadowsocks run `44284758611`: shell syntax, start status, process lifecycle runtime and adblock regression PASS; only the checker failed on an obsolete inline `ss_start_state` contract. This update checks the current wrapper delegation and ABSENT/UNKNOWN count semantics.
- Exact-head AI handoff run `44284758515` PASS. Corrected checker Actions on the new SHA are pending.

## Verification boundary
- No production source changes in this checker correction.
- Exact-head Actions for this new SHA must be checked. cfbf XY-C1 `44283616981` was still IN_PROGRESS at last observation; no current-source L3 claim. No physical-router L4.

## Exact first action for the next GPT
1. Fetch live `main` and inspect exact-head Shadowsocks, Process inventory and AI handoff workflows for this correction. Confirm the checker advances past the bounded-wait assertion and diagnose any next failure.
2. Poll cfbf XY-C1 run `44283616981` and inspect Build XY-C1, TRX verification and artifact upload individually.
3. Once validated, implement ChinaDNS-ng detached-process readiness and make `start_dns()` propagate launch errors; continue with dnsmasq restart/rollback as a separate slice.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and exact-head Action results before ending. The checkpoint is mandatory; a document edit is conditional. Leave the handoff untouched if it remains accurate.

# Cross-session execution mandate
Complete each logical source/test slice in one atomic main commit. Continue independent work while CI is PENDING. Before context exhaustion, leave a precise next action and tested/unverified boundaries. Never force-push, alter the frozen backup, reopen PR #1, or claim unperformed tests/builds.
