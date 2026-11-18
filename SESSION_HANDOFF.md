# 2026-11-18 continuation checkpoint — synchronize process-count wait contracts

## Current repository state
- Repository: `Arlene206/padavan-4.4-2`; active branch: `main`. The ABSENT-under-errexit source correction is cfbf00f9d9100daf82017403e63342658bbac29e; this follow-up synchronizes its source-contract checker with the two new bounded count-readiness loops.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 is closed/unmerged; do not reopen it.
- XY-C1 run `44282353140` targets product source SHA `e20cabfd833df25b69b7ce1e164faad09769501d` and was still IN_PROGRESS at the last poll; prerequisite checks passed, but Build XY-C1, TRX verification and artifact upload had not yet completed. This correction must have its own exact-SHA product build before current-source L3 can be claimed. Physical-router L4 remains NOT VERIFIED.

## Active task
- Shadowsocks `run_bin()` verifies a bounded PID count increase; the Lua generator is monitored through its actual long-lived `ipt2socks` child. TCP/UDP/local launchers propagate failures. `start_watchcat()` now verifies a new `ss-monitor` PID using validated `pgrep -f` count+1, skips duplicate launches, and propagates scheduling/readiness errors.
- Exact-head e20 Shadowsocks regression `44282353324` failed at `Lua-generated ipt2socks readiness status=1 expected=0`. A POSIX `/bin/sh` reproduction isolated the cause: all process inventory helpers assigned `pidof`/`pgrep` output directly before inspecting status. Since exit 1 is the documented ABSENT state, caller `set -e` could abort the nested command substitution before the helper normalized ABSENT to count zero. The 100-ms mock scheduler yield did not fix this deterministic status-handling bug.
- THIS source/test correction captures each of the four `pidof`/`pgrep` lookups inside `if` assignments, preserving RUNNING/ABSENT/UNKNOWN under `set -e`; tests now explicitly require both absent count functions to return zero. The Python checker pins conditional capture.
- On exact e20 SHA, AI handoff `44282353220` PASS; Process inventory `44282353174` and umbrella fail-closed regression `44282353250` PASS; Shadowsocks lifecycle `44282353324` FAIL at the Lua child assertion. On exact cfbf SHA, host process regression 44283617129, process inventory 44283616977, and AI docs 44283617094 PASS. Shadowsocks lifecycle 44283617060 failed only in its Python source checker: it still expected five sleep guards although the two new PID-count wait loops correctly add two more, for seven total. The full shell process regression itself passed.

## Verification boundary
- A local POSIX `/bin/sh` reproduction of the exact helper/launch pattern failed 10/10 times with direct assignment of absent `pidof` result and passed 20/20 times after conditional status capture. This is a targeted reproduction, not a complete repository test run.
- The current test fixture yields 100 ms per poll; the original Lua wait failure is fixed by preserving exit-1 ABSENT semantics under set -e. This follow-up only synchronizes the source checker.
- The cfbf XY-C1 run 44283616981 was IN_PROGRESS at last observation for exact source SHA cfbf. This checker-only follow-up is not a firmware-input change. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live main and inspect exact-head Shadowsocks lifecycle, Process inventory and AI handoff Actions for this checker correction; repair any FAIL.
2. Check cfbf XY-C1 run 44283616981 for Build/TRX/artifact on exact source SHA cfbf. Do not claim L3 if this run lacks any stage.
3. Continue bounded readiness and rollback for the ChinaDNS-ng plus dnsmasq configuration/restart path; then rotate to broader RC/userland P1 reliability.

## Mandatory handoff checkpoint
Every session MUST inspect live repository state and exact-head Action results before ending. The checkpoint is mandatory; a document edit is conditional. If those facts are already accurate, leave the handoff untouched and do not create a no-op commit.

# Cross-session execution mandate
Complete each logical source/test slice in one atomic main commit on `main`. Continue independent work while CI is PENDING. Before context exhaustion, leave the next GPT a precise first action and tested/unverified boundaries. Never force-push, mutate the frozen backup, reopen PR #1 or claim unperformed tests/builds.
