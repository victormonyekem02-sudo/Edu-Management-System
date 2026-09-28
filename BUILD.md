# Build Guide

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| CMake | >= 3.20 | `pip install cmake` works if you cannot install system-wide |
| C++17 compiler | GCC >= 9, Clang >= 10, or MSVC 2019+ | Linux: `sudo apt install build-essential`; Windows: Visual Studio "Desktop development with C++" or MinGW-w64 |
| Ninja (optional) | any | faster builds: `-G Ninja` |
| Git | any | |
| Internet on first configure | - | CMake downloads SQLite 3.46.1, Argon2 20190702 and GoogleTest 1.15.2 (pinned by SHA-256) |

## Build and run

```sh
git clone <repository-url> lms && cd lms
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
# run (creates lms.db in the current folder on first start)
./build/app/lms                      # Linux / MinGW
build\app\Release\lms.exe            # Visual Studio generator
./build/app/lms --seed-demo          # demonstration school, password Passw0rd2026
```

## Tests

```sh
ctest --test-dir build -C Release -L functional --output-on-failure   # 264 unit/integration/system tests
ctest --test-dir build -C Release -L performance -V                   # NFR-01..03 timings (use Release)
./build/tests/lms_tests --gtest_filter='SystemTest.*'                 # run a subset
```

## Coverage (GCC or Clang)

```sh
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DLMS_ENABLE_COVERAGE=ON
cmake --build build-cov --parallel
ctest --test-dir build-cov -L functional
gcovr --root . --filter 'lib/' --object-directory build-cov --html-details coverage/index.html --print-summary
```

## Static analysis and metrics

```sh
cppcheck --enable=warning,style,performance,portability --std=c++17 --language=c++ \
  --inline-suppr --suppress=missingIncludeSystem --suppress=unusedFunction \
  --suppress=useStlAlgorithm --check-level=exhaustive -I lib/include -I app/include lib/src app/src
clang-tidy -p build lib/src/services/*.cpp        # uses .clang-tidy
lizard lib/src app/src -l cpp -C 15 -w            # prints functions with CCN > 15 (none)
```

## CMake options

| Option | Default | Effect |
|--------|---------|--------|
| `LMS_BUILD_TESTS` | ON | Build the GoogleTest suites |
| `LMS_ENABLE_COVERAGE` | OFF | `--coverage` instrumentation of `lms_core` |
| `LMS_WARNINGS_AS_ERRORS` | OFF | `-Werror` / `/WX` for project targets (ON in Linux CI) |
| `LMS_STATIC_RUNTIME` | OFF | Static C/C++ runtime (MSVC `/MT`, MinGW `-static`) for release packages |

## Offline build
Download the three archives listed in `cmake/Dependencies.cmake` once, unzip them, and point CMake at them:

```sh
cmake -S . -B build -DFETCHCONTENT_SOURCE_DIR_SQLITE_AMALGAMATION=/path/sqlite-amalgamation-3460100 \
  -DFETCHCONTENT_SOURCE_DIR_ARGON2=/path/phc-winner-argon2-20190702 \
  -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/path/googletest-1.15.2
```

## Package a release

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLMS_BUILD_TESTS=OFF -DLMS_STATIC_RUNTIME=ON
cmake --build build --config Release
cpack --config build/CPackConfig.cmake -C Release     # -> ModeratedLMS-<version>-<OS>.zip
```

## Project layout

```
app/        presentation layer (console UI, main)            -> lms_ui, lms
lib/        domain, services, policy, security, data access  -> lms_core
sql/        numbered schema migrations (embedded at build time)
tests/      unit/, integration/, system/, performance/, support/
docs/       all project documents and diagrams (see docs/README.md)
cmake/      dependency pinning and warning flags
.github/    CI pipeline
```
