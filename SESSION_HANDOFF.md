# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state and compare it with `SESSION_HANDOFF.md` before ending or handing off. Verify the engineering/source baseline, active task, relevant exact-head Actions, validation boundary, blockers and exact first action. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, leave the file untouched and make no no-op commit.

Before context exhaustion, finish or reduce the current safe slice, check exact-head Actions, commit any completed work atomically and preserve the precise first next action here.

Continue this engineering task across ChatGPT conversations. Recover repository state once, do not repeat completed audits, and do not ask the user what to do next.

For each logical task, create all changed files in one Git tree and one atomic main commit parented to freshly fetched main; update main with exact expected SHA using non-forced compare-and-swap. Never force-push, create partial commits, or touch the frozen backup. Record exact Action results and the next concrete task before context exhaustion.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; active branch: main.
- Parent for THIS WAN restart VLAN-status slice: `0828b3a8acf119345bc2381f89772f213288862d`; fetch live main and use expected-SHA non-forced CAS on the next write.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` = `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable. One-time startup comparison found main 588 commits ahead / 1 behind; do not repeat backup/archive recovery.
- Latest source milestone `b715097b8fdb76ed1e96bdeed6bedafde47eeefd` verifies required iNIC MII module load and reports optional hw_nat offload accurately. Latest checker/handoff correction is `0828b3a8acf119345bc2381f89772f213288862d`.

## Active task

Call-site tracing after changing `switch_config_vlan()` to return status found `full_restart_wan()` still discarded its result. A failed VLAN-rule or switch bridge-mode ioctl could therefore be followed by WAN restart on incomplete switch state. THIS slice now captures the status, logs and aborts WAN restart before `start_wan()` when required switch VLAN reconfiguration fails. Process workflow adds `net_wan.c` to source triggers and checks fail-closed ordering.

Earlier slices remain:
- `init_bridge()` propagates essential bridge/VLAN, switch base/storm/link, interface/member and required iNIC MII module errors into PID1.
- `switch_config_base/storm/link()` attempt independent settings and preserve first failure; optional LED and `hw_nat` offload remain non-fatal.
- Wireless module post-load verification is reused because `module_smart_load()` returning zero is not authoritative.

## Verification boundary

- Exact-head Process run `43858811934` on `59047eb21838e0cf78dd24e0a6eab78bea7fc886` compiled and passed bridge-init, bridge membership, wireless module post-load and other policy regressions, but its final source contract failed due to a false-positive negative substring assertion. Exact failure was fixed in `0828b3a8acf119345bc2381f89772f213288862d` by comparing complete stripped lines.
- Exact-head AI docs run `43858811955` on `59047eb21838e0cf78dd24e0a6eab78bea7fc886` failed because the handoff omitted the required literal `Before context exhaustion`; the handoff contract was corrected in `0828b3a8acf119345bc2381f89772f213288862d`.
- On `0828b3a8acf119345bc2381f89772f213288862d`, Process `43859776339` and AI docs `43859776315` were IN_PROGRESS. Other NVRAM/infosvr/AdGuard/transport tests were also IN_PROGRESS. Recheck their exact status; do not infer pass.
- THIS WAN restart call-site fix needs fresh Process / XY-C1 Actions. No same-SHA firmware Build+TRX+artifact L3 or physical router L4 result is claimed.

## Exact first action for the next GPT

1. Inspect exact-head Process and AI docs reruns, plus XY-C1, NVRAM, HTTPD security and relevant service Actions on THIS commit.
2. Verify `full_restart_wan()` returns before any `start_wan()` when switch VLAN configuration fails.
3. Continue the RC/driver call-site scan for ignored newly-introduced status-returning functions and unchecked required modules.
4. XY-C1 L3 requires Build, TRX validation and artifact upload on the same production-source SHA; physical hardware remains L4-only.

## Verification and governance rules

Preserve security, architecture, compatibility, target toolchain and required tests. Main only; frozen backup read-only. One full Git tree and atomic commit per logical task; non-forced expected-SHA update. Update `dev_log.md` for source milestones and retain 20–119 entries. Distinguish host regression, exact-SHA Actions, firmware/TRX L3 and physical router L4. Never claim unrun results.
