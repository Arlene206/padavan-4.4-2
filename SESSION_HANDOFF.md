# Cross-session execution mandate

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Compare live main, product-source baseline, active task, exact-head Actions, validation boundary and first next action before ending. Update this record only when those facts materially change; do not make SHA-chasing or routine snapshot commits.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; development branch: `main`.
- Latest main source checkpoint: `67274f90f6da54c8688f238e78327543e046ca7b` (VPN CHAP credential snapshot plus checked LAN/custom VPN address-pool validation); frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`.
- PR #1 remains closed/unmerged and is not a development path. No force push, branch creation, or backup-branch mutation.
- The MiniUPnP SOAP internal-port-zero TODO is already implemented in `upnpsoap.c` / `upnpsoap_input_policy.h` and covered by `upnpsoap_input_policy_test.c`; do not duplicate it.

## Active task

- WAN route teardown now obtains gateway, address and netmask from one NVRAM transaction in `wan_unit_route_snapshot()` (production change `74329b3f1e1ae376930dbeb1143d305fb3f6d646`). Tuple failure denies gateway /32 deletion while a separate checked gateway read preserves best-effort default-route cleanup. PPP uses a separate checked address read for conntrack and is excluded from /32 ownership. WAN route workflow #44930698354 and AI handoff #44930698407 passed on exact SHA `95180e9d7b1d86676928ebb4c58c8af4af4adbab`.
- XY-C1 Build, TRX verification and artifact upload passed on exact product-source SHA `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192` (#44929756345). WAN teardown #44929756599, WAN DHCP callback #44929756539, NVRAM audit #44929756500 and Process inventory #44929756463 passed on d0. L3 is VERIFIED for d0 only; this does not establish hardware L4.
- HTTPD production commit `f7a3a520f9ae0df659c651ce99b4ced20d103e23` rejects duplicate Authorization fields (first value retained, duplicate clears credential and protected requests fail closed). Exact-head HTTPD security #44931678295 on `91ab0fb6ceaf0c2fa068cd9cad32f57422bfa438` still failed at Python line 1012 because the login-activity contract searched for the pre-hardening gate `if (!auth_check(authorization))`; the current workflow update asserts `authorization_header.duplicate || !auth_check(authorization)`. AI handoff #44931678330 and Process inventory #44931678314 passed on 91ab. VPN server lifecycle #44931678366 passed. XY-C1 #44931678300 and NVRAM audit #44931678280 remain pending at the last poll.
- On exact f7 SHA, AI handoff #44930885671 and Process lifecycle inventory #44930885725 passed. HTTPD security #44930885718 failed at the new source-contract block with `ValueError: substring not found` (Python line 502); it searched for an escaped-newline boundary rather than the actual `reset_lang_dict(void)` symbol. The HTTPD policy tests/source checks before that locator were reached, but do not claim the whole workflow passed.
- The 4e2 XY-C1 workflow run #44931211445 was a host-only success: its Build XY-C1, Verify TRX output and upload-artifact steps were SKIPPED by the firmware-input detector, so it is not an L3 result for the HTTPD change. The subsequent VPN source commit 91ab did trigger an in-progress XY-C1 run #44931678300.

- VPN server startup has a fail-open NVRAM gate: PID 1 invokes `start_vpn_server()` unconditionally, and `nvram_invmatch("vpns_enable", "1")` treats both a missing key and backend read failure as “not a mismatch”. With `vpns_type` also read through `nvram_get_int()`, a failed type read becomes PPTP (0). This slice replaces startup and reapply gates with checked reads: missing enable is disabled; read errors abort; absent type preserves the historical PPTP default only when enabled; malformed, unsupported or failed type reads abort before lease state is cleared. GNU89 ASan/UBSan policy tests and VPN source/workflow contracts are included. Exact-head VPN and XY-C1 validation pending.

- VPN enable/type startup gate commit `91ab0fb6ceaf0c2fa068cd9cad32f57422bfa438` uses checked NVRAM reads for startup/reapply. VPN lifecycle, AI handoff, Process, Firewall startup, OpenVPN credential staging and NVRAM audit passed on the recorded exact 91ab; HTTPD security and AI handoff source-contract correction passed on exact 588b; DNSMasq and AI handoff passed on exact 8c3. XY-C1 #44931678300 was still in progress at the last poll.

- VPN PPP CHAP credentials now use one exclusive NVRAM transaction snapshot for `vpns_num_x`, username/password pairs and each active row's address octet. Every cache-backed NVRAM value is copied before the next read; strings are bounded to the UI's 32-character field and reject control bytes, quotes and backslashes. Backend, parse/range, buffer and transaction-end failures abort before lease cleanup or truncating CHAP secrets. This fixes the fail-open case where `nvram_get_int("vpns_addr_xN")` converted backend errors to zero and wrote wildcard `*`.

- VPN LAN-backed and custom address-pool selection reads `vpns_vuse`, LAN IPv4/netmask, custom subnet and pool offsets inside one exclusive NVRAM transaction. Strict IPv4/contiguous-mask checks reject missing or malformed values; LAN pools clamp to usable host offsets and reject router-address collisions/oversized ranges; custom pools must be RFC1918 and must not overlap LAN. `get_vpns_pool()` now returns status so invalid configuration aborts before PPTP config publication.

- Exact-head validation of pool commit `d59d266a0e37436e1d5fc8ffb708ef20112b301d`: VPN server lifecycle #44933121391 PASS; WAN DHCP callback #44933121789 and WAN route #44933121597 PASS; AI handoff docs #44933121473 PASS; Process inventory #44933121412 PASS. HTTPD security #44933121393 failed only in its final inline source-contract block: two function-boundary lookups still expected `void get_vpns_pool()` after the API became status-returning `int get_vpns_pool()`. This commit updates both stale delimiters, without changing production code or weakening assertions. XY-C1 #44933121304, Process lifecycle #44933121578, NVRAM audit #44933121469 and FRP #44933121501 were in progress at the last observation.

- PPP options generation now snapshots checked auth mode, MPPE mode, virtual-address mode, MTU/MRU, LAN address and optional DHCP DNS/WINS inside one NVRAM transaction; DHCP service enablement uses its checked status API. Missing numeric keys retain legacy defaults; backend errors, malformed/out-of-range numbers, overlong strings and invalid LAN IPv4 abort before the PPP options file is opened/truncated. DNS/WINS values still use the existing validity filter when emitted.

## Verification boundary

- L1: VPN enable/type fail-closed reads, transactional CHAP credential snapshot and transactional/validated LAN or RFC1918 custom address-pool selection are implemented. WAN route teardown retains the transactional ownership snapshot and separate PPP conntrack address read.
- L2: On exact pool-source SHA `d59d266a0e37436e1d5fc8ffb708ef20112b301d`, VPN server lifecycle #44933121391 PASS, WAN DHCP callback #44933121789 PASS, WAN route #44933121597 PASS, AI handoff #44933121473 PASS, Process inventory #44933121412 PASS. HTTPD security #44933121393 failed at its final Python source-contract locator because two old delimiters expected the previous void pool API; both are corrected by this checker-only commit. Exact-head rerun is required. Process lifecycle #44933121578, NVRAM audit #44933121469, FRP #44933121501 and XY-C1 #44933121304 were PENDING at the last observation.
- L3: VERIFIED only on product-source SHA `d0f9140a9ba392dbd0a9366b5a7472d0e7c9a192` where Build, TRX verification and artifact upload all passed. This does not verify the later VPN source changes at d59; its exact XY-C1 run #44933121304 was PENDING. L4 hardware validation remains NOT VERIFIED.
- L4: Hardware WAN routing, concurrent callback behavior and on-device HTTPD duplicate-header behavior remain NOT VERIFIED.

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

1. Check exact-head VPN lifecycle and NVRAM audit after the PPP snapshot checker now extracts the actual function definition rather than its forward declaration. Recheck XY-C1 Build/TRX/artifact for the same source SHA; do not claim L3 before it passes.
2. Continue VPN remote-LAN route ACL hardening in `vpns_route_to_remote_lan()`: its current compatibility reads of `vpns_num_x`, username, remote network/mask and LAN IP/netmask can silently suppress a route or use mixed policy on NVRAM errors. Snapshot the row policy transactionally and fail before route mutation when a backend read fails.
3. Audit residual VPN hostname and DHCP-derived option inputs only after route ACL regression is committed.
