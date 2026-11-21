# Cross-session execution mandate — 2026-11-21 checkpoint

## Cross-session execution mandate

- Continue development on `main` only. Every logical change is one atomic main commit: create every blob, one complete Git tree, one commit, then update main without force using the exact expected parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Add focused regression coverage for source fixes; do not weaken checks.
- PENDING Actions do not block independent work. Diagnose the earliest causal failure log.
- Before context exhaustion, finish the current safe slice, publish atomically and leave the exact first next action here.

## Mandatory handoff checkpoint — end-of-session accuracy

The checkpoint is mandatory; a document edit is conditional. At every session end, compare live `main`, relevant exact-SHA Actions, the product-source baseline, verification boundary, blockers and the first next action with this record. Update it only when one of those resumable facts materially changes; do not create a no-op commit.

## Current repository state

- Live `main` before this Optware ownership fix: `94c0e2c11f6e73b90366ed6e74cb292858acb6e7`. Latest shared-source product baseline: `20e139d230b73eabc5f25a2b17d56a3a5768af11`.
- Frozen archival branch `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807` (read-only).
- Process lifecycle regression `#44887824016` and AI handoff `#44887824043` passed on exact SHA `94c0e2c`; the legacy shared PID-file parser integration is validated.
- XY-C1 run `#44887656164` completed SUCCESS on exact product-source SHA `20e139d230b73eabc5f25a2b17d56a3a5768af11`: Build XY-C1, Verify TRX output and artifact upload all passed. Firmware build L3 is VERIFIED for this source SHA; physical-router L4 remains NOT VERIFIED.
- The exported `kill_pidfile_s()` compatibility helper now strictly validates numeric PID-only records without changing the on-disk format. A full tracked-text inventory found no external direct callers; `kill_pidfile()` wrapper is the only internal call site, so retain the exported helper for unknown external consumers.
- New bug found in `trunk/user/optware/opt-umount.sh`: unquoted `grep ' /opt ' /proc/mounts | grep $1` was substring matching and ignored the supplied mountpoint. A request for `/dev/sda1` could match `/dev/sda10`, stopping Optware while ejecting a different partition. This patch uses exact source+mountpoint fields in a validated six-field mount inventory and fails closed on malformed, duplicate or unavailable inventory.

## Active task

1. Validate the Optware ownership fix with exact-head Storage Hotplug Actions: shell syntax, POSIX /bin/sh, BusyBox ash teardown tests, source assertions and standalone checker.
2. Fix the earliest causal CI failure without weakening exact source/mountpoint coverage; while Actions run, continue independent userland stability/security tasks.
3. Keep XY-C1 L3 attached to exact source SHA `20e139d2`; physical-router L4 remains unverified.

## Verification boundary

- Shared PID-only parser hardening and its tests are merged at product-source SHA `20e139d2`; Process integration passed at `94c0e2c`.
- XY-C1 build, TRX verification and artifact upload all passed on exact product-source SHA `20e139d2`; L3 is VERIFIED for that source SHA.
- Optware mount matching source/test/checker changes require exact-head Storage Hotplug Actions before being treated as validated.
- Physical-router L4 is NOT VERIFIED.

## Exact first action for the next GPT

Fetch live `main` and verify frozen backup SHA `72f4f157a084f8184b9597ca5bf334b8080e5807`. Inspect exact-head Storage Hotplug Actions for this Optware patch, fix the earliest causal failure without weakening checks, then continue independent stability/security tasks outside completed audits.

## Handoff maintenance

- Keep `dev_log.md` newest-first; archive only at 120 root records, retaining newest 20.
- Update `REFACTOR_ROADMAP.md` only when module status, evidence or priority materially changes.
- Keep frozen backup permanently unchanged.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate evidence levels.
