# Cross-session execution mandate

Continue repository development across GPT conversations. Recover the live state once, then implement. Do not repeat completed audits or ask the user what to do next.

- Develop on `main` only. Preserve the archival backup branch read-only.
- A logical slice means code or test-contract change, required validation, one atomic main commit, and exact-SHA Actions inspection.
- For multi-file edits, create all blobs, one complete Git tree and one commit, then update `main` with a non-forced compare-and-swap against the exact parent SHA. Never force-push or expose a partial tree.
- Before context exhaustion, inspect the exact current refs, tests and relevant Actions; continue independent work instead of waiting on pending checks.

## Mandatory handoff checkpoint

The checkpoint is mandatory; a document edit is conditional.

Every session MUST inspect the live repository state, including `main`, the immutable backup ref, relevant exact-head Actions, tests, and the first next task. If all checkpoint facts and the first action are already accurate, explicitly conclude the check with no file change and no commit. Do not create a no-op commit.

## Current repository state

- Repository: `Arlene206/padavan-4.4-2`; branch: `main`.
- Parent main SHA for this handoff correction: `3fd35299892c4499f9efbce8e50617307b71f2e3`.
- Frozen backup `backup/pre-plugin-overhaul-2026-11-14`: `72f4f157a084f8184b9597ca5bf334b8080e5807`; never modify or delete it.
- Recent implementation sequence: `72a1ca0a81a3cc7a30183bfb6527337c00bf94f8` aligned WAN IPv6 endpoint source contracts; `9e77c6924cc6bce18cb3fcb93d2e0b20ecd27f9c` added checked DHCPv6 LAN-address enumeration/publication; `a9aa6ba87339b0e21d1deed8464723a4aba50e9d` removed an obsolete callback local; `98b54718077878a93758c327277ce10fc8c6a44f` and `5e0100c059639b323c903351d8f1fab6ff181659` aligned WAN DHCP callback assertions; `1805fa23a010044d4f82c7e23e91695a6b7f904d` aligned the XY-C1 callback contract; `3fd35299892c4499f9efbce8e50617307b71f2e3` aligned the VPN IPv6 endpoint contract.
- `start_dhcp6c()` already snapshots DHCP flags and WAN hardware address through checked NVRAM transactions before session/DUID/config/client side effects. The lease callback uses generation/interface fencing, checked automatic settings, checked LAN-address publication, and independent DNS processing. Do not repeat that audit without new evidence.

## Active task

- The VPN export contract now validates the production `get_wan_addr6_host_status()` call and error gate before LAN/dual-stack IPv4 fallback. The exact-head VPN lifecycle run `45027707286` on `3fd35299892c4499f9efbce8e50617307b71f2e3` PASS.
- The AI handoff workflow `45027707271` on the same SHA failed only because this file's compact rewrite omitted mandatory end-of-session checkpoint wording and four required section headings. This correction restores the validator's checkpoint/section requirements while keeping the active task and verification facts current.
- The VPN checker change was test-only; this handoff correction changes documentation only. No product-source behavior changed in either commit.

## Verification boundary

- Exact SHA `3fd35299892c4499f9efbce8e50617307b71f2e3`: VPN server lifecycle #45027707286 PASS; AI handoff #45027707271 FAIL only on the missing mandatory checkpoint phrase/heading.
- Parent SHA `1805fa23a010044d4f82c7e23e91695a6b7f904d`: XY-C1 Build/TRX/artifact and Process inventory workflows #45027689588 and #45027689587 were IN PROGRESS at the last check; they do not establish L3 until all same-SHA target steps pass.
- No same-SHA current-head XY-C1 Build/TRX/artifact PASS is established here. L3 is VERIFIED only when Build, TRX verification and artifact upload all succeed on one exact source SHA. Hardware L4 remains NOT VERIFIED.

## Exact first action for the next GPT

1. Fetch live `main`; inspect the new exact-head AI handoff run for this checkpoint correction and confirm its result.
2. Recheck the exact-head VPN result, then inspect the pending XY-C1 Build/TRX/artifact and Process inventory results for their own exact SHAs.
3. Continue the wider Padavan userland stability/security rotation with a separate evidence-backed defect outside the completed DHCPv6 checked-read slice.

## Documentation anti-churn guard

Recovery is read-only by default. Never commit merely because a GPT has taken over. This is not a ban on useful documentation-only work; a checkpoint correction is justified when a required handoff fact or validator contract is materially wrong. If the current handoff remains accurate, leave it untouched.
