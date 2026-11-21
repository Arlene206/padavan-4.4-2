# Cross-session execution mandate

## Workflow
- Develop on `main` only. One logical slice, one atomic main commit: create all blobs, one tree and one commit, then use a non-forced expected-SHA compare-and-swap update. Never force-push, amend, create branches/PRs, or edit the immutable backup.
- Preserve security, architecture, compatibility and required tests. Target GNU89, MIPS/uClibc, BusyBox and Linux 4.4. Do not weaken tests.
- Exact-head push Actions only validate the matching main SHA. PENDING is not a gate on independent work; inspect a relevant FAIL, fix its cause, and move forward.
- Before context exhaustion, finish the safe slice, publish the commit, inspect exact-SHA Actions and leave a truthful next action.
- Keep root `dev_log.md` at 20–119 newest-first entries; preserve `docs/dev-log/2026/11/part-021.md` and its index unchanged. Do not create handoff/roadmap snapshot archives.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect the live repository state before ending, verify this handoff's source baseline, active task, exact-head Actions, validation boundary, blockers and first next action. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no no-op commit. Update this handoff only when next-session-critical facts materially change.

## Current repository state
- Live main at the start of this slice: `728468e145d625164bf2a6aaba6883b183cb20c2`, commit `test(wan): align checked NVRAM source contracts`.
- The preceding source milestone `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6` fixed NVRAM shared-cache reuse in `launch_viptv_wan()` and introduced checked DHCP renewal status reads. The new commit `728468e145d625164bf2a6aaba6883b183cb20c2` aligned HTTPD route checks and WAN route IPTV copy checks; HTTPD security passed exact-head.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; do not touch it.
- On exact SHA `728468e145d625164bf2a6aaba6883b183cb20c2`: HTTPD Security `#44896094278` PASS. WAN DHCP callback `#44896094291` FAIL later in its embedded checker because the IPTV renew half still expected the removed unchecked retry helper/direct getters. WAN Route status `#44896094274` passes VLAN checks but fails at `scripts/check_wan_initial_mtu_status.py:334`, which still references the legacy retry helper for physical and IPTV renew. AI docs `#44896094292` FAIL because the handoff also needs the exact cross-session execution wording `one atomic main commit`. Those three stale checker/governance contracts are the active repair in the next commit.
- On exact SHA `21bf595a0ecc81b2cf8e5500c93f93c03f4811e6`: Process lifecycle inventory `#44895865634`, NVRAM transaction audit `#44895865623`, MiniUPnP `#44895865555`, Process lifecycle/fail-closed `#44895865762`, USB modem identity `#44895865719`, Firewall startup `#44895865687`, wireless `#44895865597`, watchdog/detect_link/detect_internet/WAN ARP `#44895865747`, `#44895865690`, `#44895865672`, `#44895865679`, and WAN teardown `#44895865582` passed. These do not transfer as exact-head source validation to newer SHAs.
- XY-C1 L3 on `21bf...` was still Pending when last polled (`#44895865716`); the prior established L3 baseline remains `20e139d230b73eabc5f25a2b17d56a3a5768af11` / `#44887656164`. Current-source L3 and hardware L4 are NOT VERIFIED.

## Active task
- Close stale checked-NVRAM renewal contracts in `.github/workflows/wan-dhcp-callback-stability.yml` and `scripts/check_wan_initial_mtu_status.py`. Require all three checked lease/route/MTU status reads, fail-closed decision-error ordering, the retry decision before bound re-entry, and checked stored-MTU reads for both physical WAN and IPTV. Do not reintroduce removed unchecked getters.
- Once exact-head WAN DHCP/route/AI docs regressions pass, revisit any remaining FAILs and rotate to a distinct P1 reliability/security issue in a less-worked Padavan userland module. The old AliDDNS CA/HMAC PR #1 remains an unmerged preservation task; continue reviewing and port valid work to main only after the active validation slice closes.

## Verification boundary
- No local checkout/runtime was available for this session; no local C/Python tests, sanitizer run or firmware build is claimed.
- Remote Actions are the tests run. Host regressions are L2 only. XY-C1 Build + TRX + artifact on one exact production-source SHA is required for L3; physical router validation is L4 and remains NOT VERIFIED.

## Exact first action for the next GPT
1. Fetch live `main` immediately and confirm it equals the new commit SHA recorded in the just-added dev-log entry.
2. Inspect exact-head push Actions `HTTPD Security`, `WAN DHCP callback`, `WAN Route status`, and `AI handoff documentation`; fix the first causal failure and continue the loop. Check the exact SHA, not the older runs listed above.
3. Continue review of legacy PR #1 only after these active regressions converge; do not use its branch or PR as a development/validation target.
