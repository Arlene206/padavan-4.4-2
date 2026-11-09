# Current Session Handoff — Padavan 4.4

> One actionable checkpoint. Existing historical snapshots remain immutable and indexed. Always fetch live main, all branches and exact-head Actions on takeover.

## Current repository state

- Maintained fork: `Arlene206/padavan-4.4-2`; do not substitute the upstream repository.
- Live `main` baseline for this workflow-contract correction: `eb21523750bc3218121f5adf3457c587d8e9a6e0`; active branch: `main`; no separate checkpoint branch exists.
- Previous source commit `eb21523750bc3218121f5adf3457c587d8e9a6e0` contains NFS exact source/mountpoint/filesystem tuple validation, foreign/stacked mount rejection, post-mount verification and guarded teardown.
- Exact-head GitHub Actions for `eb21523750bc3218121f5adf3457c587d8e9a6e0`:
  - NFS daemon start/reload [42622521213](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521213): **SUCCESS**; shell syntax, runtime fixture, IPv6 ACL GNU89 regression and RC caller/path-filter checks passed.
  - AI handoff documentation [42622521280](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521280): **SUCCESS**; current handoff/roadmap, root log and archive validation passed.
  - USB storage hotplug [42622521225](https://github.com/Arlene206/padavan-4.4-2/actions/runs/42622521225): **FAILURE** in the final whole-caller-graph Python source contract, after all preceding shell and GNU89 regression steps passed. The assertion still expected the former numeric mount-state branches and did not match the new `case`-based fail-closed unmount helper. The run's full `head_sha` exactly matched `eb21523750bc3218121f5adf3457c587d8e9a6e0`.
- No remote checkpoint branch is pending. The NFS mount tuple implementation is complete at source and its focused exact-head NFS workflow is green; the cross-workflow storage assertion is the only currently identified blocker.

## Active task

### Align storage caller-graph regression with NFS tri-state mount teardown

P2 CI integration regression. The production NFS helper now returns 0 for exactly one expected source/mountpoint/filesystem tuple, 1 for an unused path, and 2 for conflict or inventory error. `func_unmount_if_mounted()` uses a `case` statement to fail closed on conflict before unmount, then rechecks that the mountpoint is absent. The USB storage workflow still asserted the old pair of numeric `if` conditions, so its final caller-graph contract failed even though the NFS runtime and shell regressions passed.

### Implementation in THIS commit

- Update only the stale storage workflow source contract; production NFS and storage C/shell behavior remain unchanged.
- Assert the explicit mount-state `case`, conflict refusal, exact tuple inventory call, preflight-before-unmount ordering, checked `umount`, and post-unmount absent-state verification.

### Files changed

- `.github/workflows/storage-hotplug-stability.yml`
- `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, `dev_log.md`

### Verification boundary

| Level | Status |
|---|---|
| L1 source review | PASS for matching the storage source contract to the current NFS exact-tuple / fail-closed unmount semantics; no production behavior is changed. |
| L2 host / Actions | Base `eb21523750bc3218121f5adf3457c587d8e9a6e0`: NFS run 42622521213 **SUCCESS**, AI docs run 42622521280 **SUCCESS**, USB storage run 42622521225 **FAILURE** at its final stale source-contract assertion. This correction's exact-head storage/docs runs are pending. |
| L3 MIPS/uClibc / production RC object / full firmware/TRX | NOT VERIFIED. |
| L4 NFS mount-stack behavior on router hardware, storage eject and physical-router lifecycle | NOT VERIFIED. |

The failed storage workflow is a source-contract mismatch, not a failed production compile/runtime test: all earlier steps in that job passed. The corrected contract must be confirmed by a new exact-head run. No local full checkout, local execution of the Python workflow contract, production RC build, MIPS/uClibc build, complete firmware/TRX or physical-router test is claimed.

## Exact first action for the next GPT

1. Fetch the live `main` SHA and exact-head Actions. Inspect the new USB storage caller-graph workflow and AI handoff-doc workflow; both must pass on the same exact resulting SHA.
2. If the storage source contract still fails, inspect the new traceback and adjust only a demonstrably stale assertion; do not weaken the actual exact mount tuple or conflict-safe unmount guarantees.
3. Once green, resume the NFS export permission/security audit: `no_root_squash`, AUTH_SYS UID/GID mapping, `async` durability, `insecure` client source ports, custom exports override semantics and external-mount support. Preserve historic behavior until compatibility impact is demonstrated.
4. Then rotate to Samba passdb/account mutation, ACL/permission and credential security. Keep vsftpd authentication/path confinement, DHCP leasefail/NAK/release callback state, FULLCONENAT queue namespace ownership and cross-module shutdown races open. L3/L4 remain unverified until actual target build and device tests.

## Documentation maintenance

- `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md` are live single-copy documents. Update in place only when their checkpoint, evidence, priorities or next action materially changes; use Git history for prior versions.
- `dev_log.md` accumulates newest-first entries from 20 to 119. When the next entry would make 120, archive the oldest 100 complete entries verbatim and retain the newest 20. If the 384 KiB target is exceeded earlier, batch-archive complete old entries while retaining the newest 20. Hard limit: 512 KiB.
- Existing `docs/handoffs/archive/` and `docs/roadmap-history/` snapshots and indexes are legacy history. Keep them byte-for-byte intact.
- Do not mechanically update all documents or create docs-only commits merely to record completed CI. Update only the records whose facts changed.
- Run `python3 scripts/check_ai_handoff_docs.py` when a full local checkout is available. GitHub Actions is authoritative for the exact pushed SHA.
