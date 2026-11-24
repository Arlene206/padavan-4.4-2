## Current repository state

# Cross-session execution mandate

Continue this repository as one cross-session development stream. A logical source/test change is one atomic main commit with a complete Git tree, followed by an exact-SHA Actions check; never create partial per-file commits, force-push, or modify the frozen backup. A PENDING workflow is not a reason to stop independent work.

## Mandatory handoff checkpoint

Every session MUST inspect the live repository state before ending: verify main and the frozen backup, identify the exact current SHA, inspect relevant exact-head Actions, record any state transitions from PENDING to PASS/FAIL, and name the next concrete implementation task. Before context exhaustion, complete the current safe slice and update this handoff accurately. The checkpoint is mandatory; a document edit is conditional. If all checkpoint facts and the first action remain accurate, explicitly conclude the check with no file change and no no-op commit. Do not claim tests, builds, artifacts or hardware checks that were not actually performed.


- Parent main SHA before the latest DHCP checker correction: 0db869b438d344406375dde71ee8b1764b047272. Live main is fetched again before every write; the source-validation baseline is recorded separately below.
- Frozen backup backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807; never modify/delete it.
- PR #1 remains closed/unmerged; core AliDDNS CA-bootstrap/HMAC changes are integrated on main with later evolution. Do not port stale PR wholesale.
- Main-only development: one complete tree and one commit per logical task, then non-forced expected-SHA update. Never force-push or create development branches.

## Active task

### Cross-userland continuation — WAN DHCP client transitions

- The libdisk removable-share ACL mount identity slice is implemented in production (del_folder() now pins one volume and staging descriptor set) and its GNU89 ASan/UBSan/source-contract tests passed on exact source SHA 80f1bfccd255536a0a2682c7d046f0216038a922.
- WAN DHCP client NAK/leasefail semantics are covered: REQUESTING skips deconfig; active-lease NAK must deconfigure and defer INIT_SELECTING on failure; wrong-server NAKs remain filtered; leasefail callback status remains advisory. Shared retry delay is one second minimum, and SIGUSR2/-R preserve pending cleanup/release behavior.
- The follow-up now extracts the complete DECONFIG_PENDING retry state/timer payload into a GNU89-testable plan (resume state, release intent, retry timeout, packet counter, elapsed counter), prepares it before closing the listener, and only publishes it after switching to LISTEN_NONE. Source contracts also pin pending-resume socket/timer setup, lease-expiry ordering and the SIGUSR2/-R release path. Exact L2/Actions for this source slice will be checked on its own SHA.
- Prior validation: commit `98168ad16689cf47e1efeb2ee3999374790cbc41` passed udhcpc #45479146215 and AI handoff #45479146192. Product-source L3 remains only the earlier verified SHA `80f1bfccd255536a0a2682c7d046f0216038a922`; hardware L4 is not verified.
## Verification boundary

- L3 VERIFIED for product-source SHA 80f1bfccd255536a0a2682c7d046f0216038a922 (XY-C1 Build + TRX + artifact; libdisk, Process, HTTPD, NVRAM PASS). Hardware L4 NOT VERIFIED.
- Exact da2bdbcf1ec3720d547c2665c638900bb628d289 results must be checked by run ID; it preceded the shared retry-delay changes. Its C/host suite and NAK/leasefail/source checks must not be conflated with THIS commit's expanded retry tests.
- Exact SHA `36a10c0e5844daf31b3cc483c20d9e25a798ef6b`: udhcpc #45476128312 C regression PASS / Python checker FAIL at the whitespace-normalized retry-helper return assertion; Process lifecycle #45476128289, Process inventory #45476128257, USB storage #45476128360, NVRAM #45476128492, XY-C1 #45476128451 and AI handoff #45476128686 all completed SUCCESS.
- Exact SHA `0db869b438d344406375dde71ee8b1764b047272`: udhcpc #45476513525 C regression PASS / Python checker failed on compact-return whitespace; AI handoff #45476513417 PASS.
- Exact SHA `98168ad16689cf47e1efeb2ee3999374790cbc41`: udhcpc #45479146215 PASS (GNU89 -Werror ASan/UBSan C regression and Python source contract); AI handoff #45479146192 PASS.
- Exact SHA `fd59df3710ddcadfd9ca96f9e3b486f5cce7866e`: GNU89 -Werror ASan/UBSan retry-plan regression PASS; udhcpc #45480041494 Python checker FAIL at line 238 because the source checker still expected the retry helper to be called directly from the scheduler after it was moved into the tested plan initializer. AI handoff #45480041430 PASS. THIS checker-only correction updates that assertion to the new plan call.
- Exact-head rerun for THIS correction is pending. XY-C1 #45480041423, Process inventory #45480041644, Process regression #45480041481, storage #45480041346 and NVRAM #45480041721 for the prior source SHA were still in progress at last observation; inspect by exact SHA only.
- Last verified L3 product-source baseline remains `80f1bfccd255536a0a2682c7d046f0216038a922` (XY-C1 Build + TRX + artifact). Hardware L4 remains NOT VERIFIED.
- Frozen backup remains `72f4f157a084f8184b9597ca5bf334b8080e5807` and is immutable.

## Exact first action for the next GPT

1. Inspect the exact-SHA udhcpc workflow for THIS checker correction; require both GNU89 -Werror ASan/UBSan C regression and all Python source contracts to pass.
2. Inspect still-pending prior-SHA XY-C1 Build/TRX/artifact, Process, storage and NVRAM runs without attributing them to this checker correction.
3. After the checker passes, continue the DHCP pending-resume state/timer and NAK/expiry/SIGUSR2 transition integration contracts without weakening wrong-server filtering.
