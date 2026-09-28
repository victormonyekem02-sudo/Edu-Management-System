# Test Report - release v1.0

Date: 28 Sep 2026. Build: commit on branch `main` tagged `v1.0`.
Raw evidence: [`reports/`](reports/) (GoogleTest text and XML output, performance log, acceptance transcripts, coverage HTML).

## 1. Summary

| Level | Tests | Passed | Failed | Evidence |
|-------|------:|-------:|-------:|----------|
| Unit (UT_*, HasherTest) | 145 | 145 | 0 | `functional-results-v1.0.txt` |
| Integration (IT_*, service suites) | 92 | 92 | 0 | same |
| System (ST01-ST27) | 27 | 27 | 0 | same |
| **Functional total** | **264** | **264** | **0** | also `functional-results-v1.0.xml` |
| Performance (NFR-01..03) | 5 | 5 | 0 | `performance-results-v1.0.txt` |
| Acceptance (AT-01..AT-05) | 4 scenarios | 4 | 0 | `acceptance/` |
| Acceptance AT-06 (usability, manual) | 1 | - | - | not yet executed (needs a volunteer student at the school) |

Unit tests include the parameterised cases: 12 lateness boundaries, 11 penalty cases, 52 permission-matrix cases, 25 state transitions and 6 password-policy cases.

Compilers: all functional tests pass with **GCC 16.2** (Debug, `-Werror`, coverage instrumentation) and with **Clang 21** (Zig toolchain, Debug and Release).

## 2. Coverage of the business-logic layer (`lib/`)

Measured with gcov/gcovr on the GCC build (report: [`reports/coverage/index.html`](reports/coverage/index.html), text: [`reports/coverage-summary.txt`](reports/coverage-summary.txt)).

| Metric | Result | Target |
|--------|--------|--------|
| **Line coverage** | **95.9 %** (1 815 / 1 892) | >= 70 % |
| Function coverage | 94.9 % (263 / 277) | - |
| Branch coverage | 59.4 % (1 685 / 2 835) | - (reported for information) |

Lowest files: `LmsSystem.cpp` 84 % (`createDefault`, used only by the executable), `DirectoryService.cpp` 90 %, `Database.cpp` 90 % (SQLite bind-failure paths that cannot be triggered with valid statements). Branch coverage is lower because GCC counts the hidden exception-handling branches that the compiler generates for every call that can throw.

## 3. Performance (Release build, 800 students, production Argon2id)

| NFR | Measurement | Limit | Result |
|-----|-------------|-------|--------|
| NFR-01 | Login of student 800 | 2 000 ms | **153 ms** - PASS |
| NFR-02 | List assignments (20) for a student | 1 000 ms | **2.6 ms** - PASS |
| NFR-02 | Submit work | 1 000 ms | **1.9 ms** - PASS |
| NFR-03 | HoD moderation queue, 800 items | 2 000 ms | **108 ms** - PASS |
| NFR-03 | Release 800 marks (one transaction) | 5 000 ms | **338 ms** - PASS |

An earlier run on an idle machine measured 56 ms for login. Login time is dominated by Argon2id (by design); the rest is SQLite work, and indexes keep it in milliseconds.

## 4. Requirement coverage
Every FR-01..FR-21 and NFR-01..NFR-12 has at least one passing verification (see the SRS verification column and `docs/appendices/traceability-matrix.md`). Exceptions:
- NFR-10 (usability): the menu structure is verified automatically (ST17, ST18), but the timed first-use test AT-06 is still to be run with a real student.
- NFR-12 (portability): verified on Windows with two compilers; Linux and MSVC builds are verified by the CI pipeline once the repository is pushed to GitHub (risk R13).

## 5. Defects
8 defects found and closed during v1.0 verification (see `defect-log.md`); no open defects at Gate 3.

## 6. Acceptance observations
- AT-02/03 shows BR-02 working as specified in a way a user might not expect: the demo assignment's deadline was exactly two days before submission, and the work was submitted a few seconds after that point, so it counts as **3 started days late** (30 %). The user manuals explain the rule with examples so students and lecturers are not surprised.

## 7. Conclusion (Gate 3)
Exit criteria met: all Must requirements implemented and tested, 264/264 functional tests and 5/5 performance tests pass, line coverage of the business-logic layer is 95.9 %, and there are no open Critical/Major defects. **Recommendation: release v1.0.**
