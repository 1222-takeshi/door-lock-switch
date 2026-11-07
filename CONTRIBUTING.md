# Contributing / Development Process

This repository is developed with the following workflow. The rules apply to
every change, no matter how small.

## Workflow

1. **Roadmap first.** Work follows `ROADMAP.md`. If a task is not on the
   roadmap, add it there (or propose it) before starting.
2. **One Issue per task.** Every unit of work is tracked as a GitHub Issue
   before implementation starts.
3. **TDD.** Write the test first, watch it fail, then implement until it
   passes. Hardware-dependent behavior is isolated behind stubs/mocks so the
   logic stays testable off-device.
4. **Small pull requests.** Keep each PR to roughly **200 lines of changes**
   or less. If a change grows beyond that, split it into multiple Issues and
   PRs. Small PRs keep review meaningful and rollbacks cheap.
5. **AI review before merge.** Every PR is reviewed (AI-assisted review) and
   all tests must pass before merging.
6. **Merge.** Squash or merge commits are both fine; the PR description must
   state what was verified and how.

## Conventions

- Documentation is written in English. Each repository carries a README in
  English (`README.md`) and Japanese (`README.ja.md`).
- Commit messages are short, in English, and describe the change, not the
  process.
- Acceptance criteria are decided before seeing test/measurement results and
  are not loosened afterwards.
- Safety-related behavior (see each repository's plan) is never weakened to
  make a test pass.
