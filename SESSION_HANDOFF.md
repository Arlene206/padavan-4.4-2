# Cross-session execution mandate

## 2026-11-23 continuation handoff (engineering parent main: c6b5dda145894feacbe67a2cabd4f5f60b99b046; fetch live main at takeover)

### Completed engineering slices and exact evidence
- `ba0dfca34d3e87f68cc44190509e29b2ce4e5853`: AliyunDrive process presence bound to configured executable device/inode, rejecting BusyBox `pidof` comm/argv[0] spoofing. Exact lifecycle #45230625667 PASS (shell syntax, MIPSel CLI under QEMU, lifecycle, early-lock and RC integration); Process #45230625920 PASS; AI docs #45230625723 PASS. L2 host/QEMU only; target router shell runtime and physical L4 not verified.
- `9ebc57999dad5da47e20414174cd7572ee64bbd7`: shared process inventory revalidates `/proc/PID/stat` after cmdline reads and rejects PID reuse. Exact Process #45227637642, fail-closed #45227637610 and AI docs #45227637592 PASS. XY-C1 #45227637594 passed Build, TRX and artifact upload on the identical SHA (L3).
- `7e81b11d63b1bf20d8a1eb103cc7598b9b898b1a`: repair Shadowsocks backup-server startup, firewall node identity and required-stage error propagation. Exact Shadowsocks #45228682477, Process #45228682072 and AI docs #45228682265 PASS. XY-C1 #45228682113 passed Build/TRX/artifact on the same SHA (L3).
- `53c6f6623e5531deeb889762e81bf1943c1abf50`: DNSMasq split-DNS source-contract correction. Exact DNSMasq #45229862631 and AI handoff #45229862614 PASS.
- Shadowsocks DNS proxy preflight plus malformed-upstream whitespace rejection and fixture/checker updates: exact lifecycle #45234515153 PASS on `f3acf67d58203ae7d18b6d2755014840ba10cccc`; Process #45234515707 PASS; AI docs #45234515042 PASS. XY-C1 #45234516216 was IN_PROGRESS at the last poll. Previous exact lifecycle #45233294870 on `7fdf985` also passed.
- Current RStats source slice (parent `f3acf67d58203ae7d18b6d2755014840ba10cccc`): `save_history_json()` now reads its interface selector through `nvram_get_checked()`, copies the shared NVRAM cache value before another NVRAM operation, and returns before atomic output staging on backend, NULL-present or overlong state. Missing key retains legacy empty/default behaviour. Exact-head Actions pending.

- `601ad73cb90227d01497a1a4f28a967de8c17a6a`: RStats selector uses checked NVRAM read and copies the cache-backed value before later operations. Exact RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS. Its AI-doc failure was corrected by `3760b93`, exact AI docs #45236207680 PASS; XY-C1 #45234873441 was pending.
- `5a43b18722b9d2f3f864ef6bd1025084ff1edb85`: missing-DNS-proxy-executable regression; exact Shadowsocks #45235046114 and Process #45235046275 PASS. Its AI-doc failure was likewise corrected by `3760b93`; exact AI docs #45236207680 PASS.

- Exact-source commit `eef441729725ff6b06c45a95b7c8414ab16d2f72` added bounded parsing to both `lld2d` and optional `lld2test`, including strict numeric parsing, ASan/UBSan cases, a dedicated workflow and XY-C1 path trigger. Its LLDT #45237411685 and Process #45237411877 failed only because seven fixture character literals were emitted as multi-character `'\\\\0'` instead of NUL `'\\0'`; follow-up corrects them and requires fresh exact-SHA verification. AI docs #45237411785 PASS. XY-C1 #45237411963 and NVRAM audit #45237411699 were still in progress at last poll.
- Follow-up `14be2b00c5f32eccb8694281f42d8b1bfed3dcb5`: NUL character spelling was corrected, but exact LLDT #45237898307 then reached execution and failed on the first fixture assertion because test input strings also contained doubled CR/LF escapes. The next test-only commit corrects these remaining test literals. AI handoff #45237898460 PASS; Process umbrella #45237898424, Process lifecycle #45237898195 and NVRAM #45237898280 were pending at last poll.

- Exact LLDT run #45238183801 on `d0e68ad`: GNU89/Werror/ASan/UBSan compile and parser regression completed PASS; source checker failed only because it incorrectly expected two XY-C1 header path entries. The XY-C1 firmware workflow has one push-path filter (no PR trigger); checker-only correction now accepts one. Process inventory #45238182222 PASS; AI docs #45238182196 PASS. Process fail-closed #45238182167 and NVRAM audit #45238182242 remained pending at last poll.

