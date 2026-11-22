# Cross-session execution mandate

Continue repository development across GPT conversations. Recover once, then implement. Do not repeat completed audits or ask the user what to do next.

- One logical slice means source change, regression coverage, one atomic main commit, and exact-SHA Actions inspection.
- Use Git data APIs to create all blobs, one tree and one commit. Update `main` with non-forced compare-and-swap against the expected parent SHA. Never force-push or expose a multi-file half-state.
- Before context exhaustion, inspect live main and backup refs, exact-head Actions and tests, then keep this handoff truthful about the first next task.

## Mandatory handoff checkpoint

Every session MUST inspect live repository state before ending, including `main` SHA, backup SHA, relevant exact-head Actions, test facts and the first next task.

The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Current repository state

- Repository: Arlene206/padavan-4.4-2; branch: main; current source commit: THIS COMMIT (parent 5f00e014e746562927ffe73b75db82333c170300).
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains at `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable.
- libdisk now uses checked directory enumeration and folder snapshots, bounded metadata reads and account paths, atomic permission-file publication, fail-closed folder/account initialization, and first-error propagation for folder delete/rename and account add/delete operations.
- add_account() propagates every mounted-volume permission initializer failure after attempting all volumes and reloading Samba; the existing NVRAM ordering is retained.
- del_account() frees the complete original account list, uses bounded metadata paths, ignores only missing-file unlink errors, continues through all mounted volumes, and reports the first cleanup failure after ftpsamba reload.
- mod_account() now formats both old/new per-volume metadata paths with the bounded formatter; ignores only missing old files, aggregates other rename/path errors across volumes, frees disk inventory and reloads ftpsamba before returning the first failure. The existing NVRAM commit ordering is unchanged.

## Active task

- Next: inspect remaining libdisk account bootstrap/removal call sites and the NVRAM status contract for ignored errors. Then broaden into WAN/service lifecycle stability rather than remaining only in libdisk.
- Do not redesign NVRAM transaction/rollback semantics without inspecting the NVRAM API and current regression contract.

## Verification boundary

- Parent exact-head libdisk run #45019979218, AI handoff #45019979195, HTTPD #45019979197, Process inventory #45019979224, NVRAM audit #45019979154 and XY-C1 #45019979256 were in progress at last inspection. Fresh exact-head Actions for this commit are required.
- Earlier parent #45019873074 passed libdisk sanitizers and source contracts; #45019873116 passed the AI handoff contract. Exact-head status does not transfer to a later SHA.
- XY-C1 L3 is NOT VERIFIED until Build XY-C1, TRX verification and artifact upload all pass on one identical product-source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head libdisk, handoff, Process, HTTPD, NVRAM and XY-C1 Actions for this commit; fix causal failures from logs.
2. Review remaining account lifecycle/NVRAM API call sites, then select a WAN/service lifecycle stability defect outside libdisk and add a focused regression.
3. Keep the backup branch immutable. Claim L3 only after same-SHA Build, TRX and artifact upload PASS.

## Documentation anti-churn guard

Recovery is read-only by default. Never commit merely because a GPT has taken over. This is not a ban on useful documentation-only work. If the current handoff remains accurate, leave it untouched.

