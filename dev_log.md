## 2026-11-15 — Correct MTD load regression newline and exit-status handling

- Exact parent `2feac289de28bca92469c304a0af13d90c1fb5e9` USB storage #43690684317 passed shell syntax, MTD save retry and restore transaction, then exited nonzero in the combined MTD load host/BusyBox ash test. HTTPD security #43690684263, process lifecycle #43690684217, process fail-closed #43690684307, infosvr #43690684231, AI docs #43690684277, AdGuardHome #43690684211, AliyunDrive #43690684251 and AliDDNS #43690684338 passed; NVRAM #43690684273 and XY-C1 #43690684216 were still in progress.
- Two previous-checkpoint writes used a doubled backslash printf format, producing literal backslash-n rather than a newline. The fixture now writes a proper newline.
- The old cleanup trap could mask a nonzero assertion result because its final `rm -rf` succeeded. The EXIT trap now preserves the original status, and HUP/INT/TERM exit nonzero.
- Test/handoff/roadmap only; no product source behavior changed. Exact-head storage rerun required. No local test is claimed. Parent SHA: `2feac289de28bca92469c304a0af13d90c1fb5e9`.

---

