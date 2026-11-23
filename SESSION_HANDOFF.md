# Cross-session execution mandate

## 2026-11-23 continuation handoff (parent before this checkpoint: `0e9eb13c56c6f83fe75fbd8261eedaaef6ff154c`; always fetch live main)

### Completed production slices
- `e01860cab922e8e7da2c92b3f04db913b777ad09`: propagate checked modem-WAN identity/queue errors and retry deferred reconnect delivery in `detect_link`.
- `1cb817908e779c45d3c47105e39cd2002df0ba63`: use a checked, bounded WAN ARP egress-interface identity; reject empty/truncated/unterminated legacy `SOCK_PACKET` names.
- `e6dc19c7c8ddf9c185d48817c28dc952412028b9`: make U2EC/LPRng startup idempotent across hotplug retries; only a complete ABSENT inventory permits stale PID cleanup/fork.
- `0b6226df2faa8752fc526db3053eea74809bf5cb`: roll back U2EC/LPRng after readiness or uncertain exec-status-channel failure; preserve the primary error and verify final process state.
- `9e03bacbb7b72c60379fcac1d7f52d06b46088cc`: allocate each p910nd printer its own `/dev/usb/lpN`-derived port/process identity, add the GNU89 policy regression and USB/Process/Storage source contracts.
- `e6a6a3868a0e409a009ede9b01d01c355c372b15`: scope p910nd readiness rollback to the same per-port process identity and correct the checker order contract.
- `e52ec8ffe785dbf129e0de69ec60869101213149`: fix GNU89 per-port fixture invocation and whitespace-normalized rollback source contracts; exact USB modem and Storage checks pass. The Process workflow still failed because its launcher-error assertion searched from the beginning of the function and matched input validation; this checkpoint fixes the anchor.

### Verification state
- Earlier baseline `ecff475abb782b32e0a8f4f02026acc709ea6844` passed XY-C1 firmware build, TRX validation and artifact upload, but predates the later printer rollback and p910nd per-port changes; do not treat that run as verification of current `main`.
- Exact e52 USB modem #45225619186 PASS, including GNU89/Werror/ASan/UBSan p910nd policy compilation/run and source contracts; exact e52 Storage #45225619727 PASS. Exact e52 Process fail-closed #45225619509 FAIL at the p910nd source-order contract; fixed in this checkpoint by anchoring `launch_error_gate` after `eval()`. Exact e52 AI docs #45225619139 FAIL because this file lacked the literal `one atomic main commit`; that exact phrase is restored above.
- XY-C1 #45225109549 on e6a6 reached Build XY-C1 after all preflight contracts passed; TRX and artifact were still pending at last observation. No L3 result is claimed. Hardware multi-printer behavior is not verified.
- Backup branch remains protected: `backup/pre-plugin-overhaul-2026-11-14` at `72f4f157a084f8184b9597ca5bf334b8080e5807`; never force-push or modify it.

### First task for the next continuation
1. Fetch live main and inspect exact-head Process fail-closed and AI handoff Actions for the new checkpoint; confirm the p910nd source-order contract and literal handoff wording pass. USB modem and Storage passed on parent e52 (`#45225619186`, `#45225619727`).
2. Poll XY-C1 #45225109549 on e6a6; claim L3 only if Build XY-C1, TRX verification and artifact upload all pass on one identical product-source SHA. If still pending, continue other independent work.
3. Continue the next open stability/security source task from the whole-repository roadmap. Do not re-audit completed bootstrap/APCLI/WAN-ARP/printer work absent new evidence.
4. Keep one complete tree per logical change, update main with non-forced expected-SHA CAS, and preserve the backup. Before context exhaustion, record exact commit, test/CI states and a concrete first action.



Treat this repository as one continuous engineering assignment across GPT conversations. Recover once, then execute. Work only on `main`; each logical source or CI change is one atomic main commit: create all changed blobs and one complete Git tree before creating that commit, then update main using a non-forced expected-SHA compare-and-swap. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect live repository state before ending. Record only material changes: a relevant exact-head Action changing from PENDING to PASS/FAIL, a new commit, an unresolved failure, or a changed first action. If all checkpoint facts and the first action are already accurate, explicitly conclude with no file change and no commit. Do not manufacture documentation churn or a no-op commit.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; write to `main` only.
- Parent of THIS checker-contract correction: `e6a6a3868a0e409a009ede9b01d01c355c372b15`; fetch live main for the final SHA.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; preserve unchanged.
- PR #1 is closed/unmerged; AliDDNS CA trust-store/HMAC work is already represented or superseded on main.
- No local checkout or MIPS/uClibc toolchain is available. Do not claim local compile, firmware/TRX, artifact or physical hardware validation.

