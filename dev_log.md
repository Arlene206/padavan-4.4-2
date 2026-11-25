## 2026-11-25 — Include DHCP domain codec in XY-C1 firmware validation paths

- The new decoder sanitizer workflow passed on `66efb19`, but review of the XY-C1 workflow exposed a production-validation blind spot: `networking/udhcp/domain_codec.c` was missing from the push path filter, so edits to this shipped BusyBox source did not trigger the MIPS firmware build/TRX/artifact job. Add the source to the path filter and pin the one-entry filter plus production classification in `test_xy_c1_firmware_inputs.py`. The workflow's own path is already included, so this commit re-triggers XY-C1 against the complete current tree.

---

## 2026-11-25 — Initialize domain decoder return pointer deterministically

- Remove the legacy self-initialization `char *ret = ret`, which reads an indeterminate automatic pointer before the decoder assigns its successful result. Initialize it to `NULL` and add a null-buffer rejection case alongside zero/negative length checks.

---

## 2026-11-25 — Make domain-codec sanitizer harness leak-clean

- The domain-codec sanitizer progressed past compilation after the validated signed/unsigned input-length change, then LeakSanitizer exposed allocations intentionally printed but never freed by the legacy demonstration `main()`. Replace print-only demos with checked decode/encode round-trip helpers that release every buffer; retain valid no-compression, suffix-compression and whole-name-compression coverage.

---

## 2026-11-25 — Cover zero and negative domain input lengths

- Extend the domain-decoder regression to exercise the new API guard with zero and negative lengths as well as malformed wire payloads. The input pointer remains valid in these cases, isolating length validation.

---

## 2026-11-25 — Validate decoder length and use unsigned-safe bounds

- The first domain-codec workflow exposed three pre-existing signed/unsigned comparison warnings in the decoder's unsigned cursor versus signed length bounds checks (the regression workflow treats warnings as errors). Harden the API boundary: reject null inputs and non-positive lengths, then compare cursor offsets against the validated unsigned length. This removes the warnings without weakening compiler diagnostics and makes negative caller lengths fail closed.

---

## 2026-11-25 — Reset domain completion for every new label

- Review of the first sanitizer slice found the per-name completion flag also must be cleared whenever a new label begins after a previously terminated name. Without that reset, a valid first search-domain entry could mask a truncated final entry in the same option. Reset the state on every label and add a two-entry regression (valid first name, unterminated second name); normalize diagnostic newlines in the embedded test harness.

---

## 2026-11-25 — Reject truncated and forward RFC1035 domain compression

- `dname_dec()` previously accepted a domain that reached the end of its option buffer without an encoded root terminator. In particular, a label followed by a compression pointer whose target was beyond the option length could be returned as a valid but truncated name. The decoder also followed forward/self-referential compression targets even though RFC1035 pointers are backward references; the existing jump-depth cap prevented unbounded loops but did not enforce the wire-format rule.
- Track explicit completion of each expanded name and reject any path that reaches the buffer end without a root terminator. Require each compression target to precede the pointer. Added sanitizer-backed cases for valid backward compression, truncated labels, out-of-range targets and forward targets. Exact SHA workflow run pending; no local compiler result claimed.

---

## 2026-11-25 — Anchor DHCPv6 interface contract at the function definition

- On exact SHA `14e4051`, the GNU89 ASan/UBSan policy binary passed, but the DHCPv6 source checker failed before its new assertion: its generic function parser matched the `if6init()` forward declaration, then encountered the intervening enum brace instead of the function body.
- Anchor the check at the second occurrence (the definition) before using the existing body parser. This is a checker-only correction; the production diagnostic fix remains unchanged. Exact-head rerun required.

---

## 2026-11-25 — Use initialized interface name in DHCPv6 startup failure

