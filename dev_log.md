## 2026-11-13 12:10 CST (UTC+8) — Close Adbyby third-party rule download hardening

- Live main SHA: a8681f65733f08b7c932833fafe99062492f0a2a. NVRAM-configured third-party Adbyby rule download slice is now L2 CLOSED on this exact SHA.
- All six triggered main push Actions completed SUCCESS and all exact checkout SHAs, steps and full logs were inspected: Adbyby [43363263383] / job [131681514167], infosvr [43363263454] / job [131681514465], AI docs [43363263378] / job [131681514145], AliDDNS [43363263594] / job [131681514640], AliyunDrive [43363263430] / job [131681513797], AdGuardHome [43363263365] / job [131681513585]. Adbyby sh -n, hosts preservation, lifecycle lock, stock HTTPS/MD5/cache rollback, third-party HTTPS/URL/size/last-good publication, firewall snapshot and semantic source/workflow checker all PASS. AI docs reports root log 97/119 and archives 7/22/18.
- Production commit 315d738a580e154302dfdd9c57674ee64c3ada36 closes the insecure NVRAM third-party rules path: TLS verification / firmware CA / HTTPS-only, quoted and validated URLs, transfer timeout/retries/rate/size limits, unique mode-0700 staging, accepted line filtering, last-good cache preservation and atomic complete-list publication. Checker-only corrections 267bfb4e2d669c07d0daee918302e8780abc5958 and a8681f65733f08b7c932833fafe99062492f0a2a fixed stale assertions (centralized URL check and wrapper-level umask); final exact-head Actions green.
- Firmware curl package is 8.21.0 (trunk/libs/libcurl/Makefile), confirming shared downloader flags --proto, --max-filesize and --limit-rate exist in project build.
- Next Adbyby scope: anti-ad dnsmasq fragment via anti_ad_link, then host-source downloads. Inspect actual configuration producer/default and supported dnsmasq fragment syntax before enforcing a format allowlist. Both still contain curl -k and fixed destinations. After these, return to FRP whole caller-path audit. FTP cleanup remains OPEN/BLOCKED on the Linux 4.4 final-name TOCTOU; NFS needs client evidence; L3/L4 remain NOT VERIFIED.
- Root log count: 97→98/119; size remains below 384 KiB; archives remain dev-log=7, handoff=22, roadmap=18. No rotation or archive/index changes.
- Changed paths: SESSION_HANDOFF.md, REFACTOR_ROADMAP.md, dev_log.md. This checkpoint closes the previous active task and names the next concrete one; no firmware, branch/PR or GPT Session Rotator changes.
- Resulting main commit: THIS COMMIT.
- Next exact action: inspect all anti_ad_link producers/defaults and sample config grammar, then implement and test safe same-filesystem download/publication with last-good retention.

---

