# 2026-11-18 continuation checkpoint — WAN DHCP wait contract repair

## Current repository state
- Repository `Arlene206/padavan-4.4-2`, development branch `main`. Live main immediately before this checkpoint: `90cce7425d67bac64a79d17ec859bc0054aec337`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14` remains read-only at `72f4f157a084f8184b9597ca5bf334b8080e5807`. PR #1 remains closed/unmerged; its AliDDNS CA/HMAC/bootstrap changes are already present in main. Do not mutate the backup, PR #1 or GPT Session Rotator.
- Recent main history includes the SmartDNS DNSMasq transactional publication fix (`9bb9fda`), signal-safe service waits in RC/FTP/Samba/WAN, and NFSd restart settling work in `90cce7425d67bac64a79d17ec859bc0054aec337`.

## Active task
- Repair the WAN DHCP callback CI source contract. The product change in `e0c8269b265618aa49ba9ce73796af9ac87458dc` replaces an interruptible raw `sleep(2)` after successful DHCP with `service_wait_seconds(2)`, propagates `errno` (or `EIO`) and logs a failed settle wait. This is intentional: EINTR must not shorten the full two-second settling interval, and an actual wait failure must not be silently treated as success.
- Exact WAN DHCP callback run `44322448540` on `e0c8269b265618aa49ba9ce73796af9ac87458dc` failed only because the embedded workflow contract still required `sleep(2);` at stdin line 833. The dedicated `scripts/check_apcli_wait_status.py` already asserts the new EINTR-safe wait and no raw sleep. THIS COMMIT aligns the inline workflow assertions with the real source: require the guarded helper call, errno fallback, diagnostic and absence of raw `sleep(2)`.
- This slice is workflow/checkpoint-only; it does not change the WAN production source or weaken the callback/rollback checks. The shared wait policy C test exercises repeated EINTR with decreasing remaining intervals, non-EINTR errors and invalid timespecs.

## Verification boundary
- WAN DHCP callback run `44322448540` on `e0c8269b265618aa49ba9ce73796af9ac87458dc`: FAIL at the stale inline `sleep(2)` source-contract assertion; exact rerun is required for THIS COMMIT.
- On the same `e0c8269` SHA: WAN interface teardown `44322448520`, WAN route `44322448426`, WAN gateway ARP `44322448410`, Firewall Startup `44322448303`, DNSMasq `44322448746`, detect_internet `44322448595`, detect_link `44322448216`, VPN `44322448570`, Wireless `44322448504`, HTTPD security `44322448438`, miniupnpd `44322448364`, USB modem `44322448557`, ZeroTier `44322448140`, and Process inventory `44322448547` PASS. XY-C1 `44322448282` was still in progress at the last poll; no L3 claim for `e0c8269`.
- SmartDNS autostart fixture change `b2c45fe` passed its Process inventory and AI docs runs, but its Process lifecycle run failed because the checker accidentally read the wrong test file. Checker fix `e2044c3` was cancelled when a newer commit arrived, so it still needs exact-head Process validation.
- XY-C1 `44318683585` on the exact SmartDNS production-source SHA `9bb9fdaeff826c957b599cc45e695eb3fb8cd32c`: Build XY-C1, TRX verification and artifact upload PASS (L3 verified for that source only). BusyBox/uClibc live runtime, current WAN source Build/TRX and physical-router L4 remain NOT VERIFIED.
- No local checkout-based tests were executed in this connector-only session. GitHub Actions results above are the observed evidence.

## Exact first action for the next GPT
1. Fetch live main and exact-head Actions. Inspect the WAN DHCP callback, Process lifecycle and AI docs runs created by THIS COMMIT; repair the first real failure without weakening tests.
2. Confirm Process validates the SmartDNS autostart checker fix (`autostart_test` input separated from the SmartDNS process test).
3. Reconcile the XY-C1 Build/TRX/artifact run for the latest production-source SHA before making an L3 claim; then continue the unclosed P1 WAN/firewall and networking tasks in `REFACTOR_ROADMAP.md`.

## Mandatory handoff checkpoint
The checkpoint is mandatory; a document edit is conditional. Every session MUST inspect live repository state and exact-head Actions before ending. If the active task, verification boundary and first action remain accurate, leave this handoff untouched.

## Cross-session execution mandate
One logical slice is one atomic main commit. Finish bounded source/test work, create one complete Git tree and commit, then update main with non-forced expected-SHA compare-and-swap. Continue independent work while relevant Actions are pending. Before context exhaustion, leave the next GPT the exact first action, commit SHA, tests and CI state. Never claim unobserved local tests, target builds or physical-device validation.