- The DHCPv6 client already terminates when `if_nametoindex()` returns zero, but the fatal diagnostic formatted `ifp->ifname` before that pointer was initialized (the assignment occurs after the guard). This makes the error path depend on an uninitialized pointer and can emit a bogus interface name.
- Keep the existing fail-fast behavior and report the original `ifname` argument instead. Extend the DHCPv6 source contract to pin the read → zero-index guard → fatal diagnostic → pointer assignment order. Exact-head DHCPv6 workflow required; no local test is claimed.

---

## 2026-11-25 — Bound optional SmartDNS adblock downloads

- `Start_AD` already limited connection establishment to 10 seconds but left transfer time unbounded; a user-configured or stalled adblock endpoint could therefore hold SmartDNS startup indefinitely. Add a 30-second per-attempt maximum and reduce retries from three to one, bounding worst-case transfer wait while keeping one transient retry.
- Add an exact-source contract for connection timeout, transfer timeout and retry count. SmartDNS lifecycle and source contract rerun required; this is independent of the in-progress XY-C1 validation on the previous product-source SHA.

---

## 2026-11-25 — Align Redsocks checker with split function braces

- Exact Redsocks run on `dd12814` passed the full shell regression matrix, including invalid-address preservation, duplicate draining, check/delete error propagation and partial rollback. The source-contract step then crashed with `ValueError` because it searched for literal `func_clean(){`, while production uses `func_clean()` followed by a newline and `{`.
- Replace the brittle substring lookup with a line-anchored regex accepting whitespace before the opening brace. This preserves the start-function boundary and all semantic assertions; exact-head rerun required.

---

## 2026-11-25 — Isolate Redsocks cleanup-failure trace assertion

- Exact Redsocks run #45555165127 on `437fd9e` passed invalid IPv4 rejection, duplicate-draining/idempotent reapply, and iptables check-error handling. The delete-error test then falsely detected `-N REDSOCKS` from the earlier successful setup because it reused the same diagnostic log. Clear only the command trace before injecting the delete failure; keep the existing rules and chain state intact so the assertion proves no new chain is created after cleanup failure.
- Exact-head regression rerun required. Production source unchanged; no local test result is claimed.

## 2026-11-25 — Correct Redsocks mock rule keys after chain parsing

- Exact `7cfd647` regression still failed because the stateful mock stored `table|chain rule` using an explicit `$chain` prefix while `$rule="$*"` already contains the chain token. This left an invalid doubled key and the test observed zero PREROUTING jumps. Updated check/delete/append and append-fault matching to use the complete original argv once (`table|$rule`).
- This correction is isolated to `scripts/tests/redsocks_lifecycle.sh`; production policy remains unchanged. Exact-head Redsocks CI must now validate the full test matrix.

## 2026-11-25 — Restrict downloaded native SmartDNS filters to address rules

- The native SmartDNS path previously copied the entire downloaded file whenever it found one `address=` line. A mixed or compromised list could therefore inject unrelated daemon settings such as `server`, `bind` or nested `conf-file` directives into the active SmartDNS config.
- Whitelist native filter content to address directives, comments and blank lines; reject any other directive and preserve the previous output on failure. Add mixed-content regression and exact-source contract. Exact SmartDNS lifecycle, DNSMasq regression, AI docs and any source-triggered firmware checks are pending; no local test or firmware result is claimed.

---

## 2026-11-25 — Correct Redsocks mock newline escaping

- Exact review of the preceding fixture correction found its generated `printf` format strings contained two literal backslashes before `n`, rather than the shell's single escape. That would store the rule key with a literal backslash and letters `n`, not terminate the record, so the mock still could not validate its own rule table.
- Normalize both command trace and rule storage to a single `\n` format escape. Keep complete `table|chain rule` identity; production source unchanged.

---

## 2026-11-25 — Preserve complete chain identity in Redsocks firewall mock

