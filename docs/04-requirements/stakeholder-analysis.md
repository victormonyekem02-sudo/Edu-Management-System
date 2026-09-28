# Stakeholder Analysis and Conflict Resolution

Project: Moderated Learning Management System (CS4433 final project)
Phase: 2 - Requirements engineering (input to Gate 1)

## 1. Stakeholders and their interests

| ID | Stakeholder | Type | Main interests | Influence | Interest | Engagement strategy |
|----|-------------|------|----------------|-----------|----------|---------------------|
| SH-1 | Principal | Sponsor / decision maker | Fair, defensible assessment; simplicity for non-IT staff; no premature release of marks | High | Medium | Keep satisfied: approves SRS at Gate 1, final arbiter of policy conflicts |
| SH-2 | Lecturers | Primary user | Fast marking; ability to grant extensions; quick return of marks to students | Medium | High | Manage closely: walkthrough of the marking and extension use cases |
| SH-3 | Heads of Department (HoD) | Primary user | Consistent late penalties; see every mark before release; accountability | High | High | Manage closely: owner of the moderation and penalty rules |
| SH-4 | Students (via student representative) | Primary user | Resubmission before the deadline; written feedback; late work not rejected; fairness | Low | High | Keep informed: validate the student manual and the late-work rule |
| SH-5 | IT officer / technician | Operator | Offline operation on old computers; protected passwords; speed with 800 students; easy installation and backup | Medium | Medium | Keep satisfied: owns NFRs, installation guide and backup procedure |
| SH-6 | System administrator | Primary user | Manage accounts, departments, courses and enrolment with little effort | Medium | Medium | Keep informed: admin manual and first-run set-up |
| SH-7 | Parents / guardians | Indirect | Privacy of their children's data; fairness of marks | Low | Medium | Monitor: addressed through data-protection design (see professional issues) |
| SH-8 | Ministry of Education / examinations regulator | Regulatory | Integrity and traceability of assessment records | High | Low | Keep satisfied: audit trail requirements (FR-15, NFR-07) |
| SH-9 | Course supervisor (CS4433) | Project governance | Phase-gate deliverables, traceability, working software | High | High | Formal reviews at Gates 1-3 |

## 2. Conflicts and ambiguities found in the stakeholder statements

### 2.1 Conflicts

