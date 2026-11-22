# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state and complete an end-of-session accuracy checkpoint before ending. Record current source SHA, exact-SHA Actions, test/build boundaries and the next precise task. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no commit. Make one atomic main commit for a material checkpoint change; never create a no-op commit. Before context exhaustion, leave the next GPT an accurate first action.

- Work only on `main`; never force-push, use side branches/PRs, or modify the frozen backup.
- Each logical slice creates all blobs, one complete tree and one commit, then updates main with `force:false` and a fetched expected SHA.
- Check Actions for the exact source SHA. PASS continues; FAIL requires diagnosis and correction; PENDING does not block independent work.
- Do not claim local tests, target builds, artifacts or hardware checks that were not performed.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch `main`.
- Parent of the current modem source-contract correction: `a624157703da1feabe5838db691b4b28e4d68328`. Fetch live main after the commit to get the current SHA.
- Immutable backup remains `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; PR #1 is closed/unmerged.
- WAN PPP launch mode uses one checked NVRAM transaction snapshot for multi-WAN and APP_RPL2TP L2TP selection. The locked worker accepts that immutable result for stop, start and rollback.
- The router-hosts IPv6 ordering checker now distinguishes address preflight from emission; exact run #45031159848 on ancestor `1e6238117d62087ea9eb5e99384b3ad2c65d1511` PASS.
- The LAN DHCP hosts source contract now validates `get_lan_addr6_host_status()` failure-before-stage behavior; WAN DHCP exact run #45031431084 on parent `a624157703da1feabe5838db691b4b28e4d68328` PASS.

## Active task

The shared modem/USB/firewall/process/XY checker was aligned with the production `reload_lan_addr6()` checked snapshot API but its first exact run failed because the new checker compared a full call string after whitespace normalization. This correction matches the call by stable function tokens and validates the key and callbacks separately. Product code remains unchanged.

## Verification boundary

- Local tests were not run in this connector-only session.
- Parent `a624157703da1feabe5838db691b4b28e4d68328`: WAN DHCP callback #45031431084 PASS; AI handoff #45031431144 PASS.
- On earlier source `52a8c60d1e9aa324bb7f020fdeafda99a03319e9`, NVRAM transaction audit #45031084347 PASS; on descendant `592ce6b42c4c1f3ff473cdfb8839959c2fb4b5da`, VPN lifecycle #45031330323 PASS, WAN route #45031330345 PASS and AI handoff #45031330331 PASS.
- Parent `a624157703da1feabe5838db691b4b28e4d68328`: USB modem #45031431089 and firewall #45031431129 failed at the whitespace-fragile modem source-contract assertion. Process inventory #45031431085 and XY-C1 #45031431087 were still in progress when last checked. An earlier XY run #45031330348 failed at the same stale checker before target build; no target build/TRX/artifact success is claimed.
- The backup branch remains unchanged.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head USB modem, firewall, process inventory and XY-C1 Actions for this checker correction. Fix any remaining concrete failures.
2. Then continue the broader Padavan userland stability/security rotation. Avoid repeating startup audits and avoid reopening green code without new failure evidence.
3. XY-C1 L3 needs Build, TRX validation and artifact upload all PASS on one identical source SHA; do not claim hardware L4 without router hardware testing.
