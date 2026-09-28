# Static Analysis Report

Two analysers were run on all project sources (`lib/src`, `app/src` and the headers they include). Third-party code was excluded.

| Tool | Version | Configuration |
|------|---------|---------------|
| cppcheck | 2.17.1 | `--enable=warning,style,performance,portability --std=c++17 --check-level=exhaustive --inline-suppr` |
| clang-tidy | LLVM 22.1.8 | `.clang-tidy`: bugprone, cert, clang-analyzer, cppcoreguidelines, misc, modernize, performance, readability (minus the noisy checks listed in the file) |

Raw outputs: [`reports/cppcheck-before.txt`](reports/cppcheck-before.txt), [`reports/cppcheck-after.txt`](reports/cppcheck-after.txt), [`reports/clang-tidy-before.txt`](reports/clang-tidy-before.txt), [`reports/clang-tidy-after.txt`](reports/clang-tidy-after.txt).

## 1. cppcheck

**Before: 16 findings** (0 errors, 0 warnings, 1 performance, 15 style).

| Finding | Count | Action |
|---------|------:|--------|
| `passedByValue` - `Argon2Params` copied into the constructor | 1 | **Fixed**: `const Argon2Params&` |
| `constParameterReference` - `lastId(Database&)` | 1 | **Fixed**: `const Database&` |
| `shadowFunction` - locals `title` and `items` shadow the menu's `title()`/`items()` methods | 4 | **Fixed**: renamed (`courseTitle`, `assignmentTitle`, `submissions`, `queue`). Genuine readability risk: `title` inside a `RoleMenu` method looked like a call to `title()`. |
| `useStlAlgorithm` - raw loops that could be `std::transform` / `accumulate` / `find_if` | 9 | **Not changed, suppressed project-wide.** The loops build table rows with several fields; `std::transform` with `back_inserter` and a lambda was judged less readable for the maintainers (students and teachers), and there is no correctness or performance gain. |
| `constParameter` - `argv` of `main` | 1 | **Not changed**: `int main(int, char*[])` is fixed by the C++ standard; suppressed inline with a comment. |

**After: 0 findings** with the documented suppressions. CI enforces this with `--error-exitcode=1`.

## 2. clang-tidy

**Before: 59 findings.**

| Check | Count | Action and reasoning |
|-------|------:|----------------------|
| `cppcoreguidelines-pro-bounds-avoid-unchecked-container-access` | 13 | Disabled in `.clang-tidy`: every `operator[]` is indexed by a loop variable bounded by `size()`; the new rule reports all of them without distinguishing safe use. Where the index comes from input (menu choice, role, month) the code uses `.at()`. |
| `misc-const-correctness` | 9 | **Fixed** (added `const`). |
| `readability-use-concise-preprocessor-directives` | 8 | **Fixed** (`#ifdef _WIN32`). |
| `modernize-avoid-c-arrays` / `pro-bounds-constant-array-index` | 8 | **Fixed**: `std::array` / `std::string` with `.at()`. |
| `readability-math-missing-parentheses` | 4 | **Fixed**. |
| `readability-simplify-boolean-expr` | 4 | **1 fixed, 3 rejected.** clang-tidy suggests rewriting `!(x >= 0 && x <= max)` as `x < 0 || x > max`. The rewrite is not equivalent for **NaN**: it would reintroduce defect DEF-01 in the policy, `MarkingService` and `ModerationService`. Those three lines keep the original form with `NOLINTNEXTLINE` and a reason. The fourth (a status comparison, no floating point) was simplified. |
| `performance-inefficient-vector-operation` | 3 | **Fixed**: `reserve()`. |
| `modernize-return-braced-init-list` | 2 | **Fixed**. |
| `cert-err33-c` - result of `localtime_s`/`localtime_r` ignored | 1 | **Fixed**: a failed conversion now yields a zeroed `tm` instead of uninitialised data. A real robustness defect. |
| `bugprone-unchecked-optional-access` - `*departmentId` in `AdminService::createUser` | 1 | **Fixed**: the guarantee from an earlier check is now explicit in the condition. |
| `bugprone-branch-clone` - SUBMITTED and LATE cases identical | 1 | **Fixed**: cases merged; the state machine is easier to read. |
| `cppcoreguidelines-special-member-functions` - `EchoGuard` | 1 | **Fixed**: move operations deleted (rule of five). |
| `readability-implicit-bool-conversion` (`WINBOOL`) | 1 | **Fixed**. |
| `cppcoreguidelines-pro-type-reinterpret-cast` | 1 | **Kept**, suppressed: required to convert SQLite's `const unsigned char*` text to `char*`. |
| `bugprone-empty-catch` | 1 | **Kept**, suppressed: the rollback error is deliberately discarded so that the original exception, which explains the failure, reaches the caller. |
| `modernize-use-auto` | 1 | **Fixed**. |

**After: 0 findings.**

## 3. Lessons
- Tools find defects that tests do not reach: `cert-err33-c` and the unchecked optional were real (if unlikely) failure paths.
- Tool advice is not always correct: the DeMorgan suggestion would have broken NaN handling. Each suppression carries a reason so the decision can be reviewed.
- GCC's `-Wshadow` found a problem (DEF-03) that Clang missed. Building with two compilers is itself a form of static analysis.
