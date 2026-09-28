# Third-party dependencies, pinned by version and SHA-256 so every build is reproducible.
# For an offline build, pre-download the archives and pass
#   -DFETCHCONTENT_SOURCE_DIR_SQLITE_AMALGAMATION=<dir> (and likewise ARGON2, GOOGLETEST).
include(FetchContent)
set(FETCHCONTENT_QUIET ON)

# ---- SQLite 3.46.1 (public domain) -------------------------------------------
FetchContent_Declare(sqlite_amalgamation
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
    URL_HASH SHA256=77823cb110929c2bcb0f5d48e4833b5c59a8a6e40cdea3936b99e199dbbe5784
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(sqlite_amalgamation)

add_library(sqlite3 STATIC "${sqlite_amalgamation_SOURCE_DIR}/sqlite3.c")
target_include_directories(sqlite3 SYSTEM PUBLIC "${sqlite_amalgamation_SOURCE_DIR}")
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_DQS=0                 # reject double-quoted string literals
    SQLITE_DEFAULT_FOREIGN_KEYS=1
    SQLITE_OMIT_LOAD_EXTENSION   # no run-time extension loading (attack surface)
    SQLITE_THREADSAFE=1)
find_package(Threads REQUIRED)
target_link_libraries(sqlite3 PUBLIC Threads::Threads)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(sqlite3 PRIVATE -w)
endif()

# ---- Argon2 reference implementation 20190702 (CC0 / Apache-2.0) ------------
FetchContent_Declare(argon2
    URL https://github.com/P-H-C/phc-winner-argon2/archive/refs/tags/20190702.zip
    URL_HASH SHA256=506a80b90ac3ca8407636f3b26b7fed87a55f2f1275cde850cd6698903c4f008
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
# Upstream ships a Makefile rather than a CMakeLists.txt, so this only populates the sources.
FetchContent_MakeAvailable(argon2)

add_library(argon2 STATIC
    "${argon2_SOURCE_DIR}/src/argon2.c"
    "${argon2_SOURCE_DIR}/src/core.c"
    "${argon2_SOURCE_DIR}/src/blake2/blake2b.c"
    "${argon2_SOURCE_DIR}/src/encoding.c"
    "${argon2_SOURCE_DIR}/src/ref.c"
    "${argon2_SOURCE_DIR}/src/thread.c")
target_include_directories(argon2 SYSTEM PUBLIC "${argon2_SOURCE_DIR}/include")
target_include_directories(argon2 PRIVATE "${argon2_SOURCE_DIR}/src")
target_compile_definitions(argon2 PRIVATE ARGON2_NO_THREADS)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # Always optimise the hash: an unoptimised Debug build of Argon2 is several times
    # slower and would distort login timings (NFR-01).
    target_compile_options(argon2 PRIVATE -w -O2)
endif()

# ---- GoogleTest 1.15.2 (BSD-3-Clause), tests only ----------------------------
if(LMS_BUILD_TESTS)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        URL_HASH SHA256=f179ec217f9b3b3f3c6e8b02d3e7eda997b49e4ce26d6b235c9053bec9c0bf9f
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()
