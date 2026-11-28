## 2026-11-28 — Validate Redsocks generated SOCKS5 endpoints before restart

- `func_start` wrote caller-supplied address and port directly into a privileged Redsocks config heredoc. An address/port containing a newline could inject additional directives. It also stopped a healthy daemon before discovering that a missing config had no endpoint or invalid values.
- Added strict IPv4 validation, decimal port normalization/range enforcement (1–65535), and a preflight before process-stop operations. Missing config with no endpoint remains a safe no-op; malformed input leaves a running instance untouched. Regression covers newline injection, invalid/octal IPv4, port injection, zero/out-of-range/non-numeric ports and leading-zero normalization.

---

## 2026-11-28 — Reject symlinked Transmission storage roots

- `func_start` previously checked only that `/mnt/transmission` was a directory before deriving and permission-changing its config/download descendants. A symlinked storage root (or symlinked immediate parent) still passed the directory test and allowed privileged chown/chmod to follow the link into attacker-controlled storage.
- Added a dedicated root preflight that rejects a symlinked root or parent and preserves the previous missing-root failure behavior. Regression covers a symlinked root, a symlinked parent, a real root and a missing root without creating targets.
- SmartDNS directive-injection Actions are still in progress at SHA `380918ce60cb7d955d57b90ca42868b1df964de5`; this Transmission slice is independent and is validated by the process lifecycle workflow.

---

## 2026-11-28 — Preserve legacy empty SmartDNS threshold while disabled

- Reviewing the SmartDNS DNSMasq integration fixture exposed that `sdns_ip_change_time` is a conditional directive value, not an always-emitted numeric option. Validate it only when `sdns_ip_change=1`; otherwise older NVRAM without this optional value remains compatible. Added regressions for disabled empty and enabled empty threshold states.
- The previous exact DNSMasq run on `53c09530c676ef0096ca2a89a2175212cec47fd0` showed the integration fixture omitted the production config-token helper; commit `a0cba1e2efe18728173493e0fa74b0b3b4956854` now sources it. Re-run the full DNSMasq workflow to validate both corrections.

---

## 2026-11-28 — Load SmartDNS config-token helper in DNSMasq integration regression

- Exact DNSMasq lifecycle run #46085311741 for `53c09530c676ef0096ca2a89a2175212cec47fd0` passed the process, adblock, firewall and resolver ownership checks, then failed the final runtime config integration because its extracted-function fixture sourced the DNSMasq helper but not the newly required `smartdns_config_file.sh` token validator. The production launcher already loads that helper; the fixture now mirrors the production dependency.
- The exact-SHA run verified the new token-validation code up to this fixture failure. Re-run DNSMasq lifecycle after this test-harness repair; AI handoff #46085311728 passed. Process inventory, USB storage and XY-C1 were still in progress at the last poll.

---

## 2026-11-28 — Reject SmartDNS NVRAM configuration-line injection

- `Get_sdns_conf` interpolated base settings and static-upstream address/group/port values into daemon directives without enforcing single-token boundaries. Newlines could append attacker-controlled SmartDNS directives. The generator now validates base tokens, numeric fields and selectors before truncating staging, then validates static upstream endpoints and options before emission.
- Added regressions for injected server-name, speed-check-mode, cache-size, static endpoint, group and port values. The DNSMasq lifecycle workflow runs this generator test; exact-SHA verification follows the commit.

---

## 2026-11-28 — Align checked NVRAM startup contracts with CI fixtures

- The FRP boot-state hardening changed the shared autostart reader to `nvram get_checked`, but the optional-start fixture still accepted only legacy `get`. Its selectors consequently appeared disabled and AdGuardHome was not invoked. Updated the fixture to model the checked API.
- The AliDDNS transport checker also expected the old unchecked command in the shared autostart helper; aligned that contract. Added part-039 to the archive index table with the repository-root-relative path required by the inventory checker.
- Exact base SHA `71d3bf2a3788f2ce59c1e5809c383a26841b2657`: Process lifecycle #46084656129 failed only at the stale autostart fixture; AliDDNS #46084656095 failed only at the stale source assertion. Re-run both against the repair commit.

---

## 2026-11-28 — Complete DHCPv6 crash-durable configuration publication

