## Current repository state

- Repository: Arlene206/padavan-4.4-2. Main is exact SHA b774286064475c5d09c57fe20f342623740df1e8; development/commits remain main-only. The rechecked remote branch inventory contains only main; no checkpoint branch exists. Legacy PR #1 remains CLOSED/unmerged and useful AliDDNS work is integrated. GPT Session Rotator remains immutable.
- Stock Adbyby managed lazy/video downloader is L2 CLOSED on 42d4af7e6c0ceeb8508f1446b4228068318208dc.
- NVRAM-configured Adbyby third-party rule downloads are L2 CLOSED on exact main a8681f65733f08b7c932833fafe99062492f0a2a. Implementation commit 315d738a580e154302dfdd9c57674ee64c3ada36; checker-only corrections 267bfb4e2d669c07d0daee918302e8780abc5958 and a8681f65733f08b7c932833fafe99062492f0a2a. Final exact-head Actions all passed and all checkouts matched a8681f65733f08b7c932833fafe99062492f0a2a: Adbyby [43363263383] / job [131681514167], infosvr [43363263454] / job [131681514465], AI docs [43363263378] / job [131681514145], AliDDNS [43363263594] / job [131681514640], AliyunDrive [43363263430] / job [131681513797], AdGuardHome [43363263365] / job [131681513585]. All steps and full logs inspected. Adbyby shell syntax, all five host regressions and source/workflow semantic checker passed; AI docs reports root log 97/119 and archive inventory 7/22/18. AliyunDrive bundled MIPSel daemon was checked under QEMU; this is not target firmware validation.
- Shared curl options were checked against trunk/libs/libcurl/Makefile: firmware curl version is 8.21.0, supporting HTTPS protocol restriction, maximum file size and rate limits. No full firmware build or router test was run.
- Remaining Adbyby gaps are anti_ad_link and the configured hosts-source list, both still using curl -k and fixed output paths. They are intentionally outside the completed third-party-rule scope.
- FTP failed-upload cleanup remains OPEN/BLOCKED because fstatat() identity check followed by unlinkat() leaves a final-name replacement race; Linux 4.4 has no inode-conditional unlink-by-fd. Do not weaken STOR/APPE/REST/STOU or concurrent external-writer semantics without a proven race-safe design.
- NFS async durability/AUTH_SYS/root-squash/external-mount compatibility still requires real client evidence. After Adbyby anti-ad and hosts-source downloads, continue the FRP full RC/UI/cron/shared-script audit. Current-main L3 MIPS/uClibc firmware/TRX and L4 physical router remain NOT VERIFIED.

## Active task

Harden Adbyby anti_ad_link DNSMasq fragment download.

The remaining anti_ad() path writes an untrusted NVRAM URL directly to /etc/storage/dnsmasq-adbyby.d/anti-ad-for-dnsmasq.conf using curl -k. This disables TLS certificate verification, expands the URL unquoted, writes directly over the last-good live dnsmasq fragment, has no HTTP-status or body-size enforcement, and treats mere file existence as a successful download. Because the fragment is included through conf-dir, do not invent a narrow content grammar before inspecting the actual NVRAM/UI defaults and supported upstream file format. Preserve operator compatibility while rejecting malformed/empty content and config-directive injection.

The next slice must reuse the shared bounded HTTPS downloader and firmware CA, create a private staging directory on the same filesystem as the final fragment, validate the complete source format, and publish with one same-directory atomic rename only after full success. Refuse unsafe destination symlinks/non-regular files. Retain a valid last-good fragment and its count on remote/format failure; fail closed when no usable previous fragment exists, but do not stop unrelated Adbyby dnsmasq rules unnecessarily. Add a no-network fixture for valid, TLS/HTTP/redirect, malformed, oversized, missing-CA, last-good and failed-publication cases; extend the source checker and push/PR workflow filters symmetrically.

