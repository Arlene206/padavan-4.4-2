# Cross-session execution mandate

Mandatory handoff checkpoint: Every session MUST inspect the live repository state before ending or handing off. Verify the source baseline, active task, relevant exact-head Actions, validation boundary, blockers, frozen backup and precise first action for the next session. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check and do not create a no-op commit.

Before context exhaustion, finish or reduce the current safe slice, run its regression checks, inspect exact-SHA Actions, commit completed work atomically, and preserve the exact next task.

Continue this repository as one continuous engineering effort across ChatGPT conversations. Recover once, do not repeat completed audits, do not wait for pending CI when independent work can proceed, and do not ask the user what to do next.

Every logical multi-file change must be published as one complete Git tree and one atomic main commit, parented to freshly fetched main, followed by a non-forced expected-SHA compare-and-swap. Never force-push, split a logical multi-file change into half-commits, use a work branch/PR, or modify the frozen backup branch.

## Current repository state

- Parent main: `987b3bc75261b8fed86ffd78c3bd58eb0ba79c80`. It includes generic firewall/UPnP result propagation through storage, IPv6, VPN server and the storage source-contract repair.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; immutable and read-only.
- On the previous validation head `f0294dee`, Process `43935227981` PASS and AI docs `43935227749` PASS; Storage hotplug `43935227891` failed at a stale count-7 assertion in `check_storage_safe_remove.py:302`. Commit `987b3bc` fixes that checker. On current parent `987b3bc75261b8fed86ffd78c3bd58eb0ba79c80`, Storage `43935659776`, Process `43935659401`, AI docs `43935659456`, and NVRAM `43935659452` are IN_PROGRESS at last poll.
- XY-C1 run `43934498984` is still associated with production SHA `cb0e8e5`; it was IN_PROGRESS at last poll. This follow-up changes VPN production source and must get a new exact-SHA XY-C1 Build/TRX/artifact result.
- PR #1 remains historical closed/unmerged work; do not resume it.

## Active task

The last direct `restart_firewall()` caller not yet returning its result is `restart_vpn_client_locked()`. This commit captures its firewall/UPnP refresh result, returns it when VPN startup succeeded, and preserves the first prior VPN-start or xl2tpd-restore failure including errno. VPN-server restart already returned the firewall result but did not snapshot errno when that was the first failure; this commit fixes that before later NFS/daemon restoration and lock release can clobber it.

The prior Process failure caused by a stale UPnP test-name assertion has been fixed in `f6d4192`; the subsequent Process run passed. The storage source-contract failure caused by its stale count-7 expectation has been fixed in `987b3bc`; exact-head Storage validation is now running. The canonical WAN/RC checker is extended to verify VPN client/server status and errno handling and inventory the total direct firewall references. Firewall Startup triggers now include `vpn_client.c`.

Next P1 task: audit `reload_nat_modules()` and `start_firewall_ex()`, distinguishing configured required modules from optional modules and tracing command/snapshot/restore results before API changes. The rule builder remains void; no claim is made that iptables rule application errors are propagated.

## Verification boundary

- On `f0294dee`, Process `43935227981` and AI docs `43935227749` passed. Storage hotplug `43935227891` failed only at its stale count-7 checker assertion; commit `987b3bc` updates it to require the eighth firewall-result aggregation and return ordering.
- On current parent `987b3bc75261b8fed86ffd78c3bd58eb0ba79c80`, Storage `43935659776`, Process `43935659401`, AI docs `43935659456` and NVRAM `43935659452` are in progress. Older XY-C1 `43934498984` applies to `cb0e8e5`, not this pending VPN source update.
- No full local firmware build or physical-device validation is claimed. Physical-router L4 remains NOT VERIFIED.
- `start_firewall_ex()` and `reload_nat_modules()` remain void result boundaries; internal rule/module failures remain open.
- Frozen backup remains unchanged.

## Exact first action for the next GPT

1. Inspect exact-head VPN caller commit Actions: Process, Firewall Startup, Storage hotplug, XY-C1 Build/TRX/artifact and AI docs. Diagnose failures from the first causal step; confirm the storage count-8 checker fix.
2. Continue the P1 audit at `reload_nat_modules()` and `start_firewall_ex()`, tracing full callers and configured/optional module gates before changing the APIs.
3. Keep main-only expected-SHA commits, leave the frozen backup read-only, and keep physical-router L4 NOT VERIFIED.