- DHCPv6 IA configuration publication fsyncs staged contents after `fflush`, closes the file, atomically renames it, then fsyncs the parent directory. Post-rename parent-sync errors are reported because crash durability is uncertain although the new name may already be visible.
- The staged-config regression covers successful publication, missing-parent and symlink-parent paths, and injected parent-directory-sync failure. The initial exact-SHA run on `8b1aa83d2f82da8743350bdc2aa056ebfa764649` exposed a test expectation mismatch after the injected post-rename failure; `0eef8eb09fd535d6266426ec1fa8a0b5d3ed8093` corrected the expected on-disk content. Inspect the exact-head rerun and companion WAN IA callback regression before claiming PASS.
- Concurrent WAN IA callback changes were preserved. No firmware L3 or physical L4 result is inferred from workflow completion alone.
- Next: inspect exact-head DHCPv6 publication/WAN IA tests, Process inventory and AI handoff; close outstanding XY-C1 Build → TRX → artifact chains for modem/OpenVPN, then continue an independent P1 issue.

---

## 2026-11-28 — Make USB modem identity metadata durable

- `modem_node_file_write()` already fsynced the staged metadata file but omitted parent-directory fsync after atomic rename. `modem_node_file_remove()` omitted it after unlink; power loss could lose a new USB identity or resurrect stale `wwanX`/WDM metadata. The directory prepare path, publisher and remover now sync their containing directory and propagate failures.
- Extended the existing C regression for successful/missing/symlink parent behavior and added source-order assertions to the USB modem identity workflow.
- Parent SHA: `48b617d5933f9b7bb88fe8ed194b1682985d3d7d`. Exact USB modem identity, Process inventory, AI docs and XY-C1 Build/TRX/artifact pending; no local compile claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Persist OpenVPN credential and config publication

- OpenVPN client credential and config publishers previously fsynced staged file contents but did not sync the containing directory after rename. Both now call the shared parent-directory fsync helper and propagate errors; the helper refuses symlinked parent directories via `O_NOFOLLOW`.
- Extended both ASan/UBSan C regressions for successful parent sync, ENOENT missing-parent failure and symlink-parent rejection. OpenVPN workflow push/PR filters and source checker require the common helper and sync-after-rename ordering.
- Parent SHA: `00234eecd99526edb36f7a91581ec0614716c085`. Exact-head OpenVPN/Process inventory/AI docs/XY-C1 Build/TRX/artifact checks pending; no local compile claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Repair media-config directory sync CI contract

- e0 Process regression #46082405947 failed at the existing MiniDLNA/Firefly source contract because it expected the pre-hardening `O_RDONLY | O_DIRECTORY` open form; the shared helper now correctly uses `O_RDONLY | O_DIRECTORY | O_NOFOLLOW`. The newly added MiniUPnP and IGMP compile/regression steps were skipped by the earlier assertion failure.
- Update the static contract to require `O_NOFOLLOW`; exact-SHA rerun will execute the new proxy configuration tests.
- Base SHA: `e0cc8d5834cc26b2527176a0e0fd1dfd1095f8dd`. Process inventory #46082405929 and AI handoff #46082405924 PASS. e0 XY-C1 #46082405926 still pending Build/TRX/artifact; NVRAM audit #46082405853 still running.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Persist MiniUPnP and IGMP proxy config renames

- MiniUPnP and IGMP proxy staged configuration writers fsynced file contents before atomic rename but did not sync the parent directory. They now reuse the shared parent-directory fsync barrier after rename and propagate errors instead of reporting success when the directory entry is not known durable. The helper refuses symlinked parent directories with `O_NOFOLLOW`.
- Extended both C regressions for successful parent sync, missing-parent failure and symlink-parent rejection. Added MiniUPnP C compile/run to Process lifecycle workflow; IGMP proxy compile/run remains, and a static contract checks each publisher performs the directory barrier after rename.
- Parent SHA: `91db105a946c0397c9875a1090cfc8fa1d0495fd`. Exact-head Process regression/Process inventory/AI docs and XY-C1 Build/TRX/artifact pending; no local compile claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Persist MiniDLNA and Firefly config renames

- MiniDLNA and Firefly each fsynced staged config contents before atomic rename but did not fsync the containing directory. File fsync alone does not guarantee the new directory entry survives sudden power loss. Added shared `config_file_sync.h` with EINTR-safe file and parent-directory fsync; both publishers now call it after rename and propagate failures.
- Added success and ENOENT missing-parent cases to both C regression programs. Process lifecycle workflow checks that directory sync follows rename and compiles/runs the existing ASan/UBSan tests. The new shared header is included in push/PR path filters.
- Parent SHA: `d3090c3a55ebda73b2680055e04f7324bfe56565`. Exact-head Process inventory, AI docs and XY-C1 Build/TRX/artifact pending; no local compile claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Enable BusyBox ash command builtin for firmware lifecycle scripts