After anti-ad is closed, harden the /etc/storage/adbyby_host.sh source-list / hosts-list download path: validate each quoted HTTPS URL, use bounded unique staging, validate hosts syntax/size, build one complete hosts file, then atomically publish and preserve the previous list when any configured source fails. Do not accept a stale /tmp/host.txt after a failed request.

## Verification boundary

- L1 recovery: bootstrap/protocol order followed; exact main HEAD, remote branches/checkpoints, PR #1, live handoff/roadmap, newest 20 root dev-log entries and archive index inspected. Root log is 98/119, below 120-entry rotation trigger and 384 KiB early-rotation target; archive inventory remains 7/22/18.
- L2 stock managed Adbyby downloader: CLOSED on 42d4af7e6c0ceeb8508f1446b4228068318208dc; all six exact-head Actions passed.
- L2 third-party Adbyby rules: CLOSED on exact a8681f65733f08b7c932833fafe99062492f0a2a; all six exact-head Actions above passed. Adbyby logs confirm shell syntax and hosts-preservation, lifecycle-lock, managed-rules, third-party download, firewall snapshot tests, plus semantic checker all PASS. The AI handoff validator passed.
- No local full repository checkout was available; local sh -n, host tests and source checker are not claimed. Exact verification above is GitHub Actions host-side only.
- FTP upload cleanup OPEN/BLOCKED; NFS needs real-client evidence; current-main L3 firmware/TRX and L4 hardware NOT VERIFIED.

## Exact first action for the next GPT

Fetch live main and record the SHA. Confirm the AI docs validator sees 98/119 complete records. Before changing anti-ad behavior, trace every anti_ad_link producer/default/UI field and inspect real documented/sample upstream dnsmasq fragment syntax; keep the accepted grammar compatible with that source. Then implement private same-filesystem staging + shared HTTPS/CA/size/time + full-content validation + atomic rename + last-good behavior, add the no-network fixture and wire the exact host/source checker/workflow. Validate exact-head Actions before proceeding to host-list download hardening. Keep FTP/NFS/FRP and L3/L4 boundaries above intact.

## Completed task: Adbyby NVRAM-configured third-party custom rule download

On exact main SHA a8681f65733f08b7c932833fafe99062492f0a2a, all six triggered push Actions succeeded and every job/step/full log was inspected. Runs/jobs: Adbyby [43363263383 / 131681514167], infosvr [43363263454 / 131681514465], AI docs [43363263378 / 131681514145], AliDDNS [43363263594 / 131681514640], AliyunDrive [43363263430 / 131681513797], AdGuardHome [43363263365 / 131681513585]. The Adbyby job passed launcher/test shell syntax, hosts preservation, lifecycle serialization, managed HTTPS/MD5/cache rollback, third-party HTTPS URL/size/last-good publication, firewall snapshot and source/workflow semantic checks. AI docs validator reports 97/119 root entries and archive inventory 7/22/18.

Commit 315d738a580e154302dfdd9c57674ee64c3ada36 reuses the common HTTPS downloader for NVRAM user-configured third-party lists, validates and quotes URLs, stages each response privately, checks HTTP status/time/rate/size/CA, preserves the previous regular non-empty user.txt on failure, and atomically publishes the complete combined list before invalidating user.bin. Two intervening checker-only commits aligned its source contracts with centralized URL validation and wrapper-level umask 077; neither altered production behavior. Firmware curl is 8.21.0. This closes the custom third-party rule slice at L2 host/Actions, not L3/L4.

## Completed task: Adbyby lifecycle serialization

On exact main SHA `ceeb24d6b17e03f1777c240bdf0aca9fa6481194`, Adbyby [43346579501] / job [131627282482] passed shell syntax, all host regressions and the semantic caller/workflow checker. The regression covered lifecycle lock inode/permissions, symlink/hardlink rejection, bounded contention, non-interleaved stop/start, stop-failure gating, updateadb dispatch and FD-9 closure. The checker additionally validated managed hosts-entry preservation and the C RC restart caller. This closes the lifecycle serialization L2 task; it does not validate the new download code in THIS COMMIT.

