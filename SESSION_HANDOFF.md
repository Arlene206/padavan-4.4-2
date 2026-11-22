# Cross-session execution mandate

Continue repository development across GPT conversations. Recover the live state once, then implement. Do not repeat completed audits or ask the user what to do next.

- Develop on `main` only. Preserve the archival backup branch read-only.
- A logical slice means code or test-contract change, required tests, one atomic commit, and exact-SHA Actions inspection.
- For multi-file edits, create all blobs, one complete tree and one commit, then update `main` by non-forced compare-and-swap against the exact parent SHA. Never force-push or expose a partial tree.
- Do not wait on pending Actions; advance independent work and revisit the precise SHA.

## Repository checkpoint

- Parent `main` SHA for this checkpoint: `1805fa23a010044d4f82c7e23e91695a6b7f904d`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; it is immutable.
- Latest commits in this development line include:
  - `72a1ca0a81a3cc7a30183bfb6527337c00bf94f8`: WAN IPv6 endpoint source-contract alignment.
  - `9e77c6924cc6bce18cb3fcb93d2e0b20ecd27f9c`: propagate DHCPv6 LAN address enumeration/publication failures without treating a clean miss as an error.
  - `a9aa6ba87339b0e21d1deed8464723a4aba50e9d`: remove obsolete DHCPv6 callback local.
  - `98b54718077878a93758c327277ce10fc8c6a44f`: align the WAN DHCP LAN-address callback ordering contract.
  - `5e0100c059639b323c903351d8f1fab6ff181659`: align the WAN DHCP inline callback contract.
- `start_dhcp6c()` already snapshots DHCP flags and WAN hardware address through checked NVRAM transactions before client/session/DUID/config side effects. DHCPv6 callbacks use checked setting snapshots, generation/interface fencing, status-bearing LAN address publication, and independent DNS processing. Do not repeat that completed lossy-getter audit without new evidence.

## Active item in this commit

- The previous VPN server lifecycle run `45026956696` on `db11b15bb545dbf9dafff9c4795600735884e0dc` failed only because `scripts/check_vpn_server_lifecycle.py` still required the old `get_wan_addr6_host()` pointer call.
- Production OpenVPN export now calls `get_wan_addr6_host_status()`. NVRAM identity-read or `getifaddrs()` errors abort the export; only a clean missing/empty address may proceed to LAN and, for dual-stack modes, IPv4 fallback.
- This commit updates the VPN source contract to require the checked helper, status propagation before fallback, and existing fake-backend coverage for read errors, enumeration errors and clean misses. No product-source behavior changes here.

## Verification boundary

- Exact parent `1805fa23a010044d4f82c7e23e91695a6b7f904d`: AI handoff #45027565508 PASS; WAN DHCP callback #45027565522 PASS.
- Previous WAN route #45027298482 and IPv6 service-type #45027298602 PASS on ancestor `a9aa6ba87339b0e21d1deed8464723a4aba50e9d`; they are not exact-head checks for later commits.
- No same-SHA current-head XY-C1 Build/TRX/artifact PASS is established here. L3 requires all three on one exact source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first actions for the next GPT

1. Fetch live `main`; inspect the exact new-head VPN server workflow triggered by this checker change, plus AI handoff validation. Fix any actual failure rather than inferring from ancestors.
2. Inspect current exact-head source/host workflows and XY-C1 Build/TRX/artifact status; keep the L3 claim strictly tied to one SHA.
3. Continue the wider Padavan userland stability/security rotation with a separate evidence-backed defect outside the completed DHCPv6 checked-read slice.

## Documentation guard

Recovery is read-only unless the handoff facts or first task materially change. Do not create no-op documentation commits.
