# Software Requirements Specification (SRS)

**System:** Learning Management System (LMS)
**Version:** 1.1 (baseline 1.0 approved at Gate 1; FR-22 added by change request CR1)
**Structure:** adapted from ISO/IEC/IEEE 29148:2018, clause 9.6 (SRS content)

---

## 1. Introduction

### 1.1 Purpose
This SRS specifies the requirements of a Learning Management System for a secondary school in which marks are moderated by the Head of Department before students can see them. It is the baseline for design (Phase 3), testing (Phase 5) and acceptance.

### 1.2 Scope
The LMS is a C++17 console application that runs offline on one school computer and stores its data in a local SQLite database. It supports four roles (Administrator, Lecturer, Head of Department, Student) and covers courses, assignments, submissions, individual extensions, marking with late penalties, moderation, release of marks, audit of mark changes and in-app notifications.

**Out of scope:** online payment, video conferencing, automated grading, plagiarism detection, web or mobile interfaces, e-mail/SMS, networked multi-user access.

### 1.3 Definitions

| Term | Definition |
|------|------------|
| Effective deadline | The latest of the assignment deadline, the class-wide extension (v1.1) and the student's individual extension. |
| Day late | Each started 24-hour period after the effective deadline (BR-02). |
| Raw mark | The lecturer's mark before any late penalty. |
| Final mark | Raw mark minus late deduction, or the value set by the HoD at moderation. |
| Moderation | HoD review of a mark: approve it or adjust it with a reason. |
| Release | Making moderated marks and feedback visible to students. |
| MoSCoW | Must / Should / Could / Won't priority. |

### 1.4 References
- CS4433 Final Project brief, NUL, 2026.
- ISO/IEC/IEEE 29148:2018 Systems and software engineering - Requirements engineering.
- OWASP Password Storage Cheat Sheet (Argon2id parameters).
- RFC 9106 Argon2 Memory-Hard Function for Password Hashing.
- Lesotho Data Protection Act 2011.

## 2. Overall description

### 2.1 Product perspective
A stand-alone system. The internal architecture is layered (presentation, business logic, data access); see the Software Design Document.

### 2.2 User classes

| Class | Frequency | Technical skill | Key functions |
|-------|-----------|-----------------|---------------|
| Administrator | Weekly, start of term | Moderate | Accounts, departments, courses, enrolment |
| Lecturer | Several times a week | Low ("not IT experts") | Assignments, extensions, marking |
| Head of Department | Weekly | Low | Moderation, release, audit |
| Student | Several times a week | Mixed | View, submit, resubmit, results |

### 2.3 Operating environment
Windows 10/11 x64 or Ubuntu 22.04+, dual-core CPU, 2 GB RAM, 100 MB disk, no network.

### 2.4 Constraints
- CON-1 C++17, CMake, SQLite, Git (brief).
- CON-2 Waterfall process with V-Model verification; implementation only after Gate 2.
- CON-3 Console (text) user interface.
- CON-4 Passwords must never be stored in plain text; a recognised hashing library must be used.

### 2.5 Assumptions and dependencies
- AS-1 One person uses the computer at a time and logs out afterwards.
- AS-2 The system clock of the school computer is correct (deadlines depend on it).
- AS-3 The technician copies the database file to backup media daily.

## 3. Specific requirements

Priority uses MoSCoW. Each requirement is verified by the method shown (T = test, I = inspection, D = demonstration, A = analysis). Test identifiers refer to `docs/07-testing/test-plan.md` and the automated suites.

### 3.1 Functional requirements

