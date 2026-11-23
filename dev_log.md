## 2026-11-23 08:56 UTC+8 — Trigger XY-C1 for MTK ESW kernel driver changes

- Parent main SHA: `a3a7d64ba1fbaab7cc6f6d74d9c9650c017242ee`. MTK ESW link-mode policy source was added in `a905ff6c` and its Switch CLI GNU89/ASan/UBSan host check passed. The XY-C1 workflow push allowlist omitted the kernel driver `ioctl_mt762x.c`, its policy header and test; therefore that kernel-only change did not independently trigger the target build.
- A descendant `a3a7d64b` happened to trigger XY-C1 via a separate Samba source change and includes the MTK code, but this was incidental. That build was still pending at last poll.
- Added the three MTK ESW source/test paths to the XY-C1 push allowlist and pinned their presence with the existing Switch CLI host source contract. Changing the XY-C1 workflow itself is also in its trigger list, so this commit should produce a same-SHA build for the current tree.
- Updated SESSION_HANDOFF and the Switch/PHY roadmap verification boundary.
- Resulting commit: THIS COMMIT.

---
