# Coding Standard

Based on the C++ Core Guidelines and the Google C++ Style Guide, adapted as below. Rules have IDs so that code review comments can cite them. Automated checks are noted in brackets.

## Formatting (CS-F)
- **CS-F1** Formatting follows `.clang-format` (Google base, 4-space indent, 100-column limit). [clang-format]
- **CS-F2** One class per header where practical; related small structs may share a header (`Entities.h`).
- **CS-F3** Include order: own header, C/C++ standard headers, third-party, project headers, separated by blank lines.

## Naming (CS-N)
- **CS-N1** Types and enumerators: `PascalCase` (`MarkingService`, `SubmissionStatus::Late`).
- **CS-N2** Functions and variables: `camelCase` (`effectiveDeadline`, `rawMark`).
- **CS-N3** Private data members end in `_` (`repos_`); public struct fields do not.
- **CS-N4** Constants: `kPascalCase` (`kMaxFailedAttempts`).
- **CS-N5** Interfaces start with `I` and contain only pure virtual functions and a virtual destructor.
- **CS-N6** Namespaces: `lms`, `lms::ui`, `lms::text`, `lms::time_util`, `lms::test`.

## Language use (CS-L)
- **CS-L1** C++17 only, no compiler extensions (`CMAKE_CXX_EXTENSIONS OFF`).
- **CS-L2** No raw `new`/`delete`; ownership via `std::unique_ptr`, borrowing via references. RAII for every resource (`Database`, `Statement`, `EchoGuard`).
- **CS-L3** No C-style casts (`-Wold-style-cast`); `reinterpret_cast` only at C API boundaries, with a comment.
- **CS-L4** Use `std::optional` for "may be absent" values; never sentinel values such as `-1` for marks.
- **CS-L5** Mark `override` and `final` explicitly; polymorphic base classes have a virtual destructor (`-Wnon-virtual-dtor`).
- **CS-L6** `const` by default for locals, parameters passed by `const&` when larger than two words.
- **CS-L7** Range checks on floating-point input are written as `!(x >= lo && x <= hi)` so that NaN is rejected (lesson from DEF-01).

## Errors and validation (CS-E)
- **CS-E1** Expected failures throw a subclass of `lms::LmsError`; never `exit()` or `abort()` in library code.
- **CS-E2** Every service entry point first checks the actor's permission (`actor.require(...)`), then scope (course/department), then input validation, then business rules, then persists.
- **CS-E3** All writes that must succeed together run inside `ITransactionRunner::run`.
- **CS-E4** Error messages are written for the end user, say what to do, and never contain passwords or hashes.
- **CS-E5** Empty `catch` blocks are forbidden unless commented with the reason (`bugprone-empty-catch`).

## Security (CS-S)
- **CS-S1** SQL only in `lib/src/data`; every value is bound with `Statement::bind` (no string concatenation of values).
- **CS-S2** Passwords are only hashed/verified through `IPasswordHasher`; never logged, printed or stored in plain text.
- **CS-S3** The UI layer uses `UserSummary`, never `UserRecord`.

## Design (CS-D)
- **CS-D1** The presentation layer calls services only (no repositories, no SQL).
- **CS-D2** Functions have cyclomatic complexity <= 15 and <= 80 lines. [lizard -C 15]
- **CS-D3** Business rules cite their SRS ID in a comment where they are enforced (e.g. `// BR-07`).

## Build and review (CS-B)
- **CS-B1** Code builds without warnings using the project warning set (`cmake/CompilerWarnings.cmake`); CI on Linux uses `-Werror`.
- **CS-B2** cppcheck (warning, style, performance, portability) must report nothing; justified suppressions are inline with a reason.
- **CS-B3** Every change reaches `main` through a branch and a merge commit (see configuration management) after the tests pass.
- **CS-B4** Commit messages: `type(scope): summary` (types: feat, fix, test, refactor, docs, build, ci, chore), body explains *why*.
