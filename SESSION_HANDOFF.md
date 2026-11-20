# 2026-11-20 continuation checkpoint

# Cross-session execution mandate

- Continue main-only development. Every logical source slice is one atomic main commit: create all blobs, one complete Git tree and one commit, then update main using a non-forced compare-and-swap against the exact parent SHA.
- Never amend, force-push, create development branches/PRs, or modify the frozen backup branch.
- Preserve GNU89, MIPS/uClibc, BusyBox and Linux 4.4 compatibility. Implement actual fixes with focused regressions, then inspect Actions for the exact pushed SHA.
- A PENDING Action does not block independent work. Diagnose failures from causal job logs; do not weaken tests to obtain a pass.
- Before context exhaustion, finish a safe slice and leave a factual next task. Do not ask the user what to do next.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

At the end of every session, inspect live main, verify that the frozen backup is unchanged, and check the exact-head Actions relevant to the latest source commit. Update this file only when the active task, source SHA, verification evidence, or first next action has materially changed. If the checkpoint is already accurate, leave it untouched and do not create a no-op commit.

## Current repository state

- Repository Arlene206/padavan-4.4-2; development branch main. Live main at recovery: c4e6b53c0b5a78ae841118a960bb1f137b162623. Frozen branch backup/pre-plugin-overhaul-2026-11-14 remains 72f4f157a084f8184b9597ca5bf334b8080e5807, read-only.
- PR #1 is closed/unmerged; the applicable AliDDNS CA-bootstrap, stdin-only HMAC helper, regression and workflow changes are already present on main.
- MiniUPnP source hardening now includes NAT/filter rollback, IPTC ownership cleanup, metadata/rule-state OOM handling, checked lease persistence and shared fatal event-loop teardown. The duplicate static int declaration in lease_file_remove() was corrected in 7065465d000b0bc17763b51c61533cebba243f62.
- Commit 567d5426fc26e6cbfb6664ec197566afa403755c replaced a bad escaped-regex guard with a literal source-prefix check. The corrected exact-head MiniUPnP run is queued at #44799593220; earlier #44799072749 on 7065465 failed at the over-escaped regex, not in the C helper behavior.
- Exact-head XY-C1 build #44799072655 on 7065465d000b0bc17763b51c61533cebba243f62 was still in progress at last poll; it is the relevant production-source Build/TRX/artifact validation.
- The XY-C1 workflow push/pull filters now include infosvr production and host-test paths (b90cdf4f6cd8ce0725bdee19dc7b605c0163a775). The infosvr dispatch test's incorrect assumption about response->MacAddress was fixed in 0d9719e7960f83d26a2df931902f61bee5b94bfe. Exact-head infosvr run #44799414652 nevertheless failed next in scripts/check_infosvr_packets.py because the checker still encoded the old request-MAC comparison and case-sensitive comment text; this commit updates the source contract and asserts the documented base-header/payload alias.
- On parent 0d9719e, XY-C1 #44799414860 succeeded but that commit only changed host tests; do not treat it as the target build for d1e668dc's infosvr production change. The relevant cumulative product-source build is #44799072655.
- Physical-router L4 remains NOT VERIFIED. Preserve main-only atomic non-forforced ref updates and never touch the frozen backup.

## Active task

1. Verify the infosvr regression/source-contract rerun on THIS COMMIT. Its GNU89 ASan/UBSan host binaries had passed on parent 0d9719e; verify the new ABI alias assertion and corrected source checker now pass.
2. Poll XY-C1 #44799072655 on 7065465d000b0bc17763b51c61533cebba243f62. Require Build XY-C1, TRX verification and artifact upload to all succeed before L3 closure. Diagnose a causal build error if present.
3. Continue MiniUPnP lease/firewall transaction review in upnp_redirect() and _upnp_delete_redir(): determine whether live NAT mutations can be compensated when the persistent lease rewrite fails, and whether lease state can be restored if firewall deletion fails. Inspect every caller and rollback helper before changing order.
4. Continue stability/security work across Padavan userland; do not repeat the already-covered IPTC/NAT-filter, lease-key, OOM-snapshot or shutdown fixes.

## Verification boundary

- Local: duplicated static declaration is rejected by GCC GNU89/Werror; corrected single declaration compiles. The declaration guard was rewritten after Actions exposed the escaped-regex mismatch.
- MiniUPnP #44799072749 failed on 7065465 at the malformed regex. The corrected source-prefix guard is in 567d5426; exact-head run #44799593220 must pass.
- Infosvr #44799414652 failed on 0d9719e in scripts/check_infosvr_packets.py after its packet, dispatch, signal and daemon lifecycle C tests passed; the remaining stale checker assertions and response-layout test are corrected in THIS COMMIT.
- XY-C1 #44799072655 on 7065465 was IN_PROGRESS at last poll. Older XY-C1 #44798143782 PASS on 545d8ca is only a baseline. Do not claim L3 until the relevant production-source SHA has full Build/TRX/artifact success.
- Non-image MIPS/uClibc runtime and physical router L4 remain NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch current main and inspect this commit's exact-head infosvr run and MiniUPnP run. Resolve any remaining failure from its job logs, not by weakening assertions.
2. Inspect the final conclusion and Build/TRX/artifact steps for XY-C1 #44799072655. If the build fails, take the first compiler or linker error and fix that production source slice on main.
3. Audit upnp_redirect() and _upnp_delete_redir() lease persistence/firewall consistency over all error edges; include the nftables/iptables update/delete helper compensation behavior and add an injectable GNU89 host regression.
4. Continue independent userland stability/security work while any unrelated Actions remain pending. Preserve atomic complete-tree CAS commits and keep physical L4 explicitly unverified.

## Handoff maintenance

- Keep `dev_log.md` newest-first. Archive only when the 120-record rotation threshold is reached, retaining the newest 20 records.
- Update the roadmap only when module priority/status/evidence changes materially; do not copy it into history on routine updates.
- Host regressions, exact-source firmware L3 and physical hardware L4 are separate verification levels.
