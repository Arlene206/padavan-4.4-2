## 2026-11-16 — Propagate global route-cache flush failures through full WAN restart

- Parent main: `2c503c9ee1b3cda569700d3e85f614eed0317ad8`. `flush_route_caches()` discarded the result of `system("ip route flush cache")`, and its only RC caller, `full_restart_wan()`, ignored the result before switch/WAN reconstruction.
- Converted the wrapper to return `eval("ip", "route", "flush", "cache")`, updated the public prototype, and combined global-cache and per-interface route flush results using first-error semantics. The cache attempt still runs when an interface flush fails; route-dependent WAN restart work is blocked if either cleanup stage fails.
- Added injected regression cases for route-first failure, cache-only failure, successful cleanup, and missing callback. WAN route CI compiles the test with GNU89/ASan/UBSan and checks source API, caller wiring and restart gate.
- Parent exact WAN route `43977147385`, Process `43977147335`, and AI handoff `43977147281` passed; NVRAM `43977147536` was pending. Exact 759c18 XY-C1 `43976717898` was still building. This route-cache change needs its own same-SHA Build/TRX/artifact verification. Physical-router L4 remains NOT VERIFIED.
- Next: inspect exact-head WAN route/Process/AI/NVRAM/XY-C1 Actions and resolve first causal failure.

---