- The next fixture review found a second inconsistency after clearing the escaped CHECK/DELETE controls: mocked `-C/-D` already keyed rules as `table|chain rule`, but `-A` still omitted the chain. This made valid additions invisible to the mock's own lookup/count/flush logic. Updated `-A` storage and command traces to retain chain identity consistently.
- This is regression-fixture-only; no production code change. Confirm exact Redsocks run after this correction.

---

## 2026-11-25 — Fix Redsocks mock's escaped failure controls and chain keys

- The diagnostic exact-head Redsocks run confirmed the initial failure was status 4 from the mock's very first `iptables -C PREROUTING`: a backslash left the CHECK failure-injection expression literal, so the mock always returned an error. The adjacent DELETE failure control had the same escaping defect.
- Removed the unintended escapes from both controls and made mocked `-A` / trace output retain the chain as well as the table and rule. The full rule key now agrees with the existing `-C/-D/-F/-X` model and the test's duplicate-count assertions. Production source remains unchanged; rerun required.

---

## 2026-11-25 — Correct Redsocks firewall regression mock command handling

- Exact Redsocks Actions #45554742701 on `e51171f` exposed a test-fixture defect: the firewall mock escaped its failure-injection variable, so every `iptables -C` returned synthetic status 4; it also omitted the chain name from stored rule keys, conflicting with flush/count/rollback expectations.
- Unescaped the mock's test-only failure controls and made `-C/-D/-A` use the complete `table|chain rule` identity. This changes only regression fixture behavior; production firewall policy is unchanged. AI handoff docs #45554742775 PASS; exact Redsocks rerun required.

---

## 2026-11-25 — Diagnose Redsocks initial NAT apply regression

- Exact Redsocks Actions run #45554360655 on `87a3eea` passed shell syntax and rejected the octal REMOTE_IP correctly, then failed the first valid firewall apply. Added the mock's command trace and exact return status to the regression failure path so the failed iptables operation can be identified without weakening assertions.
- The 100-record log rotation is committed in `614c30609b212a0fa9ad6db23a9f8bf1573020d3`; root log now has 20 records and archive part-034 preserves 100 oldest records byte-for-byte.

## 2026-11-24 — Redsocks NAT policy idempotence and failure propagation

- func_clean previously parsed human-readable iptables -L --line-numbers output by stripping every digit and deleting a guessed line number; cleanup statuses were ignored. func_iptables also ignored -N/-A failures and appended duplicate chain rules/jumps on reapply.
- Replaced line-number parsing with exact iptables -C/-D draining (64-delete bound), checked -S/-F/-X, and checked append helpers with rollback for partial policy publication. REMOTE_IP is validated as canonical decimal IPv4 before the old firewall is touched; invalid/octal aliases fail closed. ipset population and zone downloads now propagate errors rather than publishing the completion sentinel after failed population.
- Extended the existing Redsocks lifecycle regression and source contract with duplicate-jump cleanup, idempotent reapply, invalid IPv4 preservation, check/delete error propagation, and partial-append rollback. Exact-head Redsocks Actions pending; no local shell or target build is claimed.

## 2026-11-24 — Update SmartDNS startup rollback count contract

- DNSMasq run #45553387354 passed the shell regressions and revised stop-order assertions, then found one stale source-contract count: checked firewall setup adds a new failure branch to the existing seven SmartDNS startup rollback sites. Updated the exact expected count to eight.
- AI docs #45553386628 passed for the parent SHA. DNSMasq and XY-C1 verification remain pending on the corrected source-contract SHA.

## 2026-11-24 — Align the DNSMasq source contract with safe SmartDNS stop ordering

- Exact-head DNSMasq run #45553242639 confirmed the existing SmartDNS config-preservation fixture now extracts and runs with both firewall helpers, but its source checker still mandated the old unsafe order (stop daemon, then delete NAT redirects). Updated the checker to require redirect cleanup before daemon stop and Adbyby retargeting.
- The independent SmartDNS lifecycle checker and shell cases remain in place; exact-head rerun required. No local test or firmware result is claimed.

