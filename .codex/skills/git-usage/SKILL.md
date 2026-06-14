---
name: git-usage
description: Safe Git workflow for local repositories. Use when Codex needs to inspect git status, initialize or prepare a repository, add or update remotes, rename branches, stage and commit changes, push to GitHub, troubleshoot authentication or 403 errors, protect user changes, or check for secrets before committing or pushing.
---

# Git Usage

Use this skill for git operations in the current workspace.

## Safety Rules

- Always run `git status --short --branch` before changing git state.
- Check whether the current directory is the repository root with `git rev-parse --show-toplevel`; if it fails, inspect nearby folders before running git commands.
- Never use destructive commands such as `git reset --hard`, `git checkout --`, `git clean`, or force-push unless the user explicitly asks and the target is confirmed.
- Do not revert unrelated user changes. If unrelated changes are present, leave them alone or stage only the requested paths.
- Before committing or pushing, scan for secrets and local-only values such as API keys, passwords, tokens, private IP endpoints, `.env`, and config headers.
- If a file contains real local credentials, prefer tracking an example file and ignoring the real file, such as `config.example.h` plus `config.h` in `.gitignore`.

## Repository Prep

Run:

```powershell
git rev-parse --show-toplevel
git status --short --branch
git remote -v
```

If no repository exists and the user wants one:

```powershell
git init
git branch -M main
```

If a remote is requested:

```powershell
git remote add origin <url>
```

If `origin` already exists, inspect it first:

```powershell
git remote -v
```

Use `git remote set-url origin <url>` only when the existing remote is clearly wrong or the user asked to change it.

## Staging And Commit

Prefer path-specific staging:

```powershell
git add -- README.md src/main.cpp
```

For deleted tracked files that should stay deleted:

```powershell
git add -u -- path/to/file
```

Review what will be committed:

```powershell
git diff --cached --name-status
git diff --cached --check
```

Commit with a concise message:

```powershell
git commit -m "Describe the change"
```

## Secret Checks

Use targeted scans before commit and before push:

```powershell
git grep --cached -n -I -E "(password|passwd|secret|token|api[_-]?key|bearer|mqtt_pass|wifi_password)" -- .
git grep -n -I -E "(password|passwd|secret|token|api[_-]?key|bearer|mqtt_pass|wifi_password)" HEAD -- .
```

If matches are placeholders, verify they are clearly non-secret. If matches are real secrets, stop and remove them from the index or rewrite the unpushed commit before pushing.

For local config files that must remain on disk:

```powershell
git rm --cached -- path/to/config.h
```

Then add the local file to `.gitignore` and commit a safe example file.

## Push Workflow

Rename the branch and push:

```powershell
git branch -M main
git push -u origin main
```

If push fails with `403` or `Permission denied`, report the account shown by GitHub and do not keep retrying blindly. The fix is usually one of:

- Sign in with the GitHub account that owns or can write to the repo.
- Add the current account as a collaborator.
- Switch the remote URL to SSH or a token-authenticated HTTPS URL after the user has configured credentials.

After a successful push, report the branch, remote URL, and commit SHA.

## Amending Unpushed Commits

If the current commit has not been pushed and needs cleanup:

```powershell
git add -- <paths>
git commit --amend --no-edit
```

After amending, re-run secret checks against `HEAD`.
