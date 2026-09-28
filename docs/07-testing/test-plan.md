# Test Plan (V-Model)

## 1. Objectives and scope
Verify that every requirement in the SRS is implemented as designed and validate that the system meets stakeholder needs. In scope: `lms_core` (domain, services, policy, security, data access), `lms_ui` (console), the packaged executable. Out of scope: third-party libraries (SQLite, Argon2, GoogleTest), which are trusted and pinned by hash.

## 2. V-Model mapping

```
Requirements (SRS, use cases) ....................... Acceptance testing  (AT-xx)
   System specification (SRS FR/NFR) ............. System testing       (ST-xx, PT)
      Architecture (SDD 1, component diagram) ... Integration testing  (IT_xx suites)
         Detailed design (class, state, SQL) ..... Unit testing         (UT_xx suites)
                          Implementation (code)
```

| Level | Verifies artefact | Test basis | Technique | Environment / tool | Entry criteria | Exit criteria |
|-------|-------------------|-----------|-----------|--------------------|----------------|---------------|
| **Unit** (UT) | Detailed design: classes `LatenessCalculator`, policies, `User` hierarchy, `SubmissionLifecycle`, hasher, password policy, text/time helpers, publisher | Class diagram, state machine, BR-01/02, NFR-04/05 | Equivalence partitioning, boundary-value analysis, state-transition table (all 25 pairs), decision table (permission matrix) | GoogleTest, no database | Code compiles without warnings; unit under test reviewed | 100 % of unit tests pass; every boundary in section 5 covered |
| **Integration** (IT) | Architecture: services working with the SQLite repositories, transactions, migrations, observer | Component diagram, sequence diagrams, schema | Use-case-driven scenarios, fault injection (rollback, constraint violation), security tests (injection) | GoogleTest + in-memory SQLite (`TestSchool` fixture), `FixedClock` | Unit exit criteria met; schema migrates | 100 % pass; line coverage of `lib/` >= 70 % (NFR-11) |
| **System** (ST, PT) | SRS functional and non-functional requirements | Use-case specifications (main, alternative and exception flows); NFR table | Scenario testing through the real console UI with scripted input; performance measurement | GoogleTest driving `Application` with `istringstream`; Release build for PT | Integration exit criteria met; build of the complete application | Every use-case flow of UC-01/02/03 has a passing test; NFR-01..03 limits met; no open Critical/Major defect |
| **Acceptance** (AT) | Stakeholder needs (stakeholder analysis, conflicts C1-C7) | Stakeholder statements | Business scenarios per stakeholder on the packaged executable | `tests/acceptance/run_acceptance.py` against the real `lms` binary and a fresh demo DB | System exit criteria met; release candidate built | All AT scenarios pass; AT-06 usability test with a real student (manual, at the school) |

## 3. Test environment
- Development: Windows 11 x64; compilers Clang 21 (Zig 0.16) and MinGW-w64 GCC 16.2; CMake 4.x; Ninja.
- CI: GitHub Actions `ubuntu-latest` (GCC, -Werror, coverage, static analysis) and `windows-latest` (MSVC), Debug and Release.
- Test data: `TestSchool` fixture (2 departments, 2 HoDs, 2 lecturers, 3 students, 2 courses, 1 assignment, deadline = T0 + 7 days) and, for performance, 800 generated students, 20 assignments and 800 marked submissions.
- Time is controlled with `FixedClock` so deadline tests are deterministic.

## 4. Test design techniques

### 4.1 Equivalence partitioning and boundary values - lateness (BR-02)
Input: submission time relative to the effective deadline (seconds).

| Partition | Range | Expected days late | Boundary values tested |
|-----------|-------|--------------------|------------------------|
| EP1 on time | offset <= 0 | 0 | -7 days, -1 s, **0** |
| EP2 day 1 | 0 < offset <= 24 h | 1 | **+1 s**, +1 h, **+24 h** |
| EP3 days 2-3 | 24 h < offset <= 72 h | 2 or 3 | **+24 h + 1 s**, +48 h, +48 h + 1 s, **+72 h** |
| EP4 > 3 days | offset > 72 h | >= 4 | **+72 h + 1 s**, +30 days |