- Storage #46081098105 first exposed Transmission's `command -v stat` failure; after applet PATH diagnostics, #46081453045 on `e69afd9` made the root cause explicit: BusyBox ash reported `command: not found`. The common board config disabled `CONFIG_ASH_CMDCMD` even though Transmission, Aria2 and shared process lifecycle scripts use `command -v`.
- Enabled `CONFIG_ASH_CMDCMD=y`; Storage CI now requires the feature in the built and source config and runs an ash smoke through the exact `STUB_DIR:$PATH` before starting the launchers. Exact SHA `25b0f34f41206b0d16ceed7b8190202a53b89d63`: USB storage #46081670821 PASS (including BusyBox 1.24.2/ash and all remaining storage regressions); Process #46081670853 PASS; AI docs #46081670826 PASS. AliDDNS #46081670844, AliyunDrive #46081670881, DNS Forwarder #46081670945, FRP #46081671085 and AdGuardHome #46081670829 PASS. NVRAM #46081670831 and XY-C1 #46081670815 were still running; Build XY-C1 was active and TRX/artifact pending.
- Test harness: `9bf676f` linked stat in STUB_DIR; `7b05465` added an exact ash PATH smoke; its duplicate symlink was removed in `e69afd9`. No production fail-closed check was relaxed.
- No exact-current-source L3 firmware claim until #46081670815 Build, TRX and artifact upload all pass; no hardware L4 result.

---

## 2026-11-28 — Make BusyBox stat visible in storage launcher stubs

- Exact d79 USB storage #46081098105: Aria2 directory/file symlink regression and Transmission/Aria launch-status regression PASS; the BusyBox 1.24.2/ash step compiled BusyBox, ran Optware lifecycle tests, then failed at Transmission process identity because `command -v stat` returned unavailable. Workflow applet PATH already included stat and direct smoke check, but the test harness prepends a separate `STUB_DIR` to the BusyBox launcher's PATH, so the actual child could not resolve the applet.
- Link the actual built Busybox binary as `STUB_DIR/stat`; add an ash smoke using the exact `STUB_DIR:$PATH` form. No production logic or fail-closed checks are weakened. d79 AI docs #46081098106 PASS; d79 XY-C1 #46080742734 Build was IN_PROGRESS, TRX/artifact pending. eab6 XY #46080815493 reports success but its Build/TRX/artifact steps were skipped, so it is not an L3 result.
- Parent SHA: `9bf676f90ae2ee58dcaef408015d57fd32bbf056`. Exact USB/ash, Process inventory, AI docs and XY-C1 checks pending. No local ash test claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 — Block Aria2 storage symlink redirection

- Aria2 startup previously followed symlinks for `config`, the download directory and `incomplete.lst`; privileged startup can touch session files, publish RPC secrets/configuration and recursively chmod/chown these paths. Added directory checks for existing/dangling symlinks and rechecking after `mkdir -p`, plus regular-file checks for `aria2.conf` and `incomplete.lst` before `touch`. The configured download path is checked before recursive chmod.
- Added `tests/storage_path_symlink.sh` with real/new path, existing/dangling symlink and sentinel-preservation cases, and verified guard order before side effects. Wired into storage-hotplug push/PR CI.
- Parent SHA: `284e35183959b8cc78d28c82959e5e8815774531`. Exact storage-hotplug/Aria process/Process inventory/AI docs/XY-C1 checks pending; no local shell test claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 07:00 CST (UTC+8) — Make iperf3 process discovery fail closed

- `iperf3_process_state()` previously accepted BusyBox `pidof` status 1 with empty output as confirmed ABSENT. BusyBox 1.24 can omit a running task when its /proc scan is incomplete, allowing a duplicate iperf3 start or premature stop success.
- Reused the always-built shared `process_lifecycle.sh` tri-state scanner and its full-procfs/post-scan confirmation. Added fake-procfs integration coverage for RUNNING, UNKNOWN, stop verification, and confirmed ABSENT. Exact workflow #46079911385 exposed a test-only permission error (tracked launcher mode is not executable); follow-up `6d659d767e8ac236527ca5cf02f2daedaf386507` runs it through `sh`.
- Source commit: `3832167000112d33e6c2d30dbb8c0008ba29e119`. Test correction commit: `6d659d767e8ac236527ca5cf02f2daedaf386507`. Exact iperf3 rerun #46079981839 and Process inventory #46079981918 were queued; source-SHA inventory #46079911367 passed. XY-C1 #46079911435 was still building; no firmware artifact or hardware L4 claim.
- The independent Transmission TLS check passed on `9972e8531add4ce5bc64c3d5690892e30b80ade4`; its broader Process workflow was cancelled after that step. USB storage hotplug #46079785505 separately failed during BusyBox 1.24.2/Optware ash regression and remains to diagnose. No local shell test was executed in this connector-only session.