## 2026-11-24 — Update the existing DNSMasq preservation fixture for checked firewall helpers

- Exact-head DNSMasq run #45553050279 confirmed all seven new firewall-policy shell cases passed, then the older `dnsmasq_config_preservation.sh` fixture failed because it extracted `Change_iptable()` without the newly required helper functions. Updated its extraction block to source add, delete and dispatch helpers together.
- Extended the source contract to reject unchecked direct `iptables`/`ip6tables` invocations inside the dispatcher. Prior exact-head AI docs and process lifecycle checks passed; DNSMasq rerun required. No local shell or firmware result is claimed.

## 2026-11-24 — Keep SmartDNS alive until redirect cleanup succeeds

- Exact-head DNSMasq job #45552869102 ran all seven new firewall shell cases successfully, then the Python source contract failed on an over-broad assertion that confused helper-wrapped `iptables -A` arguments with raw unchecked calls. Removed that invalid assertion and the unrelated check against the older process test fixture.
- Tightened stop safety: NAT redirect cleanup now runs before the SmartDNS daemon is stopped. A check/delete error therefore returns while the daemon and saved resolver state are still available; successful cleanup permits daemon shutdown and later state-file removal. The source contract now pins this ordering.
- AI docs #45552869211 passed for parent source SHA `b42e026`; exact-head test reruns required on this correction. No local shell or XY build is claimed.

## 2026-11-24 — SmartDNS NAT redirect status and retry-state integrity

- `Change_iptable()` previously ignored all IPv4/IPv6 redirect add/delete results. Startup could continue without required DNS redirects; stop could report success and delete saved `smartdns_conf.ini` state while stale NAT redirects remained.
- Added checked add and idempotent delete helpers. Deletion uses bundled iptables 1.8.7 `-C` to distinguish absent rules from check errors; present rules are deleted, absent rules are harmless, other errors propagate. Stop attempts all owned rules and preserves the first failure. Startup sends add failures through existing rollback; stop preserves saved state for retry until firewall cleanup succeeds.
- Added shell regression for absent-rule idempotence, best-effort IPv4/IPv6 deletion, delete/check errors, partial and successful add, reset failure and unknown actions. DNSMasq lifecycle workflow runs the test and source contract. Exact-head Actions pending; no local shell run or firmware build is claimed.

## 2026-11-24 — Expand the PPPoE-MWAN LAN mask fixture variable

- Exact WAN run #45551840669 on `f8d0b7a` ran the repaired /16 tests but stopped earlier in the NVRAM failure matrix: the br0 mock emitted `Mask:${LAN_MASK...}` literally because the echo string remained single-quoted. Changed the mock to double quotes so its configurable mask is expanded; the production parser and firewall rules are unchanged.
- Exact-head WAN route / AI docs / XY-C1 checks must be observed on the corrected test SHA. No local shell run or firmware build is claimed.

## 2026-11-24 — Correct the PPPoE-MWAN LAN-prefix fixture splice

- The preceding repair attempt remained malformed: the remote test file still had duplicated fragments of the normal-run section, so the exact-head WAN rerun could not parse it. Rebuilt the shell regression directly from the intact `0b85e2e` baseline and inserted the two prefix cases by line index, not substring replacement.
- The /16 case now verifies both mangle and NAT command arguments; the non-contiguous mask case verifies rejection before firewall or route changes. The AI docs roadmap-size correction remains in effect.
- Exact-head WAN route / AI docs / XY-C1 reruns are required; no local shell or firmware result is claimed.

## 2026-11-24 — Repair PPPoE-MWAN prefix regression fixture and roadmap budget