- On `d0e68ad217cb910e877dd69b6f4ce1d2b8abbf53`, LLDT run #45238183801 passed compile and all GNU89/ASan/UBSan fixture runtime cases, then failed the static source checker only because its XY-C1 path count expected two entries while the workflow has one push-only trigger. Commit `38d3732` corrected that test expectation.
- The same d0 Process umbrella #45238182167 failed only at `scripts/check_process_lifecycle_integration.py`: a stale assertion still expected the former inline `sscanf("%255s = %255s%n")` parser after `main.c` had switched to shared `lldt_config_parse_policy.h`. This commit aligns the integration checker and adds its path trigger. Process inventory #45238182222 and AI docs #45238182196 PASS. NVRAM audit #45238182242 was PENDING at last poll.

- Exact source base `eef441729725ff6b06c45a95b7c8414ab16d2f72` adds the shared bounded LLDT config parser to shipped `lld2d` and optional `lld2test`. XY-C1 #45237411963 on that exact production-source SHA completed SUCCESS with Build XY-C1, TRX verification and artifact upload all PASS (L3); NVRAM audit #45237411699 and Process inventory #45237412141 also PASS. Dedicated LLDT workflow #45238664960 (GNU89/Werror/ASan/UBSan compile, runtime and Python checker) and AI docs #45238664839 PASS on test/docs descendant `38d3732`.
- Process umbrella #45239229205 PASS on `79f4d535af499995fc1f2be1d24b52b93a35d2f4`, including the corrected shared-parser integration contract. AI docs #45239229201 failed only because the newly added parser-header trigger existed in push but not pull_request; this commit restores path-set symmetry and requires fresh exact-head docs PASS.

- Infosvr exact source commit `4e751b58b471e3d07e07a02ed64df3cc1edf0cd9`: GNU89/Werror/ASan/UBSan workflow #45201353587 PASS on checker descendant `6a7ed2a` (all policy binaries, packet source contract and RC lifecycle checker). XY-C1 #45200876769 on the actual production source SHA `4e751b5` PASS with Build, TRX and artifact upload. The earlier #45200876644 failed only on a stale `sizeof(IBOX_COMM_PKT_HDR)` matcher; it was fixed by `ac3228a`/`6a7ed2a`. L2 + L3 are verified; hardware broadcast L4 remains unverified.
- WAN gateway ARP source fix `1cb817908e779c45d3c47105e39cd2002df0ba63`: exact XY-C1 #45218315124 PASS with Build/TRX/artifact; WAN L2 #45218700965 PASS on checker descendant `40ab459`; AI docs #45218701533 PASS. The earlier WAN L2 #45218314720 failed only on an outdated interface-source string assertion, normalized in `40ab459`. The current local-IP status/error correction supersedes this validation and needs fresh exact-head Actions.

- WAN local IPv4 status commit `bd56c02f7f8d10374c6857f8aac1d56b1f7a4b6e`: WAN ARP #45241824790 PASS; WAN DHCP #45241824847 PASS; Process inventory #45241824430 PASS; AI docs #45241824655 PASS. WAN route #45241824836 and Process umbrella #45241824549 failed only at stale source assertions, both corrected in follow-up `<next SHA>`; exact reruns pending. XY-C1 #45241824748 and NVRAM #45241824746 were in progress at last poll.

- WAN local IPv4 error separation follow-up `netutils.c`: invalid interface-name copies now set EINVAL; `get_interface_addr4()` preserves ioctl/close errors across cleanup. WAN EINVAL fixture and source contract added; exact CI pending.
- Current WAN source follow-up is `d3772d64a0e2471be22658aabd6299dc3670caa8`: `copy_ifname()` sets EINVAL and `get_interface_addr4()` preserves ioctl/close errors; WAN ARP #45243513318, DHCP #45243513523, Process inventory #45243513456 and AI docs #45243513254 PASS. WAN route #45243513566 failed solely because the workflow path list had two `netutils.c` entries in each event, while `check_wan_vlan_setup.py` requires one. This commit restores the prior single-entry path list; the source-check additions remain.
- Subsequent main commits `36861ae` and `eb92a7a` add bridge selector fail-closed handling and exact-source CI coverage; preserve those commits. On `eb92a7a`, AI handoff docs #45243797283 PASS; the remaining inventory/XY-C1/system checks were still running at last poll.

## Current repository state
- Repository `Arlene206/padavan-4.4-2`; work only on `main`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify, reset, merge, rebase or force-push it.
- PR #1 is closed/unmerged. AliDDNS CA trust-store/HMAC work is represented or superseded on current `main`.
- No local checkout or MIPS/uClibc toolchain is available. Do not claim local GNU89 test execution, target firmware/TRX/artifact or physical-device validation; exact GitHub Actions are the verification source.
- XY-C1 disables `CONFIG_FIRMWARE_INCLUDE_ALDRIVER`; AliyunDrive shell runtime is not covered by XY-C1. Its dedicated workflow establishes only host/QEMU checks.

## Active task

### VPN client shared xl2tpd ownership and recovery

