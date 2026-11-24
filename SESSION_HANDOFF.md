## Current repository state

- Live parent `main`: `cbbb27636807b7f195d7d87ccefb476366a07995`; the next commit is an atomic checker/documentation correction based on this exact tree. Main may advance independently; re-fetch and compare before each write.
- Router kernel hostname/domain propagation fix is in `a26746d033ffa19d9b576010f4fe4a583b4a859a`: `update_hosts_router()` now uses the tested `lan_kernel_names_apply()` policy and returns kernel setter failures through the existing DNSMasq preparation gate.
- AdGuardHome process identity hardening is in parent `cbbb27636807b7f195d7d87ccefb476366a07995`: numeric pidof candidates are filtered by exact `/proc/PID/exe`, full config/workdir argv and non-zombie `/proc/PID/stat` start_time; TERM and refreshed SIGKILL snapshots revalidate the identity immediately before signaling.
- Adbyby executable/start_time hardening exact Adbyby #45339125309 and Process fail-closed #45339125213 PASS on `17d08de1ed22fe2ff102992d28c974b42b77de3a`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains immutable at `72f4f157a084f8184b9597ca5bf334b8080e5807`.

## Active task

Align the HTTPD workflow's DNSMasq hostname-pair source contract with the checked router setters:
- Previous HTTPD exact run #45339568430 on `a26746d` failed at a stale assertion expecting a direct `sethostname(lan_hname, ...)` call inside `update_hosts_router()`.
- The new implementation deliberately calls `lan_kernel_names_apply(lan_hname, lan_dname, &wan_kernel_name_ops)` after atomic pair publication; each adapter returns positive errno/EIO, both setters are attempted, and the first error is returned after lifecycle lock release.
- This commit changes only the HTTPD workflow contract and handoff/log evidence; it must not relax runtime coverage.

## Completed development slices

- AdGuardHome identity hardening (parent `cbbb27636807b7f195d7d87ccefb476366a07995`): host tests/source contract added for exact executable, argv tuple, procfs metadata and PID reuse; Actions are listed below.
- Router kernel hostname/domain status propagation (`a26746d`): shared policy, positive error propagation, Process workflow path filter/ASan regression; exact Process, WAN DHCP, WAN route, DNSMasq, etc. pass.
- Adbyby process identity hardening (`17d08de1`): exact binary and process start_time validation before stop signaling; dedicated host and Process regressions PASS.

## Verification boundary

- Parent `a26746d033ffa19d9b576010f4fe4a583b4a859a`: Process fail-closed #45339568206 PASS, including LAN kernel-name status regression; Process inventory #45339568157 PASS; AI docs #45339568440 PASS; WAN DHCP #45339568432 PASS; WAN route #45339568336 PASS; DNSMasq #45339568424 PASS; VPN #45339568175 PASS; router NVRAM #45339568171 PASS; WAN ARP #45339568324 PASS; USB modem #45339568348 PASS; miniupnpd #45339568380 PASS; ZeroTier #45339568346 PASS; Wireless #45339568118 PASS; IPv6 #45339568388 PASS; watchdog #45339568182 PASS; detect_link #45339568318 PASS; detect_internet #45339568405 PASS.
- HTTPD #45339568430 FAILED only on the stale hostname source assertion, corrected here. XY-C1 #45339568161, Firewall #45339568335 and WAN teardown #45339568351 were still IN_PROGRESS at last poll. No same-SHA target firmware/TRX/artifact result was available, so L3 is NOT VERIFIED.
- Parent `cbbb27636807b7f195d7d87ccefb476366a07995`: AdGuardHome config security #45339988745 PASS; AI docs #45339988750 PASS; Process inventory #45339988758 was IN_PROGRESS at last poll. Exact reruns required after this checker correction.
- No physical-router L4 result is claimed. Linux 4.4 has no pidfd; the Adbyby and AdGuardHome identity checks still have the documented final validation-to-signal race.

## Exact first action for the next GPT

1. Inspect the exact-SHA HTTPD security, AI docs and Process runs spawned by this checker correction; fix any first failure.
2. Poll XY-C1 #45339568161 on `a26746d` plus Firewall #45339568335 and WAN teardown #45339568351; claim L3 only if Build, TRX verification and artifact upload all succeed on the same production source SHA.
3. Inspect completion of AdGuardHome config-security/Process inventory on `cbbb276` and continue the first evidence-backed P1 gap outside already-completed Adbyby/SmartDNS/RAETH scopes.
4. Keep the backup immutable and all main writes as complete-tree atomic commits with expected-SHA non-forced ref updates.

# Cross-session execution mandate

Treat this as continuous work across GPT conversations. Recover once, then execute on `main`; each logical source or CI change is one atomic main commit. Create all changed blobs, one complete Git tree and one commit before updating the branch with a non-forced expected-SHA compare-and-swap. Never force-push or modify the frozen backup.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST perform a handoff checkpoint before ending. A relevant exact-head Action changing from PENDING to PASS/FAIL is checkpoint information. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit.

Every session MUST inspect live repository state before ending. Record only material exact-head Action transitions, new source commits, genuine unresolved failures or a changed first action. Do not manufacture documentation churn or no-op commits.

## Development loop

- Source modification → targeted regression → one complete Git tree/one atomic commit → exact-SHA Actions.
- PASS advances; FAIL is diagnosed and fixed; PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc compatibility, router configuration, security, architecture and backward compatibility.
- Before context exhaustion, record exact latest SHA, tests, CI state, unresolved issues and one concrete first task. Never claim unverified work.