- The first exact-head WAN rerun found a fixture-edit error: the /16 and non-contiguous mask assertions were inserted into the middle of the existing shell test, leaving invalid Bash. Restored the intact pre-slice test and inserted both cases at the unique normal-run boundary; the br0 mock now supplies a configurable `LAN_MASK`.
- AI docs validation also reported `REFACTOR_ROADMAP.md` at 131,322 UTF-8 bytes, 250 bytes above its 128-KiB hard limit. Condensed a superseded 2026-11-21 historical checkpoint while preserving the live module dashboard, priority queue and pointers to the canonical dev-log archive.
- Exact-head WAN route and AI docs reruns are required. No local shell test is claimed.

## 2026-11-24 — Derive PPPoE-MWAN source scope from the active LAN netmask

- The web LAN settings expose configurable `lan_netmask`, but `detect.sh` hard-coded `$lanip/24` in both the mangle source match and NAT masquerade rule. On /16 or /25 LANs the policy therefore omitted valid clients or matched an unintended range.
- The detector now extracts the live bridge mask from `ifconfig`, validates that its IPv4 bits are a contiguous prefix, and uses the resulting CIDR in both rules. Regression coverage checks that a /16 setting reaches both mangle and NAT argv and that a non-contiguous mask fails before existing rules or routes are changed.
- Exact-head WAN-route, XY-C1 Build/TRX/artifact and AI-docs checks are pending; no local shell or firmware result is claimed.

## 2026-11-24 — Reject octal-ambiguous PPPoE-MWAN IPv4 inputs

- `detect.sh` accepted multi-digit IPv4 octets beginning with zero, but the bundled iptables 1.8.7 `xtables_numeric_to_ipaddr()` parser uses `xtables_strtoui(..., base=0, ...)`; an octet such as `010` is therefore interpreted as octal rather than decimal. A static source rule could target a different address than the NVRAM value represents.
- `mwan_ipv4_valid()` now accepts only canonical decimal octets and rejects leading-zero aliases before deleting existing firewall rules. The shell regression injects `010.0.0.7` as the static source and asserts fail-closed status, unchanged firewall/routes, and lock cleanup. The existing WAN-route workflow runs the shell regression and source contract; XY-C1 path filters already include the changed source and tests.
- Exact-head WAN-route, XY-C1 Build/TRX/artifact and AI-docs checks are pending. No local shell or firmware result is claimed.

## 2026-11-24 — Fix WAN source-contract helper ordering

- Exact WAN workflow #45550662855 failed with `NameError: body is not defined`: the new LAN udhcpc hostname source assertion called the function-body parser before its definition. Moved only that assertion below the helper definition; the production hostname guard and GNU89 regression remain unchanged. Exact-head WAN rerun required.

## 2026-11-24 — Fail closed before starting LAN udhcpc without a valid hostname

- `start_udhcpc_lan()` passed `get_our_hostname()` directly into the `-H` argv slot. When the checked NVRAM lookup failed, the helper returned NULL; that NULL terminated argv immediately after `-H`, so udhcpc was still spawned with a missing argument and the later readiness timeout obscured the original NVRAM error.
- Added a pre-spawn policy that preserves lookup errno (or EIO when absent), rejects empty/invalid hostname values, and returns before calling the launcher. GNU89 ASan/UBSan boundary tests and a WAN source-order contract cover lookup errors, missing/invalid values and valid names. Exact-head WAN/Process/NVRAM/XY-C1 checks pending; no local compile or firmware build is claimed.

## 2026-11-24 — Repair SmartDNS DNSMasq source-checker initialization order

- Exact DNSMasq run #45549996147 on `5d713067` passed all SmartDNS shell regressions, then the Python integration contract failed with `NameError: stop is not defined`: it inspected the `Stop_smartdns` body before assigning it. Moved the body extraction before its first use; no production source changed. Exact-head rerun is required.

## 2026-11-24 — Bind AP static-host fallback to its file-safety branch