Tests: `UT_LAT/LatenessBoundaries.*` (12 cases), `IT_MRK/LateMarking.*` (6 cases end-to-end with marks), `SubmissionTest.SubmissionExactlyAtDeadlineIsOnTime`, `ST03`, `ST08`.

### 4.2 Equivalence partitioning and boundary values - penalty (BR-01/02), max = 25, raw = 20

| Days late | Deduction | Final | Partition |
|-----------|-----------|-------|-----------|
| 0 | 0 | 20 | no penalty |
| 1 | 2.5 | 17.5 | penalised (lower boundary) |
| 3 | 7.5 | 12.5 | penalised (upper boundary) |
| 4 | 20 | 0 | zero partition (lower boundary) |
| raw 2, 3 days | 2 (capped) | 0 | deduction cap |
| raw 0 / raw 25 | - | - | mark range boundaries |
| raw 25.01, -0.5, NaN | error | - | invalid partitions |

Tests: `UT_PEN/TenPercentPolicy.*` (11 cases) and `UT_PEN.*` invalid-input tests.

### 4.3 Other boundaries
| Item | Invalid | Valid boundary | Invalid | Tests |
|------|---------|----------------|---------|-------|
| Password length | 7 | 8 ... 128 | 129 | `UT_SEC/PasswordPolicyCases`, `UT_SEC.PasswordLengthUpperBoundary` |
| Username length | 2 | 3 ... 32 | 33 | `AdminTest.RejectsInvalidAccounts` |
| Max mark | 0 | 1 ... 1000 | 1001 | `AssignmentTest.ValidatesDeadlineMaxMarkAndTitle` |
| Assignment deadline | now | now + 1 s | - | same |
| Extension deadline | = original | original + 1 min | - | `AssignmentTest.ExtensionRules` |
| Submission size | empty | 100 000 chars | 100 001 | `SubmissionTest.ExceptionFlowEmptyOrOversizedWork` |
| Feedback size | empty | 1 ... 5 000 | 5 001 | `MarkingTest.ValidatesMarkRangeAndFeedback` |
| Failed logins | 4 (no lock) | 5 (lock) | - | `AuthTest.FourFailures*`, `AuthTest.FiveFailures*` |
| Lockout time | 15 min - 1 s (locked) | 15 min (unlocked) | - | `AuthTest.FiveFailuresLockAccountForFifteenMinutes` |
| Date input | 2026-02-29, 2026-04-31, 24:00 | 2028-02-29, 23:59 | - | `UT_TXT.ParseLocal*` |

### 4.4 State-transition testing
All 25 (from, to) pairs of the submission state machine: `UT_LIF/TransitionTable.*`; invalid transitions through services: `ExceptionFlowAlreadyMarked`, `LecturerCannotChangeModeratedMark`, `ReleasedMarksAreFinal`, `ApproveRequiresMarkedState`.

### 4.5 Decision table - access control
4 roles x 13 permissions = 52 cases (`UT_USR/PermissionMatrix.*`) plus scope rules (own course, own department) in the integration suites.

## 5. System test cases derived from the use cases

