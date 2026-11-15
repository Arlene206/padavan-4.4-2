# Cross-session execution mandate (read before resuming)

This is a continuing autonomous engineering assignment. Recover live state once, then resume the exact P1 task below. Do not repeat completed work, stop after a plan, or ask the user to select a task. Continue safe independent engineering while CI is pending.

**Mandatory handoff checkpoint:** Every session must verify before ending that this file accurately records the meaningful source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If any next-session-critical fact is stale or missing, update this handoff; if it remains accurate, do not create a no-op documentation commit.

For each multi-file logical slice, publish source + regression/source contract + necessary documentation as one atomic main commit: create all blobs, one tree, one commit parented to the freshly fetched main SHA, then update main non-forced using that expected SHA. Never create development branches or PRs, never force-push, and never touch the preserved archive branch.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; only development branch: main.
- Networkmap source baseline: e902e806f565e26d34406483ae2133c0b55acee2. A follow-up checker/handoff correction was committed as 7343fb3a798313ba8416df0741021efc6c536a84. Current watchdog slice is parented to that correction; resolve the resulting head SHA live.
- Frozen archive branch: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807. Read-only; preserve byte-for-byte.
- PR #1 is closed/unmerged and is not a development or validation target.
- There is no local repository checkout or target cross-toolchain available in this execution. Do not claim local full builds/tests.

## Active task

### P1 — Boot daemon startup error reporting

- Networkmap boot launch result is now captured/logged by `start_services_once()` on product-source SHA e902e806; the added `check_networkmap_signals.py` source contract was subsequently corrected to use a properly escaped raw regex in checker/handoff commit 7343fb3a.
- This slice adds missing launcher-failure logging inside `start_watchdog()`: UNKNOWN process inventory was already logged and RUNNING is a successful no-op, but a failed `eval("/sbin/watchdog")` previously returned silently. Return status is preserved. The watchdog workflow inline source contract and `scripts/check_process_lifecycle_integration.py` both assert this path.
- `start_gpio_btn()` was inspected: its UNKNOWN inventory, launcher failure, UNKNOWN startup and not-remain-running failures are already logged internally. Do not add duplicate boot logging.
- Next inspect `start_infosvr()` more closely. It returns 1 when `adsc_enable` is disabled, and its optional-daemon helper logs UNKNOWN inventory; only an enabled launcher failure appears to lack a specific diagnostic. Preserve disabled sentinel semantics. Then review remaining status-returning boot functions one at a time and avoid duplicate callee diagnostics.

### Completed scoped process-signal sweep

- Reviewed all 36 `trunk/user/rc/src/*.c` files for active `pids()`, `kill_pidfile_s()`, `killall`, and direct `kill()` patterns. Only active `pids()` occurrence is legacy `is_dns_dhcpd_run()` in `services_ex.c`, with no RC C-source caller. `kill_process_pidfile()` has no RC C-source caller. The `killall` occurrence is inside a disabled `#if 0` path. This is not a whole-repository/package audit.
- Deliberate PID 1 and child-process signal sites and identity-bound signals were left unchanged. Linux 4.4 PID/start_time validation-to-signal TOCTOU remains unresolved; another pre-signal recheck does not close it.
- DNSMasq/VPN same-process lifecycle lock-order review is complete. Do not repeat it.

## Verification boundary

- Networkmap correction commit 7343fb3a798313ba8416df0741021efc6c536a84 triggered Networkmap #43732137598 and AI handoff docs #43732137652; both were IN_PROGRESS at the last observation. Its other listed process-inventory, NVRAM, AdGuardHome, AliDDNS, infosvr and AliyunDrive workflows were also IN_PROGRESS. These are exact-head runs, not results.
- The prior Networkmap run #43731654772 on e902e806 FAILED in the Python source checker due the over-escaped regex, after both GNU89 `-Wall -Wextra -Werror` ASan/UBSan policy binaries passed. The AI docs run #43731654776 failed because the preceding handoff rewrite omitted the mandatory checkpoint and canonical headings. Both defects were corrected in 7343fb3a; exact-head reruns are required.
- XY-C1 #43731654808 is associated with product-source SHA e902e806 and was IN_PROGRESS at the previous check. This current watchdog.c product change should trigger a new XY-C1 run. Require Build, TRX verification and artifact upload on the current product-source SHA before claiming L3.
- No current MIPS/uClibc build, firmware artifact or hardware test is claimed. Physical-router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and resolve this watchdog source commit SHA. Check exact-head watchdog signal, process lifecycle, Networkmap, AI handoff and XY-C1 Actions. Diagnose any FAIL by first causal job log; do not wait idly on PENDING.
2. Continue with `start_infosvr()` in `services.c`: enabled launch error propagation is not logged at the function boundary, while disabled `adsc_enable != 1` returns 1 and must remain non-error. Add a focused semantic source contract in the relevant infosvr workflow and verify XY-C1 path filtering.
3. Then rotate to other service start callers, checking internal error logs and disabled sentinels first. Preserve best-effort boot behavior.

## Safety and continuity invariants

- A pending Action is not a gate on independent work. Record its exact run ID/SHA and continue other safe work.
- Preserve legacy behavior, NVRAM/configuration compatibility, target architecture and all board support. Never weaken tests to green CI.
- FTP failed-upload unlink TOCTOU and NFS durability/AUTH_SYS/root-squash remain separately unverified; do not claim resolution.
- Before context exhaustion, complete a safe slice and update this handoff only when active task, source baseline, validation state or exact next action materially changes.

Preserve `docs/dev-log/` archive shards and `docs/dev-log/INDEX.md` byte-for-byte.