- `create_hosts_lan()` omitted braces around the safe `/tmp/hosts.static` branch, so the `else` bound to the nested load-error check rather than the file-safety test. A successful copy with non-`ENOENT` stale `errno` could emit a false “skipping” warning. Added explicit braces and a DNSMasq workflow source-contract regression for the complete branch.
- The source-contract job is the verification path; no local firmware or target build is claimed.

## 2026-11-24 — Preflight PPPoE-MWAN settings before deleting working firewall rules

- The detector previously read 443 and static policy selectors only after cleaning prior rules; transient NVRAM errors could leave a partially rebuilt firewall policy. All selectors are now read and validated first, with static entries snapshotted in Bash arrays before cleanup.
- Added failure injection for each of the five optional selector classes and checks that no firewall deletion/addition or route mutation occurs and the lifecycle lock is released.
- The exact-owner cleanup regression had failed on #45548806225/#45549253398 because a malformed test rewrite left incomplete shell assertions. The subsequent test-repair commits restored the shell fixture; WAN workflow #45549413305 PASS on `a74d142c`, XY-C1 build still pending at that checkpoint. No local shell test or firmware build is claimed.

## 2026-11-24 — Scope PPPoE-MWAN firewall cleanup to explicitly owned rules

- `detect.sh` previously deleted any mangle/NAT rule whose serialized text contained the broad tokens `ctstate`, `br0` or `ppp`. This could remove unrelated QoS/firewall policies and the primary PPP WAN masquerade rule during ordinary multi-WAN reconciliation.
- Added the exact comment owner tag `Padavan-PPPoE-MWAN` to all seven rules generated by the detector. Cleanup parses the saved rule into argv and deletes it only when a `--comment` option exactly equals that tag; malformed snapshots still fail closed, and unrelated rules with matching substrings are left intact. Added fixtures for owned mangle/NAT rows and unrelated conntrack, bridge, quoted-comment and PPP NAT rows.
- Exact WAN-route/XY-C1 CI is pending; no local shell test or firmware build is claimed.

## 2026-11-24 — Reconcile stale FTP firewall rules during storage-stop

- `stop_stor_apps()` claimed it refreshes FTP firewall state even when external FTP is disabled, but only set its refresh flag when the pre-stop daemon inventory was non-ABSENT. If FTP was already absent (including stale rules left by a previous crash), and no other storage service was running, the refresh path was skipped.
- The stop now captures pre-stop presence and the final state from `stop_ftp_locked()`. It preserves the prior refresh behavior for pre-existing RUNNING/UNKNOWN daemons and additionally refreshes whenever final inventory confirms ABSENT; an initially absent daemon that becomes UNKNOWN/RUNNING does not authorize cleanup. The original stop error still aborts storage removal. Added GNU89 policy matrix coverage and source-order assertions. Exact-head VSFTPD/AI/XY-C1 checks pending; no local compile or target build is claimed.

## 2026-11-24 — Throttle zero-delay udhcpc leasefail and DECLINE retries

- BusyBox `getopt32` documents the `A+` argument as nonnegative (0..INT_MAX), so `-A 0` is valid. Exhausted discovery (`leasefail`) and successful DHCPDECLINE recovery previously assigned the value directly; zero could spin immediate DISCOVER/ARP/DECLINE cycles. The client now applies the existing one-second retry floor to leasefail and both successful DECLINE restart paths.
- Added GNU89 ASan/UBSan policy boundaries for zero/negative/positive retry values and strengthened the source contract to pin all three call sites. Exact-head UDHCP and XY-C1 checks are pending; no local compile is claimed.

## 2026-11-24 — Record IAPPD lifecycle test assertion diagnostics

- Main commit `5b7df92aaaad83eb9f903582982070da95c92aad` adds an ordinal to the IAPPD lifecycle status assertion so a failing fixture identifies its assertion index. Exact IAPPD run #45547735320 PASS; Process lifecycle inventory #45547735377 PASS; Process inventory umbrella #45547735444 and XY-C1 #45547735319 were still pending at checkpoint. This commit changes test diagnostics only.

