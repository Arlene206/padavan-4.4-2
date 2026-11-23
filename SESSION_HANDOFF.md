# SESSION HANDOFF — 2026-11-24

## Repository state

- Repository: `Arlene206/padavan-4.4-2`; writable branch `main` only.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; preserve unchanged.
- Parent before THIS XY-C1 workflow trigger correction: `f9475bf081db66e795df8c25332253ef7c6c80a2`. Fetch live main for the final commit SHA.
- PR #1 is closed/unmerged. The AliDDNS CA trust-store/HMAC security work is already represented by the current main; do not reopen or merge the divergent PR.
- No local checkout or MIPS/uClibc toolchain is available; no local build, firmware/TRX, artifact or hardware result is claimed.

## USB printer idempotent retry

- `trunk/user/rc/src/services_usb.c` routes U2EC/LPD startup through a shared launcher. It scans process state before PID cleanup/fork, treats RUNNING as idempotent success, fails closed on UNKNOWN, and only unlinks stale PID files for ABSENT. U2EC/LPD pass their daemon-specific PID-file path and no longer unlink in their wrappers.
- USB modem exact-head run #45220033428 on `e6dc19c` first failed because its NVRAM source checker expected caller-side unlink. The checker was aligned with the shared launcher, with an LPRng path-literal correction in commit `f9475bf081db66e795df8c25332253ef7c6c80a2`. Exact successor USB modem identity run #45220509149 on f947 PASS, including the formerly failing source-contract step.
- Process fail-closed #45220191260, NVRAM audit #45220191315 and USB storage #45220191366 PASS on `ecff475abb782b32e0a8f4f02026acc709ea6844`.

## APCLI and WAN ARP stability

- APCLI SSID/WISP startup checks use checked NVRAM reads. SSID presence is snapshotted before the next NVRAM read to avoid carrying a cache-owned pointer across reads. Exact source SHA `ab4b706f220bcdc84310063936bd074802876a7f` passed Wireless, USB modem, Firewall, Process, NVRAM, HTTPD, watchdog, iNIC, Process inventory and AI docs.
- WAN ARP monitor snapshots the egress interface through checked NVRAM/bridge text, rejects empty or overlong names before socket creation, and copies the exact name into both ifreq and Linux 4.4 SOCK_PACKET's 14-byte `sockaddr.sa_data`. It requires device binding and a finite receive timeout. Host policy/source checker PASS on `40ab459a16dfb914d571c8f559723e907a1b59e9` (#45218700965), AI docs #45218701533 PASS.
- THIS commit closes an XY-C1 trigger gap: the workflow listed `detect_wan.c` but omitted its directly included production header `detect_wan_arp_socket_policy.h`. The workflow is deliberately push-only on main; the header is now a tracked path.

## XY-C1 verification state and next action

- XY-C1 #45220033426 on product SHA `e6dc19c7c8ddf9c185d48817c28dc952412028b9` and #45220191233 on `ecff475abb782b32e0a8f4f02026acc709ea6844` were IN_PROGRESS at last observation. Both had `Build XY-C1` active and TRX verification/artifact upload pending. Neither is verified L3 until Build, TRX, and artifact upload all pass on the same exact SHA.
- Poll those exact runs and record the final step statuses. The current workflow-path-only commit does not trigger XY-C1 (the workflow is push-only and does not include itself in its paths).
- Continue independent repository-wide stability/security work without repeating APCLI, ARP, or printer analysis absent new evidence. Use complete atomic Git trees, non-forced expected-SHA main updates, and leave the backup untouched.

## Validation boundary

- USB printer checker: PASS on f947 / #45220509149.
- Process fail-closed, NVRAM audit and USB storage: PASS on ecff / #45220191260, #45220191315, #45220191366.
- XY-C1 Build/TRX/artifact: pending at last observation on e6dc/ecff; L3 NOT VERIFIED.
- Router/hardware runtime L4: NOT VERIFIED.