## 2026-11-28 — Restore Transmission HTTPS tracker certificate verification

- `transmission.sh` previously exported `TR_CURL_SSL_NO_VERIFY=1`; Transmission 3.00 treats this variable as an opt-out from HTTPS tracker certificate and hostname validation. The launcher now clears any inherited value before startup and keeps verification enabled by default.
- Added `tests/tls_verification.sh`: it injects the opt-out into the environment, sources the exact launcher preamble, verifies the variable is cleared and statically rejects any forced assignment. Wired into Process lifecycle push/PR checks. The XY-C1 workflow already watches the launcher.
- Base SHA: `20e0a312051addf538ed6bf4ae85022e608ed4b3`. Exact-head Transmission/Process/AI and XY-C1 Build/TRX/artifact checks are pending. No local shell test claimed.
- Commit: THIS COMMIT.

---

## 2026-11-28 03:55 CST (UTC+8) — Correct Shadowsocks procfs source contract and handoff byte guard

- Exact SHA `f23be0cff7d7e0d1e942ebcf10094497b73be676`: Shadowsocks lifecycle #46079286423 passed shell syntax and all runtime regressions, then failed only because the Python source checker expected incorrect escaping for the literal procfs path expansion and `tr` delimiter. This checker-only correction now matches the actual shell source.
- AI handoff #46079286331 failed because the roadmap exceeded its UTF-8 hard limit by 317 bytes; compressed the new top roadmap status heading. No production behavior changed.
- Process inventory #46079286495 PASS. The corrected checker and documentation contract require new exact-push runs. XY-C1 #46079286427 is still the required production-source L3 gate; no local test or hardware L4 result is claimed.


## 2026-11-27 — Confirm Shadowsocks ABSENT lookups with complete procfs scans

- BusyBox 1.24 `procps_scan` skips entries when stat or cmdline reads fail; `pidof`/`pgrep` can consequently return status 1 despite a live matching process. Shadowsocks previously accepted that as ABSENT.
- Added a mode-aware procfs inventory for named and `pgrep -f` lookups. It validates numeric PID/stat records, live task state and cmdline readability; mirrors BusyBox short-comm/argv0/argv1 name matching and pgrep's empty-cmdline comm fallback; detects malformed/inaccessible entries as UNKNOWN; and performs a fresh lookup after a clean zero-match scan.
- Added fake-procfs regressions for both false-negative lookups and malformed stat input. Exact-SHA Actions pending; no local shell test claimed.

## 2026-11-27 — Bind Redsocks stop signals to captured process identities

- Replaced Redsocks name-wide `killall` TERM/KILL with exact-PID signalling. Each candidate must match the configured executable via `/proc/PID/exe`, match the process comm, and retain the same `/proc/PID/stat` field-22 start time across separate reads. Stop retains the first PID/start-time snapshot and refuses signalling if a new PID appears or a PID is recycled before either signal.
- Extended `redsocks-lifecycle.yml` regression coverage for exact PID signals, foreign same-comm executable rejection and PID reuse between TERM and KILL.
- Exact-SHA Actions on 7e1557690aa6aa996007b5ed4e1aecd170b44eed: Bash syntax and Redsocks runtime regression PASS; static source contract FAIL was a checker bug (stat field 22 belongs to `redsocks_read_stat_identity`), corrected in the follow-up. Process inventory PASS; AI docs FAIL was roadmap byte-limit overrun, shortened in the follow-up. No local shell run is claimed. Linux 4.4 has no pidfd, so a small validate-to-signal race remains; physical L4 is NOT VERIFIED.

## 2026-11-27 — Reject symlinked SSH host-key storage directories

