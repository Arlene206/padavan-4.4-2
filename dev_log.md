## 2026-11-16 14:18 Beijing — Align LAN restart checker and handoff checkpoint

- Parent main: `f66dc26260ba426245ad403b6703b5ba6d52336a`. Commit `b2d3288b6282c16cf23859eb9e7f8fdce083cd62` implemented first-error aggregation for the full LAN restart. Its extended service-status regression passed in both Process and XY-C1 host-test steps.
- Exact b2 Process workflow `43814394160` failed before jobs due a malformed PR path list; commit `f66dc26260ba426245ad403b6703b5ba6d52336a` repaired the YAML.
- Exact f66 Process workflow `43814587870` ran the host regressions successfully but failed in the old source checker at literal `stop_lltd();` ordering (the source now wraps that call in the status recorder). This commit adjusts the ordering contract to assert the recorder call; it preserves the VPN stop/abort and DNS-lock-before-VPN-restore assertions.
- Exact b2 AI docs `43814395799` and f66 AI docs `43814587897` both failed because the handoff did not contain the mandatory end-of-session checkpoint wording. This commit restores the validator phrase and required headings. Exact b2 XY-C1 `43814395678` was still IN_PROGRESS at last check; its LAN service status regression had passed but Build/TRX/artifact steps were pending.
- Exact f66 Adbyby `43814588010`, AliDDNS `43814587964`, and AliyunDrive `43814587985` PASS. No L3 evidence is established for b2/f66. Latest L3 remains c6 `43811175036` only (Build/TRX/artifact all PASS).
- THIS COMMIT exact-head Process and AI documentation Actions are pending after publication. No L4 physical-router test is claimed.
---

## 2026-11-16 14:05 Beijing — Preserve full LAN restart service results

- Parent main: `2c75de8133751e8063ded244faa225011f2891b6`. `full_restart_lan()` returned only DNSMasq status even though PID 1 merges its int return into the LAN notification. It discarded status-bearing logger/LLTD/infosvr/networkmap/miniupnpd stops; VPN server start/restart and IPTV restart; NFS/NetBIOS reloads; infosvr/LLTD/networkmap starts; IAPP restart; and HTTPD identity-notification results. That could acknowledge the notification while an operation failed.
- Added a first-error recorder using the existing `rc_first_service_start_status()` policy. It captures these operations plus recovery and DNSMasq-lock-release failures, then returns the first nonzero status. Existing VPN-before-DNSMasq lock order, DNSMasq start gate, early VPN/LAN aborts and restoration paths are preserved.
- Extended the GNU89/Werror/ASan/UBSan service-status regression with recovery and signed/positive status preservation cases, and extended the Process source-integration checker. Added LAN trigger paths to Process pull_request and XY-C1 workflows; XY-C1 now runs the expanded service status regression before firmware build.
- On exact `c6cf7147ee636ab6a715ee35f9219f4c5c92a3e1`, Process `43811174980`, miniupnpd `43811175012`, AI docs `43811174933`, and XY-C1 `43811175036` (Build, TRX verification, artifact upload) PASS. This remains the latest L3 baseline before the newest parent changes.
- Exact parent `01b7fc9163a8445ada6b96a7bfdee6305a2c95fa`: Process lifecycle `43813748377`, process inventory `43813748353`, VPN lifecycle `43813748332`, AI docs `43813748506`, WAN DHCP callbacks `43813748381`, and other listed workflows SUCCESS. XY-C1 `43813748358` and NVRAM `43813748378` were IN_PROGRESS at last observation. THIS COMMIT's exact-head Process/XY/AI runs pending after publish.
- No current-head firmware/TRX/artifact or L4 physical-router result is claimed.
---

