# Professional, Legal and Ethical Issues

## 1. Data protection of learners' personal information

**Legal framework.** In Lesotho, personal data is governed by the **Data Protection Act 2011 (Act No. 5 of 2011)**, which applies to public and private bodies processing personal information. It requires that personal information is processed lawfully and fairly, for a specific, explicitly defined purpose; is adequate, relevant and not excessive; is accurate; is not kept longer than necessary; is protected by appropriate technical and organisational safeguards; and that data subjects can access and correct their information. Learners are mostly minors, so their parents or guardians exercise these rights on their behalf, and the school, as **data controller**, carries the duty of care. Where a comparable standard is useful, the design also follows the principles of the EU GDPR (Article 5 principles, Article 25 data protection by design, Article 32 security of processing) and the ACM Code of Ethics (1.6 "Respect privacy").

| Principle | How the system addresses it | Remaining responsibility of the school |
|-----------|----------------------------|----------------------------------------|
| Purpose limitation and minimisation | Stores only what assessment needs: name, username, role, department, enrolments, submitted work, marks, feedback. No ID numbers, addresses, birth dates or contact details. | Do not use the LMS for other purposes (e.g. discipline records). |
| Security (confidentiality) | Role-based access checked in every service call; students see only their own work; HoDs only their department; salted Argon2id password hashes; lockout; no echo; generic login errors; data never leaves the computer (offline, no cross-border transfer). | Physically secure the PC; restrict Windows access to the data folder; encrypt backup drives (e.g. BitLocker To Go). |
| Integrity and accuracy | Transactions; state machine; append-only audit of every mark change with reasons; resubmission history kept. | Correct errors through the defined process (HoD adjustment before release). |
| Retention | All records are kept (deactivation, not deletion) so assessment decisions can be checked. | Define a retention period (e.g. current year + 2 years for appeals) and archive or remove data of learners who have left. A "purge learner" function is future work. |
| Access by data subjects | Students can see all their released results and feedback. | Provide printouts on request for parents; handle correction requests through the HoD. |
| Transparency | The rules (penalty, moderation, release) are stated in the student manual and at submission time. | Inform learners and parents that the system is used and what it stores. |

**Known limitations, stated openly.** The SQLite file itself is not encrypted, so anyone who copies `lms.db` can read names and marks (though not passwords). This is mitigated by folder permissions, and the `@file` submission is blocked from reading the database (DEF-05). Passwords are held in ordinary memory for the moment of login. Both would be addressed by SQLCipher and secure buffers in a future version; they were judged proportionate for a single offline school PC.

## 2. Security of assessment records

Assessment records have legal and personal consequences: progression, certificates and reputations. The design treats them as evidence.
- **Who can change a mark** is limited by role and scope (BR-08, BR-11), tested with 52 permission cases and scope tests.
- **Traceability:** each change creates an audit row with who, when, old value, new value and reason, in the same transaction as the change. Database triggers make the table append-only, so even a user with a SQLite tool cannot quietly change history through normal SQL (they would have to drop the triggers, which is itself visible).
- **Separation of duties:** the lecturer marks, the HoD moderates and releases, and neither can do the other's step. After moderation the lecturer cannot change the mark; after release nobody can.
- **Availability:** WAL journaling protects against a crash mid-write; the installation guide defines daily backups and a restore procedure.
- **Professional honesty in reporting:** the test report states what was *not* verified (AT-06 usability, CI on GitHub not yet run), in line with the ACM/IEEE Software Engineering Code of Ethics principle 3.10 (ensure adequate testing) and 6.07 (be accurate about the characteristics of software).

## 3. Accessibility for users with disabilities

| Need | Design response | Limitation / recommendation |
|------|-----------------|-----------------------------|
| Blind / low-vision users (screen readers) | Pure text console; no colour, icons or layout-only meaning; status words such as `[OK]`, `[ERROR]`, "Submitted LATE" are spoken by NVDA/Narrator; tables are simple columns | Long tables are tedious to hear; a future "read one row at a time" mode would help |
| Low vision | Font size and colours are controlled by the console window (Properties > Font), so users can enlarge text | - |
| Motor impairments | Keyboard only, no timing pressure in the UI (no auto-logout while typing), numbered options | Very long typed answers are better prepared in a word processor and submitted with `@file` |
| Cognitive load / non-IT experts | Numbered menus of at most 11 items per role, one task per option, plain-language errors that say what to do, confirmation before releasing marks | AT-06 must be run with real users to confirm |
| Learners who need extra time (e.g. dyslexia, medical conditions) | Individual extensions with a recorded reason, which do not require telling classmates | The school's accommodation policy decides who is eligible |

The approach follows the principles of WCAG 2.2 (perceivable, operable, understandable, robust) as far as they apply to a text interface.

## 4. Fairness of the late-penalty rules

The 10 %-per-day rule is the HoD's policy; the system's job is to apply it **consistently, transparently and without hidden harshness**.
- **Consistency:** one implementation (`PercentPerDayPenaltyPolicy`) is used for every submission, with 29 automated test cases around its boundaries. Manual calculation, which is error-prone and can vary between lecturers, is removed.
- **Precision about "a day":** a started 24-hour period is independent of time zones and midnight. It does have a harsh edge: 24 h + 1 s counts as 2 days. The alternative (rounding to the nearest day) was rejected because it rewards lateness up to 12 h for free. The rule is published in the student manual with a worked table, and shown when a late submission is made, so no one is penalised by a rule they could not know.
- **Penalty proportional to the maximum, capped at the earned mark:** a weak submission cannot go negative, and the deduction is the same for everyone at the same lateness.
- **Zero after 3 days, but still marked with feedback:** respects the student representative's point that late work should not be rejected. The learner still receives formative feedback, and the record shows the work was done.
- **Human discretion is preserved and recorded:** extensions (individual) for good reasons and HoD adjustments exist, but each needs a written reason and leaves an audit trail. This allows compassion and fairness in individual circumstances (illness, bereavement, power cuts) without favouritism going unrecorded.
- **Equity concerns:** in a context of unreliable electricity and shared computers, lateness is not always the learner's fault. The system accepts late work at any time and makes extensions easy. It also moves the lateness decision to marking time, so an extension granted afterwards (e.g. after a power cut is confirmed) removes the penalty automatically.
- **Changeability:** because the rule is a Strategy, the school can adopt a gentler policy (e.g. 5 %, or a grace period) without rewriting the system (`NoLatePenaltyPolicy` already exists).

## 5. Professional responsibilities of the developer
- **Competence and honesty:** limitations and unverified items are recorded (test report section 4, risk register), not hidden.
- **Intellectual property and licences:** all third-party components are used under their licences (SQLite public domain, Argon2 CC0/Apache-2.0, GoogleTest BSD-3), are pinned and attributed, and none is copied into the repository.
- **Use of AI assistance:** where AI tools were used in producing code or documents, the student is responsible for understanding, verifying and being able to explain every part at the viva, as the brief requires. Commits made with assistance carry a co-author trailer.
- **Public interest:** the system handles minors' data and grades, so the design errs on the side of privacy (minimal data, offline) and accountability (audit, separation of duties).
