cmake_minimum_required(VERSION 3.20)

project(ModeratedLMS
    VERSION 1.1.0
    DESCRIPTION "Moderated Learning Management System (CS4433 final project)"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(LMS_BUILD_TESTS "Build the automated GoogleTest suite" ON)
option(LMS_ENABLE_COVERAGE "Instrument lms_core for line coverage (GCC/Clang only)" OFF)
option(LMS_WARNINGS_AS_ERRORS "Treat compiler warnings in project code as errors" OFF)
option(LMS_STATIC_RUNTIME "Link the C/C++ runtime statically (release packages need no redistributable)" OFF)

if(LMS_STATIC_RUNTIME)
    if(MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    elseif(MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Dependencies)

add_subdirectory(lib)
add_subdirectory(app)

if(LMS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---- Packaging (used for the v1.x release archives) -------------------------
install(TARGETS lms RUNTIME DESTINATION bin)
install(FILES
    README.md
    docs/09-deployment/installation-guide.md
    docs/09-deployment/release-notes.md
    DESTINATION doc)
install(DIRECTORY docs/09-deployment/user-manuals DESTINATION doc)

set(CPACK_PACKAGE_NAME "ModeratedLMS")
set(CPACK_PACKAGE_VENDOR "CS4433 Software Engineering")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME "ModeratedLMS-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
include(CPack)
