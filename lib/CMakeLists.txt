# lms_core: domain, business logic (services), security and data-access layers.
# It has no console I/O so that it can be tested in isolation and reused by a GUI later.

# ---- Embed the SQL migrations (sql/NNN_*.sql) into a generated header -----------
file(GLOB LMS_MIGRATION_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/sql/[0-9][0-9][0-9]_*.sql")
list(SORT LMS_MIGRATION_FILES)
set(LMS_MIGRATIONS_CPP "")
foreach(migration IN LISTS LMS_MIGRATION_FILES)
    file(READ "${migration}" migration_sql)
    string(APPEND LMS_MIGRATIONS_CPP "    R\"lmssql(${migration_sql})lmssql\",\n")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${migration}")
endforeach()
configure_file(src/data/GeneratedMigrations.h.in
               "${CMAKE_CURRENT_BINARY_DIR}/generated/lms/data/GeneratedMigrations.h" @ONLY)

add_library(lms_core STATIC
    src/common/TextUtil.cpp
    src/common/TimeUtil.cpp
    src/domain/Role.cpp
    src/domain/SubmissionStatus.cpp
    src/domain/User.cpp
    src/policy/LatePenaltyPolicy.cpp
    src/security/PasswordHasher.cpp
    src/data/Database.cpp
    src/data/SqliteRepositories.cpp
    src/events/SubmissionEvents.cpp
    src/services/AdminService.cpp
    src/services/AssignmentService.cpp
    src/services/AuthService.cpp
    src/services/DirectoryService.cpp
    src/services/LmsSystem.cpp
    src/services/MarkingService.cpp
    src/services/ModerationService.cpp
    src/services/NotificationService.cpp
    src/services/SubmissionService.cpp)

target_include_directories(lms_core
    PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(lms_core PRIVATE sqlite3 argon2)
if(WIN32)
    target_link_libraries(lms_core PRIVATE bcrypt)
endif()
target_compile_definitions(lms_core PUBLIC LMS_VERSION="${PROJECT_VERSION}")
lms_set_warnings(lms_core)
lms_enable_coverage(lms_core)
