# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state and complete an end-of-session accuracy checkpoint. Record exact main SHA, exact-SHA Actions, test/build boundaries and the next precise task. If facts remain accurate, explicitly conclude with no file change and no no-op commit. Use one atomic main commit for material changes only. Before context exhaustion, leave an accurate handoff and complete any safe bounded engineering slice.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete Git tree and one atomic main commit; update `main` only with `force:false` and a freshly fetched expected SHA.
- PASS continues, FAIL requires diagnosis/correction, PENDING does not block independent work.
- Do not claim local tests, production builds, TRX/artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; parent main SHA: `26d91277af7c0983a4a031f280bdef72ed4010ba`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and untouched.
- Recent work includes EAPoL orphan-tail cleanup, p910nd transient I/O handling, checked iNIC TxPower preflight, AliyunDrive process/HTTP readiness and AliDDNS NUL-delimited configuration fingerprint. See exact-SHA entries in `dev_log.md` for validation boundaries.
- This slice adds opt-in `nvram get_checked` (legacy `get` unchanged) and makes all AliyunDrive startup setting reads fail closed on backend errors.

## Active task

Validate the checked NVRAM CLI and AliyunDrive startup hardening. A failed `aliyundrive_enable` read must not be treated as disabled or stop a running service. Failure reading any subsequent setting must abort before workdir hardening or daemon launch.

## Verification boundary

- Exact parent `26d91277af7c0983a4a031f280bdef72ed4010ba`: AliDDNS signed API #45044076046 PASS; AI handoff #45044076040 PASS.
- AliyunDrive ancestor `72ffcb355cff70e8dc7b54f708b3806e3eac7e66` passed shell syntax, QEMU binary compatibility and host lifecycle but failed a source checker ordering assertion; later main history corrected it. Re-run on this source SHA.
- No local C/shell test, target build, TRX/artifact or hardware validation was performed for this slice.
- L3 remains NOT VERIFIED until XY-C1 Build, TRX validation and artifact upload all PASS on one identical production source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch exact-head NVRAM CLI, NVRAM transaction-boundary and AliyunDrive lifecycle Actions; inspect every job step and diagnose any failure.
2. Inspect XY-C1 Build, TRX and artifact upload for the latest production source SHA; do not reuse ancestor results.
3. Continue another independent confirmed userland stability/security issue while Actions are pending. Keep the frozen backup read-only and develop only on main.
