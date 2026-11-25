## 2026-11-26 02:05 CST (UTC+8) — Verify OpenVPN detached startup and guard rollback resources

- Base SHA: `273f4ea22773e2bbe2ce62d85936b2ff49ee3700`; result SHA: THIS COMMIT (resolve from live main on next recovery). Scope: `vpn_openvpn.c`, new rollback policy/header test, VPN lifecycle source checker/workflow, XY-C1 production filter/test, handoff/log.
- Both OpenVPN server and client now require their own writepid file to resolve through the existing trusted process-identity reader before detached startup reports success.
- TUN/TAP helpers expose per-attempt ownership. Rollback never signals a process and never infers global absence from a missing PID file: it requires two complete OpenVPN process inventories, one second apart. RUNNING, UNKNOWN, invalid or interrupted outcomes preserve the interface/callback link; only confirmed ABSENT permits removal, and only of resources created by this attempt.
- Added GNU89 ASan/UBSan tests for repeated process-state policy, error propagation and owned-resource gate; source-order contract for both start paths; VPN workflow coverage and XY-C1 production path test. Exact-SHA Actions are pending; no firmware Build/TRX/artifact or physical-router L4 result is assumed.
- Immediate CI repairs: `f9f071b8...` adds NVRAM test include path; `273f4ea2...` corrects the HTTPD embedded getter-signature check. Exact reruns require polling.

---

