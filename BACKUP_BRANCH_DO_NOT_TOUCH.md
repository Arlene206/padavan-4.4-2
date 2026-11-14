# ⚠️ PRESERVED BACKUP BRANCH — DO NOT MODIFY OR DELETE

> **This is an intentional, owner-requested archival branch. It must be preserved.**

## Identity

- Repository: `Arlene206/padavan-4.4-2`
- Backup branch: `backup/pre-plugin-overhaul-2026-11-14`
- Snapshot source: `main`
- Source `main` commit at backup creation: `f6239a74b3feb03a056f937630e3f020e0445ec6`
- Created: 2026-11-14
- Purpose: preserve the complete repository state immediately before the planned major plugin overhaul.

## Mandatory instructions for every future maintainer and GPT session

**DO NOT DELETE THIS BRANCH. DO NOT MODIFY THIS BRANCH.**

This branch is a frozen reference snapshot, not a workspace. Do not:

- delete, rename, recreate, or automatically prune this branch;
- commit, push, amend, reset, rebase, cherry-pick, merge, or force-push to it;
- use it for development, testing, CI validation, feature work, or pull requests;
- change or remove this warning file;
- treat it as an obsolete/legacy branch when cleaning up repository refs.

The only purpose of this branch is to retain a recoverable pre-plugin-overhaul snapshot for comparison or emergency reference.

## Development must continue on `main`

The repository's normal development rule remains unchanged: **all future development, commits, pushes, and validation work belong on `main` only.**

This backup branch is a deliberate, user-authorized archival exception to the general main-only branch rule. That rule must not be misread as authorization to delete this explicitly preserved backup.

When resuming work:

1. Fetch and inspect the live `main` branch.
2. Continue all work directly on `main`.
3. Consult this backup branch read-only only when historical comparison or recovery is needed.
4. Never merge this backup branch into `main` merely to continue development.

## Snapshot integrity

The backup branch was created directly from `main` at commit `f6239a74b3feb03a056f937630e3f020e0445ec6`. The only commit added after branch creation is the commit that introduces this warning file. No firmware source, plugin, build configuration, WebUI, or other project file was changed as part of making this backup.

The source commit above is the exact pre-overhaul baseline. Preserve the branch and its history as-is.

## Protection status

This warning is a repository-level human/AI maintenance instruction. At creation time, no repository rulesets were configured, and the available GitHub connection could not administer branch-protection settings. Therefore, do not assume GitHub currently enforces technical write/delete restrictions on this branch. The preservation requirement still applies: **do not modify or delete it.**
