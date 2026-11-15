# Cross-session execution mandate

Recover live repository state once, then execute the active engineering task below. Do not repeat completed audits, remain at planning, wait on PENDING CI when independent work is available, or ask the user what to do next.

For each logical task, create all source/test/doc blobs, one complete Git tree and one commit parented to freshly fetched main; update main with exact expected SHA and force:false. Use one atomic main commit, never partial multi-file commits or force-push. Preserve frozen backup and archived log shards/index.

**Mandatory handoff checkpoint:** The checkpoint is mandatory; a document edit is conditional. Every session records exact main/backup SHAs, source changes, exact-SHA Action IDs/statuses, validation boundary and first next action before ending. Do not create a no-op handoff commit when the checkpoint is accurate.

## Current repository state

- Repository: Arlene206/padavan-4.4-2.
- Main SHA at start of this handoff checkpoint: 13a16ec70f7f78f9a8114582a51e7d9e88bd9644.
- Frozen backup branch backup/pre-plugin-overhaul-2026-11-14: 72f4f157a084f8184b9597ca5bf334b8080e5807; read-only.
- Recent main commits:
- 13a16ec70f7f fix(storage): isolate concurrent Aria config staging
- 6ad669a672f8 fix(storage): use unique Aria config staging file
- 755e8c711f1b fix(storage): stage Aria config before daemon launch
- eddf9aac4d44 fix(storage): propagate Transmission and Aria start failures
- 401066ebf04c fix(wan): declare MAN route helper before use
- b1b8991aaba1 fix(wan): abort startup on failed WISP migration
- 3f6ca487db97 test(wan): align route workflows with MAN status API
- 41ea56088a7c fix(wan): propagate required MAN gateway route failures
- 2529f9a8aadd fix(wan): guard AP-client WISP interface migration
- 7ebeaa9e02e6 fix(wan): gate protocol startup on interface readiness
- 20f1bbd6eb13 fix(wan): fail closed on static IPoE address error
- 454f3869fc3c test(wan): scope physical WAN abort ordering
- WAN firmware source SHA 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6 exact XY-C1 #43756373315 Build/TRX/artifact SUCCESS (L3 for that exact C source SHA). No physical router/L4 result.
- WAN source work includes direct IPoE static IPv4 fail-closed address application, checked initial physical WAN bring-up, safe AP-client WISP cleanup and required MAN gateway/default-route error separation for static PPTP/L2TP. Optional configured static/DHCP/DNS routes remain aggregate-only; DHCP/ZeroConf aggregate semantics and PPPoE async/no-wait are preserved.
- Storage startup work: Transmission and Aria return actual start-stop-daemon status independently from logger; Aria checks directory/session/download preparation and ownership/permissions, writes default config to PID-qualified same-directory temp and atomically renames after successful write; Transmission stages default settings in PID-qualified same-directory temp and checks generator, sed transform, mv and final nonempty file before daemon launch. Regression tests inject launch, logger, write, transform, rename and successful publication paths under host shell and BusyBox ash.
- Exact current-main Actions:
- #43760834673 AI handoff documentation contract: completed/success
- #43760834671 AliDDNS signed API transport security regression: completed/success
- #43760834666 Process lifecycle inventory - 13a16ec70f7f78f9a8114582a51e7d9e88bd9644: completed/success
- #43760834648 USB storage hotplug stability regression: completed/failure
- #43760834637 NVRAM transaction boundary audit: completed/success
- #43760834634 AliyunDrive WebDAV lifecycle stability regression: completed/success
- #43760834624 Process inventory and lifecycle fail-closed regression: completed/success
- #43760834623 infosvr packet and signal lifecycle stability regression: completed/success
- #43760834594 AdGuardHome config security regression: completed/success
- Current storage workflow state: #43760834648 USB storage hotplug stability regression: completed/failure.
- Frozen backup, docs/dev-log/INDEX.md and archive shards remain untouched. No L4 claim.

## Active task

### Validate storage launcher startup and configuration publication

The immediate next action is to inspect exact-current-main storage-hotplug workflow jobs and logs. Confirm the Transmission/Aria status, staged config and failure-injection tests pass under both host shell and BusyBox ash. If FAIL, inspect the full failing log and implement the narrow source/test correction. If PENDING, progress an independent module issue instead of waiting.

After storage validation, inspect remaining Transmission settings and directory/permission/ownership failure paths as distinct atomic slices. Do not repeat the already-completed WAN audit unless a specific exact-SHA check fails.

## Verification boundary

- Main SHA before this handoff-only checkpoint commit: 13a16ec70f7f78f9a8114582a51e7d9e88bd9644.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.
- XY-C1 L3 is established for source SHA 401066ebf04c50aaeb1ef47c36feb2ec210a0bc6 only. New changes are shell/user-space only; no physical-router L4 evidence.
- Current-main exact Action IDs/statuses are listed above. No result is transferred from a different SHA.
- Newest storage checks are represented by #43760834648 USB storage hotplug stability regression: completed/failure.

## Exact first action for the next GPT

1. Fetch main and Actions; inspect exact-current-SHA storage-hotplug jobs and full logs. Confirm host and firmware BusyBox ash cases.
2. Correct any failing source or regression test immediately with one atomic commit; on PENDING, move to another independent stability/security issue.
3. Verify Transmission/Aria config and directory preparation before allowing daemon start; do not infer successful startup from logger or echo status.
4. Preserve frozen backup, archived log index/shards, and exact L3/L4 boundary.