| ID | Requirement | Priority | Source | Verification |
|----|-------------|----------|--------|--------------|
| FR-01 | The system shall authenticate every user with a username and password before any other function is available. | Must | Brief | T: AuthTest.*, ST16 |
| FR-02 | The system shall allow each user to perform only the actions permitted for their role, as defined in the access-control matrix (3.2). | Must | Brief | T: UT_USR.PermissionMatrix (52 cases), ST17, *OnlyAdministrators*, *OtherDepartment* tests |
| FR-03 | The administrator shall be able to create user accounts of any role, activate and deactivate accounts, and reset a user's password (which also unlocks the account). | Must | Brief, IT officer | T: AdminTest.*, AuthTest.AdministratorResetUnlocksAccount, ST21 |
| FR-04 | The administrator shall be able to create and list departments; department names are unique (case-insensitive). | Must | Brief | T: AdminTest.DepartmentAndCourseNamesAreUnique |
| FR-05 | The administrator shall be able to create courses in a department and assign lecturers of that department to them. | Must | Brief | T: AdminTest.LecturerOnlyTeachesOwnDepartment |
| FR-06 | The administrator shall be able to enrol active students on courses. | Must | Brief | T: AdminTest.EnrolOnlyActiveStudentsAndIsIdempotent, ST21 |
| FR-07 | A lecturer shall be able to create an assignment for a course they teach, with title, optional description, a future deadline and a maximum mark of 1-1000. | Must | Brief | T: AssignmentTest.* |
| FR-08 | A lecturer shall be able to grant one student an individual extension to a later deadline, with a mandatory reason. | Must | Lecturer | T: AssignmentTest.Extension*, ST22 |
| FR-09 | A student shall be able to list the courses they are enrolled on and their assignments, showing their own effective deadline, time remaining, maximum mark and submission status. | Must | Brief | T: AssignmentTest.StudentsSeeOnlyEnrolledCourses, ST03, ST22 |
| FR-10 | A student shall be able to submit work (typed text or a file) for an assignment of a course they are enrolled on. | Must | Brief | T: SubmissionTest.MainFlowOnTimeSubmission, ST01, ST25 |
| FR-11 | A student shall be able to resubmit up to their effective deadline while the work is unmarked; the new version replaces the current one and all versions are retained. | Must | Student rep | T: SubmissionTest.AlternativeFlowResubmitBeforeDeadline, ST02 |
| FR-12 | The system shall accept a submission made after the effective deadline and record it as LATE. | Must | Student rep, HoD | T: SubmissionTest.AlternativeFlowLateSubmissionIsAcceptedNotRejected, ST03 |
| FR-13 | A student shall be able to view the final mark, raw mark, late deduction and written feedback of released submissions only. | Must | Principal, Student rep | T: SubmissionTest.MarkIsHiddenUntilReleased, ST11, ST12 |
| FR-14 | A lecturer shall be able to mark a submission of a course they teach by entering a raw mark and written feedback; the system shall apply the late-penalty rule and store raw mark, days late, deduction and final mark. | Must | Brief, HoD | T: MarkingTest.*, IT_MRK.LateMarking, ST08 |
| FR-15 | The system shall record every creation or change of a mark (marking, re-marking, moderation, release) in an audit log with who, when, old value, new value and reason. | Must | Brief | T: ModerationTest.AuditTrailRecordsWhoWhenOldNewWhy, MarkingTest.RemarkRequiresReasonAndIsAudited |
| FR-16 | A HoD shall be able to list the marked submissions of their department awaiting moderation and approve each mark or adjust it with a reason. | Must | Principal, HoD | T: ModerationTest.*, ST11, ST13, ST14 |
| FR-17 | A HoD shall be able to release all moderated marks of an assignment of their department; unmoderated marks stay hidden. | Must | Principal, HoD | T: ModerationTest.ReleasePublishesOnlyModeratedMarks, ST11, ST15 |
| FR-18 | A HoD shall be able to view the audit trail of a submission and of their department; the administrator shall be able to view the whole audit log. | Should | Brief | T: ModerationTest.AuditLogScope, ModerationTest.AdministratorCanInspectAnySubmissionTrail, ST13 |
| FR-19 | The system shall show in-app notifications at login: lecturers on submissions, HoDs on marks awaiting moderation, students on extensions and released marks. | Could | Lecturer (speed) | T: NotificationTest.* |
| FR-20 | Every user shall be able to change their own password after re-entering the current one. | Should | IT officer | T: AuthTest.ChangePasswordRequiresCurrentPasswordAndPolicy, ST23 |
| FR-21 | On first start with an empty database the system shall require the creation of an administrator account. | Should | Operability | T: ST20, AdminTest.BootstrapOnlyAllowedOnce |
| FR-22 | *(v1.1, CR1)* A lecturer shall be able to extend the deadline of an assignment for the whole class with a reason, and the student's assignment list shall show the original deadline, the student's effective deadline and whether an individual or class extension applies. | Must | CR1 | T: ClassExtensionTest.*, ST28-ST29 (see docs/10-maintenance) |
| FR-W1 | Released marks can be corrected after release. | Won't (v1.x) | A11 | - |
| FR-W2 | E-mail or SMS notifications. | Won't (offline) | NFR-09 | - |

### 3.2 Access-control matrix (FR-02)

| Permission | Admin | Lecturer | HoD | Student |
|------------|:-----:|:--------:|:---:|:-------:|
| Manage users / departments / courses, assign lecturers, enrol students | X | | | |
| Create assignment, grant extension, mark submission (own courses only) | | X | | |
| Moderate and release marks (own department only) | | | X | |
| View audit log | X (all) | | X (own dept) | |
| Submit work, view released results (own only) | | | | X |
| Log in, change own password, view own notifications | X | X | X | X |

### 3.3 Business rules

