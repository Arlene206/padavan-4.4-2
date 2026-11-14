# Padavan 4.4 — AI Development Protocol

**Repository:** https://github.com/Arlene206/padavan-4.4-2  
**Primary branch:** main  
**Mission:** repository-wide Padavan user-space stability bug discovery and repair first, with security hardening progressing in parallel; also improve performance, resource management, compatibility, and maintainability. Critical security vulnerabilities and severe system failures take immediate priority.

This file is the durable source of development rules. Every new ChatGPT conversation must read this file and the current handoff/roadmap before changing code. Repository state, commits, CI runs, and these documents take precedence over chat memory.

## Main-only development and validation invariant

The sole authorized development branch is `main`. Do not create, use, or push any feature/fix/temporary/checkpoint branch; do not create or use pull requests. Complete each independent fix on current `main` and push it directly. Validate through GitHub Actions `push` runs for the exact `main` SHA only. A PR/branch/older-SHA green result never replaces main validation; fix red checks with new main commits. Never force-push or rewrite history. Existing PR #1 is a legacy preservation task: compare it against main, port valid unmerged work to main, verify there, and only then close the obsolete PR. Never delete its unmerged work. Keep GPT Session Rotator unchanged; do not reset its state or replay SHA `9d5a5cb`; allow its current new-main-SHA and 12-minute quiet-period behavior to run naturally.


## 0. Non-negotiable cross-conversation recovery and record governance

**The user's first priority is uninterrupted development continuity.** Every new GPT conversation must reconstruct the active task, whole-repository progress, validation boundary and exact next action directly from GitHub within a few minutes. This is a required engineering property, not an optional documentation preference.

No session may edit firmware source before completing the recovery sequence in Section 10 and the root `AI_BOOTSTRAP_PROMPT.md`. Live GitHub state and the latest fetched `main` SHA are authoritative; hidden memory or copied chat summaries may not override them. Resume the active handoff task or resolve its pending/failed relevant CI before selecting another task.

### Canonical root files

- `SESSION_HANDOFF.md`: exactly one current resumable checkpoint. Target 8–24 KiB; hard limit 64 KiB. State the live baseline SHA, active branch, last completed task, active task, paths/functions, evidence, exact-head CI, validation gaps, blockers and actionable first inspection. Update this file in place only when its current state materially changes or before stopping. Git history is its revision archive; do not create routine snapshot copies.
- `REFACTOR_ROADMAP.md`: one current whole-repository module dashboard and priority queue. Target below 64 KiB; hard limit 128 KiB. Keep one row per real module with source map, audit status, latest meaningful evidence/commit and next action. Put deep audit notes in `docs/audits/`. Update this file in place only when inventory, module state, evidence, or priority materially changes. Git history is its revision archive; do not create routine snapshot copies.
- `dev_log.md`: newest-first chronological history. After a rotation it contains the newest 20 complete entries, then accumulates to 119. Target at or below 384 KiB; hard limit 512 KiB. Do not rotate on every entry.

### Durable product and architecture discussion continuity

- `XY-C1_DISCUSSION_CONTEXT.md` is the durable record for user requirements, constraints, source-verified facts, accepted architecture decisions, rejected alternatives, and open questions concerning XY-C1/C5 firmware, networking, DNS, and plugin design.
- Before continuing one of those discussions, read the full current file. Append each material new discussion decision or correction with a dated section; do not rely on chat memory or leave a user-confirmed decision only in conversation.
- Clearly label facts confirmed from current source, user requirements, accepted decisions, design candidates, and unresolved questions. Include repository paths / upstream references when they support a conclusion.
- Update the bootstrap prompt whenever recovery steps must change so later GPT sessions know to load this context file.
- A discussion record is not an implementation authorization. Do not edit product code, build configuration, or firmware settings unless the user explicitly authorizes implementation and the normal project protocol is followed.

### Batched, lossless log rotation

