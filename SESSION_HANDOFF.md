# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Main HEAD immediately before this correction: `f7a3a520f9ae0df659c651ce99b4ced20d103e23`; frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.

## Active development and verification

- WAN route teardown now obtains gateway, address and netmask from one NVRAM transaction in `wan_unit_route_snapshot()` (production change `74329b3f1e1ae376930dbeb1143d305fb3f6d646`). Tuple failure denies gateway /32 deletion while a separate checked gateway read preserves best-effort default-route cleanup. PPP uses a separate checked address read for conntrack and is excluded from /32 ownership. WAN route workflow #44930698354 and AI handoff #44930698407 passed on exact SHA `95180e9d7b1d86676928ebb4c58c8af4af4adbab`.
- XY-C1 Build, TRX verification and artifact upload passed on exact product-source SHA `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192` (#44929756345). WAN teardown #44929756599, WAN DHCP callback #44929756539, NVRAM audit #44929756500 and Process inventory #44929756463 passed on d0. L3 is VERIFIED for d0 only; this does not establish hardware L4.
- HTTPD production commit `f7a3a520f9ae0df659c651ce99b4ced20d103e23` hardens Basic authentication parsing: the first Authorization field is recorded, any duplicate clears the credential and marks it ambiguous, and protected requests fail the authentication gate. GNU89 unit coverage was added for first-field acceptance, duplicate rejection and third-field persistence.
- On exact f7 SHA, AI handoff #44930885671 and Process lifecycle inventory #44930885725 passed. HTTPD security #44930885718 failed at the new source-contract block with `ValueError: substring not found` (Python line 502); it searched for an escaped-newline boundary rather than the actual `reset_lang_dict(void)` symbol. The HTTPD policy tests/source checks before that locator were reached, but do not claim the whole workflow passed.
- XY-C1 did not trigger on f7 because the push path filter omitted `httpd.c` and `httpd_basic_auth.h`; this correction adds those paths (and `base64.c`) and the workflow already lists itself so the changed path filter triggers a build. NVRAM audit #44930885723 was still in progress at the latest poll.

## Verification boundary

- L1: coherent WAN route ownership snapshot, transactional failure invalidation and PPP conntrack fallback are implemented; WAN route workflow passed on exact SHA 951, and WAN teardown / DHCP callback / NVRAM audit passed on d0.
- L2: Process inventory #44930885725 and AI handoff #44930885671 passed on exact f7. HTTPD security #44930885718 failed at a source-contract boundary locator and requires a fresh exact-head run after the locator fix. NVRAM audit #44930885723 was pending at last poll.
- L3: VERIFIED on exact product-source SHA `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192`: XY-C1 Build, TRX output verification and artifact upload all PASS. The HTTPD production change in f7 needs its own exact-head cross-build before L3 can be claimed for that code.
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

Fetch the exact-head HTTPD security, NVRAM transaction audit, Process inventory and XY-C1 workflow runs for the new commit. Repair only failures supported by their exact logs. The XY-C1 workflow path filter now includes `httpd.c`, `httpd_basic_auth.h`, `base64.c` and itself so the duplicate-Authorization production change receives a full Build/TRX/artifact run. L3 for the latest HTTPD code is not verified until all three XY-C1 stages pass on one exact SHA. After these checks pass, continue with a concrete stability/security defect in another Padavan userland subsystem rather than reopening the already-verified WAN route transaction or MiniUPnP SOAP fix.
