# Change Request CR1 - Class-wide extensions

| Field | Value |
|-------|-------|
| Request | "A lecturer should be able to extend the deadline for a whole class, not just an individual student, and extensions should be shown on the student's assignment list." |
| Requested by | School (after release v1.0) |
| Received | 28 Sep 2026, against release v1.0 |
| Decision | Approved for release v1.1 |
| Branch | `feature/cr1-class-extension` |

## 1. Clarification
| Question | Answer adopted |
|----------|----------------|
| What is "a whole class"? | All students enrolled on the course of the assignment. |
| One class extension or several? | One per assignment; granting again replaces it (the latest decision counts), like individual extensions. |
| Individual + class extension together? | The student's effective deadline is the **latest** of: original deadline, class extension, individual extension. An individual extension can never shorten a class extension, and vice versa. |
| "Shown on the student's assignment list" | The list shows the original deadline, the student's own deadline and which kind of extension applies (*individual* / *class*). v1.0 only showed the effective deadline with an "(extended)" marker and did not say which kind applied or what the original deadline was. |
| After moderation? | Refused if any submission of the assignment is already moderated or released (same rule as individual extensions, BR-03 / BR-06). |

## 2. Classification: **perfective maintenance**
- **Not corrective:** v1.0 meets its specification (FR-08 asked for *individual* extensions only; all tests pass). No defect is being fixed.
- **Not adaptive:** nothing in the environment changed (same OS, hardware, database, law).
- **Not preventive:** the aim is not to make future maintenance easier or to remove latent faults.
- **Perfective:** users ask for **new functionality** (a class-wide extension) and an **improved presentation** of existing information (extension details on the list) after using the released system. This is an enhancement to meet evolving user requirements, which is the definition of perfective maintenance (Lientz and Swanson; ISO/IEC 14764).

A small **adaptive** element is part of the change: existing v1.0 databases must be upgraded in place (schema migration 002) without losing data.

## 3. Impact analysis

### 3.1 Requirements
| Item | Impact |
|------|--------|
| FR-08 Individual extension | Unchanged behaviour; interaction with the class extension defined (latest wins) |
| FR-09 View assignments | **Changed**: shows original deadline, own deadline, extension kind |
| **FR-22** (new) | Class-wide extension with reason; shown on student list |
| FR-19 Notifications | **Changed**: all enrolled students are notified of a class extension |
| BR-03 Effective deadline | **Changed**: latest of deadline, class extension, individual extension |
| FR-12, FR-14 (late status, penalty) | Indirect: both use the effective deadline, so class extensions automatically affect lateness and penalties. No code change in `SubmissionService`/`MarkingService` |
| NFR-02 | Re-verified: one extra indexed look-up per assignment in the list |

### 3.2 Use cases and diagrams
| Artefact | Impact |
|----------|--------|
| Use case diagram | New **UC-16 Grant class-wide extension** (Lecturer), «include» *Determine effective deadline* |
| UC-07 View assignments | Output changed |
| UC-01 / UC-02 specifications | Text of "effective deadline" updated (class extension) |
| Class diagram views 1-3 | New `ClassExtension` struct, `ExtensionKind` enum, `DeadlineInfo`; new methods on `AssignmentService` and `IAssignmentRepository`/`SqliteAssignmentRepository`; new event type |
| State machine | Guard on LATE -> SUBMITTED now also triggered by a class extension |
| Activity diagram | Extension step covers "individual or class" |
| Sequence diagram "Submit assignment" | `effectiveDeadline` also reads the class extension |
| ERD / data dictionary / schema | New table **`class_extension`** (migration `sql/002_class_extension.sql`) |
| Component and deployment diagrams | No change |

### 3.3 Classes and source files
| File | Change |
|------|--------|
| `lib/include/lms/domain/Entities.h` | + `ClassExtension` |
| `lib/include/lms/data/Repositories.h`, `SqliteRepositories.h/.cpp` | + `saveClassExtension`, `findClassExtension` |
| `lib/include/lms/events/SubmissionEvents.h` | + `SubmissionEventType::ClassExtensionGranted` |
| `lib/include/lms/services/AssignmentService.h/.cpp` | + `grantClassExtension`, `classExtension`, `deadlineFor`, `ExtensionKind`; `effectiveDeadline` and `listForStudent` extended |
| `lib/src/services/NotificationService.cpp` | Notify all enrolled students |
| `app/src/LecturerMenu.cpp`, `Menus.h` | New menu option 6 "Extend the deadline for the whole class"; extensions list shows the class extension (later options move down by one) |
| `app/src/StudentMenu.cpp` | New columns *Original deadline* and *Extension* |
| `sql/002_class_extension.sql` | New migration |
| `CMakeLists.txt` | Version 1.1.0 |

### 3.4 Tables
New `class_extension` (PK and FK `assignment_id`). No existing table changes, so v1.0 data is untouched and migration is additive.

### 3.5 Test cases
| Existing tests affected | Why | Action |
|-------------------------|-----|--------|
| ST22 (lecturer grants extension, student sees it) | List format changed ("(extended)" replaced by an Extension column) | Updated expectation |
| AT-05 | Same | Updated expectation |
| All other 263 tests | Regression - behaviour must not change | Re-run unchanged |
| IT_DB.MigrationCreatesLatestSchema... | Now expects `class_extension` table | Table added to the list |

| New tests | Purpose |
|-----------|---------|
| `ClassExtensionTest.*` (integration) | Deadline for all enrolled students; latest-of rule with individual extensions (both orders); validation (earlier date, empty reason, other lecturer, student); re-grant replaces; reclassifies LATE; refused after moderation; late penalty removed at marking; resubmission allowed until the class deadline; all students notified; list shows kind and original deadline |
| `IT_DB.UpgradesVersion1DatabaseWithoutDataLoss` | Adaptive part: a real v1.0 schema with data is migrated to v2 |
| ST28, ST29 (system) | Lecturer uses the new menu option; students see "class" and the original deadline; an individual extension later than the class one shows "individual" |

## 4. Estimated effort and risk
About 1 day of implementation and 0.5 day of documentation. Risk: low. The effective deadline is computed in one function (`AssignmentService`), a benefit of the single-responsibility design. Lateness and penalty code need no change, and the regression suite protects them.