| ID | Conflict | Stakeholders | Resolution | Resulting requirements |
|----|----------|--------------|------------|------------------------|
| C1 | Principal: "Students must not see any mark until it has been checked by the HoD." Lecturer: "I want students to get their marks as soon as I have marked the work." | Principal vs Lecturer | **Principal's rule adopted.** The principal is the sponsor and the HoD independently asks for the same control, and moderation protects fairness and the school's reputation. The lecturer's underlying goal, *speed*, is met instead by: an immediate notification to the HoD when a mark awaits moderation; a moderation queue that loads in under 2 s even with 800 marked scripts (NFR-03); and release of a whole assignment in one action. The lecturer sees the status "awaiting moderation" so the delay is visible, not silent. | FR-14, FR-16, FR-17, FR-19, BR-07, NFR-03 |
| C2 | HoD: "Late work... after [three days] should receive zero." Student representative: "Late work should not simply be rejected." | HoD vs Students | **Both satisfied by separating *acceptance* from *scoring*.** Late work is always accepted, stored and marked with written feedback, even after three days; the penalty only changes the final mark. A zero with feedback still gives the learner formative value and a record that the work existed. | FR-12, FR-14, BR-01, BR-02, BR-04, BR-10 |
| C3 | Lecturer: "I should be able to give [students] an extension." HoD: every late day loses 10 %. | Lecturer vs HoD | **Extensions move the deadline that the penalty is measured from.** An extension is individual, must be later than the original deadline, requires a written reason and is stored so the HoD can see it. Penalties are computed at marking time against the *effective* deadline, so an extension granted after a late submission is honoured. Extensions are blocked once a mark has been moderated, so they cannot be used to undo a moderated decision. | FR-08, BR-03, BR-09 |
| C4 | Student representative: "fix a mistake and resubmit before the deadline" vs assessment integrity (a student could resubmit after seeing others' marks or after the deadline). | Students vs HoD / Principal | Resubmission is allowed any number of times **up to the student's effective deadline and only before marking**. After the deadline, a student who has not yet submitted may submit once (late); a student who already submitted cannot replace the work. Every version is kept. | FR-11, BR-05 |
| C5 | Principal: "the system must be simple" vs IT officer: "Passwords must be protected" (strong policies, lockouts and hashing can make systems harder to use). | Principal vs IT officer | Security is made mostly invisible: Argon2id hashing costs the user nothing; the password rule is short (8+ characters, a letter and a digit); lockout only happens after 5 consecutive failures and the administrator can unlock with one menu option. The UI uses numbered plain-text menus. | NFR-04, NFR-05, NFR-10, FR-03 |
| C6 | IT officer: "computers are old" and "respond quickly even with all 800 students" vs strong password hashing (memory-hard hashing is slow by design). | IT officer (internal tension) | Argon2id at the **OWASP minimum** parameters (19 MiB, 2 iterations, 1 lane) instead of the library default (64 MiB+): the hash remains memory-hard but a login measured 56-153 ms on the development machine, far below the 2 s limit. Parameters are stored inside each hash, so they can be raised later without resetting passwords. | NFR-01, NFR-04 |
| C7 | IT officer: "must work offline", brief: "single school computer or local network" vs several users at once. | IT officer vs brief | v1.x targets **one computer** (users log in one after another) with a local SQLite file. Running the database on a network share is explicitly not supported because SQLite file locking over SMB is unreliable. A client/server version is recorded as future work. | NFR-09, deployment diagram |

### 2.2 Ambiguities

| ID | Ambiguous statement | Question raised | Interpretation adopted (confirmed at Gate 1) |
|----|---------------------|-----------------|---------------------------------------------|
| A1 | "lose 10 % of the available mark" | 10 % of the student's mark or of the maximum? | Of the **maximum** mark ("available"), e.g. 2.5 of 25 per day. |
| A2 | "for each day it is late" | What is a day? Calendar day or 24 h? Partial days? | Each **started 24-hour period** after the effective deadline counts as one day: 1 s late = 1 day, exactly 24 h = 1 day, 24 h + 1 s = 2 days. Independent of time zones and midnights. |
| A3 | "for up to three days. After that it should receive zero" | Is day 3 penalised 30 % or zero? | Days 1-3 lose 10/20/30 %; **day 4 onwards scores 0**. |
| A4 | Can the penalty make the mark negative? | - | No. The deduction is capped at the raw mark. |
| A5 | "checked by the Head of Department" | Is checking the same as releasing? | Two steps: **moderation** (approve or adjust with a reason) and **release** (publish all moderated marks of an assignment). |
| A6 | "resubmit before the deadline" | Which deadline if the student has an extension? | The student's **effective deadline** (original or extended). |
| A7 | "all 800 students on it" | 800 concurrent users or 800 stored? | 800 students **stored** in the database; on a single computer users work one at a time. Performance NFRs are measured with 800 students, 20 assignments and 800 marked submissions. |
| A8 | "work offline" | Offline sometimes, or never online? | **No network is ever required** for any function. |
| A9 | "Passwords must be protected" | In storage, in transit, on screen? | Salted Argon2id hashes at rest; no echo when typing; generic login errors; lockout after 5 failures. No network transit exists. |
| A10 | "written feedback, not just a number" | Optional or mandatory? | **Mandatory**: a mark cannot be saved without feedback. |
| A11 | Can a released mark be changed? | - | Not in v1.x: released marks are final; corrections would be a future change request with its own approval. |
| A12 | Who can see the audit trail? | - | HoD for their own department; Administrator for the whole school. Lecturers and students cannot. |

## 3. Traceability of stakeholder concerns

| Stakeholder concern | Requirement(s) | Verified by |
|---------------------|----------------|-------------|
| Principal - simplicity | NFR-10 | AT-06, ST17, ST18 |
| Principal - no mark before HoD check | BR-07, FR-13, FR-17 | ST12, SubmissionTest.MarkIsHiddenUntilReleased, NotificationTest.StudentLearnsNothingBeforeRelease |
| Lecturer - extensions | FR-08 | AssignmentTest.*, ST22 |
| Lecturer - speed of return | FR-19, NFR-03 | NotificationTest.HeadOfDepartmentIsToldWhenMarkAwaitsModeration, PT NFR03 |
| HoD - penalty rule | BR-01, BR-02 | UT_LAT, UT_PEN, IT_MRK.LateMarking |
| HoD - see every mark | FR-16 | ModerationTest.QueueShowsOnlyMarkedWorkOfOwnDepartment, ST11 |
| Student rep - resubmit | FR-11 | SubmissionTest.AlternativeFlowResubmitBeforeDeadline, ST02 |
| Student rep - written feedback | BR-10 | MarkingTest.ValidatesMarkRangeAndFeedback |
| Student rep - late work accepted | BR-04 | SubmissionTest.LateWorkIsAcceptedEvenWeeksAfterDeadline, ST03 |
| IT officer - offline | NFR-09 | IT_DB.DataPersistsAcrossRestarts, deployment test in AT-01 |
| IT officer - passwords | NFR-04, NFR-05 | HasherTest.*, AuthTest.*, IT_DB.SchemaRejectsPlainTextPasswords |
| IT officer - speed with 800 students | NFR-01..03 | PerformanceTest.* |
