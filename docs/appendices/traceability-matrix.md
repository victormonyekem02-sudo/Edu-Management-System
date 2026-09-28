# Requirements Traceability Matrix

Forward trace: requirement -> use case -> design elements (classes, diagrams, tables) -> source files -> tests. Every functional requirement is implemented and has at least one passing automated test. Paths are relative to the repository root; `S:` = `lib/src/services/`, `U:` = `app/src/`.

## Functional requirements

| FR | Use case | Classes | Diagrams | Tables | Source files | Tests |
|----|----------|---------|----------|--------|--------------|-------|
| FR-01 Login | UC-13 | `AuthService`, `IPasswordHasher`, `UserFactory` | class (2), component | app_user | S:AuthService.cpp, lib/src/security/PasswordHasher.cpp, U:Application.cpp | AuthTest.LoginReturnsPolymorphicUserForEachRole, UsernameIsCaseInsensitiveAndTrimmed, WrongPasswordAndUnknownUserGiveSameMessage, EmptyCredentialsRejected; ST16 |
| FR-02 Role-based access | UC-13 (all) | `User` + 4 subclasses, `Permission`, `RoleMenu` subclasses | class (1), use case | app_user.role | lib/src/domain/User.cpp, Role.cpp, U:RoleMenu.cpp | UT_USR/PermissionMatrix (52), UT_USR.DepartmentScopeIsPolymorphic, AdminTest.OnlyAdministratorsManageAccounts, SubmissionTest.OnlyStudentsSubmit, ModerationTest.OtherDepartmentAndOtherRolesCannotModerate, ST17 |
| FR-03 Manage accounts | UC-08 | `AdminService`, `PasswordPolicy`, `UserSummary` | class (2) | app_user | S:AdminService.cpp, U:AdminMenu.cpp | AdminTest.CreatesUsersOfEveryRole, RejectsInvalidAccounts, CannotDeactivateOwnAccount, ReactivatingHodBlocked..., ListUsersHidesPasswordHashes; AuthTest.InactiveAccountCannotLogIn, AdministratorResetUnlocksAccount; ST21 |
| FR-04 Departments | UC-09 | `AdminService`, `Department` | class (1, 2), ERD | department | S:AdminService.cpp | AdminTest.DepartmentAndCourseNamesAreUnique |
| FR-05 Courses, lecturers | UC-10 | `AdminService`, `Course`, `ICourseRepository` | class, ERD | course, course_lecturer | S:AdminService.cpp, lib/src/data/SqliteRepositories.cpp | AdminTest.LecturerOnlyTeachesOwnDepartment, DepartmentAndCourseNamesAreUnique |
| FR-06 Enrolment | UC-11 | `AdminService` | ERD | enrolment | S:AdminService.cpp | AdminTest.EnrolOnlyActiveStudentsAndIsIdempotent; ST21 |
| FR-07 Create assignment | UC-05 | `AssignmentService`, `Assignment` | class, ERD | assignment | S:AssignmentService.cpp, U:LecturerMenu.cpp | AssignmentTest.LecturerCreatesAssignmentForOwnCourse, OnlyTheCoursesLecturerMayCreate, ValidatesDeadlineMaxMarkAndTitle |
| FR-08 Individual extension | UC-04 | `AssignmentService`, `Extension` | class, state (Late->Submitted), activity | extension | S:AssignmentService.cpp | AssignmentTest.ExtensionMovesThatStudentsDeadlineOnly, RegrantingReplacesTheExtension, ExtensionRules, ExtensionReclassifiesUnmarkedLateSubmission, ExtensionShorterThanLatenessKeepsLateStatus, NoExtensionAfterModeration; ST22 |
| FR-09 View assignments | UC-07 | `AssignmentService::listForStudent`, `StudentAssignmentView` | sequence (submit) | assignment, extension, submission | S:AssignmentService.cpp, U:StudentMenu.cpp | AssignmentTest.StudentsSeeOnlyEnrolledCourses, ExtensionMovesThatStudentsDeadlineOnly; ST03, ST22 |
| FR-10 Submit | UC-01 | `SubmissionService`, `Submission` | sequence (submit), activity, state | submission, submission_version | S:SubmissionService.cpp, U:StudentMenu.cpp | SubmissionTest.MainFlowOnTimeSubmission, ExceptionFlow*; ST01, ST05-ST07, ST25-ST27 |
| FR-11 Resubmit | UC-01 A1 | `SubmissionService`, `SubmissionLifecycle` | sequence alt, state | submission_version | S:SubmissionService.cpp | SubmissionTest.AlternativeFlowResubmitBeforeDeadline, ExceptionFlowResubmitAfterDeadlineKeepsOriginal, LateSubmissionCannotBeResubmitted, ExtensionAllowsSubmitAndResubmit...; ST02, ST04 |
| FR-12 Late accepted | UC-01 A2 | `SubmissionService` | sequence alt, state | submission.status | S:SubmissionService.cpp | SubmissionTest.AlternativeFlowLateSubmissionIsAcceptedNotRejected, LateWorkIsAcceptedEvenWeeksAfterDeadline, SubmissionExactlyAtDeadlineIsOnTime; ST03 |
| FR-13 View released results | UC-06 | `SubmissionService::resultsFor`, `StudentResultView`, `SubmissionLifecycle::isVisibleToStudent` | state | submission | S:SubmissionService.cpp, U:StudentMenu.cpp | SubmissionTest.MarkIsHiddenUntilReleased, StudentsNeverSeeEachOthersWork; ST11, ST12 |
| FR-14 Mark with penalty | UC-02 | `MarkingService`, `ILatePenaltyPolicy`, `PercentPerDayPenaltyPolicy`, `LatenessCalculator` | class (1, 2), state, activity | submission | S:MarkingService.cpp, lib/src/policy/LatePenaltyPolicy.cpp, U:LecturerMenu.cpp | UT_LAT (12), UT_PEN (20), IT_MRK/LateMarking (6), MarkingTest.*, IT_MRK.PenaltyStrategyIsInjectable; ST08-ST10 |
| FR-15 Audit log | UC-02, UC-03 | `MarkAuditEntry`, `IAuditRepository` | class, ERD, sequences | mark_audit (+ triggers) | S:MarkingService.cpp, S:ModerationService.cpp, sql/001_initial_schema.sql | MarkingTest.OnTimeWorkKeepsRawMarkAndIsAudited, RemarkRequiresReasonAndIsAudited, ModerationTest.AuditTrailRecordsWhoWhenOldNewWhy, IT_DB.AuditLogIsAppendOnly |
| FR-16 Moderate | UC-03 | `ModerationService`, `ModerationItem` | sequence (moderate), state, activity | submission, mark_audit | S:ModerationService.cpp, U:HodMenu.cpp | ModerationTest.QueueShowsOnlyMarkedWorkOfOwnDepartment, ApproveMovesToModeratedAndAudits, ApproveRequiresMarkedState, AdjustChangesMarkWithMandatoryReason; ST11, ST13, ST14 |
| FR-17 Release | UC-03 | `ModerationService::releaseAssignment`, `ReleaseSummary` | sequence (moderate), state | submission | S:ModerationService.cpp | ModerationTest.ReleasePublishesOnlyModeratedMarks, ReleaseCountsUnmarkedWork, ReleasedMarksAreFinal; ST11, ST15 |
| FR-18 View audit | UC-12 | `ModerationService::auditTrail/auditLog` | use case | mark_audit | S:ModerationService.cpp, U:HodMenu.cpp, U:AdminMenu.cpp | ModerationTest.AuditLogScope, AdministratorCanInspectAnySubmissionTrail; ST13 |
| FR-19 Notifications | UC-15 | `NotificationService` (Observer), `SubmissionEventPublisher` | class (2) | notification | S:NotificationService.cpp, lib/src/events/SubmissionEvents.cpp, U:RoleMenu.cpp | UT_EVT (3), NotificationTest (6); ST11, ST22 |
| FR-20 Change password | UC-14 | `AuthService::changePassword` | - | app_user | S:AuthService.cpp, U:RoleMenu.cpp | AuthTest.ChangePasswordRequiresCurrentPasswordAndPolicy; ST23 |
| FR-21 First run | UC-08 | `AdminService::bootstrapAdministrator`, `Application` | - | app_user | S:AdminService.cpp, U:Application.cpp | AdminTest.BootstrapOnlyAllowedOnce; ST20; AT-01 |