## Completed task: Adbyby managed dnsmasq hosts-entry preservation

On exact main SHA `6fa50e11532ca5c68a9fdb3ffc95a8415841e794`, Adbyby [43336402301] / job [131593559948] passed shell syntax, extracted-helper regression, firewall snapshot/failure-cleanup regressions, and Python caller/path-filter contract. The helper removes only the exact anchored generated `addn-hosts=$PROG_PATH/hosts` line after escaping BRE-special path characters. Operator-owned addn-hosts, hostsdir, conf-file, address directives, comments, backup paths, duplicate managed lines, metacharacter paths, idempotence and missing-config behavior are covered. The other six exact-head workflows also passed: infosvr [43336402343], AliyunDrive [43336402492], AliDDNS [43336402424], AdGuardHome [43336402256], AI docs [43336402234], HTTPD security [43336402226]. All job steps/logs were inspected. This is L2 host/Actions evidence only, not target BusyBox/MIPS or live-router evidence.

## Completed task: FRP asynchronous watchdog restart dispatch

On exact main SHA edd2b4e231819fd748dea032324cd45904ddf172, check_frp() aggregates both enabled frpc/frps states before dispatching the shared restart script once. This prevents a second kill/restart while background daemon PIDs are still invisible. The unreachable WAN-failure log now occurs before its return, and the ping check uses POSIX if/status syntax. The host fixture deliberately keeps both PIDs absent and verifies both/one missing, alive, disabled and WAN-down cases. FRP [43334213696] / job [131586165339] passed shell syntax, lifecycle regression and semantic source/path-filter contract. AI docs [43334213673], infosvr [43334213527], AdGuardHome [43334213750], AliDDNS [43334213640] and AliyunDrive [43334213535] also passed on the same exact SHA; all steps/logs inspected. The preceding fixture version failed before assertions because it sourced the launcher under set -u without a positional argument; the fixture now supplies an explicit source-only argument. This is an L2 host/Actions result, not target BusyBox/MIPS or live daemon readiness evidence.

## Completed task: AliDDNS lifecycle process identity and bounded stop

Traced the HTTPD `variables.c` → PID 1 `rc.c` → `services.c` → `/usr/bin/aliddns.sh` control route. The previous `kill_ps()` selected processes with broad `ps | grep | awk | eval` matching and sent TERM/KILL without confirming exit.

Current implementation:
1. Removes generic `kill_ps()`; enumerate `/proc/[0-9]*/cmdline` and match only the exact launcher path followed by the final `keep` argument. Convert NUL separators to newlines and restore the trailing boundary that command substitution strips. Reject nonnumeric/reserved PIDs and the current shell.
2. Revalidate the exact argv immediately before signalling. Send TERM, allow at most five one-second intervals, send KILL only to still-matching keeper PIDs, then allow at most two more seconds and return failure if the keeper remains.
3. Make stop failure a hard gate: a failed close prevents status mutation or replacement keeper startup. Restart/start/stop/check still run through the stable-inode lifecycle lock and the RC restart remains one atomic locked action.
4. The host regression covers same-path/final-`keep` matching, current-shell exclusion, similar-path and wrong-action decoys, extra argv rejection, graceful TERM, bounded KILL escalation and stubborn-process failure preventing restart. The HTTPD umbrella contract now validates the exact keeper-stop helpers rather than requiring the removed grep-based `kill_ps`.

Intermediate CI failures were investigated, not hidden: the first regression failed because the test fixture used a literal dollar path instead of the test shell PID; the next exposed command substitution removing the trailing newline; the next semantic check itself was corrected to assert the final-argument pattern and literal `$$` current-shell exclusion. Current code/test/checker exact-head run [43306180515] is green. The HTTPD workflow's obsolete `kill_ps "$ALIDDNS_HELPER"` assertion was replaced with checks for `aliddns_stop_keeper`, `aliddns_keeper_pid_matches` and absence of `kill_ps`; HTTPD run [43305744811] is green on the same production source tree.