- When adding an entry would bring the root log to 120 records, move the oldest 100 complete records verbatim into `docs/dev-log/YYYY/MM/part-NNN.md` in one batch, leaving the newest 20 records in `dev_log.md`. Do not commit a 120-record root log.
- Size is a safety trigger, not a per-entry housekeeping task: if `dev_log.md` exceeds 384 KiB before reaching 120 entries, archive the oldest complete records in one batch so the newest 20 remain and the root returns below the target where possible. The 512 KiB hard limit is never allowed to be exceeded.
- Archive shards target 256–384 KiB and have a 512 KiB hard limit. Split only between complete dated records. Keep each shard chronological oldest-to-newest; root log remains newest-first. A single oversized record gets its own shard; never split or rewrite a record.
- `docs/dev-log/INDEX.md` lists every log shard path, record count, oldest/newest date, exact byte size and Git blob SHA, plus the original source blob SHA. Update it in the same commit as a log archive rotation.
- Existing `docs/handoffs/archive/YYYY/MM/` and `docs/roadmap-history/YYYY/MM/` snapshots and their indexes are legacy historical evidence. Preserve them byte-for-byte and keep them indexed, but do not create new handoff or roadmap snapshot archives for routine live-file updates. Git commit history is sufficient to retrieve earlier versions of `SESSION_HANDOFF.md` and `REFACTOR_ROADMAP.md`.
- Do not load full log archives on every new session; read the newest 20 root entries and retrieve older shards only when a specific historical fact is required.

### Documentation validation and CI

Run `python3 scripts/check_ai_handoff_docs.py` before committing documentation changes when a local checkout is available. The `AI handoff documentation contract` workflow validates the current root records and all existing indexed archives on pushes and pull requests. It must pass on the exact commit SHA. A failing governance check blocks the documentation change.

The check permits 20–119 root log entries, triggers rotation at 120 entries or the 384 KiB target, and enforces the 512 KiB hard limit plus archive inventory, byte counts and Git blob SHAs. CI results are authoritative in GitHub Actions: reconcile them during the current recovery or next takeover, but do not create a follow-up docs-only commit solely to write a completed CI result back into the files.

For a log entry inside the commit that creates it, label the result `THIS COMMIT` to avoid an impossible self-referential SHA. Reconcile the resulting commit SHA from live GitHub state on the next recovery.

## 1. Operating principles

- Work autonomously within the connected GitHub permissions and available tools. Do not ask for redundant permission or repeat questions already answered.
- **Primary direction is stability:** actively find and fix reproducible or source-demonstrable reliability defects across the whole Padavan user space. Security work continues in parallel; confirmed critical security vulnerabilities, data corruption, boot failures, and severe service outages override ordinary stability work.
- Consider performance and resource management where they cause or materially worsen failures: leaks, stale state, unbounded retries, deadlocks, race conditions, unchecked return values, partial updates, cleanup failures, process lifecycle errors, and recovery paths.
- Do actual repository work; a plan or audit report alone is not a completed task.
- Never claim a change, build, test, CI result, firmware image, or hardware validation that was not actually verified.
- Do not optimize for commit count, line count, test count, or green badges.
- A successful test suite for one component is not evidence that the whole firmware is safe or working.
- Preserve existing Padavan behavior, configuration compatibility, device support, and recovery paths unless a change explicitly fixes a demonstrated defect.
- Keep Linux 4.4 kernel/MTK driver/DTS upgrades out of ordinary user-space modernization. Treat kernel modernization as a separately scoped project.

## 2. Repository-wide coverage is mandatory

Maintain an inventory of real modules discovered from the source tree, build manifests, init scripts, configuration, and call relationships. The inventory must be based on this repository; do not assume every example below exists.

At minimum inspect these areas:

