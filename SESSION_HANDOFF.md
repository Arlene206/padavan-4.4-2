# Cross-session execution mandate

## Execution policy

Continue main-only implementation across conversations. Restore exact live source and CI state once, then resume concrete engineering tasks. Use one atomic main commit per logical task. Do not repeat completed audits, create status-only commits or stop at planning.

- Create all changed blobs, one complete Git tree and one commit with current main as parent; update main using non-forced compare-and-swap with that exact expected SHA.
- Verify Actions by exact `head_sha`. Pending runs do not block independent work; inspect and repair real failures.
- Preserve security, architecture, GNU89/MIPS/uClibc compatibility, regression coverage, lifecycle identity checks, bounded I/O and fail-closed UNKNOWN handling.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` is archival only; never alter it. Develop on main.

## Current repository state

- Current main before this CI-contract correction: `098a9d4bec3fed1cba85c1427ca7e346d52b31ce` (`fix(upnp): validate contiguous LAN netmask`), parent `5e2c7a3fd8479bd1cdbdf455b34c18d20494e1b7`.
- Exact-head MiniUPnP atomic configuration regression `44638505959`, Process lifecycle `44638506449`, WAN DHCP `44638506004`, WAN route `44638505955`, Samba `44638505951`, DNSMasq `44638505950`, and Inadyn `44638506051` PASS. Process umbrella `44638505913`, HTTPD `44638506210`, FULLCONENAT `44638506104`, NVRAM `44638506292`, and XY-C1 `44638506287` were still IN_PROGRESS at last poll.
- Firewall Startup run `44638506108` failed only in its source contract: it expected the extracted helper's function body to remain in `firewall_filter_policy.h`, but the commit correctly moved it to `ipv4_netmask_policy.h`. This commit repairs the contract to verify the include and the new helper definition.
- AI docs run `44638506375` failed because `SESSION_HANDOFF.md` omitted the required literal `Before context exhaustion`; this commit restores the section. The LAN-mask source change is already committed; no physical-router result is available.

## Active task

- The MiniUPnP LAN netmask gap is fixed on `098a9d4`: shared `ipv4_netmask_is_contiguous()` is called by both firewall and miniupnpd; the latter rejects invalid/non-contiguous masks before `ip2class()`. MiniUPnP exact-head workflow `44638505959` PASS.
- The Firewall Startup checker was stale after moving the predicate implementation into `ipv4_netmask_policy.h`: it still searched for the implementation body inside `firewall_filter_policy.h`. Updated it to assert the include relationship and helper definition, without weakening the runtime regression.
- Restored the exact AI handoff-checker heading `Before context exhaustion` and its atomic-commit/exact-SHA verification instructions. No source/runtime behavior changes in this follow-up.

## Exact first action for the next GPT

1. Check exact-head Firewall Startup and AI docs results for this correction; inspect first causal failure logs if either remains red.
2. Recheck `098a9d4` pending Process umbrella, HTTPD, FULLCONENAT, NVRAM and XY-C1 runs. XY-C1 L3 requires Build, TRX verification and artifact upload all PASS on the same production-source SHA.
3. Continue the P1 RC startup-result propagation investigation. Preserve PID 1 recovery semantics: distinguish mandatory router readiness from optional-service degraded status before changing `init_router()` return behavior.

## Verification boundary

- L1: shared nonzero contiguous IPv4 mask policy is used by firewall and MiniUPnP; MiniUPnP rejects malformed/zero/non-contiguous mask before subnet derivation.
- L2: MiniUPnP config suite `44638505959` and Process lifecycle suite `44638506449` PASS on exact SHA `098a9d4`. Firewall checker and AI docs needed source-contract/handoff-section repairs; their corrected exact-head runs are pending.
- L3: XY-C1 `44638506287` on `098a9d4` was in progress; Build/TRX/artifact must all pass on one exact source SHA.
- L4: actual MIPS/uClibc and physical-router LAN/UPnP behavior remain NOT VERIFIED.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST perform a handoff checkpoint before ending. At session end, compare live main, production-source baseline, exact-head Actions, module state and the next task. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit; do not create a no-op commit.

## Before context exhaustion

- Finish the current safe source/test slice and commit it atomically before handoff; do not publish related multi-file changes as sequential partial commits.
- Re-fetch live `main`, create one complete Git tree and one commit with that exact parent, then update main only with `force=false` and the exact parent as `expected_sha`. Never force-push.
- Query relevant Actions by exact source SHA and record PASS/FAIL/PENDING with run IDs. Pending validation does not block independent work and must not be reported as complete.
- Record the production-source baseline, test commands/results, exact CI state, unresolved limitations and the next concrete task. Distinguish host L2, firmware L3 and device L4 evidence.
- Confirm all work is committed and remotely present before claiming it is preserved across conversations.

## Development invariants

- Main-only; never alter the frozen backup or develop on a legacy PR.
- One complete tree and one atomic commit per logical task; `force=false`, exact parent `expected_sha`.
- Never force-push, rewrite history, publish partial commits or weaken tests.
- Keep L1/L2/L3/L4 distinct. Host tests are not firmware or hardware validation.
