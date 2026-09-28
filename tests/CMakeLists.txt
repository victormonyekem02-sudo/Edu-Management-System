include(GoogleTest)

# Unit tests: one class/function in isolation, no database (V-Model: detailed design).
# Integration tests: services + SQLite repositories together (V-Model: architecture).
# System tests: the real console application driven by scripted input (V-Model: SRS).
add_executable(lms_tests
    support/TestSchool.cpp
    unit/test_lateness.cpp
    unit/test_penalty_policy.cpp
    unit/test_users.cpp
    unit/test_lifecycle.cpp
    unit/test_security.cpp
    unit/test_text_time.cpp
    unit/test_events.cpp
    integration/test_database.cpp
    integration/test_auth_service.cpp
    integration/test_admin_service.cpp
    integration/test_assignment_service.cpp
    integration/test_submission_service.cpp
    integration/test_marking_service.cpp
    integration/test_moderation_service.cpp
    integration/test_notifications.cpp
    system/test_system_usecases.cpp)
target_include_directories(lms_tests PRIVATE support)
target_link_libraries(lms_tests PRIVATE lms_ui lms_core GTest::gtest_main)
lms_set_warnings(lms_tests)
gtest_discover_tests(lms_tests DISCOVERY_TIMEOUT 60 PROPERTIES LABELS "functional")

# Performance tests use production Argon2 parameters and 800 students (NFR-01..NFR-03).
add_executable(lms_perf_tests support/TestSchool.cpp performance/test_performance.cpp)
target_include_directories(lms_perf_tests PRIVATE support)
target_link_libraries(lms_perf_tests PRIVATE lms_core GTest::gtest_main)
lms_set_warnings(lms_perf_tests)
gtest_discover_tests(lms_perf_tests DISCOVERY_TIMEOUT 60 PROPERTIES LABELS "performance")