1. Web and remote management: HTTPD, HTTPS/TLS, CGI, ASP/template handlers, authentication, sessions/cookies/tokens, authorization, command dispatch, configuration APIs, uploads/downloads, file access, frontend/backend interfaces.
2. Network services: dnsmasq, DHCP, DNS, PPP/PPPoE, WAN, IPv4/IPv6, routing, NTP, DDNS, UPnP, port forwarding.
3. Network security: iptables/ip6tables, firewall, NAT, conntrack, QoS, traffic shaping, VPN/tunnels, protocol parsers.
4. System/configuration: NVRAM, config persistence, boot/init, service lifecycle, process and signal handling, IPC, logging, upgrade/restore, privilege boundaries.
5. Wireless/hardware: wireless management, hostapd/wpa_supplicant integration, MTK driver interfaces, USB, storage, hotplug.
6. File and application services: FTP, Samba, print/media services, and other actually integrated services.
7. Shared infrastructure: BaseKit, SkipDBv2, UDB, common C libraries, memory/string/file/network helpers, concurrency, third-party components and local patches.
8. Build/delivery: Makefiles, CMake, toolchain, compile flags/macros, GitHub Actions, source contracts, firmware assembly and image validation.

Do not let an already well-tested subsystem monopolize development merely because it is easy to inspect. Whole-repository coverage outranks polishing a subsystem that has already received many rounds of work.

## 3. Stability-first risk-based selection

Select work from the whole-repository ledger in `REFACTOR_ROADMAP.md`, not from habit or from whichever module is easiest to inspect.

Use this priority order:

- **P0 — immediate:** confirmed critical security vulnerability; data corruption or loss; boot/recovery blocker; severe system-wide failure; release blocker.
- **P1 — stability:** reproducible or strongly evidenced failures in core paths such as WAN/PPP/DHCP/DNS/routing, firewall/NAT, service lifecycle, configuration persistence, storage/mount/hotplug, wireless, HTTPD management, or startup/shutdown. Also include serious security flaws and exposed privileged paths.
- **P2 — preventive reliability/security:** unchecked errors, race windows, stale callbacks/state, incomplete rollback, resource leaks, unsafe retry/backoff, missing regression coverage, and important unaudited attack surfaces.
- **P3 — performance/maintainability:** measurable resource/performance improvements, safe deduplication, cleanup, and style work that does not displace higher-impact defects.

Security remains an active parallel workstream; do not defer a confirmed serious vulnerability merely because the current round is stability-focused. Conversely, do not let security-only work crowd out stability defects indefinitely.

Do not let SkipDBv2, HTTPD, firewall, or any other already heavily worked subsystem monopolize the project. Rotate across real modules in the source tree and prioritize unaddressed stability risks. Keep an explicit whole-userland inventory and record why each selected task outranks the alternatives. Resume the exact unfinished task or pending/failed CI first; once it is closed, select the next highest-value stability defect from the global ledger, unless a P0 security/system failure requires immediate action.

## 4. Bounded, continuous delivery

Each round should deliver one coherent, independently reviewable fix or tightly related set of fixes. Keep the change bounded to protect handoff reliability; do not turn a conversation into an unbounded rewrite. A round may inspect several areas to choose the best issue, but implementation should stay focused and the roadmap must keep the broader rotation visible.

Each round has one primary independent module. Before editing:

- Read the relevant implementation, headers, build integration, callers/callees, startup/shutdown path, and error paths.
- Identify concrete defects or measurable maintainability risks.
- Check public/private declarations, extern/static ownership, macros, conditional compilation, ABI/layout, integer widths, alignment, pointer conversions, allocation ownership, and platform assumptions.
- Understand persistent formats, protocol behavior, and configuration compatibility.
- Define the smallest coherent change and its regression coverage.

Do not perform unrelated opportunistic edits. Related tests, private headers, build manifests, CI contracts, and documentation may change when required for the selected module.

Prefer targeted fixes over broad rewrites. Avoid unnecessary abstraction layers and API churn.

## 5. Build and platform safety

The development host may not have the complete MIPS firmware toolchain. Never treat host compilation as proof of target compatibility.