- WAN local IPv4 error separation is closed at L2/L3: `d3772d64a0e2471be22658aabd6299dc3670caa8` preserves invalid-interface EINVAL and ioctl/close errno; WAN route #45244253977 and AI docs #45244253913 PASS on workflow-only descendant `7391f2e6245e64941ca709784719ebfb723043dc`; XY-C1 #45244253978 on that same descendant passed Build, TRX verification and artifact upload. Physical router L4 remains unverified.
- Confirmed defect in `trunk/user/rc/src/vpn_client.c`: the old `get_xl2tpd_vpnc_active()` used error-collapsing `nvram_match()` calls, and restart recovery used unchecked `l2tp_wan_t` / `l2tp_srv_t` matches after the forced stop. NVRAM read errors could be interpreted as “no shared users”, leaving WAN L2TP or L2TP VPN server without `xl2tpd`.
- This source slice replaces the stop-set decision with a checked NVRAM transaction (VPNC enable/type/client marker), validates bounded selector values, and aborts teardown if ownership is unknown. WAN/server dependencies are read from a second consistent transaction.
- If dependency state is unknown after the restart has stopped `xl2tpd`, recovery now attempts conservative daemon restoration and still reports the read error. Confirmed active dependencies also restore; confirmed absence is a no-op.
- GNU89 policy regression additions cover missing/empty/default values, malformed/overflow values and backend errors; restart-policy tests cover unknown dependency restoration; the source checker asserts transaction boundaries and fail-safe ordering. Exact-head Actions are pending; no local compile or router hardware validation is claimed.

### LLDT configuration parser parity
- The shipped `lld2d` parser was already bounded; optional `lld2test` retained unbounded `sscanf("%s")`, an unterminated `strncpy` icon path and overflow-prone `atoi`. Both now use the shared length-delimited parser; the optional tool also uses strict bounded numeric parsing.
- This source slice introduces `lldt_config_parse_policy.h` and applies it to both configuration readers. The optional tool now uses checked icon path copying and strict bounded numeric parsing with safe legacy defaults.
- Dedicated LLDT #45238664960 and AI docs #45238664839 PASS on `38d3732`; GNU89/Werror/ASan/UBSan compile/runtime and source checks passed. Process umbrella #45239229205 PASS on `79f4d53`; current Process #45239753092 and AI docs #45239753450 PASS on `f688ae9`. XY-C1 #45237411963 on production-source SHA `eef4417` PASS, including Build, TRX and artifact upload.
- `ctmain.c` is the optional `lld2test` developer utility; package ROMFS installs production `lld2d`, not `lld2test`. This utility hardening is not presented as a firmware runtime change.

## Verification boundary
- LLDT config parser: dedicated L2 workflow #45238664960 PASS; Process umbrella #45239229205 PASS. XY-C1 #45237411963 PASS on exact source SHA `eef4417` (Build/TRX/artifact). AI docs #45239229201 needs one workflow path-filter correction (this commit).
- Shadowsocks DNS proxy preflight and upstream whitespace fixes: #45234515153 PASS, Process #45234515707 PASS and AI docs #45234515042 PASS on `f3acf67`; XY-C1 #45234516216 was pending.
- RStats `601ad73`: RStats #45234873403, Process #45234873149/#45234873148, NVRAM #45234873426 and HTTPD #45234873048 PASS; AI doc repair #45236207680 PASS on `3760b93`. XY-C1 #45234873441 still requires final same-SHA step confirmation before an L3 claim.
- Shadowsocks missing-executable test `5a43b18`: lifecycle #45235046114 and Process #45235046275 PASS; AI documentation repaired by #45236207680 on `3760b93`.
- Exact LLDT #45238664960 on `38d3732` passed GNU89/Werror/ASan/UBSan compile/runtime and its Python production/optional-tool checker; AI docs #45238664839 PASS. Process #45239229205 PASS on `79f4d53`. XY-C1 #45237411963 on the production source SHA `eef4417` passed Build, TRX and artifact upload. AI docs #45239229201 exposed only the path-filter asymmetry corrected in this commit. No local compile is claimed.
- Physical router L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main` and inspect exact-head push Actions for the VPN server lifecycle regression (client shared-xl2tpd transaction and recovery), Process lifecycle, AI handoff docs and XY-C1. Diagnose every real FAIL; continue independent work while runs remain PENDING.
2. L2 is complete only when the new GNU89 policy tests and `scripts/check_vpn_server_lifecycle.py` pass on a current exact SHA. L3 requires XY-C1 Build, TRX verification and artifact upload all PASS on the identical production-source SHA. Hardware L4 remains unverified.
3. Then continue one concrete reliability/security source issue outside the corrected VPN client shared-xl2tpd path. Keep `backup/pre-plugin-overhaul-2026-11-14` frozen; use complete-tree commits and expected-SHA non-forced updates on `main`.

## Cross-session operating constraints
Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on `main`; each logical source or CI change is one atomic main commit: create all changed blobs and one complete Git tree before creating that commit, then update `main` using a non-forced expected-SHA compare-and-swap. Do not ask the user what to do next.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop
- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.