## Non-functional requirements

| NFR | Design element | Verification |
|-----|----------------|--------------|
| NFR-01..03 Performance | indexes in schema; Argon2 parameters; transactions for bulk release | PerformanceTest (5) - `docs/07-testing/reports/performance-results-v1.0.txt` |
| NFR-04 Password storage | `Argon2idPasswordHasher`, CHECK on `password_hash` | HasherTest (5), UT_SEC.ProductionParametersMeetOwaspMinimum, AuthTest.PasswordsAreStoredAsSaltedArgon2idHashes, IT_DB.SchemaRejectsPlainTextPasswords |
| NFR-05 Password policy, lockout | `PasswordPolicy`, `AuthService` | UT_SEC/PasswordPolicyCases, AuthTest.Five*/Four*/Unknown*, ST24 |
| NFR-06 Parameterised SQL | `Statement::bind`, repositories | IT_DB.ParameterisedQueriesNeutraliseSqlInjection; code review C3 |
| NFR-07 Audit integrity | triggers, `ITransactionRunner` | IT_DB.AuditLogIsAppendOnly, IT_DB.TransactionRollsBackOnException |
| NFR-08 Robustness | `LmsError` hierarchy, `RoleMenu::run`, `ConsoleIO` re-prompting | ST04-ST07, ST09, ST10, ST14, ST18, ST19, ST26 |
| NFR-09 Offline, persistence | SQLite file, WAL | IT_DB.DataPersistsAcrossRestarts, AT-01 |
| NFR-10 Usability | numbered menus, manuals | ST17, ST18; AT-06 (pending) |
| NFR-11 Maintainability | layered design, CI gates | coverage 95.9 %, lizard max CCN 14, cppcheck 0 |
| NFR-12 Portability | CMake, `#ifdef _WIN32` isolation | GCC 16 + Clang 21 builds; CI matrix |

## Backward check
No test exists without a requirement: unit tests of helpers (`UT_TXT`, `UT_LIF`) trace to NFR-08 (input validation) and the state machine (BR-05/06/07); `IT_DB` traces to NFR-06/07/09.
