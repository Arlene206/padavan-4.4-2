## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; sole authorized development branch: `main`. Base main SHA for the legacy PR #1 import is `fd9275273c385ec266333fecb813fe7d18c69ecb`. The old `fix/aliddns-ca-bundle-bootstrap` ref is read-only legacy input; never commit to it or use its Actions as validation.
- Ported PR #1's valid CA trust-store and secret-safe HMAC implementation, build integration, tests and semantic checker/workflow in THIS COMMIT. The AliDDNS workflow is now push-only; imported changes must be validated on this exact main push before PR #1 is closed.
- Governance is main-only: no branches or PR-based development/validation; exact-head `push` Actions only; no force-push/history rewrite. GPT Session Rotator remains unchanged; preserve its existing new-main-SHA, 12-minute quiet-period and same-SHA de-duplication; do not reset/replay `9d5a5cb`.
- PR #1 still must be closed only after the import's exact-head main Actions pass. The connector has no safe branch-delete operation; preserve the old branch rather than force-updating or deleting it without supported tooling.
- At the previous main SHA `4890bd4f800d3d1f1f282061f8fe7ea8e533a969`, AI docs [43233088451] and AliDDNS baseline [43233088663] passed; WAN callback [43233088640] failed at the function locator for `static int man_down_internal(`; XY-C1 build [43233088562] failed at Samba's staged include `../../shared/src/include/router_account_policy.h`. These build failures remain open and must be resumed on main after the PR import closes.
- The previous required heading restoration was validated by AI docs [43250648231] and infosvr [43250648516] SUCCESS on exact SHA `fd9275273c385ec266333fecb813fe7d18c69ecb`; AliDDNS [43250648353], AdGuardHome [43250648374] and AliyunDrive [43250648484] were still running at last inspection.
## Active task — Continue XY-C1 MIPS firmware build through TRX validation

1. Re-fetch live `main` and reconcile all Actions against the exact head.
2. The original kernel `IP_OFFSET` failure is fixed by commit `b4b8074608aa118c6fdf99e2e70bc0ed6826b1ea`; the exact-head MIPS log subsequently compiled `xt_webstr.o` and completed the kernel.
3. The BusyBox shared DUID include failure is fixed by `e0e53f6d2eb9d5f8836ea7c1335e7c9d76e2faea`; the exact-head log compiled `networking/udhcp/dhcp6c_common.o` with the package's staged shared-header search path.
4. The HTTPD `do_restore_nv_cgi()` macro-semicolon error is fixed by `10ebefe94bf344d19d8506287125f52333596bde`; its exact-head log compiled `web_ex.o` and linked `httpd`.
5. The next exact-head RC compilation failure is five malformed declaration sequences in `trunk/user/rc/src/net_wan.c`. THIS COMMIT makes the five same-file-only helper definitions exactly one `static int` each and adds a WAN source contract that checks their declaration signatures. XY-C1 also watches `net_wan.c` changes.
6. Inspect exact-head WAN callback, AI docs and XY-C1 Actions. On a new failure, read the first causal error from the complete log and repair only that issue. Continue through all firmware components.
7. L3 closes only when `Verify TRX output` finds one or more non-empty `trunk/images/*.trx` files and artifact upload succeeds on the same SHA. L4 hardware flashing/boot remains separately NOT VERIFIED.
8. After XY-C1 L3 closes, resume the parked AliDDNS security/lifecycle task; its OpenSSL AccessKeySecret argv, CA extraction postcondition and concurrency follow-ups remain OPEN.


## Verification boundary

- L1 source: the five reported duplicate/conflicting declaration specifiers in `net_wan.c` are reduced to single private `static int` definitions. The functions have only same-translation-unit call sites in the inspected source. Function bodies and runtime behavior are unchanged. The WAN source contract asserts the five declaration forms.
- L2 exact-head prior Actions on `10ebefe94bf344d19d8506287125f52333596bde`: HTTPD security [43230551897] SUCCESS (all 40 steps); AI handoff docs [43230551924] SUCCESS (root log 61/119; archives 7/22/18); infosvr/AliyunDrive/AdGuardHome also completed on that SHA. The new WAN declaration contract and RC source fix await their own exact-head run.
- L3 prior XY-C1 [43230552179] FAIL at `net_wan.c` declaration errors after kernel, BusyBox DHCPv6 and HTTPD objects/link passed. Full RC/userland completion, firmware assembly, non-empty TRX and artifact upload remain NOT VERIFIED; this commit's target build is pending.
- L4 router/hardware: NOT VERIFIED.


## Exact first action for the next GPT
Fetch live `main` and all Actions on that exact SHA. Inspect WAN DHCP callback stability, AI handoff docs and XY-C1 firmware validation. First verify that `net_wan.o` compiles after the five declaration corrections; then inspect the next causal error from the full log and fix it narrowly. Do not claim L3 success until non-empty TRX validation and artifact upload both succeed. Then resume the parked AliDDNS task.
