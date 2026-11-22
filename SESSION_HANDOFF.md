# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, do not create no-op commits. Use one atomic main commit for material changes only.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a fresh expected SHA.
- Check Actions for exact SHA; PASS continues, FAIL diagnose/fix, PENDING work other items. No unsupported test/build claims.

## Current repository state

- Repository `Arlene206/padavan-4.4-2`; parent main SHA `f8176758f1c9ec1e96381da1208be1edf0425fae`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- Wireless TxPower checked NVRAM preflight is in main; its corrected exact Wireless regression #45042323539 and AI #45042323562 passed on `2144925dd276279d7113dd1e5c16d7855202bcde`.
- AliyunDrive uses bounded HTTP readiness; the escaped shell expansions were fixed in `4b01a3e8638bb0fd94f7ca7f38d5556bb61a2c27`.

## Active task

Correct the AliyunDrive HTTP-readiness regression harness. Parent #45042972255 exposed `marker=absent probes=0 sleeps=10`: the no-listener fixture tried to start an asynchronous mock but pidof never observed its marker, so the HTTP logic was never exercised. The new tests set the process-present mock directly and verify unavailable vs delayed HTTP readiness without duplicate spawn/stop side effects.

## Verification boundary

- Parent `f8176758f1c9ec1e96381da1208be1edf0425fae`: AliyunDrive #45042972255 FAIL in no-listener test fixture; QEMU binary --version/--help compatibility passed, source contract skipped. Process inventory #45042972250 PASS and AI handoff #45042972322 PASS.
- The earlier ancestor HTTP parameter expansion issue is fixed in `4b01a3e8638bb0fd94f7ca7f38d5556bb61a2c27`; no local shell tests were run.
- No same-SHA XY-C1 L3 evidence. L3 requires Build, TRX and artifact upload all PASS on one identical SHA; hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Check exact-head AliyunDrive lifecycle, Process and AI Actions for the fixture correction; inspect Build XY-C1 only if triggered on that SHA.
2. If the HTTP tests pass, continue independent userland stability/security work; do not revisit completed audits.