## Active task

### USB p910nd per-device identity and late-child rollback

- `trunk/user/rc/src/services_usb.c` scans canonical raw printer devices `/dev/usb/lp0` through `lp9`, maps each to the corresponding p910nd port, and derives the corresponding process identity `p9100d`…`p9109d`. The same port is passed to p910nd, readiness checks that identity, and the stop list includes the original launcher plus all per-port identities.
- Main commit `9e03bacbb7b72c60379fcac1d7f52d06b46088cc` fixed the original port-0 collision. Follow-up `e6a6a3868a0e409a009ede9b01d01c355c372b15` fixed a second substantive defect: readiness rollback must stop the selected `process_name`, not literal `p9100d`.
- The GNU89 policy fixture `tests/usb_printer_p910nd_policy.c` covers all 10 device-to-port mappings, malformed paths, invalid process names, insufficient output capacity and no mutation on failures.
- Exact USB modem #45225109809 on e6a6 passed modem/WAN/firewall contracts, then failed only because `check_usb_printer_nvram.py` expected bare `marker();` calls rather than the actual `CHECK(marker() == 0)` test invocations. THIS commit aligns that assertion. The USB, Process and Storage workflows now compare the rollback call after whitespace normalization and require the selected process name.
- At the last poll on e6a6: AI docs #45225109551 PASS; Process lifecycle #45225109772, Process inventory #45225109604, Storage #45225109612, NVRAM #45225109574 and XY-C1 #45225109549 were IN_PROGRESS. USB modem #45225109809 FAIL for the stale marker invocation assertion. Exact reruns after THIS commit are pending.

### Recent completed stability work (do not re-audit without new evidence)

- USB printer U2EC/LPD start uses checked NVRAM and a shared tri-state process preflight; RUNNING is idempotent, UNKNOWN fails closed, stale PID cleanup happens only for ABSENT. USB modem checker run #45220509149 PASS on `f9475bf`; Process fail-closed #45220191260, NVRAM audit #45220191315 and USB storage #45220191366 PASS on `ecff475`.
- APCLI startup fail-closes on checked SSID/WISP failures; SSID-presence bit is snapshotted before the next cache-backed NVRAM read. Exact source SHA `ab4b706f220bcdc84310063936bd074802876a7f` passed Wireless, USB modem, Firewall, Process, NVRAM, HTTPD, watchdog, iNIC, Process inventory and AI docs.
- WAN ARP monitor validates the egress interface snapshot and Linux 4.4 `SOCK_PACKET` 14-byte sockaddr limit; host policy/source-contract tests PASS on `40ab459a16dfb914d571c8f559723e907a1b59e9` (#45218700965). XY-C1 path now includes `detect_wan_arp_socket_policy.h`.

## Verification boundary

- p910nd port policy fixture: exact GNU89/ASan/UBSan run is expected in USB modem workflow; e6a6 USB modem stopped at checker before the policy fixture step was complete/recorded as PASS.
- e6a6 USB modem #45225109809: FAIL only at test invocation assertion in `check_usb_printer_nvram.py`; fixed in THIS commit.
- e6a6 Process lifecycle #45225109772, Process inventory #45225109604, Storage #45225109612 and NVRAM #45225109574 were IN_PROGRESS at last poll.
- e6a6 XY-C1 #45225109549 was IN_PROGRESS before target Build; inspect Build, TRX and artifact upload individually. No L3 result is claimed.
- Physical multi-printer behavior / router L4: NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live main and inspect exact-head USB modem, Process lifecycle, Storage, Process inventory, NVRAM and AI docs Actions for THIS checker correction. Confirm the p910nd policy compile/run and all three per-port rollback source contracts pass. Diagnose failures from the first causal log line.
2. Resolve e6a6 XY-C1 #45225109549 and any exact-source successor. Claim L3 only if Build XY-C1, TRX verification and artifact upload all pass on one identical product-source SHA.
3. Continue a new bounded stability/security source fix from the live roadmap. Do not repeat APCLI, WAN ARP or p910nd source reviews absent new evidence.
4. Use one complete tree per logical task and non-forced expected-SHA updates to main; preserve the frozen backup.

## Cross-session operating constraints

- Recover once, then implement rather than repeatedly plan or audit completed work.
- Loop: source modification → relevant tests → atomic commit → exact-SHA Actions. PASS advances, FAIL is diagnosed, PENDING does not block independent work.
- Preserve Linux 4.4, GNU89, MIPS/uClibc, router configuration, security and backward compatibility.
- Leave accurate commit/test/CI facts and a concrete first action before context exhaustion. Never claim unverified work.