- Check the actual compiler, libc, kernel interfaces, and language mode used by the firmware.
- Avoid APIs or language features unsupported by the target toolchain.
- Verify function prototypes, header self-containment, symbol linkage, compile guards, and source registration.
- When adding/removing/renaming source or test files, update all applicable Makefiles/CMake manifests, firmware integration, CI paths/contracts, and packaging rules.
- Source contracts should test meaningful structure, integration, and safety properties. Avoid brittle dependence on whitespace, indentation, comment wording, or incidental code ordering.
- Do not weaken or delete a meaningful check just to obtain a green result.

## 6. CI decision process

Before each round, inspect the latest main HEAD and the relevant GitHub Actions runs. Match every run to its exact head SHA.

If CI is red:
1. Read the failed job and relevant logs.
2. Identify whether the failure is a source defect, regression, build/integration issue, brittle source contract, workflow error, or external infrastructure issue.
3. Fix the existing failure first. Do not start unrelated refactoring while a relevant failure remains unresolved.
4. Add or preserve regression coverage for real defects.
5. Do not suppress tests, skip required jobs, or mislabel infrastructure failures as code fixes.

If CI is green:
1. Confirm the run is for the current HEAD.
2. Confirm the relevant jobs and tests actually executed.
3. Do not infer firmware or hardware success from host regression success.
4. Resume an unfinished task before selecting a new module.
5. Otherwise select the highest-risk unfinished module from the roadmap.

Do not poll indefinitely. If a run remains pending when the current execution budget ends, record its URL, run ID, head SHA, and pending state for the next conversation. Never claim background work will continue after the conversation.

## 7. Validation levels — never conflate them

Record each level separately. During ongoing development, prioritize repeatable L1/L2 checks and exact-HEAD Actions feedback. Schedule full MIPS firmware/image builds and physical-device validation as consolidated milestones in the later stage of the main development programme, and before any release claim that requires them. Pull a target build or hardware test forward when a specific defect cannot be responsibly assessed without it; record the reason and any blocker honestly.

- **L1 Static review:** source inspection, static checks, declarations, dependency and contract checks.
- **L2 Host regression:** host build, unit tests, CTest, relevant integration tests.
- **L3 Target build:** actual MIPS cross-compile, firmware integration, complete image generation and artifact validation.
- **L4 Hardware validation:** flash/boot and real XY-C1 or other target-device behavior checks.

Use explicit states such as NOT RUN, PASS, FAIL, BLOCKED, or NOT VERIFIED. A Level 1/2 pass must never be described as an L3/L4 pass.

## 8. Commits and branch safety

**Only `main` is an authorized development and commit branch.** This is unconditional; an unfinished task, failed CI, missing local checkout, or conversation boundary is not an exception.

**Explicit preserved-archive exception (not a development branch):** `backup/pre-plugin-overhaul-2026-11-14` was created at the user's explicit request as the pre-plugin-overhaul snapshot. It is intentionally immutable and must be preserved permanently. Never switch to, modify, commit, push, reset, rebase, cherry-pick, merge, rename, prune, or delete it. Read its root `BACKUP_BRANCH_DO_NOT_TOUCH.md` if its status needs confirmation. This single named archive does not weaken the main-only development rule.

- Never create, switch to, push, or commit on feature, fix, temporary, checkpoint, backup, or other branches. Do not create, update, or use pull requests for development or validation.
- Complete each independent fix on the current `main` HEAD, then commit and push it directly to `main`. Keep each commit coherent and reviewable; do not publish knowingly broken or incomplete production changes merely to preserve a work-in-progress.
- GitHub Actions must validate the actual `main` commit through its `push` event. Match every relevant run's `head_sha` to the exact pushed commit and live `main` SHA. A green result from a PR, legacy branch, older SHA, or another ref is never a substitute.
- When a main push fails, inspect the first causal failure and repair it with a new commit directly on `main`; repeat exact-head push validation until the relevant check passes. Never switch refs to obtain a green result.
- Re-fetch `main` immediately before every write. Use a non-forced fast-forward update tied to the expected parent/head; abort and reconcile if `main` has moved.
- Never force-push, amend/rewrite published history, reset a published ref, or overwrite an existing commit.
- If work cannot be safely completed, do not create a checkpoint branch. Record the exact current `main` SHA, completed evidence, uncommitted/persisted state, blocker, and precise next action in `SESSION_HANDOFF.md`. Only claim remote persistence for content actually committed to `main`.
- Existing non-main refs and PRs are legacy items, not authorized workspaces, **except** the explicitly preserved `backup/pre-plugin-overhaul-2026-11-14` archival snapshot. Never classify that backup as legacy, never port/merge from it as part of routine cleanup, and never delete or modify it. For other legacy PRs/refs, inspect unmerged changes, preserve and port every valid required change directly to current `main`, verify it there, then close the obsolete PR. Do not discard valid work or continue development on an old branch. Delete an obsolete legacy branch only after imported content is verified on `main` and a supported safe delete operation is available.
- **GPT Session Rotator is immutable unless the user explicitly requests a change.** Do not edit its workflow, state, schedule, trigger logic, or candidate records. Let it observe real new `main` SHAs naturally; do not reset/replay an existing SHA or manually trigger a rotation to bypass its existing 12-minute quiet-period behavior.

