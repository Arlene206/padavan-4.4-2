# 2026-11-18 latest source checkpoint

- Parent main before THIS WAN VPN-stop enqueue-status fix: `fda7c018f1cb3ca4d0588f1f3e30baaba13f28c0`; frozen backup `72f4f157a084f8184b9597ca5bf334b8080e5807` unchanged.
- AP DHCP fix `0af358f3` is covered by exact WAN DHCP PASS `44203603261` on `d7f4bc9f`; HTTPD `44204487769`, AI `44204487682`, Process `44204487756`, infosvr `44204487704`, AdGuardHome `44204487714`, AliyunDrive `44204487708` and AliDDNS `44204487832` PASS on `4a9b812b`. NVRAM `44204487741` and XY-C1 `44204488447` were pending.
- XY-C1 `44202831040` on `509409f8` later completed Build + TRX + artifact all SUCCESS (AP DHCP source SHA, prior to this WAN stop patch).
- WAN VPN start notification failure is already aggregated since `816146ab`; the symmetric WAN-down stop event was still unchecked via `notify_rc()`. THIS source fix uses status-returning enqueue, preserves the error without short-circuiting cleanup and adds GNU89 regression + source contract.
- THIS WAN change requires exact-head WAN-route and XY-C1 validation. Hardware L4 NOT VERIFIED.

Next: exact-head WAN route regression + XY-C1 Build/TRX/artifact; recheck parent pending NVRAM/XY actions and continue independent P1 work.

cleanup and adds GNU89 regression + source contract.
- THIS WAN change requires exact-head WAN-route and XY-C1 validation. Hardware L4 NOT VERIFIED.

Next: exact-head WAN route regression + XY-C1 Build/TRX/artifact; recheck parent pending NVRAM/XY actions and continue independent P1 work.

