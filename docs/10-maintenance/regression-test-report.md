# CR1 - Implementation and Regression Test Report (release v1.1)

## 1. Implementation summary
Implemented on branch `feature/cr1-class-extension` exactly as planned in the impact analysis ([cr1-change-request.md](cr1-change-request.md)). No file outside the predicted list was changed.

| Area | Change |
|------|--------|
| Schema | `sql/002_class_extension.sql` - new table `class_extension`; applied automatically, `PRAGMA user_version` 1 -> 2 |
| Domain / data | `ClassExtension` struct; `IAssignmentRepository::saveClassExtension/findClassExtension` + SQLite implementation (UPSERT, bound parameters) |
| Business logic | `AssignmentService::grantClassExtension`, `classExtension`, `deadlineFor` (returns deadline + `ExtensionKind`); `effectiveDeadline` delegates to it; new event `ClassExtensionGranted`; `NotificationService` notifies every enrolled student |
| UI | Lecturer option 6 "Extend the deadline for the whole class"; extensions list shows the class extension; student list columns *Original deadline*, *Your deadline*, *Extension* |
| Version | 1.1.0 |
| Documents updated | SRS (FR-22, BR-03, glossary), use cases (UC-16, UC-01 step 5), SDD (sections 1, 4.3, 4.4), diagrams (use case, class views 1-3 and overview, ERD, state, activity, submit sequence), traceability matrix, lecturer and student manuals, release notes, risk register, README |

No changes were needed in `SubmissionService` or `MarkingService`. Both already asked `AssignmentService` for the effective deadline, so late status, penalties and resubmission picked up class extensions automatically.

## 2. Regression testing

### Run 1 - existing suite unchanged, new code in place
Evidence: [`reports/regression-run1-before-test-updates.txt`](reports/regression-run1-before-test-updates.txt)

| Result | Count |
|--------|------:|
| Passed | 263 |
| Failed | 1 - `SystemTest.ST22_LecturerGrantsExtensionShownToStudent` |

The failure was **predicted by the impact analysis**. ST22 looked for the v1.0 display text `"<deadline> (extended)"`, which CR1 deliberately replaced with the *Extension* column. The behaviour under test (extension granted, student notified, new deadline shown) was unchanged. The expectation was updated to check the new deadline and the word `individual`. This is an intended specification change, not a regression.

### Run 2 - updated and new tests
Evidence: [`reports/regression-run2-v1.1.txt`](reports/regression-run2-v1.1.txt), XML [`reports/regression-run2-v1.1.xml`](reports/regression-run2-v1.1.xml)

| Suite | Tests | Result |
|-------|------:|--------|
| All v1.0 tests (incl. updated ST22 and schema-table list) | 264 | PASS |
| New `ClassExtensionTest` (integration) | 11 | PASS |
| New `IT_DB.UpgradesVersion1DatabaseWithoutDataLoss` | 1 | PASS |
| New system tests ST28, ST29 | 2 | PASS |
| **Total** | **278** | **278 PASS, 0 FAIL** |

Both compilers: GCC 16.2 (Debug, `-Werror`, coverage) and Clang 21 (Release): 278/278.

### New test cases (CR1)
| Test | Checks |
|------|--------|
| ClassExtensionTest.AppliesToEveryEnrolledStudent | Every student's deadline and kind = class; original kept |
| ...LaterClassExtensionOverridesEarlierIndividualOne / LaterIndividualExtensionOverridesClassOne | "Latest wins" in both orders; classmates unaffected by individual extensions |
| ...WithoutExtensionsKindIsNone | Default kind and display names |
| ...Validation | Boundary: = original deadline rejected, +1 min accepted; empty reason; other lecturer, student, HoD refused; unknown assignment |
| ...RegrantingReplacesTheClassExtension | Replace semantics; access to the stored reason |
| ...ReclassifiesCoveredLateSubmissions | LATE -> SUBMITTED only when the new deadline covers the submission |
| ...RefusedOnceMarksAreModerated | Allowed while only MARKED, refused after moderation |
| ...LatePenaltyMeasuredFromClassDeadline | No penalty for work inside the class deadline |
| ...StudentsMayResubmitUntilTheClassDeadline | Resubmission window follows the class deadline; closed 1 s after it |
| ...EveryEnrolledStudentIsNotified | Notifications to enrolled students only |
| IT_DB.UpgradesVersion1DatabaseWithoutDataLoss | Real v1.0 schema + data -> v2; data intact; new table usable |
| ST28 | Lecturer menu option 6 end to end; both students see *class* and the original deadline |
| ST29 | Student with a later individual extension sees *individual*; classmate sees *class* |

### Other verification of v1.1
| Check | Result | Evidence |
|-------|--------|----------|
| Performance NFR-01..03 | Login 59 ms, list 1.1 ms, submit 0.5 ms, queue 38 ms, release 120 ms - all PASS | [`reports/performance-results-v1.1.txt`](reports/performance-results-v1.1.txt) |
| Acceptance AT-01..AT-05 (regression) and new AT-07 (class extension) | 5/5 PASS | [`reports/acceptance/`](reports/acceptance/) |
| Line coverage of `lib/` | 96.1 % (1 896 / 1 973), up from 95.9 % | [`reports/coverage-summary-v1.1.txt`](reports/coverage-summary-v1.1.txt) |
| cppcheck / clang-tidy on changed files | 0 findings | - |
| lizard | no function above CCN 15 | - |

## 3. Conclusion
CR1 is implemented, every predicted artefact was updated, the complete regression suite passes, and existing v1.0 databases upgrade without data loss. Released as **v1.1**.
