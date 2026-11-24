## Current repository state

# Cross-session execution mandate

Continue this repository as one cross-session development stream. A logical source/test change is one atomic main commit with a complete Git tree, followed by an exact-SHA Actions check; never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow is not a reason to stop independent work.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending: verify main and the frozen backup, identify the exact current SHA, inspect relevant exact-head Actions, record any state transitions from PENDING to PASS/FAIL, and name the next concrete implementation task. Before context exhaustion, complete the current safe slice and update this handoff accurately. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no no-op commit. Do not claim tests, builds, artifacts or hardware checks that were not actually performed.


- Parent main SHA before this checker correction: 0db869b438d344406375dde71ee8b1764b047272; fetch live main for the exact current tip before any write. This checkpoint records completed exact-SHA validation for the descriptor-pinned libdisk deletion slice; fetch live main for its exact SHA before every write.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged; core AliDDNS CA-bootstrap/HMAC changes are integrated on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### Cross-userland continuation — WAN DHCP client transitions

- The libdisk removable-share ACL mount identity slice is implemented in production (del_folder() now pins one volume and staging descriptor set) and its GNU89 ASan/UBSan/source-contract tests passed on exact source SHA 80f1bfccd255536a0a2682c7d046f0216038a922.
- WAN DHCP client NAK/leasefail contracts are now explicit: REQUESTING skips deconfig; active-lease NAK must deconfigure and defer INIT_SELECTING on failure; wrong-server NAKs remain filtered; leasefail callback status stays advisory. Added GNU89 coverage for the REQUESTING/active lease gate and source assertions for NAK and leasefail ordering. Next, continue broader exact transition integration coverage for failed deconfig, lease expiry and SIGUSR2/-R while DECONFIG_PENDING.
- Main parent for this checkpoint is 0db869b438d344406375dde71ee8b1764b047272; the frozen backup is 72f4f157a084f8184b9597ca5bf334b8080e5807 and remains untouched. No PR/branch work.
## Verification boundary

- L3 VERIFIED for product-source SHA 80f1bfccd255536a0a2682c7d046f0216038a922 (XY-C1 Build + TRX + artifact; libdisk, Process, HTTPD, NVRAM PASS). Hardware L4 NOT VERIFIED.
- Exact da2bdbcf1ec3720d547c2665c638900bb628d289 results must be checked by run ID; it preceded the shared retry-delay changes. Its C/host suite and NAK/leasefail/source checks must not be conflated with THIS commit's expanded retry tests.
- Exact SHA `36a10c0e5844daf31b3cc483c20d9e25a798ef6b`: udhcpc #45476128312 C regression PASS / Python checker FAIL at the whitespace-normalized retry-helper return assertion; Process lifecycle #45476128289, Process inventory #45476128257, USB storage #45476128360, NVRAM #45476128492, XY-C1 #45476128451 and AI handoff #45476128686 all completed SUCCESS.
- Exact SHA `0db869b438d344406375dde71ee8b1764b047272`: udhcpc #45476513525 C regression PASS / Python checker FAIL at line 236 because `policy_compact` removes the space after `return`; AI handoff #45476513417 PASS. THIS COMMIT corrects that remaining checker token; its exact-head Actions are pending.
- The last verified L3 source baseline remains `80f1bfccd255536a0a2682c7d046f0216038a922` (XY-C1 Build + TRX + artifact); later DHCP product-source changes require their own exact-SHA target-build evidence. Hardware L4 remains NOT VERIFIED.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and is immutable.
- Frozen backup remains 72f4f157a084f8184b9597ca5bf334b8080e5807.

## Exact first action for the next GPT

1. Inspect the exact-SHA udhcpc workflow triggered by THIS compact-return assertion correction; require both the GNU89 -Werror ASan/UBSan C regression and the Python source contract to pass.
2. If another assertion fails, compare its compact-source expression with the actual normalized production code and correct only the checker expectation while preserving ordering/semantic coverage.
3. Once the checker passes, continue the deeper DHCP transition contracts for active-lease deconfig retry (state, sockets and timer), lease expiry, and SIGUSR2/`-R` while `DECONFIG_PENDING`; retain wrong-server NAK filtering.
