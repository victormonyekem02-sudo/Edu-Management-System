# lms_ui: presentation layer (console). Kept as a library so the system tests can drive
# the real menus with scripted input.
add_library(lms_ui STATIC
    src/Application.cpp
    src/ConsoleIO.cpp
    src/DemoData.cpp
    src/RoleMenu.cpp
    src/AdminMenu.cpp
    src/LecturerMenu.cpp
    src/HodMenu.cpp
    src/StudentMenu.cpp)
target_include_directories(lms_ui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(lms_ui PUBLIC lms_core)
lms_set_warnings(lms_ui)

add_executable(lms src/main.cpp)
target_link_libraries(lms PRIVATE lms_ui)
lms_set_warnings(lms)
