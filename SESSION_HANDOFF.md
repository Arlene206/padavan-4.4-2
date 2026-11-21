# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main HEAD immediately before the VPN server configuration-gate slice: `4e2e701f33d2be2c35004015d6625b718be67a7f`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.

## Active task

- WAN route teardown now obtains gateway, address and netmask from one NVRAM transaction in `wan_unit_route_snapshot()` (production change `74329b3f1e1ae376930dbeb1143d305fb3f6d646`). Tuple failure denies gateway /32 deletion while a separate checked gateway read preserves best-effort default-route cleanup. PPP uses a separate checked address read for conntrack and is excluded from /32 ownership. WAN route workflow #44930698354 and AI handoff #44930698407 passed on exact SHA `95180e9d7b1d86676928ebb4c58c8af4af4adbab`.
- XY-C1 Build, TRX verification and artifact upload passed on exact product-source SHA `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192` (#44929756345). WAN teardown #44929756599, WAN DHCP callback #44929756539, NVRAM audit #44929756500 and Process inventory #44929756463 passed on d0. L3 is VERIFIED for d0 only; this does not establish hardware L4.
- HTTPD production commit `f7a3a520f9ae0df659c651ce99b4ced20d103e23` rejects duplicate Authorization fields (first value retained, duplicate clears credential and protected requests fail closed). The follow-up on exact SHA `4e2e701f33d2be2c35004015d6625b718be67a7f` added the required XY-C1 source paths and corrected one source locator, but HTTPD security #44931211349 still fails in the final inline source contract at Python line 1012 with `ValueError: substring not found`; inspect that exact line/log before changing the checker. XY-C1 #44931211445 and Process #44931212103 PASS on exact 4e2. AI handoff #44931211395 failed because the mandatory `## Active task` heading was absent; this slice restores that required heading.
- On exact f7 SHA, AI handoff #44930885671 and Process lifecycle inventory #44930885725 passed. HTTPD security #44930885718 failed at the new source-contract block with `ValueError: substring not found` (Python line 502); it searched for an escaped-newline boundary rather than the actual `reset_lang_dict(void)` symbol. The HTTPD policy tests/source checks before that locator were reached, but do not claim the whole workflow passed.
- XY-C1 did not trigger on f7 because the push path filter omitted `httpd.c` and `httpd_basic_auth.h`; this correction adds those paths (and `base64.c`) and the workflow already lists itself so the changed path filter triggers a build. NVRAM audit #44930885723 was still in progress at the latest poll.

- VPN server startup has a fail-open NVRAM gate: PID 1 invokes `start_vpn_server()` unconditionally, and `nvram_invmatch("vpns_enable", "1")` treats both a missing key and backend read failure as “not a mismatch”. With `vpns_type` also read through `nvram_get_int()`, a failed type read becomes PPTP (0). This slice replaces startup and reapply gates with checked reads: missing enable is disabled; read errors abort; absent type preserves the historical PPTP default only when enabled; malformed, unsupported or failed type reads abort before lease state is cleared. GNU89 ASan/UBSan policy tests and VPN source/workflow contracts are included. Exact-head VPN and XY-C1 validation pending.

## Verification boundary

- L1: coherent WAN route ownership snapshot, transactional failure invalidation and PPP conntrack fallback are implemented; WAN route workflow passed on exact SHA 951, and WAN teardown / DHCP callback / NVRAM audit passed on d0.
- L2: Process inventory #44931212103 and XY-C1 #44931211445 passed on exact 4e2. HTTPD security #44931211349 still fails at Python line 1012 (unresolved source-contract lookup); AI handoff #44931211395 failed because the mandatory `## Active task` heading was absent. NVRAM audit #44930885723 passed on exact f7.
- L3: VERIFIED on exact product-source SHA `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192`: XY-C1 Build, TRX output verification and artifact upload all PASS. XY-C1 #44931211445 also passed on exact 4e2, covering the HTTPD duplicate-Authorization production change. The new VPN configuration gate in this slice needs its own same-SHA firmware build.
- L4: hardware WAN routing, concurrent callback behavior and on-device HTTPD duplicate-header behavior remain NOT VERIFIED.

## Workflow and preservation rules

- Main only. One logical slice, one atomic main commit: create all blobs, one tree based on current main, one commit with current main as parent, then non-forced update_ref using the exact expected SHA. No force push, amend, history rewrite, branches, PR-based development or partial per-file commits.
- Re-fetch main immediately before writing. On a moved ref, discard the stale prepared tree and rebuild from the new parent.
- Inspect push Actions only when head_sha exactly matches the commit. Diagnose failures from logs. PENDING does not block independent work.
- Preserve security, architecture, ABI, GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Never weaken useful tests or focused workflow path coverage.
- Do not touch GPT Session Rotator workflow/state or replay SHA 9d5a5cb. The backup branch is permanent immutable recovery data.

## Cross-conversation documentation governance

- GitHub is the durable cross-conversation record; the one-line user entrypoint remains sufficient.
- dev_log.md stays newest-first with 20–119 complete records. Before record 120, archive the oldest 100 complete records verbatim into docs/dev-log/YYYY/MM/part-NNN.md in one batch, leaving 20 newest entries.
- Rotate early only above the 384 KiB target and never exceed 512 KiB. Preserve archive/index bytes. Git history is the version archive for handoff and roadmap; routine snapshots and SHA-chasing commits are prohibited.
- Update dev_log.md for real milestones, this handoff when first-next-action facts materially change, and REFACTOR_ROADMAP.md only when repository-wide module status/priorities materially change.
- Before context exhaustion, finish the safe slice, commit atomically, inspect exact-head Actions and leave the next precise task.

## Exact first action for the next GPT

1. Inspect exact-head VPN stability and XY-C1 Actions for the VPN configuration-gate commit; repair only failures supported by logs.
2. Fetch the exact HTTPD security #44931211349 and AI handoff #44931211395 logs from SHA 4e2. Locate the precise failing Python line 1012, correct the source contract, and keep the duplicate-Authorization behavior tests intact.
3. Audit the remaining VPN server configuration reads in `gen_vpns_pppd_options()` and `start_vpn_server_locked()` (`vpns_auth`, `vpns_mppe`, `vpns_vuse`, `vpns_num_x`, `vpns_user_x*`, `vpns_pass_x*`, and pool/interface values). Identify which backend failures can publish incomplete credentials or an unintended address pool, then add checked copied reads and regression coverage. Do not reopen the WAN route snapshot or duplicate-Authorization production fix.