| ID | Use case flow | Input summary | Expected result |
|----|---------------|---------------|-----------------|
| ST01 | UC-01 main flow | stud submits text before deadline | "Submitted on time", content stored |
| ST02 | UC-01 A1 resubmit | submit twice before deadline | "Resubmitted (version 2)", latest content |
| ST03 | UC-01 A2 late | clock = deadline + 1 h | list shows "deadline passed"; "Submitted LATE" + rule |
| ST04 | UC-01 E1 | resubmit after deadline | "[ERROR] The deadline has passed.", original kept |
| ST05 | UC-01 E4 | assignment 99 | "[ERROR] Assignment 99 does not exist.", session continues |
| ST06 | UC-01 E5 | blank work | "[ERROR] Submission must not be empty." |
| ST07 | UC-01 E3 | non-enrolled student | "[ERROR] You are not enrolled..." |
| ST08 | UC-02 main + penalty | late by 10 s, raw 20 | "1 day(s) late"; "final 17.5 (1 day late - 10% of 25 = 2.5 deducted)"; multi-line feedback stored |
| ST09 | UC-02 E3 | re-mark with blank reason | "[ERROR] A reason is required..." |
| ST10 | UC-02 E1 | -3, "abc", 30 | re-prompt; "[ERROR] Mark must be between 0 and 25." |
| ST11 | UC-03 main | approve + release; student views | notification, "Mark approved", "1 mark(s) released", student sees 20/25 and feedback |
| ST12 | BR-07 | marked but not released | student sees "Not released yet", no mark, no feedback |
| ST13 | UC-03 A1 adjust | new mark 18 + reason | "Mark adjusted", audit shows MODERATION_ADJUST and reason |
| ST14 | UC-03 E1 | HoD of other department | "[ERROR] This submission belongs to another department." |
| ST15 | UC-03 A2 | release answered "n" | "Nothing released.", still hidden |
| ST16 | UC-13 | wrong password | "[ERROR] Invalid username or password." |
| ST17 | FR-02 | each role logs in | each sees only its own menu items |
| ST18 | NFR-08 | letters, out-of-range numbers | re-prompt, no crash |
| ST19 | NFR-08 | input ends mid-submission | "Input closed - exiting.", exit code 0 |
| ST20 | FR-21 | empty database | first-run admin creation, password rule enforced |
| ST21 | UC-08/11 | admin creates and enrols a student | student can log in and see the assignment |
| ST22 | UC-04 | lecturer grants extension | student notified and sees "(extended)" deadline |
| ST23 | UC-14 | change password | new password works |
| ST24 | NFR-05 | 5 wrong passwords | "temporarily locked" even with the correct password |
| ST25 | UC-01 A3 | `@file` | file content stored |
| ST26 | UC-01 E5 | missing / oversized file | error messages, nothing stored |
| ST27 | security (R3) | `@lms.db` | "[ERROR] That file cannot be submitted." |
| PT-01..05 | NFR-01..03 | 800 students | timings under limits (see test report) |

## 6. Acceptance tests

| ID | Stakeholder | Scenario | Pass criterion |
|----|-------------|----------|----------------|
| AT-01 | IT officer | First run of the packaged exe on an empty database, no network | Admin created, menu reachable, clean exit |
| AT-02/03 | Principal, HoD | Late submission -> marked -> hidden -> moderated -> released | Penalty applied per BR-02; nothing visible before release; audit trail complete |
| AT-04 | Student rep | Submit, resubmit before deadline, late submission of another assignment | Version 2 stored; late work accepted |
| AT-05 | Lecturer | Individual extension for one student | Extended student on time; classmate late |
| AT-06 | Principal, Students | First-time student completes "submit assignment" with the manual | <= 2 minutes (NFR-10) - **manual test to be run at the school with a volunteer; not yet executed** |

Automated runner: `python tests/acceptance/run_acceptance.py <lms-exe> docs/07-testing/reports/acceptance`.

## 7. Defect management
Defects are logged in `defect-log.md` with ID, date, severity (Critical: data loss/security/wrong mark released; Major: requirement not met; Minor: workaround exists; Trivial: cosmetic), the test that found the defect, cause, fix (commit) and retest result. A defect is closed only when the test that exposed it, and the full regression suite, pass.

## 8. Suspension and resumption criteria
Testing is suspended if the build fails or more than 10 % of tests fail due to one cause; it resumes when a fix is merged and the build is green.

## 9. Deliverables
Test plan (this file), test code (`tests/`), results (`reports/functional-results-*.txt/.xml`, `performance-results-*.txt`, `acceptance/`), coverage (`reports/coverage/`), defect log, test report.
