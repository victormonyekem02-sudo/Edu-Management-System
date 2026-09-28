# Configuration Management Plan

## 1. Configuration items
Source code (`app/`, `lib/`, `tests/`), database migrations (`sql/`), build scripts (`CMakeLists.txt`, `cmake/`), CI definition (`.github/workflows/ci.yml`), tool profiles (`.clang-format`, `.clang-tidy`), and every document and diagram source (`docs/`). Third-party code is **not** copied into the repository; it is identified by URL, version and SHA-256 in `cmake/Dependencies.cmake`, so its configuration is still exact and reproducible. Build outputs, databases and coverage data are excluded by `.gitignore`.

## 2. Branching strategy - GitHub Flow with release tags

```
main  ──●────────●────────●──────●─────────●─── (always releasable; tags v1.0, v1.1)
         \      / \      /        \       /
          feature/x   fix/y        feature/cr1-class-extension
```

| Branch | Purpose | Rules |
|--------|---------|-------|
| `main` | Integrated, tested, releasable code and documents | No direct commits after set-up; changes arrive only by merge from a short-lived branch; CI must be green |
| `feature/<topic>` | New functionality or documents (e.g. `feature/services`, `feature/cr1-class-extension`) | Branch from `main`; merge back with `--no-ff` (a merge commit records the unit of work) |
| `fix/<topic>` | Defect or review fixes (e.g. `fix/code-review-findings`) | Same as feature |
| `docs/<topic>` | Documentation-only changes | Same |

**Pull requests.** On GitHub every branch is merged through a pull request: CI (build, tests, coverage, static analysis) must pass, and the author completes the review checklist (`code-review.md`) in the PR description before merging. The development PC had no remote configured during v1.0 (risk R13), so the same flow was done locally with `git merge --no-ff`, which gives the same history shape. Once the repository is pushed, branch protection on `main` requires the CI checks to pass.

**Why GitHub Flow rather than Git Flow?** A single developer with two planned releases gains nothing from permanent `develop` and `release/*` branches. They would double the merges without adding a quality gate that CI and tags do not already provide. Tags mark the releases; a hotfix of v1.0 after v1.1 would be done by branching from the `v1.0` tag.

## 3. Commits
Conventional-commit style (`feat`, `fix`, `test`, `refactor`, `docs`, `build`, `ci`), with a body that explains *why* and cites requirement or defect IDs (e.g. "fix(policy): reject NaN raw marks in the penalty policy (DEF-01)"). See `git log --graph --oneline`.

## 4. Versioning and releases
Semantic versioning: `MAJOR.MINOR.PATCH`. The version is set once in `project(... VERSION x.y.z)` and passed to the code (`LMS_VERSION`), the package name and `lms --version`. Annotated tags:

| Tag | Content |
|-----|---------|
| `v1.0` | First release: all Must requirements, full documentation |
| `v1.1` | CR1: class-wide extensions shown on the student list (schema migration 002) |

The CI `release` job builds the Windows ZIP with CPack when a `v*` tag is pushed and attaches it to a GitHub release, with `release-notes.md` as the text.

## 5. Continuous integration
`.github/workflows/ci.yml` runs on every push and pull request:

| Job | What it proves |
|-----|----------------|
| build-and-test (Ubuntu + Windows x Debug + Release) | The code builds on both platforms (NFR-12); Linux with `-Werror`; all 264 functional tests pass; Release runs the performance tests |
| coverage | gcovr line coverage of `lib/` must be >= 70 %, otherwise the job fails (`--fail-under-line 70`); HTML report uploaded as an artifact |
| static-analysis | cppcheck must report nothing (`--error-exitcode=1`); clang-tidy report uploaded |
| release (tags only) | Packaged ZIP attached to the GitHub release |

**CI evidence.** The pipeline has been validated locally job by job (same compilers and flags, results in `docs/07-testing/reports` and `docs/08-quality/reports`). Screenshots or links of the GitHub Actions runs are added after the first push (instructions in the README).

## 6. Change control
Changes after v1.0 follow the procedure used for CR1 (`docs/10-maintenance/cr1-change-request.md`): record the request, classify the maintenance type, analyse the impact on requirements, diagrams, classes, tables and tests, implement on a branch, update the affected documents, run the regression suite, merge, and tag a new version.
