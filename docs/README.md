# Project Documentation - index

Organised by the report sections required by the brief. Each row names the brief's deliverable and where it is satisfied.

| Report section | Brief deliverable | Document(s) |
|----------------|-------------------|-------------|
| 3 Project management plan | Feasibility, WBS, Gantt with dependencies/milestones/critical path, risk register (>= 6 risks, updated at gates) | [project-management-plan.md](03-project-management/project-management-plan.md), [risk-register.md](03-project-management/risk-register.md) |
| 4 Requirements engineering | Stakeholder analysis and conflict resolution | [stakeholder-analysis.md](04-requirements/stakeholder-analysis.md) |
| | SRS (ISO 29148 structure, >= 12 FR, >= 6 measurable NFR, MoSCoW, verifiable) | [srs.md](04-requirements/srs.md) |
| | Use case diagram (4 actors, include/extend/generalisation) + 3 fully dressed use cases incl. "Submit assignment" | [use-cases.md](04-requirements/use-cases.md) |
| 5 Software design document | Architecture + component diagram + rejected alternatives; class diagram with >= 2 patterns and SOLID; state, activity (swimlanes), 2 sequence diagrams; 3NF ERD, data dictionary, SQL schema, class-table mapping | [software-design-document.md](05-design/software-design-document.md), [diagrams/](diagrams/), [../sql/](../sql/) |
| 6 Implementation | Source code, build guide, coding standard | [../app](../app), [../lib](../lib), [../BUILD.md](../BUILD.md), [coding-standard.md](06-implementation/coding-standard.md) |
| 7 Verification and validation | Test plan (V-Model, EP/BVA), automated tests, results, coverage, system/acceptance tests, defect log | [test-plan.md](07-testing/test-plan.md), [test-report.md](07-testing/test-report.md), [defect-log.md](07-testing/defect-log.md), [reports/](07-testing/reports/), [../tests](../tests) |
| 8 CM and QA | Branching strategy, commits, tags, CI; static analysis, metrics, code review | [configuration-management.md](08-quality/configuration-management.md), [../.github/workflows/ci.yml](../.github/workflows/ci.yml), [static-analysis.md](08-quality/static-analysis.md), [metrics.md](08-quality/metrics.md), [code-review.md](08-quality/code-review.md) |
| 9 Deployment | Deployment diagram, installation guide, user manual per role, release notes, tag v1.0 | [installation-guide.md](09-deployment/installation-guide.md), [user-manuals/](09-deployment/user-manuals/), [release-notes.md](09-deployment/release-notes.md) |
| 10 Maintenance | CR1 classification and impact analysis, v1.1, regression results | [10-maintenance/](10-maintenance/) |
| 11 Professional issues | Data protection, security, accessibility, fairness | [professional-issues.md](11-professional-issues/professional-issues.md) |
| 14 Appendices | Traceability matrix, full test results, manuals, updated risk register | [traceability-matrix.md](appendices/traceability-matrix.md) |
| 1-14 | Final report (PDF) | [report/](report/) |

Diagram sources are PlantUML (`docs/diagrams/*.puml`); rendered PNG/SVG are in `docs/diagrams/rendered/`. To re-render: `java -jar plantuml.jar -tpng -o rendered docs/diagrams/*.puml`.
