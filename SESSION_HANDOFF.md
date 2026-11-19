# 2026-11-19 continuation checkpoint — close IPTV rollback CI and restore handoff integrity

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Current main tip: `f5fcdb9321b255acdf3e601c3b25228e6c5c7aac`. It contains workflow/checker corrections on top of product-source commit `5e9ebe5d5da73d770754808eaf68fbfbedcaa199`.
- Frozen archival branch: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`. Do not modify it. PR #1 is closed and unmerged.

## Active task

Close the cross-board IPTV VLAN rollback validation and restore canonical handoff/log records.

- WAN production changes in source commits `03105b871d62ba6597e3a419c57cde9a7e9bac4f`, `4df7ea3e998c163821d3c7fb767cdf24954dd9ee`, and `5e9ebe5d5da73d770754808eaf68fbfbedcaa199` snapshot static IPTV NVRAM settings before temporary writes, move IPTV ownership structs out of the optional GMAC2 block, and provide VLAN-only rollback on boards without GMAC2 bridge support. Commit `5e9ebe5` closes the GMAC2 feature block with a standalone `#endif` and adds a source contract for that boundary.
- Workflow-only tip `f5fcdb9` repaired the WAN DHCP checker contract for bounded local-copy static-IP preflight. Exact WAN DHCP callback run `44464784723` PASSed on `f5fcdb9`. The preceding run `44464340397` failed at a stale checker lookup for `if (viptv_mode == 2 &&`; production uses a nested static-mode validation block with checked NVRAM string copies.
- AI handoff documentation run `44464784761` failed because this file did not contain the mandatory end-of-session accuracy checkpoint wording. This checkpoint now restores those validator-required sections.

## Verification boundary

- L1: the cross-board compile root cause was the shared IPTV ownership type/helper being compiled only under `USE_GMAC2_TO_GPHY || USE_GMAC2_TO_GSW`, although `launch_viptv_wan()` uses them on every board. The ownership types and common helper are now outside that optional block; no-GMAC2 builds have a VLAN-only rollback path.
- L2: exact WAN route run `44464340419` PASSed on product-source SHA `5e9ebe5`. Exact WAN DHCP callback run `44464784723` PASSed on workflow-only tip `f5fcdb9`. On exact source SHA `5e9ebe5`, Process inventory `44464340982`, Process fail-closed `44464340448`, NVRAM audit `44464340593`, miniupnpd `44464340670`, WAN teardown `44464340557`, ZeroTier `44464340546`, DNSMasq `44464340637`, detect_internet `44464340543`, detect_link `44464340489`, Firewall `44464340389`, USB modem `44464340527`, watchdog `44464340395`, HTTPD `44464340698`, WAN ARP `44464340484`, Wireless `44464340418`, VPN `44464340468`, and WAN route `44464340419` all PASS.
- L3: exact XY-C1 run `44464340449` for source SHA `5e9ebe5d5da73d770754808eaf68fbfbedcaa199` was IN_PROGRESS at the latest inspection. Do not claim the current WAN source passed Build, TRX or artifact upload until that exact run completes.
- L4: physical-router validation NOT VERIFIED. No local checkout or local compilation was available.
- Root-log integrity: commit `9a394c3a4fe21ffe034fab2b39c1122877647903` accidentally reduced the rolling root `dev_log.md` from 435 lines to one entry. This repair restores the intact 46-entry root file from preserved parent `3b2c0f3dad38892f801d5778a4d74e3a8d6aaa04` and prepends a new complete record, giving 47 dated entries. Existing archive shards and indexes remain untouched.

## Exact first action for the next GPT

1. Fetch live main and confirm the frozen backup remains unchanged. Inspect exact-head AI handoff docs, XY-C1, WAN route, WAN DHCP callback, Process inventory and NVRAM Actions after this checkpoint commit.
2. Reconcile XY-C1 run `44464340449` on its exact source SHA. If it failed, inspect the first MIPS compiler/build error; if it passed, verify its TRX and artifact stages before claiming L3.
3. If AI docs fails, use its validator output to correct the live handoff or log/index contract without creating a no-op checkpoint or rewriting historic archive shards.
4. Once the WAN cross-board slice is closed, resume the highest-priority unfinished P1 item outside WAN/DHCP and Switch CLI. Use `REFACTOR_ROADMAP.md` to continue broad RC service-startup result propagation and independent service lifecycle/rollback coverage; do not repeat completed audits.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, the latest meaningful product-source/test/build baseline, relevant exact-head Actions, and this handoff's active task, verification boundary, blockers and exact first action. Distinguish documentation/checker-only tips from product-source commits.

A relevant exact-head Action changing from PENDING to PASS or FAIL is checkpoint information when it changes the validation gate or next action. Do not rewrite this file merely because main advanced or a pending run was polled. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

# Cross-session execution mandate

Recover once and continue the same authorized task; do not repeat completed audits or ask the user what to do next. Work only on main. Each coherent multi-file slice is one atomic main commit: create all blobs, one complete Git tree and one commit with the exact current main parent, then update main through non-forced compare-and-swap with that expected SHA. Never force-push, create a development branch or use a PR.

After each commit, inspect relevant push Actions by exact head SHA. Diagnose FAIL; PENDING does not block independent work. Preserve regression coverage and report validation only at the level actually achieved.

### Before context exhaustion

Finish the current safe slice, inspect exact-head Actions and update this handoff only when the resumable task, blocker, verification boundary or exact first action materially changes. Leave one precise first action for the next GPT. Never claim local tests, CI, firmware builds, TRX artifacts or hardware checks that were not actually completed.

## Development invariants

- Keep `backup/pre-plugin-overhaul-2026-11-14` byte-for-byte unchanged.
- Use only main. Every multi-file milestone is one complete tree and one atomic main commit, followed by a non-forced expected-SHA ref update.
- Do not weaken tests or alter workflow path filters to suppress failures. Treat Actions as valid only for their exact `head_sha`.
