# Risk Register

Scales: Likelihood (L) and Impact (I) from 1 (very low) to 5 (very high); **Exposure = L x I** (1-25). Rating: 1-6 Low, 8-12 Medium, 15-25 High. The owner is the person responsible for the mitigation. In this individual project the student owns most risks; stakeholders are named where the mitigation depends on them.

## 1. Baseline register (Planning, week 1)

| ID | Category | Risk (cause -> event -> consequence) | L | I | Exp. | Mitigation / contingency | Owner |
|----|----------|--------------------------------------|---|---|-----:|--------------------------|-------|
| R1 | Technical | Unfamiliar or missing C++ toolchain / CMake set-up -> build problems -> implementation delayed | 3 | 4 | 12 | Set up CMake, dependencies and CI in the first implementation days; pin dependency versions; document the build | Student |
| R2 | Project | Scope and documentation volume exceed 7 weeks -> late or incomplete deliverables | 4 | 5 | **20** | WBS with float; MoSCoW scope rule; write documents alongside each phase, not at the end | Student |
| R3 | Project | Requirements misunderstood or conflicting (e.g. release rule) -> rework after design | 3 | 4 | 12 | Explicit conflict/ambiguity tables; Gate 1 sign-off; traceability matrix | Student / Supervisor |
| R4 | Technical | Weak password storage or SQL injection -> security breach of learner data | 2 | 5 | 10 | Argon2id reference library; prepared statements only; security tests; code review checklist | Student |
| R5 | Technical | Late-penalty/date logic errors (boundaries, time zones) -> unfair marks | 3 | 5 | **15** | Precise BR-02 definition; integer epoch arithmetic; EP/BVA unit tests; injectable clock | Student / HoD (rule owner) |
| R6 | Technical | Data loss (disk failure, crash during write) -> assessment records lost | 2 | 5 | 10 | SQLite transactions + WAL; daily backup procedure in installation guide | IT technician |
| R7 | People | Illness or competing coursework of the single developer -> lost days | 3 | 4 | 12 | 2 days float; front-load critical-path tasks; keep work in Git daily | Student |
| R8 | Technical / environment | Unreliable internet -> cannot fetch dependencies or push to GitHub | 3 | 3 | 9 | Download dependencies once and reuse (FETCHCONTENT_SOURCE_DIR_*); offline build option; commit locally, push when online | Student |
| R9 | People | Users (teachers) find the system hard to use -> low adoption, errors | 3 | 3 | 9 | Numbered menus, role manuals, demo data for training, usability acceptance test AT-06 | Student / Principal |
| R10 | Project | Viva: student cannot explain design decisions -> marks withheld | 2 | 5 | 10 | Keep design rationale in documents; self code review; rehearse the demo script | Student |
| R11 | Technical | Performance on old hardware with 800 students | 2 | 3 | 6 | Indexes; measured NFR tests in CI; tunable Argon2 parameters | Student |

## 2. Update at Gate 1 (requirements)

| ID | Change | New L x I | Reason |
|----|--------|-----------|--------|
| R3 | Reduced | 2 x 4 = 8 | 7 conflicts and 12 ambiguities resolved and recorded; SRS reviewed |
| R5 | Unchanged | 3 x 5 = 15 | Rule now precise (BR-01/02), but not yet implemented or tested |
| R12 | **New** (Technical): SQLite on a network share corrupts data if the school tries multi-computer use | 2 x 5 = 10 | Mitigation: single-computer deployment stated in SRS (AS-1, NFR-09) and installation guide; owner: IT technician |

## 3. Update at Gate 2 (design)

| ID | Change | New L x I | Reason |
|----|--------|-----------|--------|
| R4 | Reduced | 1 x 5 = 5 | Design fixes Argon2id, repository layer with prepared statements, DB CHECK rejecting non-Argon2id hashes |
| R5 | Reduced | 2 x 5 = 10 | Lateness isolated in `LatenessCalculator` and Strategy classes that are unit-testable with a `FixedClock` |
| R6 | Reduced | 2 x 4 = 8 | Append-only audit via triggers, transactional services, WAL |
| R2 | Unchanged | 4 x 5 = 20 | Documentation volume still the main threat |

## 4. Update at Gate 3 (test readiness)

| ID | Change | New L x I | Reason |
|----|--------|-----------|--------|
| R1 | **Occurred, closed** | - | The development PC had no C++ compiler or CMake and installing Visual Studio was not possible. Contingency: portable toolchains (Zig/Clang 21 via pip, later MinGW-w64 GCC 16) and CMake/Ninja via pip. GCC building with -Werror found DEF-03 before CI would have. |
| R8 | **Occurred, closed** | - | Downloads broke repeatedly (pip `IncompleteRead`). Contingency: retries with resume; dependencies pinned by SHA-256 and reused from a local cache through `FETCHCONTENT_SOURCE_DIR_*`. |
| R5 | Reduced | 1 x 5 = 5 | 12 lateness and 11 penalty boundary cases plus 6 integration cases pass; DEF-01 (NaN accepted) found and fixed |
| R4 | Reduced | 1 x 5 = 5 | Injection tests pass; self-review found and fixed a login timing side channel (R1 in code review) |
| R11 | Closed | 1 x 3 = 3 | NFR-01..03 measured far inside limits (login 56-153 ms) |
| R2 | Reduced | 3 x 4 = 12 | Implementation and tests complete; remaining work is documentation and CR1 |
| R13 | **New** (Technical): CI on GitHub Actions has never run (no remote configured on the development PC) -> pipeline errors discovered late | 3 x 2 = 6 | Mitigation: the Linux configuration (GCC, -Werror) was reproduced locally; push early and fix CI before the viva; owner: Student |

## 5. Current top risks (at release v1.0)
1. **R2** schedule/documentation (12) - monitored weekly.
2. **R12** network-share misuse (10) - installation guide warning.
3. **R10** viva preparation (10) - demo script in the user manuals, design rationale in the SDD.
