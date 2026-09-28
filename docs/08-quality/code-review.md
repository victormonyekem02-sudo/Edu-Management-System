# Self Code Review Record

**Reviewed:** all of `lib/`, `app/`, `sql/` at commit `65f61a1` (the end of implementation, before release).
**Method:** line-by-line reading against the checklist below, plus the cppcheck, clang-tidy and lizard outputs. Findings were fixed on branch `fix/code-review-findings` (commit `4b38c14`) and merged by merge commit.

## 1. Checklist

| # | Check | Result |
|---|-------|--------|
| C1 | Every service entry point calls `actor.require(...)` before doing anything | Pass |
| C2 | Course/department scope checked for lecturers and HoDs (BR-08, BR-11) | Pass |
| C3 | All user-supplied values reach SQL only through `bind` | Pass - verified by reading every `prepare(` call; only integer `PRAGMA user_version` uses concatenation, with an int |
| C4 | No password, hash or lockout data reaches the UI; UI uses services only | Pass - the UI reads names through `DirectoryService`, which returns `UserSummary` (introduced during implementation after three direct repository calls from the UI were spotted) |
| C5 | Mark changes and their audit rows are written in one transaction | Pass |
| C6 | Floating-point range checks reject NaN | Pass after DEF-01 |
| C7 | Errors are exceptions from the `LmsError` hierarchy, with user-oriented messages | Pass |
| C8 | Resources are RAII-managed, no raw `new`/`delete` | Pass |
| C9 | Input from files and the console is bounded in size | **Fail -> fixed (R3)** |
| C10 | Authentication does not leak which usernames exist (message and timing) | **Fail -> fixed (R1)** |
| C11 | Functions <= CCN 15, <= 80 lines | **Fail -> fixed** (2 functions, see metrics) |
| C12 | Names follow the coding standard, no shadowing | **Fail -> fixed (R2)** |
| C13 | Command-line errors give a clear message | **Fail -> fixed (R4)** |
| C14 | Code matches the class diagram (names, relationships, patterns) | **Fail -> fixed**: `DirectoryService` existed in code but not in the class diagram; added to class view 2 |
| C15 | Comments explain *why*, cite BR/FR IDs where rules are enforced | Pass |
| C16 | Platform-specific code isolated and compiles on MSVC | Fixed: `NOMINMAX`, own date parser instead of `sscanf` |

## 2. Findings and changes

| ID | Location | Finding | Change made | Verified by |
|----|----------|---------|-------------|-------------|
| R1 | `AuthService::login` | Unknown or inactive username returned without hashing, so it was ~50-150 ms faster than a wrong password. Username enumeration by timing (DEF-04). | Verify the supplied password against a dummy hash before throwing. | `AuthTest.UnknownUserCostsTheSameHashVerificationAsKnownUser` |
| R2 | menus, `SqliteRepositories.cpp`, `PasswordHasher` | cppcheck: shadowed names, pass-by-value, const reference | Renamed locals, `const&` parameters | cppcheck 0 findings |
| R3 | `StudentMenu::readSubmissionFile` | `@file` read any file of any size and could submit the database itself (DEF-05) | Regular-file check, database-file check, 100 KB limit before reading | ST25, ST26, ST27 |
| R4 | `main` | `--db` without value -> misleading "Unknown argument" (DEF-06) | Clear message, exit code 2 | manual run |
| R5 | `ModerationService::itemsWithStatus` | N+1 queries (4 look-ups per item) | **Not changed**: measured 108 ms for 800 items (limit 2 s); a join would complicate the repository interface for no user-visible gain. Revisit if NFR-03 is ever at risk. | PT NFR-03 |
| R6 | `AdminService::createDepartment` | Duplicate-name check in code duplicates the UNIQUE NOCASE constraint | **Kept**: gives a clear message instead of a constraint error; the constraint remains the real guarantee. | AdminTest |
| R7 | `DemoData.cpp` | Inserts assignments through the repository, bypassing the "deadline in the future" rule | **Kept, documented**: needed so the demo contains an overdue assignment for showing late penalties; only runs with `--seed-demo` on an empty database. | AT-02/03 |
| R8 | Passwords in memory | `std::string` copies of passwords are not wiped after use | **Accepted risk**: an attacker with memory access to a running school PC already controls it; recorded in professional issues as a limitation. | - |

## 3. Outcome
11 of 16 checklist items passed first time; 5 findings led to code changes (R1-R4 plus the complexity refactor); 4 were consciously accepted with a reason. All 264 tests pass after the changes, and 4 new tests were added (1 integration, 3 system).
