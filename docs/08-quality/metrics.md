# Code Metrics

Tools: `lizard` 1.17 (NLOC = non-comment lines of code, CCN = cyclomatic complexity) and a line count of non-blank lines. Raw reports: [`reports/lizard-before.txt`](reports/lizard-before.txt), [`reports/lizard-after.txt`](reports/lizard-after.txt).

## 1. Size (release v1.0, non-blank lines)

| Part | Lines |
|------|------:|
| `lib/include` (headers of domain, services, data, security) | 1 177 |
| `lib/src` (business logic + data access) | 2 306 |
| `app/` (console UI) | 1 150 |
| `sql/` | 134 |
| **Production total** | **4 767** |
| `tests/` | 1 988 |
| Test-to-production ratio | 0.42 |

lizard (functions in `lib/src` + `app/src`): **3 230 NLOC in 274 functions**, average 11.0 NLOC and **average CCN 2.8** per function.

## 2. Cyclomatic complexity

Threshold (coding standard CS-D2, NFR-11): **CCN <= 15** per function.

| Function | Before | After | Change |
|----------|-------:|------:|--------|
| `time_util::parseLocal` | **19** | 8 | extracted `matchesDateTimePattern`, `digitsAt`, `inRange` |
| `NotificationService::onSubmissionEvent` | **16** | 10 | one handler per recipient group |
| `permissionName` (switch over 13 permissions) | 14 | 14 | accepted: flat switch, trivially readable |
| `ModerationService::releaseAssignment` | 12 | 12 | - |
| `AdminService::createUser` | 11 | 11 | - |
| `main` | 11 | 11 | - |
| `AuthService::login` | 10 | 10 | - |
| `AssignmentService::grantExtension` | 10 | 10 | - |
| `MarkingService::mark` | 9 | 9 | - |
| `SubmissionService::submit` | 9 | 9 | - |
| `LatenessCalculator::daysLate` | 2 | 2 | - |

Functions above the threshold: **2 before, 0 after**. The key business-rule functions (`submit`, `mark`, `daysLate`, `apply`) stay below 10, which keeps the number of test paths manageable: `mark` has 9 paths and is covered 100 % by line.

## 3. Coverage (from the test report)
Business-logic line coverage **95.9 %**, function coverage 94.9 %.

## 4. Interpretation
Complexity is concentrated in input validation and orchestration (login, account creation, release). Those functions are exactly the ones with the most tests (AuthTest: 11, AdminTest: 11, ModerationTest: 12). No class exceeds 230 NLOC; `SqliteRepositories.cpp` is the largest file (about 490 lines) because it holds all SQL, a deliberate concentration so that NFR-06 can be inspected in one place.
