# Moderated Learning Management System

A C++17 console LMS for a secondary school, where every mark is moderated by the Head of Department before students see it. Built for the CS4433 Software Engineering final project (Waterfall process with V-Model verification).

- **Roles:** Administrator, Lecturer, Head of Department, Student, each with its own menu and permissions.
- **Rules:** individual extensions; resubmission before the deadline; late work always accepted with a 10 %-per-day penalty (zero after 3 days); marks hidden until moderated and released; every mark change audited.
- **Tech:** C++17, CMake, SQLite (offline, one file), Argon2id password hashing, GoogleTest, GitHub Actions.

## Quick start

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release -L functional      # 278 tests
./build/app/lms --db demo.db --seed-demo             # demo accounts, password Passw0rd2026
```

Demo accounts: `admin`, `hod.sci`, `hod.lang`, `lect.sci`, `lect.lang`, `stud1` ... `stud5`. Full details: [BUILD.md](BUILD.md).

## Status

| | |
|---|---|
| Release | v1.1 (tags `v1.0`, `v1.1`) |
| Tests | 278 functional + 5 performance, all passing |
| Business-logic line coverage | 96.1 % |
| Static analysis | cppcheck 0, clang-tidy 0 findings |
| Max cyclomatic complexity | 14 |

## Documentation

All deliverables are in [`docs/`](docs/README.md), organised by the section numbers of the final report.

## Repository layout

```
app/     console presentation layer          lib/     domain, services, data access, security
sql/     schema migrations (embedded)        tests/   unit, integration, system, performance, acceptance
docs/    plans, requirements, design, test and QA reports, manuals, diagrams
```