- Both OpenSSH and Dropbear previously validated individual host-key leaf paths but accepted a configured storage-directory symlink. A root-started daemon helper could therefore chmod, delete, generate or publish keys/configuration in an unintended target directory. Added no-follow directory checks and post-mkdir revalidation; all key/config/argument operations are gated before they follow the configured leaf directory.
- Extended Dropbear runtime coverage to symlink, dangling-symlink and non-directory stores, including preservation of outside keys/extra_args and no keygen/start. Extended OpenSSH launch regression to prove a symlinked store cannot mutate external host keys or publish sshd_config. Added exact source-contract assertions.
- Process lifecycle, AI handoff and XY-C1 Actions are required on the exact implementation SHA. Local full-repository execution is unavailable in this GitHub-connector-only session; do not claim local shell results or physical-router L4.

## 2026-11-27 — Correct OpenSSH required-host-key source contract

- Process lifecycle regression #46073989322 on `2ffccd914287ad96840fd92aeb850ed49c828f56` reached `check_optional_app_start_status.py` after preceding lifecycle and compiled regression cases passed, then failed because its OpenSSH source assertion expected a `case` statement that the implementation does not use. The intended branch is an explicit `if [ "$key_requirement" = "required" ]`; the omitted-DSA generation fixture already verifies that missing required keys prevent sshd launch.
- Updated the checker to assert the actual required-key branch and its missing-key diagnostic. No product source behavior changed. Exact-push Process regression on the new commit is required; host execution is not available in this connector-only session.
- On the source baseline SHA `2ffccd9`, Process inventory #46073989223 PASS and XY-C1 Firmware Validation #46073989253 PASS, including Build XY-C1, TRX verification, and artifact upload. Physical-router/L4 remains NOT VERIFIED.

## 2026-11-28 — Quote AliDDNS provider TTL data before testing or persisting defaults

- The AliDDNS startup block used `[ -z $aliddns_ttl ]` and an unquoted `nvram set` expansion. Since NVRAM can be written by scripts or direct requests, values containing spaces or glob characters could be split or expanded by the shell.
- Added a quoted default helper preserving non-empty values and applying 600 only to an empty value. Added regressions for empty, wildcard, whitespace and numeric TTL inputs; static checker pins the helper and quoted persistence.
- Exact-SHA AliDDNS transport/AI Actions pending; local shell execution unavailable in this connector-only session.

## 2026-11-28 — Align XUPnP source checker with shared DNSMasq pair transaction

- DNSMasq exact-SHA run #46065129548 on `b0ccae9ca4fae122093a48790211f8526278a129` completed the `dnsmasq_file.c` GNU89/Werror/ASan/UBSan compile and runtime regression successfully, then failed in `scripts/check_xupnpd_config_pair.py` because the checker still expected pair-specific backup/restore calls in `dnsmasq_file_publish_pair()`.
- Updated the checker to verify that the XUPnP pair wrapper delegates both original path/backup tuples to the shared two-member set transaction, and that the set helper creates per-destination snapshots, includes a failed-but-renamed member in rollback, then restores published paths in reverse order. This is checker-only; production source is unchanged.
- Exact push rerun required on this commit. Host-side execution unavailable in this connector-only session.

## 2026-11-28 — Roll back DNSMasq outputs whose rename completed before a durability error

- `dnsmasq_file_publish_internal()` reports a parent-directory fsync failure after a successful `rename()`, correctly exposing that the new pathname is already visible but not durably committed. The multi-file `dnsmasq_file_publish_set()` and `dnsmasq_file_publish_pair()` callers previously incremented their rollback count only on a zero return, omitting the failing-but-renamed output. That could leave a mixed generation of DNSMasq configuration after an error.
- Added `dnsmasq_file_publish_set_with_ops()`; it tracks `rename_completed` independently from the callback return and rolls back the failed member plus prior successful members in reverse order. The public pair helper now routes through the same transaction implementation.
- Added GNU89 host regressions that inject EIO after the first pair rename and after the second of three set renames, then verify every prior destination content is restored. Extended the DNSMasq workflow's source contract to pin the rollback accounting. Exact-SHA Actions pending; local compiler execution unavailable here.

## 2026-11-28 — Add WAN IPv6 teardown header to XY-C1 build path filters

- Added trunk/user/rc/src/wan6_teardown_policy.h to the XY-C1 push and pull-request path filters because it is included by the production WAN IPv6 implementation. A header-only change to this helper must trigger the cross-build.
- Added a detector unit test requiring exactly two workflow entries and ensuring it is treated as a firmware input. No production behavior changed.

