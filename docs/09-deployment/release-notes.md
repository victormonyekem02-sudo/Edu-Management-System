# Release Notes

## Version 1.1.0 - 28 September 2026 (change request CR1)

### New
- **Class-wide extensions:** lecturers can extend an assignment's deadline for the whole class (Lecturer menu option 6), with a reason. Students are notified.
- **Extensions on the student's assignment list:** new columns *Original deadline* and *Extension* (`class` / `individual` / `-`).
- A student's deadline is the latest of the original deadline, the class extension and their individual extension. Late status, late penalties and resubmission all use it.

### Changed
- Lecturer menu: "View extensions" is now option 7 (and shows the class extension); notifications 8; change password 9.

### Upgrade
- Back up `lms.db`, install over the old version, start the program: the database is upgraded automatically (schema version 2). No data is changed.

### Quality
- 278 functional tests (14 new) and 5 performance tests pass; all 264 v1.0 tests still pass (one display expectation updated); business-logic line coverage 96.1 %.

---

## Version 1.0.0 - 28 September 2026

First release of the Moderated Learning Management System.

### Features
- Login for four roles (Administrator, Lecturer, Head of Department, Student), each with its own menu and permissions.
- **Administrator:** user accounts (create, activate/deactivate, reset password and unlock), departments, courses, lecturer assignment, student enrolment, school-wide mark audit log. First-run set-up of the administrator account.
- **Lecturer:** create assignments with a deadline and maximum mark, grant individual extensions with a reason, view submissions, mark with mandatory written feedback, re-mark with a reason.
- **Head of Department:** moderation queue, approve or adjust marks with a reason, release moderated marks per assignment, audit trail per submission and per department.
- **Student:** view courses and assignments with personal deadlines, submit typed work or a file, resubmit before the deadline, view released marks and feedback.
- Late penalty: 10 % of the maximum mark per started day late, up to 3 days; zero after that. Late work is always accepted and marked.
- Students see nothing about a mark until the Head of Department releases it.
- Every mark change is recorded (who, when, old mark, new mark, reason); the audit log cannot be edited.
- In-app notifications at login.
- Works completely offline; data in one SQLite file.

### Security
- Passwords stored as salted Argon2id hashes; lockout for 15 minutes after 5 failed logins; passwords are not shown while typing.

### Quality
- 264 automated functional tests and 5 performance tests pass; business-logic line coverage 95.9 %; no cppcheck or clang-tidy findings.

### Known limitations
- One computer, one user at a time (no network or multi-computer use).
- Released marks cannot be corrected in the system.
- Usability acceptance test AT-06 with a real student has not been run yet.

### Installation
See `doc/installation-guide.md`. New installation: create the administrator at first start.