| ID | Rule |
|----|------|
| BR-01 | Late penalty: 10 % of the maximum mark is deducted for each day late, for days 1 to 3. |
| BR-02 | A day late is each started 24-hour period after the effective deadline. Work more than 3 days late receives a final mark of 0. The deduction never exceeds the raw mark. |
| BR-03 | Effective deadline = the latest of the assignment deadline, the class-wide extension (v1.1) and the student's individual extension. Lateness is evaluated at marking time against the effective deadline. An unmarked LATE submission covered by a new extension becomes SUBMITTED. |
| BR-04 | Late work is never rejected. |
| BR-05 | Resubmission is allowed only while the work is unmarked and the current time is not after the effective deadline. All versions are kept. |
| BR-06 | Released marks are final. Lecturers cannot change a mark once it is moderated. |
| BR-07 | Students cannot see any mark or feedback, or receive any notification about it, until the HoD releases it. |
| BR-08 | A HoD moderates and releases only work from courses of their own department. |
| BR-09 | Re-marking, moderation adjustments and extensions require a written reason. |
| BR-10 | Written feedback is mandatory when marking. |
| BR-11 | Lecturers create assignments, grant extensions and mark only for courses they are assigned to. Lecturers are assigned only to courses of their own department. |
| BR-12 | A department has at most one active Head of Department. |

### 3.4 Non-functional requirements

Reference hardware for performance: the development PC (x64, Windows 11); the CI runner repeats the measurements on every push.

| ID | Category | Requirement (measurable) | Priority | Verification |
|----|----------|--------------------------|----------|--------------|
| NFR-01 | Performance | Login (including password verification) completes within **2 s** with 800 student accounts stored, using production Argon2id parameters. | Must | T: PerformanceTest.NFR01 (measured 153 ms, see performance-results-v1.0.txt) |
| NFR-02 | Performance | Listing a student's assignments and submitting work each complete within **1 s** with 800 students and 20 assignments stored. | Must | T: PerformanceTest.NFR02_* (measured 2.6 ms / 1.9 ms) |
| NFR-03 | Performance | The HoD moderation queue with **800** marked submissions loads within **2 s**, and releasing 800 marks completes within **5 s**. | Should | T: PerformanceTest.NFR03_* (108 ms / 338 ms) |
| NFR-04 | Security | Passwords are stored only as salted **Argon2id** hashes with memory >= 19 MiB, iterations >= 2 and a 16-byte random salt from the OS CSPRNG; the schema rejects any value that is not an Argon2id hash. | Must | T: UT_SEC.*, HasherTest.*, IT_DB.SchemaRejectsPlainTextPasswords; I: code review |
| NFR-05 | Security | Passwords have 8-128 characters with at least one letter and one digit and differ from the username; an account is locked for **15 min after 5** consecutive failed logins; login errors do not reveal whether a username exists (same message, same hashing work). | Must | T: PasswordPolicyCases, AuthTest.Five*, AuthTest.Unknown*, ST24 |
| NFR-06 | Security | **100 %** of SQL statements that include user data use prepared statements with bound parameters (0 string-concatenated values). | Must | I: grep/code review of lib/src/data; T: IT_DB.ParameterisedQueriesNeutraliseSqlInjection |
| NFR-07 | Integrity | Every mark change produces exactly one audit row in the same transaction; audit rows cannot be updated or deleted (database triggers). | Must | T: IT_DB.AuditLogIsAppendOnly, ModerationTest.AuditTrail* |
| NFR-08 | Robustness | No invalid input or database error terminates the program: 100 % of exception-flow system tests end with an error message and the session continuing (or a clean exit on end of input). | Must | T: ST04-ST07, ST09, ST10, ST14, ST18, ST19, ST26 |
| NFR-09 | Availability | All functions work with no network connection; committed data survives a restart of the program. | Must | T: IT_DB.DataPersistsAcrossRestarts; D: AT-01 with network disabled |
| NFR-10 | Usability / accessibility | Plain-text numbered menus, no information conveyed by colour alone, every function reachable within 2 menu levels; a first-time student completes "submit assignment" in <= 2 minutes using the student manual. | Should | T: ST17; D: AT-06 |
| NFR-11 | Maintainability | Business-logic layer line coverage >= **70 %**; no function with cyclomatic complexity > **15**; zero open cppcheck warnings. | Should | A: gcovr (95.9 %), lizard (max CCN 14), cppcheck report |
| NFR-12 | Portability | Builds with CMake >= 3.20 and a C++17 compiler on Windows and Ubuntu with no source changes. | Should | T: CI matrix (ubuntu-latest, windows-latest); local GCC 16 and Clang 21 builds |

## 4. Verification summary
Every FR and NFR above names at least one verification activity. The complete mapping FR -> use case -> design -> source -> test is in `docs/appendices/traceability-matrix.md`.

## 5. Approval (Gate 1)

| Role | Name | Signature / date |
|------|------|------------------|
| Author | [student name] | |
| Supervisor | | |
