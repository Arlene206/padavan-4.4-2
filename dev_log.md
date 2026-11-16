## 2026-11-16 14:05 Beijing — Preserve full LAN restart service results

- Parent main: `2c75de8133751e8063ded244faa225011f2891b6`. `full_restart_lan()` returned only DNSMasq status even though PID 1 merges its int return into the LAN notification. It discarded status-bearing logger/LLTD/infosvr/networkmap/miniupnpd stops; VPN server start/restart and IPTV restart; NFS/NetBIOS reloads; infosvr/LLTD/networkmap starts; IAPP restart; and HTTPD identity-notification results. That could acknowledge the notification while an operation failed.
- Added a first-error recorder using the existing `rc_first_service_start_status()` policy. It captures these operations plus recovery and DNSMasq-lock-release failures, then returns the first nonzero status. Existing VPN-before-DNSMasq lock order, DNSMasq start gate, early VPN/LAN aborts and restoration paths are preserved.
- Extended the GNU89/Werror/ASan/UBSan service-status regression with recovery and signed/positive status preservation cases, and extended the Process source-integration checker. Added LAN trigger paths to Process pull_request and XY-C1 workflows; XY-C1 now runs the expanded service status regression before firmware build.
- On exact `c6cf7147ee636ab6a715ee35f9219f4c5c92a3e1`, Process `43811174980`, miniupnpd `43811175012`, AI docs `43811174933`, and XY-C1 `43811175036` (Build, TRX verification, artifact upload) PASS. This remains the latest L3 baseline before the newest parent changes.
- Exact parent `01b7fc9163a8445ada6b96a7bfdee6305a2c95fa`: Process lifecycle `43813748377`, process inventory `43813748353`, VPN lifecycle `43813748332`, AI docs `43813748506`, WAN DHCP callbacks `43813748381`, and other listed workflows SUCCESS. XY-C1 `43813748358` and NVRAM `43813748378` were IN_PROGRESS at last observation. THIS COMMIT's exact-head Process/XY/AI runs pending after publish.
- No current-head firmware/TRX/artifact or L4 physical-router result is claimed.
---

