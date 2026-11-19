## 2026-11-19 12:34 CST (UTC+8) — Align Switch CLI checker with bounded ACL numeric validation

- Parent main SHA: 3b2c0f3dad38892f801d5778a4d74e3a8d6aaa04.
- Exact-head Switch CLI run 44457779961 failed in its first source-contract step before compiling or running the host parser regression. The checker still expected 12 production exit(EXIT_FAILURE) sites and the previous TrTCM argc < 10 predicate; the new bounded numeric-argument helper adds the expected 13th fail-closed exit and TrTCM now rejects both missing and surplus arguments with argc != 10.
- Update the existing workflow contract to require the new helper's checked parse, cleanup-before-exit ordering, the correct total and exact arity guard. Do not alter test coverage or workflow triggers.
- The same exact source SHA 3b2c0f3dad38892f801d5778a4d74e3a8d6aaa04 passed XY-C1 run 44457779695: Build XY-C1, TRX verification and artifact upload all succeeded (L3). Switch CLI host tests were skipped by the failed preceding contract and must rerun on this follow-up SHA. AI docs 44457779839, Process inventory 44457779710 and NVRAM boundary 44457779788 passed on the source SHA.
- No local checkout/test execution or physical-router validation was available; L4 remains NOT VERIFIED. THIS COMMIT is a workflow/checkpoint correction, not a new product-source build baseline.

---

