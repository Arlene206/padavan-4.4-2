# SESSION HANDOFF — Padavan 4.4

## Current checkpoint

- Active development branch: main. Do not create/use development branches or PRs; never force-push.
- Frozen archive branch: backup/pre-plugin-overhaul-2026-11-14 at 72f4f157a084f8184b9597ca5bf334b8080e5807. Preserve permanently; do not modify, checkout, rename or delete.
- Latest source implementation checkpoint: d73ba6d2b9df959b73f0c0cf283d9694362b85c3 — fix(dnsmasq): preserve private-domain routing on NVRAM faults.
- The current main head may be a subsequent documentation-only commit; fetch live main and match exact head SHAs before any write or CI claim.

## Recovery correction

The previous handoff's active SCUTClient credential-file task has since been implemented on main. Do not restart it unless a new exact-head failure proves a defect. The source and subsequent hardening are recorded in dev_log.md, including commits ea5264dfbdb9470bc29971dee053503e8177b3aa and b30aeaf66d022f53061fd3433e98277131cbeda8.

During this takeover, two other concurrent main fixes were identified:
- a8e1f92e1b1bfb95015eca73f8eaf94fa57d552d fixed AliyunDrive's lifecycle-lock parent-directory expansion; exact SHA run 45083513060 passed.
- 898fb503994e4e301c95fab4a0c05824ca44885e fixed missing Process checker inputs; exact SHA AI handoff documentation run 45083559512 passed. The older Process umbrella run 45083559492 was pending at last observation; do not carry that stale status forward without checking live Actions.

## Implemented source slice — split-DNS NVRAM failure

In source SHA d73ba6d2b9df959b73f0c0cf283d9694362b85c3, fill_dnsmasq_servers() was changed from lossy nvram_get_int()/nvram_safe_get() reads to one bounded NVRAM transaction snapshot over:
- vpnc_pdns
- vpnc_dom_t
- vpnc_dns_t
- wanx_domain
- wanx_dns

The change preserves the original vpnc_pdns > 0 behavior and empty/missing-key defaults. Any backend read, transaction-close, malformed integer, or truncation error aborts before dnsmasq_file_open(), leaving the previous server snapshot intact instead of publishing a file that silently drops VPN/MAN routes and exposes those scoped queries to the default resolver path.

Regression coverage was added in trunk/user/rc/src/tests/dnsmasq_nvram_option_policy.c for atomic generation copying, missing/negative flag values, failures at every key, begin/end errors, malformed integers, and oversized values. scripts/check_dnsmasq_integer_options.py now checks the source/data-flow contract. The DNSMasq workflow pull_request.paths was corrected to include that checker (push.paths already included it).

Changed in source SHA d73ba6d2b9df959b73f0c0cf283d9694362b85c3:
- trunk/user/rc/src/services_ex.c
- trunk/user/rc/src/dnsmasq_nvram_option_policy.h
- trunk/user/rc/src/tests/dnsmasq_nvram_option_policy.c
- scripts/check_dnsmasq_integer_options.py
- .github/workflows/dnsmasq-lifecycle.yml
- REFACTOR_ROADMAP.md
- dev_log.md

## Exact-SHA verification state

The push event created these runs on exactly d73ba6d2b9df959b73f0c0cf283d9694362b85c3. They were QUEUED at first observation; check current live status and do not treat queued/in-progress as PASS:
- DNSMasq lifecycle stability regression: 45084574655
- NVRAM transaction boundary audit: 45084574653
- Process inventory and lifecycle fail-closed regression: 45084574525
- Process lifecycle inventory: 45084574515
- XY-C1 Firmware Validation: 45084574492
- WAN DHCP callback state stability regression: 45084574658
- miniupnpd atomic configuration publication regression: 45084574657
- Inadyn fail-closed lifecycle regression: 45084574546
- AI handoff documentation contract: 45084574530
- WAN route operation status regression: 45084574493
- HTTPD Authentication and Security Regression: 45084574469
- Samba share configuration stability regression: 45084574447

No local host test, local compile, MIPS firmware build, TRX check, artifact upload, or physical XY-C1 test was executed in this connector-only session. Source review for the NVRAM failure path is complete; exact-SHA Actions were queued at last observation. Do not claim L3/L4 verification without matching evidence.

## First actions for next continuation

1. Fetch current main and branch inventory once. Confirm frozen backup SHA remains 72f4f157a084f8184b9597ca5bf334b8080e5807; do not touch it.
2. Check the above Action run IDs and ensure each result's head_sha equals d73ba6d2b9df959b73f0c0cf283d9694362b85c3. Start with DNSMasq and the NVRAM transaction boundary audit. If failures occur, inspect first causal logs and fix them without weakening the fail-closed policy.
3. While any runs are pending, continue independent high-priority work outside the heavily modified DNSMasq path. Focus on core stability/security defects in WAN/PPP, storage, USB, process/service lifecycle or privileged management.
4. Use one full Git tree and one atomic commit per logical change; update main only with force:false and expected_sha from a freshly fetched main. Check all triggered runs against the resulting exact SHA.

Follow AI_BOOTSTRAP_PROMPT.md and AI_DEVELOPMENT_PROTOCOL.md. Keep all development on main, preserve process-identity tri-state semantics and fail-closed UNKNOWN handling, and never invent test/CI/build/device results.