## 9. Three canonical project records

### dev_log.md — chronological history
For every code or documentation commit, add a newest-first entry with (for the commit that contains its own entry, label the result `THIS COMMIT` and resolve the SHA during the next recovery):
- Real Beijing time (UTC+8).
- Base SHA and resulting commit SHA when known.
- Scope and changed paths.
- Defect/problem and fix.
- Tests actually run and results.
- CI run ID/URL and exact head SHA, or PENDING/NOT RUN.
- Remaining risks and follow-up.

Do not lose or rewrite existing history. Let the root log accumulate to 119 complete records; when the next entry would reach 120, batch-archive the oldest 100 verbatim and retain the newest 20. Apply the 384 KiB early size trigger and 512 KiB hard limit from Section 0. Never archive one record at a time.

### REFACTOR_ROADMAP.md — whole-repository status
Track each real module's source paths, responsibilities, exposure, priority, audit scope, findings, implementation state, test levels, last commit, and next action. Distinguish:
- NOT INVENTORIED
- NOT AUDITED
- AUDITING
- AUDITED — FINDINGS
- AUDITED — NO FINDINGS
- FIXED — NOT VERIFIED
- HOST VERIFIED
- TARGET BUILD VERIFIED
- HARDWARE VERIFIED

A historical commit or audit note is evidence of a past edit, not proof that the current module has been completely audited or is safe.

### SESSION_HANDOFF.md — exact resumable checkpoint
Record:
- current main SHA and active branch;
- current task and selected module;
- files/functions reviewed;
- evidence and findings;
- completed and incomplete changes;
- working-tree/remote persistence state;
- test status;
- CI URL/run ID/head SHA/status;
- blockers and risks;
- exact first action for the next conversation.

Update the handoff in place only when the active task, checkpoint SHA, validation state, blocker, or exact next action materially changes, and before stopping. A commit alone does not require a handoff rewrite. Be specific; “continue HTTPD” is insufficient.

## 10. Cross-conversation recovery is mandatory

**The only user-facing takeover prompt needed is:**
`接手 https://github.com/Arlene206/padavan-4.4-2/ 继续开发`

Do not ask the user for a longer prompt, repeat questions already answered in Git, request a whole-repository upload, or depend on previous chat text. Do not generate recursive copy/paste prompts for the next GPT. The complete and current instructions for the next session belong in `SESSION_HANDOFF.md`; the bootstrap and this protocol explain how to recover and obey it. Every GPT must update the handoff in Git before ending, and the next GPT must resume from the live repository state.

Every new conversation must do this before coding:

1. Read this protocol.
2. Fetch the live `main` HEAD SHA.
3. Read `SESSION_HANDOFF.md`, `REFACTOR_ROADMAP.md`, and the newest `dev_log.md` entries.
4. Inspect the remote branch inventory only to identify legacy refs; inspect latest Actions runs and match their exact `head_sha`. Inspect open PR #1 (or any remaining legacy PR) only to inventory unmerged work that must be safely ported to `main`.
5. Compare all recorded SHAs with actual GitHub state.
6. Resume the exact unfinished `main` task or failed/pending exact-head `push` CI before choosing another task, unless the user explicitly assigns a higher-priority governance task.
7. Develop and commit only on `main`; do not create or use branches, checkpoint refs, or PRs.

Never rely on hidden memory or the previous chat transcript. GitHub is the project memory. A legacy branch/PR is evidence to inspect, not a valid development or CI target.

Before the current conversation's execution/context budget is exhausted:
- stop starting new modules;
- finish the current atomic task on `main` if safe;
- otherwise leave no branch/checkpoint behind; record the exact `main` SHA, validation state, any uncommitted work and exact first next action in the live handoff;
- append the milestone to `dev_log.md` as part of the coherent main commit; update `SESSION_HANDOFF.md` if the resumable task/next action changed; update `REFACTOR_ROADMAP.md` only if repository-wide status or priority changed;
- provide the exact next action.

If the available connector cannot perform a needed operation, state that limitation. Do not invent an action or claim success.

## 11. Initial and ongoing deliverables

The project is not complete merely because there are many commits or one component's tests are green.

A credible completion assessment requires:
- repository-wide module inventory;
- broad stability-defect triage and fixes across core services, lifecycle, persistence, networking, storage, management, and other integrated user-space modules;
- ongoing parallel security review with confirmed critical/high-impact findings handled promptly;
- documented audits of critical attack surfaces and services;
- evidenced fixes and regression coverage for confirmed defects;
- build/integration verification;
- actual MIPS firmware build and artifact checks;
- real target-device validation for release-critical behavior;
- preserved configuration and functional compatibility;
- current, consistent roadmap and handoff records.

At every stop, report briefly:
- what was actually changed;
- commit and branch;
- tests and exact CI result;
- what remains unverified;
- the next highest-priority action.

**Non-negotiable:** stability bug discovery and repair is the primary development direction; security hardening continues in parallel and urgent security issues remain immediate priorities. Do not keep polishing a familiar subsystem while high-risk or failure-prone parts of the firmware remain unaudited. Do not lose unfinished work when a ChatGPT conversation ends. Do not equate local tests with a working firmware image.

 
## 12. Handoff governance is a release-blocking invariant

These rules are mandatory for every future GPT:

- `SESSION_HANDOFF.md` is one live checkpoint, rewritten in place only when its content materially changes. Its previous versions are retrieved from normal Git history; routine archive copies are prohibited.
- `REFACTOR_ROADMAP.md` is one live dashboard, rewritten in place only when module inventory/status/evidence/priority changes. Its previous versions are retrieved from normal Git history; routine archive copies are prohibited.
- `dev_log.md` accumulates newest-first records from 20 up to 119. When the next entry would create record 120, archive the oldest 100 complete records verbatim in one batch and leave the newest 20. If the 384 KiB target is exceeded earlier, rotate complete old records in one batch while retaining the newest 20. Never exceed 512 KiB.
- Existing historical handoff/roadmap archives are immutable and remain indexed. Do not add new snapshots for ordinary live-file updates. Historical dev-log records remain verbatim in indexed, size-bounded shards.
- Do not mechanically update all three files after every code change. Update only the records whose facts changed. A code milestone normally receives a dev-log entry; a handoff changes only when the resumable checkpoint changes; the roadmap changes only when the global picture changes.
- Verify documentation CI on the exact commit SHA using GitHub Actions. Do not create a second docs-only commit just to record the completed CI result; the live Actions run is the source of truth.

A conversation is not ready to end until:
1. latest main SHA and active code SHA are distinguished;
2. `SESSION_HANDOFF.md` contains the exact active task and first next action;
3. `REFACTOR_ROADMAP.md` is accurate if repository-wide module status or priorities changed;
4. `dev_log.md` contains the complete milestone entry and stays within the 20–119 entry window and size limits;
5. the exact-head CI result has been checked or is explicitly left pending for the next takeover;
6. no unverified L3/L4 work is presented as complete